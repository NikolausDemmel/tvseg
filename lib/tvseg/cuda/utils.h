#ifndef TVSEG_CUDA_UTILS_H
#define TVSEG_CUDA_UTILS_H

#include "tvseg/params.h"

#include <thrust/device_ptr.h>
#include <thrust/fill.h>


// TODO: use logging facility. Refactor thrust error handling.


namespace tvseg {
namespace cuda {


    template <class T>
    inline void fillDeviceArray2DAsync(T *gpu_array, size_t pitchBytes, size_t height, T value)
    {
        try {
            thrust::device_ptr<T> ptr(gpu_array);
            thrust::fill(ptr, ptr + (pitchBytes/sizeof(T) * height), value);
        } catch(thrust::system_error &e) {
            std::cerr << "fillDeviceArray2DAsync failed with thrust::system_error: " << e.what() << std::endl;
            throw;
        }
    }

    inline Dim2 dim2(const Dim3 &dim) {
        Dim2 result;
        result.width = dim.width;
        result.height = dim.height;
        return result;
    }


}
}

#endif // TVSEG_CUDA_UTILS_H
