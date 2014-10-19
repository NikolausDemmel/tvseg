#ifndef TVSEG_SOLVERSETTINGS_H
#define TVSEG_SOLVERSETTINGS_H

#include "tvseg/settings/definesettings.h"
#include "tvseg/cuda/params.h"

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), SolverSettings, "solver", cuda::SolverParams,
    TVSEG_CUDA_SOLVER_PARAMS_SEQ)

#endif // TVSEG_SOLVERSETTINGS_H
