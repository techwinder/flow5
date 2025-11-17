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

#include "gl3dplanexflview.h"

#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <api/units.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/planexfl.h>

bool gl3dPlaneXflView::s_bHighlightSelectedPart = false;


gl3dPlaneXflView::gl3dPlaneXflView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pPlaneXfl = nullptr;

    m_bResetglPlane = true;
    m_bResetglFuse  = true;

    m_pSelectedPart = nullptr;
    m_SelectedParts.clear();

    m_bP3Select = false;
    m_bThickWings = true;

    m_iP3Node[0] = m_iP3Node[1] = m_iP3Node[2] = -1;
    m_iStrip = 0;

    m_bIsSelecting = false;
    m_bResetP3Select = true;
}


gl3dPlaneXflView::~gl3dPlaneXflView()
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
    m_vboWingOutline.clear();

    if(m_vboFrames.isCreated()) m_vboFrames.destroy();
}


void gl3dPlaneXflView::setPlane(PlaneXfl *pPlane)
{
    m_pPlaneXfl = pPlane;

    if     (m_pPlaneXfl->mainWing()) setReferenceLength(m_pPlaneXfl->span());
    else if(m_pPlaneXfl->hasFuse())  setReferenceLength(m_pPlaneXfl->fuseAt(0)->length());

    updatePartFrame(pPlane);
}


/**
 * Creates the VertexBufferObjects
 */
void gl3dPlaneXflView::glMake3dObjects()
{
    if(!m_pPlaneXfl) return;
//    QApplication::setOverrideCursor(Qt::WaitCursor);

    //make fuse geometry
    if(m_bResetglFuse || m_bResetglPlane)
    {
        m_vboFuseTriangulation.resize(m_pPlaneXfl->nFuse());
        m_vboBodyOutline.resize(m_pPlaneXfl->nFuse());

        for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        {
//            Fuse *pTranslatedFuse = m_pPlane->fuse(ifuse)->clone();
//            pTranslatedFuse->translate(m_pPlane->fusePos(ifuse));
            Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
            if(m_pPlaneXfl->fuseAt(ifuse)->isXflType())
            {
                FuseXfl const *pTranslatedFxfl = dynamic_cast<FuseXfl const*>(pFuse);

                gl::makeTriangulation3Vtx(pTranslatedFxfl->triangulation(), m_pPlaneXfl->fusePos(ifuse),
                                    m_vboFuseTriangulation[ifuse], pTranslatedFxfl->isFlatFaceType());
                int nPts = pFuse->isSplineType() ? 30 : 1;
                glMakeShellOutline(pTranslatedFxfl->shells(), m_pPlaneXfl->fusePos(ifuse), m_vboBodyOutline[ifuse], nPts);
                if(pFuse->isSplineType())
                    gl::makeFuseXflFrames(pTranslatedFxfl, pTranslatedFxfl->position(), W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes(), m_vboFrames);
                else if(pFuse->isSectionType())
                {
                    FuseSections const *pFuseSecs = dynamic_cast<FuseSections const*>(pTranslatedFxfl);
                    if(pFuseSecs)
                        gl::makeFuseXflSections(pFuseSecs, pFuseSecs->position(), W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes(), m_vboFrames);
                }
            }
            else if(m_pPlaneXfl->fuseAt(ifuse)->isOccType())
            {
                FuseOcc const* pTranslatedFocc = dynamic_cast<FuseOcc const*>(pFuse);
                gl::makeTriangulation3Vtx(pTranslatedFocc->triangulation(), m_pPlaneXfl->fusePos(ifuse), m_vboFuseTriangulation[ifuse], false);
                glMakeShellOutline(pTranslatedFocc->shells(), m_pPlaneXfl->fusePos(ifuse), m_vboBodyOutline[ifuse]);
            }
            else if(m_pPlaneXfl->fuseAt(ifuse)->isStlType())
            {
                FuseStl const* pTranslatedFstl = dynamic_cast<FuseStl const*>(pFuse);
                gl::makeTriangulation3Vtx(pTranslatedFstl->triangulation(), m_pPlaneXfl->fusePos(ifuse), m_vboFuseTriangulation[ifuse], true);
                gl::makeTrianglesOutline(pTranslatedFstl->triangles(), m_pPlaneXfl->fusePos(ifuse), m_vboBodyOutline[ifuse]);
            }
            else
            {
                m_vboFuseTriangulation[ifuse].destroy();
            }
        }
    }

    // make wing geometry
    if(m_bResetglPlane)
    {
        // initialize buffer arrays
        for(int i=0; i<m_vboWingOutline.size(); i++) m_vboWingOutline[i].destroy();
        for(int i=0; i<m_vboWingSurface.size(); i++) m_vboWingSurface[i].destroy();
        m_vboWingOutline.clear();
        m_vboWingSurface.clear();
        m_vboWingOutline.resize(m_pPlaneXfl->nWings());
        m_vboWingSurface.resize(m_pPlaneXfl->nWings());


        Fuse *pTranslatedFuse = nullptr;
        if(m_pPlaneXfl->hasFuse() && fabs(m_pPlaneXfl->fusePos(0).y)<0.001)
        {
            pTranslatedFuse = m_pPlaneXfl->fuseAt(0)->clone();
            pTranslatedFuse->translate(m_pPlaneXfl->fusePos(0));
        }
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            WingXfl *pWing = m_pPlaneXfl->wing(iw);
            if(pWing)
            {
                pWing->makeTriangulation(pTranslatedFuse, W3dPrefs::s_iChordwiseRes);
                gl::makeTriangles3Vtx(pWing->triangulation().triangles(), false, m_vboWingSurface[iw]);
                gl::makeSegments(pWing->outline(), Vector3d(), m_vboWingOutline[iw]);
            }
        }
        if(pTranslatedFuse) delete pTranslatedFuse;
    }

    if(m_bResetglSegments)
    {
        gl::makeSegments(m_Segments, Vector3d(), m_vboSegments);
        m_bResetglSegments = false;
    }

    // make tri mesh
    // cannot use the plane's tri mesh, because part meshs may be switched on/off individually
    if(m_bResetglPlane || m_bResetglFuse || m_bResetglMesh)
    {
        for(int ivbo=0; ivbo<m_vboTriMesh.size(); ivbo++)
        {
            if(m_vboTriMesh[ivbo].isCreated()) m_vboTriMesh[ivbo].destroy();
            if(m_vboTriEdges[ivbo].isCreated()) m_vboTriEdges[ivbo].destroy();
        }
        // wing tri mesh are not stored, so build them OTF
        int nvbo = m_pPlaneXfl->nWings()+m_pPlaneXfl->nFuse();
        m_vboTriMesh.resize(nvbo);
        m_vboTriEdges.resize(nvbo);

        int nPanels = 0;

        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            // make a duplicate wing to avoid messing up surface panel indexes
            WingXfl awing(*m_pPlaneXfl->wingAt(iw));

            awing.makeTriPanels(nPanels, 0, m_bThickWings);
            nPanels += awing.nPanel3();

            gl::makeTriPanels(awing.triMesh().panels(), Vector3d(), m_vboTriMesh[iw]);
            gl::makeTriEdges(awing.triMesh().panels(), Vector3d(), m_vboTriEdges[iw]);
        }
        for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
            int ivbo = m_pPlaneXfl->nWings()+ifuse;
            gl::makeTriPanels(pFuse->triMesh().panels(), m_pPlaneXfl->fusePos(ifuse), m_vboTriMesh[ivbo]);
            gl::makeTriEdges(pFuse->triMesh().panels(), m_pPlaneXfl->fusePos(ifuse), m_vboTriEdges[ivbo]);
        }

        std::vector<Panel3> p3High;
        for(int ip=0; ip<m_PanelHightlight.size(); ip++)
        {
            int idx = m_PanelHightlight.at(ip);
            if(idx>=0 && idx<m_pPlaneXfl->nPanel3())
                p3High.push_back(m_pPlaneXfl->refTriMesh().panelAt(idx));
        }
        gl::makeTriEdges(p3High, Vector3d(), m_vboHighlightPanel3);
    }

    if(m_bResetP3Select)
    {
        Fuse const *pFuse = m_pPlaneXfl->fuseAt(0);
        if(pFuse) glMakeP3Sel();
        m_bResetP3Select = false;
    }

    m_bResetglFuse  = false;
    m_bResetglPlane = false;
    m_bResetglMesh  = false;
}


void gl3dPlaneXflView::glRenderView()
{
    if(!m_pPlaneXfl) return;

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,
                                   vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
    {
        if(ifuse>=m_vboFuseTriangulation.size()) break;

        Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
        if(!pFuse->isVisible()) continue;


        bool bHighlight = s_bHighlightSelectedPart && m_SelectedParts.contains(pFuse->uniqueIndex());

        if(pFuse->isXflType())
        {
            if(m_bSurfaces)  paintTriangles3Vtx(m_vboFuseTriangulation[ifuse], pFuse->color(), false, isLightOn());
            if(m_bOutline)
            {
                if(bHighlight)
                    paintSegments(m_vboBodyOutline[0], W3dPrefs::s_HighStyle);
                else
                    paintSegments(m_vboBodyOutline[0], W3dPrefs::s_OutlineStyle);

                if(pFuse->isSplineType() || pFuse->isSectionType())
                    paintSegments(m_vboFrames, W3dPrefs::s_OutlineStyle);
            }
        }
        else if(pFuse->isOccType())
        {
            if(m_bSurfaces)  paintTriangles3Vtx(m_vboFuseTriangulation[ifuse], pFuse->color(), false, isLightOn());
            if(m_bOutline)
            {
                if(bHighlight)
                    paintSegments(m_vboBodyOutline[0], W3dPrefs::s_HighStyle);
                else
                    paintSegments(m_vboBodyOutline[0], W3dPrefs::s_OutlineStyle);
            }
        }
        else if(pFuse->isStlType())
        {
            if(m_bSurfaces)  paintTriangles3Vtx(m_vboFuseTriangulation[ifuse], pFuse->color(), false, isLightOn());
            if(m_bOutline || m_bTessellation)
            {
                if (!bHighlight) paintSegments(m_vboBodyOutline[ifuse], W3dPrefs::s_OutlineStyle);
                else             paintSegments(m_vboBodyOutline[ifuse], W3dPrefs::s_HighStyle);
            }
        }
    }

    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);
        if(!pWing->isVisible()) continue;
        if(pWing)
        {
//            if(m_bSurfaces) paintWingSurfaces(iw, pWing->wingColor());
            if(m_bSurfaces)  paintTriangles3Vtx(m_vboWingSurface[iw], pWing->color(), false, isLightOn());

            if(m_bOutline)
            {
                bool bHighlight = s_bHighlightSelectedPart && m_SelectedParts.contains(pWing->uniqueIndex());
                paintSegments(m_vboWingOutline[iw], W3dPrefs::s_OutlineStyle, bHighlight);
            }

            if(m_bFoilNames) paintFoilNames(pWing);
        }
    }

    if(m_bShowMasses)
        glDrawMasses(m_pPlaneXfl);

    paintMeshPanels();

    if(bPickPanel3() && m_PickedPanelIndex>=0 && m_PickedPanelIndex<m_pPlaneXfl->nPanel3() && m_bMeshPanels)
        highlightPickedPanel3(m_pPlaneXfl->panel3At(m_PickedPanelIndex));

    if(m_bP3Select)
    {
        paintSegments(m_vboP3Select, W3dPrefs::s_SelectStyle);
    }

    if(bPickNode() && m_bMeshPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<m_pPlaneXfl->refTriMesh().nNodes())
        {
            Node const &nd = m_pPlaneXfl->refTriMesh().nodeAt(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::darkYellow);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<m_pPlaneXfl->refTriMesh().nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = m_pPlaneXfl->refTriMesh().nodeAt(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan);
        }

        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pPlaneXfl->refTriMesh().nNodes())
        {
            Node const &nd = m_pPlaneXfl->refTriMesh().nodeAt(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red);
#ifdef QT_DEBUG
            glRenderText(nd+Vector3d(0.0,0.0,0.02/m_glScalef),
                         QString::asprintf("nd%d", m_PickedNodeIndex),
                         DisplayOptions::textColor(), true);
#endif
        }
    }
    paintMeasure();
}


void gl3dPlaneXflView::paintMeshPanels()
{
    if(m_bMeshPanels)
    {
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);
            if(pWing->isVisible() && iw<m_vboTriMesh.size() && !m_bSurfaces)
            {
                paintTriPanels(m_vboTriMesh[iw], false);
                paintSegments(m_vboTriEdges[iw], W3dPrefs::s_PanelStyle);
            }
        }
        for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
            int ivbo = m_pPlaneXfl->nWings()+ifuse;
            if(pFuse->isVisible() && ivbo<m_vboTriMesh.size() && !m_bSurfaces)
            {
                paintTriPanels(m_vboTriMesh[ivbo], false);
                paintSegments(m_vboTriEdges[ivbo], W3dPrefs::s_PanelStyle);
            }
        }

        if(m_PanelHightlight.size())
            paintSegments(m_vboHighlightPanel3, W3dPrefs::s_HighStyle);
    }

    if(m_Segments.size())
        paintSegments(m_vboSegments, W3dPrefs::highlightColor(), 1, Line::SOLID, true);
}


void gl3dPlaneXflView::resetPickedNodes()
{
    m_NodePair={-1,-1};
    m_PickedNodeIndex=-1;
    m_iP3Node[0]=m_iP3Node[1]=m_iP3Node[2]=-1;
    m_iStrip = 0;
}


/** cannot use pickPanel3 because of need to check part visibility */
bool gl3dPlaneXflView::pickTriPanel(QPoint const &point, bool &bFuse, Vector3d &I)
{
    Vector3d Inter, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

    float zmax = +1.e10;

    // consider only if parent object is visible
    // check wing panels
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);
        if(pWing->isVisible())
        {
            for(int i3=pWing->firstPanel3Index(); i3<pWing->firstPanel3Index()+pWing->nPanel3(); i3++)
            {
                Panel3 const &p3 = m_pPlaneXfl->panel3At(i3);
                if(p3.intersect(AA, U, Inter))
                {
                    worldToScreen(p3.CoG(), v4d);
                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        m_PickedPoint = p3.CoG();
                        m_PickedPanelIndex = i3;
                        I = Inter;
                        bFuse = false;
                    }
                }
            }
        }
    }

    //check fuse panels
    for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
    {
        Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
        if(pFuse->isVisible())
        {
            for(int i3=pFuse->firstPanel3Index(); i3<pFuse->firstPanel3Index()+pFuse->nPanel3(); i3++)
            {
                if(i3<m_pPlaneXfl->nPanel3())
                {
                    Panel3 const &p3 = m_pPlaneXfl->panel3At(i3);
                    if(p3.intersect(AA, U, Inter))
                    {
                        worldToScreen(p3.CoG(), v4d);
                        if(v4d.z()<zmax)
                        {
                            zmax = v4d.z();
                            m_PickedPoint = p3.CoG();
                            m_PickedPanelIndex = i3;
                            I = Inter;
                            bFuse = true;
                        }
                    }
                }
            }
        }
    }

    return m_PickedPanelIndex>=0;
}


void gl3dPlaneXflView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier) || !m_bMeshPanels)
    {
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    if((!bPickNode() && !isPicking()) || !m_bMeshPanels)
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex = -1;
        return;
    }

    Vector3d I;
    bool bFuse = false;
    if(isPicking())
//    if(bPickNode() || (m_bHighlightPanel && isPicking()))
        pickTriPanel(pEvent->pos(), bFuse, I);

    if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pPlaneXfl->refTriMesh().nPanels())
    {
        m_PickedNodeIndex = -1;
        clearTopRightOutput();
        update();
        return;
    }
    Panel3 const &p3 = m_pPlaneXfl->panel3At(m_PickedPanelIndex);

    if(bPickNode())
    {
        if(m_pPlaneXfl->hasFuse())
        {
            pickPanelNode(p3, I, m_SurfacePick);
            if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=m_pPlaneXfl->refTriMesh().nNodes())
            {
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
            }
            else
            {
                Node const &nd = m_pPlaneXfl->refTriMesh().nodeAt(m_PickedNodeIndex);
                setTopRightOutput(QString::fromStdString(nd.properties()));
            }
        }
    }
    else if(bPickPanel3())
    {
        if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pPlaneXfl->nPanel3())
        {
            m_PickedPanelIndex = -1;
            clearTopRightOutput();
        }
        else
        {
            gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);
            setTopRightOutput(p3.properties(false));
        }
    }
    update();
}


void gl3dPlaneXflView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

    if(draggeddistance<5 && (pEvent->button()==Qt::LeftButton))
    {
        if (bPickNode() && m_PickedNodeIndex>=0)
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
            }
            else
            {
                emit pickedNodeIndex(m_PickedNodeIndex);
            }
/*            else
            {
                //start again
                m_NodePair = {m_PickedNodeIndex, -1};
            }*/
        }
        else if(bPickPanel3() && m_PickedPanelIndex>=0)
        {
            int idx = m_P3Selection.indexOf(m_PickedPanelIndex);

            Panel3 const &p3 = m_pPlaneXfl->triMesh().panelAt(m_PickedPanelIndex);
            if(p3.isFusePanel())
            {
                if(idx>=0) m_P3Selection.removeAt(idx);
                else       m_P3Selection.append(m_PickedPanelIndex);
                m_bResetP3Select = true;
            }
            update();
        }
    }

    gl3dXflView::mouseReleaseEvent(pEvent);
}


bool gl3dPlaneXflView::intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    Part const *pPart = intersectPart(AA, U, I);
    if(pPart) return true;
    return false;
}


Part const * gl3dPlaneXflView::intersectPart(Vector3d const&AA,  Vector3d  &U, Vector3d &I)
{
    QVector4D v4d;
    float zmax = +1.e10;

    Vector3d PickedPoint;

    double dist=0;
    int idxSurf=-1;
    m_pSelectedPart = nullptr;
    for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
    {
        if (m_pPlaneXfl->wingAt(iw)->isVisible())
        {
            if(m_pPlaneXfl->wingAt(iw)->intersectWing(AA, U, PickedPoint, idxSurf, dist, false))
            {
                worldToScreen(PickedPoint, v4d);
                if(v4d.z()<zmax)
                {
                    m_pSelectedPart = m_pPlaneXfl->wingAt(iw);
                    zmax = v4d.z();
                    I = PickedPoint;
                }
            }
        }
    }

    for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
    {
        Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
        Vector3d OT = AA - m_pPlaneXfl->fusePos(ifuse);

        if(pFuse->isVisible())
        {
            for(int i3=0; i3<pFuse->nTriangles(); i3++)
            {
                Triangle3d const &p3 = pFuse->triangleAt(i3);
                if(p3.intersectRayInside(OT, U, PickedPoint))
                {
                    PickedPoint += m_pPlaneXfl->fusePos(ifuse); //???
                    worldToScreen(p3.CoG_g(), v4d);
                    if(v4d.z()<zmax)
                    {
                        m_pSelectedPart = pFuse;
                        zmax = v4d.z();
                        I = PickedPoint;
                    }
                }
            }
        }
    }

    return m_pSelectedPart;
}


void gl3dPlaneXflView::onPartSelClicked()
{
    if(!m_pPlaneXfl) return;
    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if(!pSenderBox) return;

    QVariant property = pSenderBox->property("partindex");
    if(property.isNull()) return;

    bool bOk = false;
    int index = property.toInt(&bOk);
    if(!bOk) return;

    if(index<m_pPlaneXfl->nWings())
    {
        WingXfl const* pWing = m_pPlaneXfl->wingAt(index);
        if(pWing) pWing->setVisible(pSenderBox->isChecked());
    }
    else if(index<m_pPlaneXfl->nWings()+m_pPlaneXfl->nFuse())
    {
        Fuse const*pFuse = m_pPlaneXfl->fuseAt(index-m_pPlaneXfl->nWings());
        if(pFuse) pFuse->setVisible(pSenderBox->isChecked());
    }

    m_bResetglMesh = true;

    update();
}


void gl3dPlaneXflView::shiftP3Nodes()
{
    // alternate orientation so that the normal points outwards
    if(m_iStrip%2==1)
    {
        m_iP3Node[0] = m_iP3Node[0];
        m_iP3Node[1] = m_iP3Node[2];
    }
    else
    {
        m_iP3Node[0] = m_iP3Node[2];
        m_iP3Node[1] = m_iP3Node[1];
    }
    m_iP3Node[2] = -1;
    m_iStrip++;

    m_NodePair.first  = m_iP3Node[0];
    m_NodePair.second = m_iP3Node[1];
}


void gl3dPlaneXflView::glMakeP3Sel()
{
    QVector<Segment3d> segments;
    for(int i=0; i<m_P3Selection.size(); i++)
    {
        int i3 = m_P3Selection.at(i);
        segments.append(m_pPlaneXfl->triMesh().panelAt(i3).edge(0));
        segments.append(m_pPlaneXfl->triMesh().panelAt(i3).edge(1));
        segments.append(m_pPlaneXfl->triMesh().panelAt(i3).edge(2));
    }
    gl::makeSegments(segments, Vector3d(), m_vboP3Select);
}





