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

#include "DataFlow/csrc/core/data_pipeline.h"

namespace data_flow {

class DataDecompressor final : public DataPipeline {
 public:
  DataDecompressor(const std::shared_ptr<DataPipeline>& data_pipeline);

  ~DataDecompressor() final;
  std::shared_ptr<StreamMeta> output_stream_meta() const final;

  absl::StatusOr<std::shared_ptr<Stream>> next() final;

  PyObject* as_python_object(std::shared_ptr<Stream> stream) const final;

 private:
  std::shared_ptr<DataPipeline> input_;
};
}  // namespace data_flow
