#include "tvdatatermbase.h"

namespace tvseg {

void TVDatatermBase::clear()
{
    dataterm_ = cv::Mat();
}

bool TVDatatermBase::datatermAvailable() const
{
    return !dataterm_.empty();
}

cv::Mat TVDatatermBase::dataterm() const
{
    return dataterm_;
}


} // namespace tvseg
