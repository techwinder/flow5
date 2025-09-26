/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <xflobjects/objects3d/fuse/fusexfl.h>


class FuseFlatFaces : public FuseXfl
{
    public:
        FuseFlatFaces();
        FuseFlatFaces(FuseXfl const& fusexfl);

    public:
        bool isFlatFaceType()  const override {return true;}
        bool isSplineType()    const override {return false;}
        bool isSectionType()   const override {return false;}
        void makeDefaultFuse() override;

        int makeQuadMesh(int idx0, const Vector3d &pos) override;

        Fuse* clone() const override {return new FuseFlatFaces(*this);}

        int quadCount() const override;
        int makeShape(QString &log) override;
        int makeSurfaceTriangulation(int axialres, int hoopres) override;

};

