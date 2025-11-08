// 11.6

#include "batch_row.h"

#include <cstdint>
#include <memory>

#include "DataFlow/csrc/common/exceptions.h"

namespace data_flow {

BatchRowMeta::BatchRowMeta(std::vector<Column>&& _columns)
    : original_column_size(_columns.size()), columns(_columns) {
  std::sort(columns.begin(), columns.end(), [](const Column& a, const Column& b) {
    return a.column_type < b.column_type;
  });
  for (size_t i = 0; i < columns.size(); ++i) {
    column_name_to_index[columns[i].name] = i;
    int64_t item_count = 1;
    for (auto v : columns[i].shape) {
      item_count *= v;
      if (item_count < 0) {
        break;
      }
    }
    if (item_count >= 0) {
      columns[i].item_count = item_count;
    }
  }
}

const Column& BatchRowMeta::get_column_by_name(const std::string& name) const {
  auto it = column_name_to_index.find(name);
  DATAFLOW_THROW_IF(it == column_name_to_index.end(),
                    absl::StrFormat("Column name '%s' not found in BatchRowMeta", name));
  return columns[it->second];
}

const Column& BatchRowMeta::get_column_by_index(size_t index) const {
  DATAFLOW_THROW_IF(
      index >= columns.size(),
      absl::StrFormat("Column index %d out of range, size=%d", index, columns.size()));
  return columns[index];
}

BatchRow::BatchRow(std::shared_ptr<BatchRowMeta> batch_row_meta) : batch_row_meta_(batch_row_meta) {
  column_blocks_ = std::make_unique<ColumnBlock[]>(batch_row_meta_->original_column_size);
}

std::shared_ptr<StreamMeta> BatchRow::stream_meta() const {
  return std::dynamic_pointer_cast<StreamMeta>(batch_row_meta_);
}

void* BatchRow::ptr() { return this; }

BatchRow::ColumnBlock* BatchRow::get_column_block(size_t index) {
  DATAFLOW_THROW_IF(index >= batch_row_meta_->columns.size(),
                    absl::StrFormat("Column index %d out of range, size=%d", index,
                                    batch_row_meta_->columns.size()));

  if (index >= batch_row_meta_->original_column_size) {
    return &external_column_blocks_[index - batch_row_meta_->original_column_size];
  }

  return &column_blocks_[index];
}

void* BatchRow::alloc_column_block_data(size_t index, size_t byte_size) {
  auto column_block = get_column_block(index);
  const auto& column = batch_row_meta_->columns[index];

  column_block->byte_size = byte_size;
  if (byte_size <= 8) {
    return &column_block->packed_data;
  }

  switch (column.column_type) {
    case ColumnType::kDense: {
      void* addr = area_.align_allocate<float>(byte_size);
      column_block->ptr = addr;
      break;
    }
    case ColumnType::kSparse: {
      void* addr = area_.align_allocate<uint64_t>(byte_size);
      column_block->ptr = addr;
      break;
    }
    case data_flow::ColumnType::kString: {
      void* addr = area_.align_allocate<char>(byte_size);
      column_block->ptr = addr;
      break;
    }
    default: {
      DATAFLOW_THROW_IF(true, "should not be here");
    }
  }

  return column_block->ptr;
}

}  // namespace data_flow