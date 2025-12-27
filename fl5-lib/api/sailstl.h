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

#include <externalsail.h>
#include <triangle3d.h>

class FL5LIB_EXPORT SailStl : public ExternalSail
{
    friend class  SailDlg;
    friend class  STLSailDlg;

    public:
        SailStl();

        bool isOccSail()    const override {return false;}
        bool isStlSail()    const override {return true;}

        Sail* clone() const override {return new SailStl(*this);}


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
        void setTriangles(std::vector<Triangle3d> const &triangles) override;

        void properties(std::string &sailprops, std::string const &prefx, bool bFull=false) const override;
};

