#include "tvseg/kmeanscpu.h"

#include "tvseg/util/logging.h"

namespace tvseg {

KMeansCPU::KMeansCPU()
{

}

void KMeansCPU::computeMeans(const cv::Mat image, const int maxNumMeans)
{
    // todo: convert to stateless function return allmeans variable

    if (image.empty()) {
        LWARNING << "Cannot comput means of empty image.";
        return;
    }

    if (maxNumMeans <= 0) {
        LWARNING << "Should compute at least 1 mean, not %d.", maxNumMeans;
        return;
    }

    // prepare size variables and resize allMeans_ member
    allMeans_.create(maxNumMeans, maxNumMeans, CV_32FC3);
    allMeans_.setTo(0);
    const int cols = image.cols;
    const int rows = image.rows;
    const int maxSize = 10000; // TODO: config
    const bool samplePixels = cols * rows > maxSize;
    const int size = samplePixels ? maxSize : cols * rows;

    // linearize image data
    cv::Mat pixels(size,1,CV_32FC3);

    if (samplePixels) {
        // sample pixels from total
        cv::RNG rng = cv::theRNG();

        for (int i = 0; i < size; ++i) {
            int r = rng.uniform(0, rows);
            int c = rng.uniform(0, cols);
            pixels.at<cv::Vec3f>(i) = image.at<cv::Vec3f>(r,c);
        }

    } else {
        // use all pixels
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                pixels.at<cv::Vec3f>(r*cols + c) = image.at<cv::Vec3f>(r,c);
            }
        }
    }

    // calculate the first mean as the centroid
    cv::Mat centroid(1,1, CV_32FC3, cv::Scalar(0.0));
    cv::reduce(pixels, centroid, 0, CV_REDUCE_SUM);
    centroid = centroid * 1.0/size;
    allMeans_.at<cv::Vec3f>(0,0) = centroid.at<cv::Vec3f>(0);

    // set up data structures
    cv::Mat clusterCenter(maxNumMeans, 1, CV_32FC3); // cluster centers
    cv::Mat clusterSize(maxNumMeans, 1, CV_32SC1);   // number of pixels assigned to each cluster
    cv::Mat dist(size, maxNumMeans, CV_32FC1);       // distances to each mean
    cv::Mat minIdx(size, 1, CV_32SC1);               // index of closest (best) region for each pixel
    cv::Mat lastMinIdx(size, 1, CV_32SC1);           // index of last iteration
    cv::Mat minDist(size, 1, CV_32FC1);              // distance to the closest center


    // main loop of kmeans
    for (int k = 0; k < maxNumMeans; ++k) {

        // TODO: implement some form of display callback
        // LDEBUGF("Computing k-means for k = %d", k);

        minIdx.setTo(INT_MIN);
        lastMinIdx.setTo(INT_MAX);

        // determine new clustercenters (until convergence)
        while ( cv::norm(minIdx, lastMinIdx, cv::NORM_L1) != 0) {

            // reset data structures
            minIdx.copyTo(lastMinIdx);
            minIdx.setTo(INT_MIN);
            clusterCenter.setTo(0);
            clusterSize.setTo(0);
            minDist.setTo(FLT_MAX);

            // determine closest center for each pixel
            for (int i = 0; i < size; ++i) {
                for (int j = 0; j <= k; ++j) {
                    // compute distance
                    dist.at<float>(i,j) = cv::norm(pixels.at<cv::Vec3f>(i), allMeans_.at<cv::Vec3f>(k,j), cv::NORM_L2SQR);

                    // update minimum distance and index
                    if (minDist.at<float>(i) > dist.at<float>(i,j)) {
                        minDist.at<float>(i) = dist.at<float>(i,j);
                        minIdx.at<int>(i) = j;
                    }
                }
            }

            // determine new centers
            for (int i = 0; i < size; ++i) {
                int j = minIdx.at<int>(i);
                clusterCenter.at<cv::Vec3f>(j) += pixels.at<cv::Vec3f>(i);
                clusterSize.at<int>(j) += 1;
            }

            // normalize new_centers and override old means with determined means
            for (int j = 0; j <= k; ++j) {
                int s = clusterSize.at<int>(j);
                if (s == 0) {
                    // should not happen since each cluster is initialized with one pixels value as its center
                    LWARNING << "Empty cluster " << j << ". This should not occur.";
                } else {
                    allMeans_.at<cv::Vec3f>(k,j) = clusterCenter.at<cv::Vec3f>(j) / s;
                }
            }
        } // end of while


        float maxDist = INT_MIN; // this pixel will be the additional initial mean for the next iteration
        int   maxIdx = 0;        // index of pixel having maximal distance (to its center)


        if (k < (maxNumMeans - 1)) {

            // setup existing means for next iteration
            for(int j = 0; j <= k; j++){
                allMeans_.at<cv::Vec3f>(k+1,j) = allMeans_.at<cv::Vec3f>(k,j);
            }

            // determine pixel with maximum distance to its center -> will be the new (additional) initial pixel
            for (int i = 0; i < size; i++) {
                if (maxDist < minDist.at<float>(i)) {
                    maxDist = minDist.at<float>(i);
                    maxIdx = i;
                }
            }
            allMeans_.at<cv::Vec3f>(k+1,k+1) = pixels.at<cv::Vec3f>(maxIdx);
        }
    }

}

cv::Mat KMeansCPU::means(const int numMeans) const
{
    const int maxMeans = maxNumMeans();

    cv::Mat result(1, numMeans, CV_32FC3);

    if (maxMeans >= numMeans) {
        // Extract row numMeans from the allMeans matrix
        result = allMeans_(cv::Rect(0,numMeans-1,numMeans,1)).clone();
    } else {
        // Make up some means
        LWARNINGF("Retriving %d means, but only %d available. Result contains arbitrary values.", numMeans, maxMeans);
        for (int i = 0; i < numMeans; ++i) {
            const float val = ((float) i)/numMeans;
            result.at<cv::Vec3f>(0, i) = cv::Vec3f(val,val,val);
        }
    }

    return result;
}

int KMeansCPU::maxNumMeans() const
{
    cv::Size size = allMeans_.size();
    if (size.width != size.height) {
        LWARNINGF("allMeans_ has size [%d,%d], which is not square.", size.width, size.height);
        return 0;
    } else if (size.width < 0) {
        LWARNINGF("allMenas_ has negative width %d.", size.width);
        return 0;
    } else {
        return size.width;
    }
}


} // namespace tcseg
