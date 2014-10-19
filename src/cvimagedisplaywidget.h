#ifndef TVSEG_UI_CVIMAGEDISPLAYWIDGET_H
#define TVSEG_UI_CVIMAGEDISPLAYWIDGET_H

#include "imagewidget.h"

#include <QWidget>

#include "tvseg/util/includeopencv.h"

class QLabel;
class QScrollArea;
class QStatusBar;
class QToolBar;
class QScrollBar;

namespace tvseg_ui {

class CvImageDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CvImageDisplayWidget(QWidget *parent = 0);

    virtual void setImage(cv::Mat image);

    virtual void clearImage();

signals:

public slots:

protected slots:
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitScale();
    virtual void updateImageDisplay();

protected:
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    QString positionValueString(QPoint widgetPos);
    void createActions();
    void updateActions();
    void updateInfo();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool adjustInputImage(const cv::Mat &input);
    virtual cv::Mat postProcessDisplayImage(cv::Mat image);
    virtual void postProcessDisplayPixmap(QPixmap &pixmap);

protected:
    cv::Mat image_;

    ImageWidget *imageWidget_;
    QScrollArea *imageScrollArea_;
    double scaleFactor_;

    QToolBar* toolBar_;
    QStatusBar* statusBar_;
    QLabel *infoLabel_;

    QAction *showStatusAct_;
    QAction *zoomInAct_;
    QAction *zoomOutAct_;
    QAction *normalSizeAct_;
    QAction *fitScaleAct_;
    QAction *remapValuesAct_;
    QAction *histEqualValuesAct_;
};

} // namespace tvseg_ui

#endif // TVSEG_UI_CVIMAGEDISPLAYWIDGET_H
