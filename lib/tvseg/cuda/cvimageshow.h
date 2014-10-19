#ifndef TVSEG_CUDA_CVIMAGESHOW_H
#define TVSEG_CUDA_CVIMAGESHOW_H

#include <string>

namespace tvseg {
namespace cuda {

    void cvImageShow(float* data, int rows, int cols, int numLabels, const char* winName);

}
}

#endif // TVSEG_CUDA_CVIMAGESHOW_H
