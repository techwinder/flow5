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

#include <segment3d.h>


class FL5LIB_EXPORT Vortex : public Segment3d
{
    public:
    /** @enum The different models for the vortex viscous core */
    enum enumVortex {POTENTIAL, CUT_OFF, LAMB_OSEEN, RANKINE, SCULLY, VATISTAS};

    public:
        Vortex();
        Vortex(Node const &vtx0, Node const &vtx1, double gamma=0);

        void setCirculation(double gamma) {m_Circulation=gamma;}
        double circulation() const {return m_Circulation;}

        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        void getInducedVelocity(Vector3d const &C, Vector3d &vel, double coreradius, enumVortex vortexmodel=POTENTIAL) const;

        static void setCoreRadius(double CoreRadius) {s_CoreRadius=CoreRadius;}
        static double coreRadius() {return s_CoreRadius; }

        static enumVortex vortexModel() {return s_VortexModel;}
        static void setVortexModel(enumVortex vtxmodel) {s_VortexModel=vtxmodel;}


    private:
        double m_Circulation;

        static enumVortex s_VortexModel;
        static double s_CoreRadius;
};

FL5LIB_EXPORT Vector3d vortexInducedVelocity(Vector3d const &A, Vector3d const &B, Vector3d const &C, double coreradius, Vortex::enumVortex vortexmodel=Vortex::POTENTIAL);
FL5LIB_EXPORT double damp(double r, Vortex::enumVortex vortexmodel, double coreradius);
