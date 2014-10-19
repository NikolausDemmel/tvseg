#ifndef TVSEG_EDGEDETECTIONBASE_H
#define TVSEG_EDGEDETECTIONBASE_H

#include "edgedetection.h"

namespace tvseg {

class EdgeDetectionBase : public EdgeDetection
{
public:
    EdgeDetectionBase();

    // EdgeDetection interface
public:
    cv::Mat edges() const;
    bool edgesAvailable() const;

protected:
    cv::Mat gradient_;
};

} // namespace tvseg

#endif // TVSEG_EDGEDETECTIONBASE_H
