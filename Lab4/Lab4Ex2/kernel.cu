// Пороговая фильтрация изображения 1024x1024. Блоки 16x16
#include <iostream>
#include <vector>
#include <chrono>
#include <cuda_runtime.h>

__global__ void ThresholdGPU(unsigned char* input, unsigned char* output,
    int width, int height, unsigned char T) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;  //определяем координату пикселя
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int idx = y * width + x;   //"одномерный" пиксель
    output[idx] = (input[idx] > T) ? 255 : 0;   //(условие) ? значение_если_да : значение_если_нет
}

void ThresholdCPU(const unsigned char* input, unsigned char* output,
    int W, int H, unsigned char T) {
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int idx = y * W + x;
            output[idx] = (input[idx] > T) ? 255 : 0;
        }
    }
}

int main() {
    int width = 1024, height = 1024;
    unsigned char T = 128;
    size_t total = width * height;
    size_t bytes = total * sizeof(unsigned char);

    std::vector<unsigned char> input(total), out_cpu(total), out_gpu(total);
    for (size_t i = 0; i < total; ++i)
        input[i] = static_cast<unsigned char>(std::rand() % 256);   //случайные пиксели

    // CPU
    auto cpu_start = std::chrono::high_resolution_clock::now(); //часы
    ThresholdCPU(input.data(), out_cpu.data(), width, height, T);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_time = cpu_end - cpu_start;
    std::cout << "CPU time: " << cpu_time.count() << " s" << std::endl;

    // GPU
    unsigned char* d_input = nullptr, * d_output = nullptr; //выделяем память
    cudaMalloc(reinterpret_cast<void**>(&d_input), bytes);
    cudaMalloc(reinterpret_cast<void**>(&d_output), bytes);
    cudaMemcpy(d_input, input.data(), bytes, cudaMemcpyHostToDevice);

    //dim3 - Это специальный тип CUDA, который хранит три целых числа: .x, .y, .z
    dim3 threadsPerBlock(16, 16);   //один блок 256=16*16
    dim3 blocksPerGrid(width / 16, height / 16);  // вычисляем кол-во блоков 1024=64*16

    auto gpu_start = std::chrono::high_resolution_clock::now();     //гпу часы
    ThresholdGPU << <blocksPerGrid, threadsPerBlock >> > (d_input, d_output, width, height, T);
    cudaDeviceSynchronize();
    auto gpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gpu_time = gpu_end - gpu_start;
    std::cout << "GPU time: " << gpu_time.count() << " s" << std::endl;

    cudaMemcpy(out_gpu.data(), d_output, bytes, cudaMemcpyDeviceToHost);
    cudaFree(d_input);
    cudaFree(d_output);

    std::cout << "\nSpeed comparison:" << std::endl;
    std::cout << "  CPU: " << cpu_time.count() << " s" << std::endl;
    std::cout << "  GPU: " << gpu_time.count() << " s" << std::endl;

        std::cout << "  GPU is " << cpu_time.count() / gpu_time.count()
        << "x faster than CPU" << std::endl;

    return 0;
}