#-------------------------------------------------
#
# Project created by QtCreator 2018-07-07T20:54:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MLTD Asset Downloader
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


SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
        src/utils.cpp \
        src/progresswindow.cpp \
        src/filedownloader.cpp

HEADERS += \
        src/mainwindow.h \
        src/utils.h \
        src/progresswindow.h \
        src/filedownloader.h \
        curl/curl.h \
        curl/curlver.h \
        curl/easy.h \
        curl/mprintf.h \
        curl/multi.h \
        curl/stdcheaders.h \
        curl/system.h \
        curl/typecheck-gcc.h

FORMS += \
        mainwindow.ui \
        progresswindow.ui

LIBS += \
        -L$$PWD/curl/ -llibcurl
