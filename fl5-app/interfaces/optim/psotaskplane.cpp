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

#define _MATH_DEFINES_DEFINED


#include <interfaces/optim/psotaskplane.h>
#include <api/panelanalysis.h>
#include <api/planetask.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/planexfl.h>


PlanePolar PSOTaskPlane::s_WPolar;


PSOTaskPlane::PSOTaskPlane() : PSOTask()
{
}


void PSOTaskPlane::setStaticPolar(PlanePolar const &wpolar)
{
    s_WPolar.duplicateSpec(&wpolar);
}


void makePSOPlane(Particle const *pParticle, PlaneXfl const*pBasePlaneXfl, PlaneXfl *pPlaneXfl)
{
    pPlaneXfl->duplicate(pBasePlaneXfl);

//    Q_ASSERT(pParticle->dimension()==NPLANEFIELDS+pBasePlaneXfl->nWings()*NWINGFIELDS);

    int iv = 0; // 0 is aoa,
    pPlaneXfl->setAutoInertia(false);
    pPlaneXfl->clearPointMasses();
    pPlaneXfl->setStructuralMass(pParticle->pos(iv+1));
    pPlaneXfl->setCoG_s({pParticle->pos(iv+2), 0.0, pParticle->pos(iv+3)});
    iv = NPLANEFIELDS;

    for(int iw=0; iw<pBasePlaneXfl->nWings(); iw++)
    {
        WingXfl const*pRefWing = pBasePlaneXfl->wingAt(iw);
        WingXfl *pWing = pPlaneXfl->wing(iw);
        pWing->setPosition(pParticle->pos(iv+0), pParticle->pos(iv+1), pParticle->pos(iv+2));
        pWing->setRx(pParticle->pos(iv+3));
        pWing->setRy(pParticle->pos(iv+4));
        pWing->setRz(pParticle->pos(iv+5));

        bool bScaling(false);

        if(fabs(pParticle->pos(iv+ 6)-pRefWing->planformSpan())>DELTAVAR)
        {
            pWing->scaleSpan( pParticle->pos(iv+ 6));
            bScaling = true;
        }
        if(fabs(pParticle->pos(iv+ 7)-pRefWing->rootChord()   )>DELTAVAR)
        {
            pWing->scaleChord(pParticle->pos(iv+ 7));
            bScaling = true;
        }
        if(fabs(pParticle->pos(iv+ 8)-pRefWing->averageSweep())>DELTAVAR)
        {
            pWing->scaleSweep(pParticle->pos(iv+ 8));
            bScaling = true;
        }
        if(fabs(pParticle->pos(iv+ 9)-pRefWing->twist()       )>DELTAVAR)
        {
            pWing->scaleTwist(pParticle->pos(iv+ 9));
            bScaling = true;
        }
        if(fabs(pParticle->pos(iv+10)-pRefWing->planformArea())>DELTAVAR)
        {
            pWing->scaleArea( pParticle->pos(iv+10));
            bScaling = true;
        }
        if(fabs(pParticle->pos(iv+11)-pRefWing->aspectRatio() )>DELTAVAR)
        {
            pWing->scaleAR(   pParticle->pos(iv+11));
            bScaling = true;
        }
        if(fabs(pParticle->pos(iv+12)-pRefWing->taperRatio()  )>DELTAVAR)
        {
            pWing->scaleTR(   pParticle->pos(iv+12));
            bScaling = true;
        }
        iv += NWINGFIELDS-1;

        for(int isec=0; isec<pRefWing->nSections(); isec++)
        {
            WingSection &sec = pWing->section(isec);
            if(!bScaling)
            {
                sec.setYPosition(pParticle->pos(iv+0));
                sec.setChord(    pParticle->pos(iv+1));
                sec.setXOffset(   pParticle->pos(iv+2));
                sec.setDihedral( pParticle->pos(iv+3));
                sec.setTwist(    pParticle->pos(iv+4));
            }
            iv += NSECTIONFIELDS;
        }

        pWing->computeGeometry();
    }
}


void PSOTaskPlane::calcFitness(Particle *pParticle, bool bLong, bool bTrace) const
{
    (void)bLong;
    PlaneXfl *pPlaneXfl = new PlaneXfl;
    makePSOPlane(pParticle, m_pPlaneXfl, pPlaneXfl);

    PanelAnalysis::setMultiThread(false);

    PlaneTask *pTask = new PlaneTask;
    Task3d::setCancelled(false);
    TriMesh::setCancelled(false);
    pTask->setAnalysisStatus(xfl::RUNNING);

    PlanePolar wpolar;
    wpolar.duplicateSpec(&s_WPolar);
    wpolar.setMass(pParticle->pos(1));
    wpolar.setAutoInertia(true); //since the CoG is set as a plane variable
    wpolar.setReferenceChordLength(pPlaneXfl->mac());
    wpolar.setReferenceArea(pPlaneXfl->projectedArea(false));
    wpolar.setReferenceSpanLength(pPlaneXfl->projectedSpan());

    pPlaneXfl->makePlane(wpolar.bThickSurfaces(), true, wpolar.isTriangleMethod());
    pTask->setObjects(pPlaneXfl, &wpolar);

    std::vector<double> opplist{pParticle->pos(0)}; // first coordinate is the aoa
    pTask->setOppList(opplist);


    pTask->run();
    if(pTask->planeOppList().size()!=0)
    {
        PlaneOpp *pPOpp = pTask->planeOppList().front();
        for(int iobj=0; iobj<pParticle->nObjectives(); iobj++)
        {
            OptObjective const &obj = m_Objective.at(iobj);
            switch(obj.m_Index)
            {
                default:
                case 0: //Cl
                    pParticle->setFitness(iobj, pPOpp->aeroForces().CL());
                    break;
                case 1: //Cd
                    pParticle->setFitness(iobj, pPOpp->aeroForces().CD());
                    break;
                case 2: //Cl/Cd
                    pParticle->setFitness(iobj, pPOpp->aeroForces().CL()/pPOpp->aeroForces().CD());
                    break;
                case 3: //Cl(3/2)/Cd
                {
                    double Cl = pPOpp->aeroForces().CL();
                    double Cd = pPOpp->aeroForces().CD();
                    pParticle->setFitness(iobj, sqrt(Cl*Cl*Cl/(Cd*Cd)));
                    break;
                }
                case 4: //Cm
                    pParticle->setFitness(iobj, pPOpp->aeroForces().Cm());
                    break;
                case 5: //m.g.Vz
                    pParticle->setFitness(iobj, pPOpp->aeroForces().Cm());
                    break;
                case 6: //bending moment
                    if(pPOpp->hasWOpp())
                        pParticle->setFitness(iobj, pPOpp->WOpp(0).m_MaxBending);
                    else
                        pParticle->setFitness(iobj, LARGEVALUE);
                    break;
            }
        }
        delete pPOpp;
    }

    delete pPlaneXfl;
    delete pTask;

    if(bTrace) postParticleEvent();
}




