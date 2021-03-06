#-------------------------------------------------
#
# Project created by QtCreator 2021-02-28T21:40:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FCWin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cartridge.cpp \
    controller.cpp \
    cpu.cpp \
    cpu_ram.cpp \
    ppu.cpp \
    ppu_bus.cpp \
    Mapper/mapper_0.cpp \
    Mapper/mapper_1.cpp \
    Mapper/mapper_2.cpp

HEADERS  += mainwindow.h \
    cartridge.h \
    colors_map.h \
    controller.h \
    cpu.h \
    cpu_ram.h \
    ppu.h \
    ppu_bus.h \
    total.h \
    Mapper/mapper_0.h \
    Mapper/mapper_1.h \
    Mapper/mapper_2.h \
    Mapper/mapper_base.h

FORMS    += mainwindow.ui
