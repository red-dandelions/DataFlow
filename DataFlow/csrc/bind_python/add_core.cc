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
  pybind11::class_<DataObjectMeta, std::shared_ptr<DataObjectMeta>>(m, "DataObjectMeta")
      .def_property_readonly("data_type", [](std::shared_ptr<DataObjectMeta> self) {
        return self->data_type().name();
      });

  pybind11::class_<DataObject, std::shared_ptr<DataObject>>(m, "DataObject")
      .def_property_readonly("data_meta", [](DataObject* self) { return self->data_meta(); });

  pybind11::class_<DataPipeline, std::shared_ptr<DataPipeline>>(m, "DataPipeline")
      .def("output_data_meta", [](std::shared_ptr<DataPipeline>) { return pybind11::none(); });
}
}  // namespace data_flow