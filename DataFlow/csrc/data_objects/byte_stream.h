/*
 * @file byte_stream.h
 * @brief Definition of Stream data object for chunked file access.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <memory>
#include <span>

#include "glog/logging.h"

#include "DataFlow/csrc/core/data_object.h"

namespace data_flow {
// Forward declaration
class ByteStream;

// Type alias for Stream metadata
using ByteStreamMeta = DataMeta<ByteStream>;

/**
 * @brief Stream is a data object that provides chunked access to a file stream.
 */
class ByteStream final : public DataObject {
 public:
  ByteStream(std::string&& file_name, size_t buffer_size = 4096)
      : file_name_(std::move(file_name)),
        buffer_size_(buffer_size),
        buffer_(new char[buffer_size]),
        pos_(0),
        end_(0) {
    local_file_ = std::fopen(file_name_.data(), "rb");
    if (!local_file_) {
      LOG(ERROR) << "Failed to open file: " << file_name_;
      throw std::runtime_error(absl::StrFormat("Failed to open file: %s", file_name_));
    }
    refill_buffer();
  }

  ~ByteStream() final {
    if (local_file_) {
      std::fclose(local_file_);
    }
    delete[] buffer_;
    VLOG(1) << "[ByteStream] destructor";
  }

  std::shared_ptr<DataObjectMeta> data_meta() const final {
    static std::shared_ptr<DataObjectMeta> meta = std::make_shared<ByteStreamMeta>();
    return meta;
  }

  void* ptr() final { return this; }

  std::span<const char> peek_chunk() {
    if (pos_ == end_) {
      refill_buffer();
    }
    return std::span<const char>(buffer_ + pos_, end_ - pos_);
  }

  std::span<const char> read_chunk() {
    if (pos_ == end_) {
      refill_buffer();
    }

    std::span<const char> chunk(buffer_ + pos_, end_ - pos_);
    pos_ = end_;

    return chunk;
  }

  bool eof() const { return pos_ >= end_ && std::feof(local_file_); }

 private:
  void refill_buffer() {
    pos_ = 0;
    end_ = std::fread(buffer_, 1, buffer_size_, local_file_);
  }

  FILE* local_file_;
  size_t buffer_size_;
  char* buffer_;
  size_t pos_;
  size_t end_;
  std::string file_name_;
};

}  // namespace data_flow