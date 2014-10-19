#ifndef TVSEG_CUDA_PARZENDATATERM_H
#define TVSEG_CUDA_PARZENDATATERM_H


#include "tvseg/cuda/params.h"
#include "tvseg/params.h"
#include "tvseg/feedback.h"


namespace tvseg {
namespace cuda {


void computeParzenDataterm(
        float* dataterm,
        const float *color,
        const float *depth,
        const int *scribbles,
        const int *numScribbles,
        const Intrinsics &intr,
        const Dim3 &dim,
        const ParzenParams &params,
        const Feedback *feedback
        );


}
}


#endif // TVSEG_CUDA_PARZENDATATERM_H
