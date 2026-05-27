//Умножение матрицы на число. Выберем число потоков = 256
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cuda_runtime.h>

__global__ void ScaleVectorGPU(float* A, float* B, int N, float k) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;    //шаг - сколько всего потоков запущено
    for (int i = idx; i < N; i += stride) { //число элементов больше числа потоков. каждый поток берет N/stride элементов
        B[i] = A[i] * k;
    }
}

void ScaleVectorCPU(const float* A, float* B, int N, float k) {
    for (int i = 0; i < N; ++i) {
        B[i] = A[i] * k;
    }
}

int main() {
    int N = 1'000'000;
    float k = 2.5f;
    size_t bytes = static_cast<size_t>(N) * sizeof(float);  //выделение памяти в gpu

    std::vector<float> default_arr(N), arr_CPU(N), arr_GPU(N);
    for (int i = 0; i < N; ++i) {
        default_arr[i] = static_cast<float>(i); //заполняем исходный флоутами
    }

    // CPU 
    auto cpu_start = std::chrono::high_resolution_clock::now(); //засекли время
    ScaleVectorCPU(default_arr.data(), arr_CPU.data(), N, k);
    auto cpu_end = std::chrono::high_resolution_clock::now();   //стоп часы
    std::chrono::duration<double> cpu_time = cpu_end - cpu_start;   //посчитали
    std::cout << "CPU time: " << cpu_time.count() << " s" << std::endl; //напечатали

    // GPU 
    float* GPU_def_arr = nullptr, * GPU_new_arr = nullptr;
    cudaMalloc(reinterpret_cast<void**>(&GPU_def_arr), bytes);  //выделяем память, нужен тип void**
    cudaMalloc(reinterpret_cast<void**>(&GPU_new_arr), bytes);
    cudaMemcpy(GPU_def_arr, default_arr.data(), bytes, cudaMemcpyHostToDevice); //копируем исходный массив на GPU

    int threadsPerBlock = 256;  //число потоков
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;    //число блоков

    std::cout  << blocksPerGrid << " blocks x "
        << threadsPerBlock << " threads" << std::endl;

    auto gpu_start = std::chrono::high_resolution_clock::now(); //часы
    ScaleVectorGPU << <blocksPerGrid, threadsPerBlock >> > (GPU_def_arr, GPU_new_arr, N, k);
    cudaDeviceSynchronize();
    auto gpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gpu_time = gpu_end - gpu_start;
    std::cout << "GPU time: " << gpu_time.count() << " s" << std::endl;

    cudaMemcpy(arr_GPU.data(), GPU_new_arr, bytes, cudaMemcpyDeviceToHost); //обратно в память CPU
    cudaFree(GPU_def_arr);
    cudaFree(GPU_new_arr);

    std::cout << "\nSpeed comparison:" << std::endl;
    std::cout << "  CPU: " << cpu_time.count() << " s" << std::endl;
    std::cout << "  GPU: " << gpu_time.count() << " s" << std::endl;
   
        std::cout << "  GPU is " << cpu_time.count() / gpu_time.count() //маленькая операция, процессор быстрее
            << "x faster than CPU" << std::endl;
    

    return 0;
}
