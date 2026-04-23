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
#include <QVBoxLayout>
#include <QApplication>

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <Geom_Line.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <GC_MakeSegment.hxx>
#include <Poly_Triangulation.hxx>

#include "gl3dshapesview.h"

#include <interfaces/opengl/globals/gl_occ.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <api/geom_global.h>
#include <api/fuse.h>
#include <api/occ_globals.h>
#include <api/occmeshparams.h>


gl3dShapesView::gl3dShapesView(QWidget*pParent) : gl3dXflView(pParent)
{
    setWindowTitle("OCC shape editor");

    m_pShapes = nullptr;

    m_bResetglShape = true;
    m_bResetglTriangles = true;
    m_bResetglNurbs = true;
}


gl3dShapesView::~gl3dShapesView()
{
}


void gl3dShapesView::glRenderView()
{
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


    if(m_bSurfaces)
    {
        QColor shapeclr(143, 151, 172);
        for(int i=0; i<m_vboShapes.size(); i++)
        {
            if(m_bVisible.at(i))
                paintTriangles3Vtx(m_vboShapes[i], shapeclr, false, true);
        }
    }

    if(m_bOutline)
    {
        for(int i=0; i<m_vboEdges.size(); i++)
        {
            paintSegments(m_vboEdges[i], W3dPrefs::s_OutlineStyle);
        }
//        for(int ie=0; ie<m_EdgeLabelPts.size(); ie++)
//            glRenderText(m_EdgeLabelPts[ie], QString::asprintf("E%d", ie), s_TextColor, true);
    }

    for(int idx=0; idx<m_HighlightedPart.size(); idx++)
    {
        int iHigh = m_HighlightedPart.at(idx);
        if(iHigh>=0 && iHigh<m_vboEdges.size())
            paintSegments(m_vboEdges[iHigh], W3dPrefs::s_HighStyle);
    }

    if(m_vboHighEdge.isCreated())
        paintSegments(m_vboHighEdge, W3dPrefs::s_HighStyle);

    if(m_vboPickedEdge.isCreated())
        paintSegments(m_vboPickedEdge, W3dPrefs::s_SelectStyle);

    if(m_bMeshPanels && m_Triangles.size())
    {
        paintTriangles3Vtx(m_vboTriangles, W3dPrefs::s_WingPanelColor, false, true);
        paintSegments(m_vboTriangleEdges, W3dPrefs::s_PanelStyle);
    }

    if(m_SLG.size())
    {
        QColor segclr(195, 155,135);
        paintSegments(m_vboSLG, segclr.darker(), 3, Line::SOLID, false);
        if(m_bShowMasses)
        {
            for(int i=0; i<int(m_SLG.size()); i++)
            {
                Segment3d const &seg = m_SLG.at(i);
                for(int j=0; j<2; j++)
                {
                    paintCube(seg.vertexAt(j).x, seg.vertexAt(j).y, seg.vertexAt(j).z, 0.0125/m_glScalef, segclr.lighter(), true);
                }
                QString strange;
                strange = QString::asprintf("%d", i);
                glRenderText(seg.CoG(), strange, DisplayOptions::textColor(), true);
            }
        }
    }

    if(m_Nodes.size() && m_bEdgeNodes)
    {
        for(int i=0; i<m_Nodes.size(); i++)
        {
            paintIcoSphere(m_Nodes.at(i), 0.013/double(m_glScalef), W3dPrefs::s_PanelStyle.m_Color, true, false);
        }
    }

    if(isPicking() && m_PickedPanelIndex>=0 && m_bHighlightPanel)
    {
        paintTriangle(m_vboTriangle, true, false, Qt::black);

        QString strong;
        strong = QString::asprintf("T%d",m_PickedPanelIndex);
        glRenderText(m_PickedPoint.x+0.015/double(m_glScalef), m_PickedPoint.y+0.015/double(m_glScalef), m_PickedPoint.z+0.015/double(m_glScalef),
                     strong, DisplayOptions::textColor(), true);

        for(int i=0; i<m_Segments.size(); i++)
        {
            Vector3d pt = m_Segments.at(i).midPoint();
            QString strong;
            strong = QString::asprintf("E%d",i);
            glRenderText(pt.x+0.015/double(m_glScalef), pt.y+0.015/double(m_glScalef), pt.z+0.015/double(m_glScalef),
                         strong, DisplayOptions::textColor(), true);
        }
    }
    else
    {
        clearTopRightOutput();
    }
}


bool gl3dShapesView::intersectTheObject(Vector3d const &A, Vector3d const&B, Vector3d &I)
{
    if(!m_pShapes || !m_pShapes->size()) return false;

    TopExp_Explorer shapeExplorer;

    Vector3d U =(B-A).normalized();

    gp_Dir N1,N2,N3;

    for (int iShape=0; iShape<m_pShapes->size(); iShape++) // for each of the fuse (cut) shells
    {
        int nFace = 0;
        for (shapeExplorer.Init(m_pShapes->value(iShape), TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Face aFace = TopoDS::Face(shapeExplorer.Current());
            TopLoc_Location location;
            Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);

            if(!hTriangulation.IsNull())
            {
//                const TColgp_Array1OfPnt& nodes = hTriangulation->Nodes();
//                const Poly_Array1OfTriangle& triangles = hTriangulation->Triangles();

                for (int i=1; i<=hTriangulation->NbTriangles(); i++)
                {
                    const Poly_Triangle& tri = hTriangulation->Triangle(i);
                    Vector3d p1 = Vector3d(hTriangulation->Node(tri(1)).X(), hTriangulation->Node(tri(1)).Y(), hTriangulation->Node(tri(1)).Z());
                    Vector3d p2 = Vector3d(hTriangulation->Node(tri(2)).X(), hTriangulation->Node(tri(2)).Y(), hTriangulation->Node(tri(2)).Z());
                    Vector3d p3 = Vector3d(hTriangulation->Node(tri(3)).X(), hTriangulation->Node(tri(3)).Y(), hTriangulation->Node(tri(3)).Z());
                    Triangle3d t3d(p1,p2,p3);
                    if(t3d.intersectRayInside(A,U,I)) return true;
                }
            }
            nFace++;
        }
        (void)nFace;
    }
    return false;
}


void gl3dShapesView::setShapes(QVector<TopoDS_Shape> const &shapes)
{
    m_pShapes = &shapes;
    m_bResetglShape = true;
    m_bVisible.resize(m_pShapes->size());
    m_bVisible.fill(true);
    updatePartFrame();
}


void gl3dShapesView::resetShapes()
{
    m_Triangles.clear();
    m_bResetglShape = true;
    m_bVisible.resize(m_pShapes->size());
    m_bVisible.fill(true);
    updatePartFrame();
}


void gl3dShapesView::glMake3dObjects()
{
    if(m_bResetglShape)
    {
        m_EdgeLabelPts.clear();
        if(m_pShapes && m_pShapes->size())
        {
            for(int i=0; i<m_vboShapes.size(); i++)
                if(m_vboShapes[i].isCreated()) m_vboShapes[i].destroy();
            m_vboShapes.resize(m_pShapes->size());

            for(int i=0; i<m_vboEdges.size(); i++)
                if(m_vboEdges[i].isCreated()) m_vboEdges[i].destroy();
            m_vboEdges.resize(m_pShapes->size());

            for(int iShape=0; iShape<m_pShapes->size(); iShape++)
            {
                BRepTools::Clean(m_pShapes->value(iShape));

                gl::glMakeShapeTriangles(m_pShapes->value(iShape), m_OccMeshParams, m_vboShapes[iShape]);
                gl::glMakeEdges(m_pShapes->value(iShape), m_vboEdges[iShape], m_EdgeLabelPts);
            }
        }
    }

    if(m_bResetglShape || m_bResetglTriangles)
    {
        if(m_Triangles.size())
        {
            gl::makeTriangles3Vtx(m_Triangles, true, m_vboTriangles);
            gl::makeTrianglesOutline(m_Triangles, Vector3d(), m_vboTriangleEdges);
        }
        if(m_SLG.size())
        {
            QVector<Segment3d> qVec = QVector<Segment3d>(m_SLG.begin(), m_SLG.end());
            gl::makeSegments(qVec, Vector3d(), m_vboSLG);
        }
    }

    m_bResetglTriangles = false;
    m_bResetglShape     = false;
    m_bResetglNurbs     = false;
}


void gl3dShapesView::setHighlightedEdge(TopoDS_Edge const &Edge)
{
    if(Edge.IsNull())  m_vboHighEdge.destroy();
    else
        gl::glMakeEdge(Edge, m_vboHighEdge);
}


void gl3dShapesView::updatePartFrame()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());
    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    if(m_pfrParts) delete m_pfrParts;
    m_pfrParts = new QFrame(this);
    m_pfrParts->setCursor(Qt::ArrowCursor);
    m_pfrParts->setAutoFillBackground(true);
    m_pfrParts->setPalette(palette);
    m_pfrParts->setFrameShape(QFrame::NoFrame);
    m_pfrParts->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_pfrParts->setAttribute(Qt::WA_NoSystemBackground);

    qDeleteAll(m_pfrParts->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
    QLayout *pCurrentLayout = m_pfrParts->layout();
    if(pCurrentLayout) delete pCurrentLayout;

    int w=0;
    int h=0;
    QVBoxLayout *pPartLayout = new QVBoxLayout;
    {
        for(int ish=0; ish<m_pShapes->size(); ish++)
        {
//            TopoDS_Shape const &shape = m_pShapes->at(ish);
            QCheckBox *pShapeCH = new QCheckBox(QString::asprintf("Shape %d", ish));
            pShapeCH->setPalette(palette);
            pShapeCH->setChecked(m_bVisible.at(ish));
            pShapeCH->setProperty("partindex", ish);
            pShapeCH->adjustSize();
            w = std::max(w, pShapeCH->width());
            h += pShapeCH->height();
            connect(pShapeCH, SIGNAL(toggled(bool)), SLOT(onPartSelClicked()));
            pPartLayout->addWidget(pShapeCH);
        }
    }
    (void)h;

    m_pfrParts->setLayout(pPartLayout);
    m_pfrParts->show();
    m_pfrParts->adjustSize();
}


void gl3dShapesView::onPartSelClicked()
{
    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if(!pSenderBox) return;

    QVariant property = pSenderBox->property("partindex");
    if(property.isNull()) return;

    bool bOk = false;
    int index = property.toInt(&bOk);
    if(!bOk) return;

    if(index<0 || index>=m_pShapes->size()) return;
    if(index<0 || index>=m_bVisible.size()) return;

    m_bVisible[index] = pSenderBox->isChecked();

    // the geometry does not need to be rebuilt
    m_bResetglMesh = true;
    update();
}


void gl3dShapesView::mouseMoveEvent(QMouseEvent *pEvent)
{
    int lastpickedindex = m_PickedPanelIndex;
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    if(!isPicking())
    {
        clearTopRightOutput();
        m_PickedPanelIndex = -1;
        return;
    }

    switch (m_PickType)
    {
        case xfl::TRIANGLE3D:
        {
            Vector3d I;
            pickTriangle3d(pEvent->pos(), m_Triangles, I);

            if(m_PickedPanelIndex<0 || m_PickedPanelIndex>=int(m_Triangles.size()))
            {
                m_PickedPanelIndex = -1;
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
                update();
            }
            else if(m_PickedPanelIndex!=lastpickedindex)
            {
                Triangle3d const &p3 = m_Triangles.at(m_PickedPanelIndex);
                if(m_bHighlightPanel)
                {
                    gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);

                    setTopRightOutput(QString::fromStdString(p3.properties(false)));
                }
                update();
            }
            break;
        }
        case xfl::SEGMENT3D:
        {
            if(!m_pShapes->size()) break;

            int lastpickedface = m_HighFace;
            int lastpickededge = m_HighEdge;
            m_HighFace = m_HighEdge = -1;
            Vector3d I;
            Vector3d AA, BB;

            screenToWorld(pEvent->pos(), -m_RefLength, AA);
            screenToWorld(pEvent->pos(),  m_RefLength, BB);
            Vector3d U((BB-AA).normalized());

            double dcrit = 0.03;
            double dmax = LARGEVALUE;
            double dist = LARGEVALUE;

            Handle(Geom_TrimmedCurve) ln = GC_MakeSegment(gp_Pnt(AA.x, AA.y, AA.z) , gp_Pnt(BB.x, BB.y, BB.z));
//            Handle(Geom_Line) ln = new Geom_Line(gp_Pnt(AA.x, AA.y, AA.z), gp_Dir(U.x, U.y, U.z));
            Standard_Real First=0, Last=0;
            TopExp_Explorer FaceExplorer;
            TopExp_Explorer EdgeExplorer;
            int iFace=0, iEdge = 0;

            for(FaceExplorer.Init(m_pShapes->first(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
            {
                TopoDS_Face const &face = TopoDS::Face(FaceExplorer.Current());
                iEdge = 0;
                for(EdgeExplorer.Init(face, TopAbs_EDGE); EdgeExplorer.More(); EdgeExplorer.Next())
                {
                    try
                    {
                        TopoDS_Edge edge = TopoDS::Edge(EdgeExplorer.Current());
                        Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(edge, First, Last);
                        GeomAPI_ExtremaCurveCurve projector(ln, edgecurve);
                        dist = projector.LowerDistance();
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
                    for(FaceExplorer.Init(m_pShapes->first(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
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
        {
            break;
        }
    }
}


void gl3dShapesView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

    if(draggeddistance<5 && (pEvent->button()==Qt::LeftButton))
    {
        if(bPickNode() && m_PickedNodeIndex>=0)
        {
            if(m_PickedNodeIndex<0) // ????
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
                emit pickedNodePair(m_NodePair);
            }
            else
            {
                //start again
                m_NodePair = {m_PickedNodeIndex, -1};
            }
        }
        else if(m_PickType==xfl::SEGMENT3D)
        {
            if(m_HighFace>=0 && m_HighEdge>=0)
            {
                emit pickedEdge(m_HighFace, m_HighEdge);

                TopExp_Explorer FaceExplorer;
                TopExp_Explorer EdgeExplorer;
                int iFace=0, iEdge=0;

                for(FaceExplorer.Init(m_pShapes->first(), TopAbs_FACE); FaceExplorer.More(); FaceExplorer.Next())
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
                                else              gl::glMakeEdge(edge, m_vboPickedEdge);

                                break;                            }
                            iEdge++;
                        }
                        break;
                    }
                    iFace++;
                }
            }
            else
            {
                m_vboPickedEdge.destroy();
            }
        }
    }
    gl3dXflView::mouseReleaseEvent(pEvent);
}
