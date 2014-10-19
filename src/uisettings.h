#ifndef TVSEG_UI_UISETTINGS_H
#define TVSEG_UI_UISETTINGS_H


#include "tvseg/settings/defineparams.h"
#include "tvseg/settings/definesettings.h"


#define TVSEG_UI_UI_SETTINGS_SEQ          \
    (float, scribbleBrushDensity, 0.5)    \
    (unsigned int, scribbleBrushSize, 5)  \
    (bool, saveScribbles, true)           \
    (bool, autoRun, false)


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg_ui), UiParams,
    TVSEG_UI_UI_SETTINGS_SEQ)

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg_ui), UiSettings, "ui", UiParams,
    TVSEG_UI_UI_SETTINGS_SEQ)


#endif // TVSEG_UI_UISETTINGS_H
