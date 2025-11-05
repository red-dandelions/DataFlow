#include "data_decompressor.h"

#include "glog/logging.h"
#include "pybind11/pybind11.h"

namespace data_flow {

DataDecompressor::DataDecompressor(const std::shared_ptr<DataPipeline>& data_pipeline) {
  DATAFLOW_THROW_IF(
      data_pipeline->output_stream_meta()->stream_type_index() != typeid(ByteStream),
      absl::StrFormat(
          "Input DataPipeline must produce ByteStream, got: %s",
          demangle_str_name(data_pipeline->output_stream_meta()->stream_type_index().name())));

  input_ = data_pipeline;
}

DataDecompressor::~DataDecompressor() { VLOG(1) << "[DataDecompressor] destructor"; }

std::shared_ptr<StreamMeta> DataDecompressor::output_stream_meta() const {
  static std::shared_ptr<StreamMeta> meta = std::make_shared<InflateStreamMeta>();
  return meta;
}

absl::StatusOr<std::shared_ptr<Stream>> DataDecompressor::next() {
  auto status_or_stream = input_->next();
  if (!status_or_stream.ok()) {
    return status_or_stream.status();
  }

  auto stream = status_or_stream.value();
  if (stream == nullptr) {
    VLOG(3) << "[DataDecompressor] end of input";
    return nullptr;
  }

  auto decompress_stream =
      std::make_shared<InflateStream>(std::dynamic_pointer_cast<ByteStream>(stream));
  return decompress_stream;
}

PyObject* DataDecompressor::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(InflateStream),
      absl::StrFormat("Stream is not of type InflateStream, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));

  auto stream_ptr = std::dynamic_pointer_cast<InflateStream>(stream->shared_from_this());
  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to InflateStream failed");

  return pybind11::cast(stream_ptr).release().ptr();
}

}  // namespace data_flow
