/**
 * @file module.h
 * @brief Declaration of module binding functions.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include "pybind11/pybind11.h"

namespace data_flow {

void add_core_bindings(pybind11::module& m);
void add_streams_bindings(pybind11::module& m);
void add_pipelines_bindings(pybind11::module& m);

}  // namespace data_flow