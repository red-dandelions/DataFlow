#include <chrono>
#include <iostream>
#include <random>
#include <vector>

void sgemm_v1(int32_t M, int32_t N, int32_t K, const float* A, const float* B, float* C) {
  for (int32_t row = 0; row < M; ++row) {
    for (int32_t col = 0; col < N; ++col) {
      float v = 0;
      for (int32_t k = 0; k < K; ++k) {
        v += A[row * K + k] * B[k * N + col];
      }
      C[row * N + col] = v;
    }
  }
}

int main() {
  constexpr int32_t M = 512;
  constexpr int32_t K = 128;
  constexpr int32_t N = 1024;
  constexpr int32_t A_TATAL = M * K;
  constexpr int32_t B_TATAL = K * N;
  constexpr int32_t C_TATAL = M * N;

  std::vector<float> A(M * K);
  std::vector<float> B(K * N);
  std::vector<float> C(M * N, 0);
  // 随机数引擎
  std::random_device rd;                                   // 真随机种子
  std::mt19937 gen(rd());                                  // Mersenne Twister 引擎
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);  // [0,1] 区间

  const auto random_init = [&gen, &dist](float* arr, int32_t size) -> void {
    for (int32_t i = 0; i < size; ++i) {
      arr[i] = dist(gen);
    }
  };

  random_init(A.data(), A_TATAL);
  random_init(B.data(), B_TATAL);

  {
    auto tp0 = std::chrono::steady_clock::now();
    sgemm_v1(M, N, K, A.data(), B.data(), C.data());
    auto tp1 = std::chrono::steady_clock::now();
    std::cout << "sgemm_v1 cost: " << (tp1 - tp0).count() / 1000 << " us\n";
  }

  return 0;
}