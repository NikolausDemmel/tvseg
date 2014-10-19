#include "consoletextedit.h"

tvseg_ui::ConsoleTextEdit::ConsoleTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setStyleSheet("font-family: Courier New; font-size: 8pt;");
}

QSize tvseg_ui::ConsoleTextEdit::sizeHint() const
{
    return QSize(0, 100);
}
