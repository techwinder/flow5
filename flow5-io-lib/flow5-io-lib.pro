
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


OBJECTS_DIR = ./objects
MOC_DIR     = ./moc
RCC_DIR     = ./rcc


linux-g++ {


    DEFINES += LINUX_OS

    isEmpty(PREFIX){
        PREFIX = /usr/local
    }

#-----XFoil-----
    LIBS += -L../XFoil-lib -lXFoil

    INCLUDEDIR = $$PREFIX/include/flow5
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
    LIBS += -L/usr/lib64/            #fedora install location


    #prevent sfinae warnings in the Qt libs
    QMAKE_CXXFLAGS += -Wsfinae-incomplete=0


}


win32-msvc {

    DEFINES += WIN_OS

    CONFIG -= debug_and_release debug_and_release_target


#------------ OPEN CASCADE --------------------------
    INCLUDEPATH += D:\bin\OCCT-7_9_2\build\inc
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\lib
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\bin


#--------------------- GMSH ------------------------
    INCLUDEPATH += D:\bin\gmsh-4.14.1-Windows64-sdk/include/
    LIBS += -L"D:\bin\gmsh-4.14.1-Windows64-sdk/lib"
    LIBS += -lgmsh.dll  # the file name is gmsh.dll.lib


#-----XFoil-----
    LIBS += -L../XFoil-lib -lXFoil1

}


macx {

    # app_bundle	Puts the executable into a bundle (this is the default).
    CONFIG -= app_bundle

    # No Qt macro, make our own
    DEFINES += MAC_OS

    # Specifies the hard minimum version of macOS that the application supports.
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.3   # needed for c++20 / std::format

    # This variable is used on macOS when building universal binaries.
    QMAKE_MAC_SDK = macosx

    # Specifies a list of architectures to build for.
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

    # If defined, the value of this variable is used as a path to be prepended to
    # the built shared library's SONAME identifier.
    # The SONAME is the identifier that the dynamic linker will later use to reference the library.
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks


    #-------------OPENCASCADE -----------------
    INCLUDEPATH += /usr/local/include/opencascade
    LIBS += -L/usr/local/lib


    #-------Lib install ----------------
    isEmpty(PREFIX){
        PREFIX = /usr/local
    }

    INCLUDEDIR = $$PREFIX/include/flow5
    inc.path = $$INCLUDEDIR
    inc.files += api/*.h

    target.path = $$PREFIX/lib

    # MAKE INSTALL
    INSTALLS += target inc

}


include (flow5-io-lib.pri)


#----- OCC -----

LIBS += \
    -lTKBO \
    -lTKBRep \
    -lTKBool \
    -lTKCDF \
    -lTKDESTEP \
    -lTKDEOBJ \
    -lTKDESTL \
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




