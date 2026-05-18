
#include <iostream>
#include <filesystem>
#include <format>

#include <api.h>
#include <constants.h>
#include <flow5-io.h>
#include <foil.h>
#include <fusenurbs.h>
#include <gmesh_globals.h>
#include <objects2d.h>
#include <objects3d.h>
#include <oppoint.h>
#include <panelanalysis.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planepolarnamemaker.h>
#include <planestl.h>
#include <planetask.h>
#include <planexfl.h>
#include <polar.h>
#include <xfoiltask.h>


#ifdef WIN32
#include <Windows.h>
#endif

#if defined ACCELERATE_NEW_LAPACK
    #include <Accelerate/Accelerate.h>
    #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/cblas.h>
#endif


/* This example shows how to
 * - import a plane and a polar from xml files
 * - build a fuselage conforming mesh
 * - run a calculation
 *
 * Copy the xml subdirectory into the build folder before running
 */
int main()
{
    #ifdef WIN32
    // enable UTF8 characters
    SetConsoleOutputCP(65001);
    #endif

    std::cout << std::endl << std::endl;

    std::cout << "flow5 plane run"  << std::endl;

    // Configure LAPACK
    std::string strange;

    #ifdef OPENBLAS

    strange.clear();
    switch(openblas_get_parallel())
    {
        //        https://github.com/OpenMathLib/OpenBLAS/wiki/Faq/a15b786986841d2e4e4e84e3f2ecff9c3b263b32
        //openblas_get_parallel() will return 0 for a single-threaded library, 1 if multithreading without OpenMP, 2 if built with USE_OPENMP=1
        case 0:  strange = "OpenBlas: single-threaded library";     break;
        case 1:  strange = "OpenBlas: multi-threading without OMP"; break;
        case 2:  strange = "OpenBlas: built with USE_OPENMP=1";     break;
        default: strange = "openblas_get_parallel() return error";
    }
    std::cout << strange << std::endl << std::endl;

    #elif defined INTEL_MKL

    strange.clear();
    int nt = mkl_get_max_threads();

    mkl_set_dynamic(0);

    if (1 == mkl_get_dynamic())
    {
        strange += "MKL dynamic threading is enabled\n";
        strange += std::format("Intel MKL may use less than {:d} threads for a large problem", nt);
    }
    else
    {
        strange += "MKL dynamic threading is disabled\n";
        strange += std::format("Intel MKL should use {:d} threads for a large problem", nt);
    }

    std::cout << strange << std::endl << std::endl;

    #endif


    // initialize Gmsh AFTER MKL - crash otherwise due to threading conflicts?
    gmsh::initialize();
    gmsh::option::setNumber("General.Terminal", 0);
    gmsh::option::setNumber("Geometry.OCCParallel", 1.0);
    gmsh::option::setNumber("General.NumThreads", 0);

    /*    std::string list;
     *    gmesh::listMainOptions(list);
     *    std::cout << list.c_str() << std::endl; */


    // Preload a project file containing the airfoils and their polar meshes
    /*    std::string logload;
     *    std::string loadfilepath = "/path/to/Preload.fl5";
     *    if(!io::loadProject(loadfilepath, logload))
     *    {
     *        std::cerr << logload << std::endl;
    }*/

    // make the foils
    std::cout << "Making the NACA foils" << std::endl;
    {
        Foil *pFoilN2413 = new Foil;
        if(!Objects2d::makeNacaFoil(pFoilN2413, 2413, 200))
        {
            // this should not happen
            std::cerr << "Error making foil NACA 2413" << std::endl;
            delete pFoilN2413;
            return 0;
        }
        pFoilN2413->setName("NACA 2413");
        Objects2d::insertThisFoil(pFoilN2413);

        Foil *pFoilN0009 = new Foil;
        if(!Objects2d::makeNacaFoil(pFoilN0009, 9, 200))
        {
            // this should not happen
            std::cerr << "Error making foil NACA 0009" << std::endl;
            delete pFoilN0009;
            return 0;
        }
        pFoilN0009->setName("NACA 0009");
        Objects2d::insertThisFoil(pFoilN0009);


        // set the style for these foils and their children objects, i.e. polars and operating points
        pFoilN0009->setTheStyle({true, Line::SOLID, 2, {31, 111, 231}, Line::NOSYMBOL});
        pFoilN2413->setTheStyle({true, Line::SOLID, 2, {231, 111, 31}, Line::NOSYMBOL});

        // repanel
        int  npanels = 149;
        double amp = 0.7; // 0.0: no bunching, 1.0: max. bunching
        pFoilN0009->rePanel(npanels, amp);
        pFoilN2413->rePanel(npanels, amp);
        std::cout << "Done airfoils"<<std::endl << std::endl;
    }


    // Create a plane from an xml file
    // Requires link to flow5-io-lib
    std::string xmlplanefilepath = "xml/plane.xml";
    std::string logmsg;
    std:: cout << "Making the plane from XML file" << std::endl;
    PlaneXfl* pPlaneXfl = io::importPlaneFromXML(xmlplanefilepath, logmsg);
    if(!pPlaneXfl)
    {
        std::cout << logmsg <<std::endl << std::endl;
        std::cout << "Aborting.\n";
        return 0;
    }
    else
    {
        // Store the pointer.
        Objects3d::insertPlane(pPlaneXfl);


        // Assemble the plane and build the triangular mesh for a thin surface calculation
        bool bThickSurfaces = false;
        std::cout << "    Building the parts and their meshes" <<std::endl;
        {
            // Build the plane and the individual part meshes
            // In the case of the fuselage this will create a default, non-conformant mesh.
            bool bIgnoreFusePanels = false; // unused in the present case, only applicable to quad meshes
            bool bMakeTriMesh = true;
            pPlaneXfl->makePlane(bThickSurfaces, bIgnoreFusePanels, bMakeTriMesh);
        }

        std::cout << "    Making a conforming mesh for the fuselage...\n";
        {
            // get a pointer to the fuselage object
            Fuse* pFuse = pPlaneXfl->fuse(0);
            // cast it to FuseXfl if need to access the methods in the derived class.
            //            FuseXfl* pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);

            // define the mesh settings
            // careful with the parameters
            pFuse->setGmshNCurvature(30); // 30 elements / 2.pi in the curvature
            pFuse->setGmshMinSize(0.030); // m
            pFuse->setGmshMaxSize(0.1); // m

            // select the wings connected to the fuselage,
            // i.e. main wing and fin in the present case
            std::vector<int> indexes = {0,2}; // indexes in addition order: 0=main wing, 1=elevator, 2=fin

            // make a conforming mesh for the fuselage
            // this will overwrite the default fuselage mesh
            std::string log;
            plane::meshFuse(pPlaneXfl, indexes, bThickSurfaces, gmesh::FRONTALDELAUNAY, log, "    ");
            std::cout << log << std::endl;
            std::cout << "    done fuse mesh" << std::endl;

            // Optional: make the fuselage's tessellation; for display only
            std::string logg;
            // define the tessellation settings
            // careful with the parameters
            pFuse->setGmshTessNCurvature(30); // 30 elements / 2.pi in the curvature
            pFuse->setGmshTessMinSize(0.015); // m
            pFuse->setGmshTessMaxSize(0.05); // m
            gmesh::makeFuseTriangulation(pFuse, logg, "   ");
        }
        std::cout << "done plane creation" << std::endl<< std::endl;

    }

    // Define an analysis from an Xml file
    // Requires link to flow5-io-lib
    std::string xmlpolarfilepath = "xml/T1-20.0 m_s-TriUniform-ThinSurf.xml";
    logmsg.clear();
    PlanePolar *pPlPolar = io::importAnalysisFromXML(xmlpolarfilepath, logmsg);
    if(!pPlPolar)
    {
        std::cout << logmsg <<std::endl << std::endl;
        std::cout << "Aborting.\n";
        return 0;
    }
    else
    {
        // In the present case we can ignore the name in the file
        // and just attach the polar to the plane
        pPlPolar->setPlaneName(pPlaneXfl->name());

        //  override viscous behaviour from file to keep it fast and simple
        pPlPolar->setViscous(false);

        // Store the pointer.
        Objects3d::insertPlPolar(pPlPolar);
    }

    // Define and run the analysis
    PlaneTask *pPlaneTask = new PlaneTask;
    {
        PanelAnalysis::setMaxThreadCount(16); // 1 by default

        pPlaneTask->outputToStdIO(true);
        pPlaneTask->setKeepOpps(true);

        pPlaneTask->setObjects(pPlaneXfl, pPlPolar);
        pPlaneTask->setComputeDerivatives(false);

        std::vector<double> opplist;
        for(int i=-3; i<14; i++) opplist.push_back(double(i));
        pPlaneTask->setOppList(opplist);

        // we are running the task in this thread, so there's
        // no stopping it once it's launched,
        pPlaneTask->run();

        // Results are automatically stored in the polar and
        // in the planeOpp array, so no action needed

        // print the results
        std::cout << std::format("Created %d plane operating points", int(pPlaneTask->planeOppList().size())) << std::endl << std::endl;

        std::string separator = ", ";
        std::string exportstr = pPlPolar->exportToString(separator);
        std::cout<<exportstr.c_str()<<std::endl;
        std::cout << std::endl;

        // clean up
        delete pPlaneTask;
    }

    // save the project; requires link to flow5-io-lib
    logmsg.clear();
    std::string projectfilepath;
    projectfilepath  = std::filesystem::temp_directory_path().string();
    projectfilepath += std::filesystem::path::preferred_separator;
    projectfilepath += "PlaneRun4.fl5";

    io::saveProject(projectfilepath, logmsg);

    if(logmsg.size()>0)
    {
        // error saving
        std::cerr << logmsg << std::endl << std::endl;
    }
    else
    {
        std::cout << "Successfully saved the project file to " << projectfilepath << std::endl << std::endl;
    }

    // Must call! will delete the planes, foils and children objects
    // Memory leak otherwise
    globals::deleteObjects();

    std::cout << "done" << std::endl;

    return 0;

}






