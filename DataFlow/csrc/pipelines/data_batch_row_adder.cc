// 2025.11.9

#include "data_batch_row_adder.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sys/types.h>
#include <utility>

#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "pybind11/numpy.h"

#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/streams/batch_row.h"

namespace data_flow {
namespace {
template <typename T>
std::vector<ssize_t> compute_strides_fortran(const std::vector<ssize_t>& shape) {
  size_t dims = shape.size();
  std::vector<ssize_t> strides(dims);
  if (dims == 0) return strides;

  ssize_t element_size = static_cast<ssize_t>(sizeof(T));
  strides[0] = element_size;  // 第一个维度步长 = 元素大小
  for (size_t i = 1; i < dims; ++i) {
    strides[i] = strides[i - 1] * shape[i - 1];
  }
  return strides;
}
}  // namespace
DataBatchRowAdder::DataBatchRowAdder(std::shared_ptr<DataPipeline> pipeline, pybind11::function fn,
                                     std::vector<std::string>&& args_name,
                                     std::vector<Column>&& add_columns)
    : fn_(fn), args_name_(std::move(args_name)), add_columns_(std::move(add_columns)) {
  auto stream_meta = pipeline->output_stream_meta();
  DATAFLOW_THROW_IF(stream_meta->stream_type_index() != typeid(BatchRow),
                    absl::StrFormat("Input DataPipeline must produce BatchRow, got: %s",
                                    demangle_str_name(stream_meta->stream_type_index().name())));
  input_pipeline_ = pipeline;

  auto batch_row_stream_meta = std::dynamic_pointer_cast<BatchRowMeta>(stream_meta);
  std::vector<Column> columns = batch_row_stream_meta->columns;
  output_stream_meta_ = std::make_shared<BatchRowMeta>(std::move(columns));
  for (const auto& add_column : add_columns_) {
    size_t idx = output_stream_meta_->columns.size();
    output_stream_meta_->columns.push_back(add_column);
  }

  output_stream_meta_->original_column_size = batch_row_stream_meta->original_column_size;
  const auto& new_columns = output_stream_meta_->columns;
  output_stream_meta_->column_name_to_index.clear();
  for (size_t i = 0; i < new_columns.size(); ++i) {
    output_stream_meta_->column_name_to_index[new_columns[i].name] = i;
  }
}

DataBatchRowAdder::~DataBatchRowAdder() { VLOG(1) << "[DataBatchRowAdder] destructor"; }

std::shared_ptr<StreamMeta> DataBatchRowAdder::output_stream_meta() const {
  return output_stream_meta_;
}

absl::StatusOr<std::shared_ptr<Stream>> DataBatchRowAdder::next() {
  auto status_or_value = input_pipeline_->next();
  DATAFLOW_THROW_IF(!status_or_value.ok(),
                    absl::StrFormat("[DataBatchRowAdder] failed to get next value, error msg: %s",
                                    status_or_value.status().message()));

  auto value = status_or_value.value();
  if (value == nullptr) {
    VLOG(3) << "[DataBatchRowAdder] end of input";
    return nullptr;
  }
  auto batch_row = std::dynamic_pointer_cast<BatchRow>(value);
  batch_row->set_batch_row_meta(output_stream_meta_);
  DATAFLOW_THROW_IF(batch_row == nullptr, absl::StrFormat("dynamic pointer cast error"));

  auto args = prepare_args(batch_row);

  auto result =
      pybind11::reinterpret_steal<pybind11::object>(PyObject_Call(fn_.ptr(), args.ptr(), nullptr));

  // 检查是否执行成功，如果执行失败，抛出异常
  DATAFLOW_THROW_IF(
      result.ptr() == nullptr,
      absl::StrFormat("invoke failed, err_msg: %s, fn: %s, args: %s",
                      pybind11::detail::error_string(), pybind11::str(fn_).cast<std::string>(),
                      pybind11::str(args).cast<std::string>()));

  // cast
  auto tuple = result.cast<pybind11::tuple>();
  add_to_batch_row(tuple, batch_row);

  return value;
}

PyObject* DataBatchRowAdder::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(BatchRow),
      absl::StrFormat("Stream is not of type BatchRow, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));
  auto stream_ptr = std::dynamic_pointer_cast<BatchRow>(stream->shared_from_this());
  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to BatchRow failed");

  return pybind11::cast(stream_ptr).release().ptr();
}

pybind11::tuple DataBatchRowAdder::prepare_args(std::shared_ptr<BatchRow> batch_row) {
  const auto& column_idx_map = output_stream_meta_->column_name_to_index;
  const auto& columns = output_stream_meta_->columns;
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

  pybind11::tuple args(args_name_.size());
  for (size_t i = 0; i < args_name_.size(); ++i) {
    auto iter = column_idx_map.find(args_name_[i]);
    if (iter == column_idx_map.end()) {
      const auto& external_data_map = batch_row->external_data();
      auto external_data_iter = external_data_map.find(args_name_[i]);
      DATAFLOW_THROW_IF(
          external_data_iter == external_data_map.end(),
          absl::StrFormat("args error, not in column and external_data, arg: %s", args_name_[i]));
      args[i] = pybind11::str(external_data_iter->second);
      continue;
    }

    const auto& column = columns[iter->second];
    const auto& column_blk = batch_row->get_column_block(iter->second);
    if (column.column_type == ColumnType::kString) {
      size_t byte_size = column_blk->byte_size;
      char* data =
          reinterpret_cast<char*>(byte_size <= 8 ? &column_blk->packed_data : column_blk->ptr);
      args[i] = pybind11::str(std::string(data, column_blk->byte_size));
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
  }
  return args;
}

void DataBatchRowAdder::add_to_batch_row(const pybind11::tuple& t,
                                         std::shared_ptr<BatchRow> batch_row) {
  DATAFLOW_THROW_IF(
      t.size() != add_columns_.size(),
      absl::StrFormat(
          "fn result size must equal add columns size, result size: %d, columns size: %d", t.size(),
          add_columns_.size()));
  for (size_t i = 0; i < t.size(); ++i) {
    auto iter = output_stream_meta_->column_name_to_index.find(add_columns_[i].name);
    DATAFLOW_THROW_IF(
        iter == output_stream_meta_->column_name_to_index.end(),
        absl::StrFormat("column: %s not in output_stream_meta.", add_columns_[i].name));

    switch (add_columns_[i].column_type) {
      case data_flow::ColumnType::kDense: {
        pybind11::array_t<float> dense_array = pybind11::array_t<float>::ensure(t[i]);
        DATAFLOW_THROW_IF(
            dense_array.ptr() == nullptr,
            absl::StrFormat("result[%d] must match dtype for column: %s", i, add_columns_[i].name));
        size_t nbytes = dense_array.nbytes();
        DATAFLOW_THROW_IF(
            nbytes != add_columns_[i].item_count * sizeof(float),
            absl::StrFormat("byte size mismatch, nbytes: %d, column byte sise: %d, column: %s",
                            nbytes, add_columns_[i].item_count * sizeof(float),
                            add_columns_[i].name));

        void* p = batch_row->alloc_column_block_data(iter->second, nbytes);
        memcpy(p, dense_array.data(), nbytes);
        break;
      }
      case data_flow::ColumnType::kSparse: {
        auto sparse_array = pybind11::array_t<int64_t>::ensure(t[i]);
        DATAFLOW_THROW_IF(
            sparse_array.ptr() == nullptr,
            absl::StrFormat("result[%d] must match dtype for column: %s", i, add_columns_[i].name));
        size_t nbytes = sparse_array.nbytes();
        if (add_columns_[i].item_count > 0) {
          DATAFLOW_THROW_IF(
              nbytes != add_columns_[i].item_count * sizeof(int64_t),
              absl::StrFormat("byte size mismatch, nbytes: %d, column byte sise: %d, column: %s",
                              nbytes, add_columns_[i].item_count * sizeof(int64_t),
                              add_columns_[i].name));
        }
        void* p = batch_row->alloc_column_block_data(iter->second, nbytes);
        memcpy(p, sparse_array.data(), nbytes);
        break;
      }
      case data_flow::ColumnType::kString: {
        std::string data = t[i].cast<std::string>();
        void* p = batch_row->alloc_column_block_data(iter->second, data.size());
        memcpy(p, data.data(), data.size());
        break;
      }
      default:
        break;
    }
  }
}

}  // namespace data_flow