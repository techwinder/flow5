
#include <iostream>


#include <api.h>
#include <constants.h>
#include <objects2d.h>
#include <polar.h>
#include <foil.h>
#include <oppoint.h>
#include <xfoiltask.h>

int main()
{
    printf("flow5 XFoil run\n");

    std::string nacaname = "theNaca2410";
    std::cout << "Making foil " << nacaname << std::endl;

    // makeNacaFoil has been deprecated in v7.55;
//    Foil *pFoil = foil::makeNacaFoil(2410, nacaname); // "high level" function which also inserts the foil object in the database

    // Using seperate methods for creating and storing
    // Create
    Foil *pFoil = new Foil;
    if(!Objects2d::makeNacaFoil(pFoil, 2410, 200))
    {
        if(pFoil) delete pFoil;
        return 0;
    }
    std::cout <<"The foil "<< pFoil-> name() <<" has been created and added to the database" << std::endl<< std::endl;
    pFoil->setName("NACA 2410");

    // Store
    Objects2d::insertThisFoil(pFoil);

//    std::string coords = pFoil->listCoords();
//    std::cout << coords << std::endl;

    std::cout << "Foil properties:" << std::endl;
    std::cout << pFoil->name() << std::endl;
    std::string props = pFoil->properties(true);
    std::cout << props << std::endl << std::endl;

    Polar *pPolar = Objects2d::createPolar(pFoil, xfl::T1POLAR, 100000.0, 0.0, 9.0, 1.0, 1.0); // "low level" function
    pPolar->setName("T1 test polar");
    Objects2d::insertPolar(pPolar); // so that it won't get lost and will be neatly deleted on exit

    std::cout << "polar properties:" << std::endl;
    std::cout << pPolar->name() << std::endl;
    std::cout << pPolar->properties() << std::endl << std::endl;

    bool bKeepOpps=true; // otherwise will still store the results in the polar but will discard (and delete) the operating point objects


    XFoilTask *pTask = new XFoilTask;

    std::cout << "Initializing XFoil task" << std::endl;

    pTask->initialize(*pFoil, pPolar, bKeepOpps);
    pTask->appendRange({true, 0.0, 11.0, 1.0});
    pTask->appendRange({true, 0.0, -7.0, 1.0});

    std::cout << "Running XFoil task" << std::endl;
    pTask->run();
    printf(pTask->log().c_str());
    printf("\n");
    std::cout << "XFoil task done" << std::endl << std::endl;

    // Retrieve the results and insert them one by one in the database so that they are
    // stored in sorted order.
    // This ensures that they will be properly deleted and the memory released on exit.
    for(OpPoint *pOpp : pTask->operatingPoints())
    {
        Objects2d::insertOpPoint(pOpp); 
    }

    delete pTask;

    // print the content of the database if needed
/*    for(OpPoint const *pOpp : Objects2d::operatingPoints())
    {
        // filter on the foil and polar names (not necessary here)
        if(pOpp->foilName()==pFoil->name() && pOpp->polarName()==pPolar->name())
            printf("alpha=%5.2f, Cl=%9.5f, Cd=%9.5f\n", pOpp->m_Alpha, pOpp->m_Cl, pOpp->m_Cd);
    }*/

    // export the content of the polar

    std::string exportstr;
    pPolar->exportToString(exportstr, false, true);

    printf(exportstr.c_str());


    globals::saveFl5Project("/tmp/XFoilRun.fl5");

    globals::deleteObjects(); // Must call! will delete the foil and the polar objects

    std::cout << "done" << std::endl;

    return 0;

}
