/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xfloptim/tasks/psotask.h>


class PlaneXfl;
class WPolar;

#define NPLANEFIELDS 4        // aoa, mass, cog.x, cog.z
#define NWINGFIELDS 14        // LEx3, Rx3, Span, chord, sweep, twist, area, AR, TR, sections
#define NSECTIONFIELDS  5     // y, chord, offset, dihedral, twist

class PSOTaskPlane : public PSOTask
{
    friend class Optim3d;
    friend class OptimPlane;
    friend class OptimCp;

    public:
        PSOTaskPlane();
        void setPlane(PlaneXfl const*pPlaneXfl) {m_pPlaneXfl=pPlaneXfl;}

        static WPolar &staticPolar() {return s_WPolar;}
        static void setStaticPolar(WPolar const &wpolar);

    private:
        void calcFitness(Particle *pParticle, bool bLong=false, bool bTrace=false) const override;

        static WPolar s_WPolar;
        PlaneXfl const *m_pPlaneXfl;

};


void makePSOPlane(const Particle *pParticle, const PlaneXfl *baseplanexfl, PlaneXfl *pPlaneXfl);
