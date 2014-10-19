
#include "tvdatatermparzengpu.h"

#include "cuda/parzendataterm.h"
#include "cuda/colordataterm.h"

#include "tvseg/util/logging.h"


// TODO: what is appropriate here??
#define MINSCRIBBLEDISTANCEFACTOR 5

namespace tvseg {

TVDatatermParzenGPU::TVDatatermParzenGPU(const settings::BackendPtr backend):
    settings_(new ParzenSettings(backend))
{
}

ParzenSettings::ConstPtr TVDatatermParzenGPU::s()
{
    return settings_;
}

settings::SettingsPtr TVDatatermParzenGPU::settings() const
{
    return settings_;
}

void TVDatatermParzenGPU::computeDataterm(
        cv::Mat color,
        cv::Mat depth,
        const std::vector<float> &intrinsics,
        const Scribbles &scribbles,
        int numLabels,
        const Feedback *feedback)
{
    if (scribbles.getNumLabels() != numLabels) {
        LERROR << "passed scribbles for " << scribbles.getNumLabels() << " labels, but number of labels is " << numLabels;
        return;
    }

    if (intrinsics.size() != 9) {
        LERROR << "Expected intrinsics of size 9, but got size " << intrinsics.size();
        return;
    }
    if (intrinsics[1] != 0 || intrinsics[3] != 0 || intrinsics[6] != 0 || intrinsics[7] != 0 || intrinsics[8] != 1) {
        LWARNING << "Intrinsics array index 1, 3, 6, 7, 8 are ignored.";
    }

    cuda::Intrinsics intr;
    intr.s_x = intrinsics[0];
    intr.s_y = intrinsics[4];
    intr.o_x = intrinsics[2];
    intr.o_y = intrinsics[5];

    const int sizeDataterm[3] = {numLabels, color.rows, color.cols};
    dataterm_.create(3, sizeDataterm, CV_32FC1);

    boost::shared_array<int> scribblesFlat = scribbles.getScribblesFlat();
    boost::shared_array<int> numScribbles = scribbles.getNumScribblesFlat();
//    for (int i = 0; i < numLabels; ++i) {
//        LDEBUG << "count " << i+1 << ": " << numScribbles[i];
//    }

    Dim3 dim;
    dim.labels = numLabels;
    dim.height = color.rows;
    dim.width = color.cols;

    cuda::ParzenParams params = s()->params();
    // TODO: what is reasonable here?
//    if (params.scribbleDistanceFactor < MINSCRIBBLEDISTANCEFACTOR) {
//        LWARNING << "scribble distance factor of " << params.scribbleDistanceFactor << " too low; using " << MINSCRIBBLEDISTANCEFACTOR << " instead.";
//        params.scribbleDistanceFactor = MINSCRIBBLEDISTANCEFACTOR;
//    }

    cuda::computeParzenDataterm(
                (float*)dataterm_.data,
                (const float*)color.data,
                (const float*)depth.data,
                scribblesFlat.get(),
                numScribbles.get(),
                intr,
                dim,
                params,
                feedback);

}

} // namespace tvseg
