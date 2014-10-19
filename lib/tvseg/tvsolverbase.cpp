#include "tvsolverbase.h"

namespace tvseg {

void TVSolverBase::clear()
{
    solution_ = cv::Mat();
}

bool TVSolverBase::solutionAvailable() const
{
    return !solution_.empty();
}

cv::Mat TVSolverBase::solution() const
{
    return solution_;
}


} // namespace tvseg
