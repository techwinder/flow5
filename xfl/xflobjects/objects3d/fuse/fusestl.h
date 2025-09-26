/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <xflobjects/objects3d/fuse/fuse.h>

class FuseStl : public Fuse
{
    public:
        FuseStl();


        Fuse* clone() const override {return new FuseStl(*this);}

        PART::enumPartType partType() const override {return PART::Fuse;}
        bool isWing() const override {return false;}
        bool isFuse() const override {return true;}
        bool isSail() const override {return false;}

        void makeFuseGeometry() override;

        bool isXflType()       const override {return false;}
        bool isFlatFaceType()  const override {return false;}
        bool isSplineType()    const override {return false;}
        bool isSectionType()    const override {return false;}
        bool isOccType()       const override {return false;}
        bool isStlType()       const override {return true;}

        bool serializePartFl5(QDataStream &ar, bool bIsStoring) override;

        void computeSurfaceProperties(QString &logmsg, const QString &prefix) override;
        void computeWettedArea() override ;

        int makeDefaultTriMesh(QString &logmsg, QString const&prefix) override;

        bool intersectFuse(Vector3d const &A, Vector3d const &B, Vector3d &I, bool bRightSide) const override;
        bool blockIntersect(int iBlock, Vector3d A, Vector3d B, Vector3d *I, bool *bIntersect) const;

        void scale(double XFactor, double YFactor, double ZFactor) override;
        void translate(Vector3d const &T) override;
        void rotate(Vector3d const &origin, Vector3d const &axis, double theta) override;

        int nPanel4() const override {return 0;}
};

