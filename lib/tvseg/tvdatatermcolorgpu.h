#ifndef TVSEG_TVDATATERMCOLORGPU_H
#define TVSEG_TVDATATERMCOLORGPU_H

#include "tvdatatermbase.h"
#include "kmeans.h"

namespace tvseg {

class TVDatatermColorGPU : public TVDatatermBase
{
public:
    TVDatatermColorGPU(KMeansPtr kmeans);

    void computeDataterm(
            cv::Mat color,
            cv::Mat depth,
            const std::vector<float> &intrinsics,
            const Scribbles &scribbles,
            int numLabels,
            const Feedback *feedback);

    settings::SettingsPtr settings() const;

private:

    KMeansPtr kmeans_;
};

} // namespace tvseg

#endif // TVSEG_TVDATATERMCOLORGPU_H
