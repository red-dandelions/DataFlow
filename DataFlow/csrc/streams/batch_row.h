
// 11.6
#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

//#include "DataFlow/csrc/common/batch_area.h"
#include "DataFlow/csrc/core/stream.h"

namespace data_flow {
class BatchRow;

enum class ColumnType { kDense, kSparse, kString };

struct Column {
  std::string name;
  int32_t dtype;
  std::vector<int64_t> shape;
  ColumnType column_type;
};

struct BatchRowMeta final : public StreamMetaBind<BatchRow> {
  const size_t original_column_size;
  std::vector<Column> columns;
  std::unordered_map<std::string, size_t> column_name_to_index;

  explicit BatchRowMeta(std::vector<Column>&& _columns);

  ~BatchRowMeta() final = default;

  const Column& get_column_by_name(const std::string& name) const;

  const Column& get_column_by_index(size_t index) const;
};

class BatchRow final : public Stream {
 public:
  struct alignas(16) ColumnBlock {
    size_t data_size;
    union {
      void* ptr;
      int64_t offset;
      int64_t packed_data;
    };
  };

 public:
  explicit BatchRow(std::shared_ptr<BatchRowMeta> batch_row_meta);

  ~BatchRow() final = default;

  std::shared_ptr<StreamMeta> stream_meta() const final;

  void* ptr() final;

  ColumnBlock* get_column_block(size_t index);
  //void* update_column_block_data(size_t index, size_t data_size, void* data);

 private:
  int32_t column_block_size_;
  std::unique_ptr<ColumnBlock[]> column_blocks_;
  std::unique_ptr<ColumnBlock[]> external_column_blocks_;
  std::shared_ptr<BatchRowMeta> batch_row_meta_;
  //BatchArea area_;
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