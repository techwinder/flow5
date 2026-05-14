
#include <format>
#include <iostream>
#include <thread>

#include <api.h>
#include <constants.h>
#include <objects2d.h>
#include <polar.h>
#include <foil.h>
#include <oppoint.h>
#include <xfoiltask.h>
#include <objects2d_globals.h>
#include <polarnamemaker.h>
#include <flow5-io.h>

int main()
{
    printf("XFoil batch run\n");

    std::string nacaname = "theNaca2410";
    std::cout << "Making foil " << nacaname << std::endl;

    // Create
    Foil *pFoil2410 = new Foil;
    if(!Objects2d::makeNacaFoil(pFoil2410, 2410, 200))
    {
        if(pFoil2410) delete pFoil2410;
        return 0;
    }
    pFoil2410->setName("NACA 2410");
    pFoil2410->setLineColor({255,127,31});
    pFoil2410->setLineWidth(2);

    // Store
    Objects2d::insertThisFoil(pFoil2410);
    std::cout <<"The foil "<< pFoil2410-> name() <<" has been created and added to the database" << std::endl<< std::endl;

    std::vector<double> reynolds = {30.0e3, 40.0e3, 60.0e3, 80.0e3, 100.0e3, 130.0e3, 160.0e3, 200.0e3, 300.0e3, 500.0e3, 1000.0e3, 3000.0e3};
    std::vector<Polar*> polarlist;

    for (unsigned int i=0; i<reynolds.size(); i++)
    {
        Polar *pPolar = Objects2d::createPolar(pFoil2410, xfl::T1POLAR, reynolds.at(i), 0.0, 9.0, 1.0, 1.0);
        pPolar->setTheStyle(pFoil2410->theStyle());
        std::string name = PolarNameMaker::makeName(pPolar);
        pPolar->setName(name);
        Objects2d::insertPolar(pPolar);
        polarlist.push_back(pPolar);
    }

    bool bKeepOpps=false;

    std::vector<std::thread> threads;
    std::vector<XFoilTask*> tasks;

    for(unsigned int itask=0; itask<polarlist.size(); itask++)
    {
        XFoilTask *pTask = new XFoilTask;
        tasks.push_back(pTask); // keep track of the pointers so that the tasks can be deleted later on
        pTask->initialize(*pFoil2410, polarlist.at(itask), bKeepOpps);
        pTask->appendRange({true, 0.0, 11.0, 1.0});
        pTask->appendRange({true, 0.0, -7.0, 1.0});
        std::cout << std::format("Starting XFoil task with Reynolds={:17.0f}", polarlist.at(itask)->Reynolds()) << std::endl;
        threads.push_back(std::thread(&XFoilTask::run, pTask));
    }

    for(unsigned int itask=0; itask<polarlist.size(); itask++)
    {
        threads[itask].join();
        std::cout << std::format("... joined task {:d}", itask) <<std::endl;
        delete tasks[itask];
    }

    std::cout << "XFoil tasks done" << std::endl << std::endl;

    // save the project; requires link to flow5-io-lib
    std::string logmsg;
    std::string projectfilepath = "/tmp/XFoilBatchRun.fl5";
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
