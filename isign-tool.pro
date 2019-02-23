#-------------------------------------------------
#
# Project created by QtCreator 2018-10-14T17:24:51
#
#-------------------------------------------------

QT       += core gui
QT       += network
#QT       += webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = isign-tool
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

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    http.cpp \
    logindialog.cpp \
    expirepage.cpp \
    imd5.cpp \
    ithread.cpp \
    dialogsignrecord.cpp \
    ifile.cpp \
    common.cpp \
    ipainfo.cpp \
    signutil.cpp \
    signconfig.cpp \
    test.cpp

HEADERS += \
        mainwindow.h \
    http.h \
    logindialog.h \
    expirepage.h \
    imd5.h \
    ithread.h \
    dialogsignrecord.h \
    ifile.h \
    common.h \
    ipainfo.h \
    signutil.h \
    signconfig.h \
    test.h

FORMS += \
        mainwindow.ui \
    logindialog.ui \
    expirepage.ui \
    dialogsignrecord.ui

DISTFILES +=

RESOURCES += \
    dylib.qrc
