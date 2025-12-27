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



#include "gl3dparetoview.h"
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>

gl3dParetoView::gl3dParetoView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_bResetBox = true;
    m_X = m_Y = m_Z = 0.1;
    m_iBest = -1;

    setAxisTitle(0, "Section_0");
    setAxisTitle(1, "Section_1");
    setAxisTitle(2, "Section_2");

    setReferenceLength(3.0);
}


bool gl3dParetoView::intersectTheObject(Vector3d const &, Vector3d const &, Vector3d &)
{
    return false;
}


void gl3dParetoView::glRenderView()
{
    double rad = 0.019;
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }

    paintTriangles3Vtx(m_vboBox, QColor(105,105,255,100), false, false);
    paintSegments(m_vboBoxEdges, W3dPrefs::s_OutlineStyle);

    double err0=0, err1=0, err2=0;
    for(int i=0; i<m_Pareto.size(); i++)
    {
        Particle const &particle = m_Pareto.at(i);
        err0 = fabs(particle.error(0));
        if(particle.nObjectives()>=2) err1 = fabs(particle.error(1));
        else                          err1 = 0.0;
        if(particle.nObjectives()>=3) err2 = fabs(particle.error(2));
        else                          err2 = 0.0;
        if(i==m_iBest)
            paintSphere({err0, err1, err2}, rad*1.1/m_glScalef, Qt::red, true);
        else
            paintSphere({err0, err1, err2}, rad/m_glScalef, QColor(173,171,233), true);
    }
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle const &particle = m_Swarm.at(i);
        err0 = fabs(particle.error(0));
        if(particle.nObjectives()>=2) err1 = fabs(particle.error(1));
        else                          err1 = 0.0;
        if(particle.nObjectives()>=3) err2 = fabs(particle.error(2));
        else                          err2 = 0.0;

        paintSphere({err0, err1, err2}, rad/m_glScalef, Qt::darkCyan, true);
    }
}


void gl3dParetoView::glMake3dObjects()
{
    if(m_bResetBox)
    {
        gl::makeCube({m_X/2, m_Y/2, m_Z/2}, m_X, m_Y, m_Z, m_vboBox, m_vboBoxEdges);
        m_bResetBox = false;
    }
}
