/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/

#define _MATH_DEFINES_DEFINED


#include <QString>

#include "api.h"

#include <fileio.h>
#include <foil.h>
#include <objects2d.h>
#include <objects2d.h>
#include <objects2d_globals.h>
#include <objects3d.h>
#include <planeopp.h>
#include <planexfl.h>
#include <polar.h>
#include <sailobjects.h>
#include <planepolar.h>
#include <xmlpolarreader.h>
#include <xfoiltask.h>


std::queue<std::string> globals::g_log;


void globals::clearLog() {while(!g_log.empty()) g_log.pop();}


void globals::pushToLog(std::string const &msg) {g_log.push(msg);}


std::string globals::poplog()
{
    if(!g_log.empty())
    {
        std::string firstmsg = g_log.front();
        g_log.pop();
        return firstmsg;
    }
    else
        return std::string();
}


bool globals::saveFl5Project(std::string const &pathname)
{
    QString PathName = QString::fromStdString(pathname);

    QFile fp(PathName);
    if (!fp.open(QIODevice::WriteOnly))
    {
        std::string msg = "Could not open the file: "+pathname+" for writing\n\n";
        globals::pushToLog(msg);

        return false;
    }

    FileIO saver;
    QDataStream ar(&fp);

    if(!saver.serializeProjectFl5(ar, true))
    {
        std::string msg = "Unknown error saving the project file " + pathname;        /** @todo send to log */
        globals::pushToLog(msg);
        return false;
    }

    fp.close(); // or let the destructor do it

    return true;
}


void globals::deleteObjects()
{
    Objects2d::deleteObjects();
    Objects3d::deleteObjects();
    SailObjects::deleteObjects();
}


Foil * foil::loadFoil(std::string const &pathname)
{
    Foil *pFoil = new Foil();

    std::stringstream ss;
    std::string log;

    int iLineError(0);
    if(objects::readFoilFile(pathname, pFoil, iLineError))
    {
        if(pFoil)
        {
            Objects2d::insertThisFoil(pFoil);
            log = "Successfully loaded " + pFoil->name() + "\n";
            globals::pushToLog(log);
        }
        else
        {
            ss << "Error reading the file at line" << iLineError << EOLstr;
            log = ss.str();
            globals::pushToLog(log);
            delete pFoil;
            pFoil = nullptr;
        }
    }
    else
    {
        delete pFoil;
        pFoil = nullptr;
    }

    return pFoil;
}


Foil *foil::makeNacaFoil(int digits, std::string const &name)
{
    Foil *pFoil = new Foil;
    if(!Objects2d::makeNacaFoil(pFoil, digits, 200))
    {
        delete pFoil;
        return nullptr;
    }

    pFoil->setName(name);

    Objects2d::insertThisFoil(pFoil);

    return pFoil;
}


Foil* foil::foil(const std::string &name)
{
    return Objects2d::foil(name);
}


Polar * foil::createAnalysis(std::string const &foilname)
{
    Foil const *pFoil = Objects2d::foil(foilname);
    if(!pFoil)
    {
        std::string msg = "The foil " + foilname + " does not exist";
        globals::pushToLog(msg);
        return nullptr;
    }

    Polar *pPolar = new Polar();
    Objects2d::insertPolar(pPolar);

    return pPolar;
}


PlaneXfl *plane::makeEmptyPlane()
{
    PlaneXfl *pPlaneXfl = new PlaneXfl;
    Objects3d::insertPlane(pPlaneXfl);
    return pPlaneXfl;
}


Polar *foil::importAnalysisFromXml(std::string const &pathname)
{
    Polar *pPolar = new Polar;

    QFile xmlFile(QString::fromStdString(pathname));

    XmlPolarReader polarReader(xmlFile, pPolar);
    polarReader.readXMLPolarFile();

    if(polarReader.hasError())
    {
        std::string errorMsg = polarReader.errorString().toStdString() +
                               QString::asprintf("\nline %d column %d", int(polarReader.lineNumber()), int(polarReader.columnNumber())).toStdString();
        globals::pushToLog(errorMsg);

        delete pPolar;
        return nullptr;
    }
    else
    {
        Foil *pFoil = Objects2d::foil(pPolar->foilName());
        if(!pFoil)
        {
            globals::pushToLog("No foil with name " + pPolar->foilName() + "to which the polar can be attached\n");
            delete pPolar;
            return nullptr;
        }
    }

    Objects2d::insertPolar(pPolar);
    return pPolar;
}






