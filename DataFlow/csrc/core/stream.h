/**
 * @file stream.h
 * @brief Definition of Stream and StreamMeta classes.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include <memory>
#include <typeindex>

#include "absl/strings/str_format.h"

#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/common/functions.h"
#include "DataFlow/csrc/common/macros.h"

namespace data_flow {

/**
 * @brief StreamMeta is an abstract base class representing metadata for a Stream.
 */
struct StreamMeta : std::enable_shared_from_this<StreamMeta> {
  virtual ~StreamMeta() = default;

  virtual std::type_index stream_type_index() const = 0;
};

/**
 * @brief Template class for binding StreamMeta to a specific Stream type.
 */
template <typename Meta>
struct StreamMetaBind : public StreamMeta {
  std::type_index stream_type_index() const final { return typeid(Meta); }
  
  virtual ~StreamMetaBind() = default;
};

/**
 * @brief Stream is an abstract base class representing a data stream.
 */
struct Stream : std::enable_shared_from_this<Stream> {
  virtual ~Stream() = default;

  virtual std::shared_ptr<StreamMeta> stream_meta() const = 0;

  virtual void* ptr() = 0;

  /**
   * @brief Cast the Stream to a specific type T.
   * @tparam T The target type to cast to.
   * @return Reference to the casted type T.
   * @throws std::runtime_error if the type does not match.
   */
  template <typename T>
  T& as() {
    DATAFLOW_THROW_IF(
        typeid(T) != stream_meta()->stream_type_index(),
        absl::StrFormat("Stream type mismatch: expected %s, got %s", demangle_type_name<T>(),
                        demangle_str_name(stream_meta()->stream_type_index().name())));

    return *reinterpret_cast<T*>(ptr());
  }
};
}  // namespace data_flow
