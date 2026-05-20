cmake_minimum_required(VERSION 3.16)

project(PlaneRun6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(USING_INTEL_MKL "Activate if flow5 has been built with Intel MKL" OFF)
option(USING_FLOW5_IO  "Activate to link to flow5-io.\nRequired for data serialization and use of xml files\nRequires linking to Qt6Core" ON)

SET(API_EXAMPLE "PlaneRun1")

if (WIN32)

#    message(STATUS  $ENV{OMP_NESTED})
#    set(ENV{OMP_NESTED} TRUE)

    link_directories(D:/dev/build/flow5/release/flow5-lib)
    link_directories(D:/dev/build/flow5/release/flow5-io-lib)
    link_directories(D:/dev/build/flow5/release/XFoil-lib)
    link_directories(D:/bin/gmsh-4.14.1-Windows64-sdk/lib)
    link_directories(D:/bin/build/OCC_792/win64/vc14/lib)
    link_directories(C:/Qt/6.9.1/msvc2022_64/lib)
    link_directories("C:/Program Files (x86)/Intel/oneAPI/mkl/latest/lib")
#    link_directories("C:/Program Files (x86)/Intel/oneAPI/compiler/latest/lib") # to link to libiomp5md
    link_directories("C:/Program Files (x86)/Intel/oneAPI/tbb/latest/lib" )     # to link to tbb12

    include_directories(D:/dev/flow5/XFoil-lib)
    include_directories(D:/dev/flow5/flow5-lib)
    include_directories(D:/dev/flow5/flow5-lib/api)
    include_directories(D:/dev/flow5/flow5-io-lib/api)
    include_directories(D:/bin/gmsh-4.14.1-Windows64-sdk/include/)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include/QtCore)
    include_directories(D:/bin/build/OCC_792/inc)
    include_directories("C:/Program Files (x86)/Intel/oneAPI/mkl/latest/include")


    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} /permissive- ")

    set(USING_INTEL_MKL true CACHE BOOL "Activate if flow5 has been built with Intel MKL")

elseif (LINUX)
    set(XFOIL_INCLUDE_DIR "/usr/local/include/XFoil"  CACHE PATH "The path to the XFoil include files")
    set(FLOW5_INCLUDE_DIR /usr/local/include/flow5    CACHE PATH "The path to the flow5 include files")
    set(GMSH_INCLUDE_DIR  "/usr/include"              CACHE PATH "The path to the gmsh.h include file")
    set(OCCT_INCLUDE_DIR  "/usr/include/opencascade"  CACHE PATH "The path to the OpenCascade include files")

    include_directories(${XFOIL_INCLUDE_DIR})
    include_directories(${FLOW5_INCLUDE_DIR})
    include_directories(${GMSH_INCLUDE_DIR})
    include_directories(${OCCT_INCLUDE_DIR})
    include_directories(/usr/include/qt6/)        #required by flow5-io
    include_directories(/usr/include/qt6/QtCore/) #required by flow5-io


    set(FLOW5_LIB_DIR "/usr/local/lib/" CACHE PATH "The path to the flow5-lib and flow5-io-lib libraries")
    link_directories(${FLOW5_LIB_DIR})

    # /usr/lib64 is set by derfault in the environment's library paths
#    set(OCCT_LIB_DIR "/usr/lib64/" CACHE PATH "The path to the OCCT TKxxx libraries")
#    link_directories(OCCT_LIB_DIR)

#    set(GMSH_LIB_DIR "/usr/lib64/" CACHE PATH "The path to the gmsh library")
#    link_directories(GMSH_LIB_DIR)

    if(USING_INTEL_MKL)
        link_directories(/opt/intel/oneapi/mkl/latest/lib/)
    endif (USING_INTEL_MKL)

elseif(APPLE)

    #todo

endif ()

add_executable(${API_EXAMPLE} ${API_EXAMPLE}.cpp)

target_link_libraries(${API_EXAMPLE} flow5-lib)

if(USING_FLOW5_IO)
    target_link_libraries(${API_EXAMPLE} flow5-io-lib Qt6Core)
endif(USING_FLOW5_IO)


if(WIN32)
    target_link_libraries(${API_EXAMPLE} XFoil1)
    target_link_libraries(${API_EXAMPLE} gmsh.dll.lib) # the file name is gmsh.dll.lib
    target_link_libraries(${API_EXAMPLE} TKernel)
    if(USING_INTEL_MKL)
        target_link_libraries(${API_EXAMPLE} mkl_rt)         # Intel® oneAPI Math Kernel Library Single Dynamic Library (SDL)
#        target_link_libraries(${API_EXAMPLE} libiomp5md )    # uses the OpenMP threading technology
        target_link_libraries(${API_EXAMPLE} tbb12 )         # uses the Intel TBB threading technology

        target_compile_definitions(${API_EXAMPLE} PUBLIC INTEL_MKL)
    endif()
elseif (LINUX)
    target_link_libraries(${API_EXAMPLE} XFoil)
    target_link_libraries(${API_EXAMPLE} gmsh)
    if(USING_INTEL_MKL)
        target_link_libraries(${API_EXAMPLE} mkl_core mkl_intel_lp64 mkl_gnu_thread)
        target_compile_definitions(${API_EXAMPLE} PUBLIC INTEL_MKL)
    else()
        # Fedora libs in /usr/lib64:
        #   openblas:  single-threaded library
        #   openblaso: built with USE_OPENMP=1
        #   openblasp: multi-threading without OMP
        target_link_libraries(${API_EXAMPLE} openblasp)
        target_compile_definitions(${API_EXAMPLE} PUBLIC OPENBLAS)
    endif()
elseif (APPLE)
    target_link_libraries(${API_EXAMPLE} XFoil lapack cblas gmsh)
    target_compile_definitions(${API_EXAMPLE} PUBLIC ACCELERATE_NEW_LAPACK)
endif ()




include(GNUInstallDirs)
install(TARGETS ${API_EXAMPLE}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
