#include "edgedetectionbase.h"

namespace tvseg {

EdgeDetectionBase::EdgeDetectionBase()
{
}

cv::Mat EdgeDetectionBase::edges() const
{
    return gradient_;
}

bool EdgeDetectionBase::edgesAvailable() const
{
    return !gradient_.empty();
}

} // namespace tvseg
