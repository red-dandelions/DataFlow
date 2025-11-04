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

 namespace data_flow {
 
class DataTextParser final : public DataPipeline {
public:
    DataTextParser() {}
    ~DataTextParser() final {
        VLOG(1) << "[DataTextParser] destructor";
    }

    std::shared_ptr<StreamMeta> output_stream_meta() const final {
        return output_stream_meta_;
    }

    absl::StatusOr<std::shared_ptr<Stream>> next() final;

    virtual PyObject* as_python_object(std::shared_ptr<Stream> stream) const final;

private:
    std::shared_ptr<BatchStreamMeta> output_stream_meta_;
};

class DataTextParser final : public DataPipeline {
 public:
  DataTextParser(std::shared_ptr<DataPipeline> input,
                 std::string sample_format,    // e.g., "sample_id|group_id|sparse|dense|label"
                 std::string separator,
                 std::vector<DenseColumn> dense_cols,
                 std::vector<SparseColumn> sparse_cols)
      : input_(std::move(input)),
        separator_(std::move(separator)) {
    // 解析样本字段
    absl::StrSplit(sample_format, '|', &sample_fields_);
    meta_ = std::make_shared<BatchStreamMeta>(
        std::move(dense_cols), std::move(sparse_cols), sample_fields_, separator_);
  }

  // data_text_parser.cc  (片段)
#include "data_text_parser.h"

#include <algorithm>
#include <charconv>

#include "absl/status/statusor.h"
#include "absl/strings/str_split.h"

namespace data_flow {

// helper: trim CR (for windows \r\n)
static inline void trim_cr(std::string &s) {
  if (!s.empty() && s.back() == '\r') s.pop_back();
}

// helper: parse sparse token like "123:0.5" or "123" -> return id only
static std::vector<int64_t> parse_sparse_ids(const std::string &token) {
  std::vector<int64_t> ids;
  size_t start = 0;
  while (start < token.size()) {
    size_t comma = token.find(',', start);
    if (comma == std::string::npos) comma = token.size();
    std::string item = token.substr(start, comma - start);
    // item may be "id" or "id:weight" or empty
    if (!item.empty()) {
      size_t colon = item.find(':');
      std::string id_str = (colon == std::string::npos) ? item : item.substr(0, colon);
      try {
        int64_t id = std::stoll(id_str);
        ids.push_back(id);
      } catch (...) {
        // skip invalid id
      }
    }
    start = comma + 1;
  }
  return ids;
}

// helper: parse token into float (dense). return optional if parse fails.
static std::optional<float> parse_dense_value(const std::string &token) {
  if (token.empty()) return std::nullopt;
  try {
    float v = std::stof(token);
    return v;
  } catch (...) {
    return std::nullopt;
  }
}

// Find dense column index by name, or -1
static int find_dense_index(const std::shared_ptr<BatchStreamMeta> &meta, const std::string &name) {
  for (size_t i = 0; i < meta->dense_columns.size(); ++i) {
    if (meta->dense_columns[i].name == name) return static_cast<int>(i);
  }
  return -1;
}

// Find sparse column index by name, or -1
static int find_sparse_index(const std::shared_ptr<BatchStreamMeta> &meta, const std::string &name) {
  for (size_t i = 0; i < meta->sparse_columns.size(); ++i) {
    if (meta->sparse_columns[i].name == name) return static_cast<int>(i);
  }
  return -1;
}

absl::StatusOr<std::shared_ptr<Stream>> DataTextParser::next() {
  // Loop to try to produce one batch. We maintain buffer_ and current_inflate_stream_.
  while (true) {
    // ensure we have an active inflate_stream_
    if (!inflate_stream_) {
      // pull next Stream from input pipeline
      absl::StatusOr<std::shared_ptr<Stream>> upstream = input_->next();
      if (!upstream.ok()) return upstream.status();
      std::shared_ptr<Stream> upstream_stream = upstream.value();
      if (upstream_stream == nullptr) {
        // upstream exhausted
        return nullptr;
      }

      // ensure it's an InflateStream
      DATAFLOW_THROW_IF(upstream_stream->stream_meta()->stream_type_index() != typeid(InflateStream),
                        absl::StrFormat("Upstream must produce InflateStream, got: %s",
                                        demangle_type_name(upstream_stream->stream_meta()->stream_type_index())));
      inflate_stream_ = std::dynamic_pointer_cast<InflateStream>(upstream_stream);
      DATAFLOW_THROW_IF(inflate_stream_ == nullptr, "Dynamic cast to InflateStream failed");

      // reset buffer for new stream
      buffer_.clear();
      buffer_pos_ = 0;
    }

    // Prepare a batch holder
    auto batch = std::make_shared<SimpleBatchStream>(meta_);
    // Reserve vectors for features
    batch->mutable_dense_data().assign(meta_->dense_columns.size(), std::vector<float>{});
    batch->mutable_sparse_data().assign(meta_->sparse_columns.size(), std::vector<std::vector<int64_t>>{});

    // Repeatedly read chunks from inflate_stream_
    while (true) {
      // Read a chunk (0 -> use InflateStream default chunk size)
      std::span<const char> chunk = inflate_stream_->read_chunk(0);

      if (chunk.empty()) {
        // this inflate stream has no more data
        // finish parsing remaining buffer (if any newline-present), else discard partial line
        if (!buffer_.empty()) {
          // try to extract last line (no trailing newline): treat it as a line
          std::string line = buffer_.substr(buffer_pos_);
          trim_cr(line);
          if (!line.empty()) {
            // parse this final line
            parse_and_append_line(line, *batch);
          }
          buffer_.clear();
          buffer_pos_ = 0;
        }
        // drop current inflate_stream_ and break to fetch next upstream
        inflate_stream_.reset();
        break;
      }

      // append chunk to buffer
      buffer_.append(chunk.data(), chunk.size());

      // parse lines from buffer
      size_t parse_pos = buffer_pos_;
      while (true) {
        size_t nl = buffer_.find('\n', parse_pos);
        if (nl == std::string::npos) break;
        std::string line = buffer_.substr(buffer_pos_, nl - buffer_pos_);
        trim_cr(line);
        if (!line.empty()) {
          parse_and_append_line(line, *batch);
        }
        // advance buffer_pos_ past this line + newline
        buffer_pos_ = nl + 1;
        parse_pos = buffer_pos_;
      }

      // If we parsed some samples, we can return a batch now.
      if (batch->num_samples() > 0) {
        // If buffer_pos_ moved a lot, shrink buffer to keep only remaining partial part
        if (buffer_pos_ > 0) {
          if (buffer_pos_ < buffer_.size()) {
            buffer_ = buffer_.substr(buffer_pos_);
          } else {
            buffer_.clear();
          }
          buffer_pos_ = 0;
        }
        return batch;
      }

      // else continue to read next chunk and accumulate more lines
    } // end reading chunks from current inflate_stream_

    // If we reach here, that upstream stream finished but we didn't produce any samples from it.
    // Try the next upstream stream (loop continues).
  } // outer loop
}

// parse line and append to batch (helper method)
void DataTextParser::parse_and_append_line(const std::string &line, SimpleBatchStream &batch) {
  // tokenise by field_delim_
  std::vector<std::string> tokens;
  tokens.reserve(8);
  size_t start = 0;
  while (start <= line.size()) {
    size_t pos = line.find(field_delim_, start);
    if (pos == std::string::npos) pos = line.size();
    tokens.emplace_back(line.substr(start, pos - start));
    start = pos + 1;
  }

  // prepare per-sample containers
  // append placeholder entries for each dense/sparse slot
  size_t sample_idx = batch.num_samples(); // current sample index (before adding)
  for (auto &vec : batch.mutable_dense_data()) {
    // each dense feature will push back one float value per sample
    (void)vec;
  }
  for (auto &vecvec : batch.mutable_sparse_data()) {
    (void)vecvec;
  }

  // find mapping and fill
  for (size_t i = 0; i < tokens.size() && i < format_fields_.size(); ++i) {
    const std::string &field = format_fields_[i];
    const std::string &tok = tokens[i];

    int didx = find_dense_index(meta_, field);
    if (didx >= 0) {
      // parse dense scalar
      auto opt = parse_dense_value(tok);
      float v = opt.value_or(0.0f);
      batch.mutable_dense_data()[didx].push_back(v);
      continue;
    }

    int sidx = find_sparse_index(meta_, field);
    if (sidx >= 0) {
      // parse sparse ids
      auto ids = parse_sparse_ids(tok);
      batch.mutable_sparse_data()[sidx].push_back(std::move(ids));
      continue;
    }

    // if field is neither dense nor sparse, ignore for now (could be sample_id/label etc.)
  }

  // For any dense/sparse slot that didn't get a value (due to missing token), push default
  size_t num_dense = batch.num_dense_features();
  size_t num_sparse = batch.num_sparse_features();
  for (size_t di = 0; di < num_dense; ++di) {
    if (batch.mutable_dense_data()[di].size() == sample_idx) {
      // no value appended for this sample, push 0.0
      batch.mutable_dense_data()[di].push_back(0.0f);
    }
  }
  for (size_t si = 0; si < num_sparse; ++si) {
    if (batch.mutable_sparse_data()[si].size() == sample_idx) {
      // push empty vector
      batch.mutable_sparse_data()[si].push_back({});
    }
  }
}

}  // namespace data_flow



 }