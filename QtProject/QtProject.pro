#-------------------------------------------------
#
# Project created by QtCreator 2015-01-04T01:09:23
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

win32: RC_ICONS = IconApp1.ico

TARGET = ControlYourArduino
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    util.cpp \
    QtArduino.cpp \
    qcustomplot.cpp


HEADERS  += dialog.h \
    util.h \
    QtArduino.h \
    qcustomplot.h



FORMS    += dialog.ui

INCLUDEPATH +=

LIBS += -lanalogwidgetsd

DISTFILES +=

RESOURCES += \
    imagenarduino.qrc


