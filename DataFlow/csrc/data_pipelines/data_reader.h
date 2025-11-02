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

#include "DataFlow/csrc/common/functions.h"
#include "DataFlow/csrc/core/data_object.h"
#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/data_objects/byte_stream.h"

namespace data_flow {

static constexpr size_t kDefaultBufferSize = 4096;

class DataReader final : public DataPipeline {
 public:
  enum class FileSource : int8_t { kFileList, kStringStream };

 public:
  DataReader(const std::vector<std::string>&& files)
      : file_source_(FileSource::kFileList), file_paths_(files.begin(), files.end()) {
    file_type_ = !files.empty() && Func::starts_with(files[0], "hdfs://") ? FileType::kHDFSFile
                                                                          : FileType::kLocalFile;
  }

  // TODO: DataReader(std::shared_ptr<DataObject> string_stream);

  ~DataReader() final = default;

  std::shared_ptr<DataObjectMeta> output_data_meta() const final {
    static std::shared_ptr<DataObjectMeta> meta = std::make_shared<ByteStreamMeta>();
    return meta;
  }

  absl::StatusOr<std::shared_ptr<DataObject>> next() final {
    switch (file_source_) {
      case FileSource::kFileList:
        return stream_from_file_list(file_paths_.front());
      // case FileSource::kStringStream:
      //     return stream_from_string_stream(string_stream_);
      default:
        return absl::InvalidArgumentError("Unknown file source");
    }
  }

  PyObject* as_python_object(std::shared_ptr<DataObject> data_object) const final {
    CHECK(data_object->data_meta()->data_type() == typeid(ByteStream))
        << "DataObject is not of type ByteStream, got: "
        << data_object->data_meta()->data_type().name();

    auto stream_ptr = std::dynamic_pointer_cast<ByteStream>(data_object->shared_from_this());
    if (!stream_ptr) {
      std::runtime_error("DataObject is not of type ByteStream");
    }

    return pybind11::cast(stream_ptr).release().ptr();
  }

 private:
  absl::StatusOr<std::shared_ptr<DataObject>> stream_from_file_list(const std::string& file_path) {
    if (file_paths_.empty()) {
      VLOG(3) << "[DataReader] end of input";
      return nullptr;  // End of iteration
    }

    switch (file_type_) {
      case FileType::kLocalFile: {
        std::string current_file = file_paths_.front();
        file_paths_.pop_front();

        // TODO: CHECK_F(Func::starts_with(current_file, "hdfs://"));

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

  FileSource file_source_;

  std::list<std::string> file_paths_;
  FileType file_type_;

  // TODO: std::shared_ptr<StringStream> string_stream_;
};
}  // namespace data_flow
