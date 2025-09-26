/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflobjects/objects3d/fuse/fusexfl.h>


class FuseNurbs : public FuseXfl
{
    public:
        FuseNurbs();

    public:
        bool isSplineType()    const override {return true;}
        int makeQuadMesh(int idx0, Vector3d const&pos) override;

        Fuse* clone() const override {return new FuseNurbs(*this);}

};


