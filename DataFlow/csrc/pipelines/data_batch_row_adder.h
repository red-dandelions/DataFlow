// 2025.11.9

#include <memory>

#include "pybind11/pybind11.h"

#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/streams/batch_row.h"

namespace data_flow {

class DataBatchRowAdder final : public DataPipeline {
 public:
  DataBatchRowAdder(std::shared_ptr<DataPipeline> pipeline, pybind11::function fn,
                    std::vector<std::string>&& args_name, std::vector<Column>&& add_columns);

  ~DataBatchRowAdder() final;

  std::shared_ptr<StreamMeta> output_stream_meta() const final;

  absl::StatusOr<std::shared_ptr<Stream>> next() final;

  virtual PyObject* as_python_object(std::shared_ptr<Stream> stream) const final;

 private:
  pybind11::tuple prepare_args(std::shared_ptr<BatchRow> batch_row);
  void add_to_batch_row(const pybind11::tuple& t, std::shared_ptr<BatchRow> batch_row);
  // void invoke_fn();
  // 用于处理数据的 python 函数
  pybind11::function fn_;
  const std::vector<std::string> args_name_;

  std::vector<Column> add_columns_;
  std::shared_ptr<DataPipeline> input_pipeline_;
  std::shared_ptr<BatchRowMeta> output_stream_meta_;
};

}  // namespace data_flow