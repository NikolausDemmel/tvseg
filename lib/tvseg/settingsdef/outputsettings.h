#ifndef TVSEG_OUTPUTSETTINGS_H
#define TVSEG_OUTPUTSETTINGS_H

#include "tvseg/settings/defineparams.h"
#include "tvseg/settings/definesettings.h"


#define TVSEG_OUTPUT_SETTINGS_SEQ                               \
    (std::string, resultPath, "results")                        \
    (bool, subFolderByDate, true)                               \
    (bool, prefixDateTime, true)                                \
    (std::string, weightReplace, "_weight_normalized\\1")       \
    (std::string, datatermReplace, "_dataterm_normalized\\1")   \
    (std::string, solutionReplace, "_solution\\1")              \
    (std::string, visualizationReplace, "_visualization\\1")    \
    (std::string, metricsReplace, "_metrics.txt")



TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), OutputParams,
    TVSEG_OUTPUT_SETTINGS_SEQ)

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), OutputSettings, "output", OutputParams,
    TVSEG_OUTPUT_SETTINGS_SEQ)

#endif // TVSEG_OUTPUTSETTINGS_H
