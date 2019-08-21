QT += core
QT -= gui

CONFIG += c++11

TARGET = dfont-install
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $$PWD/../libdeepin-font-manager
LIBS += -L$$OUT_PWD/../libdeepin-font-manager -ldeepin-font-manager

target.path = /usr/bin

INSTALLS += target
