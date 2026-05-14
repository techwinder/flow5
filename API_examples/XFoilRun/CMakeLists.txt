
cmake_minimum_required(VERSION 3.16)

project(XFoilRun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


unset(USING_INTEL_MKL CACHE)
option(USING_INTEL_MKL "Activate if flow5 has been built with Intel MKL" OFF)
option(FLOW5_IO  "Activate to link to flow5-io.\nRequired for data serialization and use of xml files" ON)


if (WIN32)
    link_directories(D:/dev/build/flow5/release/flow5-lib)
    link_directories(D:/dev/build/flow5/release/flow5-io-lib)
    link_directories(D:/dev/build/flow5/release/XFoil-lib)
    link_directories(D:/bin/gmsh-4.14.1-Windows64-sdk/lib)
    link_directories(C:/Qt/6.9.1/msvc2022_64/lib)
    link_directories("C:/Program Files (x86)/Intel/oneAPI/mkl/latest/lib")
    link_directories("C:/Program Files (x86)/Intel/oneAPI/compiler/latest/lib") # to link to libiomp5md
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
    include_directories(/usr/local/include/XFoil/)
    include_directories(/usr/local/include/flow5/)
    include_directories(/usr/include/)                     # gmsh install location in fedora
    include_directories(/usr/include/opencascade/)         # default distro location
    #include_directories(/usr/local/include/opencascade/)  # make install location if built from source
    include_directories(/usr/include/qt6/)        #required by flow5-io
    include_directories(/usr/include/qt6/QtCore/) #required by flow5-io

    link_directories(/usr/local/lib/)
    link_directories(/usr/lib64/)
    if(USING_INTEL_MKL)
        link_directories(/opt/intel/oneapi/mkl/latest/lib/)
    endif (USING_INTEL_MKL)

elseif(APPLE)

    #todo

endif ()

add_executable(XFoilRun xfoilrun.cpp)


target_link_libraries(XFoilRun flow5-lib)
if(FLOW5_IO)
    target_link_libraries(XFoilRun flow5-io-lib Qt6Core)
endif(FLOW5_IO)


if(WIN32)
    target_link_libraries(XFoilRun XFoil1 flow5-io-lib)
    target_link_libraries(XFoilRun gmsh.dll.lib) # the file name is gmsh.dll.lib
    if(USING_INTEL_MKL)
        target_link_libraries(XFoilRun mkl_rt)         # Intel® oneAPI Math Kernel Library Single Dynamic Library (SDL)
#         target_link_libraries(XFoilRun  libiomp5md ) # uses the OpenMP threading technology
        target_link_libraries(XFoilRun  tbb12 )        # uses the Intel TBB threading technology

        target_compile_definitions(XFoilRun PUBLIC INTEL_MKL)
    endif()
elseif (LINUX)
    target_link_libraries(XFoilRun XFoil)
    target_link_libraries(XFoilRun gmsh)
    if(USING_INTEL_MKL)
        target_link_libraries(XFoilRun mkl_core mkl_intel_lp64 mkl_gnu_thread)
        target_compile_definitions(XFoilRun PUBLIC INTEL_MKL)
    else()
            # Fedora libs in /usr/lib64:
        #   openblas:  single-threaded library
        #   openblaso: built with USE_OPENMP=1
        #   openblasp: multi-threading without OMP
        target_link_libraries(XFoilRun openblasp)
        target_compile_definitions(XFoilRun PUBLIC OPENBLAS)
    endif()
elseif (APPLE)
    target_link_libraries(XFoilRun XFoil lapack cblas gmsh)
    target_compile_definitions(XFoilRun PUBLIC ACCELERATE_NEW_LAPACK)
endif ()


include(GNUInstallDirs)
install(TARGETS XFoilRun
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
