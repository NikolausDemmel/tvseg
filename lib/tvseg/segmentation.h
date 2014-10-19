#ifndef TVSEG_SEGMENTATION_H
#define TVSEG_SEGMENTATION_H

#include "tvseg/settings/settings.h"
#include "tvseg/settings/backend.h"
#include "tvseg/cvalgorithminterface.h"

#include "tvseg/feedback.h"
#include "tvseg/scribbles.h"

#include "tvseg/util/includeopencv.h"
#include <boost/shared_ptr.hpp>

namespace tvseg {


// FIXME: this should be in feedback.h, but nvcc chokes on including boost/shared_ptr
typedef boost::shared_ptr<Feedback> FeedbackPtr;
typedef boost::shared_ptr<Feedback const> FeedbackConstPtr;


class Segmentation
{
public:
    virtual ~Segmentation() {}

//    virtual void setSettingsBackend(settings::BackendPtr settingsBackend) = 0;

    virtual void setFeedback(const FeedbackConstPtr &feedback) = 0;

    virtual void saveResult() = 0;

    virtual void loadInput() = 0;

    virtual void loadInputImageColor() = 0;
    virtual cv::Mat inputImageColor() const = 0;
    virtual bool    inputImageColorAvailable() const = 0;

    virtual void loadInputImageDepth() = 0;
    virtual cv::Mat inputImageDepth() const = 0;
    virtual bool    inputImageDepthAvailable() const = 0;

    virtual void loadInputImageGroundTruth() = 0;
    virtual cv::Mat inputImageGroundTruth() const = 0;
    virtual bool    inputImageGroundTruthAvailable() const = 0;

    virtual void loadInputGroundTruthLabelMapping() = 0;
    virtual void saveInputGroundTruthLabelMapping() const = 0;
    virtual const std::vector<uint>& inputGroundTruthLabelMapping() const = 0;

    virtual cv::Mat labels() const = 0;
    virtual bool    labelsAvailable() const = 0;

    virtual void loadScribbles() = 0;
    virtual void saveScribbles() const = 0;
    virtual void setScribbles(const Scribbles &scribbles) = 0;
    virtual const Scribbles& scribbles() const = 0;

    virtual void computeKMeans() = 0;
    virtual cv::Mat kMeans() const = 0;
    virtual bool    kMeansAvailable() const = 0;

    virtual void computeWeight() = 0; //< deprecate?
    virtual cv::Mat weight() const = 0; //< deprecate?
    virtual bool weightAvailable() const = 0; //< deprecate?
    virtual const CVAlgorithmInterface* weightInterface() const = 0;

    virtual void computeDataterm() = 0; //< deprecate?
    virtual cv::Mat dataterm() const = 0; //< deprecate?
    virtual bool datatermAvailable() const = 0; //< deprecate?
    virtual const CVAlgorithmInterface* datatermInterface() const = 0;

    virtual settings::SettingsPtr solverSettings() = 0; //< deprecate?
    virtual void computeSolution() = 0; //< deprecate?
    virtual cv::Mat solution() const = 0; //< deprecate?
    virtual bool solutionAvailable() const = 0; //< deprecate?
    virtual const CVAlgorithmInterface* solverInterface() const = 0;

    virtual const CVAlgorithmInterface* visualizerInterface() const = 0;

    virtual std::vector<float> computeMetrics() const = 0;

    virtual void saveScribbleImage(std::string filename, const Scribbles &scribbles) const = 0;
};


typedef boost::shared_ptr<Segmentation> SegmentationPtr;
typedef boost::shared_ptr<const Segmentation> SegmentationConstPtr;


} // namespace tvseg

#endif // TVSEG_SEGMENTATION_H
