
cmake_minimum_required(VERSION 3.16)

project(PlaneRun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(INTEL_MKL "Activate if flow5 has been built with Intel MKL" OFF)
option(FLOW5_IO  "Activate to link to flow5-io.\nRequired for data serialization and use of xml files" ON)

if (WIN32)

    link_directories(D:/dev/build/flow5/release/flow5-lib)
    link_directories(D:/dev/build/flow5/release/flow5-io-lib)
    link_directories(D:/dev/build/flow5/release/XFoil-lib)
    link_directories(C:/Qt/6.9.1/msvc2022_64/lib)
    link_directories("C:/Program Files (x86)/Intel/oneAPI/mkl/latest/lib")
    link_directories("C:/Program Files (x86)/Intel/oneAPI/compiler/latest/lib")

    include_directories(D:/dev/flow5/XFoil-lib)
    include_directories(D:/dev/flow5/flow5-lib)
    include_directories(D:/dev/flow5/flow5-lib/api)
    include_directories(D:/dev/flow5/flow5-io-lib/api)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include/QtCore)
    include_directories(D:/bin/build/OCC_792/inc)
    

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} /permissive- ")

    
else ()

    include_directories(/usr/local/include/XFoil/)
    include_directories(/usr/local/include/flow5/)
    include_directories(/usr/include/opencascade/)         # default distro location
    #include_directories(/usr/local/include/opencascade/)  # make install location if built from source
    include_directories(/usr/include/qt6/)        #required by flow5-io
    include_directories(/usr/include/qt6/QtCore/) #required by flow5-io

    link_directories(/usr/local/lib/)
    link_directories(/usr/lib64/)
    if(INTEL_MKL)
        link_directories(/opt/intel/oneapi/mkl/latest/lib/)
    endif (INTEL_MKL)

endif (WIN32)

add_executable(PlaneRun planerun.cpp)


if(FLOW5_IO)
    target_link_libraries(PlaneRun flow5-io-lib Qt6Core)
endif(FLOW5_IO)


if(WIN32)
    target_link_libraries(PlaneRun XFoil1 flow5-lib)
    if(INTEL_MKL)
        target_link_libraries(PlaneRun  mkl_core_dll mkl_intel_thread_dll mkl_intel_lp64_dll libiomp5md )
    endif()
else ()
    target_link_libraries(PlaneRun XFoil flow5-lib)
    if(INTEL_MKL)
        target_link_libraries(PlaneRun mkl_core mkl_intel_lp64 mkl_gnu_thread)
    endif(INTEL_MKL)
endif (WIN32)





include(GNUInstallDirs)
install(TARGETS PlaneRun
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
