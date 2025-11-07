/*
 * @file string_stream.h
 * @brief Definition of StringStream data object.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-3
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include "DataFlow/csrc/core/stream.h"

namespace data_flow {
// Forward declaration
class StringStream;

// Type alias for Stream metadata
using StringStreamMeta = StreamMetaBind<StringStream>;

class StringStream final : public Stream {
 public:
  StringStream(const std::string& input_string);

  ~StringStream() final;

  std::shared_ptr<StreamMeta> stream_meta() const final;

  void* ptr() final;

  std::string value() const;

 private:
  const std::string value_;
};

}  // namespace data_flow