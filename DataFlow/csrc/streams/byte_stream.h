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

#include <span>

#include "DataFlow/csrc/core/stream.h"

namespace data_flow {
// Forward declaration
class ByteStream;

// Type alias for Stream metadata
using ByteStreamMeta = StreamMetaBind<ByteStream>;

/**
 * @brief ByteStream is a data object that provides chunked access to a local file or hdfs file.
 */
class ByteStream final : public Stream {
 public:
  ByteStream(std::string&& file_name, size_t buffer_size = 4096);

  ~ByteStream() final;

  std::shared_ptr<StreamMeta> stream_meta() const final;

  void* ptr() final;

  std::span<const char> peek_chunk();

  std::span<const char> read_chunk();

  bool eof() const;

 private:
  void refill_buffer();

  FILE* local_file_;
  size_t buffer_size_;
  char* buffer_ = nullptr;
  size_t pos_;
  size_t end_;
  std::string file_name_;
};

}  // namespace data_flow