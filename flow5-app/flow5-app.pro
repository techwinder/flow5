
#    Compilation instructions:
#    https://flow5.tech/docs/flow5_doc/Source/Compilation.html

DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TEMPLATE = app
TARGET = flow5

VERSION = 7.57

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

CONFIG += c++20

# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../XFoil-lib/


#The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../flow5-lib/
INCLUDEPATH += $$PWD/../flow5-lib/api

INCLUDEPATH += $$PWD/../flow5-io-lib/
INCLUDEPATH += $$PWD/../flow5-io-lib/api


linux-g++ {

    # ----------------- Install-------------------------
    isEmpty(PREFIX):PREFIX = /usr/local
    BINDIR = $$PREFIX/bin
    SHAREDIR = $$PREFIX/share/flow5

    desktop.path = $$(HOME)/.local/share/applications
    desktop.files += ../meta/linux/$${TARGET}.desktop

    icon128.path = $$SHAREDIR
    icon128.files += ../meta/res/$${TARGET}.png

    translations.path = $$SHAREDIR/translations
    translations.files += ../meta/translations/*.qm

    target.path = $$BINDIR

    # MAKE INSTALL
    # .desktop file sould be added manually to desktop.path otherwise will have root as owner
    INSTALLS += target icon128 translations


    #comment out to use OpenBLAS
#   CONFIG += INTEL_MKL

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
        # ---------------- system OpenBLAS -----------------------------
        DEFINES += OPENBLAS

        # Fedora libs in /usr/lib64:
        #   openblas:  single-threaded library
        #   openblaso: built with USE_OPENMP=1
        #   openblasp: multi-threading without OMP
#        LIBS += -lopenblas
#        LIBS += -lopenblaso
        LIBS += -lopenblasp

    }

    #----------- OPENCASCADE -------------
    #   Ensure that the paths to the binary libraries
    #   are known either by defining them at system level
    #   or by setting them explicitely in this section
    #   The include paths to the development headers must be set explicitely
    INCLUDEPATH += /usr/local/include/opencascade/  #make install location
    INCLUDEPATH += /usr/include/opencascade/        #fedora install location
    LIBS += -L/usr/local/lib/ #make install location
    LIBS += -L/usr/lib64/     #fedora install location



    #--------------------- GMSH ------------------------
    INCLUDEPATH += /usr/local/include/  #make install location
    INCLUDEPATH += /usr/include/        #fedora install location
    LIBS += -L/usr/local/lib64           # redundant
    LIBS += -L/usr/lib64           # redundant
    LIBS += -lgmsh


    #-----XFoil----
    LIBS += -L../XFoil-lib -lXFoil


    #prevent sfinae warnings in the Qt libs
    QMAKE_CXXFLAGS += -Wsfinae-incomplete=0
}



win32-msvc {


    CONFIG += console
    CONFIG -= debug_and_release debug_and_release_target

    RC_ICONS = ../meta/win64/flow5.ico


#-----XFoil----
    LIBS += -L../XFoil-lib -lXFoil1

#----------------------- MKL  ---------------------
    DEFINES += INTEL_MKL   #only option in Windows
    INCLUDEPATH += "C:\Program Files (x86)\Intel\oneAPI\mkl\latest\include"



    LIBS += -L"C:/Program Files (x86)/Intel/oneAPI/mkl/latest/lib"
#    LIBS += -L"C:/Program Files (x86)/Intel/oneAPI/compiler/latest/lib"   # to link with libiomp5md
    LIBS += -L"C:/Program Files (x86)/Intel/oneAPI/tbb/latest/lib"        # to link with tbb12
#https://www.intel.com/content/www/us/en/docs/onemkl/developer-guide-windows/2025-2/selecting-libraries-to-link-with.html
#    LIBS += -lmkl_intel_lp64_dll
#    LIBS += -lmkl_core_dll
#    LIBS += -lmkl_intel_thread_dll -llibiomp5md  # for multithreading
#    LIBS += -lmkl_sequential_dll
#https://www.intel.com/content/www/us/en/docs/onemkl/developer-guide-windows/2023-0/using-the-single-dynamic-library.html
#You can simplify your link line through the use of the Intel® oneAPI Math Kernel Library Single Dynamic Library (SDL).
    LIBS += -lmkl_rt
#    LIBS += -llibiomp5md  # uses the OpenMP threading technology
    LIBS += -ltbb12        # uses the Intel TBB threading technology


#--------------------- GMSH ------------------------
    INCLUDEPATH += D:/bin/gmsh-4.14.1-Windows64-sdk/include/
    LIBS += -L"D:/bin/gmsh-4.14.1-Windows64-sdk/lib"
    LIBS += -lgmsh.dll  # the file name is gmsh.dll.lib

#------------ OPEN CASCADE --------------------------
    INCLUDEPATH += D:/bin/OCCT-7_9_2/build/inc
    LIBS += -LD:/bin/OCCT-7_9_2/build/win64/vc14/lib
#    LIBS += -LD:/bin/OCCT-7_9_2/build/win64/vc14/bin

#---------------- OTHER WIN LIBS -------------------
    DEFINES += _UNICODE WIN64 QT_DLL QT_WIDGETS_LIB
    LIBS += -lopengl32

    #hide the console
    LIBS += -lKernel32 -lUser32
}



macx {
    # Specifies the hard minimum version of macOS that the application supports.
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.3   # needed for c++20 / std::format and compatibility with flow5-libs

    # This variable is used on macOS when building universal binaries.
    QMAKE_MAC_SDK = macosx

    # Specifies a list of architectures to build for.
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

    DEFINES += GL_SILENCE_DEPRECATION   #Shame

    # ----------------- Install-------------------------
    isEmpty(PREFIX):PREFIX = /usr/local
    BINDIR = $$PREFIX/bin
    SHAREDIR = $$PREFIX/share/flow5

    icon128.path = $$SHAREDIR
    icon128.files += ../meta/res/$${TARGET}.png

    translations.path = $$SHAREDIR/translations
    translations.files += ../meta/translations/*.qm

    target.path = $$BINDIR

    INSTALLS += target icon128 translations

    #------------------- Bundle --------------------------
    # Add variables that will be used to build the info.plist file
    QMAKE_TARGET_BUNDLE_PREFIX = cere-aero.tech

    QMAKE_INFO_PLIST = ../meta/mac/info.plist
    ICON = ../meta/mac/flow5.icns

    #----------XFoil--------------------------
    # link to the lib:
    LIBS += -L$$OUT_PWD/../XFoil-lib -lXFoil
    # deploy the libs:
    XFoil.files = $$OUT_PWD/../XFoil-lib/libXFoil.1.dylib
    XFoil.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += XFoil

    #----------flow5-lib--------------------------
    # link to the lib:
    LIBS += -L$$OUT_PWD/../flow5-lib -lflow5-lib
    # deploy the libs:
    flow5-lib.files = $$OUT_PWD/../flow5-lib/libflow5-lib.1.dylib
    flow5-lib.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += flow5-lib

    #-------flow5-io-lib--------------------------
    # link to the lib:
    LIBS += -L$$OUT_PWD/../flow5-io-lib -lflow5-io-lib
    # deploy the libs:
    flow5-io-lib.files = $$OUT_PWD/../flow5-io-lib/libflow5-io-lib.1.dylib
    flow5-io-lib.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += flow5-io-lib

    #-------------OPENCASCADE--------------------------
    # set the paths to the OpenCascade header and lib directories
    INCLUDEPATH += /usr/local/include/opencascade
    LIBS += -L/usr/local/lib

    #_____________GMSH__________________
    INCLUDEPATH += /usr/local/include
    LIBS += -lgmsh

    #-------------vecLib -----------------
    DEFINES += ACCELERATE_NEW_LAPACK
    #    QMAKE_LFLAGS += -framework Accelerate
    LIBS += -llapack -lcblas

}


#CONFIG += warn_on
#QMAKE_CFLAGS_WARN_ON += -W3
#QMAKE_CFLAGS_WARN_ON += -W4
#QMAKE_CXXFLAGS += -WX   # warnings as errors


include(flow5-app.pri)



RESOURCES += \
    resources/qss.qrc \
    resources/icons.qrc \
    resources/images.qrc \
    resources/sailimages.qrc



LIBS += -L../flow5-lib -lflow5-lib
LIBS += -L../flow5-io-lib -lflow5-io-lib

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


DISTFILES += \
    ../meta/doc/images/flow5.png \
    ../meta/doc/releasenotes.html \
    ../meta/doc/style.css \
    ../meta/win64/flow5.ico \
    ../meta/win64/flow5_doc.ico

