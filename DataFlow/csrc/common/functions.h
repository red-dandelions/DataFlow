/**
 * @file functions.h
 * @brief Common utility functions.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-2
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once
#include <cxxabi.h>
#include <memory>
#include <string>
#include <typeinfo>

namespace data_flow {

/**
 * @brief Utility functions for string operations.
 */
struct StringFunctors {
  static bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
  }

  static bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  }
};

/**
 * @brief Demangle the type name of a given type T.
 * @tparam T The type to demangle.
 * @return A string representing the demangled type name.
 */
template <typename T>
inline std::string demangle_type_name() {
  int status = 0;
  const char* name = typeid(T).name();
  std::unique_ptr<char[], void (*)(void*)> demangled(
      abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);
  return (status == 0) ? demangled.get() : name;
}

inline std::string demangle_str_name(const std::string& type_name) {
  int status = 0;
  const char* name = type_name.data();
  std::unique_ptr<char[], void (*)(void*)> demangled(
      abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);
  return (status == 0) ? demangled.get() : name;
}

}  // namespace data_flow