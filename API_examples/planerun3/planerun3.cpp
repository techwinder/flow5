
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
 * - import a plane from an OBJ or STL mesh file
 * - configure the plane and build its trailing edge
 * - run a calculation */
int main()
{
    #ifdef WIN32
    // enable UTF8 characters
    SetConsoleOutputCP(65001);
    #endif

    std::cout << std::endl << std::endl;

    printf("Running a calcualtion on a plane imported from a mesh file\n\n");

    // flow5 works internally in IS units
    // All inputs should be provided in the IS system, i.e. meters and kilograms

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


    // Preload some project file
    /*
     *    std::string logload;
     *    std::string loadfilepath = "/path/to/file.fl5";
     *    if(!io::loadProject(loadfilepath, logload))
     *    {
     *        std::cerr << logload << std::endl;
}
*/

    std::string STLFilePath = "/path/to/mesh.stl";
//    std::string OBJFilePath = "/path/to/file.obj";
    std::string logmsg;

    // Create and define a new xfl-type plane
    std::cout << "Importing the STL plane: " << STLFilePath << std::endl;
    double FileUnitsToMeter = 1.0; // the factor by which the file units should be multiplied to produce meters
    PlaneSTL* pPlane = io::importPlaneFromMesh(STLFilePath, io::STL, FileUnitsToMeter, logmsg);
//    PlaneSTL* pPlane = io::importPlaneFromMesh(OBJFilePath, io::OBJ, FileUnitsToMeter, logmsg);
    std::cout<<logmsg<<std::endl;

    if(!pPlane)
    {
        std::cerr << "failed to import the plane... aborting"<< std::endl;
        std::cout <<"_____Done_____" << std::endl<<std::endl;
        return 0;
    }
    else
    {
        //Set the plane's name now to ensure the plane is inserted in alphabetical order
        pPlane->setName("Plane from STL");

        // We insert the plane = store the pointer
        // This ensures that the heap memory will not be lost and will be released properly
        // This should be done after the plane has been given a name
        Objects3d::insertPlane(pPlane);

        // set the triangle color
        pPlane->setSurfaceColor({73,101,137});

        // set the style for this plane's children objects, i.e. polars and operating points
        pPlane->setTheStyle({true, Line::SOLID, 2, {237, 71, 171}, Line::NOSYMBOL});

        // set the reference dimensions
        pPlane->setRefArea(0.2); // m²
        pPlane->setRefChord(0.02); // m
        pPlane->setRefSpan(2.0); //m

        pPlane->makePlane(false, false, false); // parameters are ignored

        pPlane->rotate(Vector3d(), {1.0, 0.0, 0.0}, 90.0);

        // need to connect the panels before attempting to guess the T.E.
        std::cout << "Connecting triangles... ";
        if(!pPlane->connectTriMesh(true, pPlane->isSTLType()))
        {
            std::cout << std::endl;
            std::cerr << " error connecting panels ...aborting\n\n";
            delete pPlane;
            return 0;
        }
        else
        {
            std::cout << " done." << std::endl << std::endl;
        }

        float GuessAngle = 25.0;

        // The tricky part: no guarantee that the TE panels have been correctly identified, and no
        // available way to check using the API.
        // The error list only covers the list of TE panels which do not have an opposite TE panel
        logmsg.clear();
        if(!pPlane->guessTEPanels(GuessAngle, logmsg))
        {
            std::cout << logmsg;
            std::cerr << "Error guessing trailing edge... aborting" <<std::endl;
            delete pPlane;
            return 0;
        }
        std::cout << logmsg << std::endl;

    }
    std::cout << std::endl;

    // Define an analysis
    std::cout << "Defining the polar" << std::endl;
    PlanePolar *pPlPolar = new PlanePolar;
    {
        pPlPolar->setTheStyle({true, Line::SOLID, 2, {131,29,251}, Line::NOSYMBOL});

        // attach the polar to the plane
        pPlPolar->setPlaneName(pPlane->name());
        // define the properties
        pPlPolar->setType(xfl::T1POLAR); // keep it simple
        pPlPolar->setAnalysisMethod(xfl::TRIUNIFORM); // Triangle methods only in the case of STL planes
        pPlPolar->setReferenceDim(xfl::PROJECTED);

        pPlPolar->setReferenceArea(pPlane->projectedArea());
        pPlPolar->setReferenceSpanLength(pPlane->projectedSpan());
        pPlPolar->setReferenceChordLength(pPlane->mac());

        pPlPolar->setThickSurfaces(true); // No choice, must be set explicitely
        pPlPolar->setViscous(false); // No choice since there are no airfoils
        // leave the rest of the fields to their default values

        // Use flow5's default name maker
        PlanePolarNameMaker maker;
        std::string polarname = PlanePolarNameMaker::makeName(pPlane, pPlPolar);
        pPlPolar->setName(polarname);
        // Store the pointer to ensure that the object is not lost
        // This should be done after the polar has been given a name
        // since objects are referenced by their name and are stored
        // in alphabetical order
        Objects3d::insertPlPolar(pPlPolar);
    }
    std::cout << std::endl;

    // Define and run the analysis
    std::cout << "Defining the plane calculation task" << std::endl;
    PlaneTask *pPlaneTask = new PlaneTask;
    {
        PanelAnalysis::setMaxThreadCount(16); // 1 by default

        pPlaneTask->outputToStdIO(true); // output to the terminal
        pPlaneTask->setKeepOpps(true); // keep the operating points

        pPlaneTask->setObjects(pPlane, pPlPolar);
        pPlaneTask->setComputeDerivatives(false);

        // Create a vector of operating point parameters to calculate
        // Unlike in the foil case, the order of calculation is unimportant,
        // so there is no needed for ranges; an unordered list is what is needed

        pPlaneTask->setOppList({3.1416});


        // we are running the task in this thread, so there's
        // no stopping it once it's launched,
        std::cout << "Launching the task" << std::endl;

        pPlaneTask->run();

        // Results are automatically stored in the polar and
        // in the planeOpp array, so no action needed


        // print the results
        printf("Created %d plane operating points\n\n", int(pPlaneTask->planeOppList().size()));

        printf("Polar data:\n\n");
        std::string separator = ", ";
        std::string exportstr = pPlPolar->exportToString(separator);
        std::cout<<exportstr.c_str()<<std::endl;
        printf("\n");

        // clean up
        delete pPlaneTask;
    }

    // save the project; requires link to flow5-io-lib
    std::cout << "Saving the project to file" << std::endl;
    {
        std::string logmsg;
        std::string projectfilepath;
        projectfilepath  = std::filesystem::temp_directory_path().string();
        projectfilepath += std::filesystem::path::preferred_separator;
        projectfilepath += "PlaneRun3.fl5";

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
    }
    std::cout << std::endl;

    // Must call! will delete the planes, foils and children objects
    // Memory leak otherwise
    std::cout << "Deleting objects"<<std::endl<<std::endl;
    globals::deleteObjects();



    std::cout << "_________done_____________" << std::endl  << std::endl ;

    return 0;

}






