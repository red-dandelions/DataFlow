/**
 * @file add_pipelines.cc
 * @brief Python bindings for DataPipeline classes.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include <cstdio>
#include <memory>

#include "glog/logging.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "DataFlow/csrc/pipelines/data_decompressor.h"
#include "DataFlow/csrc/pipelines/data_reader.h"
#include "DataFlow/csrc/module.h"

namespace data_flow {
void add_pipelines_bindings(pybind11::module& m) {
  /**
   * @brief DataReader bindings
   */
  auto cls =
      pybind11::class_<DataReader, std::shared_ptr<DataReader>, DataPipeline>(m, "DataReader");

  pybind11::enum_<DataReader::Source>(cls, "Source")
      .value("kFileList", DataReader::Source::kFileList)
      .value("kFilePipeline", DataReader::Source::kFilePipeline)
      .value("kPulsarMessageStream", DataReader::Source::kPulsarMessagePipeline);

  cls.def(pybind11::init([](pybind11::handle input_h, pybind11::handle source_h) {
            auto source = source_h.cast<DataReader::Source>();
            switch (source) {
              case DataReader::Source::kFileList: {
                std::vector<std::string> files = pybind11::cast<std::vector<std::string>>(input_h);
                return std::make_shared<DataReader>(std::move(files));
              }
              case DataReader::Source::kFilePipeline:
              case DataReader::Source::kPulsarMessagePipeline:
              default:
                DATAFLOW_THROW_IF(
                    true, absl::StrFormat("Unsupported Source: %d", static_cast<int32_t>(source)));
            }
          }),
          pybind11::arg("input"), pybind11::arg("source"))
      .def_property_readonly("output_stream_meta", &DataReader::output_stream_meta)
      .def("__iter__", [](std::shared_ptr<DataReader> self) {
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        return pybind11::reinterpret_borrow<pybind11::object>(obj);
      });

  /**
   * @brief DataDecompress bindings
   */
  pybind11::class_<DataDecompressor, std::shared_ptr<DataDecompressor>, DataPipeline>(
      m, "DataDecompressor")
      .def(pybind11::init([](pybind11::handle input_h) {
             auto input_pipeline = input_h.cast<std::shared_ptr<DataPipeline>>();
             return std::make_shared<DataDecompressor>(input_pipeline);
           }),
           pybind11::arg("input"))
      .def_property_readonly("output_stream_meta", &DataDecompressor::output_stream_meta)
      .def("__iter__", [](std::shared_ptr<DataDecompressor> self) {
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        return pybind11::reinterpret_borrow<pybind11::object>(obj);
      });
}
}  // namespace data_flow
