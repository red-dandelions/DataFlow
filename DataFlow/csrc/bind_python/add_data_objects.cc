/**
 * @file add_data_objects.cc
 * @brief Python bindings for DataPipeline classes.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-2
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include <memory>

#include "DataFlow/csrc/data_objects/byte_stream.h"
#include "DataFlow/csrc/data_objects/inflate_stream.h"
#include "DataFlow/csrc/module.h"

namespace data_flow {
void add_data_object_bindings(pybind11::module& m) {
  /**
   * @brief ByteStreamMeta and ByteStream bindings
   */
  pybind11::class_<ByteStreamMeta, std::shared_ptr<ByteStreamMeta>, DataObjectMeta>(
      m, "ByteStreamMeta")
      .def_property_readonly("data_type", [](std::shared_ptr<ByteStreamMeta> self) {
        return self->data_type().name();
      });

  pybind11::class_<ByteStream, std::shared_ptr<ByteStream>, DataObject>(m, "ByteStream")
      .def_property_readonly("data_meta",
                             [](std::shared_ptr<ByteStream> self) { return self->data_meta(); });

  /**
   * @brief InflateStreamMeta and InflateStream bindings.
   */
  pybind11::class_<InflateStreamMeta, std::shared_ptr<InflateStreamMeta>, DataObjectMeta>(
      m, "InflateStreamMeta")
      .def_property_readonly("data_type", [](std::shared_ptr<InflateStreamMeta> self) {
        return self->data_type().name();
      });

  pybind11::class_<InflateStream, std::shared_ptr<InflateStream>, DataObject>(m, "InflateStream")
      .def_property_readonly("data_meta",
                             [](std::shared_ptr<InflateStream> self) { return self->data_meta(); });
}
}  // namespace data_flow