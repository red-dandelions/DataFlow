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

#include "DataFlow/csrc/module.h"
#include "DataFlow/csrc/pipelines/data_decompressor.h"
#include "DataFlow/csrc/pipelines/data_reader.h"
#include "DataFlow/csrc/pipelines/data_text_parser.h"

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

  cls.def(pybind11::init([](pybind11::handle input_h,
                            pybind11::handle source_h) -> std::shared_ptr<DataReader> {
            HANDLE_DATAFLOW_ERRORS
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
            END_HANDLE_DATAFLOW_ERRORS
          }),
          pybind11::arg("input"), pybind11::arg("source"))
      .def_property_readonly("output_stream_meta", &DataReader::output_stream_meta)
      .def("__iter__", [](std::shared_ptr<DataReader> self) -> pybind11::object {
        HANDLE_DATAFLOW_ERRORS
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        return pybind11::reinterpret_borrow<pybind11::object>(obj);
        END_HANDLE_DATAFLOW_ERRORS_RET(pybind11::none())
      });

  /**
   * @brief DataDecompress bindings
   */
  pybind11::class_<DataDecompressor, std::shared_ptr<DataDecompressor>, DataPipeline>(
      m, "DataDecompressor")
      .def(pybind11::init([](pybind11::handle input_h) -> std::shared_ptr<DataDecompressor> {
             HANDLE_DATAFLOW_ERRORS
             auto input_pipeline = input_h.cast<std::shared_ptr<DataPipeline>>();
             return std::make_shared<DataDecompressor>(input_pipeline);
             END_HANDLE_DATAFLOW_ERRORS
           }),
           pybind11::arg("input"))
      .def_property_readonly("output_stream_meta", &DataDecompressor::output_stream_meta)
      .def("__iter__", [](std::shared_ptr<DataDecompressor> self) -> pybind11::object {
        HANDLE_DATAFLOW_ERRORS
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        return pybind11::reinterpret_borrow<pybind11::object>(obj);
        END_HANDLE_DATAFLOW_ERRORS_RET(pybind11::none())
      });

  //
  pybind11::class_<DataTextParser, std::shared_ptr<DataTextParser>, DataPipeline>(m,
                                                                                  "DataTextParser")
      .def(pybind11::init([](pybind11::handle input_h, const std::string& format,
                             pybind11::handle columns_h,
                             const char field_delim) -> std::shared_ptr<DataTextParser> {
             HANDLE_DATAFLOW_ERRORS
             auto input_pipeline = input_h.cast<std::shared_ptr<DataPipeline>>();

             DATAFLOW_THROW_IF(!pybind11::isinstance<pybind11::list>(columns_h),
                               "Expected a list of Column objects");

             pybind11::list columns_list = columns_h.cast<pybind11::list>();
             std::vector<Column> columns;
             for (auto item : columns_list) {
               DATAFLOW_THROW_IF(!pybind11::isinstance<Column>(item),
                                 "Expected a Column object in the list");
               columns.push_back(item.cast<Column>());
             }

             return std::make_shared<DataTextParser>(input_pipeline, format, std::move(columns),
                                                     field_delim);
             END_HANDLE_DATAFLOW_ERRORS
           }),
           pybind11::arg("input"), pybind11::arg("format"),
           pybind11::arg("columns") = std::vector<Column>{}, pybind11::arg("field_delim") = '|')
      .def_property_readonly("output_stream_meta", &DataTextParser::output_stream_meta)
      .def("__iter__", [](std::shared_ptr<DataTextParser> self) -> pybind11::object {
        HANDLE_DATAFLOW_ERRORS
        auto obj = GetDataPipelineIterator(std::reinterpret_pointer_cast<DataPipeline>(self));
        return pybind11::reinterpret_borrow<pybind11::object>(obj);
        END_HANDLE_DATAFLOW_ERRORS_RET(pybind11::none())
      });
}
}  // namespace data_flow