#ifndef TVSEG_IMAGEDIMENSIONS_H
#define TVSEG_IMAGEDIMENSIONS_H


#include "tvseg/util/includeopencv.h"
#include "tvseg/params.h"

#include "boost/enum.hpp"


namespace tvseg {

BOOST_ENUM(ImageSizeSpec,
    (Invalid)
    (Labels)
    (Input)
    (Dataterm))


bool checkDimensions(cv::Mat image, Dim3 dim, ImageSizeSpec spec);


} // namespace tvseg


#endif // TVSEG_IMAGEDIMENSIONS_H
