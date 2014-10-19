
#include "qtopencvhelper.h"

#include "tvseg/util/logging.h"

namespace tvseg {



//cv::Mat imageToMat(QImage const& src)
//{
//    int type;
//    cv::Mat result;

//    switch (src.format()) {
//    case QImage::Format_Indexed8:
//        cv::Mat tmp(src.height(), src.width(), CV_8UC1, src.data_ptr());
//        tmp.convertTo(result, CV_32FC1, 1/255.0);
//        break;
//    default:
//        LWARNING << "Conversion not implemented...";
//        return;
//    }

//    return result;
//}

QImage matToImage(cv::Mat const& src)
{
    cv::Mat temp;

    QImage::Format format;

    switch (src.type()) {
    case CV_32FC3:   // color with ranges: 0 ... 1
        src.convertTo(temp, CV_8UC3, 255.0, 0);
        cv::cvtColor(temp, temp, CV_BGR2RGB);
        format = QImage::Format_RGB888;
        break;
    case CV_32FC1:   // grayscale with ranges 0 ... 1
        src.convertTo(temp, CV_8UC1, 255.0, 0);
        format = QImage::Format_Indexed8;
        break;
    case CV_8UC1:   // grayscale with ranges 0 ... 255
        temp = src;
        format = QImage::Format_Indexed8;
        break;
    default:
        LWARNING << "Conversion not implemented.";
        return QImage();
    }

    // convert color from BGR to RGB
    cv::Mat temp2 = temp;
//    cv::cvtColor(temp, temp2, CV_BGR2RGB);

    QImage dest((uchar*) temp2.data, temp2.cols, temp2.rows, temp2.step, format);

    if (QImage::Format_Indexed8 == format) {
        // create palette
        for (int i = 0; i < 256; ++i) {
            dest.setColor(i, qRgb(i,i,i));
        }
    }

    QImage dest2(dest);
    dest2.detach(); // enforce deep copy
    return dest2;
}

void getGrayscaleImageMinMax(const QImage image, double *pmin, double *pmax, bool rescaleRange)
{
    double min = 0;
    double max = 1;

    if (image.format() != QImage::Format_Indexed8) {
        LWARNINGF("Image not grayscale; cannot compute min/max; returning default;");
    } else {
        const cv::Mat tmp(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
        cv::minMaxLoc(tmp, &min, &max);
        if (rescaleRange) {
            min /= 255.;
            max /= 255.;
        }
    }

    if (pmin != 0) {
        *pmin = min;
    }
    if (pmax != 0) {
        *pmax = max;
    }
}


namespace {

int cutoffX(QImage &image, int x) {
    return std::max(0, std::min(image.width()-1, x));
}

int cutoffY(QImage &image, int y) {
    return std::max(0, std::min(image.height()-1, y));
}

} // namespace

void paintRectangleIndexed8(QImage &image, uint value, QPoint pos, uint size, float density)
{
    uint radius = size/2;
    QRect box(pos.x() - radius, pos.y() - radius, size, size);
    box.setLeft(cutoffX(image, box.left()));
    box.setTop(cutoffY(image, box.top()));
    box.setRight(cutoffX(image, box.right()));
    box.setBottom(cutoffY(image, box.bottom()));
    for (int x = box.left(); x <= box.right(); ++x) {
        for (int y = box.top(); y <= box.bottom(); ++y) {
            if ((rand() / (float)RAND_MAX) < density) {
                image.setPixel(x, y, value);
            }
        }
    }
}

} // namespace tvseg
