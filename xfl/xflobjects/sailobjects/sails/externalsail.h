/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflobjects/sailobjects/sails/sail.h>
#include <xflgeom/geom3d/triangle3d.h>

class ExternalSail : public Sail
{
    friend class ExternalSailDlg;

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


