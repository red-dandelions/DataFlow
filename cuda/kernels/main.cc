#include <iostream>
#include <vector>
#include <cmath>

// 声明外部 CUDA 函数
extern "C" void vector_add(const float* A, const float* B, float* C, int N);

int main() {
    int N = 1 << 20;  // 1M elements
    std::vector<float> A(N), B(N), C(N);

    for (int i = 0; i < N; i++) {
        A[i] = sin(i) * sin(i);
        B[i] = cos(i) * cos(i);
    }

    vector_add(A.data(), B.data(), C.data(), N);

    float max_err = 0.0f;
    for (int i = 0; i < N; i++) {
        max_err = std::max(max_err, std::abs(C[i] - 1.0f));
    }
    std::cout << "Max error: " << max_err << std::endl;

    return 0;
}
