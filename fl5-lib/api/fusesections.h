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


#include <fusexfl.h>

class FL5LIB_EXPORT FuseSections : public FuseXfl
{
    public:
        FuseSections();

        Fuse* clone() const override {return new FuseSections(*this);}

        bool isSectionType() const override {return true;}

        void setNSections(int nsections);
        void setNPoints(int npts);

        int insertFrame(const Vector3d &Real) override;
        int insertFrameBefore(int iFrame) override;
        int insertFrameAfter(int iFrame) override;
        int removeFrame(int n) override {m_Section.erase(m_Section.begin()+n); return n-1;}
        int makeQuadMesh(int idx0, Vector3d const &pos) override;
        void makeDefaultFuse() override;
        void makeNURBS() override;
        void makeFuseGeometry() override;
        bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;
        void scale(double XFactor, double YFactor, double ZFactor) override;
        void scaleFrame(double YFactor, double ZFactor, int FrameID=0) override;
        void translateFrame(Vector3d T, int FrameID) override;
        void translate(Vector3d const &T) override;
        void duplicateFuse(const Fuse &aFuse) override;
        void insertPoint(int iPt) override;
        void insertPoint(const Vector3d &Real) override;
        void removeSideLine(int SideLine) override;
        double length() const override;
        double height() const override;

        std::vector<Vector3d> &section(int k) {return m_Section[k];}
        std::vector<Vector3d> const &sectionAt(int k) const {return m_Section.at(k);}
        int nSections() const {return int(m_Section.size());}
        int pointCount() const {if(m_Section.size()==0) return 0; else return int(m_Section.front().size());}
        Vector3d const &point(int isec, int ipt) const {return m_Section.at(isec).at(ipt);}
        void setPoint(int isec, int ipt, Vector3d const &pt);

        int isSection(const Vector3d &Real, double deltaX, double deltaZ) const;
        void setHighlightedSection(int iHigh) const {m_iHighlightSection=iHigh;}
        int highlightedSectionIndex() const {return m_iHighlightSection;}

        int isPoint(const Vector3d &Point, double deltax, double deltay, double deltaz) const;

        bool hasSections()                        const {return m_Section.size()>0;}
        bool hasActiveSection()                   const {return m_iActiveSection>=0 && m_iActiveSection<nSections();}
        void setActiveSectionIndex (int isection) const {m_iActiveSection = isection;}
        int activeSectionIndex()                  const {return m_iActiveSection;}
        std::vector<Vector3d> const & activeSection() const {return m_Section.at(m_iActiveSection);}
        std::vector<Vector3d> & activeSection() {return m_Section[m_iActiveSection];}

        void setActiveSectionPosition(double x, double z);
        void setSectionXPosition(int isec, double x);

        bool hasPoints()                  const {return m_Section.size()>0 && m_Section.front().size()>0;}
        bool hasActivePoint()             const;
        int activePointIndex()            const {return m_iActivePoint;}
        void setActivePoint(int iPt)      const {m_iActivePoint=iPt;}
        int highlightedPointIndex()       const {return m_iHighlightedPoint;}
        void setHighlightedPoint(int ipt) const {m_iHighlightedPoint=ipt;}
        void setSelectedPoint(Vector3d const &pt);      

        double fitPrecision() const {return m_FitPrecision;}
        void setFitPrecision(double precision) {m_FitPrecision=precision;}

    private:
        std::vector<std::vector<Vector3d>> m_Section; /**< the surface  points on which the nurbs will be interpolated */

        double m_FitPrecision;

        mutable int m_iHighlightSection;
        mutable int m_iActiveSection;

        mutable int m_iHighlightedPoint;
        mutable int m_iActivePoint;
};


