#ifndef TVSEG_UI_FEEDBACKWIDGET_H
#define TVSEG_UI_FEEDBACKWIDGET_H

#include "tvseg/util/includeopencv.h"

#include <QWidget>
#include <QStateMachine>

namespace tvseg_ui {

namespace Ui {
class FeedbackWidget;
}

class FeedbackWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit FeedbackWidget(QWidget *parent = 0);
    ~FeedbackWidget();


    // TODO: is qstring thread safe? Else use shared poitner.
    // TODO: make overloads with pointers private?

    void displayCVImage(const cv::Mat &image, QString windowName);
    void displayCVImage(const cv::Mat &image, const std::string &windowName);

    void displayCVImageAsync(const cv::Mat &image, const std::string &windowName);
    void updateProgressAsync(int value, int max = 100, int min = 0);

    bool running();

public slots:
    void displayCVImage(QSharedPointer<cv::Mat> image, QSharedPointer<QString> windowName);
    void updateProgress(int value, int max = 100, int min = 0);
    void algorithmStarted();
    void algorithmStopped();
    void setRunning(bool value);

signals:
    void cancelRequested();
    void runningChanged(bool value);

private:
    QWidget* findTab(QString tabText);

private slots:
    void on_cancelButton_clicked();
    void on_tabWidget_tabCloseRequested(int index);
    void currentChanged(int index);

private:
    Ui::FeedbackWidget *ui_;
    QStateMachine *stateMachine_;

    bool running_;  ///< indicates whether an algorithm using this feedback widget is running or not
};


} // namespace tvseg_ui
#endif // TVSEG_FEEDBACKWIDGET_H
