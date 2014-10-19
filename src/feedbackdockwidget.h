#ifndef TVSEG_UI_FEEDBACKDOCKWIDGET_H
#define TVSEG_UI_FEEDBACKDOCKWIDGET_H

#include "feedbackwidget.h"

#include <QDockWidget>

namespace tvseg_ui {

namespace Ui {
class FeedbackDockWidget;
}

class FeedbackDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit FeedbackDockWidget(QWidget *parent = 0);
    ~FeedbackDockWidget();

    FeedbackWidget *getFeedbackWidget();

private:
    Ui::FeedbackDockWidget *ui_;
};


} // namespace tvseg_ui
#endif // TVSEG_UI_FEEDBACKDOCKWIDGET_H
