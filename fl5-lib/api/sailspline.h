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


#include <sail.h>
#include <spline.h>
#include <triangle3d.h>
#include <bspline3d.h>


class FL5LIB_EXPORT SailSpline : public Sail
{
    public:

        SailSpline(Spline::enumType type = Spline::BSPLINE);
        SailSpline(SailSpline const &ssail);
        ~SailSpline();

        Spline::enumType splineType() const {return m_SplineType;}
        void setSplineType(Spline::enumType type) {m_SplineType=type;} // use with caution

        bool isNURBSSail()  const override {return false;}
        bool isSplineSail() const override {return true;}
        bool isOccSail()    const override {return false;}
        bool isWingSail()   const override {return false;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new SailSpline(*this);}

        void convertSplines(Spline::enumType newtype);


        void resizeSections(int nSections, int nPoints) override;
        void makeDefaultSail() override;
        void makeSurface() override;

        void createSection(int iSection) override;
        void deleteSection(int iSection) override;
        void insertSection(int is, Spline *pSpline, Vector3d position, double ry);

        void flipXZ() override;
        void scale(double XFactor, double YFactor, double ZFactor) override;
        void translate(Vector3d const &T) override;
        void scaleTwist(double newtwist) override;
        void scaleAR(   double newAR)    override;

        bool serializeSailFl5(QDataStream &ar, bool bIsStoring) override;

        Vector3d point(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const override;

        void duplicate(Sail const *pSail) override;
        void setColor(fl5Color const &clr) override;

        double luffLength() const override;
        double leechLength() const override;
        double footLength() const override;
        double twist() const override;

        double length() const override;

        Vector3d leadingEdgeAxis() const override;
        double leadingAngle(int iSection) const;
        double trailingAngle(int iSection) const;

        int sectionCount() const override {return int(m_Spline.size());}
        void clearSections();

        Vector3d sectionPosition(int idx) const {if(idx>=0&&idx<int(m_Position.size())) return m_Position[idx]; else return Vector3d();}
        void setSectionPosition(int idx, Vector3d pos) {if(idx>=0&&idx<int(m_Position.size()))  m_Position[idx]=pos;}
        void setZPosition(int idx, double z) {if(idx>=0&&idx<int(m_Position.size()))  m_Position[idx].z=z;}

        double sectionAngle(int idx) const {if(idx>=0&&idx<int(m_Ry.size())) return m_Ry[idx]; else return 0.0;}
        void setSectionAngle(int idx, double angle) {if(idx>=0&&idx<int(m_Ry.size())) m_Ry[idx]=angle;}

        Spline *firstSpline() const {if(int(m_Spline.size())>0) return m_Spline.front(); else return nullptr;}
        Spline *lastSpline() const {if(int(m_Spline.size())>0) return m_Spline.back(); else return nullptr;}

        void appendSpline(Spline *pSpline, Vector3d Pos, double Ry);
        Spline *spline(int idx) {if(idx>=0&&idx<sectionCount()) return m_Spline[idx]; else return nullptr;}
        Spline const *splineAt(int idx) const {if(idx>=0&&idx<sectionCount()) return m_Spline[idx]; else return nullptr;}
        void setSpline(int idx, Spline *pSpline) {if(idx>=0&&idx<int(m_Spline.size())) m_Spline[idx]=pSpline;}

        int activeSection() const {return m_iActiveSection;}
        void setActiveSection(int is) {m_iActiveSection=is;}
        Spline *activeSpline() {if(m_iActiveSection>=0&&m_iActiveSection<sectionCount()) return m_Spline[m_iActiveSection]; else return nullptr;}
        Spline const *activeSpline() const {if(m_iActiveSection>=0&&m_iActiveSection<sectionCount()) return m_Spline[m_iActiveSection]; else return nullptr;}
        void updateActiveSpline();

        bool makeOccShell(TopoDS_Shape &sailshape, std::string &tracelog) const override;
        bool makeBSpline3d(int ispl, BSpline3d &spline3d) const;

        Vector3d sectionPoint(int iSection, int iPt) const;


    private:
        Spline::enumType m_SplineType;


        int m_iActiveSection;

        std::vector<Spline*> m_Spline;
        std::vector<Vector3d> m_Position; /** @todo only z-component is neeeded */
        std::vector<double> m_Ry;
};


