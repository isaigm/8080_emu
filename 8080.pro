TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -L/usr/local/lib -lSDL2 -lsfml-window -lsfml-graphics -lsfml-system

SOURCES += \
        cpu.cpp \
        main.cpp

HEADERS += \
    cpu.h
