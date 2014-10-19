#ifndef TVSEG_UI_CONSOLETEXTEDIT_H
#define TVSEG_UI_CONSOLETEXTEDIT_H

#include <QPlainTextEdit>

namespace tvseg_ui {

class ConsoleTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ConsoleTextEdit(QWidget *parent = 0);

    QSize sizeHint() const;

signals:

public slots:

};

} // namespace tvseg_ui

#endif // TVSEG_UI_CONSOLETEXTEDIT_H
