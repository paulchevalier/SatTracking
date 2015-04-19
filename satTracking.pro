#-------------------------------------------------
#
# Project created by QtCreator 2013-12-24T18:11:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = satTracking
TEMPLATE = app


SOURCES += main.cpp\
        sattrackmainwindow.cpp \
    spaceobject.cpp \
    sgp4unit.cpp \
    sgp4ext.cpp \
    sgp4io.cpp \
    satmapdialog.cpp

HEADERS  += sattrackmainwindow.h \
    spaceobject.h \
    sgp4unit.h \
    sgp4ext.h \
    sgp4io.h \
    satmapdialog.h

FORMS    += sattrackmainwindow.ui \
    satmapdialog.ui
