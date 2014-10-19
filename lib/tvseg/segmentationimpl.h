#ifndef TVSEG_SEGMENTATIONIMPL_H
#define TVSEG_SEGMENTATIONIMPL_H


#include "tvseg/segmentation.h"
#include "tvseg/settingsdef/segmentationsettings.h"
#include "tvseg/settingsdef/inputsettings.h"
#include "tvseg/settingsdef/outputsettings.h"

#include "tvseg/kmeans.h"
#include "tvseg/tvweight.h"
#include "tvseg/tvdataterm.h"
#include "tvseg/tvsolver.h"
#include "tvseg/tvvisualizer.h"
#include "tvseg/cvalgorithminterfacebase.h"


namespace tvseg {

// TODO: Interface various parts of this implementation (segmentation class, label provider, dataterm, weight etc...)


class SegmentationImpl : public Segmentation
{
public:
    SegmentationImpl(settings::BackendPtr settingsBackend);


public: // Segmentation interface
    //    void setSettingsBackend(settings::BackendPtr settingsBackend);

    void setFeedback(const FeedbackConstPtr &feedback);

    void saveResult();

    void loadInput();

    void loadSettings();

    void loadInputImageColor();
    cv::Mat inputImageColor() const;
    bool    inputImageColorAvailable() const;

    void loadInputImageDepth();
    cv::Mat inputImageDepth() const;
    bool    inputImageDepthAvailable() const;

    void loadInputImageGroundTruth();
    cv::Mat inputImageGroundTruth() const;
    bool    inputImageGroundTruthAvailable() const;

    void loadInputGroundTruthLabelMapping();
    void saveInputGroundTruthLabelMapping() const;
    const std::vector<uint>& inputGroundTruthLabelMapping() const;


    cv::Mat labels() const;
    bool    labelsAvailable() const;

    void loadScribbles();
    void saveScribbles() const;
    void setScribbles(const Scribbles &scribbles);
    const Scribbles& scribbles() const;

    void computeKMeans();
    cv::Mat kMeans() const;
    bool    kMeansAvailable() const;

    void computeWeight();
    cv::Mat weight() const;
    bool weightAvailable() const;
    const CVAlgorithmInterface *weightInterface() const;

    void computeDataterm();
    cv::Mat dataterm() const;
    bool datatermAvailable() const;
    const CVAlgorithmInterface *datatermInterface() const;

    settings::SettingsPtr solverSettings();
    void computeSolution();
    cv::Mat solution() const;
    bool solutionAvailable() const;
    const CVAlgorithmInterface *solverInterface() const;

    void computeVisualization();
    const CVAlgorithmInterface *visualizerInterface() const;

    std::vector<float> computeMetrics() const;

    void saveScribbleImage(std::string filename, const Scribbles &scribbles) const;

private:

    cv::Mat loadImage(std::string filename) const;
    void saveImage(std::string filename, cv::Mat image, bool normalize=false, bool equalize=false) const;

    void saveDepthImage(std::string filename, cv::Mat image, bool normalize=false) const;
    void saveMetrics(std::string filename, const std::vector<float> &metrics) const;

    template<class E>
    std::vector<E> loadArray(std::string filename) const;

    template<class E>
    void saveArray(std::string filename, const std::vector<E> &value) const;

    SegmentationSettings::ConstPtr settings() const;
    InputSettings::ConstPtr inputSettings() const;
    OutputSettings::ConstPtr outputSettings() const;
    KMeansPtr kmeans();
    KMeansConstPtr kmeans() const;
    TVWeightPtr tvweight();
    TVWeightConstPtr tvweight() const;
    TVDatatermPtr tvdataterm();
    TVDatatermConstPtr tvdataterm() const;
    TVSolverPtr tvsolver();
    TVSolverConstPtr tvsolver() const;
    TVVisualizerPtr tvvisualizer();
    TVVisualizerConstPtr tvvisualizer() const;

    void configureWeightInterface();
    void configureDatatermInterface();
    void configureSolverInterface();
    void configureVisualizerInterface();

    void initDefaultLabels();

    std::vector<float> computeMetrics(cv::Mat prediction, cv::Mat groundTruth, uint numLabels) const;

    void copyAlgorithmicSettings(settings::BackendPtr src, settings::BackendPtr dest) const;

private:
    settings::BackendPtr settingsBackend_;
    SegmentationSettings::ConstPtr settings_;
    InputSettings::ConstPtr inputSettings_;
    OutputSettings::ConstPtr outputSettings_;

    cv::Mat inputImageColor_;
    cv::Mat inputImageDepth_;
    cv::Mat inputImageGroundTruth_;
    std::vector<uint> inputGroundTruthLabelMapping_;

    Scribbles scribbles_;

    KMeansPtr kmeans_;
    TVWeightPtr tvweight_;
    TVDatatermPtr tvdataterm_;
    TVSolverPtr tvsolver_;
    TVVisualizerPtr tvvisualizer_;

    CVAlgorithmInterfaceBasePtr weightInterface_;
    CVAlgorithmInterfaceBasePtr datatermInterface_;
    CVAlgorithmInterfaceBasePtr solverInterface_;
    CVAlgorithmInterfaceBasePtr visualizerInterface_;

    FeedbackConstPtr feedback_;

    cv::Mat allLabels_;
};


template<class E>
std::vector<E> SegmentationImpl::loadArray(std::string filename) const
{
    std::ifstream fs(filename.c_str());
    std::string line;
    std::vector<E> result;
    if (!std::getline(fs, line)) {
        LERRORF("Cannot load file '%s'", filename.c_str());
        return result;
    }
    std::istringstream ss(line);
    ss >> result;
    LINFOF("Loaded file '%s'", filename.c_str());
    return result;
}

template<class E>
void SegmentationImpl::saveArray(std::string filename, const std::vector<E> &value) const
{
    std::ofstream fs(filename.c_str());
    fs << value;
    LINFOF("Saved file '%s'", filename.c_str());
}


} // namespace tvseg


#endif // TVSEG_SEGMENTATIONIMPL_H
