#ifndef TVSEG_CUDA_COLORDATATERM_H
#define TVSEG_CUDA_COLORDATATERM_H


#include "tvseg/cuda/params.h"
#include "tvseg/params.h"


namespace tvseg {
namespace cuda {


void computeColorDataterm(
        float* dataterm,
        const float *image,
        const float *colors,
        const Dim3 &dim
        );


}
}


#endif //TVSEG_CUDA_COLORDATATERM_H
