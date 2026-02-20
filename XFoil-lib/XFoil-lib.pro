
#    Compilation instructions:
#    https://flow5.tech/docs/flow5_doc/Source/Compilation.html


QT       -= gui

TARGET = XFoil
TEMPLATE = lib

DEFINES += XFOIL_LIBRARY

greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += c++20
} else {
    CONFIG += c++17
}


SOURCES += \
    xfoil.cpp

HEADERS +=\
    xfoil-lib_global.h \
    xfoil.h \
    xfoil_params.h

OBJECTS_DIR = ./objects

VERSION = 1.1.0

linux-g++{

    isEmpty(PREFIX){
        PREFIX = /usr/local
    }

    inc.path = $$PREFIX/include/$$TARGET
    inc.files += *.h


    target.path = $$PREFIX/lib

    # MAKE INSTALL
    INSTALLS += target inc

}


win32 {
#prevent qmake from making useless \debug and \release subdirs
    CONFIG -= debug_and_release debug_and_release_target

}



macx{
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MAC_SDK = macosx
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

