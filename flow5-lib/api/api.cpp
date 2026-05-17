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
#include <gmesh_globals.h>

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


/** @todo partial duplicate with GMesherWt::onHandleMeshResults */
void plane::meshFuse(PlaneXfl *pPlaneXfl, std::vector<int>selectedWings, bool bThickSurfaces, gmesh::enumGmshAlgo algorithm, std::string &log, std::string prefix)
{
    if(!pPlaneXfl)
    {
        log += prefix + "PlaneXfl is a null object\n";
        return;
    }

    Fuse *pFuse = pPlaneXfl->fuse(0);

    if(!pFuse)
    {
        log += prefix + "No fuselage to mesh\n";
        return;
    }

    gmesh::setGmshParams(pFuse->gmshMinSize(), pFuse->gmshMaxSize(), pFuse->gmshNCurvature(), algorithm);

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
        log += prefix + "Gmsh error... aborting\n";
        return;
    }


    log += prefix + "Handling mesh results...\n";

    std::string error;
    gmsh::logger::getLastError(error);

    if(error.length())
    {
        log += prefix + "---ERROR---\n" + error +EOLstr+EOLstr;
    }

    gmsh::logger::stop();

    std::vector<Triangle3d> triangles;
    std::vector<Node> nodes;

    gmesh::convertTrianglesAndNodesFromGmsh(triangles, nodes, log, prefix);


    std::string strange;


    double nT3d(0), nNodes(0);
    gmsh::option::getNumber("Mesh.NbTriangles", nT3d);
    gmsh::option::getNumber("Mesh.NbNodes", nNodes);
    strange = std::format("Gmsh count: {:.0f} triangles and {:.0f} nodes\n", nT3d, nNodes);
    log += prefix + strange;

    // In the case of an xfl-type fuse, the shells cover only one side,
    // This is to ensure that we build a symmetric mesh and obtain symmetrical calculation results
    bool bMakeXZSymmetric = pFuse->isXflType();
    if(bMakeXZSymmetric)
    {
        int nt = int(triangles.size());
        strange = std::format("Right side triangle count = {:d}\n", nt);
        log += prefix + strange;
        log += prefix +"Making symmetric left side panels\n";
        for(int it=0; it<nt; it++)
        {
            Triangle3d t3 = triangles.at(it);
            t3.makeXZsymmetric();
            triangles.push_back(t3);
        }
        strange = std::format("Total triangle count = {:d}\n", int(triangles.size()));
        log += prefix + strange;
    }


    std::string str;
    str = prefix + "Making mesh from triangles\n";
    pFuse->triMesh().makeMeshFromTriangles(triangles, 0, xfl::FUSESURFACE, str, prefix + "  ");
    log += str + EOLstr;

    strange  = prefix + std::format("Triangle count = {:d}\n", pFuse->nPanel3());
    strange += prefix + std::format("Node count     = {:d}\n", int(pFuse->nodes().size()));
    strange += prefix + "\n_______\n\n";
    log += strange;

    pPlaneXfl->makeTriMesh(bThickSurfaces);
}







