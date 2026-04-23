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


#include <QApplication>

#include "glxplanebuffers.h"

#include <api/vorton.h>

XPlane *glXPlaneBuffers::s_pXPlane = nullptr;

glXPlaneBuffers::glXPlaneBuffers()
{
    m_LiveAlpha = 0.0;
}


glXPlaneBuffers::~glXPlaneBuffers()
{
    clearBuffers();
}


void glXPlaneBuffers::clearBuffers()
{
    for(int iWing=0; iWing<m_vboWingSurface.size(); iWing++)
    {
        if(m_vboWingSurface[iWing].isCreated()) m_vboWingSurface[iWing].destroy();
    }
    m_vboWingSurface.clear();
    for(int iWing=0; iWing<m_vboWingOutline.size(); iWing++)
    {
        if(m_vboWingOutline[iWing].isCreated()) m_vboWingOutline[iWing].destroy();
    }

    for(int iFuse=0; iFuse<m_vboFuseTriangulation.size(); iFuse++)
    {
        if(m_vboFuseTriangulation[iFuse].isCreated()) m_vboFuseTriangulation[iFuse].destroy();
    }
    m_vboFuseTriangulation.clear();

    for(int iFuse=0; iFuse<m_vboBodyOutline.size(); iFuse++)
    {
        if(m_vboBodyOutline[iFuse].isCreated()) m_vboBodyOutline[iFuse].destroy();
    }

    m_vboWingOutline.clear();

    if( m_vboGridVelocities.isCreated())       m_vboGridVelocities.destroy();
    if(m_vboContourClrs.isCreated())           m_vboContourClrs.destroy();
    if(m_vboContourLines.isCreated())          m_vboContourLines.destroy();
    if(m_vboCp.isCreated())                    m_vboCp.destroy();
    if(m_vboDownwash.isCreated())                    m_vboDownwash.destroy();
    if(m_vboFrames.isCreated())                m_vboFrames.destroy();
    if(m_vboGamma.isCreated())                 m_vboGamma.destroy();
    if(m_vboGridVelocities.isCreated())        m_vboGridVelocities.destroy();
    if(m_vboInducedDrag.isCreated())           m_vboInducedDrag.destroy();
    if(m_vboMoments.isCreated())               m_vboMoments.destroy();
    if(m_vboNormals.isCreated())               m_vboNormals.destroy();
    if(m_vboPanelForces.isCreated())           m_vboPanelForces.destroy();
    if(m_vboPlaneStlTriangulation.isCreated()) m_vboPlaneStlTriangulation.destroy();
    if(m_vboWakePanels.isCreated())            m_vboWakePanels.destroy();
    if(m_vboWakeEdges.isCreated())             m_vboWakeEdges.destroy();
    if(m_vboStreamLines.isCreated())           m_vboStreamLines.destroy();
    if(m_vboStripLift.isCreated())             m_vboStripLift.destroy();
    if(m_vboTrans.isCreated())                 m_vboTrans.destroy();
    if(m_vboMesh.isCreated())                  m_vboMesh.destroy();
    if(m_vboViscousDrag.isCreated())           m_vboViscousDrag.destroy();
    if(m_vboVortices.isCreated())              m_vboVortices.destroy();
    if(m_vboVortonLines.isCreated())           m_vboVortonLines.destroy();
    if(m_vboVortons.isCreated())               m_vboVortons.destroy();
}


void glXPlaneBuffers::resizeFuseGeometryBuffers(int nFuse)
{
    // initialize buffer arrays
    for(int i=0; i<m_vboBodyOutline.size(); i++)     m_vboBodyOutline[i].destroy();
    m_vboBodyOutline.clear();
    m_vboBodyOutline.resize(nFuse);

    for(int i=0; i<m_vboFuseTriangulation.size(); i++)     m_vboFuseTriangulation[i].destroy();
    m_vboFuseTriangulation.clear();
    m_vboFuseTriangulation.resize(nFuse);
}

//void setLiveVortons(std::vector<std::vector<Vorton>> const &vortons);


void glXPlaneBuffers::setLiveVortons(double alpha, std::vector<std::vector<Vorton>> const &vortons)
{
    m_LiveAlpha = alpha;
    m_LiveVortons = vortons;
}


void glXPlaneBuffers::clearLiveVortons()
{
    m_LiveVortons.clear();
}





