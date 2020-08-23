TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lsfml-window -lsfml-graphics -lsfml-system -lsfml-audio

SOURCES += \
        cpu.cpp \
        main.cpp

HEADERS += \
    cpu.h
