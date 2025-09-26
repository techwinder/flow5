/****************************************************************************

	flow5 application

	Copyright (C) Andre Deperrois 

	All rights reserved.

*****************************************************************************/


#pragma once

#include <QString>
#include <xflgeom/geom3d/vector3d.h>




class PointMass
{
    public:
        /** The public constructor */
        PointMass() : m_Mass{0}
        {
        }
        /** Overloaded public constructor */
        PointMass(double m, Vector3d const &p, QString const &tag) : m_Mass{m}, m_Position{p}, m_Tag{tag}
        {
        }

        /** Returns the value of the mass */
        double mass() const  {return m_Mass;}
        void setMass(double mass){m_Mass = mass;}

        /** Returns the the position of the mass */
        const Vector3d & position() const {return m_Position;}
        void setPosition(Vector3d pos) {m_Position=pos;}
        void setXPosition(double x){m_Position.x=x;}
        void setYPosition(double y){m_Position.y=y;}
        void setZPosition(double z){m_Position.z=z;}


        /** Returns the the tag of the mass */
        const QString &tag() const {return m_Tag;}
        void setTag(QString const &tag) {m_Tag=tag;}

        bool isSame(PointMass const &pm, Vector3d const& translated) const
        {
            if(fabs(pm.mass()-m_Mass)>0.001) return false;
            if(!m_Position.isSame(pm.position()+translated, 0.001)) return false;
            return true;
        }

    private:
        double m_Mass;          /**< the value of the point mass, in kg */
        Vector3d m_Position;      /**< the absolute position of the point mass */
        QString m_Tag;           /**< the description of the point mass */

};



