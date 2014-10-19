#include "tvweightbase.h"

namespace tvseg {

TVWeightBase::TVWeightBase()
{
}

void TVWeightBase::clear()
{
    weight_ = cv::Mat();
}

cv::Mat TVWeightBase::weight() const
{
    return weight_;
}

bool TVWeightBase::weightAvailable() const
{
    return !weight_.empty();
}

} // namespace tvseg
