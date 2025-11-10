// 11.8
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

#include "DataFlow/csrc/common/macros.h"

namespace data_flow {

struct MemoryBlock {
  void* addr = nullptr;
  size_t size = 0;
  size_t allocated_size = 0;
  std::atomic_int32_t refcount = {1};
};

struct BatchRowArea {
  DISABLE_COPY_MOVE_AND_ASSIGN(BatchRowArea);

  BatchRowArea() = default;
  ~BatchRowArea();

  void* allocate(size_t size, size_t align);

  template <typename T>
  T* align_allocate(size_t size) {
    return reinterpret_cast<T*>(allocate(size, alignof(T)));
  }

 private:
  std::vector<std::shared_ptr<MemoryBlock>> blocks_;
};
}  // namespace data_flow