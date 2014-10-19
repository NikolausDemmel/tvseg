#ifndef TVSEG_CUDA_PARAMS_H
#define TVSEG_CUDA_PARAMS_H

#include "tvseg/settings/defineparams.h"


namespace tvseg {
namespace cuda {

typedef unsigned int uint;

} // namespace cuda
} // namespace tvseg


#define TVSEG_CUDA_SOLVER_PARAMS_SEQ    \
    (float, lambda, 1.0f)               \
    (uint, maxSteps, 1000)              \
    (uint, minSteps,  100)              \
    (float, epsilon, 1e-5f)             \
    (uint, checkConverganceEvery, 20)   \
    (bool, displayIteration, true)      \
    (uint, displayIterationEvery, 50)   \
    (bool, autoRecompute, false)

#define TVSEG_CUDA_SOLVER_OUTPUT_SEQ    \
    (uint, steps, 0)                    \
    (bool, cancelled, false)            \
    (float, time, 0.0f)                 \
    (float, energyGap, 0.0f)

#define TVSEG_CUDA_PARZEN_PARAMS_SEQ    \
    (float, colorVariance, 0.02f)        \
    (float, scribbleDistanceFactor, 1000)\
    (float, colorScale, 1.0)            \
    (float, depthScale, 1.0)            \
    (float, depthVariance, 1.0f)        \
    (bool, normalizeSpaceScale, true)   \
    (bool, scribbleDistanceDepth, true) \
    (bool, scribbleDistancePerspective, false) \
    (bool, useColorKernel, true)        \
    (bool, useDistanceKernel, true)     \
    (bool, useDepthKernel, false)       \
    (bool, fixScribblePixels, true)     \
    (bool, fixScribblePixelsThisOnly, false) \
    (bool, fixScribblePixelsOtherLabelUseMin, false) \
    (bool, useSpatiallyActiveScribbles, false) \
    (float, spatialActivationFactor, 3.0) \
    (float, activeScribbleLambda, 0.8)


#define TVSEG_CUDA_INTRINSICS_PARAMS_SEQ \
    (float, s_x, 1.0f)                  \
    (float, s_y, 1.0f)                  \
    (float, o_x, 0.0f)                  \
    (float, o_y, 1.0f)


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg)(cuda), SolverParams,
    TVSEG_CUDA_SOLVER_PARAMS_SEQ)

TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg)(cuda), SolverOutput,
    TVSEG_CUDA_SOLVER_OUTPUT_SEQ)

TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg)(cuda), ParzenParams,
    TVSEG_CUDA_PARZEN_PARAMS_SEQ)

TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg)(cuda), Intrinsics,
    TVSEG_CUDA_INTRINSICS_PARAMS_SEQ)



#endif // TVSEG_CUDA_PARAMS_H
