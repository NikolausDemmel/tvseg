#include "tvseg/tvvisualizercpu.h"
#include "tvseg/cuda/solver.h"

#include "tvseg/cuda/errorhandling.h"
#include "tvseg/cuda/params.h"

namespace tvseg {

TVVisualizerCPU::TVVisualizerCPU(const settings::BackendPtr backend):
    settings_(new VisualizerSettings(backend))
{
}

VisualizerSettings::ConstPtr TVVisualizerCPU::s()
{
    return settings_;
}

settings::SettingsPtr TVVisualizerCPU::settings() const
{
    return settings_;
}

void TVVisualizerCPU::computeVisualization(const cv::Mat segmentation, const cv::Mat groundTruth, const cv::Mat image, const cv::Mat labels, const int numLabels, const Feedback */*feedback*/)
{
    // TODO: fix checks on input dimensions

    int rows = image.rows;
    int cols = image.cols;

    // sanity check
    if (segmentation.size != image.size)
    {
        LERROR << "Dimensions do not match in computeVisualization";
        return;
    }

    bool showGroundTruth = !groundTruth.empty() && s()->showGroundTruth();

    // Compute number of labels used
    {
        // Initialize parameters
        int histSize = numLabels;    // bin size
        float range[] = { 0, numLabels };
        const float *ranges[] = { range };

        // Calculate histogram
        cv::Mat hist;
        cv::calcHist(&segmentation, 1, NULL, cv::Mat(), hist, 1, &histSize, ranges, true, false);

//        LINFO << "used labels: " << cv::countNonZero(hist) << " of " << numLabels;
    }

    // color regions
    cv::Mat regions(rows, cols, CV_32FC3);
    cv::Mat imageAlpha(rows, cols, CV_32FC1);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cv::Vec3f color = labels.at<cv::Vec3f>(segmentation.at<unsigned char>(i,j));
            imageAlpha.at<float>(i, j) = s()->imageTransparancy();
            if (showGroundTruth && segmentation.at<uchar>(i,j) != groundTruth.at<uchar>(i,j)) {
                if ((i % 5 == 2) != (j % 5 == 2)) {
                    color = labels.at<cv::Vec3f>(groundTruth.at<unsigned char>(i,j));
                }
            }
            regions.at<cv::Vec3f>(i,j) = color;
        }
    }

    if (s()->showBorder()) {
//        cv::Mat regionsGray;
        cv::Mat cannyEdges;
//        cv::Mat temp;
//        cv::cvtColor(regions, temp, CV_BGR2GRAY);
//        temp.convertTo(regionsGray, CV_8UC1, 255, 0);
//        cv::blur(regionsGray, regionsGray, cv::Size(3,3));
        cv::Canny(segmentation, cannyEdges, 4, 10, 5);

//        cannyEdges.convertTo(temp, CV_32FC1, 1.0/255, 0);
//        feedback->displayImage((float*)temp.data, cuda::DimParams3(1,rows,cols), "edges");

        // set values at edges to black
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (cannyEdges.at<unsigned char>(i,j) == 255) {
                    regions.at<cv::Vec3f>(i,j) = cv::Vec3f(0,0,0);
                    imageAlpha.at<float>(i,j) = 0;
                }
            }
        }
    }

    cv::Mat background;
    if (s()->showImage()) {
        // combine segmented image and original image
        background = image.clone();
    } else {
//        background = regions.clone();
        background = cv::Mat(image.size(), CV_32FC3, cv::Scalar(1,1,1));
    }

    cv::cvtColor(imageAlpha, imageAlpha, CV_GRAY2RGB);

    // weighted addition
    background = background.mul(imageAlpha);
    regions = regions.mul(cv::Scalar(1,1,1)-imageAlpha);
    visualization_ = background + regions;
}

} // namespace tvseg
