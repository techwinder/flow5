/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QString>
#include <QSettings>

/**
 * Mesh parameters for OCC's Incremental mesh and Express Mesh
*/

class OccMeshParams
{
    public:
        OccMeshParams();
        void duplicate(OccMeshParams const &params);
        void setDefaults();
        void serializeParams(QDataStream &ar, bool bIsStoring);

        void setDefAbsolute(double deflection)             {m_LinDeflectionAbs = deflection;}
        void setDefRelative(double deflection)             {m_LinDeflectionRel = deflection;}
        void setAngularDeviation(double angledeviationDegree) {m_AngularDeviation=angledeviationDegree;}
        void setDefRelative(bool bRelative)                   {m_bLinDefAbs=!bRelative;}

        double deflectionAbsolute() const {return m_LinDeflectionAbs;}
        double deflectionRelative() const {return m_LinDeflectionRel;}
        double angularDeviation()   const {return m_AngularDeviation;}
        double maxElementSize()     const {return m_MaxElementSize;}
        bool isRelativeDeflection() const {return !m_bLinDefAbs;}

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        QString listParams(const QString &prefix);


    private:
        double m_LinDeflectionAbs, m_LinDeflectionRel;
        double m_AngularDeviation; // in degrees
        double m_MaxElementSize; // used to ensure compatibimity with legacy projects < beta 12
        bool m_bLinDefAbs;
};


