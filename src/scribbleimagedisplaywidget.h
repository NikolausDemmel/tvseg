#ifndef TVSEG_UI_SCRIBBLEIMAGEWIDGET_H
#define TVSEG_UI_SCRIBBLEIMAGEWIDGET_H

#include "cvimagedisplaywidget.h"

namespace tvseg_ui {


class ScribbleImageDisplayWidget: public CvImageDisplayWidget
{
    Q_OBJECT

public:
    explicit ScribbleImageDisplayWidget(QWidget *parent = 0);

    void setImage(cv::Mat image);
    void setImage(cv::Mat image, bool keepImage);
    void clearImage();

    void setLabels(cv::Mat labels);
    void setLabels(QVector<QRgb> labelColors);

    QVector<QVector<QPoint> > getScribbles();
    void setScribbles(QVector<QVector<QPoint> > scribbles);

    uint brushSize() const;
    void setBrushSize(uint value);

    float brushDensity() const;
    void setBrushDensity(float value);

protected slots:
    void clearScribbles();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent * event);

    void addScribble(QPoint pos, unsigned int label);
    void eraseScribble(QPoint pos);

    void createScribbleImage();
    unsigned int noScribblePixelValue();

    void paintScribbles(QPixmap &dest);

    void setScribblingLabel(int label);

    int numLabels();

    void setScribbleImageColorTable();

    void postProcessDisplayPixmap(QPixmap &pixmap);

protected:
    bool scribbling_;
    bool erasing_;
    QVector<QRgb> labelColors_;
    QImage scribbleImage_;
    int scribblingLabel_;
    uint brushSize_;
    float brushDensity_;

    QAction *scribbleAct_;
    QAction *clearScribbleAct_;
};

}


#endif // TVSEG_UI_SCRIBBLEIMAGEWIDGET_H
