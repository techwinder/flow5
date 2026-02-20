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

#include <QString>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

#ifndef NO_GMSH
#include <gmsh.h>
#else
#include <vector>
#include <string>
#include <utility>

namespace gmsh {
    typedef std::vector<std::pair<int, int>> vectorpair;

    inline void clear() {}
    inline void write(const std::string&) {}
    inline void merge(const std::string&) {}

    namespace option {
        inline void getNumber(const std::string&, double&) {}
        inline void setNumber(const std::string&, double) {}
        inline void getString(const std::string&, std::string&) {}
        inline void setString(const std::string&, const std::string&) {}
    }
    namespace logger {
        inline void get(std::vector<std::string>&) {}
        inline void start() {}
        inline void stop() {}
        inline void getLastError(std::string&) {}
    }
    namespace model {
        inline void add(const std::string&) {}
        inline void getEntities(vectorpair&, int /*dim*/ = -1) {}
        inline void getEntityType(int, int, std::string&) {}
        inline void getType(int, int, std::string&) {}
        inline void getParametrizationBounds(int, int, std::vector<double>&, std::vector<double>&) {}
        inline void getValue(int, int, const std::vector<double>&, std::vector<double>&) {}

        namespace occ {
            inline void synchronize() {}
            inline void getBoundingBox(int, int, double&, double&, double&, double&, double&, double&) {}
            inline void rotate(const vectorpair&, double, double, double, double, double, double, double) {}
            inline void dilate(const vectorpair&, double, double, double, double, double, double) {}
            inline void translate(const vectorpair&, double, double, double) {}
            inline int addPoint(double, double, double, double =0, int = -1) { return 0; }
            inline int addLine(int, int, int = -1) { return 0; }
            inline int addCurveLoop(const std::vector<int>&, int = -1) { return 0; }
            inline int addPlaneSurface(const std::vector<int>&, int = -1) { return 0; }
            inline void addThruSections(const std::vector<int>&, vectorpair&, int = -1, bool = true, bool = false) {}
            inline void remove(const vectorpair&, bool = false) {}
            inline int addBSplineSurface(const std::vector<int>&, int = -1, int = -1) { return 0; }
            inline void copy(const vectorpair&, vectorpair&) {}
            inline void mirror(const vectorpair&, double, double, double, double) {}
            inline void fragment(const vectorpair&, const vectorpair&, vectorpair&, std::vector<vectorpair>&, int = -1, bool = true, bool = true) {}
            inline void cut(const vectorpair&, const vectorpair&, vectorpair&, std::vector<vectorpair>&, int = -1, bool = true, bool = true) {}
            inline void getEntities(vectorpair&, int = -1) {}
            inline void getMass(int, int, double&) {}
            inline int addSurfaceFilling(int, int = -1, const std::vector<int>& = std::vector<int>()) { return 0; }
            inline void addSurfaceLoop(const std::vector<int>&, int = -1) {}
            inline void importShapesNativePointer(const void*, vectorpair&, bool = true) {}
            inline int addSpline(const std::vector<int>&, int = -1) { return 0; }
            inline void healShapes(vectorpair&, double = 1e-8, bool = false, bool = false, bool = false, bool = false, bool = false) {}
            inline void removeAllDuplicates() {}
            inline int addWire(const std::vector<int>&, int = -1, bool = false) { return 0; }
        }
        namespace mesh {
            inline void generate(int) {}
            inline void clear() {}
            inline void getNodes(std::vector<std::size_t>&, std::vector<double>&, std::vector<double>&, int /*dim*/=-1, int /*tag*/=-1, bool=false, bool=true) {}
            inline void getElements(std::vector<int>&, std::vector<std::vector<std::size_t>>&, std::vector<std::vector<std::size_t>>&, int /*dim*/=-1, int /*tag*/=-1) {}
            inline void getElementProperties(int, std::string&, int&, int&, int&, std::vector<double>&, int&) {}
            inline void getElementsByType(int, std::vector<std::size_t>&, std::vector<std::size_t>&, int /*tag*/=-1) {}
            inline void getElement(std::size_t, int&, std::vector<std::size_t>&, int&, int&) {}
        }
    }
}
#endif

#include <api/fl5lib_global.h>


class Vector3d;
class Node;
class Triangle3d;
class WingXfl;
class Fuse;
class FuseNurbs;
class FuseFlatFaces;
class SailOcc;

struct GmshParams;

namespace gmesh
{
    void listMainOptions(std::string &list);
    std::string getNumberOption(std::string name);
    std::string getStringOption(std::string name);

//    void listModelEntities(QString &list);
    void listModel(QString &list);

    void convertFromGmsh(std::vector<Triangle3d> &triangles, QString &log);
    void convertTriangles(std::vector<std::size_t>const&elementTags, std::vector<Vector3d> const &node, std::vector<Triangle3d> &m_Triangles, QString &log);
    void makeModelCurves(std::vector<std::vector<Vector3d> > &curves);
    void makeModelVertices(std::vector<Vector3d>&vertices);
    bool getVertex(int tag, Vector3d &vertex);
    bool getLine(int tag, Vector3d &v0, Vector3d &v1);

    bool importBRepList(std::vector<std::string> const &breps, std::string &brep);

    bool gmshtoBRep(std::string &brep);
    bool BReptoGmsh(const std::string &brep);
    bool BRepstoGmsh(std::vector<std::string> const &brep);

    void bRepToStepFile(std::string const &brep, const std::string &pathname);

    void getBoundingBox(Vector3d &botleft, Vector3d &topright);
    double wettedArea();


    bool wingToBRep(WingXfl const*pWing, std::string &brep, QString &log);
    bool fuseNurbsToBRep(FuseNurbs const*pFuse, std::string &brep, std::string &log);
    bool fuseQuadsToBRep(FuseFlatFaces const*pFuse, std::string &brep, std::string &log);

    bool rotateBrep(const std::string &brep, Vector3d const &O, Vector3d const &axis, double theta, std::string &rotated);
    bool scaleBrep(std::string const&brep, Vector3d const &O, double sx, double sy, double sz, std::string &scaled);
    bool translateBrep(std::string const&brep, Vector3d const &T, std::string &translated);

    bool intersectBrep(const std::string &brep, const std::vector<Node> &A, const std::vector<Node> &B, std::vector<Vector3d> &I, std::vector<bool> &bIntersect);

    void tessellateBRep(const std::string &BRep, GmshParams const &params, std::vector<Triangle3d> &triangles, QString &log);
    void tessellateShape(TopoDS_Shape const&Shape, GmshParams const &params, std::vector<Triangle3d> &triangles, QString &log);
    void tessellateFace(TopoDS_Face const&Face, GmshParams const &params, std::vector<Triangle3d> &triangles, QString &log);

    void makeSailOccTriangulation(SailOcc *pSailOcc);
    int makeFuseTriangulation(Fuse *pFuse, QString &logmsg, const QString &prefix="");

    std::string tempFile();
}

