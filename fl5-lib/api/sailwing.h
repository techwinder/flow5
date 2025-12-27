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


#include <sail.h>
#include <triangle3d.h>
#include <surface.h>
#include <wingsailsection.h>


class FL5LIB_EXPORT SailWing : public Sail
{
    public:
        SailWing();

        // overrides
        bool isNURBSSail()  const override {return false;}
        bool isSplineSail() const override {return false;}
        bool isOccSail()    const override {return false;}
        bool isWingSail() const override {return true;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new SailWing(*this);}


        void resizeSections(int nSections, int nPoints) override;
        void makeDefaultSail() override;

        void flipXZ() override;
        void scale(double XFactor, double YFactor, double ZFactor) override;
        void translate(Vector3d const &T) override;

        void duplicate(Sail const*pSail) override;

        double luffLength() const override;
        double leechLength() const override;
        double footLength() const override;
        double twist() const override;

        double area() const override;
        double length() const override;

        Vector3d point(double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const override;

        Vector3d surfacePoint(int iSection, double xrel, double zrel, xfl::enumSurfacePosition pos=xfl::MIDSURFACE) const;


        void makeSurface() override;
        bool serializeSailFl5(QDataStream &ar, bool bIsStoring) override;

        void createSection(int iSection) override;
        void deleteSection(int iSection) override;
        void insertSection(int is, WingSailSection &section, Vector3d position, double ry);

        Vector3d leadingEdgeAxis() const override;

        void makeTriangulation(int nx=s_iXRes, int nz=s_iZRes) override;

        void makeTriPanels(Vector3d const &Tack) override;

        void updateStations() override;

//        void properties(std::string &props, std::string const &frontspacer, bool bFull) const override;

        void clearSections() {m_Section.clear();}
        void appendNewSection();
        int sectionCount() const override {return int(m_Section.size());}
        WingSailSection const &sectionAt(int iSection) const {return m_Section.at(iSection);}
        WingSailSection &section(int iSection) {return m_Section[iSection];}

        WingSailSection const &rootSection() const {return m_Section.front();}
        WingSailSection const &gaffSection() const {return m_Section.back();}
        WingSailSection &rootSection() {return m_Section.front();}
        WingSailSection &gaffSection() {return m_Section.back();}

        Vector3d sectionPosition(int idx) const {if(idx>=0&&idx<int(m_Pos.size())) return m_Pos[idx]; else return Vector3d();}
        double xPosition(int idx) const {return m_Pos.at(idx).x;}
        double zPosition(int idx) const {return m_Pos.at(idx).z;}

        void setSectionPosition(int idx, Vector3d pos) {if(idx>=0&&idx<int(m_Pos.size()))  m_Pos[idx]=pos;}
        void setXPosition(int idx, double x) {if(idx>=0&&idx<int(m_Pos.size()))  m_Pos[idx].x=x;}
        void setZPosition(int idx, double z) {if(idx>=0&&idx<int(m_Pos.size()))  m_Pos[idx].z=z;}


        double sectionAngle(int idx) const {if(idx>=0&&idx<int(m_Ry.size())) return m_Ry[idx]; else return 0.0;}
        void setSectionAngle(int idx, double angle) {if(idx>=0&&idx<int(m_Ry.size())) m_Ry[idx]=angle;}

        int activeSection() const {return m_iActiveSection;}
        void setActiveSection(int is) {m_iActiveSection=is;}

        int nSurfaces() const {return int(m_Surface.size());}
        void clearSurfaces() {m_Surface.clear();}
        void computeChords();

        int nStations() const override {return m_NStation;}

        static double minSurfaceLength(){return s_MinSurfaceLength;}
        static void setMinSurfaceLength(double size){s_MinSurfaceLength=size;}

        // ------------ Variables --------------------
    private:

        int m_iActiveSection;
        int m_NStation;

        std::vector<double> m_Ry;
        std::vector<Surface> m_Surface; /**< the array of Surface objects associated to the SailWing */
        std::vector<WingSailSection> m_Section;
        std::vector<Vector3d> m_Pos;

        static double s_MinSurfaceLength;      /**< panels of less length are ignored */

};


