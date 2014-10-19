#ifndef TVSEG_WEIGHTSETTINGS_H
#define TVSEG_WEIGHTSETTINGS_H

#include "tvseg/settings/defineparams.h"
#include "tvseg/settings/definesettings.h"


#define TVSEG_WEIGHT_SETTINGS_SEQ       \
    (float, smoothingBandwidth, 0.8)    \
    (float, beta, 1.0)                  \
    (float, betaDepth, 1.0)             \
    (bool, useDepth, true)              \
    (bool, useColor, true)              \
    (bool, normalizeDepth, true)        \
    (int, variant, 1)                   \
    (bool, autoRecompute, false)


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), WeightParams,
    TVSEG_WEIGHT_SETTINGS_SEQ)

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), WeightSettings, "weight", WeightParams,
    TVSEG_WEIGHT_SETTINGS_SEQ)

#endif // TVSEG_WEIGHTSETTINGS_H
