
#    Compilation instructions:
#    https://flow5.tech/docs/flow5_doc/Source/Compilation.html

DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TEMPLATE = app
TARGET = flow5

VERSION = 7.54

QT += opengl widgets xml

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}

OBJECTS_DIR = ./objects
MOC_DIR     = ./moc
RCC_DIR     = ./rcc
DESTDIR     = .


CONFIG(release, debug|release) {
    CONFIG += optimize_full
}

greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += c++20
} else {
    CONFIG += c++17
}

# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../XFoil-lib/


#The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../fl5-lib/
INCLUDEPATH += $$PWD/../fl5-lib/api



linux-g++ {
    CONFIG += occt
    CONFIG += thread

    # VARIABLES
    isEmpty(PREFIX):PREFIX = /usr/local
    BINDIR = $$PREFIX/bin
    DATADIR = $$PREFIX/share/flow5

    desktop.path = $$(HOME)/.local/share/applications
    desktop.files += ../meta/linux/$${TARGET}.desktop

    icon128.path = $$DATADIR
    icon128.files += ../meta/res/$${TARGET}.png

    target.path = $$BINDIR

    # MAKE INSTALL
    INSTALLS += target desktop icon128

#    CONFIG += INTEL_MKL

    INTEL_MKL {
        #------------ MKL --------------------
        #    MKL can use the c++ matrices in row major order
        DEFINES += INTEL_MKL

        #   Ensure that the paths to the include files and to the binary libraries
        #   are set either by defining them in the environment variables
        #   or by setting them explicitely in the following two lines
        #
          INCLUDEPATH += /opt/intel/oneapi/mkl/latest/include/
          LIBS += -L/opt/intel/oneapi/mkl/latest/lib/intel64/

        #   The mkl libs to include may depend on MKL's version;
        #   Follow Intel's procedure to determine which libs to include
            LIBS += -lmkl_core -lmkl_intel_lp64  -lmkl_gnu_thread
        #   LIBS += -lgomp
        ##    LIBS += -lmkl_intel_thread -lmkl_sequential
    } else {
        # ---------------- system LAPACK/LAPACKE + CBLAS/OpenBLAS-----------------------------
        DEFINES += OPENBLAS

            LIBS += -llapack -llapacke
            LIBS += -lopenblas

    }

    #----------- OPENCASCADE -------------
    #   The include paths to the development headers must be set explicitely
    INCLUDEPATH += /usr/local/include/opencascade/
    #   Ensure that the paths to the binary libraries
    #   are known either by defining them at system level
    #   or by setting them explicitely in this section
    LIBS += -L/usr/local/lib/            # redundant



    #--------------------- GMSH ------------------------
    INCLUDEPATH += /usr/local/include/
#    LIBS += -L/usr/local/lib64           # redundant
    LIBS += -lgmsh


    #-----XFoil----
    LIBS += -L../XFoil-lib -lXFoil
}


win32-g++ {

    CONFIG += console
    CONFIG -= debug_and_release debug_and_release_target

    RC_ICONS = ../meta/win64/flow5.ico

    #-----XFoil----
    LIBS += -L../XFoil-lib -lXFoil1

    #------------ OPEN CASCADE / OCCT (MinGW) --------------------------
    OCCT_DIR = D:/flow5/OCCT/build-mingw-dll
    isEmpty(OCCT_DIR): OCCT_DIR = $$getenv(OCCT_DIR)
    isEmpty(OCCT_INC): OCCT_INC = $$getenv(OCCT_INC)
    isEmpty(OCCT_LIB): OCCT_LIB = $$getenv(OCCT_LIB)

    isEmpty(OCCT_INC): !isEmpty(OCCT_DIR): OCCT_INC = $$OCCT_DIR/inc
    isEmpty(OCCT_LIB): !isEmpty(OCCT_DIR): OCCT_LIB = $$OCCT_DIR/win64/gcc/lib

    !isEmpty(OCCT_INC) {
        INCLUDEPATH += $$OCCT_INC
        CONFIG += occt
    } else {
        message("OCCT_INC/OCCT_DIR not set: OCCT headers are required.")
    }

    !isEmpty(OCCT_LIB) {
        LIBS += -L$$OCCT_LIB
    }

    #---------------- OTHER WIN LIBS -------------------
    LIBS += -lopengl32
}



win32-msvc {

    CONFIG += occt

    CONFIG += console
    CONFIG -= debug_and_release debug_and_release_target

    RC_ICONS = ../meta/win64/flow5.ico


#-----XFoil----
    LIBS += -L../XFoil-lib -lXFoil1

#----------------------- MKL  ---------------------
    DEFINES += INTEL_MKL   #only option in Windows
    INCLUDEPATH += "C:\Program Files (x86)\Intel\oneAPI\mkl\latest\include"

    LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\mkl\latest\bin"
    LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\mkl\latest\lib"
    LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\compiler\latest\bin"
    LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\compiler\latest\lib"
    LIBS += -lmkl_intel_lp64_dll
    LIBS += -lmkl_core_dll
    LIBS += -lmkl_intel_thread_dll -llibiomp5md  # for multithreading
    #    LIBS += -lmkl_sequential_dll



#--------------------- GMSH ------------------------
    INCLUDEPATH += D:\bin\gmsh-4.14.1-Windows64-sdk/include/
    LIBS += -L"D:\bin\gmsh-4.14.1-Windows64-sdk/lib"
    LIBS += -lgmsh.dll  # the file name is gmsh.dll.lib


#------------ OPEN CASCADE --------------------------
    INCLUDEPATH += D:\bin\OCCT-7_9_2\build\inc
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\lib
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\bin



#---------------- OTHER WIN LIBS -------------------
    DEFINES += _UNICODE WIN64 QT_DLL QT_WIDGETS_LIB
    LIBS += -lopengl32

    #hide the console
    LIBS += -lKernel32 -lUser32
}



macx {
    CONFIG += occt
    QMAKE_MAC_SDK = macosx
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

    # Add variables that will be used to build the info.plist file
    QMAKE_TARGET_BUNDLE_PREFIX = cere-aero.tech

    QMAKE_INFO_PLIST = ../meta/mac/info.plist
    ICON = ../meta/mac/flow5.icns

    DEFINES += GL_SILENCE_DEPRECATION   #Shame

    #-------XFoil
    # link to the lib:
    LIBS += -L$$OUT_PWD/../XFoil-lib -lXFoil
    # deploy the libs:
    XFoil.files = $$OUT_PWD/../XFoil-lib/libXFoil.1.dylib
    XFoil.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += XFoil

    #-------fl5-lib
    # link to the lib:
    LIBS += -L$$OUT_PWD/../fl5-lib -lfl5-lib
    # deploy the libs:
    fl5-lib.files = $$OUT_PWD/../fl5-lib/libfl5-lib.1.dylib
    fl5-lib.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += fl5-lib


    #-------------OPENCASCADE -----------------
    # set the paths to the OpenCascade header and lib directories
    INCLUDEPATH += /usr/local/include/opencascade
    LIBS += -L/usr/local/lib

    #_____________GMSH__________________
    INCLUDEPATH += /usr/local/include
    LIBS += -lgmsh

    # deploy the libs
#    gmsh.files =/usr/local/lib/libgmsh.4.14.dylib
#    gmsh.path = Contents/Frameworks
#    QMAKE_BUNDLE_DATA += gmsh

}


#CONFIG += warn_on
#QMAKE_CFLAGS_WARN_ON += -W3
#QMAKE_CFLAGS_WARN_ON += -W4



include(fl5-app.pri)



RESOURCES += \
    resources/qss.qrc \
    resources/icons.qrc \
    resources/images.qrc \
    resources/sailimages.qrc

TRANSLATIONS += \
    translations/flow5_zh_CN.ts


LIBS += -L../fl5-lib -lfl5-lib

occt {
    LIBS += \
        -lTKBRep \
        -lTKBO \
        -lTKG3d \
        -lTKGeomAlgo \
        -lTKGeomBase \
        -lTKLCAF \
        -lTKMath \
        -lTKMesh \
        -lTKOffset \
        -lTKPrim \
        -lTKDESTEP \
        -lTKShHealing \
        -lTKTopAlgo \
        -lTKXSBase \
        -lTKernel \
        -lTKBool \
        -lTKG2d \
        -lTKCDF \
        -lTKFillet \
}

DISTFILES += \
    ../meta/doc/images/flow5.png \
    ../meta/doc/releasenotes.html \
    ../meta/doc/style.css \
    ../meta/win64/flow5.ico \
    ../meta/win64/flow5_doc.ico

win32-g++ {
    DEFINES += NO_GMSH
    # SOURCES -= interfaces/mesh/gmesh_globals.cpp
    # SOURCES += interfaces/mesh/gmesh_globals_stub.cpp
}
