
#    Compilation instructions:
#    https://flow5.tech/docs/flow5_doc/Source/Compilation.html

QT -= gui  # not using QColor

TARGET = fl5-lib

TEMPLATE = lib
DEFINES += FL5LIB_LIBRARY


DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += c++20
} else {
    CONFIG += c++17
}


# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += ../XFoil-lib/


INCLUDEPATH += ./api


OBJECTS_DIR = ./objects
MOC_DIR     = ./moc
RCC_DIR     = ./rcc


linux-g++ {

    CONFIG += occt

    DEFINES += LINUX_OS

    isEmpty(PREFIX){
        PREFIX = /usr/local
    }


    INCLUDEDIR = $$PREFIX/include/$$TARGET
    inc.path = $$INCLUDEDIR
    inc.files += api/*.h

    target.path = $$PREFIX/lib

    # MAKE INSTALL
    INSTALLS += target inc


#-----XFoil-----
    LIBS += -L../XFoil-lib -lXFoil

    #comment out to use OpenBLAS
#    CONFIG += INTEL_MKL

    INTEL_MKL {
        #------------ MKL --------------------
        #    MKL can use the c++ matrices in row major order order
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
        LIBS += -lopenblas
        LIBS += -llapack -llapacke
    }


    #----------- OPENCASCADE -------------
    #   Ensure that the paths to the binary libraries
    #   are known either by defining them at system level
    #   or by setting them explicitely in this section
    #   The include paths to the development headers must be set explicitely
        INCLUDEPATH += /usr/local/include/opencascade/
        LIBS += -L/usr/local/lib/

}


win32-g++ {

    DEFINES += WIN_OS

    CONFIG += console
    CONFIG -= debug_and_release debug_and_release_target

    #-----XFoil-----
    # On Windows, qmake typically emits import libs with the major version suffix.
    LIBS += -L../XFoil-lib -lXFoil1

    #----------------------- OpenBLAS / LAPACK (MinGW) ---------------------
    DEFINES += OPENBLAS
    INCLUDEPATH += D:/flow5/OpenBLAS_local/include
    LIBS += -LD:/flow5/OpenBLAS_local/lib
    LIBS += -lopenblas -llapack -llapacke

    #----------------------- OpenCascade / OCCT (MinGW) --------------------
    INCLUDEPATH += D:/flow5/OCCT/build-mingw-dll/inc
    LIBS += -LD:/flow5/OCCT/build-mingw-dll/win64/gcc/lib
    CONFIG += occt
}


win32-msvc {

    CONFIG += occt

    DEFINES += WIN_OS


    CONFIG += console
    CONFIG -= debug_and_release debug_and_release_target



#-----XFoil-----

    LIBS += -L../XFoil-lib -lXFoil1

#----------------------- MKL  ---------------------
    # DEFINES += INTEL_MKL
    # INCLUDEPATH += "C:\Program Files (x86)\Intel\oneAPI\mkl\latest\include"
    INCLUDEPATH += D:/flow5/OpenBLAS

    # LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\mkl\latest\bin"
    # LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\mkl\latest\lib"
    # LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\compiler\latest\bin"
    # LIBS += -L"C:\Program Files (x86)\Intel\oneAPI\compiler\latest\lib"
    # LIBS += -lmkl_intel_lp64_dll
    # LIBS += -lmkl_core_dll
    # LIBS += -lmkl_intel_thread_dll -llibiomp5md  # for multithreading
    #    LIBS += -lmkl_sequential_dll
    
    LIBS += -LD:/flow5/OpenBLAS -lopenblas


#------------ OPEN CASCADE --------------------------
    INCLUDEPATH += D:/flow5/OCCT/build-mingw-dll/inc
    LIBS += -LD:/flow5/OCCT/build-mingw-dll/win64/gcc/lib
    LIBS += -LD:/flow5/OCCT/build-mingw-dll/win64/gcc/bin
    
    CONFIG += occt

}


macx {

    CONFIG += occt
    DEFINES += MAC_OS
    DEFINES += GL_SILENCE_DEPRECATION   #Shame


    QMAKE_MAC_SDK = macosx
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks


    #-------XFoil
    # link to the lib:
    LIBS += -L$$OUT_PWD/../XFoil-lib -lXFoil


    #-------------OPENCASCADE -----------------
    INCLUDEPATH += /usr/local/include/opencascade
    LIBS += -L/usr/local/lib


    #-------------vecLib -----------------
    DEFINES += ACCELERATE
    #    QMAKE_LFLAGS += -framework Accelerate
    LIBS += -llapack -lcblas

}




include (fl5-lib.pri)




#----- OCC -----
occt {
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
}
