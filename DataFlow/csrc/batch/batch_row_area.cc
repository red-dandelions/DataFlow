// 2025.11.8

#include "batch_row_area.h"

#include <cerrno>
#include <cstddef>
#include <mutex>
#include <sys/mman.h>

#include "absl/strings/str_format.h"

#include "DataFlow/csrc/common/exceptions.h"
#include "DataFlow/csrc/common/macros.h"

namespace data_flow {
namespace {
struct MemoryBlockManager {
  DISABLE_COPY_MOVE_AND_ASSIGN(MemoryBlockManager);
  static constexpr size_t kPerAllocSize = 128 * 1024;  // 128 k
  static MemoryBlockManager* Instance() {
    static MemoryBlockManager mbm = MemoryBlockManager();
    return &mbm;
  }

  ~MemoryBlockManager() {
    for (auto block : free_memory_blocks_) {
      munmap(block->addr, block->size);
      delete block;
    }
  }

  MemoryBlock* get_memory_block(size_t size) {
    const size_t alloc_size = ALIGN_SIZE(std::max<size_t>(size, kPerAllocSize), 4096);
    size_t idx = 0;
    for (; idx < free_memory_blocks_.size(); ++idx) {
      if (free_memory_blocks_[idx]->size >= alloc_size) {
        break;
      }
    }
    if (idx >= free_memory_blocks_.size()) {
      auto mem = new MemoryBlock();
      mem->refcount = 1;
      mem->allocated_size = 0;
      mem->size = alloc_size;
      mem->addr =
          mmap(nullptr, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      DATAFLOW_THROW_IF(
          mem->addr == MAP_FAILED,
          absl::StrFormat("mmap failed, err: %s, alloc_size: %d", strerror(errno), alloc_size));
      return mem;
    }

    auto mem = free_memory_blocks_[idx];
    free_memory_blocks_[idx] = free_memory_blocks_.back();
    free_memory_blocks_.pop_back();
    return mem;
  }

  void release_memory_block(MemoryBlock* block) {
    if (--block->refcount == 0) {
      std::lock_guard<std::mutex> lock(mtx_);
      block->allocated_size = 0;
      free_memory_blocks_.push_back(block);
    }
  }

 private:
  std::mutex mtx_;
  std::vector<MemoryBlock*> free_memory_blocks_;
  MemoryBlockManager() = default;
};
}  // namespace

static MemoryBlock* g_mem_block =
    MemoryBlockManager::Instance()->get_memory_block(MemoryBlockManager::kPerAllocSize);

BatchRowArea::~BatchRowArea() {
  for (auto block : blocks_) {
    MemoryBlockManager::Instance()->release_memory_block(block);
  }
}

void* BatchRowArea::allocate(size_t size, size_t align) {
  size_t alloc_begin_pos = ALIGN_SIZE(g_mem_block->allocated_size, align);

  if (alloc_begin_pos + size > g_mem_block->size) {
    MemoryBlockManager::Instance()->release_memory_block(g_mem_block);
    g_mem_block = MemoryBlockManager::Instance()->get_memory_block(size);
    alloc_begin_pos = 0;
  }

  if (blocks_.empty() || blocks_.back() != g_mem_block) {
    blocks_.push_back(g_mem_block);
    g_mem_block->refcount++;
  }

  g_mem_block->allocated_size = alloc_begin_pos + size;
  return reinterpret_cast<char*>(g_mem_block->addr) + alloc_begin_pos;
}
}  // namespace data_flow
