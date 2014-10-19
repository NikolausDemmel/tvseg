#include "tvweightedge.h"

#include "gradientcpu.h"


#include "tvseg/util/logging.h"
#include "tvseg/util/opencvhelper.h"
#include "tvseg/util/helpers.h"


namespace tvseg {

TVWeightEdge::TVWeightEdge(settings::BackendPtr backend):
    edgeDetection_(new GradientCPU),
    settings_(new WeightSettings(backend))
{
}

WeightSettings::ConstPtr TVWeightEdge::s()
{
    return settings_;
}

void TVWeightEdge::computeWeight(cv::Mat color, cv::Mat depth)
{
    bool useColor = s()->useColor();
    bool useDepth = s()->useDepth();

//    LDEBUG << "color type: " << matTypeToStr(color.type()) << ", depth type: " << matTypeToStr(depth.type());

    if (useColor && color.empty()) {
        LWARNING << "No color image given for weight computation; not using color.";
        useColor = false;
    }

    if (useDepth && depth.empty()) {
        LWARNING << "No depth image given for weight computation; not using depth.";
        useDepth = false;
    }


    if (useDepth && s()->normalizeDepth()) {
        cv::Mat tmp;
        cv::normalize(depth, tmp, 0.0, 1.0, cv::NORM_MINMAX);
        depth = tmp;
//        LDEBUG << "depth type after norm: " << matTypeToStr(depth.type());
    }


    weight_ = cv::Mat();

    if (useColor && !useDepth) {
        edgeDetection()->computeEdges(color, s()->smoothingBandwidth());
        if (!edgeDetection()->edgesAvailable()) {
            LWARNINGF("Edges could not be computed on color image.");
        } else {
            edgeDetection()->edges().copyTo(weight_);
            weight_ *= - s()->beta();
            cv::exp(weight_, weight_);
        }
    } else if (useDepth && !useColor) {
        edgeDetection()->computeEdges(depth, s()->smoothingBandwidth());
        if (!edgeDetection()->edgesAvailable()) {
            LWARNINGF("Edges could not be computed on depth image.");
        } else {
            edgeDetection()->edges().copyTo(weight_);
            weight_ *= - s()->betaDepth();
            cv::exp(weight_, weight_);
        }
    } else if (useColor && useDepth) {
        cv::Mat colorGradient;
        cv::Mat depthGradient;
        edgeDetection()->computeEdges(color, s()->smoothingBandwidth());
        edgeDetection()->edges().copyTo(colorGradient);
        edgeDetection()->computeEdges(depth, s()->smoothingBandwidth());
        edgeDetection()->edges().copyTo(depthGradient);
        int variant = s()->variant();
        if (variant < 1 || variant > 5) {
            LWARNINGF("Invalid weight variant %d, using 1.", variant);
            variant = 1;
        }
//        LDEBUG << "color gradient type: " << matTypeToStr(colorGradient.type()) << ", depth gradient type: " << matTypeToStr(depthGradient.type());
        switch (variant) {
        case 1:
            weight_ = colorGradient.mul(colorGradient) + depthGradient.mul(depthGradient, s()->betaDepth());
            cv::sqrt(weight_, weight_);
            weight_ *= - s()->beta();
            cv::exp(weight_, weight_);
            break;
        case 2:
            weight_ = s()->beta() * colorGradient + s()->betaDepth() * depthGradient;
            weight_ *= -0.5;
            cv::exp(weight_, weight_);
            break;
        case 3:
            weight_ = colorGradient.mul(depthGradient, - s()->beta());
            cv::exp(weight_, weight_);
            break;
        case 4:
            colorGradient *= - s()->beta();
            depthGradient *= - s()->betaDepth();
            cv::exp(colorGradient, colorGradient);
            cv::exp(depthGradient, depthGradient);
            weight_ = colorGradient.mul(depthGradient);
            break;
        case 5:
            colorGradient *= - s()->beta();
            depthGradient *= - s()->betaDepth();
            cv::exp(colorGradient, colorGradient);
            cv::exp(depthGradient, depthGradient);
            weight_ = 0.5 * (colorGradient + depthGradient);
            break;
        }
    } else {
        // set weight to all 1.0
        cv::Size size;
        if (color.empty() && depth.empty()) {
            LWARNINGF("Neither depth nor color given to edge computation.");
        } else {
            if (!color.empty()) {
                size = color.size();
            } else {
                size = depth.size();
            }
            weight_ = cv::Mat(size, CV_32FC1, cv::Scalar(1.0));
        }
    }
}

settings::SettingsPtr TVWeightEdge::settings() const
{
    return settings_;
}

EdgeDetectionPtr TVWeightEdge::edgeDetection()
{
    return edgeDetection_;
}

EdgeDetectionConstPtr TVWeightEdge::edgeDetection() const
{
    return edgeDetection_;
}

} // namespace tvseg
