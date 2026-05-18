/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois

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


#pragma once


#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

#include <gmsh.h>


#include <fl5lib_global.h>
#include <gmshparams.h>

class Vector3d;
class Node;
class Triangle3d;
class WingXfl;
class Fuse;
class FuseNurbs;
class FuseFlatFaces;
class SailOcc;


namespace gmesh
{
    FL5LIB_EXPORT void listMainOptions(std::string &list);
    std::string getNumberOption(std::string name);
    std::string getStringOption(std::string name);

//    void listModelEntities(std::string &list);
    void listModel(std::string &list);

    void convertFromGmsh(std::vector<Triangle3d> &triangles, std::string &log);
    void convertTriangles(std::vector<std::size_t>const&elementTags, std::vector<Vector3d> const &node, std::vector<Triangle3d> &m_Triangles, std::string &log);
    void makeModelCurves(std::vector<std::vector<Vector3d> > &curves);
    void makeModelVertices(std::vector<Vector3d>&vertices);
    bool getVertex(int tag, Vector3d &vertex);
    bool getLine(int tag, Vector3d &v0, Vector3d &v1);

    bool importBRepList(std::vector<std::string> const &breps, std::string &brep);

    bool gmshtoBRep(std::string &brep);
    FL5LIB_EXPORT bool BReptoGmsh(const std::string &brep);
    FL5LIB_EXPORT bool BRepstoGmsh(std::vector<std::string> const &brep);

    void bRepToStepFile(std::string const &brep, const std::string &pathname);

    void getBoundingBox(Vector3d &botleft, Vector3d &topright);
    double wettedArea();


    bool wingToBRep(WingXfl const*pWing, std::string &brep, std::string &log);
    bool fuseNurbsToBRep(FuseNurbs const*pFuse, std::string &brep, std::string &log);
    bool fuseQuadsToBRep(FuseFlatFaces const*pFuse, std::string &brep, std::string &log);

    bool rotateBrep(const std::string &brep, Vector3d const &O, Vector3d const &axis, double theta, std::string &rotated);
    bool scaleBrep(std::string const&brep, Vector3d const &O, double sx, double sy, double sz, std::string &scaled);
    bool translateBrep(std::string const&brep, Vector3d const &T, std::string &translated);

    bool intersectBrep(const std::string &brep, const std::vector<Node> &A, const std::vector<Node> &B, std::vector<Vector3d> &I, std::vector<bool> &bIntersect);

    void tessellateBRep(const std::string &BRep, GmshParams const &params, std::vector<Triangle3d> &triangles, std::string &log);
    void tessellateShape(TopoDS_Shape const&Shape, GmshParams const &params, std::vector<Triangle3d> &triangles, std::string &log);
    void tessellateFace(TopoDS_Face const&Face, GmshParams const &params, std::vector<Triangle3d> &triangles, std::string &log);

    FL5LIB_EXPORT void makeSailOccTriangulation(SailOcc *pSailOcc);
    FL5LIB_EXPORT int makeFuseTriangulation(Fuse *pFuse, std::string &logmsg, const std::string &prefix="");

    std::string tempFile();


    //_________________ imported from GMesher ___________

    void setGmshParams(double emin, double emax, int iCurvature, gmesh::enumGmshAlgo algorithm);
    FL5LIB_EXPORT void setAlgorithm(enumGmshAlgo algorithm);

    void meshFuseShellsThinSurfaces( Fuse *pFuse, std::vector<WingXfl> const &wings);
    void meshFuseShellsThickSurfaces(Fuse *pFuse, std::vector<WingXfl> const &wings);

    void convertTrianglesAndNodesFromGmsh(std::vector<Triangle3d> &triangles, std::vector<Node> &nodes, std::string &log, std::string prefix);
}

