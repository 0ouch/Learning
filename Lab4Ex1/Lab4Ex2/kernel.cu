// ThresholdFilter.cu
#include <iostream>
#include <vector>
#include <chrono>
#include <cuda_runtime.h>

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ \
                      << " - " << cudaGetErrorString(err) << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

// CUDA-ядро: пороговая фильтрация изображения (2D-индексация)
__global__ void ThresholdKernel(const unsigned char* input, unsigned char* output,
    int width, int height, unsigned char threshold) {
    // Индекс столбца (x) и строки (y) для данного потока
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // Проверка, что пиксель в пределах изображения
    if (x < width && y < height) {
        int idx = y * width + x;
        unsigned char pixel = input[idx];
        output[idx] = (pixel > threshold) ? 255 : 0;
    }
}

// CPU-версия для сравнения
void ThresholdCPU(const unsigned char* input, unsigned char* output,
    int width, int height, unsigned char threshold) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            output[idx] = (input[idx] > threshold) ? 255 : 0;
        }
    }
}

int main() {
    // ===== ПАРАМЕТРЫ =====
    constexpr int W = 1024, H = 1024;
    constexpr unsigned char T = 128;
    constexpr size_t total_pixels = W * H;
    constexpr size_t bytes = total_pixels * sizeof(unsigned char);

    // ===== ПОДГОТОВКА ДАННЫХ =====
    std::vector<unsigned char> h_input(total_pixels);
    std::vector<unsigned char> h_output_cpu(total_pixels);
    std::vector<unsigned char> h_output_gpu(total_pixels);

    // Заполняем случайными значениями 0..255
    for (size_t i = 0; i < total_pixels; ++i) {
        h_input[i] = static_cast<unsigned char>(std::rand() % 256);
    }

    // ===== CPU-ВЕРСИЯ =====
    std::cout << "Starting CPU version..." << std::endl;
    auto cpu_start = std::chrono::high_resolution_clock::now();
    ThresholdCPU(h_input.data(), h_output_cpu.data(), W, H, T);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_time = cpu_end - cpu_start;
    std::cout << "CPU time: " << cpu_time.count() << " s" << std::endl;

    // ===== GPU (CUDA)-ВЕРСИЯ =====
    unsigned char* d_input = nullptr, * d_output = nullptr;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_input), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_output), bytes));
    CUDA_CHECK(cudaMemcpy(d_input, h_input.data(), bytes, cudaMemcpyHostToDevice));

    // Конфигурация двумерной сетки и блоков
    dim3 threadsPerBlock(16, 16);   // 16x16 = 256 потоков на блок
    dim3 blocksPerGrid(
        (W + threadsPerBlock.x - 1) / threadsPerBlock.x,
        (H + threadsPerBlock.y - 1) / threadsPerBlock.y
    );

    std::cout << "Starting GPU version..." << std::endl;
    std::cout << "Grid: " << blocksPerGrid.x << " x " << blocksPerGrid.y
        << " blocks, block: " << threadsPerBlock.x << " x "
        << threadsPerBlock.y << " threads" << std::endl;

    auto gpu_start = std::chrono::high_resolution_clock::now();
    ThresholdKernel << <blocksPerGrid, threadsPerBlock >> > (d_input, d_output, W, H, T);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    auto gpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gpu_time = gpu_end - gpu_start;
    std::cout << "GPU time: " << gpu_time.count() << " s" << std::endl;

    // Копируем результат обратно
    CUDA_CHECK(cudaMemcpy(h_output_gpu.data(), d_output, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_input));
    CUDA_CHECK(cudaFree(d_output));

    // ===== ПРОВЕРКА =====
    std::cout << "\nVerifying results..." << std::endl;
    bool passed = true;
    for (size_t i = 0; i < total_pixels; ++i) {
        if (h_output_cpu[i] != h_output_gpu[i]) {
            std::cout << "Mismatch at pixel " << i << ": CPU="
                << static_cast<int>(h_output_cpu[i])
                << " GPU=" << static_cast<int>(h_output_gpu[i]) << std::endl;
            passed = false;
            break;
        }
    }
    std::cout << "Verification: " << (passed ? "PASSED" : "FAILED") << std::endl;

    // ===== СРАВНЕНИЕ СКОРОСТИ =====
    std::cout << "\nSpeed comparison:" << std::endl;
    std::cout << "  CPU: " << cpu_time.count() << " s" << std::endl;
    std::cout << "  GPU: " << gpu_time.count() << " s" << std::endl;
    if (gpu_time.count() > 0) {
        std::cout << "  GPU is " << cpu_time.count() / gpu_time.count()
            << "x faster than CPU" << std::endl;
    }

    return 0;
}