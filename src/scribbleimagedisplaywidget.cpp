
#include "scribbleimagedisplaywidget.h"

#include "tvseg/util/logging.h"
#include "tvseg/util/qtopencvhelper.h"

#include <QAction>
#include <QToolBar>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>


namespace tvseg_ui {


ScribbleImageDisplayWidget::ScribbleImageDisplayWidget(QWidget *parent):
    CvImageDisplayWidget(parent),
    scribbling_(false),
    erasing_(false),
    scribblingLabel_(0),
    brushSize_(5),
    brushDensity_(0.5),
    scribbleAct_(NULL),
    clearScribbleAct_(NULL)
{
    // TODO: change this to non arbitrary initialization
    labelColors_.append(qRgb(255, 0, 0));
    labelColors_.append(qRgb(0, 255, 0));

    scribbleAct_ = new QAction("scribble", this);
    scribbleAct_->setCheckable(true);
    scribbleAct_->setChecked(true);
    toolBar_->addAction(scribbleAct_);

    clearScribbleAct_ = new QAction("clear", this);
    connect(clearScribbleAct_, SIGNAL(triggered()), this, SLOT(clearScribbles()));
    toolBar_->addAction(clearScribbleAct_);
}

void ScribbleImageDisplayWidget::setLabels(cv::Mat labels)
{
    uint numLabels = labels.size[1];
    QVector<QRgb> colors(numLabels);
    for (uint i = 0; i < numLabels; ++i) {
        cv::Vec3f c = labels.at<cv::Vec3f>(0, i); // BGR
        colors[i] = qRgb(c[2]*255, c[1]*255, c[0]*255);
    }
    setLabels(colors);
}

void ScribbleImageDisplayWidget::setLabels(QVector<QRgb> labelColors)
{
    if (labelColors.size() > 255) {
        LERROR << "ScribbleImageDisplayWidget only supports up to 255 labels. Cannot use " << labelColors.size() << " labels.";
        return;
    }
    labelColors_ = labelColors;
    if (scribblingLabel_ >= numLabels()) {
        scribblingLabel_ = 0;
    }
    if (!image_.empty()) {
        setScribbleImageColorTable();
        updateImageDisplay();
    }
}

QVector<QVector<QPoint> > ScribbleImageDisplayWidget::getScribbles()
{
    QVector<QVector<QPoint> > scribbles(numLabels());
    for (int x = 0; x < scribbleImage_.width(); ++x) {
        for (int y = 0; y  < scribbleImage_.height(); ++y) {
            int label = scribbleImage_.pixelIndex(x, y);
            if (label >= 0 && label < numLabels()) {
                scribbles[label].append(QPoint(x,y));
            }
        }
    }
    return scribbles;
}

void ScribbleImageDisplayWidget::setScribbles(QVector<QVector<QPoint> > scribbles)
{
    scribbleImage_.fill(noScribblePixelValue());
    for (int i = 0; i < numLabels(); ++i) {
        foreach (QPoint p, scribbles[i]) {
            if (p.x() < scribbleImage_.width() && p.y() < scribbleImage_.height()) {
                scribbleImage_.setPixel(p, i);
            }
        }
    }
    updateImageDisplay();
}

uint ScribbleImageDisplayWidget::brushSize() const
{
    return brushSize_;
}

void ScribbleImageDisplayWidget::setBrushSize(uint value)
{
    brushSize_ = value;
}

float ScribbleImageDisplayWidget::brushDensity() const
{
    return brushDensity_;
}

void ScribbleImageDisplayWidget::setBrushDensity(float value)
{
    brushDensity_ = value;
}

void ScribbleImageDisplayWidget::createScribbleImage()
{
    if (image_.empty()) {
        scribbleImage_ = QImage();
    } else {
        scribbleImage_ = QImage(image_.cols, image_.rows, QImage::Format_Indexed8);
        setScribbleImageColorTable();
        scribbleImage_.fill(noScribblePixelValue());
    }
}

void ScribbleImageDisplayWidget::setScribbleImageColorTable()
{
    QVector<QRgb> table = labelColors_;
    for (int i = table.size(); i < 256; ++i) {
        table.append(qRgba(0, 0, 0, 0));
    }
    scribbleImage_.setColorTable(table);
}

void ScribbleImageDisplayWidget::postProcessDisplayPixmap(QPixmap &pixmap)
{
    paintScribbles(pixmap);
}

unsigned int ScribbleImageDisplayWidget::noScribblePixelValue()
{
    return 255; // we could use labelColors_.size(), but we expand the pixels color table to size 256 anyway
}

void ScribbleImageDisplayWidget::setImage(cv::Mat image)
{
    setImage(image, false);
}

void ScribbleImageDisplayWidget::setImage(cv::Mat image, bool keepImage)
{
    QVector<QVector<QPoint> > scribbles;
    if (keepImage) {
        scribbles = getScribbles();
    }
    if (!adjustInputImage(image))
        return;
    createScribbleImage();
    if (keepImage) {
        setScribbles(scribbles);
    } else {
        updateImageDisplay();
    }
    updateInfo();
    updateActions();
}

void ScribbleImageDisplayWidget::clearImage()
{
    image_ = cv::Mat();
    createScribbleImage();
    updateImageDisplay();
    updateInfo();
    updateActions();
}

bool ScribbleImageDisplayWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == imageWidget_) {

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (showStatusAct_->isChecked()) {
                infoLabel_->setText(positionValueString(mouseEvent->pos()));
                //statusBar_->showMessage(positionValueString(mouseEvent->pos()));
            }
            if (scribbleAct_->isChecked()) {
                QPoint imagePos = imageWidget_->imagePosFromWidgetPos(mouseEvent->pos());
                if ((mouseEvent->buttons() & Qt::LeftButton) && scribbling_) {
                    addScribble(imagePos, scribblingLabel_);
                } else if ((mouseEvent->buttons() & Qt::RightButton) && erasing_) {
                    eraseScribble(imagePos);
                }
            }
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (scribbleAct_->isChecked()) {
                QPoint imagePos = imageWidget_->imagePosFromWidgetPos(mouseEvent->pos());
                if (mouseEvent->button() == Qt::LeftButton) {
                    addScribble(imagePos, scribblingLabel_);
                    scribbling_ = true;
                    return true;
                } else if (mouseEvent->button() == Qt::RightButton) {
                    eraseScribble(imagePos);
                    erasing_ = true;
                    return true;
                }
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (scribbleAct_->isChecked()) {
                if (mouseEvent->button() == Qt::LeftButton && scribbling_) {
                    scribbling_ = false;
                    return true;
                } else if (mouseEvent->button() == Qt::RightButton && erasing_) {
                    erasing_ = false;
                    return true;
                }
            }
        }

    }

    // if not returned yet, pass to parent
    return CvImageDisplayWidget::eventFilter(obj, event);
}

void ScribbleImageDisplayWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
        int label = event->key() - Qt::Key_1;
        if (label < numLabels()) {
            setScribblingLabel(label);
        }
    }
}

void ScribbleImageDisplayWidget::addScribble(QPoint pos, unsigned int label)
{
    if (pos.x() < 0)
        return;
    tvseg::paintRectangleIndexed8(scribbleImage_, label, pos, brushSize(), brushDensity());
    updateImageDisplay();
}

void ScribbleImageDisplayWidget::eraseScribble(QPoint pos)
{
    if (pos.x() < 0)
        return;
    uint size = 19;
    float density = 1.0;
    tvseg::paintRectangleIndexed8(scribbleImage_, noScribblePixelValue(), pos, size, density);
    updateImageDisplay();
}

void ScribbleImageDisplayWidget::paintScribbles(QPixmap &dest)
{
    QPainter painter(&dest);
    painter.drawImage(QPoint(0, 0), scribbleImage_);
}

void ScribbleImageDisplayWidget::setScribblingLabel(int label)
{
    scribblingLabel_ = label;
    LDEBUG << "scribelling label " << scribblingLabel_+1;
}

int ScribbleImageDisplayWidget::numLabels()
{
    return labelColors_.size();
}

void ScribbleImageDisplayWidget::clearScribbles()
{
    scribbleImage_.fill(noScribblePixelValue());
    updateImageDisplay();
}


}
