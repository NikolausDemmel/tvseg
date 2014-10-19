#ifndef TVSEG_TVWEIGHT_H
#define TVSEG_TVWEIGHT_H

#include "tvseg/util/includeopencv.h"
#include "tvseg/settings/settings.h"
#include <boost/shared_ptr.hpp>

namespace tvseg {

class TVWeight
{
public:
    virtual ~TVWeight() {}

    virtual void computeWeight(cv::Mat color, cv::Mat depth) = 0;
    virtual void clear() = 0;
    virtual cv::Mat weight() const = 0;
    virtual bool weightAvailable() const = 0;

    virtual settings::SettingsPtr settings() const = 0;
};


typedef boost::shared_ptr<TVWeight> TVWeightPtr;
typedef boost::shared_ptr<const TVWeight> TVWeightConstPtr;


} // namespace tvseg

#endif // TVSEG_TVWEIGHT_H
