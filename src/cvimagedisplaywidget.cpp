#include "cvimagedisplaywidget.h"

#include "tvseg/util/qtopencvhelper.h"
#include "tvseg/util/opencvhelper.h"
#include "tvseg/util/logging.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QEvent>
#include <QMouseEvent>

namespace tvseg_ui {

CvImageDisplayWidget::CvImageDisplayWidget(QWidget *parent) :
    QWidget(parent),
    scaleFactor_(1.0)
{
    // Image Widget and its scroll area
    imageWidget_ = new ImageWidget();
    imageWidget_->setMouseTracking(true);
    imageWidget_->installEventFilter(this);
    imageScrollArea_ = new QScrollArea;
    imageScrollArea_->setWidget(imageWidget_);

    // Create child widgets first
    infoLabel_ = new QLabel;
    statusBar_ = new QStatusBar;
    toolBar_ = new QToolBar;

    // All actions to be added to the toolbar
    createActions();
    toolBar_->addAction(showStatusAct_);
    toolBar_->addAction(fitScaleAct_);
    toolBar_->addAction(zoomInAct_);
    toolBar_->addAction(zoomOutAct_);
    toolBar_->addAction(normalSizeAct_);
    toolBar_->addAction(remapValuesAct_);
    toolBar_->addAction(histEqualValuesAct_);
    toolBar_->setMovable(false);
    toolBar_->setFloatable(false);

    // Status bar configuration
    statusBar_->setSizeGripEnabled(false);
    statusBar_->addPermanentWidget(infoLabel_);
    statusBar_->setVisible(showStatusAct_->isChecked());
    // fix frames around child widgets added by QStatusBar
    setStyleSheet("QStatusBar::item { border: 0px solid black };");

    // Create central layout and add widgets
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(toolBar_);
    centralLayout->addWidget(imageScrollArea_);
    centralLayout->addWidget(statusBar_);
    centralLayout->setStretch(1,1);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0,0,0,0);
    setLayout(centralLayout);

    // update image display and select fitscale as initial mode
    updateImageDisplay();
    fitScale();
}

void CvImageDisplayWidget::createActions()
{
    showStatusAct_ = new QAction("&info", this);
    showStatusAct_->setShortcut(tr("Ctrl+i"));
    showStatusAct_->setCheckable(true);
    showStatusAct_->setChecked(false);
    connect(showStatusAct_, SIGNAL(triggered(bool)), statusBar_, SLOT(setVisible(bool)));

    zoomInAct_ = new QAction("&+", this);
    zoomInAct_->setShortcut(tr("Ctrl++"));
    connect(zoomInAct_, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct_ = new QAction("&-", this);
    zoomOutAct_->setShortcut(tr("Ctrl+-"));
    connect(zoomOutAct_, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct_ = new QAction("&=", this);
    normalSizeAct_->setShortcut(tr("Ctrl+="));
    connect(normalSizeAct_, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitScaleAct_ = new QAction("&fit", this);
    fitScaleAct_->setShortcut(tr("Ctrl+f"));
    fitScaleAct_->setCheckable(true);
    fitScaleAct_->setChecked(true);
    connect(fitScaleAct_, SIGNAL(triggered()), this, SLOT(fitScale()));

    remapValuesAct_ = new QAction("&normalize", this);
    remapValuesAct_->setShortcut(tr("Ctrl+n"));
    remapValuesAct_->setCheckable(true);
    remapValuesAct_->setChecked(false);
    connect(remapValuesAct_, SIGNAL(triggered()), this, SLOT(updateImageDisplay()));

    histEqualValuesAct_ = new QAction("equal histo", this);
    histEqualValuesAct_->setCheckable(true);
    histEqualValuesAct_->setChecked(false);
    connect(histEqualValuesAct_, SIGNAL(triggered()), this, SLOT(updateImageDisplay()));


}

void CvImageDisplayWidget::updateActions()
{
    bool hasImage = !image_.empty();
    bool grayscale = (image_.channels() == 1);

    zoomInAct_->setEnabled(hasImage && !fitScaleAct_->isChecked());
    zoomOutAct_->setEnabled(hasImage && !fitScaleAct_->isChecked());
    normalSizeAct_->setEnabled(hasImage && !fitScaleAct_->isChecked());
    fitScaleAct_->setEnabled(hasImage);
    remapValuesAct_->setEnabled(hasImage && grayscale);
    histEqualValuesAct_->setEnabled(hasImage && grayscale);
}


void CvImageDisplayWidget::zoomIn()
{
    scaleImage(1.25);
    updateInfo();
}

void CvImageDisplayWidget::zoomOut()
{
    scaleImage(0.8);
    updateInfo();
}

void CvImageDisplayWidget::normalSize()
{
    scaleFactor_ = 1.0;
    imageWidget_->resetSize();
    updateInfo();
}

void CvImageDisplayWidget::fitScale()
{
    bool fit = fitScaleAct_->isChecked();
    imageScrollArea_->setWidgetResizable(fit);
    if (!fit) {
        normalSize();
    } else {
        updateInfo();
    }
    updateActions();
}


void CvImageDisplayWidget::setImage(cv::Mat image)
{
    if (!adjustInputImage(image))
        return;
    updateImageDisplay();
    updateInfo();
    updateActions();
}

void CvImageDisplayWidget::clearImage()
{
    image_ = cv::Mat();
    updateImageDisplay();
    updateInfo();
    updateActions();
}

void CvImageDisplayWidget::updateImageDisplay()
{
    if (image_.empty()) {
        imageWidget_->setPixmap(QPixmap());
    } else {
        cv::Mat tmp = image_;
        if (tmp.channels() == 1) {
            // grayscale image
            if (remapValuesAct_->isChecked()) {
                tmp = tvseg::rescaleGrayscaleImage(tmp);
            }
            if (histEqualValuesAct_->isChecked()) {
                cv::Mat tmp2;
                tmp.convertTo(tmp2, CV_8UC1, 255.0f);
                tmp = cv::Mat();
                cv::equalizeHist(tmp2, tmp);
            }
        }
        tmp = postProcessDisplayImage(tmp);
        QImage qimage = tvseg::matToImage(tmp);
        QPixmap pixmap = QPixmap::fromImage(qimage);
        postProcessDisplayPixmap(pixmap);
        imageWidget_->setPixmap(pixmap);
        if (!fitScaleAct_->isChecked()) {
            imageWidget_->resize(scaleFactor_ * imageWidget_->imageSize());
        }
    }
}

cv::Mat CvImageDisplayWidget::postProcessDisplayImage(cv::Mat image)
{
    // leave for derived classes
    return image;
}

void CvImageDisplayWidget::postProcessDisplayPixmap(QPixmap &/*pixmap*/)
{
    // leave for derived classes
}

void CvImageDisplayWidget::resizeEvent(QResizeEvent *)
{
    if (fitScaleAct_->isChecked()) {
        updateInfo();
    }
}

bool CvImageDisplayWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == imageWidget_) {

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (showStatusAct_->isChecked()) {
                infoLabel_->setText(positionValueString(mouseEvent->pos()));
                //statusBar_->showMessage(positionValueString(mouseEvent->pos()));
            }
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            LDEBUG << qPrintable(positionValueString(mouseEvent->pos()));
            return true;
        }

    }

    // if not returned yet, pass to parent
    return QWidget::eventFilter(obj, event);
}

QString CvImageDisplayWidget::positionValueString(QPoint widgetPos)
{
    QPoint p = imageWidget_->imagePosFromWidgetPos(widgetPos); // (-1,-1) means outside image
    QString str;

    str += QString("(%1,%2)").arg(p.x()).arg(p.y());

    if (p.x() >= 0) {
        // position inside image
        switch (image_.type()) {
        case CV_8UC1:
            {
                unsigned char val = image_.at<unsigned char>(p.y(),p.x());
                str += QString("  value: %1").arg(val, 3, 10, QChar('0'));
            }
            break;
        case CV_32FC1:
            {
                float val = image_.at<float>(p.y(),p.x());
                str += QString("  value: %1").arg(val,0,'f',3);
            }
            break;
        case CV_32FC3:
            {
                cv::Vec3f val = image_.at<cv::Vec3f>(p.y(),p.x());
                // BGR
                str += QString("  rgb: (%3,%2,%1)").arg(val(0),0,'f',3).arg(val(1),0,'f',3).arg(val(2),0,'f',3);
            }
            break;
        default:
            LWARNING << "positionValueString: Image type not supported.";
            break;
        }
    }

    return str;
}

void CvImageDisplayWidget::updateInfo()
{
    QString label;

    if (!image_.empty()) {
        double scale = imageWidget_->scaleFactor();
        label += QString("%1%  ").arg((int)(scale*100));
    }

    cv::Size size = image_.size();

    label += QString("%1 x %2  ").arg(size.width).arg(size.height);


    if (image_.empty()) {
        label += "empty";
    } else if (image_.channels() == 1) {
        label += "grayscale";
    } else {
        label += "color";
    }

    if (image_.empty()) {
        // no range info
    } else if (image_.channels() == 1) {
        // grayscale image; display value range
        double min, max;
        cv::minMaxLoc(image_, &min, &max);
        label += QString("  [%1;%2]").arg(min,0,'f',3).arg(max,0,'f',3);
    } else {
        // no range info
    }

    //infoLabel_->setText(label);
    statusBar_->showMessage(label);
}

void CvImageDisplayWidget::scaleImage(double factor)
{
    Q_ASSERT(imageWidget_->pixmap());
    scaleFactor_ *= factor;
    imageWidget_->resize(scaleFactor_ * imageWidget_->imageSize());

    adjustScrollBar(imageScrollArea_->horizontalScrollBar(), factor);
    adjustScrollBar(imageScrollArea_->verticalScrollBar(), factor);

    zoomInAct_->setEnabled(scaleFactor_ < 5.0);
    zoomOutAct_->setEnabled(scaleFactor_ > 0.2);
}

void CvImageDisplayWidget::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

bool CvImageDisplayWidget::adjustInputImage(const cv::Mat &input) {
    cv::Mat image = input.clone();

    switch (image.type()) {
    case CV_32FC3:
    case CV_32FC1:
    case CV_8UC1:
        break;
    default:
        LWARNINGF("Image type not supported.");
        return false;
    }

    switch (image.dims) {
    case 2:
        break;
    case 3:
        image = tvseg::flattenMultiRegionImage(image);
        break;
    default:
        LWARNINGF("Image dimension %d not supported.", image.dims);
        return false;
    }

    image_ = image;
    return true;
}


} // namespace tvseg_ui
