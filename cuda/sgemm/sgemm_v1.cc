#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

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

// 打印矩阵（列主序）
void print_matrix(const float* mat, int rows, int cols) {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      std::cout << std::setw(6) << mat[j * rows + i] << " ";
    }
    std::cout << "\n";
  }
}

int main() {
  int32_t M = 512;
  int32_t K = 256;
  int32_t N = 1024;

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

  // std::cout << "Matrix C = A*B:\n";
  // print_matrix(C.data(), M, N);

  return 0;
}
