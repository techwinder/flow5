/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/** @file
 *
 * This file implements the class for the PlaneXfl object.
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
#include <xflobjects/objects_globals/optstructures.h>
#include <xflpanels/mesh/quadmesh.h>
#include <xflpanels/mesh/trimesh.h>


class AngleControl;
class Surface;
class PlaneOpp;

class PlaneXfl : public Plane
{
    friend class MainFrame;
    friend class PlaneXflDlg;
    friend class XPlane;
    friend class gl3dXPlaneView;
    friend class Results3dControls;

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

        WingXfl *addWing(WingXfl::enumType wingtype = WingXfl::Main);
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

        int nWings() const override {return m_Wing.size();}
        int nParts() const override {return m_PartIndexList.size();}

        int partUniqueIndex(int pos) const {return m_PartIndexList.at(pos);}
        void swapIndexes(int k, int l);

        Vector3d const &fusePos(int idx) const { return m_Fuse.at(idx)->position(); }
        void setFusePos(int idx, Vector3d const &pos){m_Fuse[idx]->setPosition(pos);}

        Vector3d const &wingLE(int iWing) const{return m_Wing.at(iWing).position();}
        void setWingLE(int iWing, Vector3d const &LE) {m_Wing[iWing].setPosition(LE);}

        double rxAngle(int iWing) const {return m_Wing[iWing].m_rx;}
        void setRxAngle(int iWing, double rx) {m_Wing[iWing].m_rx=rx;}

        double ryAngle(int iWing) const {return m_Wing[iWing].m_ry;}
        void setRyAngle(int iWing, double ry) {m_Wing[iWing].m_ry=ry;}

        WingXfl *wing(int iw) override;
        WingXfl const*wingAt(int iw) const override;
        WingXfl *wing(WingXfl::enumType wingType);

        /** Returns a pointer to the Plane's fuse, or NULL if none. */
        Fuse *fuse(int idx) override {if(idx>=0 && idx<m_Fuse.size()) return m_Fuse[idx];    else return nullptr;}
        Fuse const *fuseAt(int idx) const override {if(idx>=0 && idx<m_Fuse.size()) return m_Fuse.at(idx); else return nullptr;}
        Fuse *fuse(QString &fusename);


        double mac()            const override {if(mainWing()) return mainWing()->MAC();           else return 0.0;}
        double span()           const override {if(mainWing()) return mainWing()->planformSpan();  else return 0.0;}
        double rootChord()      const override {if(mainWing()) return mainWing()->rootChord();     else return 0.0;}
        double tipChord()       const override {if(mainWing()) return mainWing()->tipChord();      else return 0.0;}
        double projectedArea(bool bOtherWings)  const override;
        double planformArea(bool bOtherWings)   const override;
        double projectedSpan()  const override {if(mainWing()) return mainWing()->projectedSpan(); else return 0.0;}
        double planformSpan()   const override {if(mainWing()) return mainWing()->planformSpan();  else return 0.0;}
        double aspectRatio()    const override {if(mainWing()) return mainWing()->aspectRatio();   else return 0.0;}
        double taperRatio()     const override {if(mainWing()) return mainWing()->taperRatio();    else return 0.0;}
        double averageSweep()   const override {if(mainWing()) return mainWing()->averageSweep();  else return 0.0;}

        int nStations() const override;

        Vector3d rootQuarterPoint(int iw) const;

        bool isWing()     const   override  {return nWings()==1;}

        Fuse *setFuse(bool bFuse, Fuse::enumType bodytype=Fuse::NURBS);
        bool hasFuse() const override {return m_Fuse.size()>0;}

        int partCount() const {return m_PartIndexList.size();}
        Part const *partAt(int iPart) const;
        Part const *partFromIndex(int UniqueIndex) const; // unused

        WingXfl const *wingFromName(QString const &name) const;
        Fuse const *fuseFromName(QString const &name) const;

        int wingIndex(WingXfl* pWing) const;
        int fuseIndex(Fuse *pFuse) const;

        bool checkFoils(QString &log) const override;

        //Methods related to inertia
        void computeStructuralInertia() override;


        QString planeData(bool bOtherWings=false) const override;

        void makeTriMesh(bool bThickSurfaces=false) override;
        bool connectTriMesh(bool bConnectTE, bool bThickSurfaces) override;
        void makeQuadMesh(bool bThickSurfaces, bool bIgnoreFusePanels);
        void restoreMesh() override;
        int nPanel4() const  override {return m_QuadMesh.nPanels();}

        Panel4 const &panel4(int index) const {return m_QuadMesh.panelAt(index);}
        Panel4 &panel4(int index)             {return m_QuadMesh.panel(index);}
        QVector<Panel4> const &quadpanels() const {return m_QuadMesh.panels();}
        QuadMesh const &quadMesh() const      {return m_QuadMesh;}
        QuadMesh &quadMesh()                  {return m_QuadMesh;}
        QuadMesh const & refQuadMesh() const  {return m_RefQuadMesh;}
        QuadMesh & refQuadMesh() {return m_RefQuadMesh;}

        int nFuse() const override {return m_Fuse.size();}
        int fuseCount() const {return m_Fuse.size();}
        int xflFuseCount() const;
        int occFuseCount() const;
        int stlFuseCount() const;
        void clearFuse();
        void addFuse(Fuse *pFuse);
        void removeFuse(Fuse *pFuse);

        void makeUniqueIndexList();

        bool hasWPolar(const WPolar *pWPolar) const;
        bool hasPOpp(const PlaneOpp *pPOpp) const;


        int nAVLGains() const;
        QString controlSurfaceName(int iCtrl) const;

        double flapAngle(int iWing, int iFlap) const;
        QString flapName(int iFlap) const;
        int nFlaps(int iWing) const;
        int nFlaps() const;
        double flapPosition(const AngleControl &avlc, int iWing, int iFlap) const;
        void setFlaps(WPolar const *pWPolar, QString &outstring);

        void setRangePositions4(WPolar const *pWPolar, double t, QString &outstring);
        void setRangePositions3(WPolar const *pWPolar, double t, QString &outstring);

        void rotateWingNodes(QVector<Panel3> const &panel3, QVector<Node> &node, WingXfl const *pWing, Vector3d const &hingePoint, Vector3d const & hingeVector, double alpha) const;
        void rotateFlapNodes(QVector<Panel3> const &panel3, QVector<Node> &node, Surface const &surf, Vector3d const &hingePoint, Vector3d const &hingeVector, double theta) const;

    private:

        QVector<WingXfl> m_Wing;                       /**< the array of Wing objects used to define this Plane */

        QVector<Fuse *>m_Fuse;                                /**< the fuse object */


        QVector<int> m_PartIndexList;       /** provides the order in which the part will be merged to form a solid body */

        QuadMesh m_RefQuadMesh; /** The reference quad mesh, with non-rotated panels */
        QuadMesh m_QuadMesh;    /** The active quad mesh, with panels rotated with surface angles */


};


