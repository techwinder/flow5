
#include <iostream>
#include <format>


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

    Foil *pFoil = foil::makeNacaFoil(2410, nacaname); // "high level" function which also inserts the foil object in the database

    if(pFoil)
        std::cout <<"The foil "<< pFoil-> name() <<" has been created and added to the database" << std::endl<< std::endl;
    else
    {
        std::cout <<"Error creating the foil ...aborting" << std::endl;
        return 0;
    }

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
    XFoilTask task;

    std::cout << "Initializing XFoil task" << std::endl;

    task.initialize(pFoil, pPolar, bKeepOpps);
    task.appendRange({true, 0.0, 11.0, 1.0});
    task.appendRange({true, 0.0, -7.0, 1.0});

    std::cout << "Running XFoil task" << std::endl;
    task.run();
    printf(task.log().c_str());
    printf("\n");
    std::cout << "XFoil task done" << std::endl << std::endl;

    // Retrieve the results and insert them one by one in the database so that they are
    // stored in sorted order.
    // This ensures that they will be properly deleted and the memory released on exit.
    for(OpPoint *pOpp : task.operatingPoints())
    {
        Objects2d::insertOpPoint(pOpp); 
    }

    // print the content of the database if needed
/*    for(OpPoint const *pOpp : Objects2d::operatingPoints())
    {
        // filter on the foil and polar names (not necessary here)
        if(pOpp->foilName()==pFoil->name() && pOpp->polarName()==pPolar->name())
            printf("alpha=%5.2f, Cl=%9.5f, Cd=%9.5f\n", pOpp->m_Alpha, pOpp->m_Cl, pOpp->m_Cd);
    }*/

    // export the content of the polar

    //void exportPolar(std::string &outstring, std::string const &versionName, bool bDataOnly, bool bCSV) const
    std::string exportstr;
    pPolar->exportToString(exportstr, false, true);

    printf(exportstr.c_str());

    std::cout << "done" << std::endl;



    globals::deleteObjects(); // Must call! will delete the foil and the polar objects

    return 0;

}
