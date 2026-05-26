// VectorScalingCUDA.cu
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cuda_runtime.h>

// Макрос для проверки ошибок CUDA
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ \
                      << " - " << cudaGetErrorString(err) << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

__global__ void ScaleVectorKernel(const float* A, float* B, int N, float k) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = idx; i < N; i += stride) {
        B[i] = A[i] * k;
    }
}

void ScaleVectorCPU(const float* A, float* B, int N, float k) {
    for (int i = 0; i < N; ++i) {
        B[i] = A[i] * k;
    }
}

int main() {
    const int N = 1'000'000;
    const float k = 2.5f;
    const size_t bytes = static_cast<size_t>(N) * sizeof(float);

    std::vector<float> h_A(N), h_B_cpu(N), h_B_gpu(N);
    for (int i = 0; i < N; ++i) {
        h_A[i] = static_cast<float>(i);
    }

    // CPU 
    std::cout << "Starting CPU version..." << std::endl;
    auto cpu_start = std::chrono::high_resolution_clock::now();
    ScaleVectorCPU(h_A.data(), h_B_cpu.data(), N, k);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_time = cpu_end - cpu_start;
    std::cout << "CPU time: " << cpu_time.count() << " s" << std::endl;

    // GPU 
    float* d_A = nullptr, * d_B = nullptr;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_A), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_B), bytes));
    CUDA_CHECK(cudaMemcpy(d_A, h_A.data(), bytes, cudaMemcpyHostToDevice));

    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    std::cout << "Starting GPU (CUDA) version..." << std::endl;
    std::cout << "Kernel: " << blocksPerGrid << " blocks x "
        << threadsPerBlock << " threads" << std::endl;

    auto gpu_start = std::chrono::high_resolution_clock::now();
    ScaleVectorKernel << <blocksPerGrid, threadsPerBlock >> > (d_A, d_B, N, k);
    // Проверяем запуск ядра
    CUDA_CHECK(cudaGetLastError());
    // Ждём завершения и проверяем
    CUDA_CHECK(cudaDeviceSynchronize());
    auto gpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gpu_time = gpu_end - gpu_start;
    std::cout << "GPU time: " << gpu_time.count() << " s" << std::endl;

    CUDA_CHECK(cudaMemcpy(h_B_gpu.data(), d_B, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_A));
    CUDA_CHECK(cudaFree(d_B));

    //  Проверка 
    std::cout << "\nVerifying results..." << std::endl;
    bool passed = true;
    for (int i = 0; i < N; ++i) {
        if (std::fabs(h_B_cpu[i] - h_B_gpu[i]) > 1e-5f) {
            std::cout << "Mismatch at index " << i << ": CPU="
                << h_B_cpu[i] << " GPU=" << h_B_gpu[i] << std::endl;
            passed = false;
            break;
        }
    }

    if (passed) {
        std::cout << "Verification PASSED!" << std::endl;
    }
    else {
        std::cout << "Verification FAILED!" << std::endl;
    }

    std::cout << "\nSpeed comparison:" << std::endl;
    std::cout << "  CPU: " << cpu_time.count() << " s" << std::endl;
    std::cout << "  GPU: " << gpu_time.count() << " s" << std::endl;
    if (gpu_time.count() > 0) {
        std::cout << "  GPU is " << cpu_time.count() / gpu_time.count()
            << "x faster than CPU" << std::endl;
    }

    return 0;
}