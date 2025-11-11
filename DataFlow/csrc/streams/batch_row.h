
// 11.6
#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "DataFlow/csrc/batch/batch_row_area.h"
#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/core/stream.h"

namespace data_flow {
class BatchRow;

enum class ColumnType { kDense, kSparse, kString };

// TODO: dtype 模版自动推断 T，替换 float, int64_t
struct Column {
  std::string name;
  int32_t dtype;
  std::vector<int64_t> shape;
  ColumnType column_type;
  int64_t item_count = -1;
  Column() = default;
  Column(std::string&& _name, int32_t _dtype, std::vector<int64_t>&& _shape,
         ColumnType _column_type)
      : name(std::move(_name)), dtype(_dtype), shape(std::move(_shape)), column_type(_column_type) {
    int64_t cnt = 1;
    for (auto v : shape) {
      cnt *= v;
      if (cnt < 0) {
        break;
      }
    }
    if (cnt >= 0) {
      item_count = cnt;
    }
  }
  ~Column() = default;
};

struct BatchRowMeta final : public StreamMetaBind<BatchRow> {
  size_t original_column_size;
  std::vector<Column> columns;
  std::unordered_map<std::string_view, size_t> column_name_to_index;

  explicit BatchRowMeta(std::vector<Column>&& _columns);

  ~BatchRowMeta() final = default;

  const Column& get_column_by_name(const std::string& name) const;

  const Column& get_column_by_index(size_t index) const;
};

class BatchRow final : public Stream {
 public:
  struct alignas(16) ColumnBlock {
    size_t byte_size = 0;
    union {
      void* ptr = nullptr;
      int64_t packed_data;
    };
    void* data() { return byte_size <= 8 ? reinterpret_cast<void*>(&packed_data) : ptr; }
  };

 public:
  explicit BatchRow(std::shared_ptr<BatchRowMeta> batch_row_meta);

  ~BatchRow() final = default;

  std::shared_ptr<StreamMeta> stream_meta() const final;

  void* ptr() final;

  ColumnBlock* get_column_block(size_t index);
  void* alloc_column_block_data(size_t index, size_t data_size);

  void set_external_data(const std::string& key, const std::string_view value) {
    external_data_[key] = std::string(value);
  }

  const std::unordered_map<std::string, std::string>& external_data() { return external_data_; }

  void set_batch_row_meta(std::shared_ptr<BatchRowMeta> batch_row_meta) {
    size_t ori_exteral_size =
        batch_row_meta_->columns.size() - batch_row_meta_->original_column_size;
    size_t new_external_size =
        batch_row_meta->columns.size() - batch_row_meta->original_column_size;
    ColumnBlock* p = new ColumnBlock[new_external_size]();
    DATAFLOW_THROW_IF(ori_exteral_size > new_external_size,
                      absl::StrFormat("ori_exteral_size: %d, new_external_size: %d",
                                      ori_exteral_size, new_external_size));
    for (size_t i = 0; i < ori_exteral_size; ++i) {
      p[i] = external_column_blocks_[i];
    }
    external_column_blocks_.reset(p);
    batch_row_meta_ = batch_row_meta;
  }

 private:
  std::unique_ptr<ColumnBlock[]> column_blocks_;
  std::unique_ptr<ColumnBlock[]> external_column_blocks_;
  std::shared_ptr<BatchRowMeta> batch_row_meta_;
  std::unordered_map<std::string, std::string> external_data_;
  BatchRowArea area_;
};

inline std::string to_string(const ColumnType& column_type) {
  switch (column_type) {
    case ColumnType::kDense:
      return std::string("Dense");
    case ColumnType::kSparse:
      return std::string("Sparse");
    case ColumnType::kString:
      return std::string("String");
    default:
      return std::string("Unknown");
  }
}

// TODO: 完善 dtype 到字符串的转换
inline std::string dtype_string(int32_t dtype) { return std::to_string(dtype); }

inline std::ostream& operator<<(std::ostream& os, const std::vector<int64_t>& v) {
  os << "[";
  for (size_t i = 0; i < v.size(); ++i) {
    os << v[i];
    if (i != v.size() - 1) {
      os << ",";
    }
  }
  os << "]";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const Column& column) {
  os << "{name:" << column.name << ",dtype:" << dtype_string(column.dtype)
     << ",shape:" << column.shape << ",column_type:" << to_string(column.column_type) << "}";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const std::vector<Column>& columns) {
  os << "[";
  for (size_t i = 0; i < columns.size(); ++i) {
    os << columns[i];
    if (i != columns.size() - 1) {
      os << ",";
    }
  }
  os << "]";
  return os;
}

}  // namespace data_flow