/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflobjects/sailobjects/sails/externalsail.h>
#include <xflgeom/geom3d/triangle3d.h>

class STLSail : public ExternalSail
{
    friend class SailDlg;
    friend class STLSailDlg;

    public:
        STLSail();

        void setTriangles(QVector<Triangle3d> const &triangles);

        bool isOccSail()    const override {return false;}
        bool isStlSail()    const override {return true;}

        Sail* clone() const override {return new STLSail(*this);}


        void resizeSections(int , int ) override {}
        void makeDefaultSail() override {}

        void scale(double XFactor, double YFactor, double ZFactor) override;
        void translate(Vector3d const &T) override;
        void rotate(const Vector3d &origin, const Vector3d &axis, double theta) override;
        void flipXZ() override;

        void scaleTwist(double newtwist) override;
        void scaleAR(   double newAR)    override;

        Vector3d point(double , double , xfl::enumSurfacePosition) const override {return Vector3d();}


        void makeSurface() override {}

        bool serializeSailFl5(QDataStream &ar, bool bIsStoring) override;


        void makeTriangulation(int nx=s_iXRes, int nz=s_iZRes) override;

        void properties(QString &props, QString const &prefix, bool bFull=false) const override;
};

