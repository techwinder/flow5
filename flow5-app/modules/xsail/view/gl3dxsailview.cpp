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
#include <QMenu>
#include <QPainter>
#include <QContextMenuEvent>
#include <QOpenGLPaintDevice>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>

#include "gl3dxsailview.h"



#include <api/boat.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/frame.h>
#include <api/fuse.h>
#include <api/fuseocc.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/geom_global.h>
#include <api/p3linanalysis.h>
#include <api/p3unianalysis.h>
#include <api/p4analysis.h>
#include <api/sailnurbs.h>
#include <api/sailocc.h>
#include <api/sailspline.h>
#include <api/sailwing.h>
#include <api/triangle3d.h>
#include <api/utils.h>

#include <core/displayoptions.h>
#include <api/units.h>
#include <api/trace.h>
#include <core/xflcore.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/flowctrls.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/poppctrls/streamlinesctrls.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/controls/fine3dcontrols.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <modules/xplane/analysis/streamlinemaker.h>
#include <modules/xplane/glview/glxplanebuffers.h>
#include <modules/xsail/controls/xsaildisplayctrls.h>
#include <modules/xsail/menus/xsailmenus.h>
#include <modules/xsail/xsail.h>



double gl3dXSailView::s_GammaCoef;
bool gl3dXSailView::s_bResetglBoat       = true;
bool gl3dXSailView::s_bResetglMesh       = true;
bool gl3dXSailView::s_bResetglWake       = true;
bool gl3dXSailView::s_bResetglBtOpp      = true;
bool gl3dXSailView::s_bResetglLift       = true;
bool gl3dXSailView::s_bResetglMoments    = true;
bool gl3dXSailView::s_bResetglDrag       = true;
bool gl3dXSailView::s_bResetglPanelGamma = true;
bool gl3dXSailView::s_bResetglPanelCp    = true;
bool gl3dXSailView::s_bResetglPanelForce = true;
bool gl3dXSailView::s_bResetglStream     = true;
bool gl3dXSailView::s_bResetglBodyMesh   = true;
bool gl3dXSailView::s_bResetglSail       = true;
bool gl3dXSailView::s_bResetglHull       = true;


XSail *gl3dXSailView::s_pXSail = nullptr;

gl3dXSailView::gl3dXSailView() : gl3dXflView()
{
    m_pDisplayCtrls = nullptr;
    m_pOpp3dScalesCtrls = nullptr;
    m_pCrossFlowCtrls = nullptr;

    m_LiveCtrlParam = 0.0;

    m_PickedVal = 0.0;

    m_bWindFront =  m_bWindBack = false;

    m_nBlocks = 1;

    m_FlowYPos = 0.0;
    m_bResetFlowPanels = m_bResetBoids = true;

    m_pP3UniAnalysis = new P3UniAnalysis;
    m_pP3LinAnalysis = new P3LinAnalysis;

    connect(&m_FlowTimer, SIGNAL(timeout()), this, SLOT(update()));
}


gl3dXSailView::~gl3dXSailView()
{
    for(int i=0; i<m_vboSailSurface.size(); i++) if(m_vboSailSurface[i].isCreated()) m_vboSailSurface[i].destroy();
    m_vboSailSurface.clear();

    for(int i=0; i<m_vboSailOutline.size(); i++) if(m_vboSailOutline[i].isCreated()) m_vboSailOutline[i].destroy();
    m_vboSailOutline.clear();

    for(int i=0; i<m_vboSailNormals.size(); i++) if(m_vboSailNormals[i].isCreated()) m_vboSailNormals[i].destroy();
    m_vboSailNormals.clear();

    for(int i=0; i<m_vboSailMesh.size(); i++) if(m_vboPanelCp.isCreated()) m_vboSailMesh[i].destroy();
    m_vboSailMesh.clear();

    if(m_vboPanelCp.isCreated())        m_vboPanelCp.destroy();
    if(m_vboPanelForces.isCreated())    m_vboPanelForces.destroy();

    if(m_vboTTWSpline.isCreated())       m_vboTTWSpline.destroy();
//    if(m_vboWater.isCreated())          m_vboWater.destroy();

    if(m_vboQuadWake.isCreated())       m_vboQuadWake.destroy();
    if(m_vboTriWake.isCreated())        m_vboTriWake.destroy();

    if(m_vboGridVelocities.isCreated()) m_vboGridVelocities.destroy();

    if(m_vboTriMesh.isCreated())        m_vboTriMesh.destroy();
    if(m_vboTriMeshEdges.isCreated())   m_vboTriMeshEdges.destroy();
    if(m_vboNormals.isCreated())        m_vboNormals.destroy();

    if(m_pP3UniAnalysis) delete m_pP3UniAnalysis;
    if(m_pP3LinAnalysis) delete m_pP3LinAnalysis;

}


void gl3dXSailView::setResultControls(XSailDisplayCtrls *pResults3dControls, CrossFlowCtrls *pContourCtrls)
{
    m_pDisplayCtrls = pResults3dControls;
    m_pCrossFlowCtrls = pContourCtrls;
}


void gl3dXSailView::setLiveVortons(std::vector<std::vector<Vorton>> const &vortons)
{
    m_LiveVortons = vortons;
    s_bResetglVorticity = true;
}


/**
*Overrides the contextMenuEvent method of the base class.
*/
void gl3dXSailView::contextMenuEvent (QContextMenuEvent * pEvent)
{
    m_bArcball = false;
    update();

    if (s_pXSail->is3dView())
    {
        s_pXSail->m_pMenus->m_p3dCtxMenu->exec(pEvent->globalPos());
    }
}


void gl3dXSailView::setBoat(Boat const*pBoat)
{
    updatePartFrame(pBoat);
    if(pBoat)
    {
        setBotLeftOutput(pBoat->properties(false));
        setReferenceLength(pBoat->referenceLength()*2);
    }
    else
    {
        setBotLeftOutput(QString());
    }
}


void gl3dXSailView::hideEvent(QHideEvent *pEvent)
{
    onCancelThreads();
    m_pDisplayCtrls->s_bStreamLines = false;
    cancelFlow();
    gl3dXflView::hideEvent(pEvent);
}


void gl3dXSailView::resizeSailBuffers(int nSails)
{
    // initialize buffer arrays
    for(int i=0; i<m_vboSailSurface.size(); i++) m_vboSailSurface[i].destroy();
    m_vboSailSurface.clear();
    m_vboSailSurface.resize(nSails);

    for(int i=0; i<m_vboSailOutline.size(); i++) m_vboSailOutline[i].destroy();
    m_vboSailOutline.clear();
    m_vboSailOutline.resize(nSails);

    for(int i=0; i<m_vboSailNormals.size(); i++) m_vboSailNormals[i].destroy();
    m_vboSailNormals.clear();
    m_vboSailNormals.resize(nSails);

    for(int i=0; i<m_vboSailMesh.size(); i++) m_vboSailMesh[i].destroy();
    m_vboSailMesh.clear();
    m_vboSailMesh.resize(nSails);
}


void gl3dXSailView::glMake3dObjects()
{
    if(!s_pXSail->curBoat())
    {
        setBoat(nullptr);
        return;
    }
    Boat const *pBoat = s_pXSail->curBoat();
    //BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    //make fuse geometry

    if(s_bResetglBoat)
        setBoat(pBoat);

    if(s_bResetglHull || s_bResetglBoat)
    {
        m_vboFuseOutline.resize(pBoat->nHulls());
        m_vboFuseTriangulation.resize(pBoat->nHulls());

//        glMakeCube(Vector3d(0,0,- W3dPrefs::s_BoxZ/2.0), W3dPrefs::s_BoxX,  W3dPrefs::s_BoxY, W3dPrefs::s_BoxZ, m_vboWater);

        for(int ifuse=0; ifuse<pBoat->nHulls(); ifuse++)
        {
            Fuse const *pFuse = pBoat->hullAt(ifuse);

            if(pFuse->isXflType())
            {
                FuseXfl const *pFxfl = dynamic_cast<FuseXfl const*>(pFuse);
                if (pFxfl->isSplineType())
                {
//                    glMakeNurbsOutline(pFxfl->nurbs(), pFxfl->position(), m_vboBodyOutline[ifuse]);
                    gl::makeBodySplines_outline(pFxfl, pFxfl->position(), m_vboFuseOutline[ifuse]);
                }
                else if(pFxfl->isFlatFaceType())
                {
                    gl::makeBodyFlatFaces_2triangles_outline(pFxfl, pFxfl->position(), m_vboFuseOutline[ifuse]);
                }
                gl::makeTriangulation3Vtx(pFuse->triangulation(),  pFuse->position(),
                                        m_vboFuseTriangulation[ifuse], pFxfl->isFlatFaceType());
            }
            else if(pFuse->isOccType())
            {
                FuseOcc const*pFocc = dynamic_cast<FuseOcc const*>(pFuse);
                //                glMakeTriangulation(pTranslatedFocc->triangulation(), Vector3d(), m_vboFuseTriangulation[ifuse]);
                gl::glMakeShellOutline(pFocc->shells(), pFocc->position(), m_vboFuseOutline[ifuse]);
                gl::makeTriangulation3Vtx(pFuse->triangulation(), pFuse->position(), m_vboFuseTriangulation[ifuse], false);

            }
            else if(pFuse->isStlType())
            {
                gl::makeTriangulation3Vtx(pFuse->triangulation(), pFuse->position(), m_vboFuseTriangulation[ifuse], true);
                gl::makeTrianglesOutline(pFuse->triangles(), pFuse->position(), m_vboFuseOutline[ifuse]);
            }
            gl::makeTrianglesOutline(pFuse->triangles(), pFuse->position(), m_vboTessHull);
        }
    }

    if(s_bResetglSail || s_bResetglBoat)
    {
        int nSails = pBoat->nSails();
        resizeSailBuffers(nSails);
        for(int isail=0; isail<pBoat->nSails(); isail++)
        {
            Sail const *pSail = pBoat->sailAt(isail);
//            Vector3d position = pSail->position();
            Vector3d position;
            gl::makeTriangulation3Vtx(pSail->triangulation(), position, m_vboSailSurface[isail], false);
            if(pSail->isSplineSail())
            {
                SailSpline const*pSSail = dynamic_cast<SailSpline const*>(pSail);
                gl::makeSplineSailOutline(pSSail, position, m_vboSailOutline[isail]);
            }
            else if(pSail->isNURBSSail())
            {
                SailNurbs const*pNurbsSail = dynamic_cast<SailNurbs const*>(pSail);
                gl::makeNurbsOutline(pNurbsSail->nurbs(), position, Sail::iXRes(), Sail::iZRes(), m_vboSailOutline[isail]);
            }
            else if(pSail->isWingSail())
            {
                SailWing const*pWSail = dynamic_cast<SailWing const*>(pSail);
                gl::makeWingSailOutline(pWSail, position, m_vboSailOutline[isail]);
            }
            else if(pSail->isOccSail())
            {
                SailOcc const *pOccSail = dynamic_cast<SailOcc const *>(pSail);
                gl::glMakeShellOutline(pOccSail->shapes(), position, m_vboSailOutline[isail]);
            }
            else if(pSail->isStlSail())
            {
                gl::makeTrianglesOutline(pSail->triangles(), position, m_vboSailOutline[isail]);
            }

            gl::makeTrianglesOutline(pSail->triangles(), position, m_vboTessSail);
        }
    }

    glMakeMeshBuffers();
    glMakeOppBuffers();

    s_bResetglBoat = s_bResetglHull = s_bResetglSail = false;
}


void gl3dXSailView::glMakeMeshBuffers()
{
    Boat const*pBoat = s_pXSail->curBoat();
    BoatPolar const*pBtPolar = s_pXSail->curBtPolar();
    if(s_bResetglMesh || s_bResetglBoat || s_bResetglSail || s_bResetglHull)
    {
        if(pBtPolar && pBtPolar->isQuadMethod())
        {
            // not activated
        }
        else if(!pBtPolar || pBtPolar->isTriangleMethod())
        {
            std::vector<Panel3> panel3list;
            makeVisiblePanel3List(pBoat, pBtPolar, panel3list);
            gl::makeTriPanels(panel3list, Vector3d(), m_vboTriMesh);
            gl::makeTriEdges(panel3list, Vector3d(), m_vboTriMeshEdges);
            gl::makePanelNormals(panel3list, float(m_RefLength)/50.0f, m_vboPanelNormals);

            std::vector<Panel3> p3High;
            for(int ip=0; ip<m_PanelHightlight.size(); ip++)
            {
                int idx = m_PanelHightlight.at(ip);
                if(idx>=0 && idx<pBoat->nPanel3())
                    p3High.push_back(pBoat->triMesh().panelAt(idx));
            }
            gl::makeTriEdges(p3High, Vector3d(), m_vboHighlightPanel3);
        }
    }

    if(s_bResetglWake || s_bResetglMesh || s_bResetglBoat || s_bResetglSail || s_bResetglHull)
    {
        BoatOpp const*pBtOpp  = s_pXSail->curBtOpp();
        if(pBtOpp)
        {
            if(pBtOpp->isQuadMethod())
            {
                // not activated
            }
            else  if(pBtOpp->isTriangleMethod())
                gl::makeTriPanels(pBoat->triMesh().wakePanels(), Vector3d(), m_vboTriWake);
        }
    }


    if(s_bResetglBoat || s_bResetglSail || s_bResetglMesh)
    {
        Vector3d Normal(0.0,0.0,1.0);
        Node A( W3dPrefs::s_BoxX,  W3dPrefs::s_BoxY, 0.0, Normal);
        Node B( W3dPrefs::s_BoxX, -W3dPrefs::s_BoxY, 0.0, Normal);
        Node C(-W3dPrefs::s_BoxX, -W3dPrefs::s_BoxY, 0.0, Normal);
        Node D(-W3dPrefs::s_BoxX,  W3dPrefs::s_BoxY, 0.0, Normal);
        gl::makeQuad(A,B,C,D, m_vboHPlane);
    }


    s_bResetglMesh = s_bResetglWake = false;
}


void gl3dXSailView::glMakeOppBuffers()
{
    Boat      const*pBoat    = s_pXSail->curBoat();
    BoatPolar const*pBtPolar = s_pXSail->curBtPolar();
    BoatOpp        *pBtOpp   = s_pXSail->curBtOpp(); // not const to make node values on the fly
    if(m_LiveVortons.size() && s_bResetglVorticity)
    {
        gl::makeVortons(m_LiveVortons, m_vboVortons);
        s_bResetglVorticity = false;
    }

    if(!pBtOpp || !pBtPolar || !pBoat)
    {
        setBotRightOutput(QString());
        return;
    }

    if(s_bResetglBtOpp)
    {
        std::string strange;
        pBtOpp->getProperties(pBoat, pBtPolar->density(), strange);
        setBotRightOutput(strange);
    }

    float qDyn = 0.5f * float(pBtPolar->density()*pBtOpp->QInf()*pBtOpp->QInf());

    std::vector<Panel3> panel3list;
    if(s_bResetglBoat || s_bResetglMesh || s_bResetglPanelCp || s_bResetglPanelGamma || s_bResetglBtOpp || s_bResetglStream)
    {
        makeVisiblePanel3List(pBoat, pBtPolar, panel3list);
    }

    if(s_bResetglBoat || s_bResetglMesh || s_bResetglPanelCp || s_bResetglPanelGamma || s_bResetglBtOpp)
    {
        //make wind
        glMakeWindSpline();

        if(pBtPolar && pBtPolar->isQuadMethod())
        {
            // not activated
        }
        else if(pBtPolar && pBtPolar->isTriangleMethod())
        {
            if(pBtOpp->Cp().size()>0)
            {
                if(pBtOpp->isTriUniformMethod())
                {
                    if(XSailDisplayCtrls::s_bGamma)
                    {
                        double lmin =Opp3dScalesCtrls::s_GammaMin;
                        double lmax =Opp3dScalesCtrls::s_GammaMax;
                        gl::makeTriUniColorMap(panel3list, pBtOpp->gamma(), lmin, lmax,Opp3dScalesCtrls::isAutoGammaScale(), m_vboPanelCp);
                        if(Opp3dScalesCtrls::isAutoGammaScale()) m_pOpp3dScalesCtrls->updateGammaRange(lmin, lmax);
                    }
                    else if  (XSailDisplayCtrls::s_b3dCp)
                    {
                        double lmin =Opp3dScalesCtrls::CpMin();
                        double lmax =Opp3dScalesCtrls::s_CpMax;
                        gl::makeTriUniColorMap(panel3list, pBtOpp->Cp(), lmin, lmax, Opp3dScalesCtrls::isAutoCpScale(), m_vboPanelCp);
                        if(Opp3dScalesCtrls::isAutoCpScale()) m_pOpp3dScalesCtrls->updateCpRange(lmin, lmax);
                    }
                }
                else if(pBtOpp->isTriLinearMethod())
                {
                    if(XSailDisplayCtrls::s_bGamma)
                    {
                        double lmin=0, lmax=0;

                        TriMesh::makeNodeValues(pBoat->triMesh().nodes(), pBoat->triMesh().panels(),
                                                pBtOpp->gamma(), pBtOpp->m_NodeValue,
                                                pBtOpp->m_NodeValMin, pBtOpp->m_NodeValMax, 1.0);

                        if(Opp3dScalesCtrls::isAutoGammaScale())
                        {
                            lmin = pBtOpp->m_NodeValMin;
                            lmax = pBtOpp->m_NodeValMax;
                            m_pOpp3dScalesCtrls->updateGammaRange(lmin, lmax);
                        }
                        else
                        {
                            lmin = Opp3dScalesCtrls::gammaMin();
                            lmax = Opp3dScalesCtrls::gammaMax();
                        }

                        gl::makeTriLinColorMap(panel3list, pBoat->triMesh().nodes(),
                                             pBtOpp->m_NodeValue,
                                             lmin, lmax,
                                             m_vboPanelCp);

                    }
                    else if(XSailDisplayCtrls::s_b3dCp)
                    {
                        double lmin=0, lmax=0;

                        TriMesh::makeNodeValues(pBoat->triMesh().nodes(), pBoat->triMesh().panels(),
                                                pBtOpp->Cp(), pBtOpp->m_NodeValue,
                                                pBtOpp->m_NodeValMin, pBtOpp->m_NodeValMax, 1.0);

                        if(Opp3dScalesCtrls::isAutoCpScale())
                        {
                            lmin = pBtOpp->m_NodeValMin;
                            lmax = pBtOpp->m_NodeValMax;
                            m_pOpp3dScalesCtrls->updateCpRange(lmin, lmax);
                        }
                        else
                        {
                            lmin = Opp3dScalesCtrls::CpMin();
                            lmax = Opp3dScalesCtrls::CpMax();
                        }
                        gl::makeTriLinColorMap(panel3list, pBoat->triMesh().nodes(), pBtOpp->m_NodeValue, lmin, lmax, m_vboPanelCp);
                    }
                }
            }
        }

        m_LegendOverlay.makeLegend();
        s_bResetglPanelCp = false;
        s_bResetglPanelGamma = false;
    }

    if(s_bResetglBoat || s_bResetglMesh || s_bResetglPanelForce || s_bResetglBtOpp)
    {
        if (pBoat && pBtPolar && pBtOpp)
        {
            double rmin = Opp3dScalesCtrls::pressureMin();
            double rmax = Opp3dScalesCtrls::pressureMin();
            if(pBtOpp->isQuadMethod())
            {
                // not activated
            }
            else if (pBtOpp->isTriangleMethod())
            {
                gl::makePanelForces(panel3list, pBtOpp->Cp(), qDyn,
                                  rmin, rmax, Opp3dScalesCtrls::isAutoPressureScale(), Opp3dScalesCtrls::panelForceScale(),
                                  m_vboPanelForces);
            }
            if(Opp3dScalesCtrls::isAutoPressureScale()) m_pOpp3dScalesCtrls->updatePressureRange(rmin, rmax);
        }

        m_LegendOverlay.makeLegend();
        s_bResetglPanelForce = false;
    }

    if((s_bResetglBoat || s_bResetglMesh || s_bResetglBtOpp || s_bResetglStream) && m_pDisplayCtrls->s_bStreamLines && pBtOpp)
    {
        s_bResetglStream = false; //Prevent multiple recalculations if new repaint signal received before streamline build is done

        if(pBtOpp->isQuadMethod())
        {
        }
        else if(pBtOpp->isTriangleMethod())
        {
            glMakeStreamLines(panel3list, pBoat, pBtOpp);
        }
        s_bResetglStream = false;
    }

    if(m_pCrossFlowCtrls->bGridVelocity())
    {
        if(s_bResetglBtOpp || s_bResetglGridVelocities)
        {
            gl::makeArrows(m_pCrossFlowCtrls->m_GridNodesVel, m_pCrossFlowCtrls->m_GridVectors, s_VelocityScale,
                         m_vboGridVelocities);
            s_bResetglGridVelocities = false;
        }
    }

    if(s_bResetglBtOpp || s_bResetglVorticity)
    {
        if(pBtOpp->hasVortons())
        {
            gl::makeVortons(pBtOpp->m_Vorton, m_vboVortons);

            if(m_pCrossFlowCtrls->bVorticityMap())
            {
                double lmin = CrossFlowCtrls::s_OmegaMin;
                double lmax = CrossFlowCtrls::s_OmegaMax;
                bool bAuto = CrossFlowCtrls::s_bAutoOmegaScale;

                gl::makeQuadColorMap(m_vboContourClrs, CrossFlowCtrls::s_nVorticitySamples, CrossFlowCtrls::s_nVorticitySamples,
                                   m_pCrossFlowCtrls->m_GridNodesOmega, m_pCrossFlowCtrls->m_OmegaField,
                                   lmin, lmax, bAuto, xfl::isMultiThreaded());

                gl::makeQuadContoursOnGrid(m_vboContourLines, CrossFlowCtrls::s_nVorticitySamples, CrossFlowCtrls::s_nVorticitySamples,
                                         m_pCrossFlowCtrls->m_GridNodesOmega, m_pCrossFlowCtrls->m_OmegaField, xfl::isMultiThreaded());
            }
        }
        else
        {
            m_vboVortons.bind();
            m_vboVortons.destroy();
            m_vboVortons.release();
        }

        // make vorton line
        gl::makeLines(pBtOpp->vortonLines(), m_vboVortonLines);
        s_bResetglVorticity = false;
    }

    if(s_bResetglBtOpp)
        m_bResetFlowPanels = true;
    glMakeFlowBuffers();

    s_bResetglBtOpp = false;
}


void gl3dXSailView::glMakeSailForces(BoatOpp const *pBtOpp, float qDyn, QOpenGLBuffer &vbo)
{
    if(!pBtOpp) return;

    Boat const *pBoat = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();

    if(!pBoat || !pBtPolar) return;

    Vector3d SailForce;
    float cosa=0, sina2=0, cosa2=0;

    Quaternion Qt; // Quaternion operator to align the reference arrow to the panel's normal
    Vector3d Omega; // rotation vector to align the reference arrow to the panel's normal
    Vector3d O;
    //The vectors defining the reference arrow
    Vector3d R(0.0,0.0,1.0);
    Vector3d R1( 0.05, 0.0, -0.1);
    Vector3d R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    //define the range of values to set the colors in accordance

    float coef = 0.0001f;

    // vertices array size:
    //        nSails x 1 arrow
    //      x3 lines per arrow
    //      x2 vertices per line
    //        x3 components/vertex

    int forceVertexSize = pBoat->nSails() * 3 * 2 * 3;
    QVector<float> forceVertexArray(forceVertexSize);

    int iv=0;
    for (int iSail=0; iSail<pBoat->nSails(); iSail++)
    {
        Sail const *pSail = pBoat->sailAt(iSail);

        //scale force for display
        SailForce = pBtOpp->sailForceFF(iSail) * double(qDyn *Opp3dScalesCtrls::liftScale() *coef);

        O = pSail->center();
        // rotate around the leading edge
        O.rotate(pSail->leadingEdgeAxis(), pBtOpp->sailAngle(iSail));
        //move to boat position
        O.translate(pSail->position());
        // apply the Ryangle
        O.rotate(Vector3d(), Vector3d(0,1,0), pBtOpp->Ry());
        // apply the bank angle
        O.rotate(Vector3d(), Vector3d(1,0,0), pBtOpp->phi());

        // Rotate the reference arrow to align it with the SailForce vector
        if(R.isSame(P))
        {
            Qt.set(0.0, 0.0,0.0,1.0); //Null quaternion
        }
        else
        {
            cosa   = R.dotf(SailForce.normalized());
            sina2  = sqrtf((1.0f - cosa)*0.5f);
            cosa2  = sqrtf((1.0f + cosa)*0.5f);

            Omega = R * SailForce.normalized();//crossproduct
            Omega.normalize();
            Omega *= double(sina2);
            Qt.set(double(cosa2), Omega.x, Omega.y, Omega.z);
        }

        Qt.conjugate(R,  P);
        Qt.conjugate(R1, P1);
        Qt.conjugate(R2, P2);

        // Scale the pressure vector
        P  *= SailForce.norm();
        P1 *= SailForce.norm();
        P2 *= SailForce.norm();

        // Plot

        // depression, point outwards from the surface
        P.set(-P.x, -P.y, -P.z);

        forceVertexArray[iv++] = O.xf();
        forceVertexArray[iv++] = O.yf();
        forceVertexArray[iv++] = O.zf();
        forceVertexArray[iv++] = O.xf()+P.xf();
        forceVertexArray[iv++] = O.yf()+P.yf();
        forceVertexArray[iv++] = O.zf()+P.zf();

        forceVertexArray[iv++] = O.xf()+P.xf();
        forceVertexArray[iv++] = O.yf()+P.yf();
        forceVertexArray[iv++] = O.zf()+P.zf();
        forceVertexArray[iv++] = O.xf()+P.xf()-P1.xf();
        forceVertexArray[iv++] = O.yf()+P.yf()-P1.yf();
        forceVertexArray[iv++] = O.zf()+P.zf()-P1.zf();


        forceVertexArray[iv++] = O.xf()+P.xf();
        forceVertexArray[iv++] = O.yf()+P.yf();
        forceVertexArray[iv++] = O.zf()+P.zf();
        forceVertexArray[iv++] = O.xf()+P.xf()-P2.xf();
        forceVertexArray[iv++] = O.yf()+P.yf()-P2.yf();
        forceVertexArray[iv++] = O.zf()+P.zf()-P2.zf();
    }


    Q_ASSERT(iv==forceVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(forceVertexArray.data(), forceVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}



void gl3dXSailView::initializeGL()
{
    gl3dXflView::initializeGL();

    int oglversion = 10*oglMajor()+oglMinor();
    if(oglversion<43)
    {
        QString strange = QString::asprintf("OpenGL context version is %d.%d.\n"
                                            "Set an OpenGL context version >=4.3 to enable flow animations.\n\n", oglMajor(), oglMinor());
        xfl::trace(strange);
        s_pXSail->displayMessage(strange, true);
        return;
    }

    // flow Compute shader
    QString csrc = ":/shaders/flow/flow3d_CS.glsl";
    m_shadFlow.addShaderFromSourceFile(QOpenGLShader::Compute, csrc);
    if(m_shadFlow.log().length())
    {
        QString strange = QString::asprintf("%s", QString("Compute shader log:"+m_shadFlow.log()).toStdString().c_str());
        xfl::trace(strange);
    }

    if(!m_shadFlow.link())
    {
        xfl::trace("Flow compute shader is not linked");
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


void gl3dXSailView::glRenderView()
{
    if(!s_pXSail->curBoat()) return;
#ifdef QT_DEBUG
    for(int i=0; i<int(PanelAnalysis::s_DebugPts.size()); i++)
        paintIcosahedron(PanelAnalysis::s_DebugPts.at(i), 0.0075/m_glScalef, Qt::darkRed, W3dPrefs::s_OutlineStyle, true, true);

    for(int i=0; i<int(PanelAnalysis::s_DebugVecs.size()); i++)
    {
        if(i<int(PanelAnalysis::s_DebugPts.size()))
            paintThinArrow(PanelAnalysis::s_DebugPts.at(i), PanelAnalysis::s_DebugVecs.at(i)*gl3dXflView::s_VelocityScale,
                           QColor(135,195,95).darker(), 1, Line::SOLID, m_matModel);
    }

    for(uint i=0; i<Surface::s_DebugPts.size(); i++)
        paintIcosahedron(Surface::s_DebugPts.at(i), 0.0075/m_glScalef, Qt::darkCyan, W3dPrefs::s_OutlineStyle, true, true);

    for(uint i=0; i<Surface::s_DebugVecs.size(); i++)
    {
        if(i<Surface::s_DebugPts.size())
            paintThinArrow(Surface::s_DebugPts.at(i), Surface::s_DebugVecs.at(i)*gl3dXflView::s_VelocityScale,
                           Qt::darkYellow, 2, Line::SOLID, m_matModel);
    }
#endif

    glRenderGeometry();
    if(!m_LiveVortons.size())
        glRenderPanelBasedBuffers();
    glRenderOppBuffers();


    if(m_pDisplayCtrls->s_bWater)
    {
        paintQuad(W3dPrefs::s_WaterColor, true, 2.0f, false, true, true, m_vboHPlane);
    }
}


/**
 * The model matrix is used to rotate the hull and sails using the BoatOpp's angles
 */
void gl3dXSailView::glRenderGeometry()
{
    Boat const *pBoat = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp const *pBtOpp = s_pXSail->curBtOpp();

    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);


    // the model is rotated for the rendering of the surfaces and outline
    if(pBtPolar && m_LiveVortons.size())
    {
        double phi  = pBtPolar->phi(m_LiveCtrlParam);
        m_matModel.rotate(float(phi), 1.0f, 0.0f, 0.0f);
        double Ry   = pBtPolar->Ry(m_LiveCtrlParam);
        m_matModel.rotate(float(Ry), 0.0f, 1.0f, 0.0f);
    }
    else if(pBtOpp)
    {
        m_matModel.translate(0, 0, float(pBtOpp->groundHeight()));
        m_matModel.rotate(float(pBtOpp->phi()), 1.0f, 0.0f, 0.0f);
        m_matModel.rotate(float(pBtOpp->Ry()),  0.0f, 1.0f, 0.0f);
    }
    vmMat  = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;

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


    for(int ihull=0; ihull<pBoat->nHulls(); ihull++)
    {
        Fuse const *pFuse = pBoat->hullAt(ihull);
        if(pFuse->isVisible() && ihull<m_vboFuseTriangulation.size())
        {
            if(m_bSurfaces)
            {
                paintTriangles3Vtx(m_vboFuseTriangulation[ihull], pFuse->color(), true, isLightOn());
            }
            if(m_bOutline)
            {
                if(m_SelectedParts.contains(pFuse->uniqueIndex()))
                    paintSegments(m_vboFuseOutline[ihull], W3dPrefs::s_HighStyle);
                else
                    paintSegments(m_vboFuseOutline[ihull], W3dPrefs::s_OutlineStyle);
            }
            if(m_bTessellation)
                paintSegments(m_vboTessHull, W3dPrefs::s_OutlineStyle);
        }
    }

    for(int isail=0; isail<pBoat->nSails(); isail++)
    {
        Sail const *pSail = pBoat->sailAt(isail);
        if(!pSail || !pSail->isVisible()) continue;

        m_matModel.setToIdentity();

        float Ry=0.0f, phi=0.0f, sailangle=0.0f;
        if(pBtPolar && m_LiveVortons.size())
        {
            phi = float(pBtPolar->phi(m_LiveCtrlParam));
            Ry = float(pBtPolar->Ry(m_LiveCtrlParam));
            sailangle = float(pBtPolar->sailAngle(isail, m_LiveCtrlParam));
        }
        else if(pBtOpp)
        {
            phi = float(pBtOpp->phi());
            Ry = float(pBtOpp->Ry());
            sailangle = float(pBtOpp->sailAngle(isail));
        }

        Vector3d LEaxis = pSail->leadingEdgeAxis().normalized();

        if(fabsf(phi)>ANGLEPRECISION) m_matModel.rotate(phi, 1.0f, 0.0f, 0.0f);
        if(fabsf(Ry)>ANGLEPRECISION)  m_matModel.rotate(Ry,  0.0f, 1.0f, 0.0f);
        m_matModel.translate(pSail->position().xf(), pSail->position().yf(), pSail->position().zf());
        m_matModel.translate(pSail->tack().xf(), pSail->tack().yf(), pSail->tack().zf());
        m_matModel.rotate(sailangle, LEaxis.xf(), LEaxis.yf(), LEaxis.zf());
        m_matModel.translate(-pSail->tack().xf(), -pSail->tack().yf(), -pSail->tack().zf());

//            m_pvmMatrix = m_matProj * m_matView * m_matModel;
        vmMat  = m_matView*m_matModel;
        pvmMat = m_matProj*vmMat;
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

        bool bBackGround = m_bSurfaces;
        if(pBtOpp && XSailDisplayCtrls::s_b3dCp) bBackGround=false;

        if(bBackGround)
        {
            if(isail<m_vboSailSurface.size())
                paintTriangles3Vtx(m_vboSailSurface[isail], pSail->color(), true, isLightOn());
        }
        if(m_bOutline && isail<m_vboSailOutline.size())
        {
            paintSegments(m_vboSailOutline[isail], W3dPrefs::s_OutlineStyle);
        }

        if(m_bTessellation)
            paintSegments(m_vboTessSail, W3dPrefs::s_OutlineStyle);
    }


    //leave things as they were
    m_matModel.setToIdentity();
    vmMat  = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;

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

}


/** The mesh panels have been rotated by the BoatOpp's angles, so use the identity model matrix */
void gl3dXSailView::glRenderPanelBasedBuffers()
{
    Boat      const *pBoat    = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp   const *pBtOpp   = s_pXSail->curBtOpp();
/*    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);*/

    if(m_bMeshPanels)
    {
        bool bBackGround = !m_bSurfaces;

        if(XSailDisplayCtrls::s_b3dCp  && pBtOpp) bBackGround = false;
        if(XSailDisplayCtrls::s_bGamma && pBtOpp) bBackGround = false;
        if(pBtPolar && pBtPolar->isQuadMethod())
        {
        }
        else
        {
            if(bBackGround) paintTriPanels(m_vboTriMesh, true);
            paintSegments(m_vboTriMeshEdges, W3dPrefs::s_PanelStyle);

            if(m_PanelHightlight.size())
            {
                paintSegments(m_vboHighlightPanel3, W3dPrefs::s_HighStyle);
                for(int ih=0; ih<m_PanelHightlight.size(); ih++)
                {
                    Panel3 const &p3 = pBoat->triMesh().panelAt(m_PanelHightlight.at(ih));
                    glRenderText(p3.CoG().x+0.03/double(m_glScalef), p3.CoG().y+0.03/double(m_glScalef), p3.CoG().z+0.03/double(m_glScalef),
                                 QString::asprintf("T%d", m_PanelHightlight.at(ih)),
                                 DisplayOptions::textColor(), true);
                }
            }
        }
    }

    if(m_bNormals)
        paintNormals(m_vboPanelNormals);

    if(XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma)
    {
        if(pBtOpp)
        {
            if     (pBtOpp->isQuadMethod())     paintColourMap(m_vboPanelCp, m_matModel); // two triangles/quad
            else if(pBtOpp->isTriangleMethod()) paintColourMap(m_vboPanelCp, m_matModel);
        }
    }

    if(XSailDisplayCtrls::s_bPanelForce)
    {
        if(pBtOpp) paintColorSegments(m_vboPanelForces, W3dPrefs::s_LiftStyle.m_Width);
    }

    if(m_pDisplayCtrls->s_bWakePanels)
    {
        if(pBtOpp)
        {
            if(pBtOpp->isQuadMethod())
                paintTriPanels(m_vboQuadWake, true);
            else if(pBtOpp->isTriangleMethod())
                paintTriPanels(m_vboTriWake, true);
        }
    }

    if(m_PickType==xfl::PANEL3 && m_PickedPanelIndex>=0)
    {
        if(pBtPolar && pBtPolar->isTriangleMethod())
        {
            if(pBtPolar->isTriUniformMethod())
            {
                paintTriangle(m_vboTriangle, true, false, Qt::black);
            }
            else if (pBtPolar->isTriLinearMethod())
            {
                if(pBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
                    paintSphere(m_PickedPoint, 0.0075/double(m_glScalef), Qt::red, true);
                else
                    paintTriangle(m_vboTriangle, true, false, Qt::black);
            }
        }

        QString strong;
        if(pBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
        {
            if     (pBtOpp->isTriUniformMethod()) strong = QString::asprintf("T%d: %g", m_PickedPanelIndex, m_PickedVal);
            else if(pBtOpp->isTriLinearMethod())  strong = QString::asprintf("N%d: %g", m_PickedNodeIndex,  m_PickedVal);
            else if(pBtOpp->isQuadMethod())       strong = QString::asprintf("Q%d: %g", m_PickedPanelIndex, m_PickedVal);
        }
        else if(pBtPolar)
        {
            if(pBtPolar->isTriangleMethod())
            {
                strong = QString::asprintf("T%d", m_PickedPanelIndex);
#ifdef QT_DEBUG
                Panel3 const &p3 = pBoat->triMesh().panelAt(m_PickedPanelIndex);
                for(int in=0; in<3; in++)
                {
                    Node const &nd = p3.vertexAt(in);
                    glRenderText(nd.x+0.015/double(m_glScalef), nd.y+0.015/double(m_glScalef), nd.z+0.015/double(m_glScalef),
                                 QString::asprintf("nd%d", in),
                                 DisplayOptions::textColor(), true);
                }
#endif
            }
            else if(pBtPolar->isQuadMethod()) strong = QString::asprintf("Q%d", m_PickedPanelIndex);
        }
        if(pBtPolar || pBtOpp)
            glRenderText(m_PickedPoint.x+0.03/double(m_glScalef), m_PickedPoint.y+0.03/double(m_glScalef), m_PickedPoint.z+0.03/double(m_glScalef),
                         strong, DisplayOptions::textColor(), true);
    }

    if(bPickNode() && m_bMeshPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<pBoat->triMesh().nNodes())
        {
            Node const &nd = pBoat->node(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<pBoat->nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = pBoat->node(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }
        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<pBoat->nNodes())
        {
            Node const &nd = pBoat->node(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }
}


void gl3dXSailView::glRenderOppBuffers()
{
    Boat const *pBoat = s_pXSail->curBoat();
    BoatOpp const *pBtOpp = s_pXSail->curBtOpp();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    if(!pBtOpp)
    {
        m_LegendOverlay.setVisible(false);
    }

    if(m_pDisplayCtrls->s_bPartForces && pBtOpp && pBtPolar)
    {
        double qDyn = pBtPolar->qDyn(pBtOpp->ctrl());
        double sc =  qDyn * double(Opp3dScalesCtrls::partForceScale())/1.0e5;

        paintThickArrow(pBtPolar->CoG(),
                        pBtOpp->m_AF.Fsum() *sc, xfl::fromfl5Clr(W3dPrefs::s_LiftStyle.m_Color), m_matModel);

        for(int is=0; is<pBoat->nSails(); is++)
        {
            Sail const *pSail = pBoat->sailAt(is);
            Vector3d C = pSail->center() ;
//            pSail->leadingEdgeAxis().listCoords(pSail->name());
            C.rotate(pSail->leadingEdgeAxis().normalized(), pBtOpp->sailAngle(is));
            C.translate(pSail->position());
            C.rotateY(pBtOpp->Ry());
            C.rotateX(pBtOpp->phi());
            paintThinArrow(C,
                           pBtOpp->sailForceSum(is)*sc,
                           W3dPrefs::s_LiftStyle.m_Color, W3dPrefs::s_LiftStyle.m_Width, W3dPrefs::s_LiftStyle.m_Stipple, m_matModel);
        }
    }

    if(pBtOpp && m_pDisplayCtrls->s_bFlow) glRenderFlow();
    else m_FlowTimer.stop();

    //streamlines and velocities are rotated by aoa when constructed
    if(m_pDisplayCtrls->s_bStreamLines && (!s_bResetglStream && !s_bResetglBoat && !s_bResetglMesh && !s_bResetglBtOpp))
    {
        if(!s_bResetglStream)
        {
            paintStreamLines(m_vboStreamlines, m_StreamLineColours, StreamLineCtrls::nX());
        }
    }

    if(s_bVortonLine)
    {
        paintSegments(m_vboVortonLines, W3dPrefs::s_StreamStyle);
    }

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    if(m_pCrossFlowCtrls->bGridVelocity() && pBtOpp)
    {
        paintSegments(m_vboGridVelocities, W3dPrefs::s_VelocityStyle);
    }

    if(m_LiveVortons.size())
    {
        paintSphereInstances(m_vboVortons,
                             W3dPrefs::vortonRadius()/double(m_glScalef), W3dPrefs::vortonColour(), false, true);
    }
    else if(m_pDisplayCtrls->s_bVortons && pBtOpp && pBtOpp->hasVortons())
    {
        paintSphereInstances(m_vboVortons,
                             W3dPrefs::vortonRadius()/double(m_glScalef), W3dPrefs::vortonColour(), false, true);
    }

    if(s_bNegVortices && pBtOpp)
    {
        double coef = 0.1;
        for(uint iv=0; iv<pBtOpp->m_VortexNeg.size(); iv++)
        {
            Vortex const &vtx = pBtOpp->m_VortexNeg.at(iv);
            paintCube(vtx.vertexAt(0).x, vtx.vertexAt(0).y, vtx.vertexAt(0).z, 0.015/m_glScalef, QColor(255,35,75), true);
            paintThinArrow(vtx.vertexAt(0), vtx.segment()*vtx.circulation()*coef, QColor(255,35,75), 2, Line::SOLID, m_matModel);
        }
    }

    if(m_pCrossFlowCtrls->bVorticityMap() && pBtOpp && pBtOpp->m_Vorton.size())
    {
        paintColourMap(m_vboContourClrs, m_matModel);
        paintSegments(m_vboContourLines, W3dPrefs::s_ContourLineStyle);
    }

    if(m_pDisplayCtrls->s_bWind && pBtPolar && pBtOpp && m_TrueWindSpline.size())
    {
        double ctrl = pBtOpp ? pBtOpp->ctrl() : 0.0;
        double aws_inf = pBtPolar->AWSInf(ctrl);
        double awa_inf = pBtPolar->AWAInf(ctrl);
        double Vb = pBtPolar->boatSpeed(ctrl);
        Vector3d VB(-Vb, 0.0, 0.0);
        Vector3d AWS_inf(aws_inf*cos(awa_inf*PI/180.0), aws_inf*sin(awa_inf*PI/180.0), 0.0); // apparent wind speed at high altitude
        Vector3d TWS_inf, TWS, AWS;
        Vector3d O, Trans;

        pBtPolar->trueWindSpeed(ctrl, 1000.0, TWS_inf);
        Trans = TWS_inf.normalized()*-1.3*s_pXSail->curBoat()->length();
        double x=0, y=0, z=0;
        int step = int(double(m_TrueWindSpline.size())/5.0);
        for(int i=0; i<5; i++)
        {
            int is = i*step;
            if(i==4) is = m_TrueWindSpline.size()-1;
            O.set(m_TrueWindSpline.at(is));
            TWS.set(TWS_inf*pBtPolar->windForce(O.z));
//            AWS.set(TWS.x-VB.x, TWS.y-VB.y, 0.0);
            pBtPolar->apparentWind(pBtOpp->ctrl(), O.z, AWS); //WYSIWIG

            O += Trans;
            paintThinArrow(O, TWS, W3dPrefs::s_WindStyle.m_Color, W3dPrefs::s_WindStyle.m_Width, W3dPrefs::s_WindStyle.m_Stipple, m_matModel);
            if(Vb>0.01)
                paintThinArrow(O, AWS, xfl::fromfl5Clr(W3dPrefs::s_WindStyle.m_Color).darker(), W3dPrefs::s_WindStyle.m_Width, Line::DOT, m_matModel);
            if(i==4)
            {
                x = (2.0*(O.x)+ AWS.x)/2.0;
                y = (2.0*(O.y)+ AWS.y)/2.0;
                z = (2.0*(O.z)+ AWS.z)/2.0;
                glRenderText(x+0.015/double(m_glScalef), y+0.015/double(m_glScalef), z+0.015/double(m_glScalef),
                             "AWS" + INFch + QString::asprintf("=%g ", AWS.norm()*Units::mstoUnit())+Units::speedUnitQLabel(),
                             xfl::fromfl5Clr(W3dPrefs::s_WindStyle.m_Color).darker(), true);

                if(Vb>0.01)
                {
                    x=(2.0*(O.x)+ TWS.x)/2.0;
                    y=(2.0*(O.y)+ TWS.y)/2.0;
                    z=(2.0*(O.z)+ TWS.z)/2.0;
                    glRenderText(x+0.015/double(m_glScalef), y+0.015/double(m_glScalef), z+0.015/double(m_glScalef),
                                 "TWS"+INFch+QString::asprintf("=%g ", TWS_inf.norm()*Units::mstoUnit())+Units::speedUnitQLabel(),
                                 xfl::fromfl5Clr(W3dPrefs::s_WindStyle.m_Color), true);

                    paintThinArrow(O+AWS, VB, Qt::darkCyan, W3dPrefs::s_WindStyle.m_Width, W3dPrefs::s_WindStyle.m_Stipple, m_matModel);
                    x=O.x+AWS.x+ VB.x/2.0;
                    y=O.y+AWS.y+ VB.y/2.0;
                    z=O.z+AWS.z+ VB.z/2.0;
                    glRenderText(x+0.015/double(m_glScalef), y+0.015/double(m_glScalef), z+0.015/double(m_glScalef),
                                 QString::asprintf("Boat speed=%g ", Vb*Units::mstoUnit())+Units::speedUnitQLabel(),
                                 Qt::darkCyan, true);
                }
            }
        }

        QMatrix4x4 trans44;
        trans44.translate(Trans.xf(), Trans.yf(), 0.0f);
        QMatrix4x4 pvmMatrix = m_matProj * m_matView * trans44;
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*trans44);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMatrix);
        }
        m_shadLine.release();

        paintLineStrip(m_vboTTWSpline, xfl::fromfl5Clr(W3dPrefs::s_WindStyle.m_Color).darker(), W3dPrefs::s_WindStyle.m_Width, Line::DOT);

    }

    //leave things as they were
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
}


void gl3dXSailView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    m_bArcball = false;
    Boat const *pBoat = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp const *pBtOpp = s_pXSail->curBtOpp();
    if(!isPicking() || m_PressedPoint!=pEvent->pos())
    {
        gl3dXflView::mouseReleaseEvent(pEvent);
        return;
    }
    if(!pBoat || !pBtPolar) return;

    pickTriUniPanel(pEvent->pos());
    if(pBtPolar->isTriLinearMethod() && m_PickedPanelIndex>=0 && m_PickedPanelIndex<pBoat->nPanel3())
    {
        Panel3 const &p3 = pBoat->panel3At(m_PickedPanelIndex);
        pickPanelNode(p3, m_PickedPoint, xfl::NOSURFACE);
    }

    if(m_PickedPanelIndex>=0 && !bPickNode())
    {
        if(!pBtPolar || pBtPolar->isTriUniformMethod() || pBtPolar->isQuadMethod())
        {
            s_pXSail->outputPanelProperties(m_PickedPanelIndex);
        }
        else if(pBtPolar && pBtPolar->isTriLinearMethod())
        {
            if(pBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
                s_pXSail->outputNodeProperties(m_PickedNodeIndex, m_PickedVal);
            else
                s_pXSail->outputPanelProperties(m_PickedPanelIndex);
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

    gl3dXflView::mouseReleaseEvent(pEvent);
}


void gl3dXSailView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex = -1;
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    Boat *pBoat = s_pXSail->curBoat();
    BoatPolar *pBtPolar = s_pXSail->curBtPolar();

    if(!isPicking() || !pBoat)
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex = -1;
        return;
    }

    if(pBtPolar)
    {
        pickTriUniPanel(pEvent->pos());
        if(pBtPolar->isTriLinearMethod() && m_PickedPanelIndex>=0 && m_PickedPanelIndex<pBoat->nPanel3())
        {
            Panel3 const &p3 = pBoat->panel3At(m_PickedPanelIndex);
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
        update();
    }
    else
    {
        Panel3 const &p3 = pBoat->panel3At(m_PickedPanelIndex);
        if(bPickNode())
        {
            pickPanelNode(p3, m_PickedPoint, xfl::NOSURFACE);
            if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=pBoat->triMesh().nNodes())
            {
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
            }
            else
            {
                if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<pBoat->nNodes())
                {
                    Node const &nd = pBoat->node(m_PickedNodeIndex);
                    setTopRightOutput(QString::fromStdString(nd.properties()));
                 }
            }
        }
        else
        {
            if(pBtPolar && pBtPolar->isTriangleMethod())
            {
                if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<pBoat->nPanel3())
                {
                    Panel3 const &p3 = pBoat->panel3(m_PickedPanelIndex);
                    gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);
                }
            }
            else if(pBtPolar && pBtPolar->isQuadMethod())
            {
                // not activated
            }
        }
        update();
/*        if(pBtPolar && pBtPolar->isTriangleMethod())
        {
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<pBoat->nPanel3())
            {
                Panel3 const &p3 = pBoat->triMesh().panel(m_PickedPanelIndex);
                m_pglStdBuffers->glMakeTriangle(p3.vertex(0), p3.vertex(1), p3.vertex(2));
            }
        }
        else // if(pWPolar && pWPolar->isQuadMethod())
        {
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<pBoat->nPanel4())
            {
                Panel4  const &p4 = pBoat->quadMesh().panel(m_PickedPanelIndex);
                glMakeQuad(p4.vertex(0), p4.vertex(1), p4.vertex(2), p4.vertex(3), m_pglStdBuffers->m_vboQuad);
            }
        }*/
        update();
    }
}


bool gl3dXSailView::pickTriUniPanel(QPoint const &point)
{
    Vector3d I, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

    Boat      *pBoat    = s_pXSail->curBoat();
//    BoatPolar *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp   *pBtOpp   = s_pXSail->curBtOpp();

    float zmax = +1.e10;
//    double dcrit = 0.05/m_glScaled;
    if(pBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
    {
        // pick element
        for(int i3=0; i3<pBoat->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->panel3(i3);
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
                    m_PickedPoint = p3.CoG();
                    if(p3.index()>=0 && p3.index()<pBtOpp->nPanel3())
                    {
                        if(XSailDisplayCtrls::s_bPanelForce)
                        {
                            m_PickedVal =  pBtOpp->Cp(i3*3)* 0.5*pBtOpp->QInf()*pBtOpp->QInf()*Units::PatoUnit();
                        }
                        else if(XSailDisplayCtrls::s_bGamma)
                        {
                            m_PickedVal = pBtOpp->gamma(i3*3);
                        }
                        else
                        {
                            m_PickedVal = pBtOpp->Cp(i3*3);
                        }
                        m_PickedPanelIndex = i3;
                    }
                }
            }
        }
    }
    else
    {
        for(int i3=0; i3<pBoat->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->panel3(i3);
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
                    m_PickedPoint = p3.CoG();
                    m_PickedPanelIndex = i3;
                }
            }
        }
    }

    return m_PickedPanelIndex>=0;
}


bool gl3dXSailView::pickTriLinPanel(QPoint const &point)
{
    Vector3d I, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

    Boat      *pBoat    = s_pXSail->curBoat();
//    BoatPolar *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp   *pBtOpp   = s_pXSail->curBtOpp();
/*    if(pBtOpp)
    {
        AA.rotateY(-pBtOpp->alpha());
        BB.rotateY(-pBtOpp->alpha());
    }*/

    float zmax = +1.e10;
    float dcrit = 0.05f/m_glScalef;
    if(pBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
    {
        //pick node
        float dist = 0.0f;
        float dmax = 100.0f;
        for(int in=0; in<pBoat->triMesh().nNodes(); in++)
        {
            Node const &node = pBoat->triMesh().node(in);
            dist = float(geom::distanceToLine3d(AA, BB, node));
            // first screening: set a max distance
            if(dist<dmax && dist<dcrit)
            {
                worldToScreen(node, v4d);
                // second screening: keep the node closest to the viewer
                if(v4d.z()<zmax)
                {
                    dmax = dist;
                    zmax = v4d.z();
                    m_PickedPoint = node;
                    m_PickedVal = pBtOpp->nodeValue(node.index());
                    m_PickedPanelIndex = in;
                }
            }
        }
    }
    else
    {
        // pick a panel;

        for(int i3=0; i3<pBoat->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->panel3(i3);
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
                    m_PickedPoint = p3.CoG();
                    m_PickedPanelIndex = i3;
                }
            }
        }
    }
    return m_PickedPanelIndex>=0;
}


void gl3dXSailView::onWindBack()
{
    m_bWindFront = false;
    m_bWindBack = true;

    m_pDisplayCtrls->m_pFineCtrls->onResetCtrls();

    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion Qt1(sqrt(2.0)/2.0, 0.0,            sqrt(2.0)/2.0, 0.0);// rotate by 90 deg around y
    Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90 deg around x
    Quaternion Qt3;

    Vector3d R(0.0, 0.0, 1.0);
    if(m_LiveVortons.size()>0)
    {
        double beta = s_pXSail->m_pCurBtPolar->AWAInf(m_LiveCtrlParam);
        Qt3.set(-beta-180.0, R);// rotate by 180.0+beta around z
    }
    else if(s_pXSail->m_pCurBtOpp)
    {
        Qt3.set(-s_pXSail->m_pCurBtOpp->beta()-180.0, R);// rotate by 180.0+beta around z
    }
    else
    {
        Qt3.set(-180.0, R);// rotate by 180.0 around z
    }

    m_QuatEnd = (Qt1 * Qt2) * Qt3;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
//    emit viewModified();
}


void gl3dXSailView::onWindFront()
{
    m_bWindFront = true;
    m_bWindBack = false;

    m_pDisplayCtrls->m_pFineCtrls->onResetCtrls();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion Qt1(sqrt(2.0)/2.0, 0.0,            sqrt(2.0)/2.0, 0.0);// rotate by 90 deg around y
    Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90 deg around x

    if(!s_pXSail->m_pCurBtOpp)    m_QuatEnd = Qt1 * Qt2;
    else
    {
        Vector3d R(0.0, 0.0, 1.0);
        Quaternion Qt3(-s_pXSail->m_pCurBtOpp->beta(), R);// rotate by beta around z
        m_QuatEnd = (Qt1 * Qt2) * Qt3;
    }
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
//    emit viewModified();
}


void gl3dXSailView::paintForce(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, xfl::fromfl5Clr(W3dPrefs::s_LiftStyle.m_Color));
        m_shadLine.setUniformValue(m_locLine.m_Pattern,  gl::stipple(Line::SOLID));
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        vbo.bind();
        m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));

        // 3 lines x 2 vertices x 3 coords
        int nArrows = vbo.size() /3 /2 /3 / int(sizeof(float));

        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_LiftStyle.m_Width+3.0f));

        // 3 lines x 2 vertices
        glDrawArrays(GL_LINES, 0, nArrows*3*2);

        m_shadLine.setUniformValue(m_locLine.m_UniColor, DisplayOptions::backgroundColor());
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
    vbo.release();
}


void gl3dXSailView::loadXSailSettings(QSettings &settings)
{
    settings.beginGroup("gl3dXSailView");
    {
        m_bOutline    = settings.value("bOutline",    true ).toBool();
        m_bSurfaces   = settings.value("bSurfaces",   true ).toBool();
        m_bMeshPanels = settings.value("bVLMPanels",  true ).toBool();
    }
    settings.endGroup();
}


void gl3dXSailView::saveXSailSettings(QSettings &settings)
{
    settings.beginGroup("gl3dXSailView");
    {
        settings.setValue("bOutline",   m_bOutline);
        settings.setValue("bSurfaces",  m_bSurfaces);
        settings.setValue("bVLMPanels", m_bMeshPanels);
    }
    settings.endGroup();
}


bool gl3dXSailView::glMakeStreamLines(std::vector<Panel3> const &panel3list, Boat const *pBoat, const BoatOpp *pBtOpp)
{
    m_vboStreamlines.destroy();
    if(!pBtOpp) return false;
    if(pBoat->triMesh().nWakePanels()==0) return false; //wake panels are needed

    StreamlineMaker::cancelTasks(false);

    m_StreamLineColours.clear();

    Vector3d C, VA, TC;

    VA.set(1.0,0.0,0.0);
    VA.rotateZ(pBtOpp->beta());
    VA *= StreamLineCtrls::l0();

    QVector<int> indexlist;
    QVector<Vector3d> ptList;
    QVector<Vector3d> v0List;

    if(StreamLineCtrls::startAtTE())
    {
        for (uint i3=0; i3<panel3list.size(); i3++)
        {
            Panel3 const & p3 = panel3list.at(i3);
            if(p3.isTrailing() && p3.surfacePosition()<=xfl::MIDSURFACE)
            {
                if(!indexlist.contains(p3.nodeIndex(1)))
                {
                    indexlist.append(p3.nodeIndex(1));
                    ptList.append(p3.vertexAt(1));
                    v0List.append(VA);
                    if(W3dPrefs::s_bUseWingColour)
                    {
                        for(int is=0; is<pBoat->nSails(); is++)
                        {
                            Sail const *pSail = pBoat->sailAt(is);
                            if(pSail->hasPanel3(i3))
                            {
                                m_StreamLineColours.append(xfl::fromfl5Clr(pSail->color()));
                                break;
                            }
                        }
                    }
                }
                if(!indexlist.contains(p3.nodeIndex(2)))
                {
                    indexlist.append(p3.nodeIndex(2));
                    ptList.append(p3.vertexAt(2));
                    v0List.append(VA);
                    if(W3dPrefs::s_bUseWingColour)
                    {
                        for(int is=0; is<pBoat->nSails(); is++)
                        {
                            Sail const *pSail = pBoat->sailAt(is);
                            if(pSail->hasPanel3(i3))
                            {
                                m_StreamLineColours.append(xfl::fromfl5Clr(pSail->color()));
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

        double range = objectReferenceLength();
        for(int i=0; i<n/2; i++)
        {
            ptList.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range + y0, StreamLineCtrls::ZOffset()*range});
            v0List.append(VA);
            if(i>0)
            {
                ptList.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range - y0, StreamLineCtrls::ZOffset()*range});
                v0List.append(VA);
            }
            y0 += StreamLineCtrls::deltaL();
        }
    }
    else if(StreamLineCtrls::startAtZLine())
    {
        int n= StreamLineCtrls::nStreamLines();

        double range = objectReferenceLength();
        double z0 = 0.0;
        for(int i=0; i<n; i++)
        {
            ptList.append({StreamLineCtrls::XOffset()*range, StreamLineCtrls::YOffset()*range, StreamLineCtrls::ZOffset()*range + z0});
            v0List.append(VA);
            z0 += StreamLineCtrls::deltaL();
        }
    }

    int NStreamLines = ptList.size();
    int streamArraySize =     NStreamLines * (StreamLineCtrls::nX()+1) * 3;

    QVector<float> StreamVertexArray(streamArraySize);

    QVector<Vector3d> Pt(NStreamLines);
    QVector<Vector3d> TPt(NStreamLines);

    int iv = 0;
    StreamlineMaker::cancelTasks(false);

    if(s_pXSail->curBtPolar()->isTriUniformMethod())
    {
        m_pP3UniAnalysis->initializeAnalysis(s_pXSail->curBtPolar(),0);
        m_pP3UniAnalysis->setTriMesh(s_pXSail->curBoat()->triMesh());
    }
    else if(s_pXSail->curBtPolar()->isTriLinearMethod())
    {
        m_pP3LinAnalysis->initializeAnalysis(s_pXSail->curBtPolar(),0);
        m_pP3LinAnalysis->setTriMesh(s_pXSail->curBoat()->triMesh());
    }

    // multithreaded mode only
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFutureSynchronizer<void> futureSync;
    QVector<StreamlineMaker*> makers;

    for (int in=0; in<ptList.size(); in++)
    {
        StreamlineMaker *pMaker = new StreamlineMaker;
        makers.append(pMaker);
        C  = ptList.at(in);
        TC = ptList.at(in);

        TC -= C;
        Pt[in] = C;
        TPt[in] = TC;

        pMaker->setOpp(s_pXSail->curBtPolar(), pBtOpp->QInf(), 0.0, -pBtOpp->beta(), pBtOpp->gamma().data(), pBtOpp->sigma().data());
        if     (s_pXSail->curBtPolar()->isTriUniformMethod()) pMaker->setP3Analysis(m_pP3UniAnalysis);
        else if(s_pXSail->curBtPolar()->isTriLinearMethod())  pMaker->setP3Analysis(m_pP3LinAnalysis);
        pMaker->initializeLineMaker(in, StreamVertexArray.data()+iv, Pt[in], v0List[in], TPt[in],
                                    StreamLineCtrls::nX(), StreamLineCtrls::l0(), StreamLineCtrls::XFactor());
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        futureSync.addFuture(QtConcurrent::run(pMaker, &StreamlineMaker::run));
#else
        futureSync.addFuture(QtConcurrent::run(&StreamlineMaker::run, pMaker));
#endif

//        pMaker->run();
         iv += (StreamLineCtrls::nX()+1)*3;
    }
    futureSync.waitForFinished();

    for(int i=0; i<makers.size(); i++)
    {
        delete makers[i];
    }

    m_vboStreamlines.destroy();
    m_vboStreamlines.create();
    m_vboStreamlines.bind();
    m_vboStreamlines.allocate(StreamVertexArray.data(), StreamVertexArray.size()* int(sizeof(float)));
    m_vboStreamlines.release();

    QApplication::restoreOverrideCursor();

    return true;
}


void gl3dXSailView::computeP3VelocityVectors(Opp3d const *pBtOpp, QVector<Vector3d> const&points,  QVector<Vector3d> &velvectors)
{
    int nPoints = points.size();
    velvectors.resize(nPoints);

    if(s_pXSail->curBtPolar()->isTriUniformMethod())
    {
        m_pP3UniAnalysis->setTriMesh(s_pXSail->curBoat()->triMesh());
        m_pP3UniAnalysis->initializeAnalysis(s_pXSail->curBtPolar(), 0);
        m_pP3UniAnalysis->setVortons(pBtOpp->m_Vorton);
    }
    else if(s_pXSail->curBtPolar()->isTriLinearMethod())
    {
        m_pP3LinAnalysis->setTriMesh(s_pXSail->curBoat()->triMesh());
        m_pP3LinAnalysis->initializeAnalysis(s_pXSail->curBtPolar(), 0);
        m_pP3LinAnalysis->setVortons(pBtOpp->m_Vorton);
    }

    if(xfl::isMultiThreaded())
    {
        m_nBlocks = QThreadPool::globalInstance()->maxThreadCount();
        QFutureSynchronizer<void> futureSync;

        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &gl3dXSailView::makeTriVelocityBlock,
                                                       iBlock, points, pBtOpp->gamma().data(), pBtOpp->sigma().data(), velvectors.data()));
#else
            futureSync.addFuture(QtConcurrent::run(&gl3dXSailView::makeTriVelocityBlock, this,
                                                       iBlock, points, pBtOpp->gamma().data(), pBtOpp->sigma().data(), velvectors.data()));
#endif
         }
        futureSync.waitForFinished();
    }
    else
    {
        m_nBlocks = 1;
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
            makeTriVelocityBlock(iBlock, points, pBtOpp->gamma().data(), pBtOpp->sigma().data(), velvectors.data());
        }
        update();
    }
}


void gl3dXSailView::makeTriVelocityBlock(int iBlock, QVector<Vector3d> const &C, double const *mu, double const *sigma, Vector3d *VField) const
{
    int blockSize = int(C.size()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxRows = C.size();
    int iMax = std::min(iStart+blockSize, maxRows);

    Vector3d V;

    for (int ip=iStart; ip<iMax; ip++)
    {
        if (s_pXSail->curBtPolar()->isTriUniformMethod())
            m_pP3UniAnalysis->getVelocityVector(C.at(ip), mu, sigma, V, Vortex::coreRadius(), false, false);
        else if(s_pXSail->curBtPolar()->isTriLinearMethod())
            m_pP3LinAnalysis->getVelocityVector(C.at(ip), mu, sigma, V, Vortex::coreRadius(), false, false);

        VField[ip] = V;
    }
}


bool gl3dXSailView::intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I)
{
    Boat const *pBoat = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;
    float zmax = +1.e10;
    Vector3d INear;
    bool bIntersect = false;

    if(pBtPolar && pBtPolar->isQuadMethod())
    {
        // not activated
    }
    else
    {
        for(int i3=0; i3<pBoat->nPanel3(); i3++)
        {
            Panel3 const &p3 = pBoat->panel3At(i3);
            if(p3.intersect(AA, U, I))
            {
                worldToScreen(p3.CoG(), v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
                    INear = I;
                    bIntersect = true;
                }
            }
        }
    }

    if(m_pDisplayCtrls->s_bWakePanels && pBtPolar)
    {
        if(pBtPolar->isTriangleMethod())
        {
            for(uint i3=0; i3<pBoat->triMesh().wakePanels().size(); i3++)
            {
                Panel3 const &p3 = pBoat->triMesh().wakePanelAt(i3);
                if(p3.intersect(AA, U, I))
                {
                    worldToScreen(p3.CoG(), v4d);
                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        INear = I;
                        bIntersect = true;
                    }
                }
            }
        }
        else if(pBtPolar->isQuadMethod())
        {
            // not activated
        }
    }
    if(m_pDisplayCtrls->s_bWater)
    {
        double x = W3dPrefs::s_BoxX;
        double y = W3dPrefs::s_BoxX;
        Triangle3d t[2];
        t[0].setTriangle({x,y,0}, {-x,-y,0}, {x,-y,0});
        t[1].setTriangle({x,y,0}, {-x, y,0}, {-x,-y,0});
        for(int i=0; i<2; i++)
        {
            if(t[i].intersectSegmentInside(AA, BB, I, true))
            {
                worldToScreen(I, v4d);
                if(v4d.z()<zmax)
                {
                    zmax = v4d.z();
                    INear = I;
                    bIntersect = true;
                }
            }
        }
    }

    I = INear;
    return bIntersect;
}


void gl3dXSailView::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_B:
        {
            onWindBack();
            pEvent->accept();
            return;
        }
        case Qt::Key_F:
        {
            onWindFront();
            pEvent->accept();
            return;
        }

        default:
            break;
    }
    gl3dXflView::keyPressEvent(pEvent);
}


void gl3dXSailView::onCancelThreads()
{
    StreamlineMaker::cancelTasks(true);
}


void gl3dXSailView::onPartSelClicked()
{
    Boat *pBoat= s_pXSail->curBoat();
    if(!pBoat) return;

    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if(!pSenderBox) return;

    QVariant property = pSenderBox->property("partindex");
    if(property.isNull()) return;

    bool bOk = false;
    int index = property.toInt(&bOk);
    if(!bOk) return;

    if(index<pBoat->nSails())
    {
        Sail * pSail = pBoat->sail(index);
        if(pSail) pSail->setVisible(pSenderBox->isChecked());
    }
    else if(index<pBoat->nSails()+pBoat->nHulls())
    {
        Fuse *pFuse = pBoat->hull(index-pBoat->nSails());
        if(pFuse) pFuse->setVisible(pSenderBox->isChecked());
    }

    // the geometry does not need to be rebuilt
    s_bResetglMesh = true;
    s_bResetglPanelCp = true;
    s_bResetglPanelForce = true;
    s_bResetglStream = true;
    update();
}


void gl3dXSailView::makeVisiblePanel3List(Boat const *pBoat, BoatPolar const*pBtPolar, std::vector<Panel3> &panel3list)
{
    for(int i3=0; i3<pBoat->nPanel3(); i3++)
    {
        for(int isail=0; isail<pBoat->nSails(); isail++)
        {
            Sail const*pSail = pBoat->sailAt(isail);
            if(pSail && pSail->isVisible())
            {
                if(i3>=pSail->firstPanel3Index() && i3<pSail->firstPanel3Index()+pSail->nPanel3())
                {
                    panel3list.push_back(pBoat->triMesh().panelAt(i3));
                }
            }
        }

        if(!pBtPolar || !pBtPolar->bIgnoreBodyPanels())
        {
            for(int ihull=0; ihull<pBoat->nHulls(); ihull++)
            {
                Fuse const*pHull = pBoat->hullAt(ihull);
                if(pHull && pHull->isVisible())
                {
                    if(i3>=pHull->firstPanel3Index() && i3<pHull->firstPanel3Index()+pHull->nPanel3())
                    {
                        panel3list.push_back(pBoat->triMesh().panelAt(i3));
                    }
                }
            }
        }
    }
}


void gl3dXSailView::glMakeWindSpline()
{
    BoatPolar const* pBtPolar = s_pXSail->curBtPolar();
    m_TrueWindSpline.clear();
    if(!pBtPolar)
    {
        if(m_vboTTWSpline.isCreated()) m_vboTTWSpline.destroy();
        return;
    }
    double ctrl = 0.0;
    if(s_pXSail->curBtOpp()) ctrl = s_pXSail->curBtOpp()->ctrl();

    double tws = pBtPolar->TWSInf(ctrl);
    double twa = pBtPolar->TWAInf(ctrl);
    Vector3d TWS(tws*cos(twa*PI/180.0), tws*sin(twa*PI/180.0), 0.0);

    Vector3d VT;

    for(int is=0; is<pBtPolar->windSpline().outputSize();is++)
    {
        double h = pBtPolar->windSpline().outputPt(is).y;
        double force = pBtPolar->windForce(h);
        VT.set(-TWS.x*force, -TWS.y*force, h);
        m_TrueWindSpline.append(VT);
    }
    gl::makeLineStrip(m_TrueWindSpline, m_vboTTWSpline);
}


void gl3dXSailView::resizeGL(int w, int h)
{
    gl3dXflView::resizeGL(w, h);

//    m_LegendOverlay.setLegendHeight((height()*2)/3);
    m_LegendOverlay.resize(100, (height()*2)/3, devicePixelRatioF());
    m_LegendOverlay.makeLegend();
}


void gl3dXSailView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);

    glDisable(GL_CULL_FACE);

    if(m_LegendOverlay.isVisible())
    {
        int wc = m_LegendOverlay.m_pix.width();
        QPoint pos3(width()*devicePixelRatio()-5-wc, 50);
        painter.drawPixmap(pos3, m_LegendOverlay.m_pix);
    }

    gl3dView::paintOverlay();
}


void gl3dXSailView::glMakeFlowBuffers()
{
    Boat      const *pBoat  = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp   const *pBtOpp   = s_pXSail->curBtOpp();
    if(!pBoat || !pBtPolar || !pBtOpp) return;
    if(!m_pDisplayCtrls->s_bFlow || !pBoat || !pBtPolar || !pBtOpp || !pBtOpp->isTriUniformMethod())
    {
        if(m_ssboPanels.isCreated())  m_ssboPanels.destroy();
        if(m_ssboVortons.isCreated()) m_ssboVortons.destroy();
        if(m_ssboBoids.isCreated())   m_ssboBoids.destroy();
        if(m_vboTraces.isCreated())   m_vboTraces.destroy();
        return;
    }
    if(m_bResetFlowPanels)
    {

        m_pP3UniAnalysis->initializeAnalysis(pBtPolar,0);
        m_pP3UniAnalysis->setTriMesh(pBoat->triMesh());
        m_pP3UniAnalysis->setVortons(pBtOpp->m_Vorton);

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
            BufferArray[iv++] = pBtOpp->sigma(i);
            BufferArray[iv++] = pBtOpp->gamma(3*i);

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

                Panel3 const *p3w = &m_pP3UniAnalysis->wakePanelAt(p3.iWake());
                while(p3w)
                {
                    gammw[p3w->index()] += pBtOpp->gamma(3*p3.index())*sign;
                    // is there another wake panel downstream?
                    if(p3w->m_iPD>=0) p3w = &m_pP3UniAnalysis->wakePanelAt(p3w->m_iPD);
                    else
                    {
                        p3w = nullptr;
                        break;
                    }
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
        }
        m_ssboPanels.release();

        int NVortons = pBtOpp->vortonCount();
        // create the minimal SSBO
        //need to use 4 components for positions due to std140/430 padding constraints for vec3
        buffersize = (NVortons)*(4+4); // 4 components for position and 4 for vorticity;

        BufferArray.resize(buffersize);
        iv=0;
        for(uint ir=0; ir<pBtOpp->m_Vorton.size(); ir++)
        {
            for(uint jc=0; jc<pBtOpp->m_Vorton.at(ir).size(); jc++)
            {
                Vorton const &vtn = pBtOpp->m_Vorton.at(ir).at(jc);

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
        }
        m_ssboVortons.release();


        m_bResetFlowPanels = false;
    }

    if(m_bResetBoids)
    {
        makeBoids();
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
                    BufferArray[iv++] = clr.redF()*float(j+1)/float(TRACESEGS);
                    BufferArray[iv++] = clr.greenF()*float(j+1)/float(TRACESEGS);
                    BufferArray[iv++] = clr.blueF()*float(j+1)/float(TRACESEGS);
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
        }
        m_vboTraces.release();

        m_bResetBoids = false;
    }
}


void gl3dXSailView::makeBoids()
{
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    if(!pBtPolar)
    {
        m_Boid.clear();
        return;
    }

     int nBoids = FlowCtrls::s_FlowNGroups * GROUP_SIZE;
     m_Boid.resize(nBoids);
     for(int inboid=0; inboid<nBoids; inboid++)
     {
         Boid & boid = m_Boid[inboid];
         boid.m_Position.x = FlowCtrls::flowTopLeft().x + QRandomGenerator::global()->generateDouble() * (FlowCtrls::flowBotRight().x-FlowCtrls::flowTopLeft().x);
         boid.m_Position.y = FlowCtrls::flowTopLeft().y + QRandomGenerator::global()->generateDouble() * (FlowCtrls::flowBotRight().y-FlowCtrls::flowTopLeft().y);
         boid.m_Position.z = FlowCtrls::flowTopLeft().z + QRandomGenerator::global()->generateDouble() * (FlowCtrls::flowBotRight().z-FlowCtrls::flowTopLeft().z);

         boid.Index = inboid;
         boid.m_Velocity.set(0.1,0,0);
     }
}


void gl3dXSailView::moveBoids()
{
#ifndef Q_OS_MAC

    if(oglMajor()*10+oglMinor()<43) return;

    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp const *pBtOpp     = s_pXSail->curBtOpp();
    if(!pBtOpp || !pBtOpp->isTriUniformMethod()) return;

    int NPanels = m_pP3UniAnalysis->nPanels();
    int NWakePanels = m_pP3UniAnalysis->nWakePanels();
    float Qinf = float(pBtOpp->QInf());

    m_shadFlow.bind();
    {
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_NPanels,      NPanels+NWakePanels);
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_VInf,         QVector4D(Qinf,0,0,0));
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_NVorton,      pBtOpp->vortonCount());
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_VtnCoreSize,  float(pBtPolar->vortonCoreSize()*pBtPolar->referenceChordLength()));
        if     (pBtPolar->bGroundEffect())      m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasGround, 1);
        else if(pBtPolar->bFreeSurfaceEffect()) m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasGround, -1);
        else                                    m_shadFlow.setUniformValue(m_shadFlowLoc.m_HasGround, 0);
        m_shadFlow.setUniformValue(m_shadFlowLoc.m_GroundHeight, float(pBtPolar->groundHeight()));

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

        glFlush();
    }
    m_shadFlow.release();

#endif
}


void gl3dXSailView::startFlow(bool bStart)
{
    if(bStart) m_FlowTimer.start(FLOWPERIOD);
    else       m_FlowTimer.stop();
    update();
}


void gl3dXSailView::restartFlow()
{
    startFlow(false);

    m_bResetBoids = true; //update the SSBO and VBO

    update();
    startFlow(true);
}


void gl3dXSailView::glRenderFlow()
{
    Boat      const *pBoat  = s_pXSail->curBoat();
    BoatPolar const *pBtPolar = s_pXSail->curBtPolar();
    BoatOpp   const *pBtOpp   = s_pXSail->curBtOpp();
    if(!pBoat || !pBtPolar || !pBtOpp) return;
    if(!m_pDisplayCtrls->s_bFlow) return;

    moveBoids();

    paintColourSegments8(m_vboTraces, float(W3dPrefs::s_FlowStyle.m_Width), W3dPrefs::s_FlowStyle.m_Stipple);

    getGLError();

//    m_pgl3dScaleCtrls->setFPS();
}


void gl3dXSailView::cancelFlow()
{
    m_pDisplayCtrls->s_bFlow = false;
    m_FlowTimer.stop();
}


double gl3dXSailView::objectReferenceLength() const
{
    if(s_pXSail->curBoat()) return s_pXSail->curBoat()->height()*1.5;
    return 1.0;
}
