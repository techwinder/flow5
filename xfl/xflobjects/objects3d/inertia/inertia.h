/****************************************************************************

    flow5
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

#include <QVector>

#include<xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/inertia/pointmass.h>


class Inertia
{
    friend class InertiaDlg;
    friend class PartInertiaDlg;
    friend class PointMassTable;
    friend class XmlPlaneReader;
    friend class XmlPlaneWriter;
    friend class Part;
    friend class PartInertiaModel;
    friend class Plane;
    friend class WPolar;
    friend class XmlBoatReader;

    public:
        Inertia()
        {
            reset();
        }

        void reset()
        {
            m_Mass_s = 0.0;
            m_Ixx_s = m_Ixy_s = m_Ixz_s = 0.0;
            m_Iyy_s = m_Iyz_s = 0.0;
            m_Izz_s = 0.0;
            m_CoG_s.set(0.0,0.0,0.0);
            m_PointMass.clear();
        }

        void setInertiaStruct(double Ixx_s, double Iyy_s, double Izz_s, double Ixz_s)
        {
            m_Ixx_s = Ixx_s;
            m_Iyy_s = Iyy_s;
            m_Izz_s = Izz_s;
            m_Ixz_s = Ixz_s;
            m_Ixy_s = m_Ixz_s = 0.0;
            m_Iyz_s = 0.0;
        }

        /** I is a 3x3 tensor */
        void setInertiaStruct(double const *I)
        {
            m_Ixx_s = I[0];
            m_Ixy_s = I[1];
            m_Ixz_s = I[2];
            m_Iyy_s = I[4];
            m_Iyz_s = I[5];
            m_Izz_s = I[8];
        }

        double structuralMass() const {return m_Mass_s;}
        void setStructuralMass(double mass) {m_Mass_s=mass;}

        double totalMass() const
        {
            double total = m_Mass_s;
            for(int i=0; i<m_PointMass.size(); i++)  total += m_PointMass.at(i).mass();
            return total;
        }

        Vector3d CoG_t() const;

        // in CoG frame, structural part only
        Vector3d const &CoG_s() const {return m_CoG_s;}
        void setCoG_s(Vector3d const &cog_s) {m_CoG_s=cog_s;}
        void setCoG_s(double x, double y, double z) {m_CoG_s.set(x,y,z);}

        double Ixx_s() const {return m_Ixx_s;}
        double Ixy_s() const {return m_Ixy_s;}
        double Ixz_s() const {return m_Ixz_s;}
        double Iyy_s() const {return m_Iyy_s;}
        double Iyz_s() const {return m_Iyz_s;}
        double Izz_s() const {return m_Izz_s;}

        void setIxx_s(double Ixx) {m_Ixx_s=Ixx;}
        void setIxy_s(double Ixy) {m_Ixy_s=Ixy;}
        void setIxz_s(double Ixz) {m_Ixz_s=Ixz;}
        void setIyy_s(double Iyy) {m_Iyy_s=Iyy;}
        void setIyz_s(double Iyz) {m_Iyz_s=Iyz;}
        void setIzz_s(double Izz) {m_Izz_s=Izz;}

        // in CoG frame, including point masses
        double Ixx_t() const;
        double Ixy_t() const;
        double Ixz_t() const;
        double Iyy_t() const;
        double Iyz_t() const;
        double Izz_t() const;

        void setPointMasses(QVector<PointMass> PointMass){m_PointMass = PointMass;}
        void clearPointMasses() {m_PointMass.clear();}
        const QVector<PointMass> &pointMasses() const {return m_PointMass;}
        QVector<PointMass> &pointMasses() {return m_PointMass;}
        const PointMass &pointMassAt(int index) const {return m_PointMass.at(index);}
        PointMass &pointMass(int index) {return m_PointMass[index];}
        PointMass &lastPointMass() {return m_PointMass.last();}

        int pointMassCount() const {return m_PointMass.count();}
        void insertPointMass(int pos, PointMass const &pm) {m_PointMass.insert(pos, pm);}
        void appendPointMass(PointMass const &pm) {m_PointMass.append(pm);}
        void appendPointMass(double mass, Vector3d pos, QString tag) {m_PointMass.push_back({mass, pos, tag});}
        void removePointMass(int index) {m_PointMass.removeAt(index);}

        bool serializeFl5(QDataStream &ar, bool bIsStoring);
        QString toString() const;

        void scaleMasses(double scalefactor);

        void scaleMassPositions(double scalefactor);
        void translateMasses(Vector3d const & t);
        void rotateMasses(Vector3d const &O, Vector3d const &axis, double theta);


    private:
        double m_Ixx_s;    /**< The Ixx component of the structural part of the inertia tensor, w.r.t. the CoG origin */
        double m_Ixy_s;    /**< The Ixy component of the structural part of the inertia tensor, w.r.t. the CoG origin */
        double m_Ixz_s;    /**< The Ixz component of the structural part of the inertia tensor, w.r.t. the CoG origin */
        double m_Iyy_s;    /**< The Iyy component of the structural part of the inertia tensor, w.r.t. the CoG origin */
        double m_Iyz_s;    /**< The Iyy component of the structural part of the inertia tensor, w.r.t. the CoG origin */
        double m_Izz_s;    /**< The Izz component of the structural part of the inertia tensor, w.r.t. the CoG origin */

        Vector3d m_CoG_s;      /** the CoG position of the structural part of the inertia */

        QVector<PointMass> m_PointMass;

        double m_Mass_s;                       /**< the mass of the part's structure, excluding point masses */

};
