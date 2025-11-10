// 11.10
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "DataFlow/csrc/streams/batch_row.h"

namespace data_flow {

struct BatchMeta final : public StreamMetaBind<BatchMeta> {
  explicit BatchMeta(std::shared_ptr<BatchRowMeta> _batch_row_meta);

  ~BatchMeta() final = default;

  const std::shared_ptr<BatchRowMeta> batch_row_meta;
};

struct Batch final : public Stream {
  Batch(std::shared_ptr<BatchMeta> _batch_meta, size_t _batch_size);

  ~Batch() = default;
  std::shared_ptr<StreamMeta> stream_meta() const;

  void* ptr();

  void add_batch_row(std::shared_ptr<BatchRow> batch_row);
  std::shared_ptr<BatchRow> get_batch_row(size_t idx) {return batch_rows_[idx]; }

  const std::shared_ptr<BatchMeta> batch_meta;
  size_t batch_size;

 private:
  std::vector<std::shared_ptr<BatchRow>> batch_rows_;
};
}  // namespace data_flow