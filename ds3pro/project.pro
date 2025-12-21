TEMPLATE = app
CONFIG += c++17
CONFIG -= app_bundle
QT += widgets

SOURCES += main.cpp \
           coursedata.cpp \
           mainwindow.cpp \
           scheduler.cpp

HEADERS += coursedata.h \
           mainwindow.h \
           scheduler.h

FORMS += mainwindow.ui
