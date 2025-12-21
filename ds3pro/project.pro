TEMPLATE = app
CONFIG += c++17
CONFIG -= app_bundle
QT += widgets

SOURCES += main.cpp \
           mainwindow.cpp \
           scheduler.cpp

HEADERS += mainwindow.h \
           scheduler.h

FORMS += mainwindow.ui
