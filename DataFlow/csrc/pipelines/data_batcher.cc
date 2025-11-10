// 11.10

#include "data_batcher.h"

#include <chrono>
#include <cstring>
#include <memory>
#include <sys/types.h>
#include <thread>

#include "glog/logging.h"
#include "pybind11/numpy.h"

#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/common/functions.h"
#include "DataFlow/csrc/streams/batch.h"
#include "DataFlow/csrc/streams/batch_row.h"

namespace data_flow {

DataBatcher::DataBatcher(std::shared_ptr<DataPipeline> pipeline, size_t batch_size,
                         const std::vector<std::pair<pybind11::function, pybind11::tuple>>& fn_ars,
                         bool drop_last_batch)
    : input_pipeline_(pipeline),
      batch_size_(batch_size),
      fn_ars_(fn_ars),
      drop_last_batch_(drop_last_batch) {
  auto stream_meta = pipeline->output_stream_meta();
  DATAFLOW_THROW_IF(stream_meta->stream_type_index() != typeid(BatchRow),
                    absl::StrFormat("Input DataPipeline must produce BatchRow, got: %s",
                                    demangle_str_name(stream_meta->stream_type_index().name())));
  auto batch_row_meta = std::dynamic_pointer_cast<BatchRowMeta>(stream_meta);
  DATAFLOW_THROW_IF(batch_row_meta == nullptr,
                    absl::StrFormat("dynamic pointer cast failed, got: %s",
                                    demangle_str_name(stream_meta->stream_type_index().name())));
  output_stream_meta_ = std::make_shared<BatchMeta>(batch_row_meta);
  args_map_.clear();
}

DataBatcher::~DataBatcher() { VLOG(1) << "[DataBatcher] destructor"; }

std::shared_ptr<StreamMeta> DataBatcher::output_stream_meta() const { return output_stream_meta_; }

absl::StatusOr<std::shared_ptr<Stream>> DataBatcher::next() {
  auto tp0 = std::chrono::steady_clock::now();
  size_t current_size = 0;
  auto batch = std::make_shared<Batch>(output_stream_meta_, batch_size_);
  while (current_size < batch_size_) {
    auto status_or_value = input_pipeline_->next();
    DATAFLOW_THROW_IF(!status_or_value.ok(), absl::StrFormat("[DataBatcher] get next error: %s",
                                                             status_or_value.status().message()));

    auto value = status_or_value.value();
    if (value == nullptr) {
      VLOG(3) << "[DataBatcher] end of input.";
      if (drop_last_batch_) {
        return nullptr;
      } else {
        break;
      }
    }

    auto batch_row = std::dynamic_pointer_cast<BatchRow>(value);
    bool drop = invoke_filters(batch_row);
    if (drop) {
      continue;
    }

    batch->add_batch_row(batch_row);
    ++current_size;
  }
  batch->batch_size = current_size;
  auto tp1 = std::chrono::steady_clock::now();
  VLOG(6) << "batcher cost: " << (tp1 - tp0).count() / 1000 << " us";
  return batch;
}

PyObject* DataBatcher::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(BatchMeta),
      absl::StrFormat("Stream is not of type Batch, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));
  auto stream_ptr = std::dynamic_pointer_cast<Batch>(stream->shared_from_this());
  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to Batch failed");

  return DataBatcher::batch_cast_to_dict(stream_ptr).release().ptr();
}

bool DataBatcher::invoke_filters(std::shared_ptr<BatchRow> batch_row) {
  bool drop = false;
  args_map_.clear();
  for (const auto& [fn, tuple] : fn_ars_) {
    auto args = prepare_args(batch_row, tuple);
    auto result =
        pybind11::reinterpret_steal<pybind11::object>(PyObject_Call(fn.ptr(), args.ptr(), nullptr));

    // 检查是否执行成功，如果执行失败，抛出异常
    DATAFLOW_THROW_IF(
        result.ptr() == nullptr,
        absl::StrFormat("invoke failed, err_msg: %s, fn: %s, args: %s",
                        pybind11::detail::error_string(), pybind11::str(fn).cast<std::string>(),
                        pybind11::str(args).cast<std::string>()));
    drop = (result.cast<bool>() == false);
    if (drop) {
      return drop;
    }
  }
  return drop;
}

pybind11::tuple DataBatcher::prepare_args(std::shared_ptr<BatchRow> batch_row, pybind11::tuple t) {
  const auto batch_row_meta = std::dynamic_pointer_cast<BatchRowMeta>(batch_row->stream_meta());
  DATAFLOW_THROW_IF(batch_row_meta == nullptr, "dynamic pointer cast failed.");

  const auto& column_idx_map = batch_row_meta->column_name_to_index;
  const auto& columns = batch_row_meta->columns;
  const auto check_shape = [](const std::vector<int64_t>& ori_shape,
                              std::vector<int64_t>& out_shape, size_t byte_size,
                              const std::string& name) {
    out_shape.resize(ori_shape.size());
    int32_t neg_count = 0;
    int64_t neg_idx = -1;
    int64_t item_byte_size = sizeof(int64_t);
    for (size_t i = 0; i < ori_shape.size(); ++i) {
      if (ori_shape[i] < 0) {
        ++neg_count;
        neg_idx = i;
      } else {
        out_shape[i] = ori_shape[i];
        item_byte_size *= ori_shape[i];
      }
    }
    DATAFLOW_THROW_IF(
        neg_count > 1,
        absl::StrFormat("error, sparse column shape is more than one -1, name: %s", name));
    DATAFLOW_THROW_IF(
        item_byte_size <= 0,
        absl::StrFormat("item_count <= 0, pelease check sparse column shape, name: %s", name));
    if (neg_count == 1) {
      out_shape[neg_idx] = byte_size / item_byte_size;
    }
  };

  pybind11::tuple args(t.size());
  for (size_t i = 0; i < t.size(); ++i) {
    const std::string arg_name = t[i].cast<std::string>();
    auto arg_iter = args_map_.find(arg_name);
    if (arg_iter != args_map_.end()) {
      args[i] = arg_iter->second;
      continue;
    }
    auto iter = column_idx_map.find(arg_name);
    if (iter == column_idx_map.end()) {
      const auto& external_data_map = batch_row->external_data();
      auto external_data_iter = external_data_map.find(arg_name);
      DATAFLOW_THROW_IF(
          external_data_iter == external_data_map.end(),
          absl::StrFormat("args error, not in column and external_data, arg: %s", arg_name));
      args[i] = pybind11::str(external_data_iter->second);
      args_map_[arg_name] = args[i];
      continue;
    }

    const auto& column = columns[iter->second];
    const auto& column_blk = batch_row->get_column_block(iter->second);
    if (column.column_type == ColumnType::kString) {
      size_t byte_size = column_blk->byte_size;
      char* data =
          reinterpret_cast<char*>(byte_size <= 8 ? &column_blk->packed_data : column_blk->ptr);
      args[i] = pybind11::str(std::string(data, column_blk->byte_size));
      args_map_[arg_name] = args[i];
      continue;
    }

    std::vector<ssize_t> shape;
    std::vector<ssize_t> strides;
    void* data = column_blk->byte_size <= 8 ? &column_blk->packed_data : column_blk->ptr;
    if (column.column_type == ColumnType::kSparse) {
      check_shape(column.shape, shape, column_blk->byte_size, column.name);
      strides = compute_strides_fortran<int64_t>(shape);

      args[i] = pybind11::array_t<int64_t>(shape, strides, reinterpret_cast<const int64_t*>(data));
    } else {
      shape = column.shape;
      strides = compute_strides_fortran<float>(shape);

      args[i] = pybind11::array_t<float>(shape, strides, reinterpret_cast<const float*>(data));
    }
    args_map_[arg_name] = args[i];
  }
  return args;
}

pybind11::dict DataBatcher::batch_cast_to_dict(std::shared_ptr<Batch> batch) {
  auto tp0 = std::chrono::steady_clock::now();

  const auto check_shape = [](const std::vector<int64_t>& ori_shape,
                              std::vector<int64_t>& out_shape, size_t byte_size,
                              const std::string& name) {
    out_shape.resize(ori_shape.size());
    int32_t neg_count = 0;
    int64_t neg_idx = -1;
    int64_t item_byte_size = sizeof(int64_t);
    for (size_t i = 0; i < ori_shape.size(); ++i) {
      if (ori_shape[i] < 0) {
        ++neg_count;
        neg_idx = i;
      } else {
        out_shape[i] = ori_shape[i];
        item_byte_size *= ori_shape[i];
      }
    }
    DATAFLOW_THROW_IF(
        neg_count > 1,
        absl::StrFormat("error, sparse column shape is more than one -1, name: %s", name));
    DATAFLOW_THROW_IF(
        item_byte_size <= 0,
        absl::StrFormat("item_count <= 0, pelease check sparse column shape, name: %s", name));
    if (neg_count == 1) {
      out_shape[neg_idx] = byte_size / item_byte_size;
    }
  };

  auto d = pybind11::dict();
  auto batch_meta = std::dynamic_pointer_cast<BatchMeta>(batch->stream_meta());
  const auto& columns = batch_meta->batch_row_meta->columns;

  for (size_t column_idx = 0; column_idx < columns.size(); ++column_idx) {
    const auto& column = columns[column_idx];
    const std::string& name = column.name;

    std::vector<ssize_t> shape = {static_cast<ssize_t>(batch->batch_size)};
    switch (column.column_type) {
      case ColumnType::kDense: {
        shape.insert(shape.end(), column.shape.begin(), column.shape.end());
        auto strides = compute_strides_fortran<float>(shape);
        pybind11::array_t<float> np_arr = pybind11::array_t<float>(shape, strides);
        char* data = reinterpret_cast<char*>(np_arr.mutable_data());
        size_t offset = 0;
        for (size_t i = 0; i < batch->batch_size; ++i) {
          const auto& blk = batch->get_batch_row(i)->get_column_block(column_idx);
          memcpy(data + offset, blk->data(), blk->byte_size);
          offset += blk->byte_size;
        }
        d[pybind11::str(name)] = np_arr;
        break;
      }
      case ColumnType::kSparse: {
        size_t max_byte_size = 0;
        std::vector<int64_t> final_out_shape = column.shape;
        for (size_t i = 0; i < batch->batch_size; ++i) {
          const auto& blk = batch->get_batch_row(i)->get_column_block(column_idx);
          std::vector<int64_t> out_shape;
          if (blk->byte_size > max_byte_size) {
            max_byte_size = blk->byte_size;
            check_shape(column.shape, out_shape, blk->byte_size, column.name);
            final_out_shape = out_shape;
          }
        }
        shape.insert(shape.end(), final_out_shape.begin(), final_out_shape.end());
        auto strides = compute_strides_fortran<int64_t>(shape);
        pybind11::array_t<int64_t> np_arr = pybind11::array_t<int64_t>(shape, strides);
        char* data = reinterpret_cast<char*>(np_arr.mutable_data());
        std::memset(data, 0, np_arr.nbytes());
        size_t offset = 0;
        for (size_t i = 0; i < batch->batch_size; ++i) {
          const auto& blk = batch->get_batch_row(i)->get_column_block(column_idx);
          memcpy(data + offset, blk->data(), blk->byte_size);
          offset += max_byte_size;
        }
        d[pybind11::str(name)] = np_arr;
        break;
      }
      case ColumnType::kString: {
        pybind11::list lst;
        for (size_t i = 0; i < batch->batch_size; ++i) {
          const auto& blk = batch->get_batch_row(i)->get_column_block(column_idx);
          lst.append(std::string(reinterpret_cast<char*>(blk->data()), blk->byte_size));
        }
        d[pybind11::str(name)] = pybind11::array(lst);
        break;
      }
      default:
        break;
    }
  }

  auto tp1 = std::chrono::steady_clock::now();

  VLOG(6) << "cast to batch cost: " << (tp1 -tp0).count() / 1000 << " us";
  return d;
}
}  // namespace data_flow