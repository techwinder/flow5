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



#include <string>
#include <format>


#include "api.h"

#include <foil.h>
#include <gmesh_globals.h>
#include <objects2d.h>
#include <objects2d.h>
#include <objects2d_globals.h>
#include <objects3d.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planexfl.h>
#include <polar.h>
#include <sailobjects.h>
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


void globals::deleteObjects()
{
    Objects2d::deleteObjects();
    Objects3d::deleteObjects();
    SailObjects::deleteObjects();
}


/** @deprecated Deprecated in v7.56 since the method does two different things:
 *  1. load the foil
 *  2. store it in the database
 *  Use preferably the code snippet below directly in the source code.
 */
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


/** @deprecated Deprecated in v7.55 since the method does two different things:
 *  1. create the foil
 *  2. store it in the database
 *  Use preferably the code snippet below directly in the source code.
 */
Foil *foil::makeNacaFoil(int digits, std::string const &name)
{
    Foil *pFoil = new Foil;
    if(!Objects2d::makeNacaFoil(pFoil, digits, 200))
    {
        std::cerr << "error making NACA foil" << std::endl;
        delete pFoil;
        return nullptr;
    }

    pFoil->setName(name);

    Objects2d::insertThisFoil(pFoil);

    return pFoil;
}


/** @deprecated Deprecated in v7.56: No benefit
 *  Use preferably the code snippet below directly in the source code.
 */
Foil* foil::foil(const std::string &name)
{
    return Objects2d::foil(name);
}


/** @deprecated Deprecated in v7.56: No benefit
 *  Use preferably the code snippet below directly in the source code.
 */
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

/** @deprecated Deprecated in v7.56: No benefit
 *  Use preferably the code snippet below directly in the source code.
 */
PlaneXfl *plane::makeEmptyPlane()
{
    PlaneXfl *pPlaneXfl = new PlaneXfl;
    Objects3d::insertPlane(pPlaneXfl);
    return pPlaneXfl;
}




/** @todo duplicate with GMesherWt::onHandleMeshResults */
void plane::meshFuse(PlaneXfl *pPlaneXfl, std::vector<int>selectedWings, bool bThickSurfaces, std::string &log)
{
    if(!pPlaneXfl)
    {
        log += "PlaneXfl is a null object\n";
        return;
    }

    Fuse *pFuse = pPlaneXfl->fuse(0);

    if(!pFuse)
    {
        log += "No fuselage to mesh\n";
        return;
    }

    std::vector<WingXfl> winglist;

    for(unsigned int iw=0; iw<selectedWings.size(); iw++)
    {
        int index = selectedWings.at(iw);
        if(index>=0 && index<pPlaneXfl->nWings())
        {
            WingXfl const *pWing = pPlaneXfl->wing(index);

            WingXfl wing(*pWing);
            wing.createSurfaces(wing.position(), wing.rx(), wing.ry());

            for (int j=0; j<wing.nSurfaces(); j++)
                wing.surface(j).makeSideNodes(nullptr);

            winglist.push_back(wing);
        }
    }


    if(bThickSurfaces)
        gmesh::meshFuseShellsThickSurfaces(pFuse, winglist);
    else
        gmesh::meshFuseShellsThinSurfaces(pFuse, winglist);

    try
    {
        gmsh::model::mesh::generate(2);
    }
    catch(...)
    {
        log += "Gmsh error... aborting\n";
        return;
    }


    log += "Handling mesh results...\n";

    std::string error;
    gmsh::logger::getLastError(error);

    if(error.length())
    {
        log += "---ERROR---\n" + error +EOLstr+EOLstr;
    }

    gmsh::logger::stop();

    std::vector<Triangle3d> triangles;
    std::vector<Node> nodes;

    gmesh::convertTrianglesAndNodesFromGmsh(triangles, nodes, log);


    std::string strange;


    double nT3d(0), nNodes(0);
    gmsh::option::getNumber("Mesh.NbTriangles", nT3d);
    gmsh::option::getNumber("Mesh.NbNodes", nNodes);
    strange = std::format("Gmsh count: {:.0f} triangles and {:.0f} nodes\n", nT3d, nNodes);
    log += strange;

    bool m_bMakeXZSymmetric = true;
    if(m_bMakeXZSymmetric)
    {
        int nt = int(triangles.size());
        strange = std::format("   Right side triangle count = {:d}\n", nt);
        log += "\n" + strange + "   Making symmetric left side panels\n";
        for(int it=0; it<nt; it++)
        {
            Triangle3d t3 = triangles.at(it);
            t3.makeXZsymmetric();
            triangles.push_back(t3);
        }
        strange = std::format("   Total triangle count = {:d}\n", int(triangles.size()));
        log += strange;
    }


    std::string str;
    str = "   Making mesh from triangles\n";
    pFuse->triMesh().makeMeshFromTriangles(triangles, 0, xfl::FUSESURFACE, str, "      ");
    log += str;

    strange  = std::format("\nTriangle count = {:d}\n", pFuse->nPanel3());
    strange += std::format("Node count     = {:d}\n", int(pFuse->nodes().size()));
    strange += "\n_______\n\n";
    log += strange;

    pPlaneXfl->makeTriMesh(bThickSurfaces);
}







