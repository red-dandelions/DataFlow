#pragma once

#include <cuda_runtime.h>

namespace sgemm_cuda {

__global__ sgemm_kernel_v1(const float* A, const float* B, float* C, int32_t M, int32_t N,
                           int32_t K);

__global__ sgemm_kernel_v2(const float* A, const float* B, float* C, int32_t M, int32_t N,
                           int32_t K);

}  // namespace sgemm_cuda