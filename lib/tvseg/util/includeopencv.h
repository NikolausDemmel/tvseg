
// disbale noisy warnings with the opencv headers on clang.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

#include <opencv2/opencv.hpp>

#pragma clang diagnostic pop
