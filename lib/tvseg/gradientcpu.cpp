#include "gradientcpu.h"

#include "tvseg/util/logging.h"
#include "tvseg/util/opencvhelper.h"

namespace tvseg {

GradientCPU::GradientCPU()
{
}

void GradientCPU::computeEdges(const cv::Mat image, float smoothingBandwidth)
{
    cv::Mat blurred, gray;
    // infer the kernel size from the bandwidth
    cv::GaussianBlur( image, blurred, cv::Size(0,0), smoothingBandwidth, smoothingBandwidth, cv::BORDER_DEFAULT );

    if (image.type() == CV_32FC3) {
        cv::cvtColor( blurred, gray, CV_RGB2GRAY ); // FIXME: is it RGB or BGR?
    } else {
        gray = blurred;
    }

    if (gray.type() != CV_32FC1) {
        LWARNING << "Image type is not grayscale floating point which is assumed for gradient computation";
        gradient_ = cv::Mat(); // computation failed;
        return;
    }

    cv::Mat grad_x, grad_y;

    float scale = 1.0/8.0; // soebel normalization
    float delta = 0.0;

    cv::Sobel( gray, grad_x, CV_32F, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT );
    cv::Sobel( gray, grad_y, CV_32F, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT );

    cv::pow(grad_x, 2, grad_x);
    cv::pow(grad_y, 2, grad_y);
    cv::addWeighted( grad_x, 0.5, grad_y, 0.5, 0, gradient_ );
    cv::sqrt(gradient_, gradient_);
}

} // namespace tvseg
