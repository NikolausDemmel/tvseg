#include "tvseg/scribbles.h"

#include "tvseg/util/logging.h"
#include "tvseg/util/opencvhelper.h"

#include "boost/tuple/tuple_comparison.hpp"
#include "boost/tuple/tuple_io.hpp"

namespace tvseg {

Scribbles::Scribbles()
{
}

Scribbles::Scribbles(const scribble_list_type &list):
    list_(list)
{
}

const scribble_list_type &Scribbles::get() const
{
    return list_;
}

void Scribbles::set(const scribble_list_type &list)
{
    list_ = list;
}

int Scribbles::getNumLabels() const
{
    return list_.size();
}

boost::shared_array<int> Scribbles::getScribblesFlat() const
{
    int total = getTotalNumScribbles();
    boost::shared_array<int> result(new int[total * 2]);
    int index = 0;
    for (unsigned int i = 0; i < list_.size(); ++i) {
        for (unsigned int j = 0; j < list_[i].size(); ++j) {
            result[index++] = list_[i][j].x;
            result[index++] = list_[i][j].y;
        }
    }
    return result;
}

boost::shared_array<int> Scribbles::getNumScribblesFlat() const
{
    boost::shared_array<int> result(new int[list_.size()]);
    for (unsigned int i = 0; i < list_.size(); ++i) {
        result[i] = list_[i].size();
    }
    return result;
}

int Scribbles::getTotalNumScribbles() const
{
    int sum = 0;
    for (unsigned int i = 0; i < list_.size(); ++i) {
        sum += list_[i].size();
    }
    return sum;
}

cv::Mat Scribbles::getAsImage(cv::Mat labelColors, Dim2 dim) const
{
    uint numLabels = labelColors.cols;
    if (list_.size() != numLabels) {
        LERROR << "Requesting scribble image for " << numLabels << " labels, but current label count is " << list_.size();
        return cv::Mat();
    }
    cv::Mat image(dim.height, dim.width, CV_8UC3, cv::Scalar(255, 255, 255));

    for (uint i = 0; i < list_.size(); ++i) {
        for (uint j = 0; j < list_[i].size(); ++j) {
            uint x = list_[i][j].x;
            uint y = list_[i][j].y;
            if (x >= dim.width || y >= dim.height) {
                LERROR << "Scribble (" << x << "," << y << ") out of bounds for image dimensions (" << dim.width << "," << dim.height << ")";
                continue;
            }
            if (labelColors.at<cv::Vec3f>(0, i) == cv::Vec3f(1.0,1.0,1.0)) {
                LWARNING << "Label color for label " << i << " is white. This info will be lost when saving the scribble image.";
                continue;
            }
            image.at<cv::Vec3b>(y, x) = labelColors.at<cv::Vec3f>(0, i) * 255.0;
        }
    }

    return image;
}



void Scribbles::setFromImage(cv::Mat scribbleImage, cv::Mat labelColors, Dim2 dim)
{    
    if (scribbleImage.type() != CV_8UC3) {
        LERROR << "Expected type CV_8UC3 for scribble image, got " << matTypeToStr(scribbleImage.type());
        return;
    }

    if (dim.width != (uint)scribbleImage.cols || dim.height != (uint)scribbleImage.rows) {
        LERROR << "Expected scribble image size of (" << dim.width << "," << dim.height << "), but got size (" << scribbleImage.cols << "," << scribbleImage.rows << ")";
        return;
    }

    uint numLabels = labelColors.cols;
    list_.clear();
    list_.resize(numLabels);

    // do lookup with integers not floating point
    labelColors.convertTo(labelColors, CV_8UC3, 255);

    typedef boost::tuple<uchar, uchar, uchar> triple_t;
    typedef std::map<triple_t, uint> reverse_map_t;
    reverse_map_t reverseLabels;
    for (int i = 0; i < labelColors.cols; ++i) {
        cv::Vec3b vec = labelColors.at<cv::Vec3b>(0, i);
        triple_t val(vec[0], vec[1], vec[2]);
        reverseLabels[val] = (uint)i;
    }

    for (int x = 0; x < scribbleImage.cols; ++x) {
        for (int y = 0; y < scribbleImage.rows; ++y) {
            cv::Vec3b vec = scribbleImage.at<cv::Vec3b>(y, x);
            triple_t val(vec[0], vec[1], vec[2]);
            if (val != triple_t(255,255,255)) {
                // ignore white pixels
                reverse_map_t::iterator iter = reverseLabels.find(val);
                if (iter == reverseLabels.end()) {
                    LWARNING << "Scribble color " << val << " at pixel (" << x << "," << y << ") does not match current label color... ignoring";
                    continue;
                }
                uint i = iter->second;
                list_[i].push_back(cv::Point(x, y));
            }
        }
    }
}

void Scribbles::saveScribbleImage(std::string filename, cv::Mat labelColors, Dim2 dim) const
{
    cv::Mat image = getAsImage(labelColors, dim);
    std::string type = matTypeToStr(image.type());
    LINFOF("Saving scribble image '%s' (%s %dx%d)", filename.c_str(), type.c_str(), image.cols, image.rows);
    cv::imwrite(filename, image);
}

void Scribbles::loadScribbleImage(std::string filename, cv::Mat labelColors, Dim2 dim)
{
    cv::Mat image = cv::imread(filename);
    std::string type = matTypeToStr(image.type());
    LINFOF("Loaded scribble image '%s' (%s %dx%d)", filename.c_str(), type.c_str(), image.cols, image.rows);
    setFromImage(image, labelColors, dim);
}




}
