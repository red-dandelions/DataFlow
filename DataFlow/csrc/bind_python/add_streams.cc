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
#include <sstream>
#include <string_view>

#include "DataFlow/csrc/module.h"
#include "DataFlow/csrc/pipelines/data_batcher.h"
#include "DataFlow/csrc/streams/batch.h"
#include "DataFlow/csrc/streams/batch_row.h"
#include "DataFlow/csrc/streams/byte_stream.h"
#include "DataFlow/csrc/streams/inflate_stream.h"
#include "DataFlow/csrc/streams/string_stream.h"

namespace data_flow {
void add_streams_bindings(pybind11::module& m) {
  /**
   * @brief ByteStreamMeta and ByteStream bindings
   */
  pybind11::class_<ByteStreamMeta, std::shared_ptr<ByteStreamMeta>, StreamMeta>(m, "ByteStreamMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<ByteStreamMeta> self) {
        return demangle_str_name(self->stream_type_index().name());
      });

  pybind11::class_<ByteStream, std::shared_ptr<ByteStream>, Stream>(m, "ByteStream")
      .def_property_readonly("stream_meta",
                             [](std::shared_ptr<ByteStream> self) { return self->stream_meta(); });

  pybind11::class_<StringStreamMeta, std::shared_ptr<StringStreamMeta>, StreamMeta>(
      m, "StringStreamMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<StringStreamMeta> self) {
        return demangle_str_name(self->stream_type_index().name());
      });
  pybind11::class_<StringStream, std::shared_ptr<StringStream>, Stream>(m, "StringStream")
      .def_property_readonly("stream_meta",
                             [](std::shared_ptr<StringStream> self) { return self->stream_meta(); })
      .def("__str__", [](std::shared_ptr<StringStream> self) { return self->value(); });

  /**
   * @brief InflateStreamMeta and InflateStream bindings.
   */
  pybind11::class_<InflateStreamMeta, std::shared_ptr<InflateStreamMeta>, StreamMeta>(
      m, "InflateStreamMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<InflateStreamMeta> self) {
        return demangle_str_name(self->stream_type_index().name());
      });

  pybind11::class_<InflateStream, std::shared_ptr<InflateStream>, Stream>(m, "InflateStream")
      .def_property_readonly(
          "stream_meta", [](std::shared_ptr<InflateStream> self) { return self->stream_meta(); });

  pybind11::enum_<ColumnType>(m, "ColumnType")
      .value("dense", ColumnType::kDense)
      .value("sparse", ColumnType::kSparse)
      .value("string", ColumnType::kString);

  pybind11::class_<Column, std::shared_ptr<Column>>(m, "Column")
      .def(pybind11::init([](std::string name, pybind11::handle dtype_h, pybind11::tuple shape,
                             ColumnType column_type) -> Column {
             auto check_get_dtype = [&]() -> int {
               auto np = pybind11::module_::import("numpy");
               // 如果传入的不是 dtype，则先转成 dtype
               if (!pybind11::hasattr(dtype_h, "num")) {
                 dtype_h = np.attr("dtype")(dtype_h);
               }
               return pybind11::cast<int>(dtype_h.attr("num"));
             };

             std::vector<int64_t> column_shape;
             for (auto item : shape) {
               DATAFLOW_THROW_IF(!pybind11::isinstance<pybind11::int_>(item),
                                 "Shape items must be integers");
               column_shape.push_back(item.cast<int64_t>());
             }

             Column column(std::move(name), check_get_dtype(), std::move(column_shape),
                           column_type);

             return column;
           }),
           pybind11::arg("name"), pybind11::arg("dtype"), pybind11::arg("shape"),
           pybind11::arg("column_type"))
      .def_readonly("name", &Column::name)
      .def_readonly("dtype", &Column::dtype)
      .def_property_readonly("shape",
                             [](const Column& self) {
                               auto tuple = pybind11::tuple(self.shape.size());
                               for (size_t i = 0; i < self.shape.size(); ++i) {
                                 tuple[i] = pybind11::int_(self.shape[i]);
                               }
                               return tuple;
                             })
      .def_readonly("column_type", &Column::column_type)
      .def("__str__", [](const Column& self) {
        std::ostringstream oss;
        oss << self;
        return oss.str();
      });

  pybind11::class_<BatchRowMeta, std::shared_ptr<BatchRowMeta>, StreamMeta>(m, "BatchRowMeta")
      .def(pybind11::init([](pybind11::handle columns_h) -> std::shared_ptr<BatchRowMeta> {
             HANDLE_DATAFLOW_ERRORS
             DATAFLOW_THROW_IF(!pybind11::isinstance<pybind11::list>(columns_h),
                               "Expected a list of Column objects");

             pybind11::list columns_list = columns_h.cast<pybind11::list>();
             std::vector<Column> columns;
             for (auto item : columns_list) {
               DATAFLOW_THROW_IF(!pybind11::isinstance<Column>(item),
                                 "Expected a Column object in the list");
               columns.push_back(item.cast<Column>());
             }

             return std::make_shared<BatchRowMeta>(std::move(columns));
             END_HANDLE_DATAFLOW_ERRORS
           }),
           pybind11::arg("columns"))
      .def_property_readonly("stream_type_name",
                             [](std::shared_ptr<BatchRowMeta> self) {
                               return demangle_str_name(self->stream_type_index().name());
                             })
      .def(
          "get_column_by_name",
          [](std::shared_ptr<BatchRowMeta> self, const std::string& name) {
            return self->get_column_by_name(name);
          },
          pybind11::arg("name"))
      .def(
          "get_column_by_index",
          [](std::shared_ptr<BatchRowMeta> self, size_t index) {
            return self->get_column_by_index(index);
          },
          pybind11::arg("index"));

  pybind11::class_<BatchRow, std::shared_ptr<BatchRow>, Stream>(m, "BatchRow")
      .def(pybind11::init([](pybind11::handle batch_row_meta_h) -> std::shared_ptr<BatchRow> {
             HANDLE_DATAFLOW_ERRORS
             DATAFLOW_THROW_IF(!pybind11::isinstance<BatchRowMeta>(batch_row_meta_h),
                               "Expected BatchRowMeta");
             std::shared_ptr<BatchRowMeta> batch_row_meta =
                 batch_row_meta_h.cast<std::shared_ptr<BatchRowMeta>>();
             return std::make_shared<BatchRow>(batch_row_meta);
             END_HANDLE_DATAFLOW_ERRORS
           }),
           pybind11::arg("batch_row_meta"))
      .def_property_readonly("stream_meta",
                             [](std::shared_ptr<BatchRow> self) { return self->stream_meta(); })
      .def("__str__", [](std::shared_ptr<BatchRow> self) {
        const auto& meta = std::dynamic_pointer_cast<BatchRowMeta>(self->stream_meta());

        std::ostringstream oss;
        // oss << meta;
        oss << "{\n";

        for (size_t i = 0; i < meta->columns.size(); ++i) {
          const auto& column = meta->columns[i];
          const auto b = self->get_column_block(i);

          oss << "\"" << column.name << "\": [";
          if (column.column_type == ColumnType::kDense) {
            size_t item_count = b->byte_size / sizeof(float);
            for (size_t i = 0; i < item_count; ++i) {
              oss << reinterpret_cast<float*>(b->byte_size > 8 ? b->ptr : &b->packed_data)[i];
              if (i != item_count - 1) {
                oss << ",";
              }
            }
          } else if (column.column_type == ColumnType::kSparse) {
            size_t item_count = b->byte_size / sizeof(int64_t);
            for (size_t i = 0; i < item_count; ++i) {
              oss << reinterpret_cast<int64_t*>(b->byte_size > 8 ? b->ptr : &b->packed_data)[i];
              if (i != item_count - 1) {
                oss << ",";
              }
            }
          } else {
            std::string_view data = std::string_view(
                reinterpret_cast<char*>(b->byte_size > 8 ? b->ptr : &b->packed_data), b->byte_size);
            oss << data << "]";
          }

          oss << "]";
          if (i != meta->columns.size() - 1) {
            oss << ",\n";
          }
        }
        oss << "\n}";
        return oss.str();
      });

  pybind11::class_<BatchMeta, std::shared_ptr<BatchMeta>, StreamMeta>(m, "BatchMeta")
      .def_property_readonly("stream_type_name", [](std::shared_ptr<BatchMeta> self) {
        return demangle_str_name(self->stream_type_index().name());
      });
  pybind11::class_<Batch, std::shared_ptr<Batch>, Stream>(m, "Batch")
      .def_property_readonly("stream_meta",
                             [](std::shared_ptr<Batch> self) { return self->stream_meta(); });
}
}  // namespace data_flow
