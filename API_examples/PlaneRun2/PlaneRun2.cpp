
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
#include <planeopp.h>
#include <planepolar.h>
#include <planepolarnamemaker.h>
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
 * - construct a plane with a fuselage
 * - build a conforming mesh on the fuselage
 * - run a calculation */
int main()
{
#ifdef WIN32
    // enable UTF8 characters
    SetConsoleOutputCP(65001);
#endif

    std::cout << std::endl << std::endl;

    printf("Plane with NURBS fuselage - build and calculation\n\n");

    // Configure LAPACK
    std::string strange;

#ifdef OPENBLAS
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
    gmsh::option::setNumber("General.NumThreads", 0); //Maximum number of threads used by Gmsh when compiled with OpenMP support (0: use system default, i.e. OMP_NUM_THREADS)
    //    gmsh::option::setNumber("Mesh.MaxNumThreads2D", 8); //Default value: 0; 0: use General.NumThreads


    std::string list;
    gmesh::listMainOptions(list);
    std::cout << list.c_str() << std::endl;

    // Start by creating the foils needed to build the wings
    // flow5 objects, i.e. foils, planes, boats and their polar and opp children
    // should always be allocated on the heap

    // All units must be provided in I.S., i.e. meters and kg

    std::cout << "Creating the airfoils... "  << std::endl << std::endl;

    int nPanels = 200;

    Foil *pFoilN2413 = new Foil;
    Objects2d::makeNacaFoil(pFoilN2413, 2413, nPanels);
    pFoilN2413->setName("NACA 2413");
    Objects2d::insertThisFoil(pFoilN2413);

    Foil *pFoilN0009 = new Foil;
    Objects2d::makeNacaFoil(pFoilN0009, 9, nPanels);
    pFoilN0009->setName("NACA 0009");
    Objects2d::insertThisFoil(pFoilN0009);

    // set the style for these foils and their children objects, i.e. polars and operating points
    pFoilN0009->setTheStyle({true, Line::SOLID, 2, {31, 111, 231}, Line::NOSYMBOL});
    pFoilN2413->setTheStyle({true, Line::SOLID, 2, {231, 111, 31}, Line::NOSYMBOL});

    // repanel
    nPanels = 150;
    double amp = 0.7; // 0.0: no bunching, 1.0: max. bunching
    pFoilN0009->rePanel(nPanels, amp);
    pFoilN2413->rePanel(nPanels, amp);

    // define the flaps
    pFoilN0009->setTEFlapData(true, 0.7, 0.5, 0.0); // stores the parameters but does not modify the geometry
    // in the case of the wing there will be need for two NACA 2413:  one flapped, one not
    Foil *pFoilN2413_flapped = new Foil(pFoilN2413); // duplicate
    pFoilN2413_flapped->setName("NACA 2413 flapped");
    pFoilN2413_flapped->setTEFlapData(true, 0.7, 0.5, 0.0); // stores the parameters but does not modify the geometry
    Objects2d::insertThisFoil(pFoilN2413_flapped);

    // Create and define a new xfl-type plane with a NURBS type fuselage
    // This plane will be meshed for a thin surface calculation
    std::cout << "Creating the plane" << std::endl;
    PlaneXfl* pPlaneXfl = new PlaneXfl;
    {
        //Set the plane's name now to ensure the plane is inserted in alphabetical order
        pPlaneXfl->setName("The fused plane!");

        // We insert the plane = store the pointer
        // This ensures that the heap memory will not be lost and will be released properly
        // This should be done after the plane has been given a name so that it is inserted
        // in alphabetical order
        Objects3d::insertPlane(pPlaneXfl);

        // set the style for this plane's children objects, i.e. polars and operating points
        pPlaneXfl->setTheStyle({true, Line::SOLID, 2, {71, 171, 231}, Line::NOSYMBOL});

        // Build the default plane, i.e. the one displayed by default in the plane editor
        // pPlaneXfl->makeDefaultPlane();

        // Build from scratch
        // Start with the main wing
        std::cout << "    Creating the main wing" << std::endl;
        WingXfl *pWing = pPlaneXfl->addWing();
        {
            pWing->setName("Main wing"); // for user information only
            pWing->makeDefaultWing();

            // The parts position and tilt angles are in the plane's frame of reference.
            // Position the mainwing
            // flow5 works internally in IS units and expects all input in IS + degrees
            pPlaneXfl->setWingPosition(pWing, 0,0,0);

            // define the wing
            pWing->setColor({131, 177, 209});

            // Get a ref to the wing's inertia properties
            Inertia &inertia = pWing->inertia();
            inertia.setStructuralMass(0.900);
            inertia.appendPointMass({0.03, {0.13,  0.15, 0.03}, "Right flap servo" });
            inertia.appendPointMass({0.03, {0.13, -0.15, 0.03}, "Left flap servo" });
            inertia.appendPointMass({0.03, {0.13,  0.95, 0.08}, "Right aileron servo" });
            inertia.appendPointMass({0.03, {0.13, -0.95, 0.08}, "Left aileron servo" });


            //insert a section between root and tip, i.e. between indexes 0 and 1
            pWing->insertSection(1);
            pWing->insertSection(1);

            // Edit the geometry
            for(int isec=0; isec<pWing->nSections(); isec++)
            {
                // Get a reference to the wing section for ease of access
                WingSection &sec = pWing->section(isec);
                // the number of chordwise panels - must be the same for all sections
                sec.setNX(13);
                // set a moderate panel density at LE and TE
                sec.setXDistType(xfl::TANH);
            }

            // root section
            WingSection &sec0 = pWing->rootSection(); // or pWing->section(0);
            // the goal is to leave the inner surface un-flapped
            sec0.setFoilNames(pFoilN2413->name(), pFoilN2413->name()); // left and right
            sec0.setDihedral(3.5);
            sec0.setChord(0.25);
            sec0.setNY(2);
            sec0.setYDistType(xfl::UNIFORM);

            // second section is positioned just outside the fuselage to keep the fuselage's conforming to the wing
            // irrespective of the flap deflection
            WingSection &sec1 = pWing->section(1);
            sec1.setFoilNames(pFoilN2413_flapped->name(), pFoilN2413_flapped->name()); // left and right
            sec1.setXOffset(0.005); // the offset in the X direction
            sec1.setDihedral(3.5);
            sec1.setYPosition(0.08); // just outside the fuselage
            sec1.setChord(0.240);
            sec1.setTwist(0.0); // degrees
            sec1.setNY(13);
            sec1.setYDistType(xfl::EXP); // make a dense mesh at the inner flap edge


            // mid section
            WingSection &secmid = pWing->section(2);
            secmid.setFoilNames(pFoilN2413_flapped->name(), pFoilN2413_flapped->name()); // left and right
            secmid.setXOffset(0.03); // the offset in the X direction
            secmid.setDihedral(7.5);
            secmid.setYPosition(0.75);
            secmid.setChord(0.200);
            secmid.setTwist(-2.5); // degrees
            secmid.setNY(13);
            secmid.setYDistType(xfl::INV_EXP);

            // tip section
            WingSection &sectip = pWing->tipSection(); // or pWing->section(2);
            sectip.setFoilNames(pFoilN2413_flapped->name(), pFoilN2413_flapped->name()); // left and right
            sectip.setYPosition(1.47);
            sectip.setXOffset(0.065); // the offset in the X direction
            sectip.setChord(0.13);
            sectip.setTwist(-3.5); // degrees

            //change the Aspect Ratio - just to demonstrate
            // first compute-update the geometry
            pWing->computeGeometry();
            // scale
            pWing->scaleAR(17.0);

        }

        // add the elevator
        std::cout << "    Creating the elevator" << std::endl;
        WingXfl *pElev = pPlaneXfl->addWing();
        {
            pElev->setName("Elevator");
            pElev->makeDefaultStab();
            pElev->setColor({173, 111, 57});

            //position the elevator
            pPlaneXfl->setWingPosition(pElev, 0.910, 0.0, 0.250);
            pPlaneXfl->setRyAngle(pElev, -2.5); // degrees

            // define the inertia
            Inertia &inertia = pElev->inertia();
            inertia.setStructuralMass(0.120);

            // define the geometry
            for(int isec=0; isec<pElev->nSections(); isec++)
            {
                // Get a reference to the wing section for ease of access
                WingSection &sec = pElev->section(isec);
                sec.setLeftFoilName(pFoilN0009->name());
                sec.setRightFoilName(pFoilN0009->name());
                // the number of chordwise panels - must be the same for all sections
                sec.setNX(7);
                // set a moderate panel concentration at LE and TE
                sec.setXDistType(xfl::TANH);
            }
            pElev->rootSection().setChord(0.13);
            pElev->tipSection().setXOffset(0.01);
            pElev->tipSection().setYPosition(0.247);
        }

        // add a fin
        std::cout << "    Creating the fin" << std::endl;
        WingXfl *pFin = pPlaneXfl->addWing();
        {
            pFin->setName("Fin");
            pFin->makeDefaultFin();

            //position the fin
            pPlaneXfl->setWingPosition(pFin, 0.870, 0.0, 0.010);
            pPlaneXfl->setRxAngle(pFin, -90.0);

            // Define the Fin

            pFin->inertia().setStructuralMass(0.075);

            // Open the fin at its inner section since it is connected to the fuselage
            // However in the present case this is redundant because we will be constructing
            // a thin surface mesh
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

        // add a fuselage
        std::cout << "    Creating the fuselage" << std::endl;
        Fuse *pFuse = pPlaneXfl->makeNewFuse(Fuse::NURBS);
        {
            // cast to NURBS type
            FuseNurbs *pFuseNurbs = dynamic_cast<FuseNurbs*>(pFuse);
            assert(pFuseNurbs);        // check that the cast has been successful

            //position the fuse
            pPlaneXfl->setFusePos(0, {-0.450, 0.0, 0.0});

            pFuse->setName("The fuse");  // for information only
            // add meta-data
            pFuse->setDescription("This is a NURBS type fuselage");

            // set the structural masse
            pFuse->setStructuralMass(0.75); // kg

            // scale frame nummber 4, i.e. index=3 since C-arrays start at index 0
            pFuseNurbs->scaleFrame(0.7, 0.7, 3);

            // The default fuse is a bit too long
            pFuseNurbs->scale(0.95, 1.0, 1.0);

            // convert spec to geometry
            pFuse->makeFuseGeometry();


            // define the mesh parameters
            // careful with the parameters
            pFuse->setGmshNCurvature(30); // 30 elements / 2.pi in the curvature
            pFuse->setGmshMinSize(0.030); // m
            pFuse->setGmshMaxSize(0.1); // m

            // Optional: make a triangular mesh
            // In the present case, would be overwritten by the conforming mesh at the next step anyway
            //            std::string logmsg;
            //            pFuse->makeDefaultTriMesh(logmsg, "");

            // Optional: make the fuselage's tessellation; for display only
            std::string logg;
            gmesh::makeFuseTriangulation(pFuse, logg, "   ");
        }


        // Complete the inertia properties
        std::cout << "    Defining the inertia" << std::endl;
        {
            Inertia &inertia = pPlaneXfl->inertia();
            // could also add these point masses to the fuselage instead
            inertia.appendPointMass(0.30, {-0.35,0,0},  "Nose lead");
            inertia.appendPointMass(0.20, {-0.25,0,0},  "Battery and receiver");
            inertia.appendPointMass(0.10, {-0.05,0,0},  "Two servos");
        }

        // Assemble the plane and build the triangular mesh for a thin surface calculation
        bool bThickSurfaces = false;

        std::cout << "    Building the parts and their meshes" <<std::endl;
        {
            // Build the plane and the individual part meshes
            // This will create a default, non-conformant mesh for the fuselage which will be overwritten at the next step
            bool bIgnoreFusePanels = false; // unused in the present case, only applicable to quad meshes
            bool bMakeTriMesh = true;
            pPlaneXfl->makePlane(bThickSurfaces, bIgnoreFusePanels, bMakeTriMesh);
        }

        std::cout << "    Making a conforming mesh for the fuselage... ";
        {
            // select the wings connected to the fuselage,
            // i.e. main wing and fin in the present case
            std::vector<int> indexes = {0,2}; // indexes in addition order: 0=main wing, 1=elevator, 2=fin

            // make a conforming mesh for the fuselage
            // this will overwrite the default fuselage mesh
            std::string log;
            plane::meshFuse(pPlaneXfl, indexes, bThickSurfaces, gmesh::FRONTALDELAUNAY, log, "   ");
            std::cout << "    done fuse mesh" << std::endl;
        }
        std::cout << "done plane creation" << std::endl<< std::endl;
    }

    // Define an analysis
    std::cout << "Defining the polar" << std::endl<< std::endl;
    PlanePolar *pPlPolar = new PlanePolar;
    {
        // give the polar a temporary name
        //        pPlPolar->setName("a T2 polar");

        pPlPolar->setTheStyle({true, Line::SOLID, 2, {239, 51, 153}, Line::NOSYMBOL});

        // attach the polar to the plane
        pPlPolar->setPlaneName(pPlaneXfl->name());
        // define the properties
        pPlPolar->setType(xfl::T2POLAR);
        pPlPolar->setAnalysisMethod(xfl::TRIUNIFORM);
        pPlPolar->setReferenceDim(xfl::PROJECTED);

        pPlPolar->setReferenceArea(pPlaneXfl->projectedArea());
        pPlPolar->setReferenceSpanLength(pPlaneXfl->projectedSpan());
        pPlPolar->setReferenceChordLength(pPlaneXfl->mac());

        pPlPolar->setThinSurfaces(true);
        pPlPolar->setViscous(true);
        pPlPolar->setViscOnTheFly(true);
        pPlPolar->setTransAtHinge(true);

        // [Optional]: define flap settings
        // This polar will simulate a flap down configuration
        // Resize the number of ctrls to match the number of wings
        pPlPolar->resizeFlapCtrls(pPlaneXfl);
        {
            // sanity check: the number of ctrls is the same as the number of wings
            assert(pPlPolar->nFlapCtrls()==pPlaneXfl->nWings()); // since all the wings are flapped

            // give the control a name
            pPlPolar->setFlapCtrlsName("Flaps down");

            // get a reference to the main wing's flap controls
            AngleControl &mainwingctrls = pPlPolar->flapCtrls(0);
            {
                // sanity check: the number of flap deflections should be the same
                // as the main wing's number of flaps, i.e. 4
                assert(mainwingctrls.nValues()==pPlaneXfl->mainWing()->nFlaps());

                // Flaps are numbered from left to right
                // Set their deflection, + is down, unit is degrees
                // Note: arrays is C are indexed starting at 0
                mainwingctrls.setValue(0, +5);
                mainwingctrls.setValue(1, +5);
                mainwingctrls.setValue(2, +5);
                mainwingctrls.setValue(3, +5);
            }

            // get a reference to the elevator's flap controls
            AngleControl &elevctrls = pPlPolar->flapCtrls(1);
            {
                // the elevator has been defined with two flaps
                elevctrls.setValue(0, +3);
                elevctrls.setValue(1, +3);
            }

            // The fin's flap is left to its default value = 0°
            // A deflectd flap would break the conformity of fuselage and fin meshes
            // which in this case wouldn't be a big deal anyway
        }

        // leave the rest of the fields to their default values

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


    // Run the calculation
    std::cout << "Building the plane calculation task"<<std::endl<< std::endl;
    PlaneTask *pPlaneTask = new PlaneTask;
    {
        pPlaneTask->outputToStdIO(true);
        pPlaneTask->setKeepOpps(true);

        pPlaneTask->setObjects(pPlaneXfl, pPlPolar);
        pPlaneTask->setComputeDerivatives(false);

        // Create a vector of operating point parameters to calculate
        // Unlike in the foil case, the order of calculation is unimportant,
        // so there is no needed for ranges; an unordered list is what is needed
        std::vector<double> opplist{-1.0, 1.0, 3.0, 5.0, 7.0, 9.0, 11.0};
        pPlaneTask->setOppList(opplist);


        // we are running the task in this thread, so there's
        // no stopping it once it's launched,
        std::cout << "Launching the calculation" << std::endl;
        pPlaneTask->run();

        // Results are automatically stored in the polar and
        // in the PlaneOpp arrays, so no action needed


        // print the results
        printf("Created %d plane operating points\n\n", int(pPlaneTask->planeOppList().size()));

        std::string separator = ", ";
        std::string exportstr = pPlPolar->exportToString(separator);
        std::cout<<exportstr.c_str()<<std::endl;
        printf("\n");

        // clean up
        delete pPlaneTask;
    }
    std::cout << "Done calculation" << std::endl;


    // save the project; requires link to flow5-io-lib
    std::string logmsg;
    std::string projectfilepath;
    projectfilepath  = std::filesystem::temp_directory_path().string();
    projectfilepath += std::filesystem::path::preferred_separator;
    projectfilepath += "PlaneRun2.fl5";

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






