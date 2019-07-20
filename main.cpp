#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include "d_format.hpp"


Q_DECLARE_METATYPE(Obj)
Q_DECLARE_METATYPE(SObj)
Q_DECLARE_METATYPE(std::shared_ptr<Frame>)
Q_DECLARE_METATYPE(P)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("ud1 soft");
    QCoreApplication::setOrganizationDomain("https://github.com/ud1");
    QCoreApplication::setApplicationName("ud1 viewer");

    qRegisterMetaType<Obj>("Obj");
    qRegisterMetaType<SObj>("SObj");
    qRegisterMetaType<std::shared_ptr<Frame>>("FramePtr");
    qRegisterMetaType<P>("P");
    qRegisterMetaType<std::string>("std::string");

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    RefsView w;
    w.show();

    return a.exec();
}
