
#include <iostream>
#include <format>

#include <api.h>
#include <constants.h>
#include <foil.h>
#include <objects2d.h>
#include <objects3d.h>
#include <oppoint.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planetask.h>
#include <planexfl.h>
#include <polar.h>
#include <xfoiltask.h>



int main()
{
    printf("flow5 plane run\n");

    // Start by creating the foils needed to build the wings
    // flow5 objects, i.e. foils, planes, boats and their polar and opp children
    // should always be allocated on the heap

    Foil *pFoilN2413 = foil::makeNacaFoil(2413, "NACA 2413");
    Foil *pFoilN0009 = foil::makeNacaFoil(9,    "NACA 0009");
    {
        if(!pFoilN0009 || !pFoilN2413)
        {
            // failsafe; this should not happen
            std::cout <<"Error creating the foils ...aborting" << std::endl;
            if(pFoilN0009) delete pFoilN0009;
            if(pFoilN2413) delete pFoilN2413;
            return 0;
        }


        // set the style for these foils and their children objects, i.e. polars and operating points
        pFoilN0009->setTheStyle({true, Line::SOLID, 2, {31, 111, 231}, Line::NOSYMBOL});
        pFoilN2413->setTheStyle({true, Line::SOLID, 2, {231, 111, 31}, Line::NOSYMBOL});


        // repanel
        int  npanels = 149; // prefer primes
        double amp = 0.7; // 0.0: no bunching, 1.0: max. bunching
        pFoilN0009->rePanel(npanels, amp);
        pFoilN2413->rePanel(npanels, amp);

        // define the flaps
        pFoilN0009->setTEFlapData(true, 0.7, 0.5, 0.0); // stores the parameters
        pFoilN2413->setTEFlapData(true, 0.7, 0.5, 0.0); // stores the parameters
    }


    // Create and define a new xfl-type plane
    PlaneXfl* pPlaneXfl = new PlaneXfl;
    {
        //Set the plane's name now to ensure the plane is inserted in alphabetical order
        pPlaneXfl->setName("The Plane!");

        // We insert the plane = store the pointer
        // This ensures that the heap memory will not be lost and will be released properly
        // This should be done after the plane has been given a name so that it may be
        // inserted in alphabetical order
        Objects3d::insertPlane(pPlaneXfl);

        // set the style for this plane's children objects, i.e. polars and operating points
        pPlaneXfl->setTheStyle({true, Line::SOLID, 2, {71, 171, 231}, Line::NOSYMBOL});

        // Build the default plane, i.e. the one displayed by default in the plane editor
        // Could also build from scratch
        pPlaneXfl->makeDefaultPlane();

        // Set the inertia properties
        // All units must be provided in I.S. standard, i.e. meters ang kg
        Inertia &inertia = pPlaneXfl->inertia();
        inertia.appendPointMass(0.20, {-0.35,0,0},  "Nose lead");
        inertia.appendPointMass(0.20, {-0.25,0,0},  "Battery and receiver");
        inertia.appendPointMass(0.10, {-0.05,0,0},  "Two servos");
        inertia.appendPointMass(0.15, {-0.20,0,0},  "Fuse fore");
        inertia.appendPointMass(0.30, { 0.40,0,0},  "Fuse mid");
        inertia.appendPointMass(0.10, { 0.70,0,0},  "Fuse aft");

        // Define the main wing
        {
            // Get a reference to the main wing object for ease of access
            WingXfl &mainwing = *pPlaneXfl->mainWing(); // or pPlaneXfl->wing(0);
            mainwing.setColor({131, 177, 209});

            // Get a ref to the wing's inertia properties
            Inertia &inertia = mainwing.inertia();
            inertia.setStructuralMass(0.25);
            inertia.appendPointMass({0.03, {0.13,  0.15, 0.03}, "Right flap servo" });
            inertia.appendPointMass({0.03, {0.13, -0.15, 0.03}, "Left flap servo" });
            inertia.appendPointMass({0.03, {0.13,  0.95, 0.08}, "Right aileron servo" });
            inertia.appendPointMass({0.03, {0.13, -0.95, 0.08}, "Left aileron servo" });


            // The wing's position in the plane's frame of reference is stored in the wing itself
            // The field belongs in fact to the plane, so this may change in a future version
            mainwing.setPosition(0,0,0);

            //insert a section between root and tip, i.e. between indexes 0 and 1
            mainwing.insertSection(1);

            // Edit the geometry
            for(int isec=0; isec<mainwing.nSections(); isec++)
            {
                // Get a reference to the wing section for ease of access
                WingSection &sec = mainwing.section(isec);
                sec.setLeftFoilName(pFoilN2413->name());
                sec.setRightFoilName(pFoilN2413->name());
                // the number of chordwise panels - must be the same for all sections
                sec.setNX(13);
                // set a moderate panel concentration at LE and TE
                sec.setXDistType(xfl::TANH);
            }

            //root section
            WingSection &sec0 = mainwing.rootSection(); // or mainwing.section(0);
            sec0.setDihedral(3.5);
            sec0.setChord(0.27);
            sec0.setNY(13);
            sec0.setYDistType(xfl::UNIFORM);

            // mid section
            WingSection &sec1 = mainwing.section(1);
            sec1.setXOffset(0.03); // the offset in the X direction
            sec1.setDihedral(7.5);
            sec1.setYPosition(0.9);
            sec1.setChord(0.21);
            sec1.setTwist(-2.5); // degrees
            sec1.setNY(19);
            sec1.setYDistType(xfl::INV_EXP); // moderate panel concentration at wing tip

            // tip section
            WingSection &sec2 = mainwing.tipSection(); // or mainwing.section(2);
            sec2.setYPosition(1.47);
            sec2.setChord(0.13);
            sec2.setTwist(-3.5); // degrees
        }

        // Define the elevator
        {
            WingXfl *pElev = pPlaneXfl->stab(); // or pPlaneXfl->wing(1);
            pElev->setColor({173, 111, 57});

            // position the elevator
            pElev->setPosition(0.970, 0.0, 0.210);
            // tilt the elevator down; this field belongs to the plane
            pElev->setRy(-2.5); // degrees

            // define the inertia
            Inertia &inertia = pElev->inertia();
            inertia.setStructuralMass(0.05);

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
            pElev->rootSection().setChord(0.13);
            pElev->tipSection().setXOffset(0.01);
            pElev->tipSection().setYPosition(0.247);
        }

        // Define the Fin
        {
            // get a convenience reference or a pointer for ease of access
            WingXfl &fin = *pPlaneXfl->fin();
        //    WingXfl *pFin = pPlaneXfl->fin(); // or pPlaneXfl->wing(2);

            fin.inertia().setStructuralMass(0.035);


            fin.setPosition(0.930, 0.0, 0.010);
            // Make double sure that the fin is closed on its inner section
            fin.setClosedInnerSide(true);

            for(int isec=0; isec<fin.nSections(); isec++)
            {
                WingSection &sec = fin.section(isec);
                sec.setLeftFoilName(pFoilN0009->name());
                sec.setRightFoilName(pFoilN0009->name());
                sec.setNX(7); // prime numbers are perfect by nature
                sec.setXDistType(xfl::TANH);
            }

            WingSection &rootsection = fin.rootSection();
            rootsection.setChord(0.19);

            WingSection &tipsection = fin.tipSection();
            tipsection.setYPosition(0.17);
            tipsection.setChord(0.09);
        }

        // Assemble the plane and build the triangular mesh
        bool bThickSurfaces = false;
        bool bIgnoreFusePanels = false; // unused in the present case
        bool bMakeTriMesh = true;
        pPlaneXfl->makePlane(bThickSurfaces, bIgnoreFusePanels, bMakeTriMesh);
    }

    // Define an analysis
    PlanePolar *pPlPolar = new PlanePolar;
    {
        pPlPolar->setName("a T2 polar");
        // Store the pointer to ensure that the object is not lost
        // This should be done after the polar has been given a name
        // since objects are referenced by their name and are stored
        // in alphabetical order
        Objects3d::insertPPolar(pPlPolar);

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
            assert(pPlPolar->nFlapCtrls()==pPlaneXfl->nWings()); // because all the wings are flapped

            // get a reference to the main wing's flap controls
            AngleControl &mainwingctrls = pPlPolar->flapCtrls(0);
            {
                // get a reference to the main wing
                WingXfl &mainwing = *pPlaneXfl->wing(0);
                // sanity check: the number of flap deflections should be the same
                // as the main wing's number of flaps, i.e. 4
                assert(mainwingctrls.nValues()==mainwing.nFlaps());

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

            // the fin's flap is left to its default value = 0°
        }

        // leave the rest of the fields to their default values
    }


    // Run the analysis
    PlaneTask *pPlaneTask = new PlaneTask;
    {
        pPlaneTask->outputToStdIO(true);
        pPlaneTask->setKeepOpps(true);

        pPlaneTask->setObjects(pPlaneXfl, pPlPolar);
        pPlaneTask->setComputeDerivatives(false);

        // Create a vector of operating point parameters to calculate
        // Unlike in the foil case, the order of calculation is unimportant,
        // so there is no needed for ranges; an unordered list is what is needed
        std::vector<double> opplist;
        double oppmin = -5.0; // start at -5°
        double oppmax = 11.0; // +11°
        double inc = 4.0; // (°)
        double opp = oppmin;
        int i=1;
        do
        {
            opplist.push_back(opp);
            opp = oppmin + double(i++)*inc;
        }
        while(opp<oppmax);

        pPlaneTask->setOppList(opplist);


        // we are running the task in this thread, so there's
        // no stopping it once it's launched,
        pPlaneTask->run();

        // Results are automatically stored in the polar and
        // in the planeOpp array, so no action needed


        // print the results
        printf("Created %d plane operating points\n\n", int(pPlaneTask->planeOppList().size()));

        std::string separator = ", ";
        std::string exportstr = pPlPolar->exportToString(separator);
        printf(exportstr.c_str());
        printf("\n");

        // clean up
        delete pPlaneTask;
    }

    globals::saveFl5Project("/tmp/PlaneRun.fl5");


    // Must call! will delete the planes, foils and children objects
    // Memory leak otherwise
    globals::deleteObjects();

    std::cout << "done" << std::endl;

    return 0;
}






