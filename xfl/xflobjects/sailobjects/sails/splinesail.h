/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QVector>


#include <xflobjects/sailobjects/sails/sail.h>
#include <xflgeom/geom2d/splines/spline.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflgeom/geom3d/bspline3d.h>


class SplineSail : public Sail
{
    friend class SplineSailDlg;

    public:

        SplineSail(Spline::enumType type = Spline::BSPLINE);
        SplineSail(SplineSail const &ssail);
        ~SplineSail();

        Spline::enumType splineType() const {return m_SplineType;}
        void setSplineType(Spline::enumType type) {m_SplineType=type;} // use with caution

        bool isNURBSSail()  const override {return false;}
        bool isSplineSail() const override {return true;}
        bool isOccSail()    const override {return false;}
        bool isWingSail()   const override {return false;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new SplineSail(*this);}

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
        void setColor(QColor clr) override;

        double luffLength() const override;
        double leechLength() const override;
        double footLength() const override;
        double twist() const override;

        double length() const override;

        Vector3d leadingEdgeAxis() const override;
        double leadingAngle(int iSection) const;
        double trailingAngle(int iSection) const;


        int sectionCount() const override {return m_Spline.size();}
        void clearSections();

        Vector3d sectionPosition(int idx) const {if(idx>=0&&idx<m_SectionPosition.size()) return m_SectionPosition[idx]; else return Vector3d();}
        void setSectionPosition(int idx, Vector3d pos) {if(idx>=0&&idx<m_SectionPosition.size())  m_SectionPosition[idx]=pos;}
        void setZPosition(int idx, double z) {if(idx>=0&&idx<m_SectionPosition.size())  m_SectionPosition[idx].z=z;}

        double sectionAngle(int idx) const {if(idx>=0&&idx<m_Ry.size()) return m_Ry[idx]; else return 0.0;}
        void setSectionAngle(int idx, double angle) {if(idx>=0&&idx<m_Ry.size()) m_Ry[idx]=angle;}

        Spline *firstSpline() const {if(m_Spline.size()>0) return m_Spline.first(); else return nullptr;}
        Spline *lastSpline() const {if(m_Spline.size()>0) return m_Spline.last(); else return nullptr;}

        void appendSpline(Spline *pSpline, Vector3d Pos, double Ry);
        Spline *spline(int idx) {if(idx>=0&&idx<sectionCount()) return m_Spline[idx]; else return nullptr;}
        Spline const *splineAt(int idx) const {if(idx>=0&&idx<sectionCount()) return m_Spline[idx]; else return nullptr;}
        void setSpline(int idx, Spline *pSpline) {if(idx>=0&&idx<m_Spline.size()) m_Spline[idx]=pSpline;}

        int activeSection() const {return m_iActiveSection;}
        void setActiveSection(int is) {m_iActiveSection=is;}
        Spline *activeSpline() {if(m_iActiveSection>=0&&m_iActiveSection<sectionCount()) return m_Spline[m_iActiveSection]; else return nullptr;}
        Spline const *activeSpline() const {if(m_iActiveSection>=0&&m_iActiveSection<sectionCount()) return m_Spline[m_iActiveSection]; else return nullptr;}
        void updateActiveSpline();

        bool makeOccShell(TopoDS_Shape &sailshape, QString &tracelog) const override;
        bool makeBSpline3d(int ispl, BSpline3d &spline3d) const;

    private:
        Spline::enumType m_SplineType;


        int m_iActiveSection;

        QVector<Spline*> m_Spline;
        QVector<Vector3d> m_SectionPosition; /** @todo only z-component is neeeded */
        QVector<double> m_Ry;
};


