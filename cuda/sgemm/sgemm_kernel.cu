#include "sgemm_kernel.cuh"

namespace sgemm_cuda {

__global__ sgemm_kernel_v1(const float* A, const float* B, float* C, int32_t M, int32_t N,
                           int32_t K) {
  int32_t row = blockIdx.x * blockDim.x + threadIdx.x;
  int32_t col = blockIdx.y * blockDim.y + threadIdx.y;
  if (row >= M || col >= N) {
    return;
  }
  float v = 0;
  for (int32_t i = 0; i < K; ++i) {
    v += A[row * K + k] * B[k * N + col];
  }
  C[row * N + col] = v;
}
}  // namespace sgemm_cuda