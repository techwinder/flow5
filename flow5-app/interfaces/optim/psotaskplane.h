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

#include <interfaces/optim/psotask.h>


class PlaneXfl;
class PlanePolar;

#define NPLANEFIELDS 4        // aoa, mass, cog.x, cog.z
#define NWINGFIELDS 14        // LEx3, Rx3, Span, chord, sweep, twist, area, AR, TR, sections
#define NSECTIONFIELDS  5     // y, chord, offset, dihedral, twist

class PSOTaskPlane : public PSOTask
{
    friend class  Optim3d;
    friend class  OptimPlane;
    friend class  OptimCp;

    public:
        PSOTaskPlane();
        void setPlane(PlaneXfl const*pPlaneXfl) {m_pPlaneXfl=pPlaneXfl;}

        static PlanePolar &staticPolar() {return s_WPolar;}
        static void setStaticPolar(PlanePolar const &wpolar);

    private:
        void calcFitness(Particle *pParticle, bool bLong=false, bool bTrace=false) const override;

        static PlanePolar s_WPolar;
        PlaneXfl const *m_pPlaneXfl;

};


void makePSOPlane(const Particle *pParticle, const PlaneXfl *baseplanexfl, PlaneXfl *pPlaneXfl);
