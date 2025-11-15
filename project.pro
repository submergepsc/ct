QT += widgets
CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = inventory_manager

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/itemmodel.cpp

HEADERS += \
    src/mainwindow.h \
    src/itemmodel.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    resources.qrc

