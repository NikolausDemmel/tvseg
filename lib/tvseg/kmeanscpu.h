#ifndef TVSEG_KMEANSCPU_H
#define TVSEG_KMEANSCPU_H

#include "kmeans.h"

namespace tvseg {

class KMeansCPU : public KMeans
{
public:
    KMeansCPU();

    virtual void computeMeans(const cv::Mat image, int maxNumMeans);
    virtual cv::Mat means(int numMeans) const;

    virtual int maxNumMeans() const;

private:

    cv::Mat allMeans_;
};

} // namespace tvseg

#endif // TVSEG_KMEANSCPU_H
