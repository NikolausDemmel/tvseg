
#include "colordataterm.h"
#include "errorhandling.h"
#include "config.h"

namespace tvseg {
namespace cuda {


__global__ void kernel_colorDataterm(
        float* dataterm,
        const float *image,
        const float *colors,
        const uint pitchDataterm,
        const uint pitchImage,
        const Dim3 dim)
{

    const uint x = blockDim.x * blockIdx.x + threadIdx.x;
    const uint y = blockDim.y * blockIdx.y + threadIdx.y;

    if (x >= dim.width || y >= dim.height)
        return;

    const uint datatermImageSize = dim.height * pitchDataterm;

    for (uint i = 0; i < dim.labels; ++i) {
        const float r = image[3*x + y*pitchImage    ] - colors[3*i    ];
        const float g = image[3*x + y*pitchImage + 1] - colors[3*i + 1];
        const float b = image[3*x + y*pitchImage + 2] - colors[3*i + 2];

        dataterm[x + y*pitchDataterm + i*datatermImageSize] = sqrtf(r*r + g*g + b*b);
    }
}

void computeColorDataterm(
        float* dataterm,
        const float *image,
        const float *colors,
        const Dim3 &dim)
{
    // Prepare grid size
    dim3 dimBlock(BLOCKDIMX, BLOCKDIMY);
    dim3 dimGrid;
    size_t pitchDataterm;
    size_t pitchImage;
    size_t pitchColors;
    dimGrid.x = (dim.width % dimBlock.x) ? (dim.width/dimBlock.x + 1) : (dim.width/dimBlock.x);
    dimGrid.y = (dim.height % dimBlock.y) ? (dim.height/dimBlock.y + 1) : (dim.height/dimBlock.y);

    // allocate memory on the GPU and copy data or set initial value
    float *gpu_image = NULL;
    float *gpu_dataterm = NULL;
    float *gpu_colors = NULL;

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_dataterm, &pitchDataterm, dim.width * sizeof(float), dim.height * dim.labels));
    CUDA_CHECK(cudaMemset2DAsync(gpu_dataterm, pitchDataterm, 0, dim.width * sizeof(float), dim.height * dim.labels));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_image, &pitchImage, 3*dim.width * sizeof(float), dim.height));
    CUDA_CHECK(cudaMemcpy2DAsync(gpu_image, pitchImage, image, 3*dim.width * sizeof(float), 3*dim.width * sizeof(float), dim.height, cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMallocPitch((void**)&gpu_colors, &pitchColors, 3*dim.labels * sizeof(float), 1));
    CUDA_CHECK(cudaMemcpy2DAsync(gpu_colors, pitchColors, colors, 3*dim.labels * sizeof(float), 3*dim.labels * sizeof(float), 1, cudaMemcpyHostToDevice));

    // compute dataterm on GPU
    kernel_colorDataterm<<< dimGrid, dimBlock >>>(gpu_dataterm, gpu_image, gpu_colors,
                                                  pitchDataterm/sizeof(float), pitchImage/sizeof(float),
                                                  dim);
    CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE();

    // copy result back to CPU
    CUDA_CHECK(cudaMemcpy2D((void*)dataterm, dim.width * sizeof(float), gpu_dataterm, pitchDataterm, dim.width * sizeof(float), dim.height * dim.labels, cudaMemcpyDeviceToHost));

    // delete all data on GPU
    CUDA_CHECK(cudaFree(gpu_image));
    CUDA_CHECK(cudaFree(gpu_dataterm));
    CUDA_CHECK(cudaFree(gpu_colors));
}


}
}
