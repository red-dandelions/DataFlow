// 11.10
#pragma once

#include <cstddef>
#include <memory>

#include "pybind11/pybind11.h"

#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/streams/batch.h"
#include "DataFlow/csrc/streams/batch_row.h"

namespace data_flow {

class DataBatcher final : public DataPipeline {
 public:
  explicit DataBatcher(std::shared_ptr<DataPipeline> pipeline, size_t batch_size,
                       const std::vector<std::pair<pybind11::function, pybind11::tuple>>& fn_ars,
                       bool drop_last_batch = true);
  ~DataBatcher() final;

  std::shared_ptr<StreamMeta> output_stream_meta() const final;

  absl::StatusOr<std::shared_ptr<Stream>> next() final;

  PyObject* as_python_object(std::shared_ptr<Stream> stream) const final;

  static pybind11::dict batch_cast_to_dict(std::shared_ptr<Batch> batch);

 private:
  bool invoke_filters(const std::shared_ptr<BatchRow> batch_row);
  pybind11::tuple prepare_args(std::shared_ptr<BatchRow> batch_row, pybind11::tuple t);

  std::shared_ptr<DataPipeline> input_pipeline_;
  size_t batch_size_;
  std::vector<std::pair<pybind11::function, pybind11::tuple>> fn_ars_;
  bool drop_last_batch_;
  std::shared_ptr<BatchMeta> output_stream_meta_;
  std::unordered_map<std::string, pybind11::object> args_map_;
};
}  // namespace data_flow