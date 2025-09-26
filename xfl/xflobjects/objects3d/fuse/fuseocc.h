/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_ListOfShape.hxx>

#include <xflobjects/objects3d/fuse/fuse.h>


class FuseOcc : public Fuse
{
    public:
        FuseOcc();

        void computeSurfaceProperties(QString &logmsg, const QString &prefix) override;
        void computeStructuralInertia(Vector3d const &PartPosition) override;

        int nPanel4() const override {return 0;}
        PART::enumPartType partType() const override {return PART::Fuse;}
        bool isWing() const override {return false;}
        bool isFuse() const override {return true;}
        bool isSail() const override {return false;}

        bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;
        Fuse* clone() const override {return new FuseOcc(*this);}

        void translate(Vector3d const &T) override;
        void scale(double XFactor, double YFactor, double ZFactor) override;
        void rotate(Vector3d const &origin, Vector3d const &axis, double theta) override;


        bool isXflType()      const override {return false;}
        bool isFlatFaceType() const override {return false;}
        bool isSplineType()   const override {return false;}
        bool isSectionType()   const override {return false;}
        bool isOccType()      const override {return true;}
        bool isStlType()      const override {return false;}

        void getProperties(QString &props, QString const &prefix, bool bFull=false) override;

        void reverseFuse();

        void makeEdges(QVector<Segment3d> &lines);
        //	void makeSurfaceTriangulation();
        //	void TriangulateFace(TopoDS_Face const &face);
};







