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
#include <TopoDS_Shell.hxx>



#include <enums_objects.h>
#include <occmeshparams.h>
#include <part.h>

#include <fl5lib_global.h>


class Panel3;
class Panel4;
class PlanePolar;


class FL5LIB_EXPORT Fuse : public Part
{
    public:

        enum enumType {FlatFace, NURBS, Sections, Occ, Stl, Other};


    public:
        Fuse();
        Fuse(const Fuse &fuse);

        void computeAero(const std::vector<Panel4> &panel4, double const *Cp, const PlanePolar *pWPolar, double Alpha, Vector3d &CP, Vector3d &Force, Vector3d &Moment) const;
        void computeAero(std::vector<Panel3> const &panel3, double const *Cp3Vtx, const PlanePolar *pWPolar, double Alpha, Vector3d &CP, Vector3d &Force, Vector3d &Moment) const;
        void computeViscousForces(const PlanePolar *pWPolar, double Alpha, double QInf, Vector3d &Force, Vector3d &Moment) const;

        virtual Fuse* clone() const = 0;
        virtual void duplicate(const Fuse &aFuse);

        virtual void makeFuseGeometry();
        virtual bool intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I) const;
        virtual bool intersectFuse(const Vector3d &A, const Vector3d &B, Vector3d &I, bool bRightSide) const;

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

        NCollection_List<TopoDS_Shape> &shells() {return m_Shell;}
        NCollection_List<TopoDS_Shape> const &shells() const {return m_Shell;}


        //	Methods related to the cut shells
        void setShells(const NCollection_List<TopoDS_Shape> & shells) {m_Shell=shells;}
        void clearShells() {m_Shell.Clear();}
        int shellCount() const {return m_Shell.Extent();}
        int nShells() const {return m_Shell.Extent();}
        void appendShell(TopoDS_Shape const &shell) {m_Shell.Append(shell);}
        bool stitchShells(TopoDS_Shell &fusedshell, std::string &logmsg, std::string prefx);
        void clearOccTriangulation();

        // Methods related to geometry
        double wettedArea() const {return m_WettedArea;}
        double formFactor() const;

        double maxWidth() const  {return m_MaxWidth;}
        void setMaxWidth(double w) {m_MaxWidth=w;}

        double maxHeight() const {return m_MaxHeight;}
        void setMaxHeight(double h) {m_MaxHeight=h;}

        double maxFrameArea() const {return m_MaxFrameArea;}


        void translateTriPanels(Vector3d const &T) {translateTriPanels(T.x, T.y, T.z);}
        void translateTriPanels(double tx, double ty, double tz);

        // Methods related to meshing
        virtual int makeDefaultTriMesh(std::string &logmsg, const std::string &prefix);

        int nPanel4() const override = 0;

        virtual void computeStructuralInertia(Vector3d const &PartPosition) override;
        virtual void computeSurfaceProperties(std::string &log, std::string const &prefix) = 0;
        virtual void computeWettedArea();

        virtual void getProperties(std::string &properties, const std::string &prefx, bool bFull=false);

        OccMeshParams &occTessParams() {return m_OccTessParams;}
        OccMeshParams const &occTessParams() const {return m_OccTessParams;}
        void setOccTessParams(OccMeshParams const &params) {m_OccTessParams=params;}

        double maxElementSize() const {return m_MaxElementSize;}
        void setMaxElementSize(double l) {m_MaxElementSize=l;}

        static Fuse::enumType bodyPanelType(std::string strPanelType);
        static std::string bodyPanelType(Fuse::enumType panelType);

        void listShells();

    protected:

        Fuse::enumType m_FuseType; /** @todo useless now that each fuse is defined in its own subclass - remove */

        NCollection_List<TopoDS_Shape> m_Shell;  /** The list of shells AFTER the cut operation. Used for display and mesh generation.*/


        double m_WettedArea;
        double m_MaxWidth;
        double m_MaxHeight;
        double m_MaxFrameArea;

        Vector3d m_Force;     /** The force on the fUSE (N/q) */
        Vector3d m_Mi;        /** The moments of induced or pressure forces (N.m/q) w.r.t the polar's ref CoG */
        Vector3d m_CP;        /**< the centre of pressure's position */

        double m_MaxElementSize; /** used by the flow5 mesher */

        OccMeshParams m_OccTessParams; /** @todo move into Part class with serialization format change */

    protected:
        // temp variables;
        mutable int m_nBlocks;

};



