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
#include <QKeyEvent>


#include <BRep_Tool.hxx>
#include <Geom_Line.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <StdFail_NotDone.hxx>

#include <GC_MakeSegment.hxx>

#include "gl3dsailview.h"


#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/frame.h>
#include <api/nurbssurface.h>
#include <api/triangle3d.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <api/sailnurbs.h>
#include <api/sailocc.h>
#include <api/sail.h>
#include <api/sailspline.h>
#include <api/sailstl.h>
#include <api/sailwing.h>
#include <api/trimesh.h>
#include <api/units.h>

gl3dSailView::gl3dSailView(QWidget *pSailDlg) : gl3dXflView(pSailDlg)
{
    m_pSail = nullptr;

    m_bNormals      = false;

    m_bResetglSectionHighlight = true;
    m_bResetglSail             = true;
    m_bResetglMesh             = true;
}


gl3dSailView::~gl3dSailView()
{
    if(m_bAutoDeleteBuffers)
    {
        m_vboTriangulation.bind();
        m_vboTriangulation.destroy();

        m_vboSailOutline.bind();
        m_vboSailOutline.destroy();

        m_vboTriangleNormals.bind();
        m_vboTriangleNormals.destroy();

        m_vboSectionHighlight.bind();
        m_vboSectionHighlight.destroy();

        if(m_vboSailSurfNormals.isCreated()) m_vboSailSurfNormals.destroy();
        if(m_vboHighEdge.isCreated()) m_vboHighEdge.destroy();
    }
}


bool gl3dSailView::intersectTheObject(const Vector3d &AA, const Vector3d &BB, Vector3d &I)
{
    Vector3d N;
    if(m_pSail->intersect(AA, BB, I, N))
    {
/*#ifdef QT_DEBUG
        m_DebugPts.clear();
        m_DebugPts.append(I);
#endif*/
        return true;
    }
    return false;
}


void gl3dSailView::setSail(Sail const* pSail)
{
    m_pSail = pSail;
    if(pSail->isExternalSail())
    {
        ExternalSail const *pExtSail = dynamic_cast<ExternalSail const*>(m_pSail);
        setReferenceLength(pExtSail->size()*2.5);
    }
    else
        setReferenceLength(m_pSail->luffLength()*2);

    std::string strange, frontspacer;
    m_pSail->properties(strange, frontspacer);
    setBotLeftOutput(strange);

    reset3dScale();
}


void gl3dSailView::glMake3dObjects()
{
    if(!m_pSail)
    {
        setBotLeftOutput(QString());
        return;
    }
    if(m_bResetglSail)
    {
        std::string strange, frontspacer;
        m_pSail->properties(strange, frontspacer);
        setBotLeftOutput(strange);


        gl::makeTriangulation3Vtx(m_pSail->triangulation(), Vector3d(), m_vboTriangulation, false);
        gl::makeTrianglesOutline(m_pSail->triangles(), Vector3d(), m_vboTess);

//        glMakeTriangleNormals(m_pSail->triangulation().triangles(), m_RefLength/10.0, m_vboTriangleNormals);
//        glMakeNodeNormals(m_pSail->triangulation().nodes(), Vector3d(), float(m_RefLength)/10.0f, m_vboTriangleNormals);
    }


    if(m_pSail->isSplineSail())
    {
        SailSpline const *pSS = dynamic_cast<SailSpline const *>(m_pSail);
        if(m_bResetglSail)
        {
            gl::makeSplineSailOutline(pSS, Vector3d(), m_vboSailOutline);
        }
        if(m_bResetglSail || m_bResetglSectionHighlight)
            gl::makeSectionHighlight(pSS, Vector3d(), m_vboSectionHighlight);
    }
    else if(m_pSail->isNURBSSail())
    {
        SailNurbs const *pNS = dynamic_cast<SailNurbs const *>(m_pSail);
        if(m_bResetglSail)
        {
            gl::makeNurbsOutline(pNS->nurbs(), Vector3d(), Sail::iXRes(), Sail::iZRes(), m_vboSailOutline);
        }
        if(m_bResetglSail || m_bResetglSectionHighlight)
            gl::makeFrameHighlight(pNS->nurbs(), Vector3d(), m_vboSectionHighlight);
    }
    else if(m_pSail->isWingSail())
    {
        SailWing const *pWS = dynamic_cast<SailWing const *>(m_pSail);
        if(m_bResetglSail)
        {
            gl::makeWingSailOutline(pWS, Vector3d(), m_vboSailOutline);
        }
        if(m_bResetglSail || m_bResetglSectionHighlight)
            gl::makeSectionHighlight(pWS, Vector3d(), m_vboSectionHighlight);
    }
    else if(m_pSail->isOccSail())
    {
        SailOcc const *pOccSail = dynamic_cast<SailOcc const *>(m_pSail);
        if(m_bResetglSail)
        {
            glMakeShellOutline(pOccSail->shapes(), Vector3d(), m_vboSailOutline);
        }
    }
    else if(m_pSail->isStlSail())
    {
        if(m_bResetglSail)
        {
            gl::makeTrianglesOutline(m_pSail->triangles(), Vector3d(), m_vboSailOutline);
        }
    }

    if(m_bResetglSegments)
    {
        gl::makeSegments(m_Segments, Vector3d(), m_vboSegments);
        m_bResetglSegments = false;
    }


    if(m_bResetglSail || m_bResetglMesh)
    {
        gl::makeTriPanels(m_pSail->triMesh().panels(), Vector3d(), m_vboSailMesh);
        gl::makeTriEdges(m_pSail->triMesh().panels(), Vector3d(), m_vboSailMeshEdges);
        gl::makePanelNormals(m_pSail->triMesh().panels(), m_RefLength/50.0, m_vboTriangleNormals);

        glMakeSailNormals(m_pSail,  m_RefLength/50.0, m_vboSailSurfNormals);
    }
    m_bResetglMesh = false;

    m_bResetglSail = m_bResetglSectionHighlight = false;
}


void gl3dSailView::glMakeSailNormals(Sail const*pSail, float length, QOpenGLBuffer &vbo)
{
    if(!pSail->isNURBSSail() && !pSail->isSplineSail())
    {
        vbo.destroy();
        return;
    }

    Vector3d Normal;
    double u[] = {0.0, 0.33333, 0.666667, 1.0};
    double v[] = {0.0, 0.33333, 0.666667, 1.0};
    int N = 4;

    // vertices array size:
    //        6*6 nodes
    //      x2 vertices /nodes
    //        x3 = 3 vertex components
    int buffersize = N*N*2*3;
    QVector<float>NormalVertexArray(buffersize);

    int iv = 0;
    int vec=0;
    for (int ku=0; ku<N; ku++)
    {
        for (int kv=0; kv<N; kv++)
        {
            Vector3d pt = pSail->point(u[ku], v[kv]);

            NormalVertexArray[iv++] = pt.xf();
            NormalVertexArray[iv++] = pt.yf();
            NormalVertexArray[iv++] = pt.zf();

            Normal = pSail->normal(u[ku], v[kv]);

            NormalVertexArray[iv++] = pt.xf() + Normal.xf() * length;
            NormalVertexArray[iv++] = pt.yf() + Normal.yf() * length;
            NormalVertexArray[iv++] = pt.zf() + Normal.zf() * length;
            vec++;
        }
    }
    (void)vec;

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(NormalVertexArray.data(), NormalVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dSailView::glRenderView()
{
    if(!m_pSail) return;
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

    if(m_bSurfaces)
    {
        paintTriangles3Vtx(m_vboTriangulation, m_pSail->color(), true, true);
        if(m_bTessellation) paintSegments(m_vboTess, W3dPrefs::s_OutlineStyle);
    }

    if(m_pSail->isSplineSail())
    {
        SailSpline const *pSS = dynamic_cast<SailSpline const*>(m_pSail);
        if(pSS->activeSpline())
        {
            paintLineStrip(m_vboSectionHighlight, W3dPrefs::s_SelectStyle);
        }
        if(m_bCtrlPoints) paintControlPoints();
    }
    else if(m_pSail->isNURBSSail())
    {
        SailNurbs const *pNS = dynamic_cast<SailNurbs const*>(m_pSail);
        if(pNS->nurbs().activeFrameIndex()>=0 && pNS->nurbs().activeFrameIndex()<pNS->nurbs().frameCount())
        {
            paintLineStrip(m_vboSectionHighlight, W3dPrefs::s_SelectStyle);
        }
        if(m_bCtrlPoints) paintControlPoints();
    }
    else if(m_pSail->isWingSail())
    {
        SailWing const *pWS = dynamic_cast<SailWing const*>(m_pSail);
        if(pWS->activeSection()>=0)
        {
            paintLineStrip(m_vboSectionHighlight, W3dPrefs::s_SelectStyle);
        }
    }

    if(m_bOutline) paintSegments(m_vboSailOutline, W3dPrefs::s_OutlineStyle);

    // restore line shader after controlpoints mod
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    if(m_bNormals)
    {
/*        if(m_pSail && (m_pSail->isNURBSSail() || m_pSail->isSplineSail()))
            paintNormals(m_vboSailSurfNormals);
        else */
            paintNormals(m_vboTriangleNormals);
    }

    if(m_bMeshPanels)
    {
//        if(m_pSail->isStlSail())
        {
            if(!m_bSurfaces)
            {
                paintTriPanels(m_vboSailMesh, true);
                paintSegments(m_vboSailMeshEdges, W3dPrefs::s_PanelStyle);
            }

            if((m_bHighlightPanel || m_PickType==xfl::PANEL3 || m_PickType==xfl::TRIANGLE3D) && m_PickedPanelIndex>=0 && m_PickedPanelIndex<m_pSail->nPanel3())
            {
                paintTriangle(m_vboTriangle, true, false, Qt::black);
                QString strong;
                strong = QString::asprintf("P%d", m_PickedPanelIndex);
                glRenderText(m_PickedPoint.x+0.03/double(m_glScalef), m_PickedPoint.y+0.03/double(m_glScalef), m_PickedPoint.z+0.03/double(m_glScalef),
                             strong,
                             DisplayOptions::textColor(), true);
#ifdef QT_DEBUG
                Panel3 const &t3d = m_pSail->triMesh().panelAt(m_PickedPanelIndex);
                for(int in=0; in<3; in++)
                {
                    Node const &nd = t3d.vertexAt(in);
                    glRenderText(nd.x+0.015/double(m_glScalef), nd.y+0.015/double(m_glScalef), nd.z+0.015/double(m_glScalef),
                                 QString::asprintf("nd%d", in),
                                 DisplayOptions::textColor(), true);
                }
#endif
            }
        }
    }

    if(m_Nodes.size() && m_bEdgeNodes)
    {
        for(int i=0; i<m_Nodes.size(); i++)
        {
            paintIcoSphere(m_Nodes.at(i), 0.011/double(m_glScalef), QColor(215,215,215), true, true);
        }
    }

    if(m_PickType==xfl::SEGMENT3D)
    {
        if(m_vboHighEdge.isCreated())
            paintSegments(m_vboHighEdge, W3dPrefs::s_HighStyle);
        if(m_vboPickedEdge.isCreated())
            paintSegments(m_vboPickedEdge, W3dPrefs::s_SelectStyle);
    }

    if(m_PickType==xfl::MESHNODE)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<m_pSail->triMesh().nNodes())
        {
            Node const &nd = m_pSail->triMesh().nodeAt(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<m_pSail->triMesh().nNodes())
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = m_pSail->triMesh().nodeAt(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }

        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pSail->triMesh().nNodes())
        {
            Node const &nd = m_pSail->triMesh().nodeAt(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }
    else if(m_PickType==xfl::TRIANGLENODE)
    {
        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<m_pSail->triangulation().nNodes())
        {
            Node const &nd = m_pSail->triangulation().nodeAt(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }
    else if(m_PickType==xfl::VERTEX)
    {
        if(m_bPickedVertex)
        {
            paintSphere(m_PickedPoint, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }

    if(m_bSailCornerPts)
    {
        paintSailCornerPoints(m_pSail, Vector3d());
    }

    if(m_Segments.size())
    {
        paintSegments(m_vboSegments, W3dPrefs::highlightColor(), 1, Line::SOLID, true);
#ifdef QT_DEBUG
       for(int i=0; i<m_Segments.size(); i++)
        {
            glRenderText(m_Segments.at(i).CoG(),
                         QString::asprintf("S%d", i),
                         DisplayOptions::textColor(), true);
            paintThinArrow(m_Segments.at(i).vertexAt(0), m_Segments.at(i).vertexAt(0).normal()/3.0, QColor(255,175,175),2,Line::SOLID);
        }
#endif
    }

    paintMeasure();

#ifdef QT_DEBUG
    paintDebugPts();
#endif
}


void gl3dSailView::paintControlPoints()
{
    float side = 0.025f/float(m_glScalef);
    if(m_pSail->isSplineSail())
    {
        SailSpline const *pSS = dynamic_cast<SailSpline const*>(m_pSail);
        for(int is=0; is<pSS->sectionCount(); is++)
        {
            Spline const *pSpline = pSS->splineAt(is);
            Vector3d pos = pSS->sectionPosition(is);
            double theta = pSS->sectionAngle(is);
            for(int ic=0; ic<pSpline->ctrlPointCount(); ic++)
            {
                Vector3d pt = {pos.x+pSpline->controlPoint(ic).x,
                               pos.y+pSpline->controlPoint(ic).y,
                               pos.z};
                pt.rotateY(pos, theta);
                paintCube(pt.x, pt.y, pt.z, side, xfl::fromfl5Clr(pSS->color()).darker(), true);
            }
        }
    }
    else if(m_pSail->isNURBSSail())
    {
        SailNurbs const *pNS = dynamic_cast<SailNurbs const*>(m_pSail);
        for(int is=0; is<pNS->frameCount(); is++)
        {
            Frame const &pSpline = pNS->frameAt(is);
            double theta = pNS->frameAt(is).angle();

            for(int ic=0; ic<pSpline.nCtrlPoints(); ic++)
            {
//                bool ISaidHigh =  (pNS->activeFrameIndex()==is) && (pSpline.selectedIndex()==ic);
                Vector3d pt = pSpline.ctrlPointAt(ic);
                pt.rotateY(pNS->frameAt(is).position(), theta);
                paintCube(pt.x, pt.y, pt.z, side, xfl::fromfl5Clr(pNS->color()).darker(), true);
            }
        }
    }
}


void gl3dSailView::setHighlightedEdge(TopoDS_Edge const &Edge)
{
    if(Edge.IsNull())  m_vboHighEdge.destroy();
    else
        glMakeEdge(Edge, m_vboHighEdge);
}


void gl3dSailView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    m_PickedPanelIndex = -1;
    m_PickedNodeIndex = -1;
    m_bPickedVertex = false;

    if(!m_bHighlightPanel && !isPicking())
    {
        clearTopRightOutput();
        return;
    }

    Vector3d I;

    switch (m_PickType)
    {
        case xfl::VERTEX:
        {
            if(m_pSail->isOccSail())
            {
                SailOcc const *pOccSail = dynamic_cast<SailOcc const *>(m_pSail);
                pickShapeVertex(pEvent->pos(), pOccSail->shapes(), I);
            }
            break;
        }
        case xfl::TRIANGLENODE:
        {
            pickTriangle3d(pEvent->pos(), m_pSail->triangles(), I);
            if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pSail->nTriangles())
            {
                clearTopRightOutput();
                update();
                return;
            }
            Triangle3d const &t3d = m_pSail->triangleAt(m_PickedPanelIndex);
            int lastpickedindex = m_PickedNodeIndex;
            pickTriangleNode(t3d, I);
            if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=m_pSail->triangulation().nNodes())
            {
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
                update();
                return;
            }
            if(m_PickedNodeIndex!=lastpickedindex)
            {
                Node const &nd = m_pSail->triangulation().nodeAt(m_PickedNodeIndex);
                setTopRightOutput(QString::fromStdString(nd.properties()));
            }
            break;
        }
        case xfl::MESHNODE:
        {
            pickPanel3(pEvent->pos(), m_pSail->triMesh().panels(), I);
            if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pSail->nPanel3())
            {
                clearTopRightOutput();
                update();
                return;
            }
            Panel3 const &t3d = m_pSail->triMesh().panelAt(m_PickedPanelIndex);
            int lastpickedindex = m_PickedNodeIndex;
            pickPanelNode(t3d, I, xfl::NOSURFACE);
            if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=m_pSail->triMesh().nNodes())
            {
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
                return;
            }
            if(m_PickedNodeIndex!=lastpickedindex)
            {
                Node const &nd = m_pSail->triMesh().nodeAt(m_PickedNodeIndex);
                setTopRightOutput(QString::fromStdString(nd.properties()));
            }
            break;
        }
        case xfl::TRIANGLE3D:
        {
            int lastpickedindex = m_PickedPanelIndex;
            pickTriangle3d(pEvent->pos(), m_pSail->triangles(), I);
            if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pSail->nPanel3())
            {
                m_vboTriangle.destroy();
                clearTopRightOutput();
                update();
                return;
            }
            if(m_PickedPanelIndex!=lastpickedindex)
            {
                Triangle3d const &t3d = m_pSail->triangleAt(m_PickedPanelIndex);
                gl::makeTriangle(t3d.vertexAt(0), t3d.vertexAt(1), t3d.vertexAt(2), m_vboTriangle);
                setTopRightOutput(QString::fromStdString(t3d.properties(false)));
            }
            break;
        }
        case xfl::PANEL3:
        {
            int lastpickedindex = m_PickedPanelIndex;
            pickPanel3(pEvent->pos(), m_pSail->triMesh().panels(), I);
            if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=m_pSail->nPanel3())
            {
                m_vboTriangle.destroy();
                clearTopRightOutput();
                update();
                return;
            }
            if(m_PickedPanelIndex!=lastpickedindex)
            {
                Panel3 const &p3 = m_pSail->triMesh().panelAt(m_PickedPanelIndex);
                gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);
                setTopRightOutput(p3.properties(false));
            }
            break;
        }
        case xfl::SEGMENT3D:
        {
            SailOcc const *pOccSail = dynamic_cast<SailOcc const*>(m_pSail);
            if(!pOccSail) break;
            TopoDS_ListOfShape const &shapes = pOccSail->shapes();
            if(!shapes.Extent()) break;

            int lastpickedface = m_HighFace;
            int lastpickededge = m_HighEdge;
            m_HighFace = m_HighEdge = -1;
            Vector3d I;
            Vector3d AA, BB;

            screenToWorld(pEvent->pos(), -m_RefLength, AA);
            screenToWorld(pEvent->pos(),  m_RefLength, BB);
            Vector3d U((BB-AA).normalized());

            double dcrit = 0.3;
            double dmax = 1.0e10;
            double dist = 1.0e10;


            Handle(Geom_TrimmedCurve) ln = GC_MakeSegment(gp_Pnt(AA.x, AA.y, AA.z) , gp_Pnt(BB.x, BB.y, BB.z));
//            Handle(Geom_Line) ln = new Geom_Line(gp_Pnt(AA.x, AA.y, AA.z), gp_Dir(U.x, U.y, U.z));
            Standard_Real First=0, Last=0;
            TopExp_Explorer FaceExplorer;
            TopExp_Explorer EdgeExplorer;
            int iFace=0, iEdge = 0;

            for(FaceExplorer.Init(shapes.First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
            {
                TopoDS_Face face = TopoDS::Face(FaceExplorer.Current());
                iEdge = 0;
                for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
                {
                    try
                    {
                        TopoDS_Edge edge = TopoDS::Edge(EdgeExplorer.Current());
                        Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(edge, First, Last);
                        GeomAPI_ExtremaCurveCurve ecc(ln, edgecurve);
                        dist = ecc.LowerDistance();
/*                        if(iFace==0&&iEdge==0)
                        {
                            gp_Pnt P0, P1;
                            ecc.NearestPoints(P0, P1);
                            m_DebugPts = {{P0.X(), P0.Y(), P0.Z()}, {P1.X(), P1.Y(), P1.Z()}};
                        }*/
                        if(dist<dcrit && dist<dmax)
                        {
                            dmax = dist;
                            m_HighFace = iFace;
                            m_HighEdge = iEdge;
                        }
                    }
                    catch(StdFail_NotDone const &)
                    {
                        qDebug("projection error on edge %d", iEdge);
                    }

                    iEdge++;
                }
                iFace++;
            }

            if(m_HighFace>=0 && m_HighEdge>=0)
            {
                if(m_HighFace!=lastpickedface || m_HighEdge!=lastpickededge)
                {
                    iFace = 0;
                    for(FaceExplorer.Init(shapes.First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
                    {
                        if(iFace==m_HighFace)
                        {
                            TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());
                            iEdge = 0;
                            for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
                            {
                                if(iEdge==m_HighEdge)
                                {
                                    TopoDS_Edge edge = TopoDS::Edge(EdgeExplorer.Current());
                                    setHighlightedEdge(edge);
                                    update();
                                    break;
                                }
                                iEdge++;
                            }
                            break;
                        }
                        iFace++;
                    }
                }
            }
            else
            {
                clearHighlightedEdge();
                update();
            }
            break;
        }
        default:
        case xfl::NOPICK: break;
    }

    update();
}


void gl3dSailView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(!m_bHasMouseMoved && m_bP3Select)
    {
        Vector3d I;
        if(pickPanel3(pEvent->pos(), m_pSail->triMesh().panels(), I))
        {
            emit panelSelected(m_PickedPanelIndex);
        }
    }
    else
    {
        int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

        if(draggeddistance<5 && (pEvent->button()==Qt::LeftButton))
        {
            if(bPickNode() || bPickVertex())
            {
                emit pickedNode(m_PickedPoint);
                if(m_PickedNodeIndex<0)
                {
                    // clear and start again node pair picking
                    m_NodePair = {-1,-1};
                    QApplication::restoreOverrideCursor();
                    return;
                }

                if(m_NodePair.first<0) m_NodePair.first = m_PickedNodeIndex;
                else if(m_NodePair.second<0)
                {
                    m_NodePair.second = m_PickedNodeIndex;
                    //two valid node indexes, merge them
                    emit pickedNodePair(m_NodePair);
                    m_NodePair = {-1,-1};
                }
                else
                {
                    //start again
                    m_NodePair = {m_PickedNodeIndex, -1};
                }
            }
            else if(m_PickType==xfl::SEGMENT3D)
            {

                SailOcc const *pOccSail = dynamic_cast<SailOcc const *>(m_pSail);
                if(pOccSail && m_HighFace>=0 && m_HighEdge>=0)
                {
                    TopExp_Explorer FaceExplorer;
                    TopExp_Explorer EdgeExplorer;
                    int iFace=0, iEdge=0;

                    for(FaceExplorer.Init(pOccSail->shapes().First(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
                    {
                        if(iFace==m_HighFace)
                        {
                            TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());
                            iEdge = 0;
                            for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
                            {
                                if(iEdge==m_HighEdge)
                                {
                                    TopoDS_Edge const &edge = TopoDS::Edge(EdgeExplorer.Current());
                                    if(edge.IsNull()) m_vboPickedEdge.destroy();
                                    else              glMakeEdge(edge, m_vboPickedEdge);

                                    break;                            }
                                iEdge++;
                            }
                            break;
                        }
                        iFace++;
                    }
                    QApplication::restoreOverrideCursor();
                    emit pickedEdge(m_HighFace, m_HighEdge);
                }
                else
                {
                    m_vboPickedEdge.destroy();
                }
            }
        }
    }

    gl3dXflView::mouseReleaseEvent(pEvent);
    update();
}


