#include "logwindow.h"

LogWindow::LogWindow()
{
    this->setReadOnly(true);
    this->appendMessage("*** Welcome on MLViewer! ***", 0);
}


void LogWindow::appendMessage(const QString& text, int mode)
{
    QString date_log = QDate::currentDate().toString("ddd MMMM d yy");
    QString time_log = QTime::currentTime().toString();
    QString error = "EE";
    if (mode == 0)
    {
        if (text.indexOf(error) != -1)
        {
            QString alertHtml = "<font color=#083643>" + date_log + " " + time_log + " | " + text;
            this->appendHtml(alertHtml);
        }else
            this->appendPlainText(date_log + " " + time_log + " | " + text);

        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    }else if (mode == 1)
    {
        if (text.indexOf(error) != -1)
        {
            QString alertHtml = "<font color=#083643>" + text;
            this->appendHtml(alertHtml);
        }else
            this->appendPlainText(text);

        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    }
}
