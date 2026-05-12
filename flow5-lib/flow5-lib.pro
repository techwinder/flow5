
#    Compilation instructions:
#    https://flow5.tech/docs/flow5_doc/Source/Compilation.html

CONFIG -= qt

TARGET = flow5-lib

TEMPLATE = lib
DEFINES += FL5LIB_LIBRARY




CONFIG += c++20


# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += ../XFoil-lib/


INCLUDEPATH += ./api


OBJECTS_DIR = ./objects
MOC_DIR     = ./moc
RCC_DIR     = ./rcc


linux-g++ {


    DEFINES += LINUX_OS

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


}


win32-msvc {

    DEFINES += WIN_OS

    CONFIG -= debug_and_release debug_and_release_target


#-----XFoil-----

    LIBS += -L../XFoil-lib -lXFoil1

#----------------------- MKL  ---------------------
    DEFINES += INTEL_MKL
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


#------------ OPEN CASCADE --------------------------
    INCLUDEPATH += D:\bin\OCCT-7_9_2\build\inc
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\lib
    LIBS += -LD:\bin\OCCT-7_9_2\build\win64\vc14\bin


#--------------------- GMSH ------------------------
    INCLUDEPATH += D:\bin\gmsh-4.14.1-Windows64-sdk/include/
    LIBS += -L"D:\bin\gmsh-4.14.1-Windows64-sdk/lib"
    LIBS += -lgmsh.dll  # the file name is gmsh.dll.lib



    QMAKE_CXXFLAGS += -utf-8
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


    #-------XFoil
    # link to the lib:
    LIBS += -L$$OUT_PWD/../XFoil-lib -lXFoil


    #-------------OPENCASCADE -----------------
    INCLUDEPATH += /usr/local/include/opencascade
    LIBS += -L/usr/local/lib


    #_____________GMSH__________________
    INCLUDEPATH += /usr/local/include
    LIBS += -lgmsh


    #-------------vecLib -----------------
    DEFINES += ACCELERATE_NEW_LAPACK
    LIBS += -llapack -lcblas   #todo cblas may not be necessary

}




include (flow5-lib.pri)




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

