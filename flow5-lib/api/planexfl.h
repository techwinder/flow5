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

/** @file
 *
 * This file implements the class FL5LIB_EXPORT for the PlaneXfl object.
 */



#pragma once


/**
 *@class  Plane
 *@brief
 * The class  which defines the Plane object used in 3D calculations.
 *  - defines the plane object
 *  - provides the methods for the calculation of the plane's geometric properties
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds
 * Angular data is stored in degrees
*/

#include <enums_objects.h>
#include <fuse.h>
#include <inertia.h>
#include <linestyle.h>
#include <optstructures.h>
#include <plane.h>
#include <quadmesh.h>
#include <trimesh.h>
#include <wingxfl.h>


class AngleControl;
class Surface;
class PlaneOpp;

class FL5LIB_EXPORT PlaneXfl : public Plane
{
    friend class  MainFrame;
    friend class  PlaneXflDlg;
    friend class  XPlane;
    friend class  gl3dXPlaneView;
    friend class  Results3dControls;

    public:
        PlaneXfl(bool bDefaultPlane=false);
        PlaneXfl(const PlaneXfl &aPlane);
        ~PlaneXfl() override;

        bool isSTLType() const override {return false;}
        bool isXflType() const override {return true;}

        void makeDefaultPlane();

        void swapWings(int iWing1, int iWing2);
        void swapFuses(int iFuse1, int iFuse2);

        double tailVolumeHorizontal() const override;
        double tailVolumeVertical() const override;

        void duplicate(Plane const *pPlane) override;

        void duplicatePanels(Plane const *pPlane);

        void copyMetaData(Plane const *pOtherPlane) override;
        void scale(double scalefactor) override;
        void translate(Vector3d const &T) override;

        void lock() override;
        void unlock() override;

        void createSurfaces();
        void createWingSideNodes();

        void makePlane(bool bThickSurfaces, bool bIgnoreFusePanels, bool bMakeTriMesh) override;
        Fuse *makeNewFuse(Fuse::enumType bodytype);
        void joinSurfaces(const Surface &LeftSurf, const Surface &RightSurf);

        bool serializePlaneXFL(QDataStream &ar, bool bIsStoring);
        bool serializePlaneFl5(QDataStream &ar, bool bIsStoring) override;

        bool hasMainWing() const override;
        bool hasOtherWing() const override;
//        bool hasWing2() const override;
        bool hasStab() const override;
        bool hasFin() const override;

        WingXfl *mainWing();                 /** Returns a pointer to the plane's first found MAINWING, or NULL if none. */
        WingXfl const *mainWing() const;

        WingXfl *stab();                     /** Returns a pointer to the plane's first found ELEVATOR, or NULL if none. */
//        Wing* wing2();                    /** Returns a pointer to the plane's first found OTHERWING, or NULL if none. */
        WingXfl* fin();                      /** Returns a pointer to the plane's first found FIN, or NULL if none. */

        WingXfl *addWing(xfl::enumType wingtype = xfl::Main);
        WingXfl *addWing(WingXfl *pNewWing);
        WingXfl *duplicateWing(int iWing);
        void removeWing(WingXfl*pWing);
        void removeWing(int iWing);
        void removeWings();
        void removeFuse(int iFuse=0);
        Fuse *duplicateFuse(int iFuse);

        int VLMPanelTotal() const;
        int quadCount() const;
        int triangleCount() const;

        int nWings() const override {return int(m_Wing.size());}
        int nParts() const override {return int(m_PartIndexes.size());}

        int partUniqueIndex(int pos) const {return m_PartIndexes.at(pos);}
        void swapIndexes(int k, int l);

        Vector3d const &fusePos(int idx) const { return m_Fuse.at(idx)->position(); }
        void setFusePos(int idx, Vector3d const &pos){m_Fuse[idx]->setPosition(pos);}

        Vector3d const &wingLE(int iWing) const{return m_Wing.at(iWing).position();}
        void setWingLE(int iWing, Vector3d const &LE) {m_Wing[iWing].setPosition(LE);}

        double rxAngle(int iWing) const {return m_Wing[iWing].rx();}
        void setRxAngle(int iWing, double rx) {m_Wing[iWing].setRx(rx);}

        double ryAngle(int iWing) const {return m_Wing[iWing].ry();}
        void setRyAngle(int iWing, double ry) {m_Wing[iWing].setRy(ry);}

        WingXfl *wing(int iw) override;
        WingXfl const*wingAt(int iw) const override;
        WingXfl *wing(xfl::enumType wingType);

        /** Returns a pointer to the Plane's fuse, or NULL if none. */
        Fuse *fuse(int idx) override {if(idx>=0 && idx<nFuse()) return m_Fuse[idx];    else return nullptr;}
        Fuse const *fuseAt(int idx) const override {if(idx>=0 && idx<nFuse()) return m_Fuse.at(idx); else return nullptr;}
        Fuse *fuse(const std::string &fusename);


        double mac()            const override {if(mainWing()) return mainWing()->MAC();           else return 0.0;}
        double span()           const override {if(mainWing()) return mainWing()->planformSpan();  else return 0.0;}
        double rootChord()      const override {if(mainWing()) return mainWing()->rootChord();     else return 0.0;}
        double tipChord()       const override {if(mainWing()) return mainWing()->tipChord();      else return 0.0;}
        double projectedArea(bool bOtherWings=false)  const override;
        double planformArea(bool bOtherWings=false)   const override;
        double projectedSpan()  const override {if(mainWing()) return mainWing()->projectedSpan(); else return 0.0;}
        double planformSpan()   const override {if(mainWing()) return mainWing()->planformSpan();  else return 0.0;}
        double aspectRatio()    const override {if(mainWing()) return mainWing()->aspectRatio();   else return 0.0;}
        double taperRatio()     const override {if(mainWing()) return mainWing()->taperRatio();    else return 0.0;}
        double averageSweep()   const override {if(mainWing()) return mainWing()->averageSweep();  else return 0.0;}

        int nStations() const override;

        Vector3d rootQuarterPoint(int iw) const;

        bool isWing()     const   override  {return nWings()==1;}

        Fuse *setFuse(bool bFuse, Fuse::enumType bodytype=Fuse::NURBS);
        bool hasFuse() const override {return nFuse()>0;}

        int partCount() const {return int(m_PartIndexes.size());}
        Part const *partAt(int iPart) const;
        Part const *partFromIndex(int UniqueIndex) const; // unused

        WingXfl const *wingFromName(std::string const &name) const;
        Fuse const *fuseFromName(std::string const &name) const;

        int wingIndex(WingXfl* pWing) const;
        int fuseIndex(Fuse *pFuse) const;

        bool checkFoils(std::string &log) const override;

        //Methods related to inertia
        void computeStructuralInertia() override;


        std::string planeData(bool bOtherWings=false) const override;

        void makeTriMesh(bool bThickSurfaces=false) override;
        bool connectTriMesh(bool bRefTriMesh, bool bConnectTE, bool bMultiThreaded) override;
        void makeQuadMesh(bool bThickSurfaces, bool bIgnoreFusePanels);
        void restoreMesh() override;
        int nPanel4() const  override {return m_QuadMesh.nPanels();}

        Panel4 const &panel4(int index) const {return m_QuadMesh.panelAt(index);}
        Panel4 &panel4(int index)             {return m_QuadMesh.panel(index);}
        std::vector<Panel4> const &quadpanels() const {return m_QuadMesh.panels();}
        QuadMesh const &quadMesh() const      {return m_QuadMesh;}
        QuadMesh &quadMesh()                  {return m_QuadMesh;}
        QuadMesh const & refQuadMesh() const  {return m_RefQuadMesh;}
        QuadMesh & refQuadMesh() {return m_RefQuadMesh;}

        int nFuse() const override {return int(m_Fuse.size());}
        int fuseCount() const {return int(m_Fuse.size());}
        int xflFuseCount() const;
        int occFuseCount() const;
        int stlFuseCount() const;
        void clearFuse();
        void addFuse(Fuse *pFuse);
        void removeFuse(Fuse *pFuse);

        void makeUniqueIndexList();

        bool hasPolar(const PlanePolar *pWPolar) const;
        bool hasPOpp(const PlaneOpp *pPOpp) const;


        int nAVLGains() const;
        std::string controlSurfaceName(int iCtrl) const;

        double flapAngle(int iWing, int iFlap) const;
        std::string flapName(int iFlap) const;
        int nFlaps(int iWing) const;
        int nFlaps() const;
        double flapPosition(const AngleControl &avlc, int iWing, int iFlap) const;
        void setFlaps(PlanePolar const *pWPolar, std::string &outstring);

        void setRangePositions4(PlanePolar const *pWPolar, double t, std::string &outstr);
        void setRangePositions3(PlanePolar const *pWPolar, double t, std::string &outstring);

        void rotateWingNodes(const std::vector<Panel3> &panel3, std::vector<Node> &node, WingXfl const *pWing, Vector3d const &hingePoint, Vector3d const & hingeVector, double alpha) const;
        void rotateFlapNodes(std::vector<Panel3> const &panel3, std::vector<Node> &node, Surface const &surf, Vector3d const &hingePoint, Vector3d const &hingeVector, double theta) const;


        void setThickBuild(bool b) {m_bThickBuild=b;}
        bool isThickBuild() const {return m_bThickBuild;}


    private:

        bool m_bThickBuild;

        std::vector<WingXfl> m_Wing;                       /**< the array of Wing objects used to define this Plane */

        std::vector<Fuse *>m_Fuse;                                /**< the fuse object */


        std::vector<int> m_PartIndexes;       /** provides the order in which the part will be merged to form a solid body */

        QuadMesh m_RefQuadMesh; /** The reference quad mesh, with non-rotated panels */
        QuadMesh m_QuadMesh;    /** The active quad mesh, with panels rotated with surface angles */


    public:
        mutable std::vector<OptVariable> m_OptVariables;
};


