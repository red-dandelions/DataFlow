/**
 * @file data_text_parser.h
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-3
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include <memory>

#include "DataFlow/csrc/core/data_pipeline.h"
#include "DataFlow/csrc/streams/batch_row.h"
#include "DataFlow/csrc/streams/inflate_stream.h"

namespace data_flow {

class DataTextParser final : public DataPipeline {
 public:
  explicit DataTextParser(std::shared_ptr<DataPipeline> pipeline, const std::string &format,
                          std::vector<Column> &&columns, const char field_delim = '|');
  ~DataTextParser() final;

  std::shared_ptr<StreamMeta> output_stream_meta() const final;

  absl::StatusOr<std::shared_ptr<Stream>> next() final;

  virtual PyObject *as_python_object(std::shared_ptr<Stream> stream) const final;

 private:
  std::string_view try_read_line_from_inflate_stream();
  void try_parse_line(std::shared_ptr<BatchRow> batch_row, std::string_view line);

  std::shared_ptr<BatchRowMeta> output_stream_meta_;

  std::shared_ptr<DataPipeline> input_pipeline_;
  std::shared_ptr<InflateStream> inflate_stream_;
  std::span<const char> chunk_buffer_;
  std::string line_buffer_;
  size_t chunk_offset_ = 0;


  std::vector<std::string> format_fields_;
  const char field_delim_;
  
  const size_t kPerParseSize = 20 * 1024;
};
}  // namespace data_flow