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


#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QTime>
#include <QLabel>

#include "gl3dfuseview.h"


#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/frame.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/units.h>


gl3dFuseView::gl3dFuseView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pFuse = nullptr;

    m_bResetglFrameHighlight = true;
    m_bResetglFuse = true;
    m_bResetglPanels = true;
    m_bResetglNormals = true;

    m_nHighlightLines = m_HighlightLineSize = 0;
}


gl3dFuseView::~gl3dFuseView()
{
    if(m_vboTriangleNormals.isCreated())  m_vboTriangleNormals.destroy();
    if(m_vboHighlight.isCreated())        m_vboHighlight.destroy();
    if(m_vboTriPanels.isCreated())        m_vboTriPanels.destroy();
    if(m_vboTriPanelEdges.isCreated())    m_vboTriPanelEdges.destroy();
    if(m_vboFrames.isCreated())           m_vboFrames.destroy();
}


void gl3dFuseView::setFuse(Fuse const* pFuse)
{
    m_pFuse = pFuse;
    if(m_pFuse)
    {
        double length = m_pFuse->length();
        length = std::max(m_pFuse->maxWidth(), length);
        length = std::max(m_pFuse->maxHeight(), length);
        setReferenceLength(length);
        reset3dScale();
    }
}


void gl3dFuseView::glRenderView()
{
    if(!m_pFuse) return;
//    auto t0 = std::chrono::high_resolution_clock::now();
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

    if(m_bNormals)
    {
        paintNormals(m_vboTriangleNormals);
    }

    if(m_pFuse->isXflType())
    {
        FuseXfl const *pFuseXfl = dynamic_cast<FuseXfl const*>(m_pFuse);

        if(m_bSurfaces)     paintTriangles3Vtx(m_vboSurface, m_pFuse->color(), false, isLightOn());
        if(m_bTessellation) paintSegments(m_vboTess, W3dPrefs::s_OutlineStyle);

        if(m_bOutline)
        {
            paintSegments(m_vboOutline, W3dPrefs::s_OutlineStyle);
            if(m_pFuse->isSplineType() || m_pFuse->isSectionType())
                paintSegments(m_vboFrames, W3dPrefs::s_OutlineStyle);
        }
        if(!pFuseXfl->isSectionType())
        {
            if(pFuseXfl->activeFrameIndex()>=0)
            {
                paintLineStrip(m_vboHighlight, W3dPrefs::s_SelectStyle);
            }
        }
        else
        {
            FuseSections const *pFuseSecs =  dynamic_cast<FuseSections const*>(m_pFuse);
            if(pFuseSecs->activeSectionIndex()>=0)
                paintLineStrip(m_vboHighlight, W3dPrefs::s_HighStyle);
        }

        if(m_bCtrlPoints)
        {
            QColor clr = xfl::fromfl5Clr(pFuseXfl->color());

            if(!pFuseXfl->isSectionType())
            {
                for(int ifr=0; ifr<pFuseXfl->frameCount(); ifr++)
                {
                    for(int ic=0; ic<pFuseXfl->framePointCount(); ic++)
                    {
                        if(pFuseXfl->activeFrameIndex()==ifr)
                        {
                            if(ic==pFuseXfl->activeFrame().selectedIndex())  clr = W3dPrefs::selectColor();
                            else                                             clr = W3dPrefs::highlightColor();
                        }
                        else
                            clr = xfl::fromfl5Clr(pFuseXfl->color()).darker();
                        Vector3d const &pt = pFuseXfl->nurbs().ctrlPoint(ifr, ic);
                        paintCube(pt.x, pt.y, pt.z, 0.020/m_glScalef, clr, true);
                    }
                }
            }
            else
            {
                FuseSections const *pFuseSecs =  dynamic_cast<FuseSections const*>(m_pFuse);
                for(int isec=0; isec<pFuseSecs->nSections(); isec++)
                {
                    std::vector<Vector3d> const &section = pFuseSecs->sectionAt(isec);
                    for(int ic=0; ic<int(section.size()); ic++)
                    {
                        if(pFuseSecs->activeSectionIndex()==isec)
                        {
                            if(ic==pFuseSecs->activePointIndex()) clr = W3dPrefs::selectColor();
                            else                                  clr = W3dPrefs::highlightColor();
                        }
                        else
                            clr = xfl::fromfl5Clr(pFuseXfl->color()).darker();
                        Vector3d const &pt = section.at(ic);
                        paintCube(pt.x, pt.y, pt.z, 0.020/m_glScalef, clr, true);
                    }
                }
            }
        }
    }
    else if(m_pFuse->isOccType())
    {
        if(m_bSurfaces)
            paintTriangles3Vtx(m_vboSurface, m_pFuse->color(), false, isLightOn());
        if(m_bTessellation)
            paintSegments(m_vboTess, W3dPrefs::s_OutlineStyle);

        if(m_bOutline)
            paintSegments(m_vboOutline, W3dPrefs::s_OutlineStyle);
    }
    else if(m_pFuse->isStlType())
    {
        if(m_bSurfaces) paintTriangles3Vtx(m_vboSurface, m_pFuse->color(), false, isLightOn());
        if(m_bOutline)  paintSegments(m_vboOutline, W3dPrefs::s_OutlineStyle);
    }

    if(m_bMeshPanels)
    {
        paintEditTriMesh(!m_bSurfaces);
        if(m_PanelHightlight.size())
            paintSegments(m_vboHighlightPanel3, W3dPrefs::s_HighStyle);
        if(m_Segments.size())
            paintSegments(m_vboSegments, W3dPrefs::highlightColor(), 1, Line::SOLID, true);
    }

    if(m_bShowMasses)
    {
        paintPartMasses(Vector3d(), 0.0, "Structural mass", m_pFuse->pointMasses(), m_iSelectedPartMass);
        //plot CG
        Vector3d Place(m_pFuse->CoG_t().x, m_pFuse->CoG_t().y, m_pFuse->CoG_t().z);
        paintSphere(Place, W3dPrefs::s_MassRadius*2.0/double(m_glScalef),
                    W3dPrefs::s_MassColor.darker());

        double delta = 0.02/double(m_glScalef);
        glRenderText(m_pFuse->CoG_t().x, m_pFuse->CoG_t().y, m_pFuse->CoG_t().z + delta,
                     "CoG "+QString("%1").arg(m_pFuse->totalMass()*Units::kgtoUnit(), 0,'f',2)
                     +Units::massUnitQLabel(), W3dPrefs::s_MassColor.darker(125), false, true);
    }

    if(m_PickType==xfl::PANEL3)
    {
        if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<m_pFuse->nPanel3() && m_bMeshPanels && m_bHighlightPanel)
            highlightPickedPanel3(m_pFuse->panel3At(m_PickedPanelIndex));
    }

    if(m_PickType==xfl::MESHNODE && m_bMeshPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<m_pFuse->triMesh().nNodes())
        {
            Node const &nd = m_pFuse->triMesh().nodeAt(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<m_pFuse->triMesh().nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = m_pFuse->triMesh().nodeAt(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }
        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pFuse->triMesh().nNodes())
        {
            Node const &nd = m_pFuse->triMesh().nodeAt(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }

    paintMeasure();

//    auto t1 = std::chrono::high_resolution_clock::now();
//    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
//    qDebug("gl3dFuseView::glRenderView: %7d µs  time=%s", duration, QTime::currentTime().toString("hh:mm:ss.zzz").toStdString().c_str());
//    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz").toStdString().c_str();
}


void gl3dFuseView::glMake3dObjects()
{
    if(!m_pFuse) return;

    if(m_bResetglFrameHighlight || m_bResetglFuse)
    {
        m_bResetglFrameHighlight = false;
        if(m_pFuse->isXflType())
        {
            FuseXfl const *pFuseXfl = dynamic_cast<FuseXfl const*>(m_pFuse);
            if(!pFuseXfl->isSectionType())
            {
                if(pFuseXfl->activeFrameIndex()>=0)
                {
                    glMakeBodyFrameHighlight(pFuseXfl, Vector3d(), W3dPrefs::bodyHoopRes(), pFuseXfl->activeFrameIndex());
                }
            }
            else
            {
                FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(pFuseXfl);
                if(pFuseSections && pFuseSections->activeSectionIndex()>=0)
                {
                    glMakeBodyFrameHighlight(pFuseXfl, Vector3d(),  W3dPrefs::bodyHoopRes(), pFuseSections->activeSectionIndex());
                }
            }
        }
    }

    if(m_bResetglFuse)
    {
        if(m_pFuse->isXflType())
        {
            FuseXfl const *pXflFuse = dynamic_cast<FuseXfl const*>(m_pFuse);
            gl::makeTriangulation3Vtx(m_pFuse->triangulation(), Vector3d(), m_vboSurface,
                                    pXflFuse->isFlatFaceType());
            gl::makeTrianglesOutline(m_pFuse->triangles(), Vector3d(), m_vboTess);

            int nPts = pXflFuse->isSplineType() ? 30 : 1;
            gl::glMakeShellOutline(m_pFuse->shells(), Vector3d(), m_vboOutline, nPts);

            if(pXflFuse->isSplineType())
            {
                gl::makeFuseXflFrames(pXflFuse, Vector3d(), W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes(), m_vboFrames);
            }
            else if(pXflFuse->isSectionType())
            {
                FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(pXflFuse);
                gl::makeFuseXflSections(pFuseSections, Vector3d(), W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes(), m_vboFrames);
            }
        }
        else if(m_pFuse->isXflType() || m_pFuse->isOccType())
        {
            gl::makeTriangulation3Vtx(m_pFuse->triangulation(), Vector3d(), m_vboSurface, false);
            gl::makeTrianglesOutline(m_pFuse->triangles(), Vector3d(), m_vboTess);
            gl::glMakeShellOutline(m_pFuse->shells(), Vector3d(), m_vboOutline);
        }
        else if(m_pFuse->isStlType())
        {
            gl::makeTriangulation3Vtx(m_pFuse->triangulation(), Vector3d(), m_vboSurface, true);
            gl::makeTrianglesOutline(m_pFuse->triangulation().triangles(), Vector3d(), m_vboOutline);
        }
    }

    if(m_bResetglFuse || m_bResetglPanels || m_bResetglSegments)
    {
        gl::makeSegments(m_Segments, Vector3d(), m_vboSegments);
        m_bResetglSegments = false;
    }

    if(m_bResetglFuse || m_bResetglPanels)
    {
        gl::makeFuseTriMesh(m_pFuse, Vector3d(), m_vboTriPanels, m_vboTriPanelEdges);

        std::vector<Panel3> p3High;
        for(int ip=0; ip<m_PanelHightlight.size(); ip++)
        {
            int idx = m_PanelHightlight.at(ip);
            if(idx>=0 && idx<m_pFuse->nPanel3())
                p3High.push_back(m_pFuse->triMesh().panelAt(idx));
        }
        gl::makeTriEdges(p3High, Vector3d(), m_vboHighlightPanel3);


        if(m_bResetglFuse || m_bResetglNormals)
        {
            gl::makePanelNormals(m_pFuse->triMesh().panels(), 0.1f, m_vboTriangleNormals);
        }
    }

    m_bResetglFuse = m_bResetglPanels = false;
}


void gl3dFuseView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

    if(draggeddistance<5 && (pEvent->button()==Qt::LeftButton) && bPickNode() && m_PickedNodeIndex>=0)
    {
        if(m_PickedNodeIndex<0)
        {
            // clear and start again node pair picking
            m_NodePair = {-1,-1};
            return;
        }

        if(m_NodePair.first<0) m_NodePair.first = m_PickedNodeIndex;
        else if(m_NodePair.second<0)
        {
            m_NodePair.second = m_PickedNodeIndex;
            //two valid node indexes, merge them
            emit (pickedNodePair(m_NodePair));
        }
        else
        {
            //start again
            m_NodePair = {m_PickedNodeIndex, -1};
        }
    }
    else if(m_bTrans || m_bArcball)
    {
        gl3dXflView::mouseReleaseEvent(pEvent);

    }
    update();
}


void gl3dFuseView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    if(!isPicking() || !m_bMeshPanels)
    {
        m_PickedPanelIndex = m_PickedNodeIndex = -1;
        update();
        return;
    }

    if(bPickFace())
    {
        QPair<int, int> picked = pickFace(pEvent->pos(), m_pFuse->shells());
        emit pickedFace(picked.first, picked.second);
        return;
    }

    Vector3d I;
    if(bPickNode() || (m_bHighlightPanel || bPickPanel3()))
        pickPanel3(pEvent->pos(), m_pFuse->triMesh().panels(), I);

    if(isPicking())
    {
        if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pFuse->triMesh().nPanels())
        {
            m_PickedNodeIndex = -1;
            clearTopRightOutput(); // do not put in mouvemove: slow operation
            update();
            return;
        }
    }

    if(m_PickedPanelIndex<0) return;

    Panel3 const &p3 = m_pFuse->panel3At(m_PickedPanelIndex);

    if(bPickNode())
    {
        pickPanelNode(p3, I, xfl::FUSESURFACE);
        if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=m_pFuse->triMesh().nNodes())
        {
            m_PickedNodeIndex = -1;
            clearTopRightOutput();
        }
        else
        {
            if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pFuse->triMesh().nNodes())
            {
                Node const &nd = m_pFuse->triMesh().nodeAt(m_PickedNodeIndex);
                setTopRightOutput(QString::fromStdString(nd.properties()));
             }
        }
    }
    else
    {
        if(!m_bHighlightPanel)
        {
            m_PickedPanelIndex = -1;
            clearTopRightOutput();
            update();
            return;
        }

        if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pFuse->nPanel3())
        {
            m_PickedPanelIndex = -1;
            clearTopRightOutput();
        }
        else
        {
            Panel3 const &p3 = m_pFuse->panel3At(m_PickedPanelIndex);
            gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);
            bool bLong = false;
#ifdef QT_DEBUG
            bLong = true;
#endif
            setTopRightOutput(p3.properties(bLong));
        }
    }

    update();
}


bool gl3dFuseView::intersectTheObject(const Vector3d &AA, const Vector3d &BB, Vector3d &I)
{
    return m_pFuse->intersectFuseTriangulation(AA, BB, I);
}


void gl3dFuseView::paintEditTriMesh(bool bBackground)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor,  xfl::fromfl5Clr(W3dPrefs::s_PanelStyle.m_Color));
        m_shadLine.setUniformValue(m_locLine.m_Pattern,   gl::stipple(W3dPrefs::s_PanelStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_PanelStyle.m_Width));

        m_vboTriPanelEdges.bind();
        {
            m_shadLine.enableAttributeArray(m_locSurf.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
            int nLines = m_vboTriPanelEdges.size()/2/3/int(sizeof(float));
            glDrawArrays(GL_LINES, 0, nLines*2);
        }
        m_vboTriPanelEdges.release();
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, xfl::fromfl5Clr(W3dPrefs::s_PanelStyle.m_Color));

        m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);


        if(bBackground)
        {
            m_vboTriPanels.bind();
            {
                m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

                glEnable(GL_CULL_FACE);
                if(W3dPrefs::s_bUseBackClr)
                    m_shadSurf.setUniformValue(m_locSurf.m_UniColor, DisplayOptions::backgroundColor());
                else
                    m_shadSurf.setUniformValue(m_locSurf.m_UniColor, W3dPrefs::s_FusePanelColor);

                int nTriangles = m_vboTriPanels.size()/3/3/int(sizeof(float)); // four vertices and three components

                glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);

                glDisable(GL_POLYGON_OFFSET_FILL);
                glDisable(GL_CULL_FACE);
            }
            m_vboTriPanels.release();
        }

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
    }
    m_shadSurf.release();

    glDisable(GL_POLYGON_OFFSET_FILL);
}


void gl3dFuseView::glMakeBodyFrameHighlight(FuseXfl const *pFuseXfl, const Vector3d &bodyPos, int NHOOOP, int iFrame)
{
    Vector3d Point;
    double hinc=0, u=0, v=0;
    if(iFrame<0 || iFrame>=pFuseXfl->frameCount()) return;

    Frame const &pFrame = pFuseXfl->frameAt(iFrame);
    //    xinc = 0.1;
    hinc = 1.0/double(NHOOOP-1);

    int bufferSize = 0;
    QVector<float> pHighlightVertexArray;

    m_nHighlightLines = 2; // left and right - could make one instead

    //create 3d Splines or Lines to overlay on the body
    int iv = 0;

    if(pFuseXfl->isFlatFaceType())
    {
        m_HighlightLineSize = pFrame.nCtrlPoints();
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3;
        pHighlightVertexArray.resize(bufferSize);
        for (int k=0; k<pFrame.nCtrlPoints();k++)
        {
            pHighlightVertexArray[iv++] = pFrame.position().xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = pFrame.ctrlPointAt(k).yf();
            pHighlightVertexArray[iv++] = pFrame.ctrlPointAt(k).zf()+bodyPos.zf();
        }

        for (int k=0; k<pFrame.nCtrlPoints();k++)
        {
            pHighlightVertexArray[iv++] =  pFrame.position().xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = -pFrame.ctrlPointAt(k).yf();
            pHighlightVertexArray[iv++] =  pFrame.ctrlPointAt(k).zf()+bodyPos.zf();
        }
    }
    else if(pFuseXfl->isSplineType())
    {
        m_HighlightLineSize = int(NHOOOP);
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3;
        pHighlightVertexArray.resize(bufferSize);

        if(pFuseXfl->activeFrameIndex()>=0)
        {
            u = pFuseXfl->getu(pFrame.position().x);
            v = 0.0;
            for (int k=0; k<NHOOOP; k++)
            {
                pFuseXfl->getPoint(u,v,true, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v += hinc;
            }

            v = 1.0;
            for (int k=0; k<NHOOOP; k++)
            {
                pFuseXfl->getPoint(u,v,false, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v -= hinc;
            }
        }
    }
    else if(pFuseXfl->isSectionType())
    {
        FuseSections const *pFuseSections = dynamic_cast<FuseSections const *>(pFuseXfl);
        if(pFuseSections)
        {
            m_HighlightLineSize = int(NHOOOP);
            bufferSize = m_nHighlightLines * m_HighlightLineSize *3;
            pHighlightVertexArray.resize(bufferSize);

            if(pFuseSections->activeSectionIndex()>=0)
            {
                std::vector<Vector3d> const& section = pFuseSections->activeSection();
                u = pFuseSections->getu(section.front().x);
                v = 0.0;
                for (int k=0; k<NHOOOP; k++)
                {
                    pFuseSections->getPoint(u,v,true, Point);
                    pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                    pHighlightVertexArray[iv++] = Point.yf();
                    pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                    v += hinc;
                }

                v = 1.0;
                for (int k=0; k<NHOOOP; k++)
                {
                    pFuseSections->getPoint(u,v,false, Point);
                    pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                    pHighlightVertexArray[iv++] = Point.yf();
                    pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                    v -= hinc;
                }
            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    m_vboHighlight.destroy();
    m_vboHighlight.create();
    m_vboHighlight.bind();
    m_vboHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboHighlight.release();
}

