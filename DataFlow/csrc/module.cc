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

#include "glog/logging.h"

#include "DataFlow/csrc/core/data_object.h"

PYBIND11_MODULE(pybind_module, m) {
  // 初始化 glog，默认 /tmp
  google::InitGoogleLogging("DataFlow");

  // 配置日志输出到终端（STDERR）
  google::SetStderrLogging(google::GLOG_INFO);  // INFO 及以上级别日志输出到终端

  data_flow::add_core_bindings(m);
  data_flow::add_data_object_bindings(m);
  data_flow::add_data_pipeline_bindings(m);
}