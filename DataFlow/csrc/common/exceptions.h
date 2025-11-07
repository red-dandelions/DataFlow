/**
 * @file exceptions.h
 * @brief exception handling utilities.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-3
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
inline std::string format_message(const std::string& msg, size_t width = 70, size_t indent = 11) {
  std::ostringstream oss;
  size_t start = 0;
  while (start < msg.size()) {
    size_t end = std::min(start + width, msg.size());
    // 找到最后一个空格
    if (end < msg.size()) {
      size_t space = msg.rfind(' ', end);
      if (space != std::string::npos && space > start) end = space;
    }
    oss << msg.substr(start, end - start);
    start = end;
    while (start < msg.size() && msg[start] == ' ') ++start;  // 跳过多余空格
    if (start < msg.size()) oss << "\n" << std::string(indent, ' ');
  }
  return oss.str();
}
}  // namespace

// Macro to throw a runtime_error with detailed message if condition is true.
#define DATAFLOW_THROW_IF(condition, message)                                                      \
  do {                                                                                             \
    if ((condition)) [[unlikely]] {                                                                \
      std::ostringstream oss;                                                                      \
      oss << "\n";                                                                                 \
      oss << "================================================================================\n"; \
      oss << "[DataFlow] C++ RUNTIME ERROR\n";                                                     \
      oss << "--------------------------------------------------------------------------------\n"; \
      oss << "condition: " << #condition << "\n";                                                  \
      oss << "message:   " << format_message(message) << "\n";                                     \
      auto loc = std::source_location::current();                                                  \
      oss << "--------------------------------------------------------------------------------\n"; \
      oss << "Function:  " << loc.function_name() << "\n";                                         \
      oss << "File:      " << loc.file_name() << ":" << loc.line() << "\n";                        \
      oss << "================================================================================\n"; \
      throw std::runtime_error(oss.str());                                                         \
    }                                                                                              \
  } while (false)

#define HANDLE_DATAFLOW_ERRORS try {
#define END_HANDLE_DATAFLOW_ERRORS_RET(retval)            \
  }                                                       \
  catch (pybind11::error_already_set & e) {               \
    e.restore();                                          \
    return retval;                                        \
  }                                                       \
  catch (pybind11::builtin_exception & e) {               \
    e.set_error();                                        \
    return retval;                                        \
  }                                                       \
  catch (std::exception & e) {                            \
    PyErr_SetString(PyExc_RuntimeError, e.what());        \
    return retval;                                        \
  }                                                       \
  catch (...) {                                           \
    PyErr_SetString(PyExc_RuntimeError, "unknown error"); \
    return retval;                                        \
  }
#define END_HANDLE_DATAFLOW_ERRORS END_HANDLE_DATAFLOW_ERRORS_RET(nullptr)
