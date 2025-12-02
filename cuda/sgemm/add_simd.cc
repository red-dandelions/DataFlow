#include <chrono>
#include <immintrin.h>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>
#include <xmmintrin.h>

template <int32_t N>
void __cpu_add_kernel(const float*, const float*, float*);

template <>
void __cpu_add_kernel<1>(const float* a, const float* b, float* dst) {
  dst[0] = a[0] + b[0];
}

// 2 * 32 = 64 bit
template <>
void __cpu_add_kernel<2>(const float* a, const float* b, float* dst) {
  __m128 va = _mm_set_ps(0.0f, 0.0f, a[1], a[0]);
  __m128 vb = _mm_set_ps(0.0f, 0.0f, b[1], b[0]);
  __m128 vc = _mm_add_ps(va, vb);
  // 提取前两个 float 输出
  _mm_storel_pi((__m64*)dst, vc);
}

// 4 * 32 = 128 bit
template <>
void __cpu_add_kernel<4>(const float* a, const float* b, float* dst) {
  _mm_store_ps(dst, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

// 8 * 32 = 256 bit
template <>
void __cpu_add_kernel<8>(const float* a, const float* b, float* dst) {
  __m256 va = _mm256_load_ps(a);
  __m256 vb = _mm256_load_ps(b);
  __m256 vc = _mm256_add_ps(va, vb);
  _mm256_store_ps(dst, vc);
}

template <int32_t N>
void __cpu_add_kernel(const float* a, const float* b, float* dst) {
  if constexpr (N >= 8) {
    __cpu_add_kernel<8>(a, b, dst);
    __cpu_add_kernel<N - 8>(a + 8, b + 8, dst + 8);
  } else if constexpr (N >= 4) {
    __cpu_add_kernel<4>(a, b, dst);
    __cpu_add_kernel<N - 4>(a + 4, b + 4, dst + 4);
  } else if constexpr (N >= 2) {
    __cpu_add_kernel<2>(a, b, dst);
    __cpu_add_kernel<N - 2>(a + 2, b + 2, dst + 2);
  } else {
    __cpu_add_kernel<1>(a, b, dst);
    __cpu_add_kernel<N - 1>(a + 1, b + 1, dst + 1);
  }
}

inline void add_n(const float* a, const float* b, float* c, size_t N) {
  constexpr int32_t kBlockSize = 32;
  for (int i = 0; i < N;) {
    const int32_t block_size = std::min<int32_t>(kBlockSize, N - i);
    __cpu_add_kernel<kBlockSize>(a, b, c);

    i += block_size;
    a += block_size;
    b += block_size;
    c += block_size;
  }
}

// normal
void add(const float* a, const float* b, float* c, size_t N) {
  for (int i = 0; i < N; ++i) {
    c[i] = a[i] + b[i];
  }
}

// 普通列主序 SGEMM
void sgemm(int M, int N, int K, const float* A, const float* B, float* C) {
  for (int m = 0; m < M; ++m) {
    for (int n = 0; n < N; ++n) {
      float acc = 0.0f;
      for (int k = 0; k < K; ++k) {
        acc += A[k * M + m] * B[n * K + k];  // 列主序
      }
      C[n * M + m] = acc;
    }
  }
}

int main() {
  int32_t M = 512;
  int32_t K = 128;
  constexpr int32_t N = 1024;

  {
    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C(M * N, 0.0f);

    // 填充随机数或者固定模式
    for (int i = 0; i < M * K; ++i) A[i] = i % 10 + 1;
    for (int i = 0; i < K * N; ++i) B[i] = i % 7 + 1;

    auto tp0 = std::chrono::steady_clock::now();
    sgemm(M, N, K, A.data(), B.data(), C.data());
    auto tp1 = std::chrono::steady_clock::now();

    std::cout << "sgemm v1 cost: " << (tp1 - tp0).count() / 1000 << " us" << std::endl;
  }

  float* AA = static_cast<float*>(aligned_alloc(32, sizeof(float) * N * N * K));
  float* BB = static_cast<float*>(aligned_alloc(32, sizeof(float) * N * N * K));
  // 填充随机数或者固定模式
  for (int i = 0; i < N * N * K; ++i) AA[i] = i % 10 + 1;
  for (int i = 0; i < N * N * K; ++i) BB[i] = i % 7 + 1;

  {
    float* CC = static_cast<float*>(aligned_alloc(32, sizeof(float) * N * N * K));
    {
      auto tp0 = std::chrono::steady_clock::now();
      add(AA, BB, CC, K * N * N);
      auto tp1 = std::chrono::steady_clock::now();

      std::cout << "add v1 cost: " << (tp1 - tp0).count() / 1000 << " us" << std::endl;
    }
    {
      auto tp0 = std::chrono::steady_clock::now();
      size_t s = K * N * N / 2;
      std::thread th1(add_n, AA, BB, CC, s);
      std::thread th2(add_n, AA + s, BB + s, CC + s, s);
      th1.join();
      th2.join();
      auto tp1 = std::chrono::steady_clock::now();

      std::cout << "add v2 cost: " << (tp1 - tp0).count() / 1000 << " us" << std::endl;
    }
    free(CC);
  }

  return 0;
}
