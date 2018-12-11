#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include "d_format.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("ud1 soft");
    QCoreApplication::setOrganizationDomain("https://github.com/ud1");
    QCoreApplication::setApplicationName("ud1 viewer");

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    RefsView w;
    w.show();

    return a.exec();
}
