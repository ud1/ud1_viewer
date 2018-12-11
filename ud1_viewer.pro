#-------------------------------------------------
#
# Project created by QtCreator 2018-11-26T19:12:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = ud1_viewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += nanovg/nanovg.c \
        main.cpp \
        mainwindow.cpp \
    view.cpp \
    settingsdialog.cpp \
    settings.cpp \
    tcpserver.cpp \
    camera.cpp \
    shader.cpp \
    d_format.cpp

HEADERS += \
        mainwindow.h \
    view.h \
    myutils.hpp \
    settingsdialog.h \
    settings.h \
    tcpserver.h \
    myutils3d.hpp \
    camera.h \
    shader.h \
    d_format.hpp

FORMS += \
        mainwindow.ui \
    settingsdialog.ui \
    settingsdialog.ui

INCLUDEPATH += "nanovg/"
LIBS += -lGL -lGLEW

CONFIG += c++1z

RESOURCES += \
    resources.qrc
