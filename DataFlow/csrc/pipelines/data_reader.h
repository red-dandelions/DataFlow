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

#include <cstdint>
#include <stdexcept>

#include "glog/logging.h"
#include "pybind11/pybind11.h"

#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/streams/byte_stream.h"

namespace data_flow {

static constexpr size_t kDefaultBufferSize = 4096;

class DataReader final : public DataPipeline {
 public:
  enum class Source : int8_t { kFileList, kFilePipeline, kPulsarMessagePipeline };

 public:
  DataReader(const std::vector<std::string>&& files)
      : file_source_(Source::kFileList), file_paths_(files.begin(), files.end()) {
    file_type_ = !files.empty() && StringFunctors::starts_with(files[0], "hdfs://")
                     ? FileType::kHDFSFile
                     : FileType::kLocalFile;
  }

  // TODO: DataReader(std::shared_ptr<DataObject> string_stream);
  // TODO: DataReader(std::shared_ptr<DataObject> pulsar_stream);

  ~DataReader() final { VLOG(1) << "[DataReader] destructor"; }

  std::shared_ptr<StreamMeta> output_stream_meta() const final {
    static std::shared_ptr<StreamMeta> meta = std::make_shared<ByteStreamMeta>();
    return meta;
  }

  absl::StatusOr<std::shared_ptr<Stream>> next() final {
    switch (file_source_) {
      case Source::kFileList:
        return stream_from_file_list(file_paths_.front());
      case Source::kFilePipeline:
        // TODO: implement file pipeline support
      case Source::kPulsarMessagePipeline:
        // TODO: impplement pulsar message pipeline support
      default:
        return absl::InvalidArgumentError("Unknown file source");
    }
  }

  PyObject* as_python_object(std::shared_ptr<Stream> stream) const final {
    DATAFLOW_THROW_IF(
        stream->stream_meta()->stream_type_index() != typeid(ByteStream),
        absl::StrFormat("Stream is not of type ByteStream, got: %s",
                        demangle_type_name(stream->stream_meta()->stream_type_index())));

    auto stream_ptr = std::dynamic_pointer_cast<ByteStream>(stream->shared_from_this());

    DATAFLOW_THROW_IF(strea_ptr == nullptr, "dynamic cast to ByteStream failed");

    return pybind11::cast(stream_ptr).release().ptr();
  }

 private:
  absl::StatusOr<std::shared_ptr<Stream>> stream_from_file_list(const std::string& file_path) {
    if (file_paths_.empty()) {
      VLOG(3) << "[DataReader] end of input";
      return nullptr;  // End of iteration
    }

    switch (file_type_) {
      case FileType::kLocalFile: {
        std::string current_file = file_paths_.front();
        file_paths_.pop_front();

        DATAFLOW_THROW_IF(!StringFunctors::starts_with(current_file, "hdfs://"),
                          absl::StrFormt("Local file expected, but got: %s", current_file));

        auto stream = std::make_shared<ByteStream>(std::move(current_file), kDefaultBufferSize);
        return stream;
      } break;
      case FileType::kHDFSFile:
        return absl::UnimplementedError("HDFS file support not implemented yet");
      default:
        return absl::InvalidArgumentError("Unknown file type");
    }
  }

  /** TODO:
  absl::StatusOr<std::shared_ptr<DataObject>> stream_from_string_stream() {
        auto status = string_stream_->next();
        if (!status.ok()) {
            return status.status();
        }
        auto str_obj = status.value();
        if (str_obj == nullptr) {
            return nullptr;  // End of iteration
        }
        auto f = str_obj->data;
        auto stream = std::make_shared<Stream>(std::move(f), kDefaultBufferSize);
        return stream;
    }*/
  enum class FileType : int8_t { kLocalFile, kHDFSFile };

  Source file_source_;

  std::list<std::string> file_paths_;
  FileType file_type_;

  // TODO: std::shared_ptr<StringStream> string_stream_;
};
}  // namespace data_flow
