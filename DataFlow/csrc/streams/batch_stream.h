

#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

#include "DataFlow//csrc//core/stream.h"

namespace data_flow {
class BatchStream;

enum class ColumnType { kDense, kSparse, kString };

struct Column {
  std::string name;
  int32_t dtype;
  std::vector<int64_t> shape;
  ColumnType column_type;
};

struct BatchStreamMeta : public StreamMetaBind<BatchStream> {
  const std::vector<Column> columns;
  const std::unordered_map<std::string, size_t> column_name_to_index;

  explicit BatchStreamMeta(std::vector<Column> _columns) : columns(std::move(_columns)) {
    for (size_t i = 0; i < columns.size(); ++i) {
      column_name_to_index.insert({columns[i].name, i});
    }
  }

  ~BatchStreamMeta() final = default;
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

inline std::string dtype_string(int32_t dtype) { return ""; }

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

class BatchStream : public Stream {
 public:
  explicit BatchStream(std::shared_ptr<BatchStreamMeta> meta) : meta_(std::move(meta)) {}

  ~BatchStream() final = default;

  std::shared_ptr<StreamMeta> stream_meta() const final { return meta_; }

  void* ptr() final { return this; }

  PyObject* as_python_object(std::shared_ptr<Stream> stream) const;

  // ===== Dense 数据接口 =====
  // 获取 dense 特征数据（按列组织，每列是一批样本）
  virtual const std::vector<std::vector<float>>& dense_data() const = 0;
  virtual std::vector<std::vector<float>>& mutable_dense_data() = 0;

  // ===== Sparse 数据接口 =====
  // 每个 sparse slot 对应一批样本，每个样本是一组 int64_t 的 ID
  virtual const std::vector<std::vector<std::vector<int64_t>>>& sparse_data() const = 0;
  virtual std::vector<std::vector<std::vector<int64_t>>>& mutable_sparse_data() = 0;

  // ===== Batch 维度信息 =====
  virtual size_t num_samples() const = 0;
  virtual size_t num_dense_features() const = 0;
  virtual size_t num_sparse_features() const = 0;

 protected:
  std::shared_ptr<BatchStreamMeta> meta_;
};

}  // namespace data_flow