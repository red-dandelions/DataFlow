// 11.10
#include "batch.h"

namespace data_flow {

BatchMeta::BatchMeta(std::shared_ptr<BatchRowMeta> _batch_row_meta)
    : batch_row_meta(_batch_row_meta) {}

Batch::Batch(std::shared_ptr<BatchMeta> _batch_meta, size_t _batch_size)
    : batch_meta(_batch_meta), batch_size(_batch_size) {}

std::shared_ptr<StreamMeta> Batch::stream_meta() const { return batch_meta; }

void* Batch::ptr() { return this; }

void Batch::add_batch_row(std::shared_ptr<BatchRow> batch_row) { batch_rows_.push_back(batch_row); }

}  // namespace data_flow