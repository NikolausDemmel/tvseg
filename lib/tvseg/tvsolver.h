#ifndef TVSEG_TVSOLVER_H
#define TVSEG_TVSOLVER_H

#include "tvseg/settings/settings.h"
#include "tvseg/feedback.h"
#include "tvseg/util/includeopencv.h"

#include <boost/shared_ptr.hpp>

namespace tvseg {

class TVSolver
{
public:
    virtual ~TVSolver() {}

    virtual void computeSolution(const cv::Mat dataterm, const cv::Mat weight, int numLabels, const Feedback *feedback) = 0;
    virtual void clear() = 0;
    virtual bool solutionAvailable() const = 0;
    virtual cv::Mat solution() const = 0;

    virtual settings::SettingsPtr settings() const = 0;
};

typedef boost::shared_ptr<TVSolver> TVSolverPtr;
typedef boost::shared_ptr<const TVSolver> TVSolverConstPtr;

} // namespace tvseg

#endif // TVSEG_TVSOLVER_H
