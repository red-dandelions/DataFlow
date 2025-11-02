/**
 * @file add_data_pipelines.cc
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

#include "DataFlow/csrc/data_pipelines/data_decompressor.h"
#include "DataFlow/csrc/data_pipelines/data_reader.h"
#include "DataFlow/csrc/module.h"

namespace data_flow {
void add_data_pipeline_bindings(pybind11::module& m) {
  /**
   * @brief DataReader bindings
   */
  auto cls =
      pybind11::class_<DataReader, std::shared_ptr<DataReader>, DataPipeline>(m, "DataReader");

  pybind11::enum_<DataReader::FileSource>(cls, "FileSource")
      .value("kFileList", DataReader::FileSource::kFileList)
      .value("kStringStream", DataReader::FileSource::kStringStream);

  cls.def(pybind11::init([](pybind11::handle input_h, pybind11::handle file_source_h) {
            auto file_source = file_source_h.cast<DataReader::FileSource>();
            switch (file_source) {
              case DataReader::FileSource::kFileList: {
                std::vector<std::string> files = pybind11::cast<std::vector<std::string>>(input_h);
                return std::make_shared<DataReader>(std::move(files));
              }
              // case DataReader::FileSource::kStringStream: {
              //     auto string_stream = input_h.cast<std::shared_ptr<DataObject>>();
              //     return std::make_shared<DataReader>(string_stream);
              // }
              default:
                throw std::invalid_argument("Unknown FileSource");
            }
          }),
          pybind11::arg("input"), pybind11::arg("file_source"))
      .def_property_readonly("output_data_meta", &DataReader::output_data_meta)
      .def("__iter__", [](std::shared_ptr<DataReader> self) {
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        VLOG(6) << "[DataReader] Iterator object: " << obj;
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
           pybind11::arg("input_pipeline"))
      .def_property_readonly("output_data_meta", &DataDecompressor::output_data_meta)
      .def("__iter__", [](std::shared_ptr<DataDecompressor> self) {
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        VLOG(6) << "[DataDecompress] Iterator object: " << obj;
        return pybind11::reinterpret_borrow<pybind11::object>(obj);
      });
}
}  // namespace data_flow