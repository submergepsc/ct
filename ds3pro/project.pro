TEMPLATE = app
CONFIG += c++17
CONFIG -= app_bundle
QT += widgets

SOURCES += main.cpp \
           scheduler.cpp

HEADERS += scheduler.h

FORMS += schedulerwidget.ui
