/**
 * @file add_core.cc
 * @brief Python bindings for core DataFlow classes.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/module.h"

namespace data_flow {
void add_core_bindings(pybind11::module& m) {
  pybind11::class_<StreamMeta, std::shared_ptr<StreamMeta>>(m, "StreamMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<StreamMeta>) {
        return pybind11::none();  // abstract property
      });

  pybind11::class_<Stream, std::shared_ptr<Stream>>(m, "Stream")
      .def_property_readonly("stream_meta", [](std::shared_ptr<Stream>) {
        return pybind11::none();  // abstract property
      });

  pybind11::class_<DataPipeline, std::shared_ptr<DataPipeline>>(m, "DataPipeline")
      .def("output_data_meta", [](std::shared_ptr<DataPipeline>) {
        return pybind11::none();  // abstract method
      });
}
}  // namespace data_flow