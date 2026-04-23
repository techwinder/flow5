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

#include <sail.h>
#include <nurbssurface.h>
#include <triangle3d.h>

class Geom_BSplineSurface;

class FL5LIB_EXPORT SailNurbs : public Sail
{
    friend class  SailDlg;
    friend class  NURBSSailDlg;
    friend class  XmlBoatReader;

    public:
        SailNurbs();

        bool isNURBSSail()  const override {return true;}
        bool isSplineSail() const override {return false;}
        bool isOccSail()    const override {return false;}
        bool isWingSail()   const override {return false;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new SailNurbs(*this);}

        void resizeSections(int nSections, int nPoints) override;
        void makeDefaultSail() override;

        void createSection(int iSection) override;
        void deleteSection(int iSection) override;

        bool serializeSailFl5(QDataStream &ar, bool bIsStoring) override;

        double length() const override;

        Node edgeNode(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const override;
        Vector3d point(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const override;
        Vector3d normal(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const override;

        void setColor(fl5Color const &clr) override;

        void duplicate(Sail const *pSail) override;

        void flipXZ() override;
        void translate(Vector3d const &T) override;

        void scale(double XFactor, double YFactor, double ZFactor) override;
        void scaleTwist(double newtwist) override;
        void scaleAR(   double newAR)    override;

        void makeSurface() override;

        double luffLength() const override;
        double leechLength() const override;
        double footLength() const override;
        double twist() const override;
        Vector3d leadingEdgeAxis() const override {return m_nurbs.leadingEdgeAxis();}

        bool intersect(const Vector3d &A, const Vector3d &B, Vector3d &I, Vector3d &N) const override;

        bool makeOccShell(TopoDS_Shape &sailshape, std::string &tracelog) const override;

        double chord(double zrel) const ;

        NURBSSurface const &nurbs() const {return m_nurbs;}
        NURBSSurface &nurbs() {return m_nurbs;}
        NURBSSurface *pNurbs() {return &m_nurbs;}

        int sectionCount() const override {return m_nurbs.frameCount();}

        int activeFrameIndex() const  {return m_nurbs.activeFrameIndex();}
        Frame &activeFrame() {return m_nurbs.activeFrame();}

        Frame &frame(int iFrame) {return m_nurbs.frame(iFrame);}
        Frame const&frameAt(int iFrame) const {return m_nurbs.frameAt(iFrame);}

        void setActiveFrameIndex(int iFrame) {return m_nurbs.setActiveFrameIndex(iFrame);}
        void removeActiveFrame() {m_nurbs.removeActiveFrame();}

        int frameCount()      const {return m_nurbs.frameCount();}
        int framePointCount() const {return m_nurbs.framePointCount();}
        int sideLineCount()   const {return m_nurbs.framePointCount();}// same as FramePointCount();
        double framePosition(int iFrame) const;

        double leadingAngle(int iSection) const;
        double trailingAngle(int iSection) const;


    private:
        NURBSSurface m_nurbs;


};

