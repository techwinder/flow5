/****************************************************************************

    flow5
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
#include <TopoDS_Shell.hxx>
#include <TopoDS_ListOfShape.hxx>

#include <xflcore/enums_objects.h>
#include <xflgeom/geom3d/triangulation.h>
#include <xflobjects/objects3d/part.h>
#include <xflocc/occmeshparams.h>

#include <xflpanels/mesh/trimesh.h>


class Panel3;
class Panel4;
class WPolar;


class Fuse : public Part
{
    public:

        enum enumType {FlatFace, NURBS, Sections, Occ, Stl, Other};


    public:
        Fuse();
        Fuse(const Fuse &fuse);

        void computeAero(QVector<Panel4> const &panel4, double const *Cp,     const WPolar *pWPolar, double Alpha, Vector3d &CP, Vector3d &Force, Vector3d &Moment) const;
        void computeAero(QVector<Panel3> const &panel3, double const *Cp3Vtx, const WPolar *pWPolar, double Alpha, Vector3d &CP, Vector3d &Force, Vector3d &Moment) const;
        void computeViscousForces(const WPolar *pWPolar, double Alpha, double QInf, Vector3d &Force, Vector3d &Moment) const;

        virtual Fuse* clone() const = 0;
        virtual void duplicateFuse(const Fuse &aFuse);

        virtual void makeFuseGeometry();
        virtual bool intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I) const;
        virtual bool intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I, bool bRightSide) const;
        bool intersectFuseShape(TopoDS_Shape &shape, const Segment3d &seg, Vector3d *I, bool *bIntersect, bool bRightSide) const;

        bool intersectFuseTriangulation(const Vector3d &A, const Vector3d &B, Vector3d &I) const;

        virtual void scale(double XFactor, double YFactor, double ZFactor);
        virtual void translate(Vector3d const &T);
        virtual void rotate(const Vector3d &origin, const Vector3d &axis, double theta);

        /** @todo unnecessary just test m_FuseType here */
        virtual bool isXflType()       const = 0;
        virtual bool isFlatFaceType()  const = 0;
        virtual bool isSectionType()   const = 0;
        virtual bool isSplineType()    const = 0;
        virtual bool isOccType()       const = 0;
        virtual bool isStlType()       const = 0;

        Fuse::enumType fuseType() const {return m_FuseType;}
        void setFuseType(Fuse::enumType type){m_FuseType=type;}

        TopoDS_ListOfShape &shapes() {return m_Shape;}
        TopoDS_ListOfShape const &shapes() const {return m_Shape;}
        TopoDS_ListOfShape const &shells() const {return m_Shell;}

        // Methods related to the Uncut shape
        void appendShape(TopoDS_Shape const &shape) {m_Shape.Append(shape);}
        int shapeCount() const {return m_Shape.Extent();}
        void clearShapes() {m_Shape.Clear();}

        //	Methods related to the cut shells
        void makeShellsFromShapes();
        void clearShells() {m_Shell.Clear();}
        int shellCount() const {return m_Shell.Extent();}
        void appendShell(TopoDS_Shape const &shell) {m_Shell.Append(shell);}
        bool stitchShells(TopoDS_Shell &fusedshell, QString &logmsg, QString prefix);
        virtual int makeShellTriangulation(QString &logmsg, QString const &prefix);
        void clearOccTriangulation();

        // Methods related to geometry
        double wettedArea() const {return m_WettedArea;}
        double formFactor() const;
        double maxWidth() const  {return m_MaxWidth;}
        double maxHeight() const {return m_MaxHeight;}
        double maxFrameArea() const {return m_MaxFrameArea;}

        void translateTriPanels(Vector3d const &T) {translateTriPanels(T.x, T.y, T.z);}
        void translateTriPanels(double tx, double ty, double tz);

        // Methods related to meshing
        virtual int makeDefaultTriMesh(QString &logmsg, const QString &prefix);

        int nPanel4() const override = 0;
        int nPanel3() const override {return m_TriMesh.panelCount();}

        int panel3NodeCount() const {return m_TriMesh.nodeCount();}
        Panel3 &panel3(int index) {return m_TriMesh.panel(index);}
        Panel3 const &panel3At(int index) const {return m_TriMesh.panelAt(index);}

        Node const & panel3Node(int index)  {return m_TriMesh.node(index);}
        QVector<Node> &nodes() {return m_TriMesh.nodes();}

        TriMesh const &triMesh() const {return m_TriMesh;}
        TriMesh &triMesh() {return m_TriMesh;}
        void setTriMesh(TriMesh const &trimesh) {m_TriMesh=trimesh;}

        Triangulation const &triangulation() const {return m_Triangulation;}
        Triangulation &triangulation() {return m_Triangulation;}

        void saveBaseTriangulation() {m_BaseTriangulation = m_Triangulation;}

        void setBaseTriangles(QVector<Triangle3d> const &trianglelist) {m_BaseTriangulation.setTriangles(trianglelist);}
        void setModTriangles(QVector<Triangle3d> const &trianglelist) {m_Triangulation.setTriangles(trianglelist);}

        void clearTriangles() {m_Triangulation.clear();}
        void appendTriangle(Triangle3d const & t3) {m_Triangulation.appendTriangle(t3);}
        int triangleCount() const {return m_Triangulation.nTriangles();}
        Triangle3d const & triangleAt(int index) const {return m_Triangulation.triangleAt(index);}
        Triangle3d & triangle(int index) {return m_Triangulation.triangle(index);}
        QVector<Triangle3d> const & triangles() const {return m_Triangulation.triangles();}

        void translateTriangles(const Vector3d &T) {m_Triangulation.translate(T);}
        int makeTriangleNodes() {return m_Triangulation.makeNodes();}
        void makeNodeNormals(bool bReversed=false) {m_Triangulation.makeNodeNormals(bReversed);}

        int triangleNodeCount() const {return m_Triangulation.nodeCount();}
        void clearTriangleNodes() {m_Triangulation.clearNodes();}
        QVector<Node> const &triangleNodes() {return m_Triangulation.nodes();}
        void appendTriangleNodes(QVector<Node>const & nodelist) {m_Triangulation.appendNodes(nodelist);}
        void setTriangleNodes(QVector<Node> const &nodes) {m_Triangulation.setNodes(nodes);}

        Node const & triangleNode(int index) {return m_Triangulation.node(index);}

        virtual void computeStructuralInertia(Vector3d const &PartPosition) override;
        virtual void computeSurfaceProperties(QString &log, QString const &prefix) = 0;
        virtual void computeWettedArea();

        virtual bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;

        virtual void getProperties(QString &props, const QString &prefix, bool bFull=false);

        OccMeshParams const &occMeshParams() const {return m_OccMeshParams;}
        void setOccMeshParams(OccMeshParams const &params) {m_OccMeshParams=params;}

        double maxElementSize() const {return m_MaxElementSize;}
        void setMaxElementSize(double l) {m_MaxElementSize=l;}

        static Fuse::enumType bodyPanelType(QString strPanelType);
        static QString bodyPanelType(Fuse::enumType panelType);

        static bool isOccTessellator() {return s_bOccTessellator;}
        static void setOccTessellator(bool bOcc) {s_bOccTessellator=bOcc;}

        void listShells();
        void listShapes();

    protected:

        Fuse::enumType m_FuseType; /** @todo useless now that each fuse is defined in its own subclass - remove */

        TopoDS_ListOfShape m_Shape;  /** The list of shapes of which this body is made: solids, shells, etc. */
        TopoDS_ListOfShape m_Shell;  /** The list of shells AFTER the cut operation. Used for display and mesh generation.*/


        double m_WettedArea;
        double m_MaxWidth;
        double m_MaxHeight;
        double m_MaxFrameArea;

        Vector3d m_Force;     /** The force on the fUSE (N/q) */
        Vector3d m_Mi;        /** The moments of induced or pressure forces (N.m/q) w.r.t the polar's ref CoG */
        Vector3d m_CP;        /**< the centre of pressure's position */


        TriMesh m_TriMesh;

        Triangulation m_BaseTriangulation;  /** the triangulation of the UNCUT fuse; used to make the wing surfaces */
        Triangulation m_Triangulation;

        double m_MaxElementSize;

        OccMeshParams m_OccMeshParams;

    protected:
        // temp variables;
        mutable int m_nBlocks;

    public:
        static bool s_bOccTessellator;
};



