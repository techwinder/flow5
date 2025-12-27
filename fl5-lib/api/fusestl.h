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


#include <fuse.h>

class FL5LIB_EXPORT FuseStl : public Fuse
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

        void computeSurfaceProperties(std::string &msg, const std::string &prefx) override;
        void computeWettedArea() override ;
        void computeStructuralInertia(Vector3d const &PartPosition) override;

        int makeDefaultTriMesh(std::string &logmsg, const std::string &prefix) override;

        bool intersectFuse(Vector3d const &A, Vector3d const &B, Vector3d &I, bool bMultiThreaded) const override;

        void scale(double XFactor, double YFactor, double ZFactor) override;
        void translate(Vector3d const &T) override;
        void rotate(Vector3d const &origin, Vector3d const &axis, double theta) override;

        int nPanel4() const override {return 0;}
};

