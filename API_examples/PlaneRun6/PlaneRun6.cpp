
#include <iostream>
#include <filesystem>
#include <format>


#include <api.h>
#include <flow5-io.h>
#include <foil.h>
#include <fuseocc.h>
#include <gmesh_globals.h>
#include <objects2d.h>
#include <objects3d.h>
#include <occ_globals.h>
#include <planepolar.h>
#include <planepolarnamemaker.h>
#include <planexfl.h>


#ifdef WIN32
#include <Windows.h>
#endif



/* This example shows how to
 * - import a fuselage from a STEP file into an xfl-type plane
 *
 * Copy the step subdirectory into the build folder before running
 */
int main()
{

#ifdef WIN32
    // enable UTF8 characters
    SetConsoleOutputCP(65001);
#endif

    std::cout << std::endl << std::endl;

    std:: cout << "Plane with fuselage from STEP file" << std::endl << std::endl;

    gmsh::initialize();
    gmsh::option::setNumber("General.Terminal", 0);
    gmsh::option::setNumber("Geometry.OCCParallel", 1.0);
    gmsh::option::setNumber("General.NumThreads", 0);
    //    gmsh::option::setNumber("Mesh.MaxNumThreads2D", 8); //Default value: 0; 0: use General.NumThreads

    // Start by creating the foils needed to build the wings
    // flow5 objects, i.e. foils, planes, boats and their polar and opp children
    // should always be allocated on the heap

    // All units must be provided in I.S., i.e. meters and kg

    std::cout << "Creating the airfoils... "  << std::endl << std::endl;

    Foil *pFoilN2413 = new Foil;
    Objects2d::makeNacaFoil(pFoilN2413, 2413, 200);
    pFoilN2413->setName("NACA 2413");
    Objects2d::insertThisFoil(pFoilN2413);

    Foil *pFoilN0009 = new Foil;
    Objects2d::makeNacaFoil(pFoilN0009, 9, 200);
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

        //Set the plane's name now to ensure that it is inserted in alphabetical order
        pPlaneXfl->setName("Plane with STEP type fuselage");
        pPlaneXfl->setDescription("xfl-type plane with PW5 fuselage imported from a STEP file generated in OpenVSP.\n");

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
        // This example uses the fuselage of the PW5 glider found in the VSP airshow
        //    PW-5 Smyk World-class glider.
        //    Author: Marek Cel
        //    https://airshow.openvsp.org/vsp/C1S5HvELccHjhxiiV8Ra
        // It has been exported to STEP using OpenVSP's default options

        // make sure that the STEP file has been copied into the build directory, or update the path
        std::string fusefilepath = "step/pw5_2024-06-27T20_19_49.010Z.stp";

        std::string logmsg;
        FuseOcc *pFuseOcc = io::importFuseFromSTEP(fusefilepath, logmsg);
        std::cout << logmsg << std::endl;
        if(!pFuseOcc)
        {
            std::cerr << "Error importing the fuselage... aborting\n\n";
            globals::deleteObjects();
            return 0;
        }
        else
        {
            // The fuselage has been created successfully which means that the SHAPE(s)
            // in OCCT terminology have been read and stored in the FuseOcc object.
            // However these raw shapes are not suitable for flow5 if they do not contain a SHELL.
            // flow5 requires that a fuselage contains at least a SHELL to build the mesh on.
            // https://flow5.tech/docs/flow5_doc/Modelling/Fuse_CAD.html

            pFuseOcc->setName("STEP imported fuse");


            std::string description = "PW-5 Smyk World-class glider.\n"
                                      "Author: Marek Cel\n"
                                      "https://airshow.openvsp.org/vsp/C1S5HvELccHjhxiiV8Ra";
            pFuseOcc->setDescription(description);

            // list what has been imported - for information only
            logmsg.clear();
            occ::listAllShapes(pFuseOcc->shapes(), logmsg);
            std::cout << logmsg << std::endl;

            // check if there are any shells in the imported shapes
            logmsg.clear();
            int nShells = occ::shellCount(pFuseOcc->shapes(), logmsg);
            std::cout << "Found " << nShells << " shell(s) in the imported list of shapes" << std::endl;

            if(nShells==0)
            {
                // The example STEP file does not contain any SHELL or SOLID,
                // so the FACEs must be stiched together to create one

/*                Before stitching it may be necessary to heal the imported shapes.
                 *   See the link below for explanations and for the definition of Precision, MinTolerance and MaxTolerance.
                 *   https://dev.opencascade.org/doc/overview/html/occt_user_guides__shape_healing.html
                 */
                double Precision = 1.e-4;
                //            double MinTolerance = 1.e-4;
                //            double MaxTolerance = 1.e-3;

/*            // [optional: fix gaps before attempting to stitch]
                 *   // OCCT Doc: "Fixes gaps between ends of curves of adjacent edges (both 3d and pcurves) in wires"
                 *   logmsg.clear();
                 *   occ::fuseFixGaps(pFuseOcc, Precision, MinTolerance, MaxTolerance, logmsg);
                 *   std::cout << logmsg << std::endl;*/

/*            // [optional: fix small edges before attempting to stitch]
                 *   // OCCT Doc: "Fixes small edges in shape by merging adjacent edges"
                 *   logmsg.clear();
                 *   occ::fuseFixSmallEdges(pFuseOcc, Precision, MinTolerance, MaxTolerance, logmsg);
                 *   std::cout << logmsg << std::endl;*/

                // For some reason, shapes in STEP files imported from OpenVSP are inverted, so restore orientation
                std::cout << "Reversing orientation of imported shapes" << std::endl;
                logmsg.clear();
                occ::fuseReverseShapes(pFuseOcc, logmsg);
                std::cout << logmsg << std::endl;

                std::cout << "Stitching FACE(s) present in imported shapes" << std::endl;
                logmsg.clear();
                occ::fuseStitchFaces(pFuseOcc, Precision, logmsg);
                // In the present case, the stitching operation has created a SOLID, which means
                // that the SHELL is watertight - precisely what is required by the Boundary Element Method
                std::cout << logmsg << std::endl;

                // flow5 however has no use for the SOLID and only requires the external SHELL bounding this SOLID
                // So extract the shell and store it in the fuselage object.
                std::cout << "Extracting and storing SHELL(s) present in the fuselage's list of shapes" << std::endl;
                pFuseOcc->extractShellsFromShapes();
                std::cout << std::format("Extracted {:d} shell(s)", pFuseOcc->nShells()) <<std::endl;

                // One shell has been created, so we can continue to build and mesh the plane
            }

            // Build the tessellation using the default parameters
            // used for display and to evaluate the structural ineria
            logmsg.clear();
            gmesh::makeFuseTriangulation(pFuseOcc, logmsg);
            pFuseOcc->makeFuseGeometry();

            // Define the mesh parameters;
            // Careful not to set too small element sizes which would lead to long meshing times
            // and large mesh sizes
            pFuseOcc->setGmshNCurvature(30); // 30 elements / 2.pi in the curvature
            pFuseOcc->setGmshMinSize(0.03); // m
            pFuseOcc->setGmshMaxSize(0.1); // m

            // add the fuselage to the plane
            pPlaneXfl->addFuse(pFuseOcc);

            //position the fuse
            pPlaneXfl->setFusePos(0, {-0.13, 0.0, 0.0});
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
            pPlaneXfl->setWingPosition(pWing, -0.05, 0.0, 0.0);

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
            pPlaneXfl->setWingPosition(pElev, 1.03, 0.0, 0.260);

            // tilt the elevator down
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

            pPlaneXfl->setWingPosition(pFin, 0.97, 0.0, 0.020);
            pPlaneXfl->setRxAngle(2, -90.0);

            // OPEN the fin at its inner section since it is connected to the fuselage
            pFin->setClosedInnerSide(false);

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

        std::cout << "    Making a conforming mesh for the fuselage... ";
        {
            // Note that gmsh DOES NOT ALWAYS succeed in creating a valid mesh
            // No way to test the result, other than by vizualizing the mesh in flow5

            // select the wings connected to the fuselage,
            // i.e. main wing and fin in the present case
            std::vector<int> indexes = {0,2}; // indexes in addition order: 0=main wing, 1=elevator, 2=fin

            // make a conforming mesh for the fuselage
            // this will overwrite the default fuselage mesh
            std::string log;

            plane::meshFuse(pPlaneXfl, indexes, bThickSurfaces, gmesh::FRONTALDELAUNAY, log, "   ");
            std::cout << "    done Fuse mesh" << std::endl;
        }
        std::cout << "done plane creation" << std::endl<< std::endl;

    }

    // Define a tri-uniform analysis to enable visualization of the triangular mesh in flow5
    std::cout << "Defining the polar" << std::endl<< std::endl;
    PlanePolar *pPlPolar = new PlanePolar;
    {
        pPlPolar->setTheStyle({true, Line::SOLID, 2, {239, 51, 153}, Line::NOSYMBOL});

        // attach the polar to the plane
        pPlPolar->setPlaneName(pPlaneXfl->name());
        // define the properties
        pPlPolar->setType(xfl::T1POLAR);
        pPlPolar->setVelocity(30.0); // m/s
        pPlPolar->setAnalysisMethod(xfl::TRIUNIFORM);
        pPlPolar->setReferenceDim(xfl::PROJECTED);

        pPlPolar->setReferenceArea(pPlaneXfl->projectedArea());
        pPlPolar->setReferenceSpanLength(pPlaneXfl->projectedSpan());
        pPlPolar->setReferenceChordLength(pPlaneXfl->mac());

        pPlPolar->setThinSurfaces(true);
        pPlPolar->setViscous(false); //keep it fast and simple


        // Now that the polar's parameters have been defined,
        // it is possible to use flow5's default name maker
        PlanePolarNameMaker maker;
        std::string polarname = PlanePolarNameMaker::makeName(pPlaneXfl, pPlPolar);
        pPlPolar->setName(polarname);
        // Store the pointer to ensure that the object is not lost
        // This should be done after the polar has been given a name
        // since objects are referenced by their name and are stored
        // in alphabetical order
        Objects3d::insertPlPolar(pPlPolar);
    }

    // save the project; requires link to flow5-io-lib
    std::string logmsg;
    std::string projectfilepath;
    projectfilepath  = std::filesystem::temp_directory_path().string();
    projectfilepath += std::filesystem::path::preferred_separator;
    projectfilepath += "PlaneRun6.fl5";

    if(!io::saveProject(projectfilepath, logmsg))
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






