#ifndef TVSEG_TVWEIGHTBASE_H
#define TVSEG_TVWEIGHTBASE_H

#include "tvweight.h"

namespace tvseg {

class TVWeightBase : public TVWeight
{
public:
    TVWeightBase();

    // TVWeight interface
public:
    void clear();
    cv::Mat weight() const;
    bool weightAvailable() const;

protected:
    cv::Mat weight_;
};

} // namespace tvseg

#endif // TVSEG_TVWEIGHTBASE_H
