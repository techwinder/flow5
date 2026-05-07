
cmake_minimum_required(VERSION 3.16)

project(XFoilRun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(INTEL_MKL "Activate if flow5 has been built with Intel MKL" ON)

if (WIN32)
include_directories(D:/dev/flow5/XFoil-lib)
include_directories(D:/dev/flow5/flow5-lib)
include_directories(D:/dev/flow5/flow5-lib/api)
include_directories(C:/Qt/6.9.1/msvc2022_64/include)
include_directories(C:/Qt/6.9.1/msvc2022_64/include/QtCore)

link_directories(D:/dev/build/flow5/release/flow5-lib)
link_directories(D:/dev/build/flow5/release/XFoil-lib)
link_directories(C:/Qt/6.9.1/msvc2022_64/lib)
set(CMAKE_CXX_FLAGS /Zc:__cplusplus)
else ()
include_directories(/usr/local/include/XFoil/)
include_directories(/usr/local/include/flow5-lib/)
include_directories(/usr/include/qt6)
include_directories(/usr/include/qt6/QtCore)

link_directories(/usr/local/lib/)
link_directories(/usr/lib64/)

if(INTEL_MKL)
link_directories(/opt/intel/oneapi/mkl/latest/lib/)
endif()

endif (WIN32)


add_executable(XFoilRun xfoilrun.cpp)


if(INTEL_MKL)
target_link_libraries(XFoilRun XFoil flow5-lib Qt6Core mkl_core mkl_intel_lp64 mkl_gnu_thread)
else()
target_link_libraries(XFoilRun XFoil flow5-lib Qt6Core)
endif()


include(GNUInstallDirs)
install(TARGETS XFoilRun
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
