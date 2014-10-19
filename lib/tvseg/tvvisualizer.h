#ifndef TVSEG_TVVISUALIZER_H
#define TVSEG_TVVISUALIZER_H

#include "tvseg/settings/settings.h"
#include "tvseg/feedback.h"
#include "tvseg/util/includeopencv.h"

#include <boost/shared_ptr.hpp>

namespace tvseg {

class TVVisualizer
{
public:
    virtual ~TVVisualizer() {}

    virtual void computeVisualization(const cv::Mat segmentation, const cv::Mat groundTruth, const cv::Mat image, const cv::Mat labels, int numLabels, const Feedback *feedback) = 0;
    virtual void clear() = 0;
    virtual bool visualizationAvailable() const = 0;
    virtual cv::Mat visualization() const = 0;

    virtual settings::SettingsPtr settings() const = 0;
};

typedef boost::shared_ptr<TVVisualizer> TVVisualizerPtr;
typedef boost::shared_ptr<const TVVisualizer> TVVisualizerConstPtr;

} // namespace tvseg

#endif // TVSEG_TVVISUALIZER_H
