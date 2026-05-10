
#    Compilation instructions:
#    https://flow5.tech/docs/flow5_doc/Source/Compilation.html

QT -= gui

TEMPLATE = lib
DEFINES += FLOW5_IO_LIB

CONFIG += c++20
TARGET = flow5-io-lib

#The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../flow5-lib/
INCLUDEPATH += $$PWD/../flow5-lib/api

LIBS += -L../flow5-lib -lflow5-lib
INCLUDEPATH += ./api

#-----XFoil----
# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../XFoil-lib/


linux-g++ {


    DEFINES += LINUX_OS

    isEmpty(PREFIX){
        PREFIX = /usr/local
    }

#-----XFoil-----
    LIBS += -L../XFoil-lib -lXFoil

    INCLUDEDIR = $$PREFIX/include/$$TARGET
    inc.path = $$INCLUDEDIR
    inc.files += api/*.h

    target.path = $$PREFIX/lib

    # MAKE INSTALL
    INSTALLS += target inc

    #----------- OPENCASCADE -------------
    #   Ensure that the paths to the binary libraries
    #   are known either by defining them at system level
    #   or by setting them explicitely in this section
    #   The include paths to the development headers must be set explicitely
    INCLUDEPATH += /usr/local/include/opencascade/  #make install location
    INCLUDEPATH += /usr/include/opencascade/        #fedora install location
    LIBS += -L/usr/local/lib/

}


win32-msvc {

    DEFINES += WIN_OS

    CONFIG -= debug_and_release debug_and_release_target


#------------ OPEN CASCADE --------------------------
    INCLUDEPATH += D:\bin\OCCT-7_9_2\build\inc
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\lib
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\bin

#-----XFoil-----

    LIBS += -L../XFoil-lib -lXFoil1

}



include (flow5-io-lib.pri)


#----- OCC -----

LIBS += \
    -lTKBO \
    -lTKBRep \
    -lTKBool \
    -lTKCDF \
    -lTKDESTEP \
    -lTKFillet \
    -lTKG2d \
    -lTKG3d \
    -lTKGeomAlgo \
    -lTKGeomBase \
    -lTKLCAF \
    -lTKMath \
    -lTKMesh \
    -lTKOffset \
    -lTKPrim \
    -lTKShHealing \
    -lTKTopAlgo \
    -lTKXSBase \
    -lTKernel \


