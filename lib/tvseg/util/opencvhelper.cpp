
#include "tvseg/util/opencvhelper.h"
#include "tvseg/util/logging.h"


namespace tvseg {


std::string matTypeToStr(int type) {
    std::string result;

    unsigned char depth = type & CV_MAT_DEPTH_MASK;
    unsigned char channels = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  result = "8U"; break;
        case CV_8S:  result = "8S"; break;
        case CV_16U: result = "16U"; break;
        case CV_16S: result = "16S"; break;
        case CV_32S: result = "32S"; break;
        case CV_32F: result = "32F"; break;
        case CV_64F: result = "64F"; break;
        default:     result = "User"; break;
    }

    result += "C";
    result += (channels + '0');

    return result;
}

cv::Mat mapLabels(cv::Mat labeling, std::vector<uint> mapping, unsigned int numLabels)
{
    if (labeling.type() != CV_8UC1) {
        LERROR << "Expected type CV_8UC1 for labelling, but got " << matTypeToStr(labeling.type());
        return cv::Mat();
    }

    if (mapping.size() == 0) {
        return labeling.clone();
    }

    cv::Mat tmp(labeling.size(), CV_8UC1);
    for (int i = 0; i < tmp.rows; ++i) {
        for (int j = 0; j < tmp.cols; ++j) {
            uchar val = labeling.at<uchar>(i,j);
            if (val >= mapping.size()) {
                val = 0;
            } else {
                uint mapped = mapping[val];
                if (mapped > 255) {
                    val = 255;
                    LWARNING << "Trying to map to label > 255";
                } else {
                    val = (uchar) mapped;
                }
            }
            if (val >= numLabels) {
                val = 0;
            }
            tmp.at<uchar>(i,j) = val;
        }
    }
    return tmp;
}


cv::Mat rescaleGrayscaleImage(const cv::Mat &src)
{
    if (src.channels() != 1) {
        LWARNINGF("Trying to rescale image with more than one channel.");
        return src;
    }

    cv::Mat temp;

    switch (src.type()) {
    case CV_32FC1:
        temp = src;
        break;
    case CV_8UC1:
        src.convertTo(temp, CV_32FC1);
        break;
    default:
        LWARNINGF("rescaleGrayscaleImage: Image type not supproted.");
        return src;
    }

    double min, max;
    cv::minMaxLoc(temp, &min, &max);
    return (temp - min) / (max - min);
}


cv::Mat flattenMultiRegionImage(const cv::Mat &src)
{
    if(src.dims != 3)
    {
        LWARNINGF("Flattening multi region image only supports 3 dimensional cv::Mat.");
        return cv::Mat();
    }

    int numRegions = src.size[0];
    int rows = src.size[1];
    int cols = src.size[2];

    cv::Mat flattened(rows, numRegions * cols, src.type());

    for (int r = 0; r < numRegions; ++r) {
        const cv::Mat src_region(rows, cols, src.type(), const_cast<void *>(src.ptr<void>(r,0,0)), src.step[1]);
        cv::Mat flattend_region = flattened(cv::Rect(r * cols, 0, cols, rows));
        src_region.copyTo(flattend_region);
    }

    return flattened;
}



cv::Mat createMat(const float *data, Dim3 dim)
{
    int dimSize[3] = {dim.labels, dim.height, dim.width};
    // FIXME: constness in opencv is not handled nicely unfortunately; find a better way than const_cast
    return cv::Mat(3, dimSize, CV_32FC1, const_cast<float*>(data));
}

Dim3 matDim3(cv::Mat image)
{
    Dim3 dim;

    switch (image.dims) {
    case 2:
        dim.width = image.size().width;
        dim.height = image.size().height;
        dim.labels = 0;
        break;
    case 3:
        // fixme: right indices?
        dim.width = image.size[0];
        dim.height = image.size[1];
        dim.labels = image.size[2];
        break;
    default:
        LWARNING << "matDim3: Invalid image dimension " << image.dims;
        break;
    }

    return dim;
}

float diceScore(cv::Mat prediction, cv::Mat groundTruth, uint label)
{
    if (prediction.size != groundTruth.size) {
        LERROR << "cannot compute dice score for images of unequal size";
        return 1.0;
    }
    if (prediction.channels() != 1 || groundTruth.channels() != 1) {
        LERROR << "expected single channel image for dice score";
        return 1.0;
    }

    cv::Mat a = prediction == label;
    cv::Mat b = groundTruth == label;
    double sum = (cv::sum(a)[0] + cv::sum(b)[0]);
    if (sum > 0) {
        return (2.0 * cv::sum(a & b)[0]) / sum;
    } else {
        return 1.0;
    }
}


}
