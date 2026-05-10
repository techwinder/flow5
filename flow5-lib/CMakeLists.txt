cmake_minimum_required(VERSION 3.16)

project(my-lib VERSION 1.0 DESCRIPTION "flow5 back-end library containing object and analyses classes" LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(flow5-lib SHARED
    analysis3d/boattask.cpp
    analysis3d/llttask.cpp
    analysis3d/p3analysis.cpp
    analysis3d/p3linanalysis.cpp
    analysis3d/p3unianalysis.cpp
    analysis3d/p4analysis.cpp
    analysis3d/panelanalysis.cpp
    analysis3d/planetask.cpp
    analysis3d/task3d.cpp
    api/aeroforces.h
    api/analysisrange.h
    api/anglecontrol.h
    api/api.cpp api/api.h
    api/apilog.h
    api/arcspline.h
    api/bezierspline.h
    api/bldata.h
    api/blxfoil.h
    api/boat.h
    api/boatopp.h
    api/boatpolar.h
    api/boattask.h
    api/boid.h
    api/bspline.h
    api/bspline3d.h
    api/cartesianframe.h
    api/cartesianframe2d.h
    api/constants.h
    api/ctrlrange.h
    api/cubicinterpolation.h
    api/cubicspline.h
    api/edgesplit.h
    api/eigenvalues.h
    api/enums_objects.h
    api/externalsail.h
    api/extradrag.h
    api/fl5color.h
    api/fl5core.h
    api/fl5lib_global.h
    api/fl5object.h
    api/foil.h
    api/frame.h
    api/fuse.h
    api/fuseflatfaces.h
    api/fusenurbs.h
    api/fuseocc.h
    api/fusesections.h
    api/fusestl.h
    api/fusexfl.h
    api/gaussquadrature.h
    api/geom_enums.h
    api/geom_global.h
    api/geom_params.h
    api/gmshparams.h
    api/gqtriangle.h
    api/hanning.h
    api/hermiteinterpolation.h
    api/inertia.h
    api/linestyle.h
    api/llttask.h
    api/mathelem.h
    api/matrix.h
    api/mctriangle.h
    api/mesh_globals.h
    api/naca4spline.h
    api/node.h
    api/node2d.h
    api/nurbssurface.h
    api/objects2d.h
    api/objects2d_globals.h
    api/objects3d.h
    api/objects_global.h
    api/objects_params.h
    api/occ_globals.h
    api/occmeshparams.h
    api/opp3d.h
    api/oppoint.h
    api/optstructures.h
    api/p3analysis.h
    api/p3linanalysis.h
    api/p3unianalysis.h
    api/p4analysis.h
    api/panel.h
    api/panel2d.h
    api/panel3.h
    api/panel4.h
    api/panelanalysis.h
    api/panelprecision.h
    api/part.h
    api/plane.h
    api/planeopp.h
    api/planepolar.h
    api/planepolarext.h
    api/planepolarnamemaker.h
    api/planestl.h
    api/planetask.h
    api/planexfl.h
    api/pointmass.h
    api/pointspline.h
    api/polar.h
    api/polar3d.h
    api/pslg2d.h
    api/qrleastsquares.h
    api/quad2d.h
    api/quad3d.h
    api/quadmesh.h
    api/quaternion.h
    api/rungekutta.h
    api/s7spline.h
    api/sail.h
    api/sailglobals.h
    api/sailnurbs.h
    api/sailobjects.h
    api/sailocc.h
    api/sailspline.h
    api/sailstl.h
    api/sailwing.h
    api/segment2d.h
    api/segment3d.h
    api/sgsmooth.h
    api/spandistribs.h
    api/spline.h
    api/splinefoil.h
    api/stabderivatives.h
    api/stream2d.h
    api/surface.h
    api/t8opp.h
    api/task3d.h
    api/testpanels.h
    api/triangle2d.h
    api/triangle3d.h
    api/triangulation.h
    api/trimesh.h
    api/units.h
    api/utils.h
    api/vector2d.h
    api/vector3d.h
    api/vortex.h
    api/vorton.h
    api/wingopp.h
    api/wingsailsection.h
    api/wingsection.h
    api/wingxfl.h
    api/xflmesh.h
    api/xflobject.h
    api/xfoiltask.h
    geom/geom2d/node2d.cpp
    geom/geom2d/pslg2d.cpp
    geom/geom2d/quad2d.cpp
    geom/geom2d/segment2d.cpp
    geom/geom2d/splines/bezierspline.cpp
    geom/geom2d/splines/bspline.cpp
    geom/geom2d/splines/cubicspline.cpp
    geom/geom2d/splines/pointspline.cpp
    geom/geom2d/splines/spline.cpp
    geom/geom2d/triangle2d.cpp
    geom/geom2d/vector2d.cpp
    geom/geom3d/bspline3d.cpp
    geom/geom3d/frame.cpp
    geom/geom3d/node.cpp
    geom/geom3d/nurbssurface.cpp
    geom/geom3d/quad3d.cpp
    geom/geom3d/quaternion.cpp
    geom/geom3d/segment3d.cpp
    geom/geom3d/triangle3d.cpp
    geom/geom3d/triangulation.cpp
    geom/geom3d/vector3d.cpp
    geom/geom_globals/geom_global.cpp
    math/cubicinterpolation.cpp
    math/gaussquadrature.cpp
    math/hanning.cpp
    math/hermiteinterpolation.cpp
    math/mathelem.cpp
    math/matrix.cpp
    math/qrleastsquares.cpp
    math/rungekutta.cpp
    math/sgsmooth.cpp
    objects2d/analysis2d/stream2d.cpp
    objects2d/analysis2d/xfoiltask.cpp
    objects2d/foilobjects/bldata.cpp
    objects2d/foilobjects/blxfoil.cpp
    objects2d/foilobjects/foil.cpp
    objects2d/foilobjects/oppoint.cpp
    objects2d/foilobjects/panel2d.cpp
    objects2d/foilobjects/polar.cpp
    objects2d/foilobjects/splinefoil.cpp
    objects2d/globals/objects2d_globals.cpp
    objects2d/objects2d.cpp
    objects3d/analysis3d/aeroforces.cpp
    objects3d/analysis3d/boatopp.cpp
    objects3d/analysis3d/boatpolar.cpp
    objects3d/analysis3d/opp3d.cpp
    objects3d/analysis3d/planeopp.cpp
    objects3d/analysis3d/planepolar.cpp
    objects3d/analysis3d/planepolarext.cpp
    objects3d/analysis3d/planepolarnamemaker.cpp
    objects3d/analysis3d/polar3d.cpp
    objects3d/analysis3d/spandistribs.cpp
    objects3d/analysis3d/stabderivatives.cpp
    objects3d/analysis3d/wingopp.cpp
    objects3d/globals/objects_global.cpp
    objects3d/objects3d.cpp
    objects3d/planeobjects/fuse/fuse.cpp
    objects3d/planeobjects/fuse/fuseflatfaces.cpp
    objects3d/planeobjects/fuse/fusenurbs.cpp
    objects3d/planeobjects/fuse/fuseocc.cpp
    objects3d/planeobjects/fuse/fusesections.cpp
    objects3d/planeobjects/fuse/fusestl.cpp
    objects3d/planeobjects/fuse/fusexfl.cpp
    objects3d/planeobjects/inertia/inertia.cpp
    objects3d/planeobjects/part.cpp
    objects3d/planeobjects/plane/plane.cpp
    objects3d/planeobjects/plane/planestl.cpp
    objects3d/planeobjects/plane/planexfl.cpp
    objects3d/planeobjects/wing/surface.cpp
    objects3d/planeobjects/wing/wingsection.cpp
    objects3d/planeobjects/wing/wingxfl.cpp
    objects3d/sailobjects/boat.cpp
    objects3d/sailobjects/sailglobals.cpp
    objects3d/sailobjects/sailobjects.cpp
    objects3d/sailobjects/sails/externalsail.cpp
    objects3d/sailobjects/sails/sail.cpp
    objects3d/sailobjects/sails/sailnurbs.cpp
    objects3d/sailobjects/sails/sailocc.cpp
    objects3d/sailobjects/sails/sailspline.cpp
    objects3d/sailobjects/sails/sailstl.cpp
    objects3d/sailobjects/sails/sailwing.cpp
    objects3d/sailobjects/sails/wingsailsection.cpp
    objects3d/sailobjects/splines7/arcspline.cpp
    objects3d/sailobjects/splines7/naca4spline.cpp
    objects3d/sailobjects/splines7/s7spline.cpp
    occ/occ_globals.cpp
    occ/occmeshparams.cpp
    panels/mesh/mesh_globals.cpp
    panels/mesh/quadmesh.cpp
    panels/mesh/trimesh.cpp
    panels/mesh/xflmesh.cpp
    panels/panels/gqtriangle.cpp
    panels/panels/mctriangle.cpp
    panels/panels/panel.cpp
    panels/panels/panel3.cpp
    panels/panels/panel4.cpp
    panels/panels/testpanels.cpp
    panels/panels/vortex.cpp
    panels/panels/vorton.cpp
    panels/shell/edgesplit.cpp
    utils/apilog.cpp
    utils/fl5color.cpp
    utils/fl5core.cpp
    utils/units.cpp
    utils/utils.cpp
)

set_target_properties(flow5-lib PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(flow5-lib PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})


#FILE(GLOB MY_HEADER_FILES  api/*.h)
#set_target_properties(flow5-lib PROPERTIES PUBLIC_HEADER  ${MY_HEADER_FILES})

set_target_properties(flow5-lib PROPERTIES PUBLIC_HEADER
"api/aeroforces.h;\
api/analysisrange.h;\
api/anglecontrol.h;\
api/api.h;\
api/apilog.h;\
api/arcspline.h;\
api/bezierspline.h;\
api/bldata.h;\
api/blxfoil.h;\
api/boat.h;\
api/boatopp.h;\
api/boatpolar.h;\
api/boattask.h;\
api/boid.h;\
api/bspline.h;\
api/bspline3d.h;\
api/cartesianframe.h;\
api/cartesianframe2d.h;\
api/constants.h;\
api/ctrlrange.h;\
api/cubicinterpolation.h;\
api/cubicspline.h;\
api/edgesplit.h;\
api/eigenvalues.h;\
api/enums_objects.h;\
api/externalsail.h;\
api/extradrag.h;\
api/fl5color.h;\
api/fl5core.h;\
api/fl5lib_global.h;\
api/fl5object.h;\
api/foil.h;\
api/frame.h;\
api/fuse.h;\
api/fuseflatfaces.h;\
api/fusenurbs.h;\
api/fuseocc.h;\
api/fusesections.h;\
api/fusestl.h;\
api/fusexfl.h;\
api/gaussquadrature.h;\
api/geom_enums.h;\
api/geom_global.h;\
api/geom_params.h;\
api/gmshparams.h;\
api/gqtriangle.h;\
api/hanning.h;\
api/hermiteinterpolation.h;\
api/inertia.h;\
api/linestyle.h;\
api/llttask.h;\
api/mathelem.h;\
api/matrix.h;\
api/mctriangle.h;\
api/mesh_globals.h;\
api/naca4spline.h;\
api/node.h;\
api/node2d.h;\
api/nurbssurface.h;\
api/objects2d.h;\
api/objects2d_globals.h;\
api/objects3d.h;\
api/objects_global.h;\
api/objects_params.h;\
api/occ_globals.h;\
api/occmeshparams.h;\
api/opp3d.h;\
api/oppoint.h;\
api/optstructures.h;\
api/p3analysis.h;\
api/p3linanalysis.h;\
api/p3unianalysis.h;\
api/p4analysis.h;\
api/panel.h;\
api/panel2d.h;\
api/panel3.h;\
api/panel4.h;\
api/panelanalysis.h;\
api/panelprecision.h;\
api/part.h;\
api/plane.h;\
api/planeopp.h;\
api/planepolar.h;\
api/planepolarext.h;\
api/planepolarnamemaker.h;\
api/planestl.h;\
api/planetask.h;\
api/planexfl.h;\
api/pointmass.h;\
api/pointspline.h;\
api/polar.h;\
api/polar3d.h;\
api/pslg2d.h;\
api/qrleastsquares.h;\
api/quad2d.h;\
api/quad3d.h;\
api/quadmesh.h;\
api/quaternion.h;\
api/rungekutta.h;\
api/s7spline.h;\
api/sail.h;\
api/sailglobals.h;\
api/sailnurbs.h;\
api/sailobjects.h;\
api/sailocc.h;\
api/sailspline.h;\
api/sailstl.h;\
api/sailwing.h;\
api/segment2d.h;\
api/segment3d.h;\
api/sgsmooth.h;\
api/spandistribs.h;\
api/spline.h;\
api/splinefoil.h;\
api/stabderivatives.h;\
api/stream2d.h;\
api/surface.h;\
api/t8opp.h;\
api/task3d.h;\
api/testpanels.h;\
api/triangle2d.h;\
api/triangle3d.h;\
api/triangulation.h;\
api/trimesh.h;\
api/units.h;\
api/utils.h;\
api/vector2d.h;\
api/vector3d.h;\
api/vortex.h;\
api/vorton.h;\
api/wingopp.h;\
api/wingsailsection.h;\
api/wingsection.h;\
api/wingxfl.h;\
api/xflmesh.h;\
api/xflobject.h;\
api/xfoiltask.h"
)

if(WIN32)
    target_include_directories(flow5-lib PUBLIC D:/dev/flow5/XFoil-lib)
elseif(LINUX)
    target_include_directories(flow5-lib PUBLIC /usr/local/include/XFoil/)
endif()

target_include_directories(flow5-lib PRIVATE ..)

target_include_directories(flow5-lib PUBLIC  api)

target_compile_definitions(flow5-lib PUBLIC FL5LIB_LIBRARY)



if(LINUX)
    option(USE_INTEL_MKL "Activate to build with Intel MKL. Uses OpenBLAS otherwise." ON)

    target_include_directories(flow5-lib PUBLIC
        /usr/include/opencascade
        /usr/local/include/opencascade
    )

    target_compile_definitions(flow5-lib PUBLIC LINUX_OS)

    target_link_libraries(flow5-lib PUBLIC XFoil)

    if(USE_INTEL_MKL)
        target_include_directories(flow5-lib PUBLIC /opt/intel/oneapi/mkl/latest/include )

        target_compile_definitions(flow5-lib PUBLIC INTEL_MKL)

        target_link_libraries(flow5-lib PUBLIC mkl_core mkl_gnu_thread mkl_intel_lp64)
    else()
        target_compile_definitions(flow5-lib PUBLIC OPENBLAS)

        target_link_libraries(flow5-lib PUBLIC openblasp)
    endif()
endif()

if(WIN32)
    target_compile_definitions(flow5-lib PUBLIC WIN_OS )
    target_compile_definitions(flow5-lib PUBLIC INTEL_MKL )
    


    link_directories(C:/Qt/6.9.1/msvc2022_64/lib)
    link_directories(D:/bin/build/OCC_792/win64/vc14/lib)

    link_directories(C:/Program Files (x86)/Intel/oneAPI/mkl/latest/lib)
    link_directories(C:/Program Files (x86)/Intel/oneAPI/compiler/latest/lib)


    include_directories(C:/Qt/6.9.1/msvc2022_64/include)
    include_directories(C:/Qt/6.9.1/msvc2022_64/include/QtCore)
    include_directories(D:/bin/build/OCC_792/inc)
    include_directories("C:/Program Files (x86)/inteloneapi/mkl/latest/include")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} /permissive- ")
    
   
   link_libraries(PUBLIC mkl_core_dll mkl_intel_thread_dll mkl_intel_lp64_dll libiomp5md )
 endif()

link_libraries(PUBLIC
    TKBO
    TKBRep
    TKBool
    TKCDF
    TKDESTEP
    TKFillet
    TKG2d
    TKG3d
    TKGeomAlgo
    TKGeomBase
    TKLCAF
    TKMath
    TKMesh
    TKOffset
    TKPrim
    TKShHealing
    TKTopAlgo
    TKXSBase
    TKernel
)


#install(FILES ${MY_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flow5-lib)
if(LINUX)
    # initializes ${CMAKE_INSTALL_*DIR} to sane defaults
    include(GNUInstallDirs)

    # headers are now installed to ${CMAKE_INSTALL_PREFIX}/include/mylib, and
    # library into ${CMAKE_INSTALL_PREFIX}/lib or ${CMAKE_INSTALL_PREFIX}/lib64 depending on arch
    install(
        TARGETS flow5-lib
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flow5
    )

    # make uninstall
    #avoid - does not remove the install dir
    #add_custom_target(uninstall COMMAND xargs rm -vf <install_manifest.txt )

endif()


