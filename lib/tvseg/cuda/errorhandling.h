// LICENSE INFORMATION:
// This software contains source code provided by NVIDIA Corporation.
// Parts of this file are based on the NVIDIA CUDA Samples.
// See the following EULA for the NVIDIA CUDA Samples:
// http://developer.download.nvidia.com/compute/cuda/5_5/rel/docs/EULA.pdf


#ifndef TVSEG_CUDA_ERRORHANDLING_H
#define TVSEG_CUDA_ERRORHANDLING_H

#include "tvseg/util/errorhandling.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#include "cuda_samples/common/inc/helper_cuda.h"
#pragma clang diagnostic pop




namespace tvseg {
namespace cuda {
namespace _detail {

template< typename T >
inline void check(const T result, const char *const message, char const *const func, const char *const file, int const line)
{
    if (result) {
        std::stringstream ss;
        ss << "CUDA error: ";
        if (message) ss << message << " ";
        ss << "code=" << result << "(" << _cudaGetErrorEnum(result) << ")";
        tvseg::Exception e(ss.str(), func, file, line);
        std::cerr << e << std::endl;
        cudaDeviceReset();
        throw e;
    }
}

} // namespace _detail
} // namespace cuda
} // namespace tvseg



#define CUDA_CHECK(S) CUDA_CHECK_MESSAGE(S, "")


#define CUDA_CHECK_MESSAGE(S, MESSAGE)                                          \
    tvseg::cuda::_detail::check(S, MESSAGE, __func__, __FILE__, __LINE__)


#define CUDA_CHECK_AND_SYNCHRONIZE(S)                                           \
    do {                                                                        \
        CUDA_CHECK_MESSAGE(S, "before DeviceSynchronize");                      \
        CUDA_CHECK_MESSAGE(cudaDeviceSynchronize(), "after DeviceSynchronize"); \
    } while (false)


#define CUDA_CHECK_KERNEL_LAUNCH()                                              \
    CUDA_CHECK_MESSAGE(cudaGetLastError(), "KernelLaunch")


#define CUDA_CHECK_KERNEL_LAUNCH_AND_SYNCHRONIZE()                              \
    do {                                                                        \
        CUDA_CHECK_MESSAGE(cudaGetLastError(),                                  \
                           "KernelLaunch before DeviceSynchronize");            \
        CUDA_CHECK_MESSAGE(cudaDeviceSynchronize(),                             \
                           "KernelLaunch after DeviceSynchronize");             \
    } while (false)


#ifdef _DEBUG
# define CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE CUDA_CHECK_KERNEL_LAUNCH_AND_SYNCHRONIZE
#else
# define CUDA_CHECK_KERNEL_LAUNCH_AND_DEBUG_SYNCHRONIZE CUDA_CHECK_KERNEL_LAUNCH
#endif


#endif // TVSEG_CUDA_ERRORHANDLING_H
