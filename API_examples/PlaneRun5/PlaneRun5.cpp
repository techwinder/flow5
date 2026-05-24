
#include <iostream>
#include <filesystem>
#include <format>

#include <api.h>
#include <constants.h>
#include <flow5-io.h>
#include <foil.h>
#include <fusestl.h>
#include <gmesh_globals.h>
#include <objects2d.h>
#include <objects3d.h>
#include <planexfl.h>
#include <planestl.h>


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
 * - import a fuselage from an STL type file into an xfl-type plane
 *
 * Copy the stl subdirectory into the build folder before running
 */
int main()
{
    #ifdef WIN32
    // enable UTF8 characters
    SetConsoleOutputCP(65001);
    #endif

    std::cout << std::endl << std::endl;

    printf("Plane with STL fuselage - build and calculation\n\n");

    // Configure LAPACK
    std::string strange;

    // Start by creating the foils needed to build the wings
    // flow5 objects, i.e. foils, planes, boats and their polar and opp children
    // should always be allocated on the heap

    // All units must be provided in I.S., i.e. meters and kg

    std::cout << "Creating the airfoils... "  << std::endl << std::endl;
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
    int  npanels = 150;
    double amp = 0.7; // 0.0: no bunching, 1.0: max. bunching
    pFoilN0009->rePanel(npanels, amp);
    pFoilN2413->rePanel(npanels, amp);

    // define the flaps
    pFoilN0009->setTEFlapData(true, 0.7, 0.5, 0.0); // stores the parameters but does not modify the geometry
    pFoilN2413->setTEFlapData(true, 0.7, 0.5, 0.0); // stores the parameters but does not modify the geometry


    // Create and define a new xfl-type plane with a Nurbs type fuselage
    // This plane will be meshed for a thin surface calculation
    std::cout << "Creating the plane"<<std::endl;
    PlaneXfl* pPlaneXfl = new PlaneXfl;
    {
        //Set the plane's name now to ensure the plane is inserted in alphabetical order
        pPlaneXfl->setName("Plane with STL type fuselage");
        pPlaneXfl->setDescription("xfl-type plane with fuselage imported from an STL file.");

        // We insert the plane = store the pointer
        // This ensures that the heap memory will not be lost and will be released properly
        // This should be done after the plane has been given a name
        Objects3d::insertPlane(pPlaneXfl);

        // set the style for this plane's children objects, i.e. polars and operating points
        pPlaneXfl->setTheStyle({true, Line::SOLID, 2, {71, 171, 231}, Line::NOSYMBOL});

        // Build the default plane, i.e. the one displayed by default in the plane editor
        // pPlaneXfl->makeDefaultPlane();

        // Build from scratch

        // Start with the fuselage
        // Import it from a file
        std::cout << "    Importing the fuselage" << std::endl;
        std::string fusefilepath = "stl/pw5_fuse.stl";
        std::string logmsg;
        FuseStl *pFuseStl = io::importFuseFromMesh(fusefilepath, io::STL, 1.0, logmsg); // file contains data in m
        std::cout << logmsg << std::endl;
        if(!pFuseStl)
        {
            std::cerr << "Error importing the fuselage... aborting";
            globals::deleteObjects();
            return 0;
        }
        else
        {
            pFuseStl->setName("STL imported fuse");
            pFuseStl->setDescription("PW-5 Smyk World-class glider.\n"
                                     "Author: Marek Cel\n"
                                     "https://airshow.openvsp.org/vsp/C1S5HvELccHjhxiiV8Ra");

            // add the fuselage to the plane
            pPlaneXfl->addFuse(pFuseStl);

            // scale it down to make it the fuselage of a model glider
            pFuseStl->scale(0.3, 0.3, 0.3);

            //position the fuse
            // make sure that it does not touch or intersect the wings
            pPlaneXfl->setFusePos(0, {-0.05, 0.0, 0.0});

            // The default fuse is a bit too long
            pFuseStl->scale(0.95, 1.0, 1.0);

            // convert spec to geometry
            pFuseStl->makeFuseGeometry();
        }

        // Add the main wing
        std::cout << "    Creating the main wing" << std::endl;
        WingXfl *pWing = pPlaneXfl->addWing();
        {
            pWing->setName("Main wing"); // for user information only
            pWing->makeDefaultWing();

            // The parts position and tilt angles in the plane's frame of reference are stored in the part itself.
            // These fields belong in fact to the plane, so this may change in a future version
            // Position the mainwing
            // flow5 works internally in IS units and expects all input in IS + degrees
            pPlaneXfl->setWingPosition(0, 0.0, 0.0, 0.075);

            // define the wing
            pWing->setColor({131, 177, 209});

            //insert a section between root and tip, i.e. between indexes 0 and 1
            pWing->insertSection(1);

            // Edit the geometry
            for(int isec=0; isec<pWing->nSections(); isec++)
            {
                // Get a reference to the wing section for ease of access
                WingSection &sec = pWing->section(isec);
                sec.setLeftFoilName(pFoilN2413->name());
                sec.setRightFoilName(pFoilN2413->name());
                // the number of chordwise panels - must be the same for all sections
                sec.setNX(13);
                // set a moderate panel concentration at LE and TE
                sec.setXDistType(xfl::TANH);
            }

            //root section
            WingSection &sec0 = pWing->rootSection(); // or pWing->section(0);
            sec0.setDihedral(3.5);
            sec0.setChord(0.29);
            sec0.setNY(13);
            sec0.setYDistType(xfl::UNIFORM);

            // mid section
            WingSection &sec1 = pWing->section(1);
            sec1.setXOffset(0.03); // the offset in the X direction
            sec1.setDihedral(7.5);
            sec1.setYPosition(0.95);
            sec1.setChord(0.21);
            sec1.setTwist(-2.5); // degrees
            sec1.setNY(19);
            sec1.setYDistType(xfl::INV_EXP); // moderate panel concentration at wing tip

            // tip section
            WingSection &sec2 = pWing->tipSection(); // or pWing->section(2);
            sec2.setYPosition(1.9);
            sec2.setXOffset(0.065); // the offset in the X direction
            sec2.setChord(0.12);
            sec2.setTwist(-3.5); // degrees

            //change the Aspect Ratio - why not?
            // first compute-update the geometry
            pWing->computeGeometry();
        }

        // add the elevator
        std::cout << "    Creating the elevator" << std::endl;
        WingXfl *pElev = pPlaneXfl->addWing();
        {
            pElev->setName("Elevator");
            pElev->makeDefaultStab();

            //position the elevator
            pPlaneXfl->setWingPosition(pElev, 1.05, 0.0, 0.280);

            // tilt the elevator down;
            pPlaneXfl->setRyAngle(pElev, -2.5); // degrees

            // Define the elevator
            pElev->setColor({173, 111, 57});

            // define the geometry
            for(int isec=0; isec<pElev->nSections(); isec++)
            {
                // Get a reference to the wing section for ease of access
                WingSection &sec = pElev->section(isec);
                sec.setLeftFoilName(pFoilN0009->name());
                sec.setRightFoilName(pFoilN0009->name());
                // the number of chordwise panels - must be the same for all sections
                sec.setNX(7); // prime numbers are perfect by nature
                // set a moderate panel concentration at LE and TE
                sec.setXDistType(xfl::TANH);
            }
            pElev->rootSection().setChord(0.15);

            pElev->tipSection().setXOffset(0.03);
            pElev->tipSection().setChord(0.09);
            pElev->tipSection().setYPosition(0.3);
        }

        // add a fin
        std::cout << "    Creating the fin" << std::endl;
        WingXfl *pFin = pPlaneXfl->addWing();
        {
            pFin->setName("Fin");
            pFin->makeDefaultFin();

            //position the fin
            pPlaneXfl->setWingPosition(2, 0.97, 0.0, 0.040);
            pPlaneXfl->setRxAngle(pFin, -90.0);

            // CLOSE the fin at its inner section since it is NOT connected to the fuselage
            pFin->setClosedInnerSide(true);

            for(int isec=0; isec<pFin->nSections(); isec++)
            {
                WingSection &sec = pFin->section(isec);
                sec.setLeftFoilName(pFoilN0009->name());
                sec.setRightFoilName(pFoilN0009->name());
                sec.setNX(7); // prime numbers are perfect by nature
                sec.setXDistType(xfl::TANH);
            }

            WingSection &rootsection = pFin->rootSection();
            rootsection.setChord(0.19);

            WingSection &tipsection = pFin->tipSection();
            tipsection.setYPosition(0.23);
            tipsection.setChord(0.09);
        }

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

        std::cout << "done Plane creation" << std::endl<< std::endl;
    }


    // save the project; requires link to flow5-io-lib
    std::string logmsg;
    std::string projectfilepath;
    projectfilepath  = std::filesystem::temp_directory_path().string();
    projectfilepath += std::filesystem::path::preferred_separator;
    projectfilepath += "PlaneRun5.fl5";


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
    std::cout << "Deleting objects" << std::endl;
    globals::deleteObjects();

    std::cout << "done" << std::endl;

    return 0;


}






