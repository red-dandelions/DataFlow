/**
 * @file module.cc
 * @brief Definition of the Python module and its bindings.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include "module.h"

#include <exception>

#include "glog/logging.h"

#include "DataFlow/csrc/common/exceptions.h"

PYBIND11_MODULE(pybind_module, m) {
  HANDLE_DATAFLOW_ERRORS

  data_flow::setup_signal_handler();

  // 初始化 glog，默认 /tmp
  google::InitGoogleLogging("DataFlow");

  // 配置日志输出到终端（STDERR）
  google::SetStderrLogging(google::GLOG_INFO);  // INFO 及以上级别日志输出到终端

  data_flow::add_core_bindings(m);
  data_flow::add_streams_bindings(m);
  data_flow::add_pipelines_bindings(m);

  END_HANDLE_DATAFLOW_ERRORS_RET()
}