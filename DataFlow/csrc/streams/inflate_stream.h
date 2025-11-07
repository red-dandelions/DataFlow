/*
 * @file inflate_stream.h
 * @brief Definition of Stream data object for chunked file access.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-2
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include <climits>
#include <cstdio>
#include <cstring>
#include <memory>
#include <span>

#include "glog/logging.h"
#include "zlib.h"

#include "byte_stream.h"

namespace data_flow {
// Forward declaration
class InflateStream;

// Type alias for Stream metadata
using InflateStreamMeta = StreamMetaBind<InflateStream>;

/**
 * @brief InflateStream is a data object that provides on-the-fly decompression of a compressed
 * ByteStream.
 */
class InflateStream final : public Stream {
 public:
  InflateStream(std::shared_ptr<Stream> stream);

  ~InflateStream() final;

  std::shared_ptr<StreamMeta> stream_meta() const final;

  void* ptr() final;

  /**
   * @brief Read and decompress a chunk of data from the ByteStream.
   * @param size The size of the chunk to read. If size is 0, a default size will be used.
   * @return A span representing the decompressed data chunk.
   */
  std::span<const char> read_chunk(size_t size);

 private:
  // 自动判断输入的格式
  static constexpr int32_t kFormatAutomatic = 32;
  static constexpr int32_t kMaxWindowSize = 15;
  void inflate_stream_init();

 private:
  // in decompression
  z_stream z_stream_;
  std::shared_ptr<ByteStream> compressed_stream_;

  // output buffer
  size_t output_chunk_size_ = 0;
  std::unique_ptr<char[]> output_chunk_;
  bool end_of_stream_ = false;

  size_t available_data_ = 0;

  // 用于跟踪当前chunk中未处理的数据
  static constexpr size_t kDefaultChunkSize = 20 * 1024 * 1024;  // 20 MB
};

}  // namespace data_flow
