#ifndef TVSEG_UI_CONSOLEDOCKWIDGET_H
#define TVSEG_UI_CONSOLEDOCKWIDGET_H

#include <QDockWidget>

namespace tvseg_ui {

namespace Ui {
class ConsoleDockWidget;
}

class ConsoleDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ConsoleDockWidget(QWidget *parent = 0);
    ~ConsoleDockWidget();

public slots:
    void addMessage(QString msg, QColor color);

private:
    Ui::ConsoleDockWidget *ui_;
};


} // namespace tvseg_ui
#endif // TVSEG_UI_CONSOLEDOCKWIDGET_H
