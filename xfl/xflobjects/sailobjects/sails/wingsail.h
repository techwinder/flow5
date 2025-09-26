/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <xflobjects/sailobjects/sails/sail.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflobjects/objects3d/wing/surface.h>
#include <xflobjects/sailobjects/sails/wingsailsection.h>


class WingSail : public Sail
{
    public:
        WingSail();

        // overrides
        bool isNURBSSail()  const override {return false;}
        bool isSplineSail() const override {return false;}
        bool isOccSail()    const override {return false;}
        bool isWingSail() const override {return true;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new WingSail(*this);}


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

        void properties(QString &props, QString const &frontspacer, bool bFull) const override;

        void clearSections() {m_SailSection.clear();}
        void appendNewSection();
        int sectionCount() const override {return m_SailSection.size();}
        WingSailSection const &sectionAt(int iSection) const {return m_SailSection.at(iSection);}
        WingSailSection &section(int iSection) {return m_SailSection[iSection];}

        WingSailSection const &rootSection() const {return m_SailSection.first();}
        WingSailSection const &gaffSection() const {return m_SailSection.last();}
        WingSailSection &rootSection() {return m_SailSection.first();}
        WingSailSection &gaffSection() {return m_SailSection.last();}

        Vector3d sectionPosition(int idx) const {if(idx>=0&&idx<m_SectionPos.size()) return m_SectionPos[idx]; else return Vector3d();}
        double xPosition(int idx) const {return m_SectionPos.at(idx).x;}
        double zPosition(int idx) const {return m_SectionPos.at(idx).z;}

        void setSectionPosition(int idx, Vector3d pos) {if(idx>=0&&idx<m_SectionPos.size())  m_SectionPos[idx]=pos;}
        void setXPosition(int idx, double x) {if(idx>=0&&idx<m_SectionPos.size())  m_SectionPos[idx].x=x;}
        void setZPosition(int idx, double z) {if(idx>=0&&idx<m_SectionPos.size())  m_SectionPos[idx].z=z;}


        double sectionAngle(int idx) const {if(idx>=0&&idx<m_Ry.size()) return m_Ry[idx]; else return 0.0;}
        void setSectionAngle(int idx, double angle) {if(idx>=0&&idx<m_Ry.size()) m_Ry[idx]=angle;}

        int activeSection() const {return m_iActiveSection;}
        void setActiveSection(int is) {m_iActiveSection=is;}

        void clearSurfaces() {m_Surface.clear();}
        void computeChords();

        int nStations() const override {return m_NStation;}

        static double minSurfaceLength(){return s_MinSurfaceLength;}
        static void setMinSurfaceLength(double size){s_MinSurfaceLength=size;}

        // ------------ Variables --------------------
    private:

        int m_iActiveSection;
        int m_NStation;

        QVector<double> m_Ry;
        QVector<Surface> m_Surface; /**< the array of Surface objects associated to the WingSail */
        QVector<WingSailSection> m_SailSection;
        QVector<Vector3d> m_SectionPos;

        static double s_MinSurfaceLength;      /**< panels of less length are ignored */

};


