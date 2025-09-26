#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T04:06:28
#
#-------------------------------------------------

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT       -= gui

TARGET = XFoil
TEMPLATE = lib

DEFINES += XFOILLIB_LIBRARY

SOURCES += \
    xfoil.cpp

HEADERS +=\
    xfoil-lib_global.h \
    xfoil.h \
    xfoil_params.h

OBJECTS_DIR = ./objects

win32 {
#prevent qmake from making useless \debug and \release subdirs
    CONFIG -= debug_and_release debug_and_release_target

}



macx{
#    CONFIG += lib_bundle
#    CONFIG += i386
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MAC_SDK = macosx
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}


linux-g++{

    isEmpty(PREFIX){
        PREFIX = /usr/local
    }

    INCLUDEDIR = $$PREFIX/include
    inc.path = $$INCLUDEDIR
    inc.files += xfoil.h xfoil_params.h


    LIBDIR = $$PREFIX/lib
    target.path = $$PREFIX/lib

    # MAKE INSTALL
    INSTALLS += target inc

}

