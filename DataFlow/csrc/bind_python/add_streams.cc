/**
 * @file add_streams.cc
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
void add_stream_bindings(pybind11::module& m) {
  /**
   * @brief ByteStreamMeta and ByteStream bindings
   */
  pybind11::class_<ByteStreamMeta, std::shared_ptr<ByteStreamMeta>, StreamMeta>(m, "ByteStreamMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<ByteStreamMeta> self) {
        return demangle_type_name(self->stream_type_index());
      });

  pybind11::class_<ByteStream, std::shared_ptr<ByteStream>, Stream>(m, "ByteStream")
      .def_property_readonly("stream_meta",
                             [](std::shared_ptr<ByteStream> self) { return self->stream_meta(); });

  /**
   * @brief InflateStreamMeta and InflateStream bindings.
   */
  pybind11::class_<InflateStreamMeta, std::shared_ptr<InflateStreamMeta>, StreamMeta>(
      m, "InflateStreamMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<InflateStreamMeta> self) {
        return demangle_type_name(self->stream_type_index());
      });

  pybind11::class_<InflateStream, std::shared_ptr<InflateStream>, Stream>(m, "InflateStream")
      .def_property_readonly(
          "stream_meta", [](std::shared_ptr<InflateStream> self) { return self->stream_meta(); });
}
}  // namespace data_flow