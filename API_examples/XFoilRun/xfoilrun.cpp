
#include <iostream>


#include <api.h>
#include <constants.h>
#include <flow5-io.h>
#include <foil.h>
#include <objects2d.h>
#include <objects2d_globals.h>
#include <oppoint.h>
#include <polar.h>
#include <xfoiltask.h>

int main()
{
    printf("flow5 XFoil run\n");

    std::string nacaname = "theNaca2410";
    std::cout << "Making foil " << nacaname << std::endl;

    // makeNacaFoil has been deprecated in v7.55;
    //    Foil *pFoil2410 = foil::makeNacaFoil(2410, nacaname); // "high level" function which also inserts the foil object in the database

    // Using seperate methods for creating and storing
    // Create
    Foil *pFoil2410 = new Foil;
    if(!Objects2d::makeNacaFoil(pFoil2410, 2410, 200))
    {
        if(pFoil2410) delete pFoil2410;
        return 0;
    }
    pFoil2410->setName("NACA 2410");

    // Store
    Objects2d::insertThisFoil(pFoil2410);
    std::cout <<"The foil "<< pFoil2410-> name() <<" has been created and added to the database" << std::endl<< std::endl;

    //    std::string coords = pFoil2410->listCoords();
    //    std::cout << coords << std::endl;

/*
    // Read a foil from a file
    Foil *pFoilClarkY = new Foil;
    std::string pathname = "/path/to/CLARK Y.dat";
    int iLineError(-1);
    bool bOK = objects::readFoilFile(pathname, pFoilClarkY, iLineError);

    if(bOK)
    {
        pFoilClarkY->setLineWidth(2);
        pFoilClarkY->setLineColor({255, 201, 51});
        Objects2d::insertThisFoil(pFoilClarkY);

        std::cout <<"The foil "<< pFoilClarkY-> name() <<" has been created and added to the database" << std::endl<< std::endl;
    }
    else
    {
        delete pFoilClarkY;
        std::cerr <<  "Error reading the file " << pathname << " at line " << iLineError << std::endl;
    }*/


    std::cout << "Foil properties:" << std::endl;
    std::cout << pFoil2410->name() << std::endl;
    std::string props = pFoil2410->properties(true);
    std::cout << props << std::endl << std::endl;

    Polar *pPolar = Objects2d::createPolar(pFoil2410, xfl::T1POLAR, 100000.0, 0.0, 9.0, 1.0, 1.0);
    pPolar->setName("T1 test polar");
    Objects2d::insertPolar(pPolar); // so that it doesn't get lost and will be deleted on exit

    std::cout << "polar properties:" << std::endl;
    std::cout << pPolar->name() << std::endl;
    std::cout << pPolar->properties() << std::endl << std::endl;

    bool bKeepOpps=true; // otherwise will still store the results in the polar but will discard (and delete) the operating point objects


    XFoilTask *pTask = new XFoilTask;

    std::cout << "Initializing XFoil task" << std::endl;

    pTask->initialize(*pFoil2410, pPolar, bKeepOpps);
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
     {        *
     // filter on the foil and polar names (not necessary here)
     if(pOpp->foilName()==pFoil2410->name() && pOpp->polarName()==pPolar->name())
         printf("alpha=%5.2f, Cl=%9.5f, Cd=%9.5f\n", pOpp->m_Alpha, pOpp->m_Cl, pOpp->m_Cd);
}*/

    // export the content of the polar

    std::string exportstr;
    pPolar->exportToString(exportstr, false, true);

    printf(exportstr.c_str());

    // save the project; requires link to flow5-io-lib
    logmsg.clear();
    std::string projectfilepath;
    projectfilepath  = std::filesystem::temp_directory_path().string();
    projectfilepath += std::filesystem::path::preferred_separator;
    projectfilepath += "XFoilRun.fl5";

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

    // Must call! will delete the foil and the polar objects
    // Memory leak otherwise
    globals::deleteObjects();

    std::cout << "done" << std::endl;

    return 0;

}
