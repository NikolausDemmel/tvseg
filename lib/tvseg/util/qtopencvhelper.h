#ifndef TVSEG_UTIL_QTOPENCVHELPER_H
#define TVSEG_UTIL_QTOPENCVHELPER_H

#include "tvseg/util/includeopencv.h"

//#include "tvseg/params.h"

#include <QImage>

namespace tvseg {

void getGrayscaleImageMinMax(const QImage image, double *min, double *max, bool rescaleRange = true);

QImage matToImage(const cv::Mat & src);

//cv::Mat imageToMat(QImage const& src);

void paintRectangleIndexed8(QImage &image, uint value, QPoint pos, uint size, float density);


} // namespace tvseg

#endif // TVSEG_UTIL_QTOPENCVHELPER_H
