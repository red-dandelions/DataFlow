

#include "byte_stream.h"
#include "glog/logging.h"

namespace data_flow {

ByteStream::ByteStream(std::string&& file_name, size_t buffer_size)
    : file_name_(std::move(file_name)),
      buffer_size_(buffer_size),
      buffer_(new char[buffer_size]),
      pos_(0),
      end_(0) {
  local_file_ = std::fopen(file_name_.data(), "rb");
  DATAFLOW_THROW_IF(local_file_ == nullptr, absl::StrFormat("Failed to open file: %s", file_name_));
  refill_buffer();
}

ByteStream::~ByteStream() {
  LOG(INFO) << "[ByteStream] destructor";
  if (local_file_) {
    std::fclose(local_file_);
  }
  if (buffer_) {
  delete[] buffer_;
}
}

std::shared_ptr<StreamMeta> ByteStream::stream_meta() const {
  return std::make_shared<ByteStreamMeta>();
}

void* ByteStream::ptr() { return this; }

std::span<const char> ByteStream::peek_chunk() {
  if (pos_ == end_) {
    refill_buffer();
  }
  return std::span<const char>(buffer_ + pos_, end_ - pos_);
}

std::span<const char> ByteStream::read_chunk() {
  if (pos_ == end_) {
    refill_buffer();
  }

  std::span<const char> chunk(buffer_ + pos_, end_ - pos_);
  pos_ = end_;

  return chunk;
}

bool ByteStream::eof() const { return pos_ >= end_ && std::feof(local_file_); }

void ByteStream::refill_buffer() {
  pos_ = 0;
  end_ = std::fread(buffer_, 1, buffer_size_, local_file_);
}

}  // namespace data_flow