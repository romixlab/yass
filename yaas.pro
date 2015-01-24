#-------------------------------------------------
#
# Project created by QtCreator 2015-01-21T21:26:53
#
#-------------------------------------------------

QT       += core network

QT       -= gui

LIBS += -lQtRedis -lqmsgpack
INCLUDEPATH += /home/roman/Qt/5.3/gcc_64/include/QtCore/5.3.1/QtCore/private/

TARGET = yaas
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    yass.cpp \
    dqobject.cpp \
    tester.cpp

HEADERS += \
    yass.h \
    dqobject.h \
    tester.h
