//Умножение матрицы на число с ИСКУССТВЕННО УСЛОЖНЁННЫМ ВЫЧИСЛЕНИЕМ
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cuda_runtime.h>

__global__ void ScaleVectorGPU(float* A, float* B, int N, float k) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int i = idx; i < N; i += stride) {
        float val = A[i];
        // Тысяча лишних умножений — имитация сложной вычислительной задачи
        for (int j = 0; j < 1000; ++j)
            val = val * k;
        B[i] = val;
    }
}

void ScaleVectorCPU(const float* A, float* B, int N, float k) {
    for (int i = 0; i < N; ++i) {
        float val = A[i];
        for (int j = 0; j < 1000; ++j)
            val = val * k;
        B[i] = val;
    }
}

int main() {
    int N = 1'000'000;
    float k = 2.5f;
    size_t bytes = static_cast<size_t>(N) * sizeof(float);

    std::vector<float> default_arr(N), arr_CPU(N), arr_GPU(N);
    for (int i = 0; i < N; ++i)
        default_arr[i] = static_cast<float>(i);

    // CPU
    auto cpu_start = std::chrono::high_resolution_clock::now();
    ScaleVectorCPU(default_arr.data(), arr_CPU.data(), N, k);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_time = cpu_end - cpu_start;
    std::cout << "CPU time: " << cpu_time.count() << " s" << std::endl;

    // GPU
    float* GPU_def_arr = nullptr, * GPU_new_arr = nullptr;
    cudaMalloc(reinterpret_cast<void**>(&GPU_def_arr), bytes);
    cudaMalloc(reinterpret_cast<void**>(&GPU_new_arr), bytes);
    cudaMemcpy(GPU_def_arr, default_arr.data(), bytes, cudaMemcpyHostToDevice);

    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    auto gpu_start = std::chrono::high_resolution_clock::now();
    ScaleVectorGPU << <blocksPerGrid, threadsPerBlock >> > (GPU_def_arr, GPU_new_arr, N, k);
    cudaDeviceSynchronize();
    auto gpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gpu_time = gpu_end - gpu_start;
    std::cout << "GPU time: " << gpu_time.count() << " s" << std::endl;

    cudaMemcpy(arr_GPU.data(), GPU_new_arr, bytes, cudaMemcpyDeviceToHost);
    cudaFree(GPU_def_arr);
    cudaFree(GPU_new_arr);

    std::cout << "\nSpeed comparison:" << std::endl;
    std::cout << "  CPU: " << cpu_time.count() << " s" << std::endl;
    std::cout << "  GPU: " << gpu_time.count() << " s" << std::endl;
   
        std::cout << "  GPU is " << cpu_time.count() / gpu_time.count()
        << "x faster than CPU" << std::endl;

    return 0;
}