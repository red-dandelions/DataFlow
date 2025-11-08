// 11.6
#include "data_text_parser.h"

#include <memory>
#include <span>
#include <string_view>

#include "absl/strings/str_split.h"
#include "glog/logging.h"
#include "pybind11/pybind11.h"

#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/common/functions.h"
#include "DataFlow/csrc/streams/inflate_stream.h"

namespace data_flow {
DataTextParser::~DataTextParser() { VLOG(1) << "[DataTextParser] destructor"; }

std::shared_ptr<StreamMeta> DataTextParser::output_stream_meta() const {
  return output_stream_meta_;
}

PyObject *DataTextParser::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(BatchRow),
      absl::StrFormat("Stream is not of type BatchRow, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));
  auto stream_ptr = std::dynamic_pointer_cast<BatchRow>(stream->shared_from_this());
  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to BatchRow failed");

  return pybind11::cast(stream_ptr).release().ptr();
}

DataTextParser::DataTextParser(std::shared_ptr<DataPipeline> pipeline, const std::string &format,
                               std::vector<Column> &&columns, const char field_delim)
    : field_delim_(field_delim) {
  DATAFLOW_THROW_IF(
      pipeline->output_stream_meta()->stream_type_index() != typeid(InflateStream),
      absl::StrFormat(
          "Input DataPipeline must produce InflateStream, got: %s",
          demangle_str_name(pipeline->output_stream_meta()->stream_type_index().name())));

  input_pipeline_ = pipeline;

  // 解析 format 字段
  absl::StrSplit(format, field_delim_, &format_fields_);

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
        VLOG(1) << "[DataTextParser] end of input";
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
  
}

}  // namespace data_flow
