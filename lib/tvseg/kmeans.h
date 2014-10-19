#ifndef TVSEG_KMEANS_H
#define TVSEG_KMEANS_H

#include "tvseg/util/includeopencv.h"
#include <boost/shared_ptr.hpp>

namespace tvseg {

// TODO: Generalize as "Clustering"

class KMeans
{
public:
    virtual ~KMeans() {}

//    virtual void load(std::string fileName) = 0;
//    virtual void save(std::string fileName) const = 0;

    virtual void computeMeans(const cv::Mat image, int maxNumMeans) = 0;
    virtual cv::Mat means(int numMeans) const = 0;

    virtual int maxNumMeans() const = 0;
};

typedef boost::shared_ptr<KMeans> KMeansPtr;
typedef boost::shared_ptr<const KMeans> KMeansConstPtr;

} // namespace tvseg

#endif // TVSEG_KMEANS_H
