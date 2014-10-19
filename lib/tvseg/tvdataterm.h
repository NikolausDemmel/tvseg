#ifndef TVSEG_TVDATATERM_H
#define TVSEG_TVDATATERM_H

#include "tvseg/util/includeopencv.h"
#include "tvseg/settings/settings.h"
#include "tvseg/feedback.h"
#include "tvseg/scribbles.h"
#include <boost/shared_ptr.hpp>

namespace tvseg {

class TVDataterm
{
public:
    virtual ~TVDataterm() {}

    virtual void computeDataterm(
            cv::Mat color,
            cv::Mat depth,
            const std::vector<float> &intrinsics,
            const Scribbles &scribbles,
            int numLabels,
            const Feedback *feedback) = 0;
    virtual void clear() = 0;
    virtual bool datatermAvailable() const = 0;
    virtual cv::Mat dataterm() const = 0;

    virtual settings::SettingsPtr settings() const = 0;
};


typedef boost::shared_ptr<TVDataterm> TVDatatermPtr;
typedef boost::shared_ptr<const TVDataterm> TVDatatermConstPtr;


} // namespace tvseg

#endif // TVSEG_TVDATATERM_H
