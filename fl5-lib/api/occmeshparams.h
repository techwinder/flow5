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

#include <QDataStream>

#include <fl5lib_global.h>


class FL5LIB_EXPORT OccMeshParams
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

        std::string listParams(const std::string &prefix);


    private:
        double m_LinDeflectionAbs, m_LinDeflectionRel;
        double m_AngularDeviation; // in degrees
        double m_MaxElementSize; // used to ensure compatibimity with legacy projects < beta 12
        bool m_bLinDefAbs;
};


