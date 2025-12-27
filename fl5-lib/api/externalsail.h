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

#include <sail.h>
#include <triangle3d.h>

class FL5LIB_EXPORT ExternalSail : public Sail
{
    friend class  ExternalSailDlg;

    public:
        ExternalSail();
        bool isNURBSSail()  const override {return false;}
        bool isSplineSail() const override {return false;}
        bool isWingSail()   const override {return false;}
        double area() const override { return m_WettedArea;}
        double luffLength()  const override {return m_Tack.distanceTo(m_Head);}
        double leechLength() const override {return m_Clew.distanceTo(m_Peak);}
        double footLength()  const override {return m_Clew.distanceTo(m_Tack);}
        Vector3d leadingEdgeAxis() const override {return (m_Head-m_Tack).normalized();}

        void duplicate(Sail const*pSail) override;


        void createSection(int ) override {}
        void deleteSection(int ) override {}

        void computeProperties();
        double size() const override;

        virtual void rotate(const Vector3d &origin, const Vector3d &axis, double theta) = 0;

    protected:
        double m_Lx, m_Ly, m_Lz;
        double m_WettedArea;
};


