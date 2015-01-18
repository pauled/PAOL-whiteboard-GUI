#-------------------------------------------------
#
# Project created by QtCreator 2014-06-11T10:45:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PAOL-whiteboard-GUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    uf.cpp \
    clock.cpp \
    webcamimagescanner.cpp \
    datasetimagescanner.cpp \
    PAOLProcUtils.cpp

HEADERS  += mainwindow.h \
    uf.h \
    clock.h \
    ImageScanner.h \
    webcamimagescanner.h \
    datasetimagescanner.h \
    ImageProcessor.h \
    PAOLProcUtils.h

FORMS    += mainwindow.ui

LIBS += `pkg-config opencv --libs`
