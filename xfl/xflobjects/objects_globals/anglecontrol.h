/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QVector>
#include <QDataStream>

#include <xflgeom/geom_globals/geom_params.h>

class AngleControl
{
    public:
        AngleControl() {m_Name=QString();}
        AngleControl(QString const &name, QVector<double> const &values) {m_Name=name; m_Value=values;}

        inline void resizeValues(int n) {m_Value.resize(n);}
        inline void resetValues() {m_Value.fill(0);}

        inline QString const &name() const {return m_Name;}
        inline void setName(QString const &controlname) {m_Name=controlname;}

        inline int nValues() const {return m_Value.size();}

        inline double value(  int iCtrl) const {if (iCtrl<0 || iCtrl>=m_Value.size())   return 0.0; else return m_Value.at(iCtrl);}
        inline void setValue( int iCtrl, double g) {if (iCtrl>=0 && iCtrl<m_Value.size())   m_Value[iCtrl]=g;}
        inline void addValue(double g) {m_Value.append(g);}

        inline bool hasActiveAngle() const {for(int ig=0; ig<m_Value.size(); ig++) {if(fabs(m_Value.at(ig))>FLAPANGLEPRECISION) return true;} return false;}

        inline void serializeFl5(QDataStream &ar, bool bIsStoring)
        {
            int ArchiveFormat = 500750;
            int n=0;
            if(bIsStoring)
            {
                ar << ArchiveFormat;
                ar << m_Name;
                ar << int(m_Value.size());
                for(int ig=0; ig<m_Value.size(); ig++) ar<<m_Value.at(ig);
            }
            else
            {
                ar >> ArchiveFormat;
                ar >> m_Name;
                ar >> n;
                m_Value.resize(n);
                for(int ig=0; ig<m_Value.size(); ig++) ar>>m_Value[ig];
            }
        }

    private:

        QString m_Name;     /**< The control's name which serves as its unique identifier */
        QVector<double> m_Value; /**< The flap angles in degrees in the case of a T12358 polar; the gain angles in degrees in the case of a T7 polar.*/
};

