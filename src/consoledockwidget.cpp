#include "consoledockwidget.h"
#include "ui_consoledockwidget.h"

namespace tvseg_ui {

ConsoleDockWidget::ConsoleDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui_(new Ui::ConsoleDockWidget)
{
    ui_->setupUi(this);
}

ConsoleDockWidget::~ConsoleDockWidget()
{
    delete ui_;
}

void ConsoleDockWidget::addMessage(QString msg, QColor color)
{
    QTextCharFormat tf;
    tf = ui_->textEdit->currentCharFormat();
    tf.setForeground(QBrush(color));
    ui_->textEdit->setCurrentCharFormat(tf);
    ui_->textEdit->appendPlainText(msg);
}

} // namespace tvseg_ui
