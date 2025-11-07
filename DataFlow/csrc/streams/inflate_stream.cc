/*

 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-2
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include "inflate_stream.h"

#include "glog/logging.h"
#include "zlib.h"

namespace data_flow {

InflateStream::InflateStream(std::shared_ptr<Stream> stream) {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(ByteStream),
      absl::StrFormat("Input Stream must be of type ByteStream, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));

  compressed_stream_ = std::dynamic_pointer_cast<ByteStream>(stream->shared_from_this());
  DATAFLOW_THROW_IF(compressed_stream_ == nullptr, "Failed to cast Stream to ByteStream");

  inflate_stream_init();
}

InflateStream::~InflateStream() { inflateEnd(&z_stream_); }

std::shared_ptr<StreamMeta> InflateStream::stream_meta() const {
  return std::make_shared<InflateStreamMeta>();
}

void* InflateStream::ptr() { return this; }

std::span<const char> InflateStream::read_chunk(size_t size) {
  if (end_of_stream_) {
    return std::span<const char>{};
  }

  // 如果请求的大小为0，使用默认大小
  if (size == 0) {
    size = kDefaultChunkSize;
  }

  // 确保输出buffer足够大
  if (output_chunk_size_ < size) {
    output_chunk_size_ = size;
    output_chunk_ = std::make_unique<char[]>(output_chunk_size_);
  }

  DATAFLOW_THROW_IF(size >= INT_MAX, "Requested chunk size size exceeds INT_MAX");

  // set zlib output buffer
  z_stream_.avail_out = size;
  z_stream_.next_out = reinterpret_cast<Bytef*>(output_chunk_.get());

  // 持续解压直到获得足够的数据或到达流末尾
  while ((z_stream_.next_out - reinterpret_cast<Bytef*>(output_chunk_.get())) < size) {
    if (z_stream_.avail_in == 0 && !end_of_stream_) {
      // 获取新的压缩数据
      auto input_chunk = compressed_stream_->read_chunk();
      if (!input_chunk.empty()) {
        // 设置输入数据
        z_stream_.avail_in = input_chunk.size();
        z_stream_.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input_chunk.data()));
      } else {
        end_of_stream_ = true;
        break;
      }
    }
    // 执行解压缩
    int ret = inflate(&z_stream_, Z_NO_FLUSH);

    VLOG(5) << "Decompressing: output_size="
            << (z_stream_.next_out - reinterpret_cast<Bytef*>(output_chunk_.get()))
            << ", want_size=" << size << ", avail_in=" << z_stream_.avail_in;

    DATAFLOW_THROW_IF(ret != Z_OK && ret != Z_STREAM_END,
                      absl::StrFormat("Inflation failed: %d, msg: %s", ret, z_stream_.msg));

    if (ret == Z_STREAM_END) {
      end_of_stream_ = true;
      break;
    }
  }

  // 计算实际解压的数据大小
  size_t decompressed_size = z_stream_.next_out - reinterpret_cast<Bytef*>(output_chunk_.get());

  // 返回解压后数据的视图，实现零拷贝
  return std::span<const char>(output_chunk_.get(), decompressed_size);
}

void InflateStream::inflate_stream_init() {
  z_stream_ = {};

  int ret = inflateInit2(&z_stream_, kFormatAutomatic | kMaxWindowSize);

  DATAFLOW_THROW_IF(
      ret != Z_OK,
      absl::StrFormat("Failed to initialize zlib inflate stream: %d, msg: %s", ret, z_stream_.msg));
}

}  // namespace data_flow
