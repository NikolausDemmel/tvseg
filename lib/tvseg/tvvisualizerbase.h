#ifndef TVSEG_TVVISUALIZERBASE_H
#define TVSEG_TVVISUALIZERBASE_H

#include "tvvisualizer.h"

namespace tvseg {

class TVVisualizerBase : public TVVisualizer
{
public:

    // TVVisualizer interface
public:
    void clear();
    bool visualizationAvailable() const;
    cv::Mat visualization() const;

protected:
    cv::Mat visualization_;
};

} // namespace tvseg

#endif // TVSEG_TVVISUALIZERBASE_H
