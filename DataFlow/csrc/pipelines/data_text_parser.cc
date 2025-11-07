// 11.6
#include "data_text_parser.h"

#include "absl/strings/str_split.h"
#include "glog/logging.h"
#include "pybind11/pybind11.h"

#include "DataFlow/csrc/streams/inflate_stream.h"

namespace data_flow {
DataTextParser::~DataTextParser() { VLOG(1) << "[DataTextParser] destructor"; }

std::shared_ptr<StreamMeta> DataTextParser::output_stream_meta() const {
  return output_stream_meta_;
}

PyObject *DataTextParser::as_python_object(std::shared_ptr<Stream> stream) const {
  DATAFLOW_THROW_IF(
      stream->stream_meta()->stream_type_index() != typeid(BatchRow),
      absl::StrFormat("Stream is not of type BatchRow, got: %s",
                      demangle_str_name(stream->stream_meta()->stream_type_index().name())));
  auto stream_ptr = std::dynamic_pointer_cast<BatchRow>(stream->shared_from_this());
  DATAFLOW_THROW_IF(stream_ptr == nullptr, "dynamic cast to BatchRow failed");

  return pybind11::cast(stream_ptr).release().ptr();
}

DataTextParser::DataTextParser(std::shared_ptr<DataPipeline> pipeline, const std::string &format,
                               std::vector<Column> &&columns, const char field_delim)
    : field_delim_(field_delim) {
  DATAFLOW_THROW_IF(
      pipeline->output_stream_meta()->stream_type_index() != typeid(InflateStream),
      absl::StrFormat(
          "Input DataPipeline must produce InflateStream, got: %s",
          demangle_str_name(pipeline->output_stream_meta()->stream_type_index().name())));

  input_pipeline_ = pipeline;

  // 解析 format 字段
  absl::StrSplit(format, field_delim_, &format_fields_);

  output_stream_meta_ = std::make_shared<BatchRowMeta>(std::move(columns));
}

absl::StatusOr<std::shared_ptr<Stream>> DataTextParser::next() {
  std::string_view line = try_read_line_from_inflate_stream();
  if (line.empty()) {
    VLOG(3) << "[DataTextParser] end of input";
    return nullptr;
  }

  LOG(INFO) << "read line: " << line;
  DATAFLOW_THROW_IF(true, "test");

  auto batch_row = std::make_shared<BatchRow>(output_stream_meta_);

  try_parse_line(batch_row, line);

  return batch_row;
}

std::string_view DataTextParser::try_read_line_from_inflate_stream() { return "test line"; }

void DataTextParser::try_parse_line(std::shared_ptr<BatchRow> batch_row, std::string_view line) {}

}  // namespace data_flow
