#-------------------------------------------------
#
# Project created by QtCreator 2020-11-12T01:09:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Step_06
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        Mapper/mapper_0.cpp \
        Mapper/mapper_1.cpp \
        Mapper/mapper_2.cpp \
        apu.cpp \
        cartridge.cpp \
        controller.cpp \
        cpu.cpp \
        cpu_ram.cpp \
        main.cpp \
        mainwindow.cpp \
        ppu.cpp \
        ppu_bus.cpp

HEADERS += \
        Mapper/mapper_0.h \
        Mapper/mapper_1.h \
        Mapper/mapper_2.h \
        Mapper/mapper_base.h \
        apu.h \
        audio_map.h \
        cartridge.h \
        colors_map.h \
        controller.h \
        cpu.h \
        cpu_ram.h \
        mainwindow.h \
        ppu.h \
        ppu_bus.h \
        total.h

FORMS += \
        mainwindow.ui

# LIBS += \
#         -L/usr/lib/x86_64-linux-gnu -lopenal
LIBS += -lopenal

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
