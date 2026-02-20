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
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>

#include "gl3dxplaneview.h"



#include <api/bspline3d.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/geom_global.h>
#include <api/p3linanalysis.h>
#include <api/p3unianalysis.h>
#include <api/p4analysis.h>
#include <api/panel3.h>
#include <api/panel4.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/planestl.h>
#include <api/planexfl.h>
#include <api/surface.h>
#include <api/wingopp.h>


#include <core/displayoptions.h>
#include <api/units.h>
#include <api/trace.h>
#include <core/xflcore.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/flowctrls.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/poppctrls/streamlinesctrls.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <modules/xplane/analysis/streamlinemaker.h>
#include <modules/xplane/controls/popp3dctrls.h>
#include <modules/xplane/controls/stab3dctrls.h>
#include <modules/xplane/glview/glxplanebuffers.h>
#include <modules/xplane/menus/xplanemenus.h>
#include <modules/xplane/xplane.h>


XPlane *gl3dXPlaneView::s_pXPlane = nullptr;
bool gl3dXPlaneView::s_bResetglGeom(true);
bool gl3dXPlaneView::s_bResetglMesh(true);
bool gl3dXPlaneView::s_bResetglWake(true);
bool gl3dXPlaneView::s_bResetglOpp(true);
bool gl3dXPlaneView::s_bResetglLift(true);
bool gl3dXPlaneView::s_bResetglMoments(true);
bool gl3dXPlaneView::s_bResetglDrag(true);
bool gl3dXPlaneView::s_bResetglDownwash(true);
bool gl3dXPlaneView::s_bResetglPanelGamma(true);
bool gl3dXPlaneView::s_bResetglPanelCp(true);
bool gl3dXPlaneView::s_bResetglPanelForce(true);
bool gl3dXPlaneView::s_bResetglStream(true);
bool gl3dXPlaneView::s_bResetglVortons(true);



gl3dXPlaneView::gl3dXPlaneView(QWidget *parent) : gl3dXflView(parent)
{
    m_pglXPlaneBuffers = nullptr;

    m_pPOpp3dControls = nullptr;
    m_pCrossFlowCtrls = nullptr;

    m_bAutoDeleteBuffers = true;

    s_bResetglGeom       = true;
    s_bResetglMesh       = true;
    s_bResetglWake       = true;
    s_bResetglOpp        = true;
    s_bResetglLift       = true;
    s_bResetglMoments    = true;
    s_bResetglDrag       = true;
    s_bResetglDownwash   = true;
    s_bResetglPanelGamma = true;
    s_bResetglPanelCp    = true;
    s_bResetglPanelForce = true;
    s_bResetglStream     = true;
    s_bResetglVortons    = true;

    m_nBlocks = 20;

    m_PickedVal = 76.54321;

    m_PixLegend = QPixmap(107, 97);
    m_PixLegend.fill(Qt::transparent);

    m_bShowCpScale    = false;

    m_bPanelNormals = m_bNodeNormals = m_bVortices = false;

    m_FlowYPos = 0.0f;
    m_bResetBoids = m_bResetFlowPanels = true;

    m_stackInterval.resize(50);
    m_stackInterval.fill(0);

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());

    m_pP4Analysis = new P4Analysis;
    m_pP3UniAnalysis = new P3UniAnalysis;
    m_pP3LinAnalysis = new P3LinAnalysis;

    connect(&m_FlowTimer, SIGNAL(timeout()), this, SLOT(update()));
}


gl3dXPlaneView::~gl3dXPlaneView()
{
    if(m_pglXPlaneBuffers && m_bAutoDeleteBuffers)
    {
        delete m_pglXPlaneBuffers;
        m_pglXPlaneBuffers = nullptr;
    }

    if (m_pP4Analysis)
    {
        delete m_pP4Analysis;
        m_pP4Analysis = nullptr;
    }
    if(m_pP3UniAnalysis)
    {
        delete m_pP3UniAnalysis;
        m_pP3UniAnalysis = nullptr;
    }
    if(m_pP3LinAnalysis)
    {
        delete m_pP3LinAnalysis;
        m_pP3LinAnalysis = nullptr;
    }
}


void gl3dXPlaneView::resetglPOpp()
{
    s_bResetglOpp        = true;
    s_bResetglLift       = true;
    s_bResetglMoments    = true;
    s_bResetglDrag       = true;
    s_bResetglDownwash   = true;
    s_bResetglPanelGamma = true;
    s_bResetglPanelCp    = true;
    s_bResetglPanelForce = true;
    s_bResetglStream     = true;
    s_bResetglVortons    = true;

    m_bResetBoids = true;
    m_bResetFlowPanels = true;
}


void gl3dXPlaneView::setResultControls(POpp3dCtrls *pResults3DControls)
{
    m_pPOpp3dControls = pResults3DControls;
}


void gl3dXPlaneView::setSharedBuffers(gl3dXPlaneView *pgl3dXPlaneView)
{
    m_pglXPlaneBuffers = pgl3dXPlaneView->m_pglXPlaneBuffers;

    m_bAutoDeleteBuffers = false;
}


void gl3dXPlaneView::onPartSelClicked()
{
    Plane *pPlane = s_pXPlane->curPlane();
    if(!pPlane || !pPlane->isXflType()) return;
    PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(s_pXPlane->curPlane());

    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if(!pSenderBox) return;

    QVariant property = pSenderBox->property("partindex");
    if(property.isNull()) return;

    bool bOk = false;
    int index = property.toInt(&bOk);
    if(!bOk) return;

    if(index<pPlaneXfl->nWings())
    {
        WingXfl * pWing = pPlaneXfl->wing(index);
        if(pWing) pWing->setVisible(pSenderBox->isChecked());
    }
    else if(index<pPlaneXfl->nWings()+pPlaneXfl->nFuse())
    {
        Fuse *pFuse = pPlaneXfl->fuse(index-pPlaneXfl->nWings());
        if(pFuse) pFuse->setVisible(pSenderBox->isChecked());
    }

    // the geometry does not need to be rebuilt
    s_pXPlane->updateVisiblePanels();

    s_bResetglMesh = true;
    s_bResetglPanelCp = s_bResetglPanelGamma = s_bResetglPanelForce = true;
    s_bResetglStream = true;

    update();
}


void gl3dXPlaneView::setPlane(Plane const *pPlane)
{
    updatePartFrame(pPlane);
    if(pPlane)
    {
        if(s_pXPlane->curPlPolar())
            setBotLeftOutput(pPlane->planeData(s_pXPlane->curPlPolar()->bIncludeOtherWingAreas()));
        else
            setBotLeftOutput(pPlane->planeData(true));
    }
    else
        clearBotLeftOutput();

    resizeLabels();
}


void gl3dXPlaneView::glRenderPanelBasedBuffers()
{
    Plane      const *pPlane    = s_pXPlane->curPlane();
    PlanePolar const *pPlPolar  = s_pXPlane->curPlPolar();
    PlaneOpp   const *pPOpp     = s_pXPlane->curPOpp();

    bool bBackGround = !m_bSurfaces;
    if(m_pPOpp3dControls->m_b3dCp  && pPOpp) bBackGround = false;
    if(m_pPOpp3dControls->m_bGamma && pPOpp) bBackGround = false;

    if(pPOpp && pPOpp->hasVortons())
    {
        if(s_bVortonLine)
        {
            paintSegments(m_pglXPlaneBuffers->m_vboVortonLines, W3dPrefs::s_StreamStyle);
        }
        if(s_bNegVortices)
        {
            double coef = 0.1;
            for(uint iv=0; iv<pPOpp->m_VortexNeg.size(); iv++)
            {
                Vortex const &vtx = pPOpp->m_VortexNeg.at(iv);
                paintBox(vtx.vertexAt(0).x, vtx.vertexAt(0).y, vtx.vertexAt(0).z,
                         0.015/m_glScalef, 0.015/m_glScalef, 0.015/m_glScalef, QColor(255,35,75), true);
                paintThinArrow(vtx.vertexAt(0), vtx.segment()*vtx.circulation()*coef, QColor(255,35,75), 2.0f, Line::SOLID, m_matModel);
            }
        }

        if(m_pPOpp3dControls->m_bVortons)
        {
            paintSphereInstances(m_pglXPlaneBuffers->m_vboVortons,
                                 W3dPrefs::vortonRadius()/double(m_glScalef), W3dPrefs::vortonColour(), false, true);
        }

        if(m_pCrossFlowCtrls && m_pCrossFlowCtrls->bVorticityMap() && pPOpp->m_Vorton.size())
        {
            paintColourMap(m_pglXPlaneBuffers->m_vboContourClrs, m_matModel);
            paintSegments(m_pglXPlaneBuffers->m_vboContourLines, W3dPrefs::s_ContourLineStyle);
        }
    }

    if(m_bMeshPanels && pPlPolar)
    {
        if(pPlane->isXflType() && pPlPolar->isQuadMethod())
        {
            PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(s_pXPlane->curPlane());

            if(bBackGround) paintTriPanels(m_pglXPlaneBuffers->m_vboMesh,true);
            paintSegments(m_pglXPlaneBuffers->m_vboMeshEdges, W3dPrefs::s_PanelStyle);

            if(m_bPanelNormals)
                paintNormals(m_pglXPlaneBuffers->m_vboPanelNormals);
            if(pPlPolar->isVLM() && m_bVortices)
            {
                paintSegments(m_pglXPlaneBuffers->m_vboVortices, QColor(25, 195,135), 3.0f, Line::SOLID, false);
                for(uint i4=0; i4<pPlaneXfl->quadpanels().size(); i4++)
                {
                    Vector3d const &p = pPlaneXfl->panel4(i4).ctrlPt(pPlPolar->isVLM());
                    paintCube(p.x, p.y, p.z, 0.015/m_glScalef, QColor(25, 195,135), true);
                }
            }
        }
        else if(pPlPolar->isTriangleMethod())
        {
            if(bBackGround)
            {
                bool bFrontAndBack = pPlPolar && pPlPolar->bThinSurfaces();
                paintTriPanels(m_pglXPlaneBuffers->m_vboMesh, bFrontAndBack);
            }
            paintSegments(m_pglXPlaneBuffers->m_vboMeshEdges, W3dPrefs::s_PanelStyle);

            if(m_bPanelNormals || m_bNodeNormals)
                paintNormals(m_pglXPlaneBuffers->m_vboPanelNormals);

            if(m_PanelHightlight.size())
            {
                paintSegments(m_vboHighlightPanel3, W3dPrefs::s_HighStyle);
                for(int ih=0; ih<m_PanelHightlight.size(); ih++)
                {
                    int idx = m_PanelHightlight.at(ih);
                    if(idx<pPlane->triMesh().panelCount())
                    {
                        Panel3 const &p3 = pPlane->triMesh().panelAt(idx);
                        glRenderText(p3.CoG().x+0.03/double(m_glScalef), p3.CoG().y+0.03/double(m_glScalef), p3.CoG().z+0.03/double(m_glScalef),
                                     QString::asprintf("T%d", idx),
                                     DisplayOptions::textColor(), true);
                    }
                }
            }
        }
    }

    if(m_pPOpp3dControls->m_bWakePanels)
    {
        if(pPlPolar)
        {
            if(pPlPolar->isLinearPolar()/* && !pPOpp*/)
            {
                if(pPlPolar->isQuadMethod())
                {
                    paintTriPanels(m_pglXPlaneBuffers->m_vboWakePanels, true);
                    paintSegments(m_pglXPlaneBuffers->m_vboWakeEdges, W3dPrefs::s_PanelStyle);
                }
                else if(pPlPolar->isTriangleMethod())
                {
                    paintTriPanels(m_pglXPlaneBuffers->m_vboWakePanels, true);
                    paintSegments(m_pglXPlaneBuffers->m_vboWakeEdges, W3dPrefs::s_PanelStyle);
                }
            }
            else if(pPlPolar->isType6())
            {
                // wake panels should be aligned with the x-axis for other polar types
                if(pPlPolar->isQuadMethod())
                {
                    paintTriPanels(m_pglXPlaneBuffers->m_vboWakePanels, true);
                    paintSegments(m_pglXPlaneBuffers->m_vboWakeEdges, W3dPrefs::s_PanelStyle);
                }
                else if(pPlPolar->isTriangleMethod())
                {
                    paintTriPanels(m_pglXPlaneBuffers->m_vboWakePanels, true);
                    paintSegments(m_pglXPlaneBuffers->m_vboWakeEdges, W3dPrefs::s_PanelStyle);
                }
            }
        }
    }

    if(pPOpp)
    {
        if(m_pPOpp3dControls->m_b3dCp)
        {
            if(pPOpp->isQuadMethod())
                paintColourMap(m_pglXPlaneBuffers->m_vboCp, m_matModel); // two triangles/quad
            else if(pPOpp->isTriangleMethod())
            {
                paintColourMap(m_pglXPlaneBuffers->m_vboCp, m_matModel);
            }
        }
        else if(m_pPOpp3dControls->m_bGamma)
        {
            if(pPOpp->isQuadMethod())
                paintColourMap(m_pglXPlaneBuffers->m_vboGamma, m_matModel); // two triangles/quad
            else if(pPOpp->isTriangleMethod())
                paintColourMap(m_pglXPlaneBuffers->m_vboGamma, m_matModel);
        }

        if(m_pPOpp3dControls->m_bPanelForce && (pPOpp->isPanelMethod() || pPOpp->isVLMMethod()))
        {
            paintColorSegments(m_pglXPlaneBuffers->m_vboPanelForces, 2.0f, Line::SOLID);
        }

        if(m_pPOpp3dControls->m_bMoments)
        {
            paintMoments();
        }
    }

    if(isPicking() && m_PickedPanelIndex>=0)
    {
        if(pPlPolar && pPlPolar->isTriangleMethod())
        {
            if(pPlPolar->isTriUniformMethod())
            {
                paintTriangle(m_vboTriangle, true, false, Qt::black);
            }
            else if (pPlPolar->isTriLinearMethod())
            {
                if(pPOpp && (m_pPOpp3dControls->m_b3dCp || m_pPOpp3dControls->m_bGamma || m_pPOpp3dControls->m_bPanelForce))
                    paintSphere(m_PickedPoint, 0.0075/double(m_glScalef), Qt::red, true);
//                else paintTriangle(true, false, Qt::black);
            }
        }
        else // if (pWPolar->isQuadMethod())
            paintQuad(Qt::black, true, 1.0f, true, false, false, m_vboPickedQuad);

        QString strong;
        if(pPOpp && (m_pPOpp3dControls->m_b3dCp || m_pPOpp3dControls->m_bGamma || m_pPOpp3dControls->m_bPanelForce))
        {
            if     (pPOpp->isTriUniformMethod()) strong = QString::asprintf("T%d: %g", m_PickedPanelIndex, m_PickedVal);
            else if(pPOpp->isTriLinearMethod())  strong = QString::asprintf("N%d: %g", m_PickedNodeIndex,  m_PickedVal);
            else if(pPOpp->isQuadMethod())       strong = QString::asprintf("Q%d: %g", m_PickedPanelIndex, m_PickedVal);
        }
        else if(pPlPolar)
        {
            if(pPlPolar->isTriangleMethod())  strong = QString::asprintf("T%d", m_PickedPanelIndex);
            //            else if(pWPolar->isTriLinearMethod()) strong = QString::asprintf("N%d", m_PickedIndex);
            else if(pPlPolar->isQuadMethod()) strong = QString::asprintf("Q%d", m_PickedPanelIndex);
        }
        if(pPlPolar || pPOpp)
            glRenderText(m_PickedPoint.x+0.03/double(m_glScalef), m_PickedPoint.y+0.03/double(m_glScalef), m_PickedPoint.z+0.03/double(m_glScalef),
                         strong, DisplayOptions::textColor(), true);
    }

    if(m_pCrossFlowCtrls && m_pCrossFlowCtrls->bGridVelocity() && pPOpp)
    {
        paintSegments(m_pglXPlaneBuffers->m_vboGridVelocities, W3dPrefs::s_VelocityStyle);
    }
}


/**
 * @brief renders the buffer objects which are based on the plane's geometry
 */
void gl3dXPlaneView::glRenderGeometryBasedBuffers()
{
    Plane const *pPlane  = s_pXPlane->curPlane();
    if(!pPlane) return;

    if(m_bShowMasses) glDrawMasses(pPlane);
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            if(ifuse>=m_pglXPlaneBuffers->m_vboFuseTriangulation.size()) break;
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);

            if(pFuse->isVisible())
            {
                if(pFuse->isXflType())
                {
                    if(m_bSurfaces)
                    {
                        paintTriangles3Vtx(m_pglXPlaneBuffers->m_vboFuseTriangulation[ifuse], pFuse->color(), false, isLightOn());
//                        if(m_bTessellation)             paintNormals(m_pglXPlaneBuffers->m_vboTessNormals);
                    }
                    if(m_bOutline)
                    {
                        paintSegments(m_pglXPlaneBuffers->m_vboBodyOutline[ifuse], W3dPrefs::s_OutlineStyle);
                        if(pFuse->isSplineType() || pFuse->isSectionType())
                            paintSegments(m_pglXPlaneBuffers->m_vboFrames, W3dPrefs::s_OutlineStyle);
                    }
                }
                else if(pFuse->isOccType())
                {
                    if(m_bSurfaces)
                        paintTriangles3Vtx(m_pglXPlaneBuffers->m_vboFuseTriangulation[ifuse], pFuse->color(), false, isLightOn());
                    if(m_bOutline)
                        paintSegments(m_pglXPlaneBuffers->m_vboBodyOutline[ifuse], W3dPrefs::s_OutlineStyle);
                }
                else if(pFuse->isStlType())
                {
                    if(m_bSurfaces)
                        paintTriangles3Vtx(m_pglXPlaneBuffers->m_vboFuseTriangulation[ifuse], pFuse->color(), false, isLightOn());
                    if(m_bOutline)
                        paintSegments(m_pglXPlaneBuffers->m_vboBodyOutline[ifuse], W3dPrefs::s_OutlineStyle);
                }
            }
        }

        if(m_pPOpp3dControls->m_bFlaps)
        {
            paintFlaps(pPlaneXfl, s_pXPlane->curPlPolar(), s_pXPlane->curPOpp());
        }

        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
            if(pWing && pWing->isVisible())
            {
                if(iw<m_pglXPlaneBuffers->m_vboWingSurface.size())
                {
                    if(m_bSurfaces)  paintTriangles3Vtx(m_pglXPlaneBuffers->m_vboWingSurface[iw], pWing->color(), false, isLightOn());

                    if(m_bOutline)   paintSegments(m_pglXPlaneBuffers->m_vboWingOutline[iw], W3dPrefs::s_OutlineStyle);

                    if(m_bFoilNames) paintFoilNames(pWing);
                }
            }
        }
    }
    else if(pPlane->isSTLType())
    {
        if(m_bSurfaces)
        {
            PlaneSTL const * pPlaneSTL = dynamic_cast<PlaneSTL const*>(pPlane);
            paintTriangles3Vtx(m_pglXPlaneBuffers->m_vboPlaneStlTriangulation,
                               pPlaneSTL->surfaceColor(), false, isLightOn());
        }
        if(m_bOutline) paintSegments(m_pglXPlaneBuffers->m_vboPlaneStlOutline, W3dPrefs::s_OutlineStyle);
    }
}


void gl3dXPlaneView::glRenderPOppBasedBuffers()
{
    Plane    const *pPlane  = s_pXPlane->curPlane();
    PlanePolar   const *pWPolar = s_pXPlane->curPlPolar();
    PlaneOpp const *pPOpp   = s_pXPlane->curPOpp();
    if(!pPlane || !pWPolar || !pPOpp) return;

    if(pPlane->isXflType())
    {
        PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        if(m_pPOpp3dControls->m_bXTop || m_pPOpp3dControls->m_bXBot)
            paintSegments(m_pglXPlaneBuffers->m_vboTrans,     W3dPrefs::s_TransStyle);
        if(m_pPOpp3dControls->m_bDownwash)
        {
            paintSegments(m_pglXPlaneBuffers->m_vboDownwash,        W3dPrefs::s_VelocityStyle);
        }
        if(m_pPOpp3dControls->m_bLiftStrip) paintSegments(m_pglXPlaneBuffers->m_vboStripLift,   W3dPrefs::s_LiftStyle);
        if(m_pPOpp3dControls->m_bICd)       paintSegments(m_pglXPlaneBuffers->m_vboInducedDrag, W3dPrefs::s_IDragStyle);
        if(m_pPOpp3dControls->m_bVCd)       paintSegments(m_pglXPlaneBuffers->m_vboViscousDrag, W3dPrefs::s_VDragStyle);

        if(s_pXPlane->isCpView())
        {
            // node normal has Cp amplitude
            for(uint i=0; i<m_CpSections.size(); i++)
                paintThinArrow(m_CpSections.at(i), m_CpSections.at(i).normal(), W3dPrefs::s_LiftStyle);
        }

        if(m_pPOpp3dControls->m_bPartForces)
        {
            double sc = Opp3dScalesCtrls::partForceScale() / 10000.0;;
            // global force
            double qDyn = 0.5*pWPolar->density()*pPOpp->QInf()*pPOpp->QInf();
            Vector3d windD(objects::windDirection(pPOpp->alpha(), pPOpp->beta()));
            Vector3d force(pPOpp->m_AF.Fff() + windD * pPOpp->m_AF.viscousDrag());
            paintThickArrow(pPOpp->m_AF.centreOfPressure(), force*qDyn*sc, xfl::fromfl5Clr(W3dPrefs::s_LiftStyle.m_Color), m_matModel);

            for(int iw=0; iw<pPOpp->nWOpps(); iw++)
            {
                WingOpp const &wopp = pPOpp->WOpp(iw);
                force.set(wopp.m_AF.Fff()+ windD * wopp.m_AF.viscousDrag());
                paintThinArrow(pPlaneXfl->rootQuarterPoint(iw), force*qDyn*sc,
                               W3dPrefs::s_LiftStyle.m_Color, W3dPrefs::s_LiftStyle.m_Width, W3dPrefs::s_LiftStyle.m_Stipple, m_matModel);
            }
        }
    }

    if(m_pPOpp3dControls->m_bStreamLines && !pPOpp->isLLTMethod())
    {
        if(!s_bResetglStream)
            paintStreamLines(m_pglXPlaneBuffers->m_vboStreamLines, m_StreamLineColours, StreamLineCtrls::nX());
    }
}


void gl3dXPlaneView::glRenderView()
{
    QMatrix4x4 modeMatrix;
    Plane    const *pPlane  = s_pXPlane->curPlane();
    PlanePolar   const *pWPolar = s_pXPlane->curPlPolar();
    PlaneOpp const *pPOpp   = s_pXPlane->curPOpp();

    if(!pPlane) return;

    if(W3dPrefs::s_bShowRefLength) drawReferenceLength();
    if(pWPolar && pWPolar->isStabilityPolar())
    {
        if(pPOpp && pPOpp->isType7())
        {
            QString strong = QString::asprintf("Time = %9.5f s", m_pPOpp3dControls->m_pStab3dCtrls->modeTime());
            setTopRightOutput(strong);
        }

        double const *mode = m_pPOpp3dControls->m_pStab3dCtrls->modeState();

        modeMatrix.translate(float(mode[0]), float(mode[1]), float(mode[2]));
        modeMatrix.rotate(float(mode[3]*180.0/PI), 1.0f, 0.0f , 0.0f);
        modeMatrix.rotate(float(mode[4]*180.0/PI), 0.0f, 1.0f , 0.0f);
        modeMatrix.rotate(float(mode[5]*180.0/PI), 0.0f, 0.0f , 1.0f);
    }

    m_matModel = modeMatrix;
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

//    m_pvmMatrix = m_matProj * m_matView * m_matModel;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    if(!m_pglXPlaneBuffers->m_LiveVortons.size())
        glRenderPanelBasedBuffers();
    else
    {
        paintSphereInstances(m_pglXPlaneBuffers->m_vboVortons,
                             W3dPrefs::vortonRadius()/double(m_glScalef), W3dPrefs::vortonColour(), false, true);
    }

    glRenderPOppBasedBuffers();

    if(pPOpp && m_pPOpp3dControls->isFlowActive()) glRenderFlow();
    else m_FlowTimer.stop();

    // We use the model matrix to apply alpha and beta rotations to the geometry.
    if(m_pglXPlaneBuffers->m_LiveVortons.size())
    {
        //apply aoa rotation
        m_matModel.rotate(float(m_pglXPlaneBuffers->m_LiveAlpha),0.0f,1.0f,0.0f);
    }
    else if(pPOpp)
    {
        //apply aoa rotation
        m_matModel.rotate(float(pPOpp->alpha()),0.0,1.0,0.0);

        // apply sideslip
        if(fabs(pPOpp->beta())>ANGLEPRECISION)
        {
            m_matModel.rotate(pPOpp->beta(), 0.0f, 0.0f, 1.0f);
        }
    }


    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  m_matView);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView);
    }
    m_shadLine.release();
    if(m_Segments.size())
        paintSegments(m_vboSegments, W3dPrefs::s_HighStyle, true);


    vmMat = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;


    glRenderGeometryBasedBuffers();

    // restore unit identity model matrix for panel-ray intersections
    m_matModel.setToIdentity();
//        m_pvmMatrix = m_matProj * m_matView * m_matModel;


    if(bPickNode() && m_bMeshPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<pPlane->triMesh().nNodes())
        {
            Node const &nd = pPlane->node(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<pPlane->nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = pPlane->node(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }
        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<pPlane->nNodes())
        {
            Node const &nd = pPlane->node(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }

    // paint the ground before rotation by aoa
    if(pWPolar && pWPolar->bHPlane() && m_pPOpp3dControls->m_bHPlane)
    {
        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView);
        }
        m_shadSurf.release();

        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  m_matView);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView);
        }
        m_shadLine.release();

        paintQuad(W3dPrefs::s_WaterColor, true, 2.0f, false, true, true, m_vboHPlane);
    }

    paintMeasure();

/*
#ifdef QT_DEBUG
    m_stackInterval.push_back(QTime::currentTime().msecsSinceStartOfDay());

    double average = 0.0;
    for(int i=0; i<m_stackInterval.size()-1; i++)
        average += m_stackInterval.at(i+1)-m_stackInterval.at(i);
    average /= double(m_stackInterval.size()-1);

    setOutputInfo(QString::asprintf("FPS = %4.1f Hz", 1000.0/average), false);

    m_stackInterval.pop_front();
#endif
*/

    m_matModel.setToIdentity();
#ifdef QT_DEBUG
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  m_matView);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView);
    }
    m_shadLine.release();

    for(uint i=0; i<PanelAnalysis::s_DebugPts.size(); i++)
        paintIcosahedron(PanelAnalysis::s_DebugPts.at(i), 0.0075/m_glScalef, Qt::darkCyan, W3dPrefs::s_OutlineStyle, true, true);

    for(uint i=0; i<PanelAnalysis::s_DebugVecs.size(); i++)
    {
        if(i<PanelAnalysis::s_DebugPts.size())
            paintThinArrow(PanelAnalysis::s_DebugPts.at(i), PanelAnalysis::s_DebugVecs.at(i)*gl3dXflView::s_VelocityScale,
                           QColor(135,195,95).darker(), 2, Line::SOLID, m_matModel);
    }

    for(uint i=0; i<Surface::s_DebugPts.size(); i++)
        paintIcosahedron(Surface::s_DebugPts.at(i), 0.0075/m_glScalef, Qt::darkRed, W3dPrefs::s_OutlineStyle, true, true);
    for(uint i=0; i<Surface::s_DebugVecs.size(); i++)
    {
        if(i<Surface::s_DebugPts.size())
            paintThinArrow(Surface::s_DebugPts.at(i), Surface::s_DebugVecs.at(i)*gl3dXflView::s_VelocityScale,
                           QColor(135,195,95).darker(), 2, Line::SOLID, m_matModel);
    }
    for(uint i=0; i<TriMesh::s_DebugPts.size(); i++)
        paintIcosahedron(TriMesh::s_DebugPts.at(i), 0.0075/m_glScalef, Qt::darkYellow, W3dPrefs::s_OutlineStyle, true, true);
#endif

}


void gl3dXPlaneView::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bShift = (pEvent->modifiers() & Qt::ShiftModifier);
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);

    if(pEvent->key()==Qt::Key_T)
    {
        if(bCtrl)
        {
            m_bTessellation = !m_bTessellation;
            update();
        }
    }
    gl3dXflView::keyPressEvent(pEvent);
}


void gl3dXPlaneView::contextMenuEvent(QContextMenuEvent * pEvent)
{
    QPoint ScreenPt = pEvent->globalPos();
    m_bArcball = false;
    update();

    if (s_pXPlane->is3dView())
    {
        s_pXPlane->m_pMenus->m_p3dCtxMenu->exec(ScreenPt);
    }
}


void gl3dXPlaneView::initializeGL()
{
    gl3dXflView::initializeGL();

    int oglversion = 10*oglMajor()+oglMinor();
    if(oglversion<43)
    {
        QString strange = QString::asprintf("OpenGL context version is %d.%d.\n"
                                            "Set an OpenGL context version >=4.3 to enable flow animations.\n\n", oglMajor(), oglMinor());
        xfl::trace(strange);
        s_pXPlane->displayMessage(strange, true, false);
        return;
    }

    // flow Compute shader
    QString csrc = ":/shaders/flow/flow3d_CS.glsl";
    m_shadFlow.addShaderFromSourceFile(QOpenGLShader::Compute, csrc);
    if(m_shadFlow.log().length())
    {
        QString strange = QString::asprintf("%s", QString("Compute shader log:"+m_shadFlow.log()).toStdString().c_str());
        xfl::trace(strange);
        qDebug()<<"ComputeShader log:";
        qDebug()<<strange;
    }

    if(!m_shadFlow.link())
    {
        QString strange("Compute shader for flow animations is not linked.\nSet an OpenGL context version >=4.3 to enable compute shaders.\n\n");
        xfl::trace(strange);
//        qDebug()<<strange;
        s_pXPlane->displayMessage(strange, true, false);
    }
    else
    {
        m_shadFlow.bind();
        {
            m_shadFlowLoc.m_RK           = m_shadFlow.uniformLocation("RK");
            m_shadFlowLoc.m_NPanels      = m_shadFlow.uniformLocation("npanels");
            m_shadFlowLoc.m_Dt           = m_shadFlow.uniformLocation("dt");
            m_shadFlowLoc.m_VInf         = m_shadFlow.uniformLocation("vinf");
            m_shadFlowLoc.m_TopLeft      = m_shadFlow.uniformLocation("topleft");
            m_shadFlowLoc.m_BotRight     = m_shadFlow.uniformLocation("botright");
            m_shadFlowLoc.m_VtnCoreSize  = m_shadFlow.uniformLocation("VtnCoreSize");
            m_shadFlowLoc.m_NVorton      = m_shadFlow.uniformLocation("nvortons");
            m_shadFlowLoc.m_UniColor     = m_shadFlow.uniformLocation("UniformColor");
            m_shadFlowLoc.m_HasUniColor  = m_shadFlow.uniformLocation("HasUniColor");
            m_shadFlowLoc.m_HasGround    = m_shadFlow.uniformLocation("HasGround");
            m_shadFlowLoc.m_GroundHeight = m_shadFlow.uniformLocation("GroundHeight");
        }
        m_shadFlow.release();
    }
}


void gl3dXPlaneView::resizeGL(int w, int h)
{
    gl3dXflView::resizeGL(w, h);

    m_ColourLegend.resize(100, (height()*2)/3, devicePixelRatioF());
    m_ColourLegend.makeLegend();
}


void gl3dXPlaneView::hideEvent(QHideEvent *pEvent)
{
    onCancelThreads();
    m_pPOpp3dControls->m_bStreamLines = false;

    cancelFlow();

    gl3dXflView::hideEvent(pEvent);
}


bool gl3dXPlaneView::glMakeStreamLines(const std::vector<Panel3> &panel3list, std::vector<Node>const &nodelist, PlaneOpp const *pPOpp)
{
    if(m_pglXPlaneBuffers->m_vboStreamLines.isCreated()) m_pglXPlaneBuffers->m_vboStreamLines.destroy();
    if(!pPOpp || pPOpp->isLLTMethod()) return false;

    if(!s_pXPlane->curPlane()) return false;

    Plane const *pPlane = s_pXPlane->curPlane();
    PlaneXfl const * pPlaneXfl = nullptr;
    if(pPlane->isXflType()) pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    StreamlineMaker::cancelTasks(false);

    Vector3d C, VA, TC;

    VA.set(cos(pPOpp->alpha()*PI/180.0), 0, -sin(pPOpp->alpha()*PI/180.0));

    QVector<int> pointindexes;
    QVector<Vector3d> points;
    QVector<Vector3d> v0List;
    m_StreamLineColours.clear();

    if (StreamLineCtrls::startAtTE())
    {
        for(uint i3=0; i3<panel3list.size(); i3++)
        {
            Panel3 const &p3 = panel3list.at(i3);

            if(p3.isTrailing() && p3.surfacePosition()<=xfl::MIDSURFACE)
            {
                if(!pointindexes.contains(p3.nodeIndex(1)))
                {
                    pointindexes.append(p3.nodeIndex(1));
                    // since this point falls by construction on a wake line, move
                    // its endpoint slightly outwards, and reduce the vector's length
                    VA = p3.TELeftBisector();
    //                VA*= s_pXPlane->m_pCurPlane->rootChord()/50.0;
                    v0List.append(VA);
                    if(pPlaneXfl && W3dPrefs::s_bUseWingColour)
                    {
                        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                        {
                            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
                            if(pWing->hasPanel3(i3))
                            {
                                m_StreamLineColours.append(xfl::fromfl5Clr(pWing->color()));
                                break;
                            }
                        }
                    }
                }
                if(!pointindexes.contains(p3.nodeIndex(2)))
                {
                    pointindexes.append(p3.nodeIndex(2));
                    // since this point falls by construction on a wake line, move
                    // its endpoint slightly outwards, and reduce the vector's length
                    VA = p3.TELeftBisector();
    //                VA*= s_pXPlane->m_pCurPlane->rootChord()/50.0;
                    v0List.append(VA);
                    if(pPlaneXfl && W3dPrefs::s_bUseWingColour)
                    {
                        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                        {
                            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
                            if(pWing->hasPanel3(i3))
                            {
                                m_StreamLineColours.append(xfl::fromfl5Clr(pWing->color()));
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(StreamLineCtrls::startAtYLine())
    {
        double y0 = 0;
        int n= StreamLineCtrls::nStreamLines();
        if(isOdd(n)) n++;
        double range = objectReferenceLength();

        for(int i=0; i<n/2; i++)
        {
            points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range + y0, StreamLineCtrls::ZOffset()*range});
            v0List.append(VA);
            if(i>0)
            {
                points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range - y0, StreamLineCtrls::ZOffset()*range});
                v0List.append(VA);
            }
            y0 += StreamLineCtrls::deltaL();
        }
    }
    else if(StreamLineCtrls::startAtZLine())
    {
        double z0 = 0;
        int n= StreamLineCtrls::nStreamLines();
        if(isOdd(n)) n++;
        double range = objectReferenceLength();

        for(int i=0; i<n/2; i++)
        {
            points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range, StreamLineCtrls::ZOffset()*range + z0});
            v0List.append(VA);
            if(i>0)
            {
                points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range, StreamLineCtrls::ZOffset()*range - z0});
                v0List.append(VA);
            }
            z0 += StreamLineCtrls::deltaL();
        }
    }

    int m_NStreamLines = 0;
    if(StreamLineCtrls::startAtTE())
        m_NStreamLines = pointindexes.size();
    else
        m_NStreamLines = points.size();

    int streamArraySize = m_NStreamLines * (StreamLineCtrls::nX()+1) * 3;
    QVector<float> StreamVertexArray(streamArraySize);

    int iv = 0;
    StreamlineMaker::cancelTasks(false);

    if(s_pXPlane->curPlPolar()->isTriUniformMethod())
    {
        m_pP3UniAnalysis->initializeAnalysis(s_pXPlane->curPlPolar(),0);
        m_pP3UniAnalysis->setTriMesh(s_pXPlane->curPlane()->triMesh());
        m_pP3UniAnalysis->setVortons(pPOpp->m_Vorton);
    }
    else if(s_pXPlane->curPlPolar()->isTriLinearMethod())
    {
        m_pP3LinAnalysis->initializeAnalysis(s_pXPlane->curPlPolar(),0);
        m_pP3LinAnalysis->setTriMesh(s_pXPlane->curPlane()->triMesh());
        m_pP3LinAnalysis->setVortons(pPOpp->m_Vorton);
    }
    m_pP3UniAnalysis->makeWakePanels(Vector3d(1.0, 0.0, 0.0), s_pXPlane->curPlPolar()->bVortonWake());


    // multithreaded mode only
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFutureSynchronizer<void> futureSync;
    QVector<StreamlineMaker*> makers;
    double range = objectReferenceLength();

    for (int in=0; in<m_NStreamLines; in++)
    {
        if(StreamLineCtrls::startAtTE())
        {
            C = TC = nodelist.at(pointindexes.at(in)) + Vector3d(StreamLineCtrls::XOffset()*range, 0.0, StreamLineCtrls::ZOffset()*range);
//                TC.rotateY(refPoint, pPOpp->alpha());
            TC -= C;
//                v0List[in].rotateY( pPOpp->alpha());
        }
        else if(StreamLineCtrls::startAtYLine())
        {
            C = points.at(in);// + Vector3d(StreamLinesCtrl::XOffset()*range, StreamLinesCtrl::YOffset()*range, StreamLinesCtrl::ZOffset()*range);
            TC.set(0,0,0);
        }
        else if(StreamLineCtrls::startAtZLine())
        {
            C = points.at(in);// + Vector3d(StreamLinesCtrl::XOffset()*range, StreamLinesCtrl::YOffset()*range, StreamLinesCtrl::ZOffset()*range);
            TC.set(0,0,0);
        }

        StreamlineMaker *pLineMaker = new StreamlineMaker;
        makers.append(pLineMaker);
        pLineMaker->setOpp(s_pXPlane->curPlPolar(), pPOpp->QInf(), 0.0, 0.0, pPOpp->gamma().data(), pPOpp->sigma().data());
        if     (s_pXPlane->curPlPolar()->isTriUniformMethod()) pLineMaker->setP3Analysis(m_pP3UniAnalysis);
        else if(s_pXPlane->curPlPolar()->isTriLinearMethod())  pLineMaker->setP3Analysis(m_pP3LinAnalysis);
        pLineMaker->initializeLineMaker(in, StreamVertexArray.data()+iv, C, v0List[in], TC,
                                        StreamLineCtrls::nX(), StreamLineCtrls::l0(), StreamLineCtrls::XFactor());

 #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        futureSync.addFuture(QtConcurrent::run(pLineMaker, &StreamlineMaker::run));

#else
            futureSync.addFuture(QtConcurrent::run(&StreamlineMaker::run, pLineMaker));
#endif
        iv += (StreamLineCtrls::nX()+1)*3;
    }
    futureSync.waitForFinished();

    for(int i=0; i<makers.size(); i++)
    {
        delete makers[i];
    }

    if(m_pglXPlaneBuffers->m_vboStreamLines.isCreated()) m_pglXPlaneBuffers->m_vboStreamLines.destroy();
    m_pglXPlaneBuffers->m_vboStreamLines.create();
    m_pglXPlaneBuffers->m_vboStreamLines.bind();
    m_pglXPlaneBuffers->m_vboStreamLines.allocate(StreamVertexArray.data(), StreamVertexArray.size()* int(sizeof(float)));
    m_pglXPlaneBuffers->m_vboStreamLines.release();

    QApplication::restoreOverrideCursor();

    return true;
}


bool gl3dXPlaneView::glMakeStreamLines(std::vector<Panel4> const &panel4list, PlaneOpp const *pPOpp)
{
    if(m_pglXPlaneBuffers->m_vboStreamLines.isCreated()) m_pglXPlaneBuffers->m_vboStreamLines.destroy();
    if(!pPOpp) return false;
    if(!s_pXPlane->curPlane() || !s_pXPlane->curPlane()->isXflType()) return false;

    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl*>(s_pXPlane->curPlane());

    StreamlineMaker::cancelTasks(false);

    Vector3d C, VA, TC, V0;
    //make the initial streamline vector from the freestream velocity
    VA.set(cos(pPOpp->alpha()*PI/180.0), 0, -sin(pPOpp->alpha()*PI/180.0));

    m_StreamLineColours.clear();

    QVector<Vector3d> points;
    QVector<Vector3d> v0List;

    if( StreamLineCtrls::startAtTE())
    {
        for(uint i4=0; i4<panel4list.size(); i4++)
        {
            Panel4 const &p4 = panel4list.at(i4);
            if(p4.isTrailing() && p4.surfacePosition()<=xfl::MIDSURFACE)
            {
                bool bFound = false;
                for(int k=0; k<points.size(); k++)
                {
                    if(points.at(k).isSame(p4.vertex(1)))
                    {
                        bFound=true;
                        break;
                    }
                }
                if(!bFound)
                {
                    points.append(p4.vertex(1));
                    // since this point falls by construction on a wake line, move
                    // its endpoint slightly outwards, and reduce the vector's length
                    VA = p4.TELeftBisector();
    //                VA*= s_pXPlane->m_pCurPlane->rootChord()/50.0;
                    v0List.append(VA);
                    if(W3dPrefs::s_bUseWingColour)
                    {
                        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                        {
                            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
                            if(pWing->hasPanel4(i4))
                            {
                                m_StreamLineColours.append(xfl::fromfl5Clr(pWing->color()));
                                break;
                            }
                        }
                    }

                }

                bFound = false;
                for(int k=0; k<points.size(); k++)
                {
                    if(points.at(k).isSame(p4.vertex(2)))
                    {
                        bFound=true;
                        break;
                    }
                }
                if(!bFound)
                {
                    points.append(p4.vertex(2));
                    // since this point falls by construction on a wake line, move
                    // its endpoint slightly outwards, and reduce the vector's length
                    VA = p4.TELeftBisector();
    //                VA*= s_pXPlane->m_pCurPlane->rootChord()/50.0;
                    v0List.append(VA);
                    if(W3dPrefs::s_bUseWingColour)
                    {
                        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                        {
                            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
                            if(pWing->hasPanel4(i4))
                            {
                                m_StreamLineColours.append(xfl::fromfl5Clr(pWing->color()));
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(StreamLineCtrls::startAtYLine() )
    {
        double range = objectReferenceLength();

        double y0 = 0;
        for(int i=0; i<StreamLineCtrls::nStreamLines()/2; i++)
        {
            points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range + y0, StreamLineCtrls::ZOffset()*range});
            v0List.append(VA);
            if(i>0)
            {
                points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range - y0, StreamLineCtrls::ZOffset()*range});
                v0List.append(VA);
            }
            y0 += StreamLineCtrls::deltaL();
        }
    }
    else if(StreamLineCtrls::startAtZLine())
    {
        double z0 = 0;
        double range = objectReferenceLength();

        for(int i=0; i<StreamLineCtrls::nStreamLines()/2; i++)
        {
            points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range, StreamLineCtrls::ZOffset()*range + z0});
            v0List.append(VA);
            if(i>0)
            {
                points.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range, StreamLineCtrls::ZOffset()*range - z0});
                v0List.append(VA);
            }
            z0 += StreamLineCtrls::deltaL();
        }
    }

    int m_NStreamLines = 0;
    if(StreamLineCtrls::startAtTE())
        m_NStreamLines = points.size();
    else
        m_NStreamLines = points.size();

    int streamArraySize =     m_NStreamLines * (StreamLineCtrls::nX()+1) * 3;
    QVector<float> StreamVertexArray(streamArraySize);

    int iv = 0;
    StreamlineMaker::cancelTasks(false);

    if(m_pP4Analysis->polar3d()!=s_pXPlane->curPlPolar() && s_pXPlane->curPlane()->isXflType())
    {
        PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl*>(s_pXPlane->curPlane());
        m_pP4Analysis->setQuadMesh(pPlaneXfl->refQuadMesh());
        m_pP4Analysis->initializeAnalysis(s_pXPlane->curPlPolar(), 0);
    }

    m_pP4Analysis->setVortons(pPOpp->m_Vorton);


    // multithreaded mode only
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFutureSynchronizer<void> futureSync;
    QVector<StreamlineMaker*> makers;
//    double range = gl3dScales::referenceLength();

    for (int in=0; in<m_NStreamLines; in++)
    {
        if(StreamLineCtrls::startAtTE())
        {
            C = TC = points.at(in);
//                TC.rotateY(refPoint, pPOpp->alpha());
            TC -= C;
//                v0List[in].rotateY( pPOpp->alpha());
        }
        else if(StreamLineCtrls::startAtYLine())
        {
            C = points.at(in);// + Vector3d(StreamLinesCtrl::XOffset()*range, StreamLinesCtrl::YOffset()*range, StreamLinesCtrl::ZOffset()*range);
            TC.set(0,0,0);
        }
        else if(StreamLineCtrls::startAtZLine())
        {
            C = points.at(in);
            TC.set(0,0,0);
        }

        StreamlineMaker *pLineMaker = new StreamlineMaker(this);
        pLineMaker->setOpp(s_pXPlane->curPlPolar(), pPOpp->QInf(), 0.0, 0.0, pPOpp->gamma().data(), pPOpp->sigma().data());
        pLineMaker->setP4Analysis(m_pP4Analysis);

        pLineMaker->initializeLineMaker(in, StreamVertexArray.data()+iv, C, V0, TC,
                                        StreamLineCtrls::nX(), StreamLineCtrls::l0(), StreamLineCtrls::XFactor());

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        futureSync.addFuture(QtConcurrent::run(pLineMaker, &StreamlineMaker::run));
#else
        futureSync.addFuture(QtConcurrent::run(&StreamlineMaker::run, pLineMaker));
#endif

        iv += (StreamLineCtrls::nX()+1)*3;
    }
    futureSync.waitForFinished();


    for(int i=0; i<makers.size(); i++)
    {
        delete makers[i];
    }

    if(m_pglXPlaneBuffers->m_vboStreamLines.isCreated()) m_pglXPlaneBuffers->m_vboStreamLines.destroy();
    m_pglXPlaneBuffers->m_vboStreamLines.create();
    m_pglXPlaneBuffers->m_vboStreamLines.bind();
    m_pglXPlaneBuffers->m_vboStreamLines.allocate(StreamVertexArray.data(), StreamVertexArray.size()* int(sizeof(float)));
    m_pglXPlaneBuffers->m_vboStreamLines.release();

    QApplication::restoreOverrideCursor();
    return true;
}


void gl3dXPlaneView::onCancelThreads()
{
    StreamlineMaker::cancelTasks(true);
}


void gl3dXPlaneView::computeP4VelocityVectors(Opp3d const *pPOpp, QVector<Vector3d> const &points, QVector<Vector3d> &velvectors, bool bMultithread)
{
    velvectors.resize(points.size());

    if(m_pP4Analysis->polar3d()!=s_pXPlane->curPlPolar())
    {
        PlaneXfl * pPlaneXfl = dynamic_cast<PlaneXfl*>(s_pXPlane->curPlane());
        m_pP4Analysis->setQuadMesh(pPlaneXfl->quadMesh());
        m_pP4Analysis->initializeAnalysis(s_pXPlane->curPlPolar(), 0);
    }
    m_pP4Analysis->setVortons(pPOpp->m_Vorton);


    if(bMultithread)
    {
        m_nBlocks = QThreadPool::globalInstance()->maxThreadCount();
        QFutureSynchronizer<void> futureSync;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &gl3dXPlaneView::makeQuadVelocityBlock,
                                                   iBlock, points, pPOpp->gamma().data(), pPOpp->sigma().data(), velvectors.data()));
#else
            futureSync.addFuture(QtConcurrent::run(&gl3dXPlaneView::makeQuadVelocityBlock, this,
                                                   iBlock, points, pPOpp->gamma().data(), pPOpp->sigma().data(), velvectors.data()));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        m_nBlocks = 1;
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeQuadVelocityBlock(iBlock, points, pPOpp->gamma().data(), pPOpp->sigma().data(), velvectors.data());
        }
        update();
    }
}


void gl3dXPlaneView::computeP3VelocityVectors(Opp3d const *pPOpp, QVector<Vector3d> const&points,  QVector<Vector3d> &velvectors, bool bMultithread)
{
    int nPoints = points.size();
    velvectors.resize(nPoints);

    Plane *pPlane = s_pXPlane->curPlane();
    if(s_pXPlane->curPlPolar()->isTriUniformMethod())
    {
        m_pP3UniAnalysis->setTriMesh(pPlane->triMesh());
        m_pP3UniAnalysis->initializeAnalysis(s_pXPlane->curPlPolar(),0);
        m_pP3UniAnalysis->setVortons(pPOpp->m_Vorton);
    }
    else if(s_pXPlane->curPlPolar()->isTriLinearMethod())
    {
        m_pP3LinAnalysis->setTriMesh(pPlane->triMesh());
        m_pP3LinAnalysis->initializeAnalysis(s_pXPlane->curPlPolar(),0);
        m_pP3LinAnalysis->setVortons(pPOpp->m_Vorton);
    }

    if(bMultithread)
    {
        m_nBlocks = QThreadPool::globalInstance()->maxThreadCount();
        QFutureSynchronizer<void> futureSync;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &gl3dXPlaneView::makeTriVelocityBlock,
                                                   iBlock, points, pPOpp->gamma().data(), pPOpp->sigma().data(), velvectors.data()));
#else
            futureSync.addFuture(QtConcurrent::run(&gl3dXPlaneView::makeTriVelocityBlock, this,
                                                   iBlock, points, pPOpp->gamma().data(), pPOpp->sigma().data(), velvectors.data()));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        m_nBlocks = 1;
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeTriVelocityBlock(iBlock, points, pPOpp->gamma().data(), pPOpp->sigma().data(), velvectors.data());
        }
        update();
    }
}


void gl3dXPlaneView::makeQuadVelocityBlock(int iBlock, QVector<Vector3d> const &C, double const *mu, double const*sigma, Vector3d *VField) const
{
    int blockSize = int(C.size()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = C.size();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d V;

    for (int ip=iStart; ip<iMax; ip++)
    {
        m_pP4Analysis->getVelocityVector(C.at(ip), mu, sigma, V, Vortex::coreRadius(), false, false);
        VField[ip] = V;
    }
}


void gl3dXPlaneView::makeTriVelocityBlock(int iBlock, QVector<Vector3d> const &C, double const *mu, double const *sigma, Vector3d *VField) const
{
    int blockSize = int(C.size()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = C.size();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d V;

    for (int ip=iStart; ip<iMax; ip++)
    {
        if (s_pXPlane->curPlPolar()->isTriUniformMethod())
            m_pP3UniAnalysis->getVelocityVector(C.at(ip), mu, sigma, V, Vortex::coreRadius(), false, false);
        else if(s_pXPlane->curPlPolar()->isTriLinearMethod())
            m_pP3LinAnalysis->getVelocityVector(C.at(ip), mu, sigma, V, Vortex::coreRadius(), false, false);

        VField[ip] = V;
    }
}


void gl3dXPlaneView::paintMoments()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, xfl::fromfl5Clr(W3dPrefs::s_MomentStyle.m_Color));
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(W3dPrefs::s_MomentStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_MomentStyle.m_Width));

        m_pglXPlaneBuffers->m_vboMoments.bind();

        m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

        int nLines = m_pglXPlaneBuffers->m_vboMoments.size()/int(sizeof(float))/3;

        glDrawArrays(GL_LINES, 0, nLines);
        m_pglXPlaneBuffers->m_vboMoments.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


bool gl3dXPlaneView::intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I)
{
    Plane const *pPlane = s_pXPlane->m_pCurPlane;
    if(!pPlane) return false;

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;
    float zmax = +1.e10;
    Vector3d INear, ITmp;
    bool bIntersect = false;
    for(int i3=0; i3<pPlane->nPanel3(); i3++)
    {
        Panel3 const &p3 = pPlane->panel3At(i3);
        if(p3.intersect(AA, U, ITmp))
        {
            worldToScreen(ITmp, v4d);
            if(v4d.z()<zmax)
            {
                zmax = v4d.z();
                I = ITmp;
                bIntersect = true;
            }
        }
    }

    PlanePolar const *pWPolar = s_pXPlane->curPlPolar();
    if(m_pPOpp3dControls->m_bWakePanels && pPlane && pWPolar && pWPolar->isType6() && s_pXPlane->m_pCurPOpp)
    {
        if(pWPolar->isTriangleMethod())
        {
            for(uint i3=0; i3<pPlane->triMesh().wakePanels().size(); i3++)
            {
                Panel3 const &p3 = pPlane->triMesh().wakePanelAt(i3);
                if(p3.intersect(AA, U, ITmp))
                {
                    worldToScreen(ITmp, v4d);
                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        I = ITmp;
                        bIntersect = true;
                    }
                }
            }
        }
        else if(pPlane->isXflType() && pWPolar->isQuadMethod())
        {
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            for(uint i4=0; i4<pPlaneXfl->quadMesh().wakePanels().size(); i4++)
            {
                Panel4 const &p4 = pPlaneXfl->quadMesh().wakePanel(i4);
                if(p4.intersect(AA, U, ITmp))
                {
                    worldToScreen(ITmp, v4d);
                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        I = ITmp;
                        bIntersect = true;
                    }
                }
            }
        }
    }
    return bIntersect;
}


void gl3dXPlaneView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

    if(draggeddistance>=5 || !isPicking())
    {
        gl3dXflView::mouseReleaseEvent(pEvent);
        return;
    }

    if(!s_pXPlane->m_pCurPlane || !s_pXPlane->m_pCurPlPolar)
    {
        m_NodePair = {-1,-1};
        return;
    }
    Plane const *pPlane = s_pXPlane->curPlane();
    PlanePolar const *pWPolar = s_pXPlane->curPlPolar();
    if(pWPolar->isQuadMethod()) pickQuadPanel(pEvent->pos());
    else
    {
        pickTriUniPanel(pEvent->pos());
        if(pWPolar->isTriLinearMethod() && m_PickedPanelIndex>=0 && m_PickedPanelIndex<pPlane->nPanel3())
        {
            Panel3 const &p3 = pPlane->panel3At(m_PickedPanelIndex);
            pickPanelNode(p3, m_PickedPoint, xfl::NOSURFACE);
        }
    }

    if(m_PickedPanelIndex>=0 && !bPickNode())
    {
        if(!pWPolar || pWPolar->isTriUniformMethod() || pWPolar->isQuadMethod())
        {
            s_pXPlane->outputPanelProperties(m_PickedPanelIndex);
        }
        else if(pWPolar && pWPolar->isTriLinearMethod())
        {
            if(s_pXPlane->curPOpp() && (m_pPOpp3dControls->m_b3dCp || m_pPOpp3dControls->m_bGamma || m_pPOpp3dControls->m_bPanelForce))
                s_pXPlane->outputNodeProperties(m_PickedNodeIndex, m_PickedVal);
            else
                s_pXPlane->outputPanelProperties(m_PickedPanelIndex);
        }
        emit panelSelected(m_PickedPanelIndex);
    }
    else if(bPickNode())
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
            //start again
            m_NodePair = {m_PickedNodeIndex, -1};
        }
    }
    else
    {
        gl3dXflView::mouseReleaseEvent(pEvent);
    }
}


void gl3dXPlaneView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex  = -1;
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    Plane  const *pPlane  = s_pXPlane->curPlane();
    PlanePolar const *pWPolar = s_pXPlane->curPlPolar();

    if(!isPicking() || !pPlane)
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex  = -1;
        return;
    }

    if(pWPolar && pWPolar->isQuadMethod())
    {
        pickQuadPanel(pEvent->pos());
    }
    else if(pWPolar)
    {
        pickTriUniPanel(pEvent->pos());
        if(pWPolar->isTriLinearMethod() && m_PickedPanelIndex>=0 && m_PickedPanelIndex<pPlane->nPanel3())
        {
            Panel3 const &p3 = pPlane->panel3At(m_PickedPanelIndex);
            pickPanelNode(p3, m_PickedPoint, xfl::NOSURFACE);
        }
    }
    else
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex  = -1;
    }

    if(m_PickedPanelIndex<0)
    {
        m_PickedNodeIndex  = -1;
        setTopRightOutput(QString());
        update();
    }
    else if(pWPolar && pWPolar->isQuadMethod())
    {
        if(pPlane->isXflType())
        {
            PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<pPlaneXfl->quadMesh().nPanels())
            {
                Panel4 const &p4 = pPlaneXfl->quadMesh().panelAt(m_PickedPanelIndex);
                gl::makeQuad(p4.vertex(0), p4.vertex(1), p4.vertex(2), p4.vertex(3), m_vboPickedQuad);
                setTopRightOutput(p4.properties(true));
            }
        }
    }
    else if(pWPolar && pWPolar->isTriangleMethod())
    {
        Panel3 const &p3 = pPlane->panel3At(m_PickedPanelIndex);
        if(bPickNode())
        {
            pickPanelNode(p3, m_PickedPoint, xfl::NOSURFACE);
            if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=pPlane->triMesh().nNodes())
            {
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
            }
            else
            {
                if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<pPlane->nNodes())
                {
                    Node const &nd = pPlane->node(m_PickedNodeIndex);
                    setTopRightOutput(QString::fromStdString(nd.properties()));
                 }
            }
        }
        else
        {
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<pPlane->nPanel3())
            {
                Panel3 const &p3 = pPlane->panel3At(m_PickedPanelIndex);
                gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);

                setTopRightOutput(p3.properties());
            }
        }
    }
    update();
}


bool gl3dXPlaneView::pickTriUniPanel(QPoint const &point)
{
    Vector3d I, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

//    Plane const *pPlane = s_pXPlane->m_pCurPlane;
    PlanePolar const *pWPolar = s_pXPlane->m_pCurPlPolar;
    PlaneOpp const *pPOpp = s_pXPlane->m_pCurPOpp;

    float zmax = LARGEVALUE;
    if(pPOpp && (m_pPOpp3dControls->m_b3dCp || m_pPOpp3dControls->m_bGamma || m_pPOpp3dControls->m_bPanelForce))
    {
        // pick element
//        for(int i3=0; i3<pPlane->nPanel3(); i3++)
//        {
//            Panel3 const &p3 = pPlane->panel3At(i3);
        for(uint i3=0; i3<m_Panel3Visible.size(); i3++)
        {
            Panel3 const &p3 = m_Panel3Visible.at(i3);
            int idx=p3.index();
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
                    m_PickedPoint = I;

                    if(m_pPOpp3dControls->m_bPanelForce)
                    {
                        double q = 0.5*pWPolar->density()*pPOpp->QInf()*pPOpp->QInf();
                        m_PickedVal =  pPOpp->Cp(idx*3)* q*Units::PatoUnit();
                    }
                    else if(m_pPOpp3dControls->m_bGamma)
                    {
                        m_PickedVal = pPOpp->gamma(idx*3);
                    }
                    else
                    {
                        m_PickedVal = pPOpp->Cp(idx*3);
                    }
                    m_PickedPanelIndex = p3.index();
                }
            }
        }
    }
    else
    {
/*        for(int i3=0; i3<pPlane->nPanel3(); i3++)
        {
            Panel3 const &p3 = pPlane->panel3At(i3);
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
//                    m_PickedPoint = p3.CoG();
                    m_PickedPoint = I;
                    m_PickedPanelIndex = i3;
                }
            }
        }*/
        for(uint i3=0; i3<m_Panel3Visible.size(); i3++)
        {
            Panel3 const &p3 = m_Panel3Visible.at(i3);
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
//                    m_PickedPoint = p3.CoG();
                    m_PickedPoint = I;
                    m_PickedPanelIndex = p3.index();
                }
            }
        }
    }

    return m_PickedPanelIndex>=0;
}


bool gl3dXPlaneView::pickQuadPanel(QPoint const &point)
{
    PlaneXfl * pPlaneXfl = dynamic_cast<PlaneXfl*>(s_pXPlane->curPlane());
    if(!pPlaneXfl) return false;

    Vector3d I, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

    PlaneOpp const *pPOpp = s_pXPlane->m_pCurPOpp;
    float zmax = +1.e10;
    //    double dcrit = 0.05/m_glScaled;
    if(pPOpp && (m_pPOpp3dControls->m_b3dCp || m_pPOpp3dControls->m_bGamma || m_pPOpp3dControls->m_bPanelForce))
    {
        if(pPOpp->isQuadMethod() && s_pXPlane->curPlane()->isXflType())
        {
            for(uint i4=0; i4<m_Panel4Visible.size(); i4++)
            {
                Panel4 const &p4 = m_Panel4Visible.at(i4);
                if(p4.intersect(AA, U, I))
                {
                    worldToScreen(p4.CoG(), v4d);

                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        m_PickedPoint = p4.CoG();

                        if(m_pPOpp3dControls->m_bPanelForce)
                        {
                            m_PickedVal =  pPOpp->Cp(p4.index())* 0.5*pPOpp->QInf()*pPOpp->QInf()*Units::PatoUnit();
                        }
                        else if(m_pPOpp3dControls->m_bGamma)
                        {
                            m_PickedVal = pPOpp->gamma(p4.index());
                        }
                        else
                        {
                            m_PickedVal = pPOpp->Cp(p4.index());
                        }

                        m_PickedPanelIndex = p4.index();
                    }
                }
            }
        }
    }
    else
    {
        // pick a panel;
        if(s_pXPlane->m_pCurPlPolar->isQuadMethod() && s_pXPlane->curPlane()->isXflType())
        {
            PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(s_pXPlane->curPlane());

            for(int i4=0; i4<pPlaneXfl->quadMesh().nPanels(); i4++)
            {
                Panel4 const &p4 = pPlaneXfl->quadMesh().panelAt(i4);
                if(p4.intersect(AA, U, I))
                {
                    worldToScreen(p4.CoG(), v4d);
                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        m_PickedPoint = p4.CoG();
                        m_PickedPanelIndex = p4.index();
                    }
                }
            }
        }
    }

    return m_PickedPanelIndex>=0;
}


void gl3dXPlaneView::glMake3dObjects()
{
    if(!s_pXPlane->m_pCurPlane) return;
    Plane    *pPlane  = s_pXPlane->curPlane();
    PlanePolar   const *pWPolar = s_pXPlane->curPlPolar();
//    PlaneOpp const *pPOpp   = s_pXPlane->curPOpp();

    if(s_bResetglGeom)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        if(pPlane->isXflType())
        {
            PlaneXfl *pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
            // initialize buffer arrays
            for(int i=0; i<m_pglXPlaneBuffers->m_vboWingOutline.size(); i++)
            {
                if(m_pglXPlaneBuffers->m_vboWingOutline[i].isCreated()) m_pglXPlaneBuffers->m_vboWingOutline[i].destroy();
            }
            for(int i=0; i<m_pglXPlaneBuffers->m_vboWingSurface.size(); i++)
            {
                if(m_pglXPlaneBuffers->m_vboWingSurface[i].isCreated()) m_pglXPlaneBuffers->m_vboWingSurface[i].destroy();
            }
            m_pglXPlaneBuffers->m_vboWingOutline.clear();
            m_pglXPlaneBuffers->m_vboWingSurface.clear();
            m_pglXPlaneBuffers->m_vboWingOutline.resize(pPlaneXfl->nWings());
            m_pglXPlaneBuffers->m_vboWingSurface.resize(pPlaneXfl->nWings());

            m_pglXPlaneBuffers->resizeFuseGeometryBuffers(pPlaneXfl->nFuse());

            for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
            {
                Fuse const*pFuse = pPlaneXfl->fuseAt(ifuse);

                if(pFuse->isXflType())
                {
                    FuseXfl const *pTranslatedXflFuse = dynamic_cast<FuseXfl const*>(pFuse);
                    gl::makeTriangulation3Vtx(pTranslatedXflFuse->triangulation(), pPlaneXfl->fusePos(ifuse),
                                            m_pglXPlaneBuffers->m_vboFuseTriangulation[ifuse],
                                            pTranslatedXflFuse->isFlatFaceType());
//                    glMakeNodeNormals(pTranslatedXflFuse->triangulation().nodes(), Vector3d(), 0.05f, m_pglXPlaneBuffers->m_vboTessNormals);
                    int nPts = pFuse->isSplineType() ? 30 : 1;
                    gl::glMakeShellOutline(pFuse->shells(), pPlaneXfl->fusePos(ifuse), m_pglXPlaneBuffers->m_vboBodyOutline[ifuse], nPts);
                    if(pFuse->isSplineType())
                        gl::makeFuseXflFrames(pTranslatedXflFuse, pTranslatedXflFuse->position(), W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes(), m_pglXPlaneBuffers->m_vboFrames);
                    else if(pFuse->isSectionType())
                    {
                        FuseSections const *pFuseSecs = dynamic_cast<FuseSections const*>(pTranslatedXflFuse);
                        if(pFuseSecs)
                            gl::makeFuseXflSections(pFuseSecs, pFuseSecs->position(), W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes(), m_pglXPlaneBuffers->m_vboFrames);
                    }
                }
                else if(pFuse->isOccType())
                {
                    gl::makeTriangulation3Vtx(pFuse->triangulation(), pPlaneXfl->fusePos(ifuse),
                                            m_pglXPlaneBuffers->m_vboFuseTriangulation[ifuse], false);
                    gl::glMakeShellOutline(pFuse->shells(), pPlaneXfl->fusePos(ifuse), m_pglXPlaneBuffers->m_vboBodyOutline[ifuse]);
                }
                else if(pFuse->isStlType())
                {
                    gl::makeTriangulation3Vtx(pFuse->triangulation(), pPlaneXfl->fusePos(ifuse),
                                            m_pglXPlaneBuffers->m_vboFuseTriangulation[ifuse], true);
                    gl::makeTrianglesOutline(pFuse->triangles(), pPlaneXfl->fusePos(ifuse),  m_pglXPlaneBuffers->m_vboBodyOutline[ifuse]);
                }

                if(ifuse==0)
                {
                    Fuse *pTranslatedFuse = nullptr;
                    if(fabs(pPlaneXfl->fusePos(0).y)<0.001)
                    {
                        pTranslatedFuse = pPlaneXfl->fuseAt(ifuse)->clone();
                        pTranslatedFuse->translate(pPlaneXfl->fusePos(ifuse));
                    }

                    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                    {
                        WingXfl *pWing = pPlaneXfl->wing(iw);
                        pWing->makeTriangulation(pTranslatedFuse, W3dPrefs::s_iChordwiseRes);
                        gl::makeTriangles3Vtx(pWing->triangulation().triangles(), false, m_pglXPlaneBuffers->m_vboWingSurface[iw]);
                        gl::makeSegments(pWing->outline(), Vector3d(), m_pglXPlaneBuffers->m_vboWingOutline[iw]);
//                        m_pglWingBuffers->glMakeWingGeometry(iw, pWing, pTranslatedFuse);
//                        glMakeNodeNormals(pWing->triangulation().nodes(), Vector3d(), 0.05f, m_pglXPlaneBuffers->m_vboTessNormals);
                    }

                    delete pTranslatedFuse;
                }
            }

            // make the wings now if they weren't made with the fuse0
            if(!pPlaneXfl->hasFuse())
            {
                for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                {
                    WingXfl *pWing = pPlaneXfl->wing(iw);
                    pWing->makeTriangulation(nullptr, W3dPrefs::s_iChordwiseRes);
                    gl::makeTriangles3Vtx(pWing->triangulation().triangles(), false, m_pglXPlaneBuffers->m_vboWingSurface[iw]);
                    gl::makeSegments(pWing->outline(), Vector3d(), m_pglXPlaneBuffers->m_vboWingOutline[iw]);
//                    m_pglWingBuffers->glMakeWingGeometry(iw, pPlaneXfl->wingAt(iw), nullptr);
//                    m_pglWingBuffers->glMakeWingOutline(iw, pPlaneXfl->wingAt(iw), nullptr);
//                    glMakeNodeNormals(pWing->triangulation().nodes(), Vector3d(), 0.05f, m_vboTessNormals);
//                    glMakeTriangleNormals(pWing->triangulation().triangles(), 0.05f, m_vboTessNormals);

                }
            }

/*
            WingXfl const*main = pPlaneXfl->mainWing();

            int degree = 3;
            int nCtrlPoints = 11;
            int nOutPoints = 37;

                Surface const &surf = main->surfaceAt(0);
                BSpline3d b3dtopleft, b3dbotleft, b3dtopright, b3dbotright;
                TopoDS_Wire TopLeftWire, BotLeftWire, TopRightWire, BotRightWire;
                surf.makeSectionSplines(true,  true, degree, nCtrlPoints, nOutPoints, b3dtopleft);
                glMakeLineStrip(b3dtopleft.outputPts(), m_vboSplineTop);
                surf.makeSectionSplines(false, true, degree, nCtrlPoints, nOutPoints, b3dbotleft);
                glMakeLineStrip(b3dbotleft.outputPts(), m_vboSplineBot);

*/


        }
        else if(pPlane->isSTLType())
        {
            PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(s_pXPlane->m_pCurPlane);
            gl::makeTriangulation3Vtx(pPlaneSTL->triangulation(), Vector3d(),
                                                   m_pglXPlaneBuffers->m_vboPlaneStlTriangulation, true);
            gl::makeTrianglesOutline(pPlaneSTL->triangles(), Vector3d(), m_pglXPlaneBuffers->m_vboPlaneStlOutline);
        }

        s_bResetglGeom = false;
        QApplication::restoreOverrideCursor();
    }

    if ((s_bResetglMesh || s_bResetglGeom) && m_bResetglSegments)
    {
        gl::makeSegments(m_Segments, Vector3d(), m_vboSegments);
        m_bResetglSegments = false;
    }

    if(s_bResetglMesh)
    {
        if(pWPolar)
        {
            if(pWPolar->isQuadMethod() && pPlane->isXflType())
            {
                PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
                gl::makeQuadPanels(m_Panel4Visible, Vector3d(), m_pglXPlaneBuffers->m_vboMesh);
                gl::makeQuadEdges(m_Panel4Visible, Vector3d(), m_pglXPlaneBuffers->m_vboMeshEdges);
                gl::makePanelNormals(pPlaneXfl->quadMesh().panels(), float(m_RefLength)/50.0f, m_pglXPlaneBuffers->m_vboPanelNormals);

/*                if(pWPolar->isVLM1())
                {
                    m_Vortices.resize(m_Panel4Visible.size()*3);
                    for(int i4=0; i4<m_Panel4Visible.size(); i4++)
                    {
                        Vector3d const &VA = m_Panel4Visible.at(i4).m_VA;
                        Vector3d const &VB = m_Panel4Visible.at(i4).m_VB;
                        m_Vortices[3*i4].setNodes(VA, VB);
                        m_Vortices[3*i4+1].setNodes(VA, VA+Vector3d(5,0,0));
                        m_Vortices[3*i4+2].setNodes(VB, VB+Vector3d(5,0,0));
                    }
                    glMakeSegments(m_Vortices, Vector3d(), m_pglXPlaneBuffers->m_vboVortices);
                }
                else if(pWPolar->isVLM2())*/
                {
                    m_Vortices.resize(m_Panel4Visible.size());
                    for(int i4=0; i4<m_Vortices.size(); i4++)
                    {
                        m_Vortices[i4].setNodes(m_Panel4Visible.at(i4).m_VA, m_Panel4Visible.at(i4).m_VB);
                    }
                    for(uint i4=0; i4<m_Panel4Visible.size(); i4++)
                    {
                        Panel4 const &p4 = m_Panel4Visible.at(i4);
                        if(p4.isTrailing())
                        {
                            m_Vortices.append({p4.m_VA, p4.m_VA+Vector3d(1,0,0)});
                            m_Vortices.append({p4.m_VB, p4.m_VB+Vector3d(1,0,0)});
                        }
                    }
                    gl::makeSegments(m_Vortices, Vector3d(), m_pglXPlaneBuffers->m_vboVortices);
                }

            }
            else if(pWPolar->isTriangleMethod())
            {                
                gl::makeTriPanels(m_Panel3Visible, Vector3d(), m_pglXPlaneBuffers->m_vboMesh);
                gl::makeTriEdges(m_Panel3Visible, Vector3d(), m_pglXPlaneBuffers->m_vboMeshEdges);

                if(m_bPanelNormals)
                    gl::makePanelNormals(pPlane->triMesh().panels(), float(m_RefLength)/15.0f,
                                       m_pglXPlaneBuffers->m_vboPanelNormals);
                else if(m_bNodeNormals)
                {
                    // Node normals are not required for the analysis or the display, so need to make them on the fly
                    gl::makeNodeNormals(pPlane->triMesh().nodes(), Vector3d(), float(m_RefLength)/15.0f,
                                      m_pglXPlaneBuffers->m_vboPanelNormals);
                }

                std::vector<Panel3> p3High;
                for(int ip=0; ip<m_PanelHightlight.size(); ip++)
                {
                    int idx = m_PanelHightlight.at(ip);
                    if(idx>=0 && idx<pPlane->nPanel3())
                        p3High.push_back(pPlane->triMesh().panelAt(idx));
                }
                gl::makeTriEdges(p3High, Vector3d(), m_vboHighlightPanel3);
            }
        }
        else
        {
            if(m_pglXPlaneBuffers->m_vboPanelNormals.isCreated()) m_pglXPlaneBuffers->m_vboPanelNormals.destroy();
            if(m_pglXPlaneBuffers->m_vboMesh.isCreated())         m_pglXPlaneBuffers->m_vboMesh.destroy();
            if(m_pglXPlaneBuffers->m_vboMeshEdges.isCreated())    m_pglXPlaneBuffers->m_vboMeshEdges.destroy();
            if(m_vboHighlightPanel3.isCreated())                  m_vboHighlightPanel3.destroy();
        }
    }

    if(m_pPOpp3dControls->m_bWakePanels && (s_bResetglMesh || s_bResetglWake))
    {
        if(pWPolar /* && pPOpp */)
        {
            if(pWPolar->isQuadMethod() && pPlane->isXflType())
            {
                PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
                std::vector<Panel4> qVec = std::vector<Panel4>(pPlaneXfl->quadMesh().wakePanels().begin(), pPlaneXfl->quadMesh().wakePanels().end());
                gl::makeQuadPanels(qVec, Vector3d(), m_pglXPlaneBuffers->m_vboWakePanels);
                gl::makeQuadEdges( qVec, Vector3d(), m_pglXPlaneBuffers->m_vboWakeEdges);
            }
            else if(pWPolar->isTriangleMethod())
            {
                gl::makeTriPanels(pPlane->triMesh().wakePanels(), Vector3d(), m_pglXPlaneBuffers->m_vboWakePanels);
                gl::makeTriEdges( pPlane->triMesh().wakePanels(), Vector3d(), m_pglXPlaneBuffers->m_vboWakeEdges);
            }
        }
        s_bResetglWake = false;
    }

    glMakeOppBuffers();
    glMakeFlowBuffers();

    if(s_bResetglMesh && pWPolar && pWPolar->bHPlane())
    {
        Vector3d Normal(0.0,0.0,1.0);
        Node A( W3dPrefs::s_BoxX,  W3dPrefs::s_BoxY, -pWPolar->groundHeight(), Normal);
        Node B( W3dPrefs::s_BoxX, -W3dPrefs::s_BoxY, -pWPolar->groundHeight(), Normal);
        Node C(-W3dPrefs::s_BoxX, -W3dPrefs::s_BoxY, -pWPolar->groundHeight(), Normal);
        Node D(-W3dPrefs::s_BoxX,  W3dPrefs::s_BoxY, -pWPolar->groundHeight(), Normal);
        gl::makeQuad(A,B,C,D, m_vboHPlane);
    }

    s_bResetglMesh = false;
}


void gl3dXPlaneView::glMakeOppBuffers()
{
    Plane    const *pPlane  = s_pXPlane->m_pCurPlane;
    PlanePolar   const *pWPolar = s_pXPlane->m_pCurPlPolar;
    PlaneOpp       *pPOpp   = s_pXPlane->m_pCurPOpp; // not const to make node values on the fly

    if(!pPlane || !pWPolar) return;
    if(pWPolar && (pWPolar->planeName()!=pPlane->name()))     return; // failsafe

    if(m_pglXPlaneBuffers->m_LiveVortons.size() && s_bResetglVortons)
    {
        gl::makeVortons(m_pglXPlaneBuffers->m_LiveVortons, m_pglXPlaneBuffers->m_vboVortons);
        s_bResetglVortons = false;
    }

    if(!pPOpp) return;
    if(pPOpp->polarName()!=pWPolar->name() || pPOpp->planeName()!=pWPolar->planeName()) return;

    if(s_bResetglPanelCp || s_bResetglOpp || s_bResetglMesh)
    {
        double lmin=0, lmax=0;
        if(m_pPOpp3dControls->m_b3dCp)
        {
            if(pWPolar && pWPolar->isQuadMethod())
            {
                lmin =Opp3dScalesCtrls::CpMin();
                lmax =Opp3dScalesCtrls::CpMax();
                gl::makeQuadNodeClrMap(m_Panel4Visible, pPOpp->m_Cp, lmin, lmax, Opp3dScalesCtrls::isAutoCpScale(), m_pglXPlaneBuffers->m_vboCp);
                if(Opp3dScalesCtrls::isAutoCpScale()) m_pPOpp3dControls->m_pOpp3dScalesCtrls->updateCpRange(lmin, lmax);

            }
            else if(pWPolar && pWPolar->isTriangleMethod())
            {
                if(pPOpp->isTriUniformMethod())
                {
                    lmin =Opp3dScalesCtrls::CpMin();
                    lmax =Opp3dScalesCtrls::CpMax();
                    gl::makeTriUniColorMap(m_Panel3Visible, pPOpp->m_Cp, lmin, lmax, Opp3dScalesCtrls::isAutoCpScale(), m_pglXPlaneBuffers->m_vboCp);
                    if(Opp3dScalesCtrls::isAutoCpScale()) m_pPOpp3dControls->m_pOpp3dScalesCtrls->updateCpRange(lmin, lmax);
                }
                else if(pPOpp->isTriLinearMethod())
                {

                    TriMesh::makeNodeValues(pPlane->triMesh().nodes(), pPlane->triMesh().panels(),
                                            pPOpp->m_Cp, pPOpp->m_NodeValue, pPOpp->m_NodeValMin, pPOpp->m_NodeValMax, 1.0);

                    if(Opp3dScalesCtrls::isAutoCpScale())
                    {
                        lmin = pPOpp->m_NodeValMin;
                        lmax = pPOpp->m_NodeValMax;
                        m_pPOpp3dControls->m_pOpp3dScalesCtrls->updateCpRange(lmin, lmax);
                    }
                    else
                    {
                        lmin = Opp3dScalesCtrls::CpMin();
                        lmax = Opp3dScalesCtrls::CpMax();
                    }

                    gl::makeTriLinColorMap(m_Panel3Visible, pPlane->triMesh().nodes(), pPOpp->m_NodeValue,
                                         lmin, lmax, m_pglXPlaneBuffers->m_vboCp);

                }
            }
        }
        if(m_pPOpp3dControls->m_b3dCp)
        {
            m_ColourLegend.setTitle(tr("Cp"));
            m_ColourLegend.setRange(lmin, lmax);
            m_ColourLegend.makeLegend();
        }
        s_bResetglPanelCp = false;
    }

    if(s_bResetglPanelGamma || s_bResetglOpp || s_bResetglMesh)
    {
        double lmin(0), lmax(0);
        if(m_pPOpp3dControls->m_bGamma)
        {
            if(pWPolar && pWPolar->isQuadMethod())
            {
                lmin =Opp3dScalesCtrls::s_GammaMin;
                lmax =Opp3dScalesCtrls::s_GammaMax;
                gl::makeQuadNodeClrMap(m_Panel4Visible, pPOpp->m_gamma, lmin, lmax, Opp3dScalesCtrls::s_bAutoGammaScale,
                                     m_pglXPlaneBuffers->m_vboGamma);
                if(Opp3dScalesCtrls::s_bAutoGammaScale) m_pPOpp3dControls->m_pOpp3dScalesCtrls->updateGammaRange(lmin, lmax);
            }
            else if(pPOpp->isTriUniformMethod())
            {
                lmin =Opp3dScalesCtrls::s_GammaMin;
                lmax =Opp3dScalesCtrls::s_GammaMax;
                gl::makeTriUniColorMap(m_Panel3Visible, pPOpp->m_gamma, lmin, lmax, Opp3dScalesCtrls::s_bAutoGammaScale,
                                   m_pglXPlaneBuffers->m_vboGamma);
                if(Opp3dScalesCtrls::s_bAutoGammaScale) m_pPOpp3dControls->m_pOpp3dScalesCtrls->updateGammaRange(lmin, lmax);
            }
            else if(pPOpp->isTriLinearMethod())
            {

                TriMesh::makeNodeValues(pPlane->triMesh().nodes(), pPlane->triMesh().panels(),
                                        pPOpp->m_gamma, pPOpp->m_NodeValue,
                                        pPOpp->m_NodeValMin, pPOpp->m_NodeValMax, 1.0);

                if(Opp3dScalesCtrls::s_bAutoGammaScale)
                {
                    lmin = pPOpp->m_NodeValMin;
                    lmax = pPOpp->m_NodeValMax;
                    m_pPOpp3dControls->m_pOpp3dScalesCtrls->updateGammaRange(lmin, lmax);
                }
                else
                {
                    lmin = Opp3dScalesCtrls::gammaMin();
                    lmax = Opp3dScalesCtrls::gammaMax();
                }

                gl::makeTriLinColorMap(m_Panel3Visible,  pPlane->triMesh().nodes(),
                                     pPOpp->m_NodeValue, lmin, lmax, m_pglXPlaneBuffers->m_vboGamma);
            }
        }
        if(m_pPOpp3dControls->m_bGamma)
        {
            m_ColourLegend.setTitle(tr("Gamma x1000"));
            m_ColourLegend.setRange(lmin*1000.0,lmax*1000.0);
            m_ColourLegend.makeLegend();
        }
        s_bResetglPanelGamma = false;
    }

    if(s_bResetglPanelForce || s_bResetglOpp || s_bResetglMesh)
    {
        double lmin(0), lmax(0);
        if (m_pPOpp3dControls->m_bPanelForce && pWPolar && pPOpp)
        {
            double qDyn = 0.5*pWPolar->density()*pPOpp->QInf()*pPOpp->QInf();
            lmin = Opp3dScalesCtrls::pressureMin();
            lmax = Opp3dScalesCtrls::pressureMax();
            if(pPOpp->isQuadMethod())
            {
                gl::makePanelForces(m_Panel4Visible,
                                    pPOpp->m_Cp, float(qDyn), pWPolar->isVLM(),
                                    lmin, lmax, Opp3dScalesCtrls::s_bAutoPressureScale, Opp3dScalesCtrls::panelForceScale(),
                                    m_pglXPlaneBuffers->m_vboPanelForces);
            }
            else if (pPOpp->isTriUniformMethod())
            {
                gl::makePanelForces(m_Panel3Visible,
                                  pPOpp->m_Cp, float(qDyn),
                                  lmin, lmax, Opp3dScalesCtrls::isAutoPressureScale(), Opp3dScalesCtrls::panelForceScale(),
                                  m_pglXPlaneBuffers->m_vboPanelForces);
            }
            else if (pPOpp->isTriLinearMethod())
            {
                std::vector<Node> const &nodes = m_NodeVisible;
                TriMesh::makeNodeValues(pPlane->triMesh().nodes(), pPlane->triMesh().panels(),
                                        pPOpp->m_Cp, pPOpp->m_NodeValue,
                                        pPOpp->m_NodeValMin, pPOpp->m_NodeValMax, 1.0);
                gl::makeNodeForces(nodes,
                                   pPOpp->m_NodeValue, float(qDyn),
                                   lmin, lmax, Opp3dScalesCtrls::isAutoPressureScale(), Opp3dScalesCtrls::panelForceScale(),
                                   m_pglXPlaneBuffers->m_vboPanelForces);
            }
            if(Opp3dScalesCtrls::isAutoPressureScale()) m_pPOpp3dControls->m_pOpp3dScalesCtrls->updatePressureRange(lmin, lmax);
        }

        if(m_pPOpp3dControls->m_bPanelForce)
        {
            m_ColourLegend.setTitle(Units::pressureUnitQLabel());
            m_ColourLegend.setRange(lmin, lmax);
            m_ColourLegend.makeLegend();
        }
        s_bResetglPanelForce = false;
    }

    if(m_pPOpp3dControls->m_bMoments && (s_bResetglMoments || s_bResetglOpp))
    {
        glMakeMoments(pPlane->planformSpan(), pWPolar, pPOpp, float(m_RefLength));
        s_bResetglMoments = false;
    }

    if(pPlane->isXflType())
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        if(s_bResetglLift || s_bResetglOpp)
        {
            if(pWPolar->isLLTMethod())
            {
                glMakeLLTLiftStrip(  pPlaneXfl, pWPolar, pPOpp);
                glMakeLLTTransitions(pPlaneXfl, pWPolar, pPOpp, m_pglXPlaneBuffers->m_vboTrans);
            }
            else
            {
                glMakeLiftStrip(  pPlaneXfl, pWPolar, pPOpp);
                glMakeTransitions(pPlaneXfl, pWPolar, pPOpp, m_pglXPlaneBuffers->m_vboTrans);
            }
            s_bResetglLift = false;
        }

        if(s_bResetglDrag || s_bResetglOpp)
        {
            if(pWPolar->isLLTMethod())
            {
                glMakeLLTDragStrip(pPlaneXfl, pWPolar, pPOpp, m_pPOpp3dControls->m_bICd, m_pPOpp3dControls->m_bVCd);
            }
            else
            {
                glMakeDragStrip(pPlaneXfl, pWPolar, pPOpp, m_pPOpp3dControls->m_bICd, m_pPOpp3dControls->m_bVCd);
            }
            s_bResetglDrag = false;
        }

        if(m_pPOpp3dControls->m_bDownwash && (s_bResetglDownwash || s_bResetglOpp))
        {
            QVector<Vector3d> points, arrows;
            if(pWPolar->isLLTMethod())
            {
                makeLLTDownwash(pPlaneXfl, pWPolar, pPOpp, points, arrows);
            }
            else
            {
                makeDownwash(pPlaneXfl, pWPolar, pPOpp, points, arrows);
            }
            gl::makeArrows(points, arrows, s_VelocityScale, m_pglXPlaneBuffers->m_vboDownwash);
            s_bResetglDownwash = false;
        }
    }

    if((s_bResetglOpp || s_bResetglStream) && m_pPOpp3dControls->m_bStreamLines)
    {
        s_bResetglStream = false; //Prevent multiple recalculations if new repaint signal received before streamline build is done

        if(pPOpp->isQuadMethod())
        {
            glMakeStreamLines(m_Panel4Visible, pPOpp);
            s_bResetglStream = false;
        }
        else if(pPOpp->isTriangleMethod())
        {
            glMakeStreamLines(m_Panel3Visible, pPlane->triMesh().nodes(), pPOpp);
            s_bResetglStream = false;
        }
    }

    if(s_bResetglOpp || s_bResetglVortons)
    {
        gl::makeVortons(pPOpp->m_Vorton, m_pglXPlaneBuffers->m_vboVortons);
        s_bResetglVortons = false;
    }

    if(m_pCrossFlowCtrls && m_pCrossFlowCtrls->bGridVelocity())
    {
        if(s_bResetglOpp || s_bResetglGridVelocities)
        {
            gl::makeArrows(m_pCrossFlowCtrls->m_GridNodesVel, m_pCrossFlowCtrls->m_GridVectors, s_VelocityScale,
                           m_pglXPlaneBuffers->m_vboGridVelocities);
            s_bResetglGridVelocities = false;
        }
    }

    if(m_pCrossFlowCtrls && m_pCrossFlowCtrls->bVorticityMap())
    {
        if(s_bResetglOpp || s_bResetglVorticity)
        {
            if(pPOpp->m_Vorton.size())
            {
                double lmin = CrossFlowCtrls::omegaMin();
                double lmax = CrossFlowCtrls::omegaMax();
                bool bAuto = CrossFlowCtrls::s_bAutoOmegaScale;

                gl::makeQuadColorMap(m_pglXPlaneBuffers->m_vboContourClrs, m_pCrossFlowCtrls->s_nVorticitySamples, m_pCrossFlowCtrls->s_nVorticitySamples,
                                   m_pCrossFlowCtrls->m_GridNodesOmega, m_pCrossFlowCtrls->m_OmegaField,
                                   lmin, lmax, bAuto, xfl::isMultiThreaded());

                gl::makeQuadContoursOnGrid(m_pglXPlaneBuffers->m_vboContourLines, m_pCrossFlowCtrls->s_nVorticitySamples, m_pCrossFlowCtrls->s_nVorticitySamples,
                                         m_pCrossFlowCtrls->m_GridNodesOmega, m_pCrossFlowCtrls->m_OmegaField, xfl::isMultiThreaded());
                m_ColourLegend.setRange(lmin*1000.0, lmax*1000.0);
                m_ColourLegend.setTitle(tr("Omega x1000.0"));
                m_ColourLegend.makeLegend();
            }

            // make vorton line
            gl::makeLines(pPOpp->vortonLines(), m_pglXPlaneBuffers->m_vboVortonLines);
            s_bResetglVorticity = false;
        }
    }

    s_bResetglOpp = false;
}


void gl3dXPlaneView::resetWakeControls()
{
    if(m_pCrossFlowCtrls)
    {
        m_pCrossFlowCtrls->makeXPlaneVelocityVectors();
        m_pCrossFlowCtrls->makeOmegaMap();
    }
    update();
}


void gl3dXPlaneView::onUpdate3dScales()
{
    s_bResetglPanelCp        = true;
    s_bResetglPanelGamma     = true;
    s_bResetglPanelCp        = true;
    s_bResetglPanelForce     = true;
    s_bResetglLift           = true;
    s_bResetglMoments        = true;
    s_bResetglDrag           = true;
    s_bResetglDownwash       = true;
    s_bResetglGridVelocities = true;

    m_ColourLegend.makeLegend();

    update();
}


void gl3dXPlaneView::onUpdate3dStreamlines()
{
    s_bResetglStream = true;

    update();
}


void gl3dXPlaneView::makeLLTDownwash(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp,
                                        QVector<Vector3d> &points,QVector<Vector3d> &arrows) const
{
    if(!pPlane || !pPlane->mainWing() || !pWPolar || !pPOpp) return;

    Vector3d N;

    float sina = -float(sin(pPOpp->alpha()*PI/180.0));
    float cosa =  float(cos(pPOpp->alpha()*PI/180.0));

    WingXfl const*pWing = pPlane->mainWing();
    WingOpp const &wOpp = pPOpp->WOpp(0);
    SpanDistribs const &sd = wOpp.spanResults();

    points.resize(sd.nStations());
    arrows.resize(sd.nStations());

    for (int i=0; i<wOpp.m_NStation; i++)
    {
        pWing->surfacePoint(1, sd.m_StripPos.at(i), xfl::MIDSURFACE, points[i], N);

        points[i].rotateY(pPOpp->alpha());

        arrows[i].x = sd.m_Vd.at(i).x * sina;
        arrows[i].y = sd.m_Vd.at(i).y;
        arrows[i].z = sd.m_Vd.at(i).z * cosa;
    }
}


void gl3dXPlaneView::makeDownwash(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp,
                                  QVector<Vector3d> &points, QVector<Vector3d> &arrows) const
{
    if(!pPlane || !pWPolar || !pPOpp)
    {
        return;
    }

    std::vector<Panel3> const &panel3 = pPlane->triPanels();
    std::vector<Panel4> const &panel4 = pPlane->quadMesh().panels();

    int npoints = 0;
    for(int iWing=0; iWing<pPlane->nWings(); iWing++)
    {
        WingXfl const *pWing = pPlane->wingAt(iWing);
        npoints += pWing->nStations();
    }

    points.resize(npoints);
    arrows.resize(npoints);

    Vector3d C;

    float sina = -float(sin(pPOpp->alpha()*PI/180.0));
    float cosa =  float(cos(pPOpp->alpha()*PI/180.0));

    int m = 0;
    Panel const *pPanel = nullptr;

    for(int iWing=0; iWing<pPlane->nWings(); iWing++)
    {
        WingXfl const *pWing = pPlane->wingAt(iWing);
        WingOpp const *pWOpp = &pPOpp->WOpp(iWing);
        SpanDistribs const &sd = pWOpp->spanResults();


        int npanels = 0;
        int i0 = -1;
        if(pPOpp->isQuadMethod())
        {
            npanels = pWing->nPanel4();
            i0      = pWing->firstPanel4Index();
        }
        else if(pPOpp->isTriangleMethod())
        {
            npanels = pWing->nPanel3();
            i0      = pWing->firstPanel3Index();
        }


        int i = 0;
        for (int ip=0; ip<npanels; ip++)
        {
            int idx = i0+ip;
            if(pPOpp->isQuadMethod())     pPanel = &panel4.at(idx);
            if(pPOpp->isTriangleMethod()) pPanel = &panel3.at(idx);
            if(pPanel && pPanel->isTrailing() && (pPanel->isBotPanel()||pPanel->isMidPanel()))
            {
                points[m].set(pPanel->midTrailingPoint());

                arrows[m].x = sd.m_Vd.at(i).x*sina;
                arrows[m].y = sd.m_Vd.at(i).y;
                arrows[m].z = sd.m_Vd.at(i).z*cosa;

                i++;
                m++;
            }
        }
    }
}


void gl3dXPlaneView::glMakeLLTTransitions(PlaneXfl const *pPlane, PlanePolar const *pWPolar, PlaneOpp const *pPOpp, QOpenGLBuffer &vbo) const
{
    if(!pPlane || !pPlane->mainWing() || !pWPolar || !pPOpp) return;

    WingXfl const*pWing = pPlane->mainWing();
    WingOpp const &wOpp = pPOpp->WOpp(0);
    SpanDistribs const &sd = wOpp.spanResults();

    Vector3d N;

    int buffersize =(wOpp.m_NStation-1)*6*2;
    QVector<float> pTransVertexArray(buffersize);
    int iv=0;
    Vector3d top, bot, lasttop, lastbot;
    int m=0;
    for(int j=0; j<wOpp.m_NStation; j++)
    {
        pWing->surfacePoint(sd.m_XTrTop.at(j), sd.m_StripPos.at(j), xfl::TOPSURFACE, top, N);
        pWing->surfacePoint(sd.m_XTrBot.at(j), sd.m_StripPos.at(j), xfl::BOTSURFACE, bot, N);
        if(m>0)
        {
            pTransVertexArray[iv++] = lasttop.xf();
            pTransVertexArray[iv++] = lasttop.yf();
            pTransVertexArray[iv++] = lasttop.zf();
            pTransVertexArray[iv++] = top.xf();
            pTransVertexArray[iv++] = top.yf();
            pTransVertexArray[iv++] = top.zf();

            pTransVertexArray[iv++] = lastbot.xf();
            pTransVertexArray[iv++] = lastbot.yf();
            pTransVertexArray[iv++] = lastbot.zf();
            pTransVertexArray[iv++] = bot.xf();
            pTransVertexArray[iv++] = bot.yf();
            pTransVertexArray[iv++] = bot.zf();
        }
        lasttop = top;
        lastbot = bot;
        m++;
    }

    Q_ASSERT(iv==buffersize);
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(pTransVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dXPlaneView::glMakeTransitions(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp, QOpenGLBuffer &vbo) const
{
    if(!pPlane || !pWPolar || !pPOpp) return;

    int buffersize = 0;
    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl const*pWing = pPlane->wingAt(iw);
        buffersize += (pWing->nStations()-1)*6; // a 2 point segment is 6 floats
    }
    buffersize *=2; //top and bot
    QVector<float> pTransVertexArray(buffersize);

    Vector3d N;
    Vector3d top, bot, lasttop, lastbot;
    int iv=0;
    double yrel=0;
    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl const*pWing = pPlane->wingAt(iw);
        int m = 0;

        if(!pWing) continue; // unnecessary precaution
        if(iw>=pPOpp->nWOpps()) return;
        WingOpp const *pWOpp = &pPOpp->WOpp(iw);
        SpanDistribs const &SD = pWOpp->spanResults();

        for(int j=0; j<pWing->nSurfaces(); j++)
        {
            for(int k=0; k<pWing->surfaceAt(j).NYPanels(); k++)
            {
                yrel = pWing->ySectionRel(SD.m_StripPos.at(m));
                pWing->surfaceAt(j).getSurfacePoint(SD.m_XTrTop.at(m), SD.m_XTrTop.at(m),yrel, xfl::TOPSURFACE, top, N);
                top.rotateY(pPOpp->alpha());

                yrel = pWing->ySectionRel(SD.m_StripPos.at(m));
                pWing->surfaceAt(j).getSurfacePoint(SD.m_XTrBot.at(m), SD.m_XTrBot.at(m), yrel, xfl::BOTSURFACE, bot, N);
                bot.rotateY(pPOpp->alpha());

                if(m>0)
                {
                    pTransVertexArray[iv++] = lasttop.xf();
                    pTransVertexArray[iv++] = lasttop.yf();
                    pTransVertexArray[iv++] = lasttop.zf();
                    pTransVertexArray[iv++] = top.xf();
                    pTransVertexArray[iv++] = top.yf();
                    pTransVertexArray[iv++] = top.zf();

                    pTransVertexArray[iv++] = lastbot.xf();
                    pTransVertexArray[iv++] = lastbot.yf();
                    pTransVertexArray[iv++] = lastbot.zf();
                    pTransVertexArray[iv++] = bot.xf();
                    pTransVertexArray[iv++] = bot.yf();
                    pTransVertexArray[iv++] = bot.zf();
                }
                lasttop = top;
                lastbot = bot;
                m++;
            }
        }
    }
    Q_ASSERT(iv==buffersize);
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(pTransVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dXPlaneView::glMakeDragStrip(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp, bool bICd, bool bVCd)
{
    if(!pPlane || !pWPolar || !pPOpp)
    {
        m_pglXPlaneBuffers->m_vboInducedDrag.destroy();
        m_pglXPlaneBuffers->m_vboViscousDrag.destroy();
        return;
    }

    std::vector<Panel3> const &panel3 = pPlane->triPanels();
    std::vector<Panel4> const &panel4 = pPlane->quadMesh().panels();

    Vector3d C;

    //DRAGLINE
    double q0 = 0.5 * pWPolar->density() * pWPolar->referenceArea() * pPOpp->QInf() * pPOpp->QInf();

    float factor = 0.001f;
    float amp1=0, amp2=0;

    // count the stations
    int ICDbuffersize = 0;
    int VCDbuffersize = 0;
    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl const*pWing = pPlane->wingAt(iw);
        int nstations = pWing->nStations();
        if(bICd) ICDbuffersize += nstations*6 + (nstations-1)*6;
        if(bVCd) VCDbuffersize += nstations*6 + (nstations-1)*6;
    }
    QVector<float> pICdVertexArray(ICDbuffersize);
    QVector<float> pVCdVertexArray(VCDbuffersize);

    Panel const *pPanel = nullptr;
    Vector3d cdi, cdv, lastcdi, lastcdv;
    int ii=0, iv=0;
    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl const*pWing = pPlane->wingAt(iw);
        WingOpp const *pWOpp = &pPOpp->WOpp(iw);
        SpanDistribs const &SD = pWOpp->spanResults();

        int m=0; // the station index

        int npanels = 0;
        int i0 = -1;
        if(pPOpp->isQuadMethod())
        {
            npanels = pWing->nPanel4();
            i0      = pWing->firstPanel4Index();
        }
        else if(pPOpp->isTriangleMethod())
        {
            npanels = pWing->nPanel3();
            i0      = pWing->firstPanel3Index();
        }

        for (int ip=0; ip<npanels; ip++)
        {
            int idx = i0+ip;
            if(pPOpp->isQuadMethod())     pPanel = &panel4.at(idx);
            if(pPOpp->isTriangleMethod()) pPanel = &panel3.at(idx);

            if(pPanel && pPanel->isTrailing() && (pPanel->isBotPanel()||pPanel->isMidPanel()))
            {
                C.set(pPanel->midTrailingPoint());
                amp1 = float(q0*SD.m_ICd.at(m)*SD.m_Chord.at(m)/pWing->MAC()) * Opp3dScalesCtrls::dragScale() * factor;
                amp2 = float(q0*SD.m_PCd.at(m)*SD.m_Chord.at(m)/pWing->MAC()) * Opp3dScalesCtrls::dragScale() * factor;

                cdi.x = C.x + amp1;
                cdi.y = C.y;
                cdi.z = C.z;
                if(bICd)
                {
                    pICdVertexArray[ii++] = C.xf();
                    pICdVertexArray[ii++] = C.yf();
                    pICdVertexArray[ii++] = C.zf();
                    pICdVertexArray[ii++] = cdi.xf();
                    pICdVertexArray[ii++] = cdi.yf();
                    pICdVertexArray[ii++] = cdi.zf();
                }
                if(bVCd)
                {
                    if(!bICd)
                    {
                        cdv.x = C.x + amp2;
                        cdv.y = C.y;
                        cdv.z = C.z;
                        pVCdVertexArray[iv++] = C.xf();
                        pVCdVertexArray[iv++] = C.yf();
                        pVCdVertexArray[iv++] = C.zf();
                        pVCdVertexArray[iv++] = cdv.xf();
                        pVCdVertexArray[iv++] = cdv.yf();
                        pVCdVertexArray[iv++] = cdv.zf();
                    }
                    else
                    {
                        cdv.x = C.x + amp1+amp2;
                        cdv.y = C.y;
                        cdv.z = C.z;
                        pVCdVertexArray[iv++] = cdi.xf();
                        pVCdVertexArray[iv++] = cdi.yf();
                        pVCdVertexArray[iv++] = cdi.zf();
                        pVCdVertexArray[iv++] = cdv.xf();
                        pVCdVertexArray[iv++] = cdv.yf();
                        pVCdVertexArray[iv++] = cdv.zf();
                    }
                }
                if(m>0)
                {
                    // add a transverse line joining the two stations
                    if(bICd)
                    {
                        pICdVertexArray[ii++] = lastcdi.xf();
                        pICdVertexArray[ii++] = lastcdi.yf();
                        pICdVertexArray[ii++] = lastcdi.zf();
                        pICdVertexArray[ii++] = cdi.xf();
                        pICdVertexArray[ii++] = cdi.yf();
                        pICdVertexArray[ii++] = cdi.zf();
                    }

                    if(bVCd)
                    {
                        pVCdVertexArray[iv++] = lastcdv.xf();
                        pVCdVertexArray[iv++] = lastcdv.yf();
                        pVCdVertexArray[iv++] = lastcdv.zf();
                        pVCdVertexArray[iv++] = cdv.xf();
                        pVCdVertexArray[iv++] = cdv.yf();
                        pVCdVertexArray[iv++] = cdv.zf();
                    }
                }
                lastcdv = cdv;
                lastcdi = cdi;
                m++;
            }
        }
    }

    Q_ASSERT(ii==ICDbuffersize);
    Q_ASSERT(iv==VCDbuffersize);

    m_pglXPlaneBuffers->m_vboInducedDrag.destroy();
    m_pglXPlaneBuffers->m_vboInducedDrag.create();
    m_pglXPlaneBuffers->m_vboInducedDrag.bind();
    m_pglXPlaneBuffers->m_vboInducedDrag.allocate(pICdVertexArray.data(), ICDbuffersize * int(sizeof(GLfloat)));
    m_pglXPlaneBuffers->m_vboInducedDrag.release();

    m_pglXPlaneBuffers->m_vboViscousDrag.destroy();
    m_pglXPlaneBuffers->m_vboViscousDrag.create();
    m_pglXPlaneBuffers->m_vboViscousDrag.bind();
    m_pglXPlaneBuffers->m_vboViscousDrag.allocate(pVCdVertexArray.data(), VCDbuffersize * int(sizeof(GLfloat)));
    m_pglXPlaneBuffers->m_vboViscousDrag.release();
}


void gl3dXPlaneView::glMakeLLTDragStrip(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp, bool bICd, bool bVCd)
{
    if(!pPlane || !pPlane->mainWing() || !pWPolar || !pPOpp) return;

    Vector3d PtTE, N;

    float factor = 0.001f;
    float amp1=0, amp2=0;

    WingXfl const*pWing = pPlane->mainWing();

    WingOpp const &wOpp = pPOpp->WOpp(0);
    SpanDistribs const &sd = wOpp.m_SpanDistrib;
    float cosa =  float(cos(pPOpp->alpha() * PI/180.0));
    float sina = -float(sin(pPOpp->alpha() * PI/180.0));
    float cosb =  float(cos(pPOpp->beta()*PI/180.0));
    float sinb =  float(sin(pPOpp->beta()*PI/180.0));

    cosa = cosb = 1.0;
    sina = sinb = 0.0;

    int bufferSize = sd.nStations()*6 + (sd.nStations()-1)*6;
    QVector<float> pICdVertexArray(bufferSize);
    QVector<float> pVCdVertexArray(bufferSize);

    //DRAGLINE
    double q0 = 0.5 * pWPolar->density() * pWPolar->referenceArea() * pPOpp->QInf() * pPOpp->QInf();

    int ii=0, iv=0;

    Vector3d cdi, cdv, lastcdi, lastcdv;

    //Panel type drag
    int m = 0;
    ii=0;
    iv=0;
    for (int j=0; j<wOpp.m_NStation; j++)
    {
        pWing->surfacePoint(1.0, sd.m_StripPos.at(j), xfl::MIDSURFACE, PtTE, N);

        PtTE.rotateY(pPOpp->alpha());

        amp1 = float(q0*sd.m_ICd.at(m)*wOpp.spanResults().m_Chord.at(m)/pWing->MAC()) * Opp3dScalesCtrls::dragScale() * factor;
        amp2 = float(q0*sd.m_PCd.at(m)*wOpp.spanResults().m_Chord.at(m)/pWing->MAC()) * Opp3dScalesCtrls::dragScale() * factor;
        if(bICd)
        {
            cdi.x = PtTE.x + amp1*cosa * cosb;
            cdi.y = PtTE.y + amp1*cosa * sinb;
            cdi.z = PtTE.z - amp1*sina;
            pICdVertexArray[ii++] = PtTE.xf();
            pICdVertexArray[ii++] = PtTE.yf();
            pICdVertexArray[ii++] = PtTE.zf();
            pICdVertexArray[ii++] = cdi.xf();
            pICdVertexArray[ii++] = cdi.yf();
            pICdVertexArray[ii++] = cdi.zf();
        }
        if(bVCd)
        {
            if(!bICd)
            {
                cdv.x = PtTE.x + amp2*cosa * cosb;
                cdv.y = PtTE.y + amp2*cosa * sinb;
                cdv.z = PtTE.z - amp2*sina;
                pVCdVertexArray[iv++] = PtTE.xf();
                pVCdVertexArray[iv++] = PtTE.yf();
                pVCdVertexArray[iv++] = PtTE.zf();
                pVCdVertexArray[iv++] = cdv.xf();
                pVCdVertexArray[iv++] = cdv.yf();
                pVCdVertexArray[iv++] = cdv.zf();
            }
            else
            {
                cdv.x = PtTE.x + (amp1+amp2)*cosa * cosb;
                cdv.y = PtTE.y + (amp1+amp2)*cosa * sinb;
                cdv.z = PtTE.z - (amp1+amp2)*sina;
                pVCdVertexArray[iv++] = cdi.xf();
                pVCdVertexArray[iv++] = cdi.yf();
                pVCdVertexArray[iv++] = cdi.zf();
                pVCdVertexArray[iv++] = cdv.xf();
                pVCdVertexArray[iv++] = cdv.yf();
                pVCdVertexArray[iv++] = cdv.zf();
            }
        }

        if(m>0)
        {
            // add a transverse line joining the two stations
            pICdVertexArray[ii++] = lastcdi.xf();
            pICdVertexArray[ii++] = lastcdi.yf();
            pICdVertexArray[ii++] = lastcdi.zf();
            pICdVertexArray[ii++] = cdi.xf();
            pICdVertexArray[ii++] = cdi.yf();
            pICdVertexArray[ii++] = cdi.zf();

            pVCdVertexArray[iv++] = lastcdv.xf();
            pVCdVertexArray[iv++] = lastcdv.yf();
            pVCdVertexArray[iv++] = lastcdv.zf();
            pVCdVertexArray[iv++] = cdv.xf();
            pVCdVertexArray[iv++] = cdv.yf();
            pVCdVertexArray[iv++] = cdv.zf();
        }
        lastcdv = cdv;
        lastcdi = cdi;
        m++;
    }

    if(bICd) Q_ASSERT(ii==bufferSize);
    if(bVCd) Q_ASSERT(iv==bufferSize);


    m_pglXPlaneBuffers->m_vboInducedDrag.destroy();
    m_pglXPlaneBuffers->m_vboInducedDrag.create();
    m_pglXPlaneBuffers->m_vboInducedDrag.bind();
    m_pglXPlaneBuffers->m_vboInducedDrag.allocate(pICdVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_pglXPlaneBuffers->m_vboInducedDrag.release();

    m_pglXPlaneBuffers->m_vboViscousDrag.destroy();
    m_pglXPlaneBuffers->m_vboViscousDrag.create();
    m_pglXPlaneBuffers->m_vboViscousDrag.bind();
    m_pglXPlaneBuffers->m_vboViscousDrag.allocate(pVCdVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_pglXPlaneBuffers->m_vboViscousDrag.release();
}


void gl3dXPlaneView::glMakeMoments(double planformspan, PlanePolar const *pWPolar, PlaneOpp const *pPOpp, float reflength)
{
    //    The most common aeronautical convention defines
    //    - the roll as acting about the longitudinal axis, positive with the starboard wing down.
    //    - The yaw is about the vertical body axis, positive with the nose to starboard.
    //    - Pitch is about an axis perpendicular to the longitudinal plane of symmetry, positive nose up.
    //    -- Wikipedia flight dynamics --
    if(!pWPolar || !pPOpp)
    {
        m_pglXPlaneBuffers->m_vboMoments.destroy();
        return;
    }

    float angle=0.0;//radian
    float endx, endy, endz, dx, dy, dz,xae, yae, zae;
    float factor = 1.0;
    float radius= float(planformspan)/4.0f;
    float frac=0;

    float ampL = float(0.5*pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceChordLength()
                       *pPOpp->QInf()*pPOpp->QInf() * pPOpp->aeroForces().Cli() * double(Opp3dScalesCtrls::momentScale()))*factor;
    float ampM = float(0.5*pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceSpanLength()
                       *pPOpp->QInf()*pPOpp->QInf() * pPOpp->aeroForces().Cm() * double(Opp3dScalesCtrls::momentScale()))*factor;
    float ampN = float(0.5*pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceSpanLength()
                       *pPOpp->QInf()*pPOpp->QInf()*(pPOpp->aeroForces().Cn()) * double(Opp3dScalesCtrls::momentScale()))*factor;

    float tiplength = 0.015f*reflength;
    int nMomentPoints = 1000;

    int nLines = (nMomentPoints+2)*2*3;// x2 end points x3 coords
    QVector<float> momentVertexArray(nLines*3); //  x3 moments
    int iv = 0;
    //ROLLING MOMENT
    float sign=1.0;
    if (ampL>0.0f) sign = -1.0; else sign = 1.0;
    for (int i=0; i<nMomentPoints; i++)
    {
        frac = sign * float(i)/float(nMomentPoints-1) * factor * ampL;
        angle = PIf/180.0f * frac;
        momentVertexArray[iv++] = radius*cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = radius*sin(angle);
        frac = sign * float(i+1)/float(nMomentPoints-1) * factor * ampL;
        angle = PIf/180.0f * frac;
        momentVertexArray[iv++] = radius*cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = radius*sin(angle);
    }

    endy = radius*cos(angle);
    endz = radius*sin(angle);
    dy = tiplength;
    dz = tiplength*sign;

    yae = (radius-dy)*cos(angle) +dz *sin(angle);
    zae = (radius-dy)*sin(angle) -dz *cos(angle);
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = endy;
    momentVertexArray[iv++] = endz;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = yae;
    momentVertexArray[iv++] = zae;

    yae = (radius+dy)*cos(angle) +dz *sin(angle);
    zae = (radius+dy)*sin(angle) -dz *cos(angle);
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = endy;
    momentVertexArray[iv++] = endz;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = yae;
    momentVertexArray[iv++] = zae;

    //PITCHING MOMENT
    if (ampM>0.0f) sign = -1.0f; else sign = 1.0f;
    for (int i=0; i<nMomentPoints; i++)
    {
        frac = sign * float(i)/float(nMomentPoints-1) * factor * ampM;
        angle = PIf/180.0f * frac;
        momentVertexArray[iv++] = radius*cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = radius*sin(angle);
        frac = sign * float(i+1)/float(nMomentPoints-1) * factor * ampM;
        angle = PIf/180.0f * frac;
        momentVertexArray[iv++] = radius*cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = radius*sin(angle);
    }

    endx = radius*cos(angle);
    endz = radius*sin(angle);
    dx = tiplength;
    dz = tiplength*sign;

    xae = (radius-dx)*cos(angle) +dz *sin(angle);
    zae = (radius-dx)*sin(angle) -dz *cos(angle);
    momentVertexArray[iv++] = endx;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = endz;
    momentVertexArray[iv++] = xae;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = zae;

    xae = (radius+dx)*cos(angle) +dz *sin(angle);
    zae = (radius+dx)*sin(angle) -dz *cos(angle);
    momentVertexArray[iv++] = endx;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = endz;
    momentVertexArray[iv++] = xae;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = zae;

    //YAWING MOMENT

    if (ampN>0.0f) sign = -1.0f; else sign = 1.0f;
    angle = 0.0;
    for (int i=0; i<nMomentPoints; i++)
    {
        frac = sign * float(i)/float(nMomentPoints-1) * factor * ampN;
        angle = PIf/180.0f * frac;
        momentVertexArray[iv++] = radius*cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = radius*sin(angle);
        frac = sign * float(i+1)/float(nMomentPoints-1) * factor * ampN;
        angle = PIf/180.0f* frac;
        momentVertexArray[iv++] = radius*cos(angle);
        momentVertexArray[iv++] = 0.0;
        momentVertexArray[iv++] = radius*sin(angle);

    }

    endx = -radius*cos(angle);
    endy = -radius*sin(angle);
    dx =   tiplength;
    dy =  -tiplength*sign;

    xae = (-radius+dx)*cos(angle) +dy *sin(angle);
    yae = (-radius+dx)*sin(angle) -dy *cos(angle);
    momentVertexArray[iv++] = endx;
    momentVertexArray[iv++] = endy;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = xae;
    momentVertexArray[iv++] = yae;
    momentVertexArray[iv++] = 0.0;

    xae = (-radius-dx)*cos(angle) +dy *sin(angle);
    yae = (-radius-dx)*sin(angle) -dy *cos(angle);
    momentVertexArray[iv++] = endx;
    momentVertexArray[iv++] = endy;
    momentVertexArray[iv++] = 0.0;
    momentVertexArray[iv++] = xae;
    momentVertexArray[iv++] = yae;
    momentVertexArray[iv++] = 0.0;


    Q_ASSERT(iv==momentVertexArray.size());
    m_pglXPlaneBuffers->m_vboMoments.destroy();
    m_pglXPlaneBuffers->m_vboMoments.create();
    m_pglXPlaneBuffers->m_vboMoments.bind();
    m_pglXPlaneBuffers->m_vboMoments.allocate(momentVertexArray.data(), momentVertexArray.size()*int(sizeof(float)));
    m_pglXPlaneBuffers->m_vboMoments.release();
}


void gl3dXPlaneView::glMakeLiftStrip(PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp)
{
    if(!pPlane || !pWPolar || !pPOpp)
    {
        m_pglXPlaneBuffers->m_vboStripLift.destroy();
        return;
    }

    Vector3d C, F;

    float amp=0.0;
    float factor = 0.000025f;

    // count the stations
    int buffersize = 0;
    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl const*pWing = pPlane->wingAt(iw);
        int nstations = pWing->nStations();
        buffersize += nstations*6 + (nstations-1)*6;
    }
    QVector<float> pLiftVertexArray(buffersize);

    Vector3d ptl, lastptl;
    int iv=0;
    for(int iw=0; iw<pPlane->nWings(); iw++)
    {
        WingXfl const*pWing = pPlane->wingAt(iw);
        WingOpp const *pWOpp = &pPOpp->WOpp(iw);
        SpanDistribs const &SD = pWOpp->spanResults();
        int m=0; // the station index

        //lift lines
        for (int j=0; j<pWing->nSurfaces(); j++)
        {
            Surface const &surf = pWing->surfaceAt(j);
            for (int k=0; k< surf.NYPanels(); k++)
            {
                surf.getLeadingPt(k, C);
                amp = float(SD.m_Chord.at(m) / SD.m_StripArea.at(m) / pWing->MAC()) * Opp3dScalesCtrls::liftScale() * factor;
                C.x += SD.m_XCPSpanRel.at(m) * surf.chord(k);
                C.rotateY(pPOpp->alpha());
                F = SD.m_F.at(m);

                F.rotateY(pPOpp->alpha());

                ptl = C + F*amp;

                pLiftVertexArray[iv++] = C.xf();
                pLiftVertexArray[iv++] = C.yf();
                pLiftVertexArray[iv++] = C.zf();

                pLiftVertexArray[iv++] = ptl.xf();
                pLiftVertexArray[iv++] = ptl.yf();
                pLiftVertexArray[iv++] = ptl.zf();

                if(m>0)
                {
                    pLiftVertexArray[iv++] = lastptl.xf();
                    pLiftVertexArray[iv++] = lastptl.yf();
                    pLiftVertexArray[iv++] = lastptl.zf();
                    pLiftVertexArray[iv++] = ptl.xf();
                    pLiftVertexArray[iv++] = ptl.yf();
                    pLiftVertexArray[iv++] = ptl.zf();
                }
                lastptl = ptl;
                m++;
            }
        }
    }

    Q_ASSERT(iv==buffersize);

    m_pglXPlaneBuffers->m_vboStripLift.destroy();
    m_pglXPlaneBuffers->m_vboStripLift.create();
    m_pglXPlaneBuffers->m_vboStripLift.bind();
    m_pglXPlaneBuffers->m_vboStripLift.allocate(pLiftVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_pglXPlaneBuffers->m_vboStripLift.release();
}


void gl3dXPlaneView::glMakeLLTLiftStrip(PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp)
{
    if(!pPlane || !pWPolar || !pPOpp || !pPOpp->isLLTMethod())
    {
        m_pglXPlaneBuffers->m_vboStripLift.destroy();
        return;
    }

    int iw=0;
    WingXfl const *pWing = pPlane->mainWing();
    if(iw>=pPOpp->nWOpps()) return;
    WingOpp const *pWOpp = &pPOpp->WOpp(iw);
    if(!pWOpp) return;

    SpanDistribs const &sd = pWOpp->spanResults();

    Vector3d C;

    float amp=0.0;
    float factor = 0.000025f;

    //LIFTLINE
    //dynamic pressure x area
    int buffersize = sd.nStations()*6 + (sd.nStations()-1)*6;
    QVector<float> pLiftVertexArray(buffersize);

    int iv=0, m=0;
    Vector3d ptl, lastptl;
    //lift lines
    for (int j=0; j<sd.nStations(); j++)
    {
        C = sd.m_PtC4.at(m);
        amp = float(sd.m_Chord.at(j) / sd.m_StripArea.at(m) / pWing->MAC()) * Opp3dScalesCtrls::liftScale() * factor;
        C.x += sd.m_XCPSpanRel.at(m) * sd.m_Chord.at(j);

        C.rotateY(pPOpp->alpha());

        ptl = C + sd.m_F.at(m)*amp;

        pLiftVertexArray[iv++] = C.xf();
        pLiftVertexArray[iv++] = C.yf();
        pLiftVertexArray[iv++] = C.zf();

        pLiftVertexArray[iv++] = ptl.xf();
        pLiftVertexArray[iv++] = ptl.yf();
        pLiftVertexArray[iv++] = ptl.zf();

        if(m>0)
        {
            pLiftVertexArray[iv++] = lastptl.xf();
            pLiftVertexArray[iv++] = lastptl.yf();
            pLiftVertexArray[iv++] = lastptl.zf();
            pLiftVertexArray[iv++] = ptl.xf();
            pLiftVertexArray[iv++] = ptl.yf();
            pLiftVertexArray[iv++] = ptl.zf();
        }
        lastptl = ptl;
        m++;
    }

    Q_ASSERT(iv==buffersize);

    m_pglXPlaneBuffers->m_vboStripLift.destroy();
    m_pglXPlaneBuffers->m_vboStripLift.create();
    m_pglXPlaneBuffers->m_vboStripLift.bind();
    m_pglXPlaneBuffers->m_vboStripLift.allocate(pLiftVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_pglXPlaneBuffers->m_vboStripLift.release();
}


void gl3dXPlaneView::paintOverlay()
{
    gl3dView::paintOverlay();
//    qDebug()<<"device pixel ratio"<<devicePixelRatio()<<qApp->devicePixelRatio();
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);

    glDisable(GL_CULL_FACE);

    if(m_ColourLegend.isVisible())
    {
        int wc = m_ColourLegend.m_pix.width();
//        QPoint pos3(width()*qApp->devicePixelRatio()-5-wc, 50);
        QPoint pos4(width()*devicePixelRatio()-5-wc, 50);
        painter.drawPixmap(pos4, m_ColourLegend.m_pix);
    }
}


void gl3dXPlaneView::glMakeFlowBuffers()
{
    Plane    const *pPlane  = s_pXPlane->m_pCurPlane;
    PlanePolar   const *pWPolar = s_pXPlane->m_pCurPlPolar;
    PlaneOpp const *pPOpp   = s_pXPlane->m_pCurPOpp;

    if(!m_pPOpp3dControls->isFlowActive() || !pPlane || !pWPolar ||!pPOpp || !pPOpp->isTriUniformMethod())
    {
        if(m_ssboPanels.isCreated())  m_ssboPanels.destroy();
        if(m_ssboVortons.isCreated()) m_ssboVortons.destroy();
        if(m_ssboBoids.isCreated())   m_ssboBoids.destroy();
        if(m_vboTraces.isCreated())   m_vboTraces.destroy();
        return;
    }

    if(m_bResetFlowPanels)
    {
        m_pP3UniAnalysis->initializeAnalysis(s_pXPlane->curPlPolar(),0);
        m_pP3UniAnalysis->setTriMesh(s_pXPlane->curPlane()->triMesh());
        m_pP3UniAnalysis->setVortons(pPOpp->m_Vorton);
        m_pP3UniAnalysis->makeWakePanels(Vector3d(1.0,0,0), pWPolar->bVortonWake());

        // Create a VBO and an SSBO for the vortices
        // VBO is used for display and SSBO is used in the compute shader
        int NPanels = m_pP3UniAnalysis->nPanels();
        int NWakePanels = m_pP3UniAnalysis->nWakePanels();
        // create the minimal SSBO
        //need to use 4 components for positions due to std140/430 padding constraints for vec3
        int buffersize = (NPanels+NWakePanels)*(8*4);

        QVector<float>BufferArray(buffersize);
        int iv=0;
        for(int i=0; i<NPanels; i++)
        {
            Panel3 const &p3 = m_pP3UniAnalysis->panel3At(i);

            BufferArray[iv++] = p3.area();
            BufferArray[iv++] = p3.minSize();
            BufferArray[iv++] = pPOpp->sigma(i);
            BufferArray[iv++] = pPOpp->gamma(3*i);

            for(int j=0; j<3; j++)
            {
                BufferArray[iv++] = p3.vertexAt(j).xf();
                BufferArray[iv++] = p3.vertexAt(j).yf();
                BufferArray[iv++] = p3.vertexAt(j).zf();
                BufferArray[iv++] = 0.0f;
            }

            BufferArray[iv++] = p3.CoG().xf();
            BufferArray[iv++] = p3.CoG().yf();
            BufferArray[iv++] = p3.CoG().zf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = p3.m_l.xf();
            BufferArray[iv++] = p3.m_l.yf();
            BufferArray[iv++] = p3.m_l.zf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = p3.m_m.xf();
            BufferArray[iv++] = p3.m_m.yf();
            BufferArray[iv++] = p3.m_m.zf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = p3.normal().xf();
            BufferArray[iv++] = p3.normal().yf();
            BufferArray[iv++] = p3.normal().zf();
            BufferArray[iv++] = 0.0f;
        }

        // append the wake panels
        // make the wake doublet densities
        QVector<float> gammw(NWakePanels);
        gammw.fill(0); //redundant
        float sign = 1.0f;
        for(int i=0; i<NPanels; i++)
        {
            Panel3 const &p3 = m_pP3UniAnalysis->panel3At(i);
            if(p3.isTrailing())
            {
                //If so, we need to add the contribution of the wake column shedded by this panel
                if(p3.isBotPanel()) sign=-1.0f; else sign=1.0f;
                if(p3.iWake()<0) continue;

//                int iRow=0;
                Panel3 const *p3w = &m_pP3UniAnalysis->wakePanelAt(p3.iWake());
                while(p3w)
                {
                    gammw[p3w->index()] += pPOpp->gamma(3*p3.index())*sign;
                    // is there another wake panel downstream?
                    if(p3w->m_iPD>=0) p3w = &m_pP3UniAnalysis->wakePanelAt(p3w->m_iPD);
                    else
                    {
                        p3w = nullptr;
                        break;
                    }

 //                   iRow++;
                }
            }
        }

        for(int iw=0; iw<NWakePanels; iw++)
        {
            Panel3 const &p3 = m_pP3UniAnalysis->wakePanelAt(iw);

            BufferArray[iv++] = p3.area();
            BufferArray[iv++] = p3.minSize();
            BufferArray[iv++] = 0.0;
            BufferArray[iv++] = gammw.at(iw);

            for(int j=0; j<3; j++)
            {
                BufferArray[iv++] = p3.vertexAt(j).xf();
                BufferArray[iv++] = p3.vertexAt(j).yf();
                BufferArray[iv++] = p3.vertexAt(j).zf();
                BufferArray[iv++] = 0.0f;
            }

            BufferArray[iv++] = p3.CoG().xf();
            BufferArray[iv++] = p3.CoG().yf();
            BufferArray[iv++] = p3.CoG().zf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = p3.m_l.xf();
            BufferArray[iv++] = p3.m_l.yf();
            BufferArray[iv++] = p3.m_l.zf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = p3.m_m.xf();
            BufferArray[iv++] = p3.m_m.yf();
            BufferArray[iv++] = p3.m_m.zf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = p3.normal().xf();
            BufferArray[iv++] = p3.normal().yf();
            BufferArray[iv++] = p3.normal().zf();
            BufferArray[iv++] = 0.0f;
        }
        Q_ASSERT(iv==buffersize);


        if(m_ssboPanels.isCreated()) m_ssboPanels.destroy();
        m_ssboPanels.create();
        m_ssboPanels.bind();
        {
            m_ssboPanels.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
//            qDebug("ssboPanels %f MB", float(m_ssboPanels.size())/1024.0f/1024.0f);
        }
        m_ssboPanels.release();

        int NVortons = pPOpp->vortonCount();
        // create the minimal SSBO
        //need to use 4 components for positions due to std140/430 padding constraints for vec3
        buffersize = (NVortons)*(4+4); // 4 components for position and 4 for vorticity;

        BufferArray.resize(buffersize);
        iv=0;
        for(uint ir=0; ir<pPOpp->m_Vorton.size(); ir++)
        {
            for(uint jc=0; jc<pPOpp->m_Vorton.at(ir).size(); jc++)
            {
                Vorton const &vtn = pPOpp->m_Vorton.at(ir).at(jc);

                BufferArray[iv++] = vtn.xf();
                BufferArray[iv++] = vtn.yf();
                BufferArray[iv++] = vtn.zf();
                BufferArray[iv++] = 1.0f;

                BufferArray[iv++] = vtn.vortex().xf();
                BufferArray[iv++] = vtn.vortex().yf();
                BufferArray[iv++] = vtn.vortex().zf();
                if(vtn.isActive())  BufferArray[iv++] = 1.0f;
                else                BufferArray[iv++] = 0.0f;
            }
        }
        Q_ASSERT(iv==buffersize);

        if(m_ssboVortons.isCreated()) m_ssboVortons.destroy();
        m_ssboVortons.create();
        m_ssboVortons.bind();
        {
            m_ssboVortons.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
//            qDebug("m_ssboVortons %f MB", float(m_ssboPanels.size())/1024.0f/1024.0f);
        }
        m_ssboVortons.release();

        m_bResetFlowPanels = false;
    }

    if(m_bResetBoids)
    {
        PlanePolar const *pWPolar = s_pXPlane->curPlPolar();
        PlaneOpp const *pPOpp = s_pXPlane->curPOpp();
        if(!pPOpp || !pPOpp->isTriUniformMethod()) return;

        if(!pWPolar)
        {
            m_Boid.clear();
            return;
        }

        int nBoids = FlowCtrls::s_FlowNGroups * GROUP_SIZE;
        m_Boid.resize(nBoids);

        for(int inboid=0; inboid<nBoids; inboid++)
        {
            Boid &boid = m_Boid[inboid];
            boid.m_Position.x = FlowCtrls::flowTopLeft().x + QRandomGenerator::global()->generateDouble() * (FlowCtrls::flowBotRight().x-FlowCtrls::flowTopLeft().x);
            boid.m_Position.y = FlowCtrls::flowTopLeft().y + QRandomGenerator::global()->generateDouble() * (FlowCtrls::flowBotRight().y-FlowCtrls::flowTopLeft().y);
            boid.m_Position.z = FlowCtrls::flowTopLeft().z + QRandomGenerator::global()->generateDouble() * (FlowCtrls::flowBotRight().z-FlowCtrls::flowTopLeft().z);

            boid.Index = inboid;
            boid.m_Velocity.set(0,0,0);
        }

        // use only one buffer object used both as VBO and SSBO
        int NBoids = m_Boid.size();

        //need to use v4 vertices for velocity due to std140/430 padding constraints for vec3:
        int buffersize = NBoids*(4+4+4); //4 vertices + 4 velocity + 4 color components for each boid


        QColor clr(xfl::fromfl5Clr(W3dPrefs::s_FlowStyle.m_Color));
        QVector<float>BufferArray(buffersize);
        int iv=0;
        for(int i=0; i<NBoids; i++)
        {
            Boid const &boid = m_Boid.at(i);
            BufferArray[iv++] = boid.m_Position.xf();
            BufferArray[iv++] = boid.m_Position.yf();
            BufferArray[iv++] = boid.m_Position.zf();
            BufferArray[iv++] = 1.0f;

            BufferArray[iv++] = boid.m_Velocity.xf();
            BufferArray[iv++] = boid.m_Velocity.xf();
            BufferArray[iv++] = boid.m_Velocity.xf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = clr.redF();
            BufferArray[iv++] = clr.greenF();
            BufferArray[iv++] = clr.blueF();
            BufferArray[iv++] = 1.0f;
        }
        Q_ASSERT(iv==buffersize);

        if(m_ssboBoids.isCreated()) m_ssboBoids.destroy();
        m_ssboBoids.create();
        m_ssboBoids.bind();
        {
            m_ssboBoids.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
//            qDebug("Boids %f  MB", float(m_ssboBoids.size())/1024.0f/1024.0f);
        }
        m_ssboBoids.release();

        buffersize = NBoids*TRACESEGS*2*(4+4); //TRACESEGS segments x 2 pts x (4 vertices + 4 color components)
        BufferArray.resize(buffersize);
        iv=0;
        for(int i=0; i<NBoids; i++)
        {
            Boid const &boid = m_Boid.at(i);
            //  write TRACESEGS segments x 2 vertices, with all vertices set to the boid's position as a starting point
            for(int j=0; j<TRACESEGS; j++)
            {
                for(int k=0; k<2; k++)
                {
                    BufferArray[iv++] = boid.m_Position.xf();
                    BufferArray[iv++] = boid.m_Position.yf();
                    BufferArray[iv++] = boid.m_Position.zf();
                    BufferArray[iv++] = 1.0f;
                    BufferArray[iv++] = clr.redF();
                    BufferArray[iv++] = clr.greenF();
                    BufferArray[iv++] = clr.blueF();
                    BufferArray[iv++] = 1.0f;
                }
            }
        }
        Q_ASSERT(iv==buffersize);
        if(m_vboTraces.isCreated()) m_vboTraces.destroy();
        m_vboTraces.create();
        m_vboTraces.bind();
        {
            m_vboTraces.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
//            qDebug("Boids trace size = %.2f MB", float(m_vboTraces.size())/1024.0f/1024.0f);
        }
        m_vboTraces.release();

        m_bResetBoids = false;
    }
}


void gl3dXPlaneView::moveBoids()
{
#ifndef Q_OS_MAC


    if(oglMajor()*10+oglMinor()<43) return;

    PlanePolar const *pWPolar     = s_pXPlane->curPlPolar();
    PlaneOpp const *pPOpp     = s_pXPlane->curPOpp();
    if(!pPOpp || !pPOpp->isTriUniformMethod()) return;

    int NPanels = m_pP3UniAnalysis->nPanels();
    int NWakePanels = m_pP3UniAnalysis->nWakePanels();
    float Qinf = float(pPOpp->QInf());

    m_shadFlow.bind();
    {
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_NPanels,      NPanels+NWakePanels);
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_VInf,         QVector4D(Qinf,0,0,0));
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_NVorton,      pPOpp->vortonCount());
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_VtnCoreSize,  float(pWPolar->vortonCoreSize()*pWPolar->referenceChordLength()));
        if     (pWPolar->bGroundEffect())      m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasGround, 1);
        else if(pWPolar->bFreeSurfaceEffect()) m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasGround, -1);
        else                                   m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasGround, 0);
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_GroundHeight, float(pWPolar->groundHeight()));

        switch(FlowCtrls::s_ODE)
        {
            case FlowCtrls::EULER:  m_shadFlow.setUniformValue(m_shadFlowLoc.m_RK, 1);    break;
            case FlowCtrls::RK2:    m_shadFlow.setUniformValue(m_shadFlowLoc.m_RK, 2);    break;
            case FlowCtrls::RK4:    m_shadFlow.setUniformValue(m_shadFlowLoc.m_RK, 4);    break;
        }

        m_shadFlow.setUniformValue(m_shadFlowLoc.m_Dt,           FlowCtrls::s_Flowdt);

        m_shadFlow.setUniformValue(m_shadFlowLoc.m_TopLeft,      QVector4D(FlowCtrls::flowTopLeft().x,  FlowCtrls::flowTopLeft().y,  FlowCtrls::flowTopLeft().z,  0.0));
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_BotRight,     QVector4D(FlowCtrls::flowBotRight().x, FlowCtrls::flowBotRight().y, FlowCtrls::flowBotRight().z, 0.0));

        m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasUniColor,  1);
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_UniColor,     xfl::fromfl5Clr(W3dPrefs::s_FlowStyle.m_Color));

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboBoids.bufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboPanels.bufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ssboVortons.bufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_vboTraces.bufferId());

        glDispatchCompute(FlowCtrls::s_FlowNGroups, 1, 1);

//        glFlush();
    }
    m_shadFlow.release();

#endif
}


void gl3dXPlaneView::cancelFlow()
{
    m_FlowTimer.stop();
}


void gl3dXPlaneView::startFlow(bool bStart)
{
    if(bStart) m_FlowTimer.start(FLOWPERIOD);
    else       m_FlowTimer.stop();
    update();
}


void gl3dXPlaneView::restartFlow()
{
    startFlow(false);

    m_bResetBoids = true; //make a new set of boids and update the SSBO & VBO

    update();
    startFlow(true);
}


void gl3dXPlaneView::glRenderFlow()
{
    if(!m_pPOpp3dControls->isFlowActive()) return;

    Plane    const *pPlane  = s_pXPlane->curPlane();
    PlanePolar   const *pWPolar = s_pXPlane->curPlPolar();
    PlaneOpp const *pPOpp   = s_pXPlane->curPOpp();
    if(!pPlane || !pWPolar || !pPOpp) return;

    moveBoids();

    paintColourSegments8(m_vboTraces, float(W3dPrefs::s_FlowStyle.m_Width), W3dPrefs::s_FlowStyle.m_Stipple);

    m_pPOpp3dControls->m_pFlowCtrls->setFPS();
}

double gl3dXPlaneView::objectReferenceLength() const
{
    if(s_pXPlane->curPlane()) return s_pXPlane->curPlane()->span();
    return 1.0;
}

