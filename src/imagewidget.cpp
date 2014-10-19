#include "imagewidget.h"
#include <math.h>

namespace tvseg_ui {

ImageWidget::ImageWidget(QWidget *parent) :
    QLabel(parent),
    scaling_(true)
{
    setMinimumSize(1,1);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    //setStyleSheet("QLabel { background-color : white}");
}

bool ImageWidget::scaling()
{
    return scaling_;
}

void ImageWidget::resetSize()
{
    if (!pixmap_.isNull()) {
        resize(pixmap_.size());
    }
}

QSize ImageWidget::imageSize()
{
    if (!pixmap_.isNull()) {
        return pixmap_.size();
    } else {
        return QSize(0,0);
    }
}

double ImageWidget::scaleFactor()
{
    QSize size = imageSize();
    if (size.width() <= 0 || size.height() <= 0) {
        return 1;
    }
    int w = width();
    int h = height();
    QSize scale = size.scaled(w, h, Qt::KeepAspectRatio);
    return ((double)scale.width()) / size.width();
}

QPoint ImageWidget::imagePosFromWidgetPos(QPoint widgetPos)
{
    QPoint pos(floor(widgetPos.x() / scaleFactor()),
               floor(widgetPos.y() / scaleFactor()));
    QSize size = imageSize();

    if (pos.x() >= size.width() || pos.y() >= size.height() || pos.x() < 0 || pos.y() < 0) {
        return QPoint(-1,-1);
    } else {
        return pos;
    }
}

void ImageWidget::setPixmap(const QPixmap &p)
{
    pixmap_ = p;
    updatePixmap();
}

void ImageWidget::setScaling(bool scaling)
{
    scaling_ = scaling;
    updatePixmap();
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updatePixmap();
}

void ImageWidget::updatePixmap()
{
    if (!pixmap_.isNull())
    {
        if (scaling_) {
            int w = width();
            int h = height();
            QLabel::setPixmap(pixmap_.scaled(w, h, Qt::KeepAspectRatio));
        } else {
            QLabel::setPixmap(pixmap_);
        }
    } else {
        if (QLabel::pixmap() && !QLabel::pixmap()->isNull()) {
            QLabel::setPixmap(pixmap_);
        }
    }
}

} // namespace tvseg_ui
