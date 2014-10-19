#ifndef TVSEG_CUDA_SOLVER_H
#define TVSEG_CUDA_SOLVER_H

#include "tvseg/cuda/params.h"
#include "tvseg/params.h"
#include "tvseg/feedback.h"


namespace tvseg {
namespace cuda {


void computeSolution(
        unsigned char *segmentation,
        float *u,
        const float *dataterm,
        const float *weight,
        const Dim3 &size,
        const SolverParams &params,
        const Feedback *feedback,
        SolverOutput &output
        );


}
}

#endif // TVSEG_CUDA_SOLVER_H
