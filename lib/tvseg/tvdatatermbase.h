#ifndef TVSEG_TVDATATERMBASE_H
#define TVSEG_TVDATATERMBASE_H

#include "tvdataterm.h"

namespace tvseg {

class TVDatatermBase : public TVDataterm
{
public:

    // Dataterm interface
public:
    void clear();
    bool datatermAvailable() const;
    cv::Mat dataterm() const;

protected:
    cv::Mat dataterm_;
};

} // namespace tvseg

#endif // TVSEG_TVDATATERMBASE_H
