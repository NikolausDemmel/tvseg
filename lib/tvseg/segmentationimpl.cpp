#include "tvseg/segmentationimpl.h"

#include "tvseg/kmeanscpu.h"
#include "tvseg/tvweightedge.h"
//#include "tvseg/tvdatatermcolorgpu.h"
#include "tvseg/tvdatatermparzengpu.h"
#include "tvseg/tvsolvergpu.h"
#include "tvseg/tvvisualizercpu.h"
#include "tvseg/util/opencvhelper.h"
#include "tvseg/util/helpers.h"
#include "tvseg/settings/serializerqt.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>


namespace {

const int MAX_LABELS = 256;



// FIXME: don't implement copying settings this way, but rather add public methods
//        to the Settings class that allow to save/load to/from a different backend.
template<class T>
void copySettings(tvseg::settings::BackendPtr src, tvseg::settings::BackendPtr dest)
{
    T(dest).setParams(T(src).params());
}


}



namespace tvseg {


namespace fs = boost::filesystem;


SegmentationImpl::SegmentationImpl(settings::BackendPtr settingsBackend):
    settingsBackend_(settingsBackend),
    settings_(new SegmentationSettings(settingsBackend_)),
    inputSettings_(new InputSettings(settingsBackend_)),
    outputSettings_(new OutputSettings(settingsBackend_)),
    kmeans_(new KMeansCPU()),

    weightInterface_(new CVAlgorithmInterfaceBase()),
    datatermInterface_(new CVAlgorithmInterfaceBase()),
    solverInterface_(new CVAlgorithmInterfaceBase()),
    visualizerInterface_(new CVAlgorithmInterfaceBase())
{
    tvweight_.reset(new TVWeightEdge(settingsBackend_));

    // TODO: make this configurable; some sort of factory pattern
    //tvdataterm_.reset(new TVDatatermColorGPU(kmeans_));
    tvdataterm_.reset(new TVDatatermParzenGPU(settingsBackend_));

    tvsolver_.reset(new TVSolverGPU(settingsBackend_));
    tvvisualizer_.reset(new TVVisualizerCPU(settingsBackend_));
    configureWeightInterface();
    configureDatatermInterface();
    configureSolverInterface();
    configureVisualizerInterface();
    initDefaultLabels();
}

void SegmentationImpl::setFeedback(const FeedbackConstPtr &feedback)
{
    feedback_ = feedback;
}

void SegmentationImpl::saveResult()
{
    try {
        fs::path outputFolder(outputSettings()->resultPath());
        if (outputSettings()->subFolderByDate()) {
            outputFolder /= currentDateString();
        }
        fs::create_directories(outputFolder);

        // compute filename (remove possible exisiting datetime prefix when loading an image from the results folder)
        std::string colorFilenameStr = fs::path(inputSettings()->color()).filename().string();
        boost::regex re("^\\d{8}_\\d{6}_");
        colorFilenameStr = boost::regex_replace(colorFilenameStr, re, "");
        if (outputSettings()->prefixDateTime()) {
            colorFilenameStr = currentDateTimeString() + "_" + colorFilenameStr;
        }

        // Note: we don't save groundTruthLabelMapping, since it will already be applied to the save ground truth image.
        fs::path colorFilename = colorFilenameStr;
        fs::path depthFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), inputSettings()->depthReplace());
        fs::path depthFilenameNormalized = depthFilename.stem().string() + "_norm" + depthFilename.extension().string();
        fs::path groundTruthFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), inputSettings()->groundTruthReplace());
        fs::path groundTruthFilenameNormalized = groundTruthFilename.stem().string() + "_norm" + groundTruthFilename.extension().string();
        fs::path scribbleFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), inputSettings()->scribblesReplace());
        fs::path settingsFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), inputSettings()->settingsReplace());
        fs::path solutionFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), outputSettings()->solutionReplace());
        fs::path solutionFilenameNormalized = solutionFilename.stem().string() + "_norm" + solutionFilename.extension().string();
        fs::path weightFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), outputSettings()->weightReplace());
        fs::path datatermFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), outputSettings()->datatermReplace());
        fs::path datatermFilenameEqual = datatermFilename.stem().string() + "_equalized" + datatermFilename.extension().string();
        fs::path visualizationFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), outputSettings()->visualizationReplace());
        fs::path metricsFilename = replaceString(colorFilenameStr, inputSettings()->colorMatch(), outputSettings()->metricsReplace());

        std::string colorPath = (outputFolder / colorFilename).string();

        if (inputImageColorAvailable()) {
            saveImage(colorPath, inputImageColor());
        }
        if (inputImageDepthAvailable()) {
            saveDepthImage((outputFolder / depthFilename).string(), inputImageDepth());
            saveDepthImage((outputFolder / depthFilenameNormalized).string(), inputImageDepth(), true);
        }
        if (inputImageGroundTruthAvailable()) {
            saveImage((outputFolder / groundTruthFilename).string(), inputImageGroundTruth(), false);
            saveImage((outputFolder / groundTruthFilenameNormalized).string(), inputImageGroundTruth(), true);
        }
        saveScribbleImage((outputFolder / scribbleFilename).string(), scribbles_);
        if (solutionAvailable()) {
            saveImage((outputFolder / solutionFilename).string(), solution(), false);
            saveImage((outputFolder / solutionFilenameNormalized).string(), solution(), true);
        }
        if (weightAvailable()) {
            saveImage((outputFolder / weightFilename).string(), weight(), true);
        }
        if (datatermAvailable()) {
            saveImage((outputFolder / datatermFilename).string(), dataterm(), true);
            saveImage((outputFolder / datatermFilenameEqual).string(), dataterm(), true, true);
        }
        if (tvvisualizer_->visualizationAvailable()) {
            saveImage((outputFolder / visualizationFilename).string(), tvvisualizer_->visualization());
        }

        settings::BackendPtr backend(new settings::Backend());
        copyAlgorithmicSettings(settingsBackend_, backend);
        InputSettings inputSettings(backend);
        inputSettings.set_color(colorPath);
        inputSettings.set_overrideGroundTruthLabelMapping(false);
        settings::SerializerQt serializer(backend, (outputFolder / settingsFilename).string(), false);
        serializer.save();

        if (solutionAvailable() && inputImageGroundTruthAvailable()) {
            saveMetrics((outputFolder / metricsFilename).string(), computeMetrics());
        }
    } catch (const fs::filesystem_error& ex) {
        LERROR << "Failed to save result with filesystem error: " << ex.what();
    }
}

//void SegmentationImpl::setSettingsBackend(settings::BackendPtr settingsBackend)
//{
//    settingsBackend_ = settingsBackend;
//    // TODO: call replace
//    //settings_->replaceBackend(settingsBackend_);
//    settings_.reset(new SettingsTest(settingsBackend_));
//}

void SegmentationImpl::loadInput()
{
    if (inputSettings()->loadSettings()) {
        loadSettings();
    }
    loadInputImageColor();
    if (inputSettings()->loadDepth()) {
        loadInputImageDepth();
    }
    if (inputSettings()->loadGroundTruth()) {
        loadInputImageGroundTruth();
        loadInputGroundTruthLabelMapping();
    }
    if (inputSettings()->loadScribbles()) {
        loadScribbles();
    }
}

void SegmentationImpl::loadSettings()
{
    std::string filename = replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->settingsReplace());
    if (fs::exists(filename)) {
        LINFO << "Loading instance specific settings file '" << filename << "'.";
        settings::BackendPtr backend(new settings::Backend());
        settings::SerializerQt serializer(backend, filename, true);
        copyAlgorithmicSettings(backend, settingsBackend_);
    } else {
        LINFO << "Skipping to load non-exisitant instance specific settings file '" << filename << "'.";
    }
}

void SegmentationImpl::loadInputImageColor()
{
    std::string fileName = inputSettings()->color();
    cv::Mat input = loadImage(fileName);

    if (!input.empty()) {
        if (input.type() != CV_8UC3) {
            LERRORF("Expected type '8UC3' for color image, but got '%s'.", matTypeToStr(input.type()).c_str());
        } else {
            // convert input image from 0 ... 255 to 0 ... 1
            input.convertTo(inputImageColor_, CV_32FC3, 1/255.0f, 0);
        }
    }
}

cv::Mat SegmentationImpl::inputImageColor() const
{
    return inputImageColor_;
}

bool SegmentationImpl::inputImageColorAvailable() const
{
    return !inputImageColor_.empty();
}

void SegmentationImpl::loadInputImageDepth()
{
    std::string fileName = replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->depthReplace());
    cv::Mat input = loadImage(fileName);

    if (!input.empty()) {
        const float scale = inputSettings()->depthScale();
        switch (input.type()) {
        case CV_16UC1:
            input.convertTo(inputImageDepth_, CV_32FC1, 1/scale, 0);
            break;
        case CV_8UC1:
            input.convertTo(inputImageDepth_, CV_32FC1, 255/scale, 0);
            break;
        default:
            LERRORF("Expected type 'CV_16UC1' or 'CV_8UC1' for depth image, but got '%s'.", matTypeToStr(input.type()).c_str());
            break;
        }
    }
}

cv::Mat SegmentationImpl::inputImageDepth() const
{
    return inputImageDepth_;
}

bool SegmentationImpl::inputImageDepthAvailable() const
{
    return !inputImageDepth_.empty();
}

void SegmentationImpl::loadInputImageGroundTruth()
{
    std::string fileName = replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->groundTruthReplace());
    cv::Mat input = loadImage(fileName);

    if (!input.empty()) {
        switch (input.type()) {
        case CV_8UC1:
            inputImageGroundTruth_ = input;
            break;
        case CV_16UC1:
            input.convertTo(inputImageGroundTruth_, CV_8UC1, 1.0f, 0);
            break;
        default:
            LERRORF("Expected type CV_16UC1 or CV_16UC1 for ground truth image, but got '%s'.", matTypeToStr(input.type()).c_str());
        }
    }
}

cv::Mat SegmentationImpl::inputImageGroundTruth() const
{
    return tvseg::mapLabels(inputImageGroundTruth_, inputGroundTruthLabelMapping(), settings()->numLabels());
}

bool SegmentationImpl::inputImageGroundTruthAvailable() const
{
    return !inputImageGroundTruth_.empty();
}

void SegmentationImpl::loadInputGroundTruthLabelMapping()
{
    std::string fileName = replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->groundTruthLabelMappingReplace());
    std::ifstream fs(fileName.c_str());
    if (fs.good()) {
        inputGroundTruthLabelMapping_ = loadArray<uint>(fileName);
    } else {
        LINFO << "Skipping to load non-exisitant label mapping file '" << fileName << "'.";
        inputGroundTruthLabelMapping_.clear();
    }
}

void SegmentationImpl::saveInputGroundTruthLabelMapping() const
{
    std::string filename = replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->groundTruthLabelMappingReplace());
    saveArray<uint>(filename, inputGroundTruthLabelMapping());
}

const std::vector<uint>& SegmentationImpl::inputGroundTruthLabelMapping() const
{
    if (inputSettings()->overrideGroundTruthLabelMapping()) {
        return inputSettings()->groundTruthLabelMappingOverride();
    } else {
        return inputGroundTruthLabelMapping_;
    }
}

cv::Mat SegmentationImpl::labels() const
{
    const uint n = settings()->numLabels();

    if (n > MAX_LABELS) {
        LERROR << "Trying to access more labels than available.";
        return cv::Mat();
    }

    cv::Mat labels(1, n, CV_32FC3);
    for (uint i = 0; i < n; ++i) {
        labels.at<cv::Vec3f>(0, i) = allLabels_.at<cv::Vec3f>(0, i);
    }
    return labels;
}

bool SegmentationImpl::labelsAvailable() const
{
    return settings()->numLabels() <= MAX_LABELS;
}


void SegmentationImpl::loadScribbles()
{
    Dim2 dim;
    if (!inputImageColorAvailable() && !inputImageDepthAvailable()) {
        LERROR << "Cannot laod scribbles when no input image is loaded";
        return;
    }
    if (!labelsAvailable()) {
        LERROR << "Cannot load scribbles when label colors are not available";
        return;
    }
    cv::Mat inputImage;
    if (inputImageColorAvailable()) {
        inputImage = inputImageColor();
    } else if (inputImageDepthAvailable()) {
        inputImage = inputImageDepth();
    }
    dim.width = inputImage.cols;
    dim.height = inputImage.rows;
    std::string filename = "scribbles.png";
    if (!inputSettings()->overrideScribbles()) {
        std::string tmp = tvseg::replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->scribblesReplace());
        if (fs::exists(tmp)) {
            filename = tmp;
        } else {
            LINFO << "Skipping to load non-existent scribble image '" << filename << "'.";
        }
    }
    if (!fs::exists(filename)) {
        LINFO << "Skipping to load non-existent scribble image '" << filename << "'.";
    } else {
        scribbles_.loadScribbleImage(filename, labels(), dim);
    }
}

void SegmentationImpl::saveScribbles() const
{
    std::string filename = tvseg::replaceString(inputSettings()->color(), inputSettings()->colorMatch(), inputSettings()->scribblesReplace());
    saveScribbleImage(filename, scribbles_);
}

void SegmentationImpl::setScribbles(const Scribbles &scribbles)
{
    scribbles_ = scribbles;
}

const Scribbles& SegmentationImpl::scribbles() const
{
    return scribbles_;
}

void SegmentationImpl::computeKMeans()
{
    const int n = settings()->numLabels();

    if (!inputImageColorAvailable()) {
        LDEBUGF("No image available; Cannot compute %d means", n);
        return;
    }

    kmeans()->computeMeans(inputImageColor(), n);
    if (kmeans()->maxNumMeans() >= n) {
        LINFOF("Computed %d means successfully", n);
    } else {
        LWARNINGF("Failed to compute %d means", n);
    }
}

cv::Mat SegmentationImpl::kMeans() const
{
    return kmeans()->means(settings()->numLabels());
}

bool SegmentationImpl::kMeansAvailable() const
{
    return kmeans_->maxNumMeans() >= (int) settings()->numLabels();
}

void SegmentationImpl::computeWeight()
{
    tvweight()->computeWeight(inputImageColor(), inputImageDepth());
    if (tvweight()->weightAvailable()) {
        LINFOF("Computed weight");
    } else {
        LWARNINGF("Failed to compute weight.");
    }
}

cv::Mat SegmentationImpl::weight() const
{
    return tvweight()->weight();
}

bool SegmentationImpl::weightAvailable() const
{
    return tvweight()->weightAvailable();
}

const CVAlgorithmInterface *SegmentationImpl::weightInterface() const
{
    return weightInterface_.get();
}

void SegmentationImpl::computeDataterm()
{
    if (!inputImageColorAvailable()) {
        LDEBUGF("No image available. Cannot compute dataterm.");
        return;
    }

    tvdataterm()->computeDataterm(inputImageColor(),
                                  inputImageDepth(),
                                  inputSettings()->intrinsics(),
                                  scribbles(),
                                  settings()->numLabels(),
                                  feedback_.get());
    if (tvdataterm()->datatermAvailable()) {
        LINFOF("Computed dataterm");
    } else {
        LWARNINGF("Failed to compute dataterm.");
    }
}

cv::Mat SegmentationImpl::dataterm() const
{
    return tvdataterm()->dataterm();
}

bool SegmentationImpl::datatermAvailable() const
{
    return tvdataterm()->datatermAvailable();
}

const CVAlgorithmInterface *SegmentationImpl::datatermInterface() const
{
    return datatermInterface_.get();
}

settings::SettingsPtr SegmentationImpl::solverSettings()
{
    return tvsolver()->settings();
}

void SegmentationImpl::computeSolution()
{
    if (!datatermAvailable()) {
        LDEBUGF("No dataterm available. Cannot compute solution.");
        return;
    }

    if (!weightAvailable()) {
        LDEBUGF("No weight available. Cannot compute solution.");
        return;
    }

    tvsolver()->computeSolution(dataterm(), weight(), settings()->numLabels(), feedback_.get());
    if (tvsolver()->solutionAvailable()) {
        LINFOF("Computed solution");
    } else {
        LWARNINGF("Failed to compute solution.");
    }
}

cv::Mat SegmentationImpl::solution() const
{
    return tvsolver()->solution();
}

bool SegmentationImpl::solutionAvailable() const
{
    return tvsolver()->solutionAvailable();
}

const CVAlgorithmInterface *SegmentationImpl::solverInterface() const
{
    return solverInterface_.get();
}

const CVAlgorithmInterface *SegmentationImpl::visualizerInterface() const
{
    return visualizerInterface_.get();
}

std::vector<float> SegmentationImpl::computeMetrics() const
{
    if (!solutionAvailable()) {
        LERROR << "Solution not available to compute metrics";
        return std::vector<float>();
    }
    if (!inputImageGroundTruthAvailable()) {
        LERROR << "No ground truth available to compute metrics";
        return std::vector<float>();
    }
    return computeMetrics(solution(), inputImageGroundTruth(), settings()->numLabels());
}

void SegmentationImpl::computeVisualization()
{
    if (!solutionAvailable()) {
        LDEBUGF("No solution available. Cannot compute visualization.");
        return;
    }

    if (!inputImageColorAvailable()) {
        LDEBUGF("No inputImageColor available. Cannot compute visualization.");
        return;
    }

    if (!labelsAvailable()) {
        LDEBUGF("No labels available. Cannot compute visualization.");
        return;
    }

    tvvisualizer()->computeVisualization(solution(), inputImageGroundTruth(), inputImageColor(), labels(), settings()->numLabels(), feedback_.get());
    if (tvvisualizer()->visualizationAvailable()) {
        LINFOF("Computed visualization");
    } else {
        LWARNINGF("Failed to compute visualization.");
    }
}

cv::Mat SegmentationImpl::loadImage(std::string filename) const
{
    cv::Mat input = cv::imread(filename, CV_LOAD_IMAGE_UNCHANGED);

    if (input.empty()) {
        LWARNINGF("Cannot load image '%s'", filename.c_str());
    } else {
        std::string type = matTypeToStr(input.type());
        LINFOF("Loaded image '%s' (%s %dx%d)", filename.c_str(), type.c_str(), input.cols, input.rows);
    }

    return input;
}

void SegmentationImpl::saveImage(std::string filename, cv::Mat image, bool normalize, bool equalize) const
{
    if (image.dims == 3) {
        image = flattenMultiRegionImage(image);
    }
    if (image.channels() == 1) {
        if (normalize) {
            cv::Mat tmp;
            cv::normalize(image, tmp, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            image = tmp;
        } else {
            switch (image.type()) {
            case CV_32FC1:
                image.convertTo(image, CV_8UC1, 255, 0);
                break;
            case CV_8UC1:
            case CV_16UC1:
            default:
                break;
            }
        }
        if (equalize) {
            cv::Mat tmp;
            cv::equalizeHist(image, tmp);
            image = tmp;
        }
    } else if (image.channels() == 3) {
        if (image.type() == CV_32FC3) {
            image.convertTo(image, CV_8UC3, 255.0f, 0);
        }
    }
    std::string type = matTypeToStr(image.type());
    if (cv::imwrite(filename, image)) {
        LINFOF("Saved image '%s' (%s %dx%d)", filename.c_str(), type.c_str(), image.cols, image.rows);
    } else {
        LINFOF("Failed to save image '%s' (%s %dx%d)", filename.c_str(), type.c_str(), image.cols, image.rows);
    }
}

void SegmentationImpl::saveDepthImage(std::string filename, cv::Mat image, bool normalize) const
{
    if (image.type() != CV_32FC1) {
        LERRORF("Expected type 'CV_32FC1' for depth image, but got '%s'.", matTypeToStr(image.type()).c_str());
    } else {
        cv::Mat tmp;
        if (normalize) {
            cv::normalize(image, tmp, 0, 65535, cv::NORM_MINMAX, CV_16UC1);
        } else {
            float scale = inputSettings()->depthScale();
            image.convertTo(tmp, CV_16UC1, scale, 0);
        }
        saveImage(filename, tmp);
    }
}

void SegmentationImpl::saveScribbleImage(std::string filename, const Scribbles &scribbles) const
{
    Dim2 dim;
    if (!inputImageColorAvailable() && !inputImageDepthAvailable()) {
        LERROR << "Cannot save scribbles when no input image is loaded";
        return;
    }
    if (!labelsAvailable()) {
        LERROR << "Cannot save scribbles when label colors are not available";
        return;
    }
    cv::Mat inputImage;
    if (inputImageColorAvailable()) {
        inputImage = inputImageColor();
    } else if (inputImageDepthAvailable()) {
        inputImage = inputImageDepth();
    }
    dim.width = inputImage.cols;
    dim.height = inputImage.rows;
    scribbles.saveScribbleImage(filename, labels(), dim);
}

void SegmentationImpl::saveMetrics(std::string filename, const std::vector<float> &metrics) const
{
    std::ofstream s(filename.c_str());
    s << "# The first value is the mean dice score. The following values are the dice scores for each label.\n";
    for (uint i = 0; i < metrics.size(); ++i)
        s << metrics[i] << " ";
    s << "\n";
}

SegmentationSettings::ConstPtr SegmentationImpl::settings() const
{
    return settings_;
}

InputSettings::ConstPtr SegmentationImpl::inputSettings() const
{
    return inputSettings_;
}

OutputSettings::ConstPtr SegmentationImpl::outputSettings() const
{
    return outputSettings_;
}

KMeansPtr SegmentationImpl::kmeans()
{
    return kmeans_;
}

KMeansConstPtr SegmentationImpl::kmeans() const
{
    return kmeans_;
}

TVWeightPtr SegmentationImpl::tvweight()
{
    return tvweight_;
}

TVWeightConstPtr SegmentationImpl::tvweight() const
{
    return tvweight_;
}

TVDatatermPtr SegmentationImpl::tvdataterm()
{
    return tvdataterm_;
}

TVDatatermConstPtr SegmentationImpl::tvdataterm() const
{
    return tvdataterm_;
}

TVSolverPtr SegmentationImpl::tvsolver()
{
    return tvsolver_;
}

TVSolverConstPtr SegmentationImpl::tvsolver() const
{
    return tvsolver_;
}

TVVisualizerPtr SegmentationImpl::tvvisualizer()
{
    return tvvisualizer_;
}

TVVisualizerConstPtr SegmentationImpl::tvvisualizer() const
{
    return tvvisualizer_;
}

void SegmentationImpl::configureWeightInterface()
{
    weightInterface_->setTypeName("Weight");
    weightInterface_->setName("WeightEdge");
    weightInterface_->setSettings(tvweight()->settings());
    weightInterface_->clearActions();
    weightInterface_->addAction(CVAlgorithmInterface::Action(
                                    "weight",
                                    boost::bind(&SegmentationImpl::computeWeight, this),
                                    boost::bind(&TVWeight::clear, tvweight_.get()),
                                    boost::bind(&SegmentationImpl::weightAvailable, this),
                                    boost::bind(&SegmentationImpl::weight, this)));
}

void SegmentationImpl::configureDatatermInterface()
{
    datatermInterface_->setTypeName("Dataterm");
    datatermInterface_->setName("DatatermColorGPU");
    datatermInterface_->setSettings(tvdataterm()->settings());
    datatermInterface_->clearActions();
    datatermInterface_->addAction(CVAlgorithmInterface::Action(
                                    "dataterm",
                                    boost::bind(&SegmentationImpl::computeDataterm, this),
                                    boost::bind(&TVDataterm::clear, tvdataterm_.get()),
                                    boost::bind(&SegmentationImpl::datatermAvailable, this),
                                    boost::bind(&SegmentationImpl::dataterm, this)));

}

void SegmentationImpl::configureSolverInterface()
{
    // TODO: generalize and maybe let some of this info come from the algorithm object itself

    solverInterface_->setTypeName("Solver");
    solverInterface_->setName("SolverGPU");
    solverInterface_->setSettings(tvsolver()->settings());
    solverInterface_->clearActions();
    solverInterface_->addAction(CVAlgorithmInterface::Action(
                                    "solution",
                                    boost::bind(&SegmentationImpl::computeSolution, this),
                                    boost::bind(&TVSolver::clear, tvsolver_.get()),
                                    boost::bind(&SegmentationImpl::solutionAvailable, this),
                                    boost::bind(&SegmentationImpl::solution, this)));

}

void SegmentationImpl::configureVisualizerInterface()
{
    visualizerInterface_->setTypeName("Visualizer");
    visualizerInterface_->setName("VisualizerCPU");
    visualizerInterface_->setSettings(tvvisualizer()->settings());
    visualizerInterface_->clearActions();
    visualizerInterface_->addAction(CVAlgorithmInterface::Action(
                                        "visualization",
                                        boost::bind(&SegmentationImpl::computeVisualization, this),
                                        boost::bind(&TVVisualizer::clear, tvvisualizer_.get()),
                                        boost::bind(&TVVisualizer::visualizationAvailable, tvvisualizer_.get()),
                                        boost::bind(&TVVisualizer::visualization, tvvisualizer_.get())));

}

void SegmentationImpl::initDefaultLabels()
{
    allLabels_ = cv::Mat(1, MAX_LABELS, CV_32FC3);

    // Some hardcoded values
    allLabels_.at<cv::Vec3f>(0, 0) = cv::Vec3f(0  , 0  , 1  ); // BGR
    allLabels_.at<cv::Vec3f>(0, 1) = cv::Vec3f(0  , 1  , 0  );
    allLabels_.at<cv::Vec3f>(0, 2) = cv::Vec3f(1  , 0  , 0  );
    allLabels_.at<cv::Vec3f>(0, 3) = cv::Vec3f(1  , 1  , 0  );
    allLabels_.at<cv::Vec3f>(0, 4) = cv::Vec3f(0  , 1  , 1  );
    allLabels_.at<cv::Vec3f>(0, 5) = cv::Vec3f(1  , 0  , 1  );
    allLabels_.at<cv::Vec3f>(0, 6) = cv::Vec3f(1  , 0.5, 0.5);
    allLabels_.at<cv::Vec3f>(0, 7) = cv::Vec3f(0.5, 1  , 0.5);
    allLabels_.at<cv::Vec3f>(0, 8) = cv::Vec3f(0.5, 0.5, 1  );

    // fill the rest with some grey values for now; if needed add more hardcoded values later
    for (int i = 9; i < MAX_LABELS; ++i) {
        allLabels_.at<cv::Vec3f>(0, i) = cv::Vec3f(i / 255., i / 255., i / 255.);
    }
}

std::vector<float> SegmentationImpl::computeMetrics(cv::Mat prediction, cv::Mat groundTruth, uint numLabels) const
{
    std::vector<float> metrics;

    float sum = 0;
    for (uint i = 0; i < numLabels; ++i) {
        float score = diceScore(prediction, groundTruth, i);
        metrics.push_back(score);
        sum += score;
    }
    metrics.insert(metrics.begin(), sum/numLabels);

    return metrics;
}

void SegmentationImpl::copyAlgorithmicSettings(settings::BackendPtr src, settings::BackendPtr dest) const
{
    copySettings<InputSettings>(src, dest);
    copySettings<ParzenSettings>(src, dest);
    copySettings<SegmentationSettings>(src, dest);
    copySettings<SolverSettings>(src, dest);
    copySettings<VisualizerSettings>(src, dest);
    copySettings<WeightSettings>(src, dest);
}


} // namespace tvseg
