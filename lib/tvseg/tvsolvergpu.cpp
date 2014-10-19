#include "tvseg/tvsolvergpu.h"
#include "tvseg/cuda/solver.h"

#include "tvseg/util/logging.h"
#include "tvseg/cuda/errorhandling.h"

namespace tvseg {

TVSolverGPU::TVSolverGPU(const settings::BackendPtr backend):
    settings_(new SolverSettings(backend))
{
}

SolverSettings::ConstPtr TVSolverGPU::s()
{
    return settings_;
}

settings::SettingsPtr TVSolverGPU::settings() const
{
    return settings_;
}

void TVSolverGPU::computeSolution(const cv::Mat dataterm, const cv::Mat weight, const int numLabels, const Feedback *feedback)
{
    int rows = weight.rows;
    int cols = weight.cols;

    // sanity check
    if (! (dataterm.dims == 3 &&
           dataterm.size[0] == numLabels &&
           dataterm.size[1] == rows &&
           dataterm.size[2] == cols) )
    {
        LERRORF("Dimensions do not match in computeSolution. weight (%d,%d); dataterm (%d,%d,%d)",
                cols, rows, dataterm.size[2], dataterm.size[1], dataterm.size[0]);
        return;
    }

    // create u and initialize with 0, then set first region to 1
    const int sizeU[3] = {numLabels, rows, cols};
    cv::Mat u(3, sizeU, CV_32FC1);
    u.setTo(0.0);
    cv::Mat segmentation(rows, cols, CV_8UC1);
    //TODO:
//    cv::Mat region0(rows, cols, CV_32FC1, u.data);
//    region0.setTo(1.0);

    Dim3 dim;
    dim.width = cols;
    dim.height = rows;
    dim.labels = numLabels;

    cuda::SolverOutput output;

    try {
        cuda::computeSolution((unsigned char*) segmentation.data,
                              (float*) u.data,
                              (const float*) dataterm.data,
                              (const float*) weight.data,
                              dim,
                              s()->params(),
                              feedback,
                              output);
        LINFO << "Solver output: " << output;
    } catch (Exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "solver failed" << std::endl;
        return;
    }

    solution_ = segmentation;
}

} // namespace tvseg
