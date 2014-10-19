#include "tvvisualizerbase.h"

namespace tvseg {

void TVVisualizerBase::clear()
{
    visualization_ = cv::Mat();
}

bool TVVisualizerBase::visualizationAvailable() const
{
    return !visualization_.empty();
}

cv::Mat TVVisualizerBase::visualization() const
{
    return visualization_;
}


} // namespace tvseg
