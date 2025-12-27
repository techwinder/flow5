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


#include <gmshparams.h>
#include <inertia.h>
#include <linestyle.h>
#include <mathelem.h>
#include <pointmass.h>
#include <triangulation.h>
#include <trimesh.h>


namespace PART
{
    /** @enum The types of parts of which a plane or boat is made*/
    enum enumPartType {Wing, Fuse, Sail, OtherPart};
}


class FL5LIB_EXPORT Part
{
    public:
        Part();
        virtual ~Part();

        std::string const &name() const {return m_Name;}
        std::string const &description() const {return m_Description;}

        void setName(std::string const &name) {m_Name=name;}
        void setDescription(std::string const &des) {m_Description=des;}

        LineStyle theStyle() const {return m_theStyle;}
        void setTheStyle(LineStyle ls){m_theStyle=ls;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) const {m_theStyle.m_bIsVisible=bVisible;}
        int style()    const {return m_theStyle.m_Stipple;}
        int width()    const {return m_theStyle.m_Width;}
        fl5Color const &color() const {return m_theStyle.m_Color;}

        void setStyle(Line::enumLineStipple Style)   {m_theStyle.m_Stipple = Style;}
        void setWidth(int Width)                     {m_theStyle.m_Width = Width;}
        virtual void setColor(fl5Color const &clr) {m_theStyle.m_Color = clr;}
        void setRed(int red)          {m_theStyle.m_Color.setRed(red);}
        void setGreen(int green)      {m_theStyle.m_Color.setGreen(green);}
        void setBlue(int blue)        {m_theStyle.m_Color.setBlue(blue);}
        void setAlphaBlend(int alpha) {m_theStyle.m_Color.setAlpha(alpha);}

        int firstPanel3Index() const {return m_FirstPanel3Index;}
        int firstPanel4Index() const {return m_FirstPanel4Index;}
        void setFirstPanel3Index(int index) {m_FirstPanel3Index = index;}
        void setFirstPanel4Index(int index) {m_FirstPanel4Index = index;}

        int firstNodeIndex() const {return m_FirstNodeIndex;}
        void setFirstNodeIndex(int index) {m_FirstNodeIndex = index;}

        void lock()   {m_bLocked=true;}
        void unlock() {m_bLocked=false;}
        bool isLocked() const {return m_bLocked;}

        void duplicatePart(Part const &part);

        virtual int nPanel4() const = 0;
        virtual int nPanel3() const {return m_TriMesh.nPanels();}
        int nNodes() const {return m_TriMesh.nNodes();}
        TriMesh & triMesh() {return m_TriMesh;}
        TriMesh const & triMesh() const{return m_TriMesh;}
        void clearTriMesh() {m_TriMesh.clearMesh();}

        int panel3NodeCount() const {return m_TriMesh.nodeCount();}
        Panel3 &panel3(int index) {return m_TriMesh.panel(index);}
        Panel3 const &panel3At(int index) const {return m_TriMesh.panelAt(index);}

        Node const & panel3Node(int index)  {return m_TriMesh.node(index);}
        std::vector<Node> &nodes() {return m_TriMesh.nodes();}
        Node const &nodeAt(int index) const {return m_TriMesh.nodeAt(index);}

        void setTriMesh(TriMesh const &trimesh) {m_TriMesh=trimesh;}

        virtual PART::enumPartType partType() const = 0;

        virtual bool isWing() const = 0;
        virtual bool isFuse() const = 0;
        virtual bool isSail() const = 0;

        virtual void computeStructuralInertia(Vector3d const &PartPosition) = 0;

        void setLength(double l) {m_Length=l;} /** @todo remove? */
        virtual double length() const {return m_Length;}

        int uniqueIndex() const {return m_UniqueIndex;}
        void setUniqueIndex();

        virtual bool serializePartFl5(QDataStream &ar, bool bIsStoring);

        // INERTIA
        // all inertia properties are defined in the part's body axis

        void setAutoInertia(bool bAuto) {m_bAutoInertia=bAuto;}
        bool bAutoInertia() const {return m_bAutoInertia;}

        Inertia const &inertia() const {return m_Inertia;}
        Inertia &inertia() {return m_Inertia;}
        void setInertia(Inertia const &inertia) {m_Inertia = inertia;}
        void copyInertia(Part const &part);

        double totalMass()      const {return m_Inertia.totalMass();}
        double structuralMass() const {return m_Inertia.structuralMass();}
        void setStructuralMass(double mass){m_Inertia.setStructuralMass(mass);}
        Vector3d CoG_t() const {return m_Inertia.CoG_t();}
        Vector3d CoG_s() const {return m_Inertia.CoG_s();}
        void setCoG_s(Vector3d cog) {m_Inertia.setCoG_s(cog);}

        double Ixx_s() const {return m_Inertia.Ixx_s();}
        double Iyy_s() const {return m_Inertia.Iyy_s();}
        double Izz_s() const {return m_Inertia.Izz_s();}
        double Ixy_s() const {return m_Inertia.Ixz_s();}
        double Ixz_s() const {return m_Inertia.Ixy_s();}
        double Iyz_s() const {return m_Inertia.Iyz_s();}

        double Ixx_t() const {return m_Inertia.Ixx_t();}
        double Iyy_t() const {return m_Inertia.Iyy_t();}
        double Izz_t() const {return m_Inertia.Izz_t();}
        double Ixy_t() const {return m_Inertia.Ixy_t();}
        double Ixz_t() const {return m_Inertia.Ixz_t();}
        double Iyz_t() const {return m_Inertia.Iyz_t();}

        void setIxx_s(double Ixx) {m_Inertia.setIxx_s(Ixx);}
        void setIyy_s(double Iyy) {m_Inertia.setIyy_s(Iyy);}
        void setIzz_s(double Izz) {m_Inertia.setIzz_s(Izz);}
        void setIxz_s(double Ixz) {m_Inertia.setIxz_s(Ixz);}

        void setPointMasses(std::vector<PointMass> const &PointMass){m_Inertia.setPointMasses(PointMass);}
        void clearPointMasses() {m_Inertia.clearPointMasses();}
        void removePointMass(int iMass) {if(iMass>=0 && iMass<pointMassCount()) m_Inertia.removePointMass(iMass);}

        std::vector<PointMass> const &pointMasses() const {return m_Inertia.pointMasses();}

        const PointMass &pointMassAt(int index) const {return m_Inertia.pointMassAt(index);}
        PointMass &pointMass(int index) {return m_Inertia.pointMass(index);}

        int pointMassCount() const {return m_Inertia.pointMassCount();}

        void insertPointMass(int pos, PointMass const &pm) {m_Inertia.insertPointMass(pos, pm);}
        void appendPointMass(PointMass const &pm) {m_Inertia.appendPointMass(pm);}

        Vector3d const &position() const {return m_LE;}
        void setPosition(Vector3d const &pos){m_LE = pos;}
        void setPosition(double x, double y, double z){m_LE.set(x,y,z);}

        double rx() const {return m_rx;}
        double ry() const {return m_ry;}
        double rz() const {return m_rz;}
        void setRx(double r) {m_rx=r;}
        void setRy(double r) {m_ry=r;}
        void setRz(double r) {m_rz=r;}

        GmshParams const &gmshTessParams() const {return m_GmshTessParams;}
        void setGmshTessParams(GmshParams const &params) {m_GmshTessParams=params;}

        GmshParams const &gmshParams() const {return m_GmshParams;}
        void setGmshParams(GmshParams const &params) {m_GmshParams=params;}

        void flipTriangulationNormals() {m_Triangulation.flipNormals();}
        int nTriangles() const {return m_Triangulation.nTriangles();}

        Triangulation const &triangulation() const {return m_Triangulation;}
        Triangulation &triangulation() {return m_Triangulation;}

        void clearTriangles() {m_Triangulation.clear();}
        virtual void setTriangles(std::vector<Triangle3d> const &trianglelist) {m_Triangulation.setTriangles(trianglelist);}
        void appendTriangle(Triangle3d const & t3) {m_Triangulation.appendTriangle(t3);}
        Triangle3d const & triangleAt(int index) const {return m_Triangulation.triangleAt(index);}
        Triangle3d & triangle(int index) {return m_Triangulation.triangle(index);}
        std::vector<Triangle3d> const & triangles() const {return m_Triangulation.triangles();}

        void translateTriangles(const Vector3d &T) {m_Triangulation.translate(T);}
        int makeTriangleNodes() {return m_Triangulation.makeNodes();}
        void makeNodeNormals(bool bReversed=false) {m_Triangulation.makeNodeNormals(bReversed);}

        int triangleNodeCount() const {return m_Triangulation.nodeCount();}
        void clearTriangleNodes() {m_Triangulation.clearNodes();}
        std::vector<Node> const &triangleNodes() {return m_Triangulation.nodes();}
        void appendTriangleNodes(std::vector<Node>const & nodelist) {m_Triangulation.appendNodes(nodelist);}
        void setTriangleNodes(std::vector<Node> const &nodes) {m_Triangulation.setNodes(nodes);}

        Node const & triangleNode(int index) {return m_Triangulation.node(index);}


        static bool isOccTessellator() {return s_bOccTessellator;}
        static void setOccTessellator(bool bOcc) {s_bOccTessellator=bOcc;}

    protected:
        int m_FirstPanel3Index;
        int m_FirstPanel4Index;
        int m_FirstNodeIndex; // only used for fuse trimeshes


        int m_UniqueIndex;            /** the unique identifier of the part */

        mutable LineStyle m_theStyle; /** The style, only mutable field*/

        std::string m_Name;
        std::string m_Description;


        /** @todo these fields belong to the plane or boat */
        Vector3d m_LE;                 /**< the part's position relative to the parent object i.e. plane or boat */
        double m_rx;                  /**< the rotation in degrees of the part about the x-axis */
        double m_ry;                  /**< the rotation in degrees of the part about the y-axis */
        double m_rz;                  /**< unused - the rotation in degrees of the part about the z-axis */


        double m_Length;

        Inertia m_Inertia;

        bool m_bAutoInertia;   /** true if the inertia properties are to be estimated automatically from the mass and the geometry */


        TriMesh m_TriMesh;

        Triangulation m_Triangulation;

        GmshParams m_GmshTessParams; /** used for tessellation */
        GmshParams m_GmshParams;     /** used to create the panels - CAD fuse + and Occ, spline and Nurbs sails*/

        bool m_bLocked;  /** true if the object instance is used by a running analysis */
        static bool s_bOccTessellator;  /** true if parts are to be tesselated with OCC's IncrementalMesher, false if using Gmsh */


};









