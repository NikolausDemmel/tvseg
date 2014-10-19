#ifndef TVSEG_TVSOLVERBASE_H
#define TVSEG_TVSOLVERBASE_H

#include "tvsolver.h"

namespace tvseg {

class TVSolverBase : public TVSolver
{
public:

    // Solver interface
public:
    void clear();
    bool solutionAvailable() const;
    cv::Mat solution() const;

protected:
    cv::Mat solution_;
};

} // namespace tvseg

#endif // TVSEG_TVSOLVERBASE_H
