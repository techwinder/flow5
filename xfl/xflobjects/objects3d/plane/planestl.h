/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/** @file
 *
 * This file implements the class for the PlaneSTl object.
 */



#pragma once


/**
 *@class Plane
 *@brief
 * The class which defines the Plane object used in 3D calculations.
 *  - defines the plane object
 *  - provides the methods for the calculation of the plane's geometric properties
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds
 * Angular data is stored in degrees
*/

#include <xflcore/enums_objects.h>
#include <xflcore/linestyle.h>
#include <xflobjects/objects3d/fuse/fuse.h>
#include <xflobjects/objects3d/inertia/inertia.h>
#include <xflobjects/objects3d/plane/plane.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflpanels/mesh/quadmesh.h>
#include <xflpanels/mesh/trimesh.h>


class PlaneSTL : public Plane
{
    friend class PlaneSTLDlg;

    public:
        PlaneSTL();
        bool isSTLType() const override {return true;}
        bool isXflType() const override {return false;}

        QString planeData(bool bOtherWings = false) const override;

        void copyMetaData(const Plane *pOtherPlane) override;

        void duplicate(Plane const *pPlane) override;
        void duplicatePanels(Plane const *pPlane);

        void makePlane(bool bThickSurfaces, bool bIgnoreFusePanels, bool bMakeTriMesh) override;
        bool serializePlaneFl5(QDataStream &ar, bool bIsStoring) override;

        void makeTriMesh(bool bThickSurfaces = false) override;

        int nParts() const override {return 0;}
        int nWings() const override {return 0;}
        int nFuse()  const override {return 0;}


        bool hasMainWing() const override {return false;}
//        bool hasWing2() const override {return false;}
        bool hasOtherWing() const override {return false;}
        bool hasStab() const override {return false;}
        bool hasFin() const override {return false;}
        bool hasFuse() const override {return false;}

        // ref dimensions
        double mac()            const override {return m_ReferenceChord;}
        double span()           const override {return m_ReferenceSpan;} /** @todo duplicate with planform and projected spans? */
        double rootChord()      const override {return m_ReferenceChord;}
        double tipChord()       const override {return m_ReferenceChord;}
        double projectedArea(bool)  const override {return m_ReferenceArea;}
        double planformArea(bool)   const override {return m_ReferenceArea;}
        double projectedSpan()  const override {return m_ReferenceSpan;}
        double planformSpan()   const override {return m_ReferenceSpan;}
        double aspectRatio()    const override {if(fabs(m_ReferenceArea)>0.00001) return m_ReferenceSpan*m_ReferenceSpan/m_ReferenceArea; else return 0.0;}
        double taperRatio()     const override {return 1.0;}
        double averageSweep()   const override {return 0.0;}

        int nStations() const override;

        double refChord() const {return m_ReferenceChord;}
        double refSpan() const {return m_ReferenceSpan;}
        double refArea() const {return m_ReferenceArea;}
        void setRefChord(double ch) {m_ReferenceChord=ch;}
        void setRefSpan(double span) {m_ReferenceSpan=span;}
        void setRefArea(double area) {m_ReferenceArea=area;}

        bool connectTriMesh(bool bConnectTE, bool bThickSurfaces) override;

        bool intersectTriangles(Vector3d A, Vector3d B, Vector3d &I);
        void blockIntersect(int iBlock, Vector3d A, Vector3d B, Vector3d *I, bool *bIntersect) const;

        bool reversed() const {return m_bReversed;}
        void setReversed(bool bReversed) {m_bReversed = bReversed;}

        void scale(double ScaleFactor) override;
        void translate(Vector3d const &T)  override;
        void rotate(const Vector3d &O, const Vector3d &axis, double theta);

        //Methods related to inertia and geometry
        void computeStructuralInertia() override;
        void computeSurfaceProperties();

        //TRIANGULATION related methods
        Triangulation const &triangulation() const {return m_Triangulation;}
        Triangulation &triangulation() {return m_Triangulation;}

        void setBaseTriangles(QVector<Triangle3d> const &trianglelist) {m_Triangulation.setTriangles(trianglelist); m_bIsInitialized=false;}

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

        void flipTriangleNormals();

        double maxSize() const;

        QColor surfaceColor() const {return m_SurfaceColor;}
        void setSurfaceColor(QColor const &clr) {m_SurfaceColor=clr;}

    private:
        Triangulation m_Triangulation;
        bool m_bReversed;
        int m_nBlocks;

        double m_ReferenceArea;
        double m_ReferenceSpan;
        double m_ReferenceChord;

        double m_WettedArea;
        double m_Span;
        double m_Length;
        double m_Height;

        QColor m_SurfaceColor;
};


