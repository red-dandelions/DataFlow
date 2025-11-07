/**

 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include "data_reader.h"

#include "glog/logging.h"
#include "pybind11/pybind11.h"

#include "DataFlow/csrc/streams/byte_stream.h"

namespace data_flow {
namespace {
constexpr size_t kDefaultBufferSize = 4096;
}

DataReader::DataReader(const std::vector<std::string>&& files)
    : file_source_(Source::kFileList), file_paths_(files.begin(), files.end()) {
  file_type_ = !files.empty() && StringFunctors::starts_with(files[0], "hdfs://")
                   ? FileType::kHDFSFile
                   : FileType::kLocalFile;
}

DataReader::~DataReader() { VLOG(1) << "[DataReader] destructor"; }

std::shared_ptr<StreamMeta> DataReader::output_stream_meta() const {
  return std::make_shared<ByteStreamMeta>();
}

absl::StatusOr<std::shared_ptr<Stream>> DataReader::next() {
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

PyObject* DataReader::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(ByteStream),
      absl::StrFormat("Stream is not of type ByteStream, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));

  auto stream_ptr = std::dynamic_pointer_cast<ByteStream>(stream->shared_from_this());

  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to ByteStream failed");

  return pybind11::cast(stream_ptr).release().ptr();
}

absl::StatusOr<std::shared_ptr<Stream>> DataReader::stream_from_file_list(
    const std::string& file_path) {
  if (file_paths_.empty()) {
    VLOG(3) << "[DataReader] end of input";
    return nullptr;  // End of iteration
  }

  switch (file_type_) {
    case FileType::kLocalFile: {
      std::string current_file = file_paths_.front();
      file_paths_.pop_front();

      DATAFLOW_THROW_IF(StringFunctors::starts_with(current_file, "hdfs://"),
                        absl::StrFormat("Local file expected, but got: %s", current_file));

      auto stream = std::make_shared<ByteStream>(std::move(current_file), kDefaultBufferSize);
      return stream;
    } break;
    case FileType::kHDFSFile:
      return absl::UnimplementedError("HDFS file support not implemented yet");
    default:
      return absl::InvalidArgumentError("Unknown file type");
  }
}

}  // namespace data_flow
