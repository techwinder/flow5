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
#include <QLayout>
#include <QCheckBox>
#include <QPainter>
#include <QMouseEvent>

#include "gl3dplanestlview.h"

#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <api/units.h>
#include <api/geom_global.h>
#include <api/planestl.h>


gl3dPlaneSTLView::gl3dPlaneSTLView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pPlaneSTL = nullptr;

    m_bResetglPlane = true;
    m_bResetglMesh  = true;
    m_bResetglNormals = true;

    m_pSelectedPart = nullptr;
    m_SelectedParts.clear();
}


gl3dPlaneSTLView::~gl3dPlaneSTLView()
{
    if(m_vboHighlightPanel3.isCreated())  m_vboHighlightPanel3.destroy();
    if(m_vboTriangulation.isCreated())    m_vboTriangulation.destroy();
    if(m_vboTriMesh.isCreated())          m_vboTriMesh.destroy();
    if(m_vboTriEdges.isCreated())         m_vboTriEdges.destroy();
    if(m_vboTriangleNormals.isCreated())  m_vboTriangleNormals.destroy();
    if(m_vboSegments.isCreated())         m_vboSegments.destroy();
}


void gl3dPlaneSTLView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();
    if(draggeddistance<5)
    {
        if(!m_bHasMouseMoved && m_bP3Select)
        {
            Vector3d I;
            if(pickPanel3(pEvent->pos(), m_pPlaneSTL->refTriMesh().panels(), I))
            {
                emit panelSelected(m_PickedPanelIndex);
            }
        }
        else
        {
            if((pEvent->button()==Qt::LeftButton) && bPickNode() && m_PickedNodeIndex>=0)
            {
                if(m_PickedNodeIndex<0)
                {
                    // clear and start again node pair picking
                    m_NodePair = {-1,-1};
                    return;
                }

                if(m_NodePair.first<0)
                {
                    emit pickedNodeIndex(m_PickedNodeIndex);
                    m_NodePair.first = m_PickedNodeIndex;
                }
                else if(m_NodePair.second<0)
                {
                    m_NodePair.second = m_PickedNodeIndex;
                    //two valid node indexes
                    emit pickedNodeIndex(m_PickedNodeIndex);
                    emit pickedNodePair(m_NodePair);
                    m_NodePair = {-1,-1};
                }
                else
                {
                    //start again
                    m_NodePair = {m_PickedNodeIndex, -1};
                }
            }
        }
    }

    gl3dXflView::mouseReleaseEvent(pEvent);

    update();
}


void gl3dPlaneSTLView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    int lastpickedindex = m_PickedPanelIndex;
    if((!bPickNode() && !isPicking()) || !m_bMeshPanels)
    {
        m_PickedNodeIndex = -1;
        m_PickedPanelIndex = -1;
        return;
    }

    Vector3d I;
    pickPanel3(pEvent->pos(), m_pPlaneSTL->refTriMesh().panels(), I);

    if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pPlaneSTL->refTriMesh().nPanels())
    {
        m_PickedNodeIndex = -1;
        clearTopRightOutput();
        update();
        return;
    }

    if(isPicking())
    {
        if(m_PickedPanelIndex!=lastpickedindex)
        {
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<m_pPlaneSTL->nPanel3())
            {
                Panel3 const& p3 = m_pPlaneSTL->refTriMesh().panelAt(m_PickedPanelIndex);

                gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);

                setTopRightOutput(p3.properties(false));
             }

            update();
        }
    }

    if(bPickNode())
    {
        Panel3 const& p3 = m_pPlaneSTL->refTriMesh().panelAt(m_PickedPanelIndex);
        int lastpickedindex = m_PickedNodeIndex;
        pickPanelNode(p3, I, xfl::NOSURFACE);
        if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=m_pPlaneSTL->refTriMesh().nNodes())
        {
            m_PickedNodeIndex = -1;
            clearTopRightOutput();
        }
        else if(m_PickedNodeIndex!=lastpickedindex)
        {
            if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pPlaneSTL->refTriMesh().nNodes())
            {
                Node const &nd = m_pPlaneSTL->refTriMesh().node(m_PickedNodeIndex);
                setTopRightOutput(QString::fromStdString(nd.properties()));
             }
        }
    }
    update();
}


void gl3dPlaneSTLView::setPlane(PlaneSTL *pPlane)
{
    m_pPlaneSTL = pPlane;

    setReferenceLength(pPlane->span());
    updatePartFrame(pPlane);
}


void gl3dPlaneSTLView::keyPressEvent(QKeyEvent *pEvent)
{
    bool bAlt = false;
    if(pEvent->modifiers() & Qt::AltModifier) bAlt =true;

    switch (pEvent->key())
    {
        case Qt::Key_N:
        {
            if(bAlt)
            {
                m_bNormals = !m_bNormals;
                update();
            }
            break;
        }
        default:
            break;
    }
    gl3dXflView::keyPressEvent(pEvent);
}


void gl3dPlaneSTLView::glMake3dObjects()
{
    if(!m_pPlaneSTL) return;

    if(m_bResetglPlane)
    {
        gl::makeTriangulation3Vtx(m_pPlaneSTL->triangulation(), Vector3d(), m_vboTriangulation, true);
        gl::makeTrianglesOutline(m_pPlaneSTL->triangles(), Vector3d(), m_vboOutline);
    }

    if(m_bResetglSegments)
    {
        gl::makeSegments(m_Segments, Vector3d(), m_vboSegments);
        m_bResetglSegments = false;
    }

    // make tri mesh
    if(m_bResetglPlane || m_bResetglMesh)
    {
        gl::makeTriPanels(m_pPlaneSTL->refTriMesh().panels(), Vector3d(), m_vboTriMesh);
        gl::makeTriEdges( m_pPlaneSTL->refTriMesh().panels(), Vector3d(), m_vboTriEdges);

        std::vector<Panel3> p3High;
        for(int ip=0; ip<m_PanelHightlight.size(); ip++)
        {
            int idx = m_PanelHightlight.at(ip);
            if(idx>=0 && idx<m_pPlaneSTL->nPanel3())
                p3High.push_back(m_pPlaneSTL->refTriMesh().panelAt(idx));
        }
        gl::makeTriEdges(p3High, Vector3d(), m_vboHighlightPanel3);
    }

    if(m_bResetglPlane || m_bResetglNormals)
    {
        gl::makeTriangleNormals(m_pPlaneSTL->triangles(), float(m_pPlaneSTL->span())/20.0f, m_vboTriangleNormals);
        m_bResetglNormals = false;
    }

    m_bResetglPlane = false;
    m_bResetglMesh = false;
}


void gl3dPlaneSTLView::glRenderView()
{
    if(!m_pPlaneSTL) return;

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    if(m_bSurfaces)
        paintTriangles3Vtx(m_vboTriangulation, m_pPlaneSTL->surfaceColor(), false, isLightOn());

   if(m_bOutline || m_bTessellation)
       paintSegments(m_vboOutline, W3dPrefs::s_OutlineStyle);

    if(m_Segments.size())
        paintSegments(m_vboSegments, W3dPrefs::highlightColor(), 1, Line::SOLID, true);

    if(m_bMeshPanels && !m_bSurfaces)
    {
        paintTriPanels(m_vboTriMesh, false);
        paintSegments(m_vboTriEdges, W3dPrefs::s_PanelStyle);
    }
    if(m_PanelHightlight.size())
        paintSegments(m_vboHighlightPanel3, W3dPrefs::s_HighStyle);

    if(m_bNormals)
    {
        paintNormals(m_vboTriangleNormals);
    }

    for(int in=0; in<m_pPlaneSTL->refTriMesh().nodeCount(); in++)
    {
        Node const &nd = m_pPlaneSTL->refTriMesh().nodeAt(in);
        if(nd.isTrailing())
        {
            paintIcosahedron(nd, 0.015/m_glScalef, Qt::darkRed, W3dPrefs::s_OutlineStyle, true, true);
        }
    }

    if(bPickPanel3() && m_PickedPanelIndex>=0 && m_PickedPanelIndex<m_pPlaneSTL->refTriMesh().nPanels() && m_bMeshPanels)
        highlightPickedPanel3(m_pPlaneSTL->refTriMesh().panel(m_PickedPanelIndex));


    if(bPickNode() && m_bMeshPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<m_pPlaneSTL->refTriMesh().nNodes())
        {
            Node const &nd = m_pPlaneSTL->refTriMesh().node(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<m_pPlaneSTL->refTriMesh().nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = m_pPlaneSTL->refTriMesh().node(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }

        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pPlaneSTL->refTriMesh().nNodes())
        {
            Node const &nd = m_pPlaneSTL->refTriMesh().node(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }

    if(m_bShowMasses)
        glDrawMasses(m_pPlaneSTL);

    paintMeasure();
}

bool gl3dPlaneSTLView::intersectTheObject(const Vector3d &AA, const Vector3d &BB, Vector3d &I)
{
    if(!m_pPlaneSTL) return false;
    Vector3d U = (BB-AA).normalized();
    return m_pPlaneSTL->intersectTriangles(AA,BB,I, xfl::isMultiThreaded());
}

