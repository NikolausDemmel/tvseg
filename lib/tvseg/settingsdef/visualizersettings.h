#ifndef TVSEG_VISUALIZERSETTINGS_H
#define TVSEG_VISUALIZERSETTINGS_H


#include "tvseg/settings/defineparams.h"
#include "tvseg/settings/definesettings.h"


#define TVSEG_VISUALIZER_SETTINGS_SEQ   \
    (bool, showImage, true)             \
    (float, imageTransparancy, 0.5)     \
    (bool, showGroundTruth, true)       \
    (bool, showBorder, true)            \
    (bool, autoRecompute, false)        \


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), VisualizerParams,
    TVSEG_VISUALIZER_SETTINGS_SEQ)

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), VisualizerSettings, "visualizer", VisualizerParams,
    TVSEG_VISUALIZER_SETTINGS_SEQ)


#endif // TVSEG_VISUALIZERSETTINGS_H
