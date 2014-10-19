#ifndef TVSEG_PARAMS_H
#define TVSEG_PARAMS_H


#include "tvseg/settings/defineparams.h"


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), Dim2,
    (unsigned int, height, 0)
    (unsigned int, width, 0))


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), Dim3,
    (unsigned int, labels, 0)
    (unsigned int, height, 0)
    (unsigned int, width, 0))


#endif // TVSEG_PARAMS_H
