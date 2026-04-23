
cmake_minimum_required(VERSION 3.16)

project(PlaneRun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
include_directories(/usr/local/include/flow5-lib/api/)
include_directories(/usr/local/include/opencascade)
include_directories(/usr/include/qt6)
include_directories(/usr/include/qt6/QtCore)

link_directories(/usr/local/lib/)
link_directories(/usr/lib64/)
endif (WIN32)


add_executable(PlaneRun planerun.cpp)

# if using MKL, link their libraries
target_link_libraries(PlaneRun XFoil flow5-lib Qt6Core)


include(GNUInstallDirs)
install(TARGETS PlaneRun
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
