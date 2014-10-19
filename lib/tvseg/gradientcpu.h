#ifndef TVSEG_GRADIENTCPU_H
#define TVSEG_GRADIENTCPU_H

#include "edgedetectionbase.h"

namespace tvseg {

class GradientCPU : public EdgeDetectionBase
{
public:
    GradientCPU();

    void computeEdges(cv::Mat image, float smoothingBandwidth);
};

} // namespace tvseg

#endif // TVSEG_GRADIENTCPU_H
