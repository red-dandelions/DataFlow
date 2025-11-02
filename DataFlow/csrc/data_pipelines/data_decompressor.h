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

#include "DataFlow/csrc/common/functions.h"
#include "DataFlow/csrc/core/data_object.h"
#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/data_objects/inflate_stream.h"

namespace data_flow {

class DataDecompressor final : public DataPipeline {
 public:
  DataDecompressor(const std::shared_ptr<DataPipeline>& data_pipeline) {
    CHECK(data_pipeline->output_data_meta()->data_type() == typeid(ByteStream))
        << "Input DataPipeline must produce ByteStream, got: "
        << data_pipeline->output_data_meta()->data_type().name();
    input_ = data_pipeline;
  }

  std::shared_ptr<DataObjectMeta> output_data_meta() const final {
    static std::shared_ptr<DataObjectMeta> meta = std::make_shared<InflateStreamMeta>();
    return meta;
  }

  absl::StatusOr<std::shared_ptr<DataObject>> next() final {
    auto status_or_obj = input_->next();
    if (!status_or_obj.ok()) {
      return status_or_obj.status();
    }

    auto obj = status_or_obj.value();
    if (obj == nullptr) {
      VLOG(3) << "[DataDecompressor] end of input pipeline";
      return nullptr;
    }

    auto decompress_stream =
        std::make_shared<InflateStream>(std::dynamic_pointer_cast<ByteStream>(obj));
    return decompress_stream;
  }

  PyObject* as_python_object(std::shared_ptr<DataObject> data_object) const final {
    CHECK(data_object->data_meta()->data_type() == typeid(InflateStream))
        << "DataObject is not of type InflateStream, got: "
        << data_object->data_meta()->data_type().name();

    auto stream_ptr = std::dynamic_pointer_cast<InflateStream>(data_object->shared_from_this());
    if (!stream_ptr) {
      std::runtime_error("DataObject is not of type InflateStream");
    }

    return pybind11::cast(stream_ptr).release().ptr();
  }

 private:
  std::shared_ptr<DataPipeline> input_;
};
}  // namespace data_flow
