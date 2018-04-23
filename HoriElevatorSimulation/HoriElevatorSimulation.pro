#-------------------------------------------------
#
# Project created by QtCreator 2016-08-16T09:34:31
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HoriElevatorSimulation
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    md5.c

HEADERS  += mainwindow.h \
    md5.h

FORMS    += mainwindow.ui
