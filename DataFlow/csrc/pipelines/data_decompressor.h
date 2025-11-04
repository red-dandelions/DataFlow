/**
 * @file data_decompressor.h
 * @brief Definition of Stream data object for chunked file access.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-2
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include <cstdint>
#include <stdexcept>

#include "glog/logging.h"
#include "pybind11/pybind11.h"

#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/streams/inflate_stream.h"

namespace data_flow {

class DataDecompressor final : public DataPipeline {
 public:
  DataDecompressor(const std::shared_ptr<DataPipeline>& data_pipeline) {
    CHECK(data_pipeline->output_data_meta()->data_type() == typeid(ByteStream))
        << "Input DataPipeline must produce ByteStream, got: "
        << data_pipeline->output_data_meta()->data_type().name();

    DATAFLOW_THROW_IF(
        data_pipeline->output_stream_meta()->stream_type_index() != typeid(ByteStream),
        absl::StrFormat(
            "Input DataPipeline must produce ByteStream, got: %s",
            demangle_type_name(data_pipeline->output_stream_meta()->stream_type_index())));

    input_ = data_pipeline;
  }

  ~DataDecompressor() final { VLOG(1) << "[DataDecompressor] destructor"; }

  std::shared_ptr<StreamMeta> output_stream_meta() const final {
    static std::shared_ptr<StreamMeta> meta = std::make_shared<InflateStreamMeta>();
    return meta;
  }

  absl::StatusOr<std::shared_ptr<Stream>> next() final {
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

  PyObject* as_python_object(std::shared_ptr<Stream> stream) const final {
    DATAFLOW_THROW_IF(
        stream->stream_meta()->stream_type_index() != typeid(InflateStream),
        absl::StrFormat("Stream is not of type InflateStream, got: %s",
                        demangle_type_name(stream->stream_meta()->stream_type_index())));

    auto stream_ptr = std::dynamic_pointer_cast<InflateStream>(stream->shared_from_this());
    DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to InflateStream failed");

    return pybind11::cast(stream_ptr).release().ptr();
  }

 private:
  std::shared_ptr<DataPipeline> input_;
};
}  // namespace data_flow
