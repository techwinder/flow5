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

#include <fstream>
#include <string>


#include <objects2d_globals.h>

#include <constants.h>
#include <foil.h>
#include <polar.h>
#include <utils.h>



bool foil::readFoilFile(const std::string &filename, Foil *pFoil, int &iLineError)
{
    std::string line;
    std::string FoilName;

    std::vector<Node2d> basenodes;

    std::ifstream file(filename);


    int iLine(0);

    if(!file.is_open())
    {
        iLineError = 0;
        return false;
    }

    FoilName = filename;

    float val[] {0,0,0};

    // identify and read the first non-empty line
    while (std::getline(file, line))
    {
        iLine++;
        xfl::trim(line);

        if (line.length()==0) continue;

        if(xfl::readValues(line, val, 2)==2)
        {
            //there isn't a name on the first line, use the file's name
            FoilName = filename;
            // store initial coordinates
            basenodes.push_back({val[0], val[1]});
        }
        else FoilName = line;

        break;
    }

    // read coordinates
    while (std::getline(file, line))
    {
        iLine++;
        xfl::trim(line);
        if(line.length()==0) continue;

        if(xfl::readValues(line, val, 2)==2)
        {
            basenodes.push_back({val[0], val[1]});
        }
        else
        {
            // Non-empty but unreadable line, skip
            // Drela's airfoil files end with a string comment
            iLineError = iLine;
//            return false;
        }
    }

    pFoil->setName(FoilName);

    // Check if the foil was written clockwise or counter-clockwise
    int ip = 0;
    double area = 0.0;
    for (int i=0; i<pFoil->nBaseNodes(); i++)
    {
        if(i==pFoil->nBaseNodes()-1) ip = 0;
        else                         ip = i+1;
        area +=  0.5*(pFoil->yb(i)+pFoil->yb(ip))*(pFoil->xb(i)-pFoil->xb(ip));
    }

    if(area < 0.0)
    {
        //reverse the points order
        double xtmp(0), ytmp(0);
        for (int i=0; i<pFoil->nBaseNodes()/2; i++)
        {
            xtmp         = pFoil->xb(i);
            ytmp         = pFoil->yb(i);
            basenodes[i].x = pFoil->xb(pFoil->nBaseNodes()-i-1);
            basenodes[i].y = pFoil->yb(pFoil->nBaseNodes()-i-1);
            basenodes[pFoil->nBaseNodes()-i-1].x = xtmp;
            basenodes[pFoil->nBaseNodes()-i-1].y = ytmp;
        }
    }

    pFoil->setBaseNodes(basenodes);
    pFoil->initGeometry();

    return true;
}


void foil::deRotate(Foil *pFoil)
{
    pFoil->deRotate();
    pFoil->initGeometry();
}


void foil::normalize(Foil *pFoil)
{
    pFoil->normalizeGeometry();
    pFoil->initGeometry();
}


void foil::scaleFoil(Foil *pFoil, double camber, double xCamber, double thickness, double xThickness)
{
    xCamber    = pFoil->baseCbLine().front().x + (pFoil->baseCbLine().back().x-pFoil->baseCbLine().front().x) * xCamber;
    xThickness = pFoil->baseCbLine().front().x + (pFoil->baseCbLine().back().x-pFoil->baseCbLine().front().x) * xThickness;

    pFoil->setThickness(xThickness, thickness);

    pFoil->setCamber(xCamber, camber);

    pFoil->makeBaseFromCamberAndThickness();
    pFoil->rebuildPointSequenceFromBase();
    pFoil->applyBase();
}


void foil::interpolateFoils(Foil*pFoil, Foil* const pFoil1, Foil *const pFoil2, double frac)
{
    if(pFoil1->nNodes()>pFoil2->nNodes()) pFoil->copy(pFoil1, false);
    else                                  pFoil->copy(pFoil2, false);

    pFoil->interpolate(pFoil1, pFoil2, frac);
    pFoil->makeBaseFromCamberAndThickness();
    pFoil->rebuildPointSequenceFromBase();
    pFoil->applyBase();
}


void foil::setTEGap(Foil *pFoil, double targetgap, double blendinglength)
{

    double dg = (targetgap - pFoil->TEGap());
    double length = pFoil->length();

    CubicSpline const &CS = pFoil->cubicSpline();

    for(int i=0; i<CS.ctrlPointCount(); i++)
    {
        double arg = pFoil->TE().x - CS.controlPoint(i).x;
        //decay exponentially
        double dth = exp(-arg/(1.0-blendinglength)*length);
        double u = double(i) / double(CS.ctrlPointCount()-1);
        if(u<pFoil->CSfracLE())
        {
            // top surface
            pFoil->setBaseNode(i, pFoil->xb(i), pFoil->yb(i) + dth * dg/2.0);
        }
        else
        {
            // bot surface
            pFoil->setBaseNode(i, pFoil->xb(i), pFoil->yb(i) - dth * dg/2.0);
        }
    }

    pFoil->initGeometry();
}



