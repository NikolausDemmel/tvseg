#ifndef TVSEG_EDGEDETECTION_H
#define TVSEG_EDGEDETECTION_H

#include "tvseg/util/includeopencv.h"
#include <boost/shared_ptr.hpp>

namespace tvseg {

class EdgeDetection
{
public:
    virtual ~EdgeDetection() {}

    virtual void computeEdges(cv::Mat image, float smoothingBandwidth) = 0;
    virtual cv::Mat edges() const = 0;
    virtual bool edgesAvailable() const = 0;
};


typedef boost::shared_ptr<EdgeDetection> EdgeDetectionPtr;
typedef boost::shared_ptr<const EdgeDetection> EdgeDetectionConstPtr;


} // namespace tvseg

#endif // TVSEG_EDGEDETECTION_H
