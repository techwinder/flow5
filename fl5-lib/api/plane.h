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

#include <fl5object.h>
#include <inertia.h>

class Surface;
class PlaneOpp;
class PlanePolar;
class WingXfl;
class Fuse;

class FL5LIB_EXPORT Plane : public fl5Object
{
    public:
        Plane();
        virtual ~Plane() = default;

        virtual std::string planeData(bool bOtherWings=false) const = 0;

        virtual bool isSTLType() const = 0;
        virtual bool isXflType() const = 0;

        virtual void duplicate(Plane const*pOtherPlane);
        virtual void copyMetaData(Plane const *pOtherPlane) = 0;
        virtual void makePlane(bool bThickSurfaces, bool bIgnoreFusePanels, bool bMakeTriMesh) = 0;
        virtual void makeTriMesh(bool bThickSurfaces = false) = 0;
        virtual bool connectTriMesh(bool bRefTriMesh, bool bConnectTE, bool bMultithreaded) = 0;

        virtual bool serializePlaneFl5(QDataStream &ar, bool bIsStoring) = 0;

        virtual int nParts() const = 0;
        virtual int nWings() const = 0;
        virtual int nFuse()  const = 0;

        virtual WingXfl* wing(int )  {return nullptr;}
        virtual WingXfl const* wingAt(int ) const  {return nullptr;}

        virtual Fuse* fuse(int ) {return nullptr;}
        virtual Fuse const* fuseAt(int ) const  {return nullptr;}

        virtual bool isWing() const {return false;}
        virtual bool hasMainWing() const = 0;
        virtual bool hasOtherWing() const = 0;
        virtual bool hasStab() const = 0;
        virtual bool hasFin() const = 0;
        virtual bool hasFuse() const = 0;

        // ref dimensions
        virtual double mac()             const = 0;
        virtual double span()            const = 0;
        virtual double rootChord()       const = 0;
        virtual double tipChord()        const = 0;
        virtual double projectedArea(bool bOtherWings=false) const = 0;
        virtual double planformArea(bool bOtherWings=false)  const = 0;
        virtual double projectedSpan()   const = 0;
        virtual double planformSpan()    const = 0;
        virtual double aspectRatio()     const = 0;
        virtual double taperRatio()      const = 0;
        virtual double averageSweep()    const = 0;
        virtual double tailVolumeHorizontal()   const  {return 0.0;}
        virtual double tailVolumeVertical()     const  {return 0.0;}

        virtual bool checkFoils(std::string &) const {return true;}


        virtual int nPanel4() const {return 0;}

        virtual void scale(double scalefactor) = 0;
        virtual void translate(Vector3d const &T) = 0;

        virtual void lock() {m_bLocked = true;}
        virtual void unlock() {m_bLocked = false;}
        bool isLocked() const {return m_bLocked;}

        //Methods related to inertia
        Inertia &inertia() {return m_Inertia;}
        Inertia const &inertia() const {return m_Inertia;}
        void setInertia(Inertia const &inertia) {m_Inertia = inertia;}

        double totalMass() const {return m_Inertia.totalMass();}
        double structuralMass() const {return m_Inertia.structuralMass();}
        void setStructuralMass(double m) {m_Inertia.setStructuralMass(m);}
        virtual void computeStructuralInertia() = 0;

        Vector3d CoG_t() const {return m_Inertia.CoG_t();}
        double Ixx_t() const {return m_Inertia.Ixx_t();}
        double Iyy_t() const {return m_Inertia.Iyy_t();}
        double Izz_t() const {return m_Inertia.Izz_t();}
        double Ixz_t() const {return m_Inertia.Ixz_t();}

        Vector3d const &CoG_s() const {return m_Inertia.CoG_s();}
        void setCoG_s(Vector3d const &cog) {m_Inertia.setCoG_s(cog);}

        void setPointMasses(std::vector<PointMass> const &PointMassList){m_Inertia.setPointMasses(PointMassList);}
        void clearPointMasses() {m_Inertia.clearPointMasses();}
        std::vector<PointMass> const &pointMassList() const {return m_Inertia.pointMasses();}
        PointMass const &pointMassAt(int index) const {return m_Inertia.pointMassAt(index);}
        PointMass &pointMass(int index) {return m_Inertia.pointMass(index);}
        int pointMassCount() const {return m_Inertia.pointMassCount();}
        void appendPointMass(PointMass const &pm) {m_Inertia.appendPointMass(pm);}

        virtual int nStations() const = 0;

        bool isInitialized() const {return m_bIsInitialized;}
        void setInitialized(bool b) {m_bIsInitialized=b;}

        bool hasPolar(const PlanePolar *pWPolar) const;
        bool hasPOpp(const PlaneOpp *pPOpp) const;

        void setActive(bool b) const {m_bIsActive=b;}
        bool isActive() const {return m_bIsActive;}

        bool bAutoInertia() const {return m_bAutoInertia;}
        void setAutoInertia(bool bFromParts) {m_bAutoInertia=bFromParts;}


    protected:
        bool m_bIsInitialized;
        bool m_bAutoInertia;

        Inertia m_Inertia;


        mutable bool m_bLocked;  /**< true if the object instance is used by an running analysis; not stored. */
        mutable bool m_bIsActive; /**< convenience field used in batch analyses; not stored. */

    public:

};


