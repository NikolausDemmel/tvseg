#ifndef TVSEG_UI_IMAGEWIDGET_H
#define TVSEG_UI_IMAGEWIDGET_H

#include <QLabel>

namespace tvseg_ui {

class ImageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = 0);

    bool scaling();

    void resetSize();
    QSize imageSize();
    double scaleFactor();

    QPoint imagePosFromWidgetPos(QPoint widgetPos);
signals:

public slots:
    void setPixmap(const QPixmap &p);
    void setScaling(bool scaling);

protected:
    void resizeEvent(QResizeEvent * event);

private:
    void updatePixmap();

private:
    QPixmap pixmap_;

    bool scaling_;

};

} // namespace tvseg_ui

#endif // TVSEG_UI_IMAGEWIDGET_H
