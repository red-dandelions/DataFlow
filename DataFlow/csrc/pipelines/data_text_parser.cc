// 11.6
#include "data_text_parser.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <span>
#include <string_view>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "pybind11/pybind11.h"

#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/common/functions.h"
#include "DataFlow/csrc/streams/batch_row.h"
#include "DataFlow/csrc/streams/inflate_stream.h"

namespace data_flow {
DataTextParser::~DataTextParser() { VLOG(1) << "[DataTextParser] destructor"; }

std::shared_ptr<StreamMeta> DataTextParser::output_stream_meta() const {
  return output_stream_meta_;
}

PyObject* DataTextParser::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(BatchRow),
      absl::StrFormat("Stream is not of type BatchRow, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));
  auto stream_ptr = std::dynamic_pointer_cast<BatchRow>(stream->shared_from_this());
  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to BatchRow failed");

  return pybind11::cast(stream_ptr).release().ptr();
}

DataTextParser::DataTextParser(std::shared_ptr<DataPipeline> pipeline, const std::string& format,
                               std::vector<Column>&& columns, const char field_delim)
    : field_delim_(field_delim) {
  DATAFLOW_THROW_IF(
      pipeline->output_stream_meta()->stream_type_index() != typeid(InflateStream),
      absl::StrFormat(
          "Input DataPipeline must produce InflateStream, got: %s",
          demangle_str_name(pipeline->output_stream_meta()->stream_type_index().name())));

  input_pipeline_ = pipeline;

  // 解析 format 字段
  format_fields_ = absl::StrSplit(format, field_delim_);

  output_stream_meta_ = std::make_shared<BatchRowMeta>(std::move(columns));
}

absl::StatusOr<std::shared_ptr<Stream>> DataTextParser::next() {
  std::string_view line = try_read_line_from_inflate_stream();
  if (line.empty() && line_buffer_.empty()) {
    VLOG(3) << "[DataTextParser] end of input";
    return nullptr;
  }

  auto batch_row = std::make_shared<BatchRow>(output_stream_meta_);

  try_parse_line(batch_row, line.empty() ? line_buffer_ : line);

  return batch_row;
}

std::string_view DataTextParser::try_read_line_from_inflate_stream() {
  line_buffer_.clear();
  while (true) {
    // inflate stream 数据消费完了，重新获取一个
    if (inflate_stream_ == nullptr) {
      auto status = input_pipeline_->next();
      DATAFLOW_THROW_IF(!status.ok(),
                        absl::StrFormat("error: %s", status.status().message().data()));
      auto stream = status.value();
      if (stream == nullptr) {
        VLOG(3) << "[DataTextParser] end of input";
        return std::string_view();
      }
      inflate_stream_ = std::dynamic_pointer_cast<InflateStream>(stream);
      DATAFLOW_THROW_IF(
          inflate_stream_ == nullptr,
          absl::StrFormat("expected a inflate_stream, but got: %s",
                          demangle_str_name(stream->stream_meta()->stream_type_index().name())));
      chunk_buffer_ = std::span<const char>{};
      chunk_offset_ = 0;
    }

    // 解压缩一个 chunk buffer
    if (chunk_buffer_.empty() || chunk_offset_ >= chunk_buffer_.size()) {
      chunk_buffer_ = inflate_stream_->read_chunk(kPerParseSize);
      if (chunk_buffer_.empty()) {
        inflate_stream_ = nullptr;
        chunk_offset_ = 0;
        continue;
      }
    }

    size_t beg_pos = chunk_offset_;
    size_t end_pos = beg_pos;
    while (chunk_offset_ < chunk_buffer_.size() && chunk_buffer_[end_pos] != '\n') {
      ++chunk_offset_;
      ++end_pos;
    }

    // 到末尾没有遇到 \n
    if (chunk_offset_ >= chunk_buffer_.size()) {
      line_buffer_ += std::string(chunk_buffer_.data() + beg_pos, end_pos - beg_pos);
      chunk_buffer_ = std::span<const char>{};
      chunk_offset_ = 0;
      continue;
    }

    // 接上一块的 chunk 数据
    if (!line_buffer_.empty()) {
      line_buffer_ += std::string(chunk_buffer_.data() + beg_pos, end_pos - beg_pos);
      ++chunk_offset_;
      return std::string_view();
    }

    // 截取当前 chunk buffer 数据
    std::string_view l = std::string_view(chunk_buffer_.data() + beg_pos, end_pos - beg_pos);
    ++chunk_offset_;
    return l;
  }

  DATAFLOW_THROW_IF(true, absl::StrFormat("should not be here."));
  return std::string_view();
}

void DataTextParser::try_parse_line(std::shared_ptr<BatchRow> batch_row, std::string_view line) {
  size_t field_idx = 0;
  size_t beg_field_pos = 0;
  size_t end_field_pos = 0;

  const auto skip_filed_func = [&]() -> void {
    while (end_field_pos < line.size() && line[end_field_pos] != field_delim_) {
      ++end_field_pos;
    }
  };

  while (field_idx < format_fields_.size() && end_field_pos < line.size()) {
    const auto& field_name = format_fields_[field_idx];
    if (field_name != dense_slot_field_ && field_name != sparse_slot_field_) {
      skip_filed_func();
      auto iter = external_data_.find(field_name);
      if (iter != external_data_.end()) {
        iter->second = std::string(
            std::string_view(line.data() + beg_field_pos, end_field_pos - beg_field_pos));
      }
    } else {
      if (field_name == dense_slot_field_) {
        size_t rel = try_parse_dense_slot(batch_row, line.data() + beg_field_pos, 0,
                                          line.size() - beg_field_pos);
        end_field_pos = beg_field_pos + rel;
      } else if (field_name == sparse_slot_field_) {
        size_t rel = try_parse_sparse_slot(batch_row, line.data() + beg_field_pos, 0,
                                           line.size() - beg_field_pos);
        end_field_pos = beg_field_pos + rel;
      } else [[unlikely]] {
        DATAFLOW_THROW_IF(true, absl::StrFormat("should not be here, field name: %s", field_name));
      }
    }

    ++field_idx;
    ++end_field_pos;
    beg_field_pos = end_field_pos;
  }

  // check
  for (size_t i = 0; i < output_stream_meta_->original_column_size; ++i) {
    auto row_chunk = batch_row->get_column_block(i);
    DATAFLOW_THROW_IF(row_chunk->byte_size == 0,
                      absl::StrFormat("column: %s is empty, line: %s",
                                      output_stream_meta_->get_column_by_index(i).name, line));
  }
}

size_t DataTextParser::try_parse_dense_slot(std::shared_ptr<BatchRow> batch_row, const char* data,
                                            size_t pos, size_t end_pos) {
  const auto& column_idx_map = output_stream_meta_->column_name_to_index;
  const auto& columns = output_stream_meta_->columns;

  // pos / end_pos 是相对于 data 指针的偏移（0 .. end_pos)
  const auto is_field_end = [&](size_t p) { return p >= end_pos || data[p] == field_delim_; };
  const auto is_slot_delim = [&](size_t p) { return p < end_pos && data[p] == slot_delim_; };

  while (!is_field_end(pos)) {
    // 解析 slot_name
    size_t slot_beg = pos;
    while (pos < end_pos && data[pos] != slot_value_delim_ && data[pos] != slot_delim_ &&
           data[pos] != value_delim_ && data[pos] != field_delim_) {
      ++pos;
    }
    size_t slot_len = pos - slot_beg;
    DATAFLOW_THROW_IF(slot_len == 0, absl::StrFormat("empty slot name in dense field: %s",
                                                     std::string_view(data, end_pos)));
    if (pos >= end_pos || data[pos] != slot_value_delim_) {
      DATAFLOW_THROW_IF(true,
                        absl::StrFormat("parse error in dense, please check format, field: %s",
                                        std::string_view(data, end_pos)));
    }
    // skip '@'
    ++pos;

    std::string_view slot_name(data + slot_beg, slot_len);
    // 检查是否在 column map 中
    auto iter = column_idx_map.find(slot_name);
    // skip slot
    if (iter == column_idx_map.end()) {
      // skip unused slot
      while (pos < end_pos && data[pos] != slot_delim_ && data[pos] != field_delim_) {
        pos++;
      }
      if (pos < end_pos && data[pos] == slot_delim_) {
        ++pos;
      }
      continue;
    }

    // parse data
    const auto& column = columns[iter->second];
    DATAFLOW_THROW_IF(
        column.item_count <= 0 || column.column_type != ColumnType::kDense,
        absl::StrFormat(
            "check dense column category or dense column should set shape, dense slot: %s",
            column.name));
    size_t written = 0;
    float* column_data = reinterpret_cast<float*>(
        batch_row->alloc_column_block_data(iter->second, sizeof(float) * column.item_count));

    // parse values separated by value_delim_ (',') until slot_delim_ ';' or field_delim_
    while (pos < end_pos && data[pos] != slot_delim_ && data[pos] != field_delim_) {
      // find end of this value
      size_t val_beg = pos;
      while (pos < end_pos && data[pos] != value_delim_ && data[pos] != slot_delim_ &&
             data[pos] != field_delim_) {
        ++pos;
      }
      size_t val_len = pos - val_beg;
      DATAFLOW_THROW_IF(val_len == 0, absl::StrFormat("empty value for slot %s, context: %s",
                                                      slot_name, std::string_view(data, end_pos)));

      // parse float
      std::string_view val(data + val_beg, val_len);
      DATAFLOW_THROW_IF(!absl::SimpleAtof(val, column_data + written),
                        absl::StrFormat("parse dense value error, value string: %s, context: %s",
                                        val, std::string_view(data, end_pos)));
      ++written;

      if (written >= column.item_count) {
        // 已经填满，跳过该 slot 剩余的值，进入下一个 slot
        while (pos < end_pos && data[pos] != slot_delim_ && data[pos] != field_delim_) ++pos;
        break;
      }

      // skip ',' if present
      if (pos < end_pos && data[pos] == value_delim_) ++pos;
    }

    // 如果写入值少于 shape，填充 0
    for (size_t i = written; i < column.item_count; ++i) {
      column_data[i] = 0.0f;
    }

    // 如果当前是 ';'，跳过它；如果是 '|'（field_delim_）则不要跳过，让外层识别字段结束
    if (pos < end_pos && data[pos] == slot_delim_) ++pos;
  }

  // 返回相对于 data 的位置（结束位置，指向 field_delim_ 或等于 end_pos）
  return pos;
}

size_t DataTextParser::try_parse_sparse_slot(std::shared_ptr<BatchRow> batch_row, const char* data,
                                             size_t pos, size_t end_pos) {
  // 解析格式: slot_name@id:weight ; slot_name@id:weight ; ... (分隔符为 slot_delim_，字段结束为
  // field_delim_) 收集每个 column(index) 对应的 id 列表，最后一次性写入 batch_row weight
  // 不用了，如果以后需要再解析
  const auto& column_idx_map = output_stream_meta_->column_name_to_index;
  const auto& columns = output_stream_meta_->columns;

  auto is_field_end = [&](size_t p) { return p >= end_pos || data[p] == field_delim_; };
  // vector id
  std::vector<int64_t> tmp;

  while (!is_field_end(pos)) {
    // parse slot_name (until '@' or slot_delim_ or field_delim_)
    size_t slot_beg = pos;
    while (pos < end_pos && data[pos] != slot_value_delim_ && data[pos] != slot_delim_ &&
           data[pos] != field_delim_) {
      ++pos;
    }
    size_t slot_len = pos - slot_beg;
    DATAFLOW_THROW_IF(slot_len == 0, absl::StrFormat("empty slot name in sparse field: %s",
                                                     std::string_view(data, end_pos)));

    // 必须遇到 '@' 才能继续解析 id:weight
    if (pos >= end_pos || data[pos] != slot_value_delim_) {
      DATAFLOW_THROW_IF(true,
                        absl::StrFormat("parse error in spars, please check format, field: %s",
                                        std::string_view(data, end_pos)));
    }
    ++pos;  // skip '@'

    // 构造 slot_name 字符串用于在 column map 中查找
    std::string_view slot_name(data + slot_beg, slot_len);
    auto iter = column_idx_map.find(slot_name);
    if (iter == column_idx_map.end()) {
      // skip unused slot
      while (pos < end_pos && data[pos] != slot_delim_ && data[pos] != field_delim_) {
        pos++;
      }
      if (pos < end_pos && data[pos] == slot_delim_) {
        ++pos;
      }
      continue;
    }

    const Column& column = columns[iter->second];
    DATAFLOW_THROW_IF(column.column_type != ColumnType::kSparse,
                      absl::StrFormat("column: %s should be sparse", column.name));
    bool need_tmp = (column.item_count < 0);
    size_t written = 0;
    int64_t* column_data = nullptr;
    if (!need_tmp) {
      column_data = reinterpret_cast<int64_t*>(
          batch_row->alloc_column_block_data(iter->second, sizeof(int64_t) * column.item_count));
    } else {
      tmp.clear();
    }

    while (pos < end_pos && data[pos] != slot_delim_ && data[pos] != field_delim_) {
      // parse id (直到 ':' 或 slot_delim_/field_delim_)
      size_t id_beg = pos;
      while (pos < end_pos && data[pos] != id_weight_delim_ && data[pos] != slot_delim_ &&
             data[pos] != field_delim_) {
        ++pos;
      }
      DATAFLOW_THROW_IF(
          pos >= end_pos || data[pos] != id_weight_delim_,
          absl::StrFormat("parse sparse error, field: %s", std::string_view(data, end_pos)));
      size_t id_len = pos - id_beg;
      std::string_view id_str = std::string_view(data + id_beg, id_len);
      if (need_tmp) {
        int64_t id;
        DATAFLOW_THROW_IF(!absl::SimpleAtoi(id_str, &id),
                          absl::StrFormat("parse sparse value error, value string: %s, context: %s",
                                          id_str, std::string_view(data, end_pos)));
        tmp.emplace_back(std::move(id));
      } else {
        DATAFLOW_THROW_IF(!absl::SimpleAtoi(id_str, column_data + written),
                          absl::StrFormat("parse sparse value error, value string: %s, context: %s",
                                          id_str, std::string_view(data, end_pos)));
        ++written;
      }

      // skip ':'
      ++pos;

      // skip weight
      while (pos < end_pos && data[pos] != value_delim_ && data[pos] != slot_delim_ &&
             data[pos] != field_delim_) {
        pos++;
      }

      // next id
      if (pos < end_pos && data[pos] == value_delim_) {
        ++pos;
        continue;
      }

      // next slot or next field
      if (pos < end_pos && data[pos] == slot_delim_ || data[pos] == field_delim_) {
        break;
      }
    }

    // 填充数据
    if (need_tmp && !tmp.empty()) {
      void* column_ptr =
          batch_row->alloc_column_block_data(iter->second, sizeof(int64_t) * tmp.size());
      memcpy(column_ptr, tmp.data(), sizeof(int64_t) * tmp.size());
    } else {
      // 如果写入值少于 shape，填充 0
      for (size_t i = written; i < column.item_count; ++i) {
        column_data[i] = 0;
      }
    }

    // 如果当前是 ';'，跳过它；如果是 '|'（field_delim_）则不要跳过，让外层识别字段结束
    if (pos < end_pos && data[pos] == slot_delim_) ++pos;
  }

  return pos;
}

}  // namespace data_flow
