/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QApplication>

#include "glxplanebuffers.h"

#include <xflpanels/panels/vorton.h>

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
    if(m_vboDW.isCreated())                    m_vboDW.destroy();
    if(m_vboFrames.isCreated())                m_vboFrames.destroy();
    if(m_vboGamma.isCreated())                 m_vboGamma.destroy();
    if(m_vboGridVelocities.isCreated())        m_vboGridVelocities.destroy();
    if(m_vboIndDrag.isCreated())               m_vboIndDrag.destroy();
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
    if(m_vboViscDrag.isCreated())              m_vboViscDrag.destroy();
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

void setLiveVortons(QVector<QVector<Vorton>> const &vortons);


void glXPlaneBuffers::setLiveVortons(double alpha, QVector<QVector<Vorton>> const &vortons)
{
    m_LiveAlpha = alpha;
    m_LiveVortons = vortons;
}


void glXPlaneBuffers::clearLiveVortons()
{
    m_LiveVortons.clear();
}





