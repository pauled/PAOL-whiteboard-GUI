#-------------------------------------------------
#
# Project created by QtCreator 2014-04-16T14:08:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PAOL-Whiteboard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    paolMat.cpp \
    timer.cpp \
    shift.cpp \
    seglist.cpp

HEADERS  += mainwindow.h \
    paolMat.h \
    timer.h \
    shift.h \
    seglist.h

FORMS    += mainwindow.ui

LIBS += `pkg-config opencv --libs`
