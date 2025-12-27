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


#include <fusexfl.h>


class FL5LIB_EXPORT FuseFlatFaces : public FuseXfl
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
        int makeShape(std::string &log) override;
        int makeSurfaceTriangulation(int axialres, int hoopres) override;

};

