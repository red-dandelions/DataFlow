#include "string_stream.h"

namespace data_flow {

StringStream::StringStream(const std::string& input_string) : value_(input_string) {}

std::shared_ptr<StreamMeta> StringStream::stream_meta() const {
  return std::make_shared<StringStreamMeta>();
}

void* StringStream::ptr() { return this; }

std::string StringStream::value() const { return value_; }

}  // namespace data_flow