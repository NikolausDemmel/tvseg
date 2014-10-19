#ifndef TVSEG_PARZENSETTINGS_H
#define TVSEG_PARZENSETTINGS_H

#include "tvseg/settings/definesettings.h"
#include "tvseg/cuda/params.h"

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), ParzenSettings, "parzen", cuda::ParzenParams,
    TVSEG_CUDA_PARZEN_PARAMS_SEQ)

#endif // TVSEG_PARZENSETTINGS_H
