#include "mlviewer.h"
#include <QApplication>
#include <QProxyStyle>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile File("../MLViewer/stylesheet.qss");
    File.open(QFile::ReadOnly);
    QString StyleSheet = QLatin1String(File.readAll());

    qApp->setStyleSheet(StyleSheet);
    mlviewer w;
    w.show();

    return a.exec();
}
