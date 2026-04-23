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



#include <vector>



#include <triangulation.h>
#include <vector3d.h>
#include <mathelem.h>
#include <aeroforces.h>
#include <spandistribs.h>
#include <part.h>
#include <enums_objects.h>
#include <quadmesh.h>
#include <trimesh.h>
#include <edgesplit.h>



class BoatPolar;
class QuadPanel;
class WingSailSection;
class TriMesh;
class Panel4;
class Panel3;

class FL5LIB_EXPORT Sail : public Part
{
    friend class Boat;
    friend class BoatAnalysisDlg;
    friend class BoatDlg;
    friend class BoatOpp;
    friend class BoatPolar;
    friend class BoatPolarDlg;
    friend class BoatTask;
    friend class MainFrame;
    friend class SailDlg;
    friend class ExternalSailDlg;
    friend class ThinSailDlg;
    friend class SailOccDlg;
    friend class STLSailDlg;
    friend class SailViewWidget;
    friend class SectionViewWidget;
    friend class gl3dXSailView;

    public:
        Sail();
        virtual ~Sail();

        bool isExternalSail() const {return isStlSail()||isOccSail();}

        virtual bool isNURBSSail()  const = 0;
        virtual bool isSplineSail() const = 0;
        virtual bool isOccSail() const = 0;
        virtual bool isWingSail() const = 0;
        virtual bool isStlSail() const = 0;

        virtual Sail* clone() const = 0;

        virtual void resizeSections(int nSections, int nPoints) = 0;
        virtual void makeDefaultSail() = 0;

        virtual void scale(double XFactor, double YFactor, double ZFactor);
        virtual void translate(Vector3d const &T);

        virtual void scaleArea( double newarea);
        virtual void scaleTwist(double newtwist) {(void)newtwist;}
        virtual void scaleAR(   double newAR)    {(void)newAR;}

        virtual void duplicate(const Sail *pSail);
        virtual void makeTriangulation(int nx=s_iXRes, int nh=s_iZRes); // on request from the UI only

        virtual double aspectRatio() const {double l=luffLength(); return l*l/area();}
        virtual double luffLength() const =0;
        virtual double leechLength() const =0;
        virtual double footLength() const =0;
        virtual double twist() const;

        virtual double area() const {return m_Triangulation.wettedArea();}


        virtual int sectionCount() const {return 0;}
        int nSections() const {return sectionCount();}

        virtual double size() const;

        virtual void flipXZ() = 0;

        virtual Vector3d point(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const =0;
        virtual Node edgeNode(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const;
        virtual Vector3d normal(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const;
        virtual void makeSurface() = 0;
        virtual bool serializeSailFl5(QDataStream &ar, bool bIsStoring);

        virtual void createSection(int iSection) = 0;
        virtual void deleteSection(int iSection) = 0;

        virtual void properties(std::string &properties, std::string const &prefix, bool bFull=false) const;
        virtual Vector3d leadingEdgeAxis() const = 0;

        PART::enumPartType partType() const override {return PART::Sail;}
        bool isWing() const override {return false;}
        bool isFuse() const override {return false;}
        bool isSail() const override {return true;}

        bool bRuledMesh() const {return m_bRuledMesh;}
        void setRuledMesh(bool bRuled) {m_bRuledMesh=bRuled;}
        virtual void makeTriPanels(Vector3d const &Tack);
        virtual void makeRuledMesh(Vector3d const &);

        void mergeRefNodes(Node const& src, Node const&dest);

        double luffAngle() const {return m_LuffAngle;}
        void setLuffAngle(double angle) {m_LuffAngle = angle;}

        Vector3d center() const {return point(0.5,0.5);}


        virtual bool intersect(const Vector3d &A, const Vector3d &B, Vector3d &I, Vector3d &N) const;

        int nXPanels() const {return m_NXPanels;}
        void setNXPanels(int nx) {m_NXPanels=nx;}

        xfl::enumDistribution xDistType() const {return m_XDistrib;}
        void setXDistType(xfl::enumDistribution xdisttype) {m_XDistrib = xdisttype;}

        virtual int nStations() const {return int(m_BotMidTEIndexes.size());}

        int nPanel4() const override {return 0;}

        bool hasPanel3(int index) const;

        void panel3ComputeInviscidForces(const std::vector<Panel3> &panel3list,
                                         BoatPolar *pBtPolar, Vector3d const &cog, double beta, double *Cp3Vtx,
                                         Vector3d &CP, Vector3d &Force, Vector3d &Moment) const;

        void computeStructuralInertia(Vector3d const &) override {}

        double refArea() const {return m_RefArea;}
        void setRefArea(double a) {m_RefArea=a;}

        double refChord() const {return m_RefChord;}
        void setRefChord(double ch) {m_RefChord=ch;}

        SpanDistribs & spanDistFF() {return m_SpanResFF;}
        SpanDistribs const & spanDistSum() const {return m_SpanResSum;}

        Vector3d const &clew() const {return m_Clew;}
        Vector3d const &head() const {return m_Head;}
        Vector3d const &tack() const {return m_Tack;}
        Vector3d const &peak() const {return m_Peak;}

        virtual void updateStations();

        void setTEfromIndexes();
        void clearTEIndexes();
        void addTEindex(int idx, bool bBotMid);
        bool removeTEindex(int i3, bool bBotMid);
        std::vector<int> const &topTEIndexes() const {return m_TopTEIndexes;}
        std::vector<int> const &botMidTEIndexes() const {return m_BotMidTEIndexes;}

        void clearRefTriangles() {m_RefTriangles.clear();}
        Triangle3d const& refTriangleAt(int index) const {return m_RefTriangles.at(index);}
        Triangle3d & refPanel(int index) {return m_RefTriangles[index];}
        std::vector<Triangle3d> const& refTriangles() {return m_RefTriangles;}
        void setRefTriangles(std::vector<Triangle3d> const &triangles) {m_RefTriangles = triangles;}

        bool isThinSurface() const {return m_bThinSurface;}
        void setThinSurface(bool bThin) {m_bThinSurface=bThin;}

        double maxElementSize() const {return m_MaxElementSize;}
        void setMaxElementSize(double l) {m_MaxElementSize=l;}


        void saveConnections();

        double edgeLength(double umin, double vmin, double umax, double vmax) const;

        virtual bool makeOccShell(TopoDS_Shape &, std::string &) const {return false;}

        int nZPanels() const {return m_NZPanels;}
        void setNZPanels(int nz) {m_NZPanels=nz;}
        xfl::enumDistribution zDistType() const {return m_ZDistrib;}
        void setZDistType(xfl::enumDistribution xdisttype) {m_ZDistrib = xdisttype;}


        std::vector<std::vector<EdgeSplit>> const &edgeSplit() {return m_EdgeSplit;} // for each face<each edge>



        static void setTessellation(int iXRes, int iZRes) {s_iXRes=iXRes; s_iZRes=iZRes;}
        static int iXRes() {return s_iXRes;}
        static int iZRes() {return s_iZRes;}

    protected:


        bool m_bThinSurface;


        //relative position of corner points in the sail's referential
        Vector3d m_Tack, m_Head, m_Clew, m_Peak;

        double m_RefArea;    /** Refernce area for the calculation of lift and drag coefficients if this sail is the main sail */
        double m_RefChord;   /** Refernce chord mainly used to define the vorton core size and the step of the VPW */

        //Global geometry
        double m_LuffAngle; /** @todo remove */

        int m_NXPanels; 		// VLM Panels along horizontal direction
        xfl::enumDistribution m_XDistrib;

        int m_NZPanels;
        xfl::enumDistribution m_ZDistrib;

        SpanDistribs m_SpanResFF;         /** Span strip coefficients from far-field calculations */
        SpanDistribs m_SpanResSum;        /** Span strip coefficients from force summation  */

        bool m_bRuledMesh;
        double m_MaxElementSize; /** used by the flow5 mesher */


        std::vector<int> m_TopTEIndexes, m_BotMidTEIndexes;
        std::vector<Triangle3d> m_RefTriangles; /** the array of triangles used to construct the triangle mesh */

        std::vector<std::vector<EdgeSplit>> m_EdgeSplit; // for each face<each edge>



        static int s_iXRes;
        static int s_iZRes;
};

