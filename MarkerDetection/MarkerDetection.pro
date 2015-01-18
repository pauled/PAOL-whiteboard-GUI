#-------------------------------------------------
#
# Project created by QtCreator 2015-01-18T11:08:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MarkerDetection
TEMPLATE = app


SOURCES += main.cpp \
    ../PAOL-whiteboard-GUI/datasetimagescanner.cpp \
    ../PAOL-whiteboard-GUI/imagescanner.cpp \
    ../PAOL-whiteboard-GUI/PAOLProcUtils.cpp \
    ../PAOL-whiteboard-GUI/uf.cpp

HEADERS  += \
    ../PAOL-whiteboard-GUI/datasetimagescanner.h \
    ../PAOL-whiteboard-GUI/PAOLProcUtils.h \
    ../PAOL-whiteboard-GUI/ImageScanner.h \
    ../PAOL-whiteboard-GUI/uf.h

LIBS += `pkg-config opencv --libs`
