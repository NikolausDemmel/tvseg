#ifndef TVSEG_TVWEIGHTEDGE_H
#define TVSEG_TVWEIGHTEDGE_H

#include "tvseg/tvweightbase.h"
#include "tvseg/edgedetection.h"
#include "tvseg/settingsdef/weightsettings.h"

namespace tvseg {

class TVWeightEdge : public TVWeightBase
{
public:
    TVWeightEdge(settings::BackendPtr backend);

    WeightSettings::ConstPtr s();

    void computeWeight(cv::Mat color, cv::Mat depth);

    settings::SettingsPtr settings() const;

protected:
    EdgeDetectionPtr edgeDetection();
    EdgeDetectionConstPtr edgeDetection() const;

private:
    EdgeDetectionPtr edgeDetection_;

    WeightSettings::Ptr settings_;
};

} // namespace tvseg

#endif // TVSEG_TVWEIGHTEDGE_H
