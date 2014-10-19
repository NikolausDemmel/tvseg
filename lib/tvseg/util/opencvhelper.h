#ifndef TVSEG_UTIL_OPENCVHELPER_H
#define TVSEG_UTIL_OPENCVHELPER_H

#include "tvseg/util/includeopencv.h"
#include "tvseg/params.h"


namespace tvseg {


std::string matTypeToStr(int type);

cv::Mat mapLabels(cv::Mat labeling, std::vector<unsigned int> mapping, unsigned int numLabels);

cv::Mat rescaleGrayscaleImage(const cv::Mat & src);

cv::Mat flattenMultiRegionImage(cv::Mat const& src);

cv::Mat createMat(const float* data, Dim3 dim);

Dim3 matDim3(cv::Mat image);

float diceScore(cv::Mat prediction, cv::Mat groundTruth, unsigned int label);


}

#endif // TVSEG_UTIL_OPENCVHELPER_H
