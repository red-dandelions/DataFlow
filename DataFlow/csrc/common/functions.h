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
#include <string>

namespace data_flow {

struct Func {
  static bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
  }

  static bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  }
};
}  // namespace data_flow