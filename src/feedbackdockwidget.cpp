#include "feedbackdockwidget.h"
#include "ui_feedbackdockwidget.h"

namespace tvseg_ui {

FeedbackDockWidget::FeedbackDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui_(new Ui::FeedbackDockWidget)
{
    ui_->setupUi(this);
}

FeedbackDockWidget::~FeedbackDockWidget()
{
    delete ui_;
}

FeedbackWidget *FeedbackDockWidget::getFeedbackWidget()
{
    return ui_->feedbackWidget;
}

} // namespace tvseg_ui
