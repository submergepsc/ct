QT += widgets
CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = expression_evaluator

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/expressionevaluator.cpp

HEADERS += \
    src/mainwindow.h \
    src/expressionevaluator.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    resources.qrc
