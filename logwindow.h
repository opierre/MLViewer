#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QObject>
#include <QtWidgets>

class LogWindow : public QPlainTextEdit
{
    Q_OBJECT

public:
    LogWindow();
    void appendMessage(const QString& text, int mode);
};

#endif // LOGWINDOW_H
