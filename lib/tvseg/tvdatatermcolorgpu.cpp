#include "tvdatatermcolorgpu.h"

#include "cuda/colordataterm.h"

#include "tvseg/util/logging.h"

namespace tvseg {

TVDatatermColorGPU::TVDatatermColorGPU(KMeansPtr kmeans)
{
    kmeans_ = kmeans;
}

void TVDatatermColorGPU::computeDataterm(
        cv::Mat color,
        cv::Mat /*depth*/,
        const std::vector<float> &/*intrinsics*/,
        const Scribbles &/*scribbles*/,
        int numLabels,
        const Feedback */*feedback*/)
{
    const int maxNumMeans = kmeans_->maxNumMeans();
    if (maxNumMeans < numLabels){
        LDEBUGF("Max means is %d, recomputing for dataterm.", maxNumMeans);
        kmeans_->computeMeans(color, numLabels);
        if (kmeans_->maxNumMeans() < numLabels) {
            LWARNINGF("Failed to compute means.");
            dataterm_ = cv::Mat();
            return;
        }
    }

    cv::Mat means = kmeans_->means(numLabels);

    const int sizeDataterm[3] = {numLabels, color.rows, color.cols};
    dataterm_.create(3, sizeDataterm, CV_32FC1);

    Dim3 dim;
    dim.labels = numLabels;
    dim.height = color.rows;
    dim.width = color.cols;

    // FIXME: We should always also pass the step of cv matrices, or ensure otherwise that the step is minimal
    // (which is true for matrices created by opencv, but might not be true for matrices from external sources)
    cuda::computeColorDataterm((float*)dataterm_.data, (const float*)color.data, (const float*)means.data, dim);

}

settings::SettingsPtr TVDatatermColorGPU::settings() const
{
    return settings::SettingsPtr();
}

} // namespace tvseg
