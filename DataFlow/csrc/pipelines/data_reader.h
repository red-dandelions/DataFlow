/**
 * @file data_reader.h
 * @brief Definition of Stream data object for chunked file access.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include "DataFlow/csrc/core/data_pipeline.h"

namespace data_flow {
class DataReader final : public DataPipeline {
 public:
  enum class Source : int8_t { kFileList, kFilePipeline, kPulsarMessagePipeline };

 public:
  DataReader(const std::vector<std::string>&& files);

  ~DataReader() final;

  std::shared_ptr<StreamMeta> output_stream_meta() const final;

  absl::StatusOr<std::shared_ptr<Stream>> next() final;

  PyObject* as_python_object(std::shared_ptr<Stream> stream) const final;

 private:
  absl::StatusOr<std::shared_ptr<Stream>> stream_from_file_list(const std::string& file_path);

  enum class FileType : int8_t { kLocalFile, kHDFSFile };

  Source file_source_;

  std::list<std::string> file_paths_;
  FileType file_type_;
};
}  // namespace data_flow
