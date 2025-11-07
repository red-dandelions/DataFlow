/*
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-3
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include "string_stream.h"

#include "glog/logging.h"

namespace data_flow {

StringStream::StringStream(const std::string& input_string) : value_(input_string) {}

StringStream::~StringStream() { VLOG(6) << "[StringStream] destructor"; }

std::shared_ptr<StreamMeta> StringStream::stream_meta() const {
  return std::make_shared<StringStreamMeta>();
}

void* StringStream::ptr() { return this; }

std::string StringStream::value() const { return value_; }

}  // namespace data_flow