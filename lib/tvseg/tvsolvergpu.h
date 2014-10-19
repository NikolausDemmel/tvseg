#ifndef TVSEG_TVSOLVERGPU_H
#define TVSEG_TVSOLVERGPU_H

#include "tvseg/tvsolverbase.h"

#include "tvseg/settingsdef/solversettings.h"

namespace tvseg {

class TVSolverGPU : public tvseg::TVSolverBase
{
public:
    TVSolverGPU(settings::BackendPtr backend);

    SolverSettings::ConstPtr s();

    // TVSolver interface
public:
    void computeSolution(const cv::Mat dataterm, const cv::Mat weight, int numLabels, const Feedback *feedback);

    settings::SettingsPtr settings() const;

private:
    SolverSettings::Ptr settings_;
};

} // namespace tvseg

#endif // TVSEG_TVSOLVERGPU_H
