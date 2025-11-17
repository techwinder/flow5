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
#include <QMouseEvent>

#include "gl3dboatview.h"

#include <core/xflcore.h>
#include <api/frame.h>
#include <api/triangle3d.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <api/fuse.h>
#include <api/fuseocc.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/boat.h>
#include <api/sailnurbs.h>
#include <api/sailocc.h>
#include <api/sailspline.h>
#include <api/sailwing.h>
#include <api/units.h>

bool gl3dBoatView::s_bSurfaces = true;
bool gl3dBoatView::s_bOutline = true;
bool gl3dBoatView::s_bPanels= false;
bool gl3dBoatView::s_bNormals= false;


gl3dBoatView::gl3dBoatView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pBoat = nullptr;

    m_bNormals = false;
    m_bResetglSectionHighlight   = true;
    m_bResetglBoat  = m_bResetglHull = m_bResetglSail = true;
    m_bResetglMesh = true;
}


gl3dBoatView::~gl3dBoatView()
{
    if(m_bAutoDeleteBuffers)
    {
        for(int i=0; i<m_vboSailSurface.size(); i++)
        {
            m_vboSailSurface[i].bind();
            m_vboSailSurface[i].destroy();
            m_vboSailSurface[i].release();
        }
        for(int i=0; i<m_vboSailOutline.size(); i++)
        {
            m_vboSailOutline[i].bind();
            m_vboSailOutline[i].destroy();
            m_vboSailOutline[i].release();
        }
        for(int i=0; i<m_vboSailNormals.size(); i++)
        {
            m_vboSailNormals[i].bind();
            m_vboSailNormals[i].destroy();
            m_vboSailNormals[i].release();
        }

        if(m_vboNormals.isCreated())  m_vboNormals.destroy();

        for(int i=0; i<m_vboFuseTriangulation.size(); i++)
        {
            m_vboFuseTriangulation[i].bind();
            m_vboFuseTriangulation[i].destroy();
            m_vboFuseTriangulation[i].release();
        }

        for(int i=0; i<m_vboFuseOutline.size(); i++)
        {
            m_vboFuseOutline[i].bind();
            m_vboFuseOutline[i].destroy();
            m_vboFuseOutline[i].release();
        }
    }
}


bool gl3dBoatView::intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I)
{
    Vector3d N;
    for(int i=0; i<m_pBoat->nSails(); i++)
    {
        Sail const*pSail = m_pBoat->sail(i);
        if(pSail->intersect(AA-pSail->position(), BB-pSail->position(), I, N))
        {
            return true;
        }
    }
    for(int i=0; i<m_pBoat->nHulls(); i++)
    {
        Fuse const*pFuse = m_pBoat->hull(i);
        if(pFuse->intersectFuse(AA+pFuse->position(), BB+pFuse->position(), I))
        {
            return true;
        }
    }
    return false;
}


void gl3dBoatView::setBoat(Boat *pBoat)
{
    m_pBoat = pBoat;
    setBoatReferenceLength(pBoat);
    reset3dScale();
    m_bResetglBoat = true;
}


void gl3dBoatView::glMakeHulls()
{
    if(m_bResetglHull || m_bResetglBoat)
    {
        m_vboFuseTriangulation.resize(m_pBoat->nHulls());
        m_vboFuseOutline.resize(m_pBoat->nHulls());

        for(int ifuse=0; ifuse<m_pBoat->nHulls(); ifuse++)
        {
            Fuse *pFuse = m_pBoat->hull(ifuse);

            m_vboFuseTriangulation[ifuse].destroy();

            if(m_pBoat->hull(ifuse)->isXflType())
            {
                FuseXfl *pFxfl = dynamic_cast<FuseXfl*>(pFuse);
//                glMakeShellOutline(pFxfl->shells(), pFxfl->position(), m_vboBodyOutline[ifuse]);
                gl::makeBodySplines_outline(pFxfl, pFxfl->position(), m_vboFuseOutline[ifuse]);
                gl::makeTriangulation3Vtx(pFuse->triangulation(),
                                        pFuse->position(),
                                        m_vboFuseTriangulation[ifuse],
                                        pFxfl->isFlatFaceType());
            }
            else if(m_pBoat->hull(ifuse)->isOccType())
            {
                FuseOcc* pFocc = dynamic_cast<FuseOcc*>(pFuse);
                glMakeShellOutline(pFocc->shells(), pFocc->position(), m_vboFuseOutline[ifuse]);
                gl::makeTriangulation3Vtx(pFuse->triangulation(),
                                        pFuse->position(),
                                        m_vboFuseTriangulation[ifuse],
                                        false);
            }
            else if(m_pBoat->hull(ifuse)->isStlType())
            {
                gl::makeTriangulation3Vtx(pFuse->triangulation(),
                                        pFuse->position(),
                                        m_vboFuseTriangulation[ifuse],
                                        true);
                gl::makeTrianglesOutline(pFuse->triangles(), pFuse->position(), m_vboFuseOutline[ifuse]);
            }
        }
    }
}


void gl3dBoatView::glMakeSails()
{
    if(m_bResetglSail || m_bResetglBoat)
    {
        int nSails = m_pBoat->nSails();
        resizeSailBuffers(nSails);
        for(int isail=0; isail<m_pBoat->nSails(); isail++)
        {
            Sail *pSail = m_pBoat->sail(isail);
            gl::makeTriangulation3Vtx(pSail->triangulation(), pSail->position(), m_vboSailSurface[isail], false);
            gl::makeNodeNormals(pSail->triangulation().nodes(), pSail->position(), float(m_RefLength)/50.0f, m_vboSailNormals[isail]);
            if(pSail->isSplineSail())
            {
                SailSpline *pSSail = dynamic_cast<SailSpline*>(pSail);
                gl::makeSplineSailOutline(pSSail, pSSail->position(), m_vboSailOutline[isail]);
            }
            else if(pSail->isNURBSSail())
            {
                SailNurbs *pNurbsSail = dynamic_cast<SailNurbs*>(pSail);
                gl::makeNurbsOutline(*pNurbsSail->pNurbs(), pSail->position(), Sail::iXRes(), Sail::iZRes(), m_vboSailOutline[isail]);
            }
            else if(pSail->isWingSail())
            {
                SailWing *pWingSail = dynamic_cast<SailWing*>(pSail);
                gl::makeWingSailOutline(pWingSail, pSail->position(), m_vboSailOutline[isail]);
            }
            else if(pSail->isOccSail())
            {
                SailOcc const *pOccSail = dynamic_cast<SailOcc const *>(pSail);
                glMakeShellOutline(pOccSail->shapes(), pSail->position(), m_vboSailOutline[isail]);
            }
            else if(pSail->isStlSail())
            {
                gl::makeTrianglesOutline(pSail->triangles(), pSail->position(), m_vboSailOutline[isail]);
            }
        }
    }
}


void gl3dBoatView::glMakeMesh()
{
    if(!s_bPanels) return; //just in time build only

    if((m_bResetglMesh || m_bResetglBoat || m_bResetglSail))
    {
        Vector3d pos;
        gl::makeTriPanels(m_pBoat->refTriMesh().panels(), pos, m_vboBoatMesh);
        gl::makeTriEdges(m_pBoat->refTriMesh().panels(), pos, m_vboBoatMeshEdges);
    }
    m_bResetglMesh = false;
}


void gl3dBoatView::glMake3dObjects()
{
    if(!m_pBoat)
    {
        setBotLeftOutput(QString());
        updatePartFrame(m_pBoat);
        return;
    }
    if(m_bResetglBoat)
    {
        updatePartFrame(m_pBoat);
        setBotLeftOutput(m_pBoat->properties(false));
    }

    glMakeHulls();
    glMakeSails();
    glMakeMesh();

    m_bResetglBoat = m_bResetglHull = m_bResetglSail = m_bResetglSectionHighlight =  false;
}


void gl3dBoatView::resizeSailBuffers(int nSails)
{
    // initialize buffer arrays
    for(int i=0; i<m_vboSailSurface.size(); i++)
    {
        if(m_vboSailSurface[i].isCreated()) m_vboSailSurface[i].destroy();}

    m_vboSailSurface.clear();
    m_vboSailSurface.resize(nSails);

    for(int i=0; i<m_vboSailOutline.size(); i++)
    {
        if(m_vboSailOutline[i].isCreated()) m_vboSailOutline[i].destroy();
    }
    m_vboSailOutline.clear();
    m_vboSailOutline.resize(nSails);

    for(int i=0; i<m_vboSailNormals.size(); i++)
    {
        if(m_vboSailNormals[i].isCreated()) m_vboSailNormals[i].destroy();
    }
    m_vboSailNormals.clear();
    m_vboSailNormals.resize(nSails);

    if(m_vboBoatMesh.isCreated())      m_vboBoatMesh.destroy();
    if(m_vboBoatMeshEdges.isCreated()) m_vboBoatMeshEdges.destroy();
}


void gl3dBoatView::glRenderView()
{
    if(!m_pBoat) return;
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    for(int ihull=0; ihull<m_pBoat->nHulls(); ihull++)
    {
        Fuse *pFuse = m_pBoat->hull(ihull);
        if(!pFuse->isVisible()) continue;


        if(ihull>=m_vboFuseTriangulation.size()) continue; // overriding past errors

        if(s_bSurfaces) paintTriangles3Vtx(m_vboFuseTriangulation[ihull], xfl::fromfl5Clr(pFuse->color()), false, isLightOn());
        if(pFuse->isXflType())
        {
            if(s_bOutline)
            {
/*                if(m_SelectedParts.contains(pFuse->uniqueIndex()))
                    paintFuseXflOutline(pXflFuse, ihull, true);
                else if(s_bOutline)
                    paintFuseXflOutline(pXflFuse, ihull, false); */
                paintSegments(m_vboFuseOutline[ihull], W3dPrefs::s_OutlineStyle);
            }
        }
        else if(pFuse->isOccType())
        {
            if(s_bOutline)
            {
                if(m_SelectedParts.contains(pFuse->uniqueIndex()))
                    paintSegments(m_vboFuseOutline[0], W3dPrefs::s_HighStyle);
                else
                    paintSegments(m_vboFuseOutline[0], W3dPrefs::s_OutlineStyle);
            }
        }
        else if(pFuse->isStlType())
        {
            if(s_bOutline)
            {
                if(m_SelectedParts.contains(pFuse->uniqueIndex()))
                    paintSegments(m_vboFuseOutline[0], W3dPrefs::s_HighStyle);
                else
                    paintSegments(m_vboFuseOutline[0], W3dPrefs::s_OutlineStyle);
            }
        }
    }


    for(int isail=0; isail<m_pBoat->nSails(); isail++)
    {
        Sail *pSail = m_pBoat->sail(isail);

        if(!pSail->isVisible()) continue;
        if(isail>=m_vboSailSurface.size()) continue;
        if(isail>=m_vboSailOutline.size()) continue;

        if(s_bSurfaces)
            paintTriangles3Vtx(m_vboSailSurface[isail], xfl::fromfl5Clr(pSail->color()), true, isLightOn());
        if(s_bOutline)
            paintSegments(m_vboSailOutline[isail], W3dPrefs::s_OutlineStyle);


        if(m_bNormals && isail<m_vboSailNormals.size())
            paintNormals(m_vboSailNormals[isail]);

        if(s_bPanels && !s_bSurfaces)
        {
            paintTriPanels(m_vboBoatMesh, false);
            paintSegments(m_vboBoatMeshEdges, W3dPrefs::s_PanelStyle);
        }

        if(m_bSailCornerPts)
        {
            paintSailCornerPoints(pSail, pSail->position());
        }
    }

    if(bPickNode() && s_bPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<m_pBoat->refTriMesh().nNodes())
        {
            Node const &nd = m_pBoat->refTriMesh().node(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<m_pBoat->refTriMesh().nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = m_pBoat->refTriMesh().node(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }

        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pBoat->refTriMesh().nNodes())
        {
            Node const &nd = m_pBoat->refTriMesh().node(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }
    paintMeasure();
}


void gl3dBoatView::onSurfaces(bool bChecked)
{
    s_bSurfaces = bChecked;
    update();
}


void gl3dBoatView::onOutline(bool bChecked)
{
    s_bOutline = bChecked;
    update();
}


void gl3dBoatView::onPanels(bool bChecked)
{
    s_bPanels = bChecked;
    update();
}


void gl3dBoatView::onPartSelClicked()
{
    if(!m_pBoat) return;

    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if(!pSenderBox) return;

    QVariant property = pSenderBox->property("partindex");
    if(property.isNull()) return;

    bool bOk = false;
    int index = property.toInt(&bOk);
    if(!bOk) return;

    if(index<m_pBoat->nSails())
    {
        Sail * pSail = m_pBoat->sail(index);
        if(pSail) pSail->setVisible(pSenderBox->isChecked());
    }
    else if(index<m_pBoat->nSails()+m_pBoat->nHulls())
    {
        Fuse *pFuse = m_pBoat->hull(index-m_pBoat->nSails());
        if(pFuse) pFuse->setVisible(pSenderBox->isChecked());
    }

    // the geometry does not need to be rebuilt
    m_bResetglBoat = true;
    m_bResetglMesh = true;
    m_bResetglSectionHighlight = true;
    update();
}



void gl3dBoatView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(!m_bHasMouseMoved && m_bP3Select)
    {
        Vector3d I;
        if(pickPanel3(pEvent->pos(), m_pBoat->refTriMesh().panels(), I))
        {
            emit panelSelected(m_PickedPanelIndex);
        }
    }
    else
    {
        int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

        if(draggeddistance<5 && (pEvent->button()==Qt::LeftButton) && bPickNode() && m_PickedNodeIndex>=0)
        {
            if(m_PickedNodeIndex<0)
            {
                // clear and start again node pair picking
                m_NodePair = {-1,-1};
                return;
            }

            if(m_NodePair.first<0)
            {
                emit (pickedNodeIndex(m_PickedNodeIndex));
                m_NodePair.first = m_PickedNodeIndex;
            }
            else if(m_NodePair.second<0)
            {
                m_NodePair.second = m_PickedNodeIndex;
                //two valid node indexes
                emit (pickedNodeIndex(m_PickedNodeIndex));
                emit (pickedNodePair(m_NodePair));
                m_NodePair = {-1,-1};
            }
            else
            {
                //start again
                m_NodePair = {m_PickedNodeIndex, -1};
            }
        }
    }


    gl3dXflView::mouseReleaseEvent(pEvent);

    update();
}


void gl3dBoatView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    int lastpickedindex = m_PickedPanelIndex;
    if((!bPickNode() && !m_bHighlightPanel) || !s_bPanels)
    {
        m_PickedNodeIndex = -1;
        m_PickedPanelIndex = -1;
        return;
    }

    Vector3d I;
    pickPanel3(pEvent->pos(), m_pBoat->refTriMesh().panels(), I);

    if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pBoat->refTriMesh().nPanels())
    {
        m_PickedNodeIndex = -1;
        clearTopRightOutput();
        update();
        return;
    }

    if(m_bHighlightPanel)
    {
        if(m_PickedPanelIndex!=lastpickedindex)
        {
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<m_pBoat->nPanel3())
            {
                Panel3 const& p3 = m_pBoat->refTriMesh().panelAt(m_PickedPanelIndex);

                gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);

                setTopRightOutput(p3.properties(false));
            }
            update();
        }
    }

    if(bPickNode())
    {
        Panel3 const& p3 = m_pBoat->refTriMesh().panelAt(m_PickedPanelIndex);
        int lastpickedindex = m_PickedNodeIndex;
        pickPanelNode(p3, I, xfl::NOSURFACE);
        if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=m_pBoat->refTriMesh().nNodes())
        {
            m_PickedNodeIndex = -1;
            clearTopRightOutput();
        }
        else if(m_PickedNodeIndex!=lastpickedindex)
        {
            if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pBoat->refTriMesh().nNodes())
            {
                Node const &nd = m_pBoat->refTriMesh().node(m_PickedNodeIndex);
                setTopRightOutput(QString::fromStdString(nd.properties()));
             }
        }
    }

    update();
}


