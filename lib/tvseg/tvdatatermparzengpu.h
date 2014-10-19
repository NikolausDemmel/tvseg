#ifndef TVSEG_TVDATATERMPARZENGPU_H
#define TVSEG_TVDATATERMPARZENGPU_H

#include "tvseg/tvdatatermbase.h"
#include "tvseg/settingsdef/parzensettings.h"

namespace tvseg {

class TVDatatermParzenGPU : public TVDatatermBase
{
public:
    TVDatatermParzenGPU(settings::BackendPtr backend);

    ParzenSettings::ConstPtr s();

    // TVDaterm interface
public:
    void computeDataterm(
            cv::Mat color,
            cv::Mat depth,
            const std::vector<float> &intrinsics,
            const Scribbles &scribbles,
            int numLabels,
            const Feedback *feedback);

    settings::SettingsPtr settings() const;

private:
    ParzenSettings::Ptr settings_;
};

} // namespace tvseg

#endif // TVSEG_TVDATATERMPARZENGPU_H
