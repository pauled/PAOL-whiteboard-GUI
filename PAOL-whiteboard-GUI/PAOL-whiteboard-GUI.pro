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
    paolMat.cpp \
    uf.cpp

HEADERS  += mainwindow.h \
    paolMat.h \
    uf.h

FORMS    += mainwindow.ui

LIBS += `pkg-config opencv --libs`
