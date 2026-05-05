
cmake_minimum_required(VERSION 3.16)

project(PlaneRun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(INTEL_MKL "Activate to use Intel MKL; using OpenBLAS or ACCELERATE otherwise" ON)

if (WIN32)

    link_directories(D:/dev/build/flow5/release/flow5-lib)
    link_directories(D:/dev/build/flow5/release/XFoil-lib)
    link_directories(C:/Qt/6.9.1/msvc2022_64/lib)
    link_directories("C:/Program Files (x86)/Intel/oneAPI/mkl/latest/lib")
    link_directories("C:/Program Files (x86)/Intel/oneAPI/compiler/latest/lib")

    include_directories(D:/dev/flow5/XFoil-lib)
    include_directories(D:/dev/flow5/flow5-lib)
    include_directories(D:/dev/flow5/flow5-lib/api)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include/QtCore)
    include_directories(D:/bin/build/OCC_792/inc)
    
    set(CMAKE_CXX_FLAGS /Zc:__cplusplus)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} /permissive- ")

    
else ()

    include_directories(/usr/local/include/XFoil/)
    include_directories(/usr/local/include/flow5-lib/)
    include_directories(/usr/local/include/flow5-lib/api/)
    include_directories(/usr/include/opencascade/)         #default distro location
    #include_directories(/usr/local/include/opencascade/)  #make install location
    include_directories(/usr/include/qt6/)
    include_directories(/usr/include/qt6/QtCore/)

    link_directories(/usr/local/lib/)
    link_directories(/usr/lib64/)
    if(INTEL_MKL)
        link_directories(/opt/intel/oneapi/mkl/latest/lib/)
    endif (INTEL_MKL)

endif (WIN32)

add_executable(PlaneRun planerun.cpp)

if(WIN32)
    target_link_libraries(PlaneRun Qt6Core XFoil1 flow5-lib mkl_core_dll mkl_intel_thread_dll mkl_intel_lp64_dll libiomp5md )
else ()
    if(INTEL_MKL)
        target_link_libraries(PlaneRun XFoil flow5-lib Qt6Core mkl_core mkl_intel_lp64 mkl_gnu_thread)
    else()
        target_link_libraries(PlaneRun XFoil flow5-lib Qt6Core)
    endif(INTEL_MKL)
endif (WIN32)




include(GNUInstallDirs)
install(TARGETS PlaneRun
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
