/**
 * @file data_object.h
 * @brief Definition of DataObject and DataObjectMeta classes.
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

namespace data_flow {

/**
 * @brief DataObjectMeta is an abstract base class representing metadata for a data object.
 */
struct DataObjectMeta : std::enable_shared_from_this<DataObjectMeta> {
  virtual ~DataObjectMeta() = default;

  virtual std::type_index data_type() const = 0;
};

/**
 * @brief DataObject is an abstract base class representing a data object.
 */
struct DataObject : std::enable_shared_from_this<DataObject> {
  virtual ~DataObject() = default;

  virtual std::shared_ptr<DataObjectMeta> data_meta() const = 0;

  virtual void* ptr() = 0;

  template <typename T>
  T& as() {
    if (typeid(T) != data_meta()->data_type()) [[unlikely]] {
      throw std::runtime_error(absl::StrFormat("DataObject type mismatch: expected %s, got %s",
                                               typeid(T).name(), data_meta()->data_type().name()));
    }
    return *reinterpret_cast<T*>(ptr());
  }
};

/**
 * @brief DataMeta is a template class representing metadata for a specific data type T.
 */
template <typename T>
struct DataMeta : public DataObjectMeta {
  std::type_index data_type() const override { return typeid(T); }
};

}  // namespace data_flow