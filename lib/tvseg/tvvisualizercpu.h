#ifndef TVSEG_TVVISUALIZERCPU_H
#define TVSEG_TVVISUALIZERCPU_H

#include "tvseg/tvvisualizerbase.h"

#include "tvseg/settingsdef/visualizersettings.h"

namespace tvseg {

class TVVisualizerCPU : public TVVisualizerBase
{
public:
    TVVisualizerCPU(settings::BackendPtr backend);

    VisualizerSettings::ConstPtr s();

    // TVSolutionVisualizer interface
public:
    void computeVisualization(const cv::Mat segmentation, const cv::Mat groundTruth, const cv::Mat image, const cv::Mat labels, int numLabels, const Feedback *feedback);

    settings::SettingsPtr settings() const;

private:
    VisualizerSettings::Ptr settings_;
};

} // namespace tvseg

#endif // TVSEG_TVVISUALIZERCPU_H
