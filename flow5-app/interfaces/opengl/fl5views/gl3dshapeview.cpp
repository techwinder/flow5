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



#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Poly.hxx>




#include <QMouseEvent>

#include "gl3dshapeview.h"

#include <api/occ_globals.h>
#include <api/fuse.h>
#include <api/fusexfl.h>
#include <api/utils.h>


#include <core/displayoptions.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/mesh/gmesh_globals.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_occ.h>


gl3dShapeView::gl3dShapeView(QWidget*pParent) : gl3dXflView(pParent)
{
    m_bResetglGeom = true;
    m_bResetglShape = true;

    m_pFuse  = nullptr;
    m_pShape = nullptr;
}


gl3dShapeView::~gl3dShapeView()
{
    for(int i=0; i<m_vboWires.size(); i++)
    {
        if(m_vboWires[i].isCreated()) m_vboWires[i].destroy();
    }
    m_vboWires.clear();

    if(m_vboEdges.isCreated()) m_vboEdges.destroy();
    if(m_vboFaces.isCreated()) m_vboFaces.destroy();
}


void gl3dShapeView::glRenderView()
{
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
        paintTriangles3Vtx(m_vboFaces, xfl::Orchid, false, true);

    if(m_bOutline)
    {
        paintSegments(m_vboEdges, W3dPrefs::s_OutlineStyle);
        paintWires();
    }

    QColor panelclr(200,70,40);
    if(m_bMeshPanels && m_Triangles.size())
    {
        paintTriangles3Vtx(m_vboTriangles, panelclr, false, true);
        paintSegments(m_vboTriangleEdges, W3dPrefs::s_PanelStyle);
        if(m_bNormals)
            paintNormals(m_vboNormals);
    }

    if(m_bHighlightPanel && m_HighlightList.size())
        paintTriangles3VtxOutline(m_vboHighlight, W3dPrefs::highlightColor(), W3dPrefs::highlightWidth());

    if(m_SLG.size())
    {
        paintSegments(m_vboSLG, panelclr.darker(250), 5, Line::SOLID, false);
        if(m_bShowMasses)
        {
            for(int i=0; i<int(m_SLG.size()); i++)
            {
                Segment3d const &seg = m_SLG.at(i);
                paintCube(seg.vertexAt(0).x, seg.vertexAt(0).y, seg.vertexAt(0).z, 0.0125/m_glScalef, panelclr.lighter(), true);
//                paintThinArrow(seg.vertexAt(0), seg.vertexAt(0).normal()/50.0, QColor(205,105,75), 3, Line::SOLID);
                QString strange;
                strange = QString::asprintf("%d", i);
                glRenderText(seg.CoG(), strange, DisplayOptions::textColor(), true);
            }
        }
    }

//    paintShapeVertices();

    if(isPicking() && m_PickedPanelIndex>=0)
    {
//        paintLine(m_vboSelectedFace, W3dPrefs::s_HighlightColor, 5);
        paintWires();

        QString strong;
        strong = QString::asprintf("Face_%d",m_PickedPanelIndex);
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
    else {
        clearTopRightOutput();
    }
}


bool gl3dShapeView::intersectTheObject(Vector3d const &A, Vector3d const&B, Vector3d &I)
{
    TopExp_Explorer shapeExplorer;

    Vector3d U =(B-A).normalized();

    int nFace = 0;
    for (shapeExplorer.Init(*m_pShape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Face aFace = TopoDS::Face(shapeExplorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);

        if(!hTriangulation.IsNull())
        {
            int nTriangles = hTriangulation->NbTriangles();

            for (int i=1; i<=nTriangles; i++)
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
    return false;
}


void gl3dShapeView::setFuse(Fuse *pFuse)
{
    m_pFuse = pFuse;
    m_OccTessParams = pFuse->occTessParams();
    m_GmshTessParams = pFuse->gmshTessParams();

    setReferenceLength(pFuse->length());

    if(pFuse->isXflType())
    {
        FuseXfl const *pFuseXfl = dynamic_cast<FuseXfl const*>(m_pFuse);
        m_pShape = &pFuseXfl->rightSideShells().First();
    }
    else if(pFuse->isOccType())
    {
        m_pShape = &m_pFuse->shells().First();

    }
    else clearShape();

    m_bResetglShape = true;
    std::string strange;
    occ::listShapeContent(*m_pShape, strange);

    setBotLeftOutput(strange);
    reset3dScale();
}


void gl3dShapeView::glMake3dObjects()
{
    if(m_bResetglShape && m_pFuse)
    {
        m_EdgeLabelPts.clear();
        gl::glMakeEdges(*m_pShape, m_vboEdges, m_EdgeLabelPts);
//        glMakeWires();

        QString log;
        gmesh::makeFuseTriangulation(m_pFuse, log);
        gl::makeTriangles3Vtx(m_pFuse->triangles(), false, m_vboFaces);

        m_bResetglShape = false;
    }

    if(m_bResetglGeom)
    {
        if(m_Triangles.size())
        {
            gl::makeTriangles3Vtx(m_Triangles, true, m_vboTriangles);
            gl::makeTrianglesOutline(m_Triangles, Vector3d(), m_vboTriangleEdges);
            if(m_bNormals) gl::makeTriangleNormals(m_Triangles, 0.05f, m_vboNormals);
        }
        if(m_SLG.size())
        {
            QVector<Segment3d> qVec = QVector<Segment3d>(m_SLG.begin(), m_SLG.end());
            gl::makeSegments(qVec, Vector3d(), m_vboSLG);
        }
        if(m_HighlightList.size())
        {
            std::vector<Triangle3d> highlighted(m_HighlightList.size());
            int nt = 0;
            for(int i=0; i<m_HighlightList.size(); i++)
            {
                if(m_HighlightList.at(i)>=0 && m_HighlightList.at(i)<int(m_Triangles.size()))
                {
                    highlighted[i] = m_Triangles.at(m_HighlightList.at(i));
                    nt++;
                }
            }
            highlighted.resize(nt);
            gl::makeTriangles3Vtx(highlighted, true, m_vboHighlight);
        }
        else
        {
            m_vboHighlight.bind();
            m_vboHighlight.destroy();
            m_vboHighlight.release();
        }

        m_bResetglGeom = false;
    }
}


void gl3dShapeView::paintWires()
{
    for(int i=0; i<m_vboWires.size(); i++)
        paintLineStrip(m_vboWires[i], W3dPrefs::highlightColor(), 2, Line::SOLID);
}


void gl3dShapeView::paintShapeVertices()
{
    if(!m_pShape) return;
    float side = 0.025f/float(m_glScalef);
    TopExp_Explorer shapeExplorer;
    for (shapeExplorer.Init(*m_pShape, TopAbs_VERTEX); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Vertex const &vtx = TopoDS::Vertex(shapeExplorer.Current());
        gp_Pnt P = BRep_Tool::Pnt(vtx);
        paintCube(P.X(), P.Y(), P.Z(), side, QColor(55,105,195), true);
    }
}


void gl3dShapeView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        m_PickedPanelIndex = -1;
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    if(!isPicking())
    {
        m_PickedPanelIndex = -1;
        return;
    }

    int LastPickedIndex = m_PickedPanelIndex;

    TopoDS_Face pickedface;
    pickFace(pEvent->pos(), *m_pShape, pickedface);

    if(m_PickedPanelIndex>=0 && LastPickedIndex!=m_PickedPanelIndex)
    {
        std::string log;

        m_Segments.clear();
        TopoDS_Wire theOuterWire;
        TopoDS_ListOfShape theInnerWires;
        occ::findWires(pickedface, theOuterWire, theInnerWires, log);
        for(int i=0; i<m_vboWires.size(); i++)
        {
            if(m_vboWires[i].isCreated()) m_vboWires[i].destroy();
        }
        m_vboWires.clear();

        for(TopTools_ListIteratorOfListOfShape ShapeIt(theInnerWires); ShapeIt.More(); ShapeIt.Next())
        {
            m_vboWires.append(QOpenGLBuffer());
            TopoDS_Wire const &aWire = TopoDS::Wire(ShapeIt.Value());

            gl::glMakeWire(aWire, m_vboWires.back());

            TopExp_Explorer WireExplorer;
            Standard_Real First(0),Last(0);
            gp_Pnt pt0, pt1;
            int nEdge = 0;
            for (WireExplorer.Init(aWire, TopAbs_EDGE); WireExplorer.More(); WireExplorer.Next())
            {
                TopoDS_Edge const &anEdge = TopoDS::Edge(WireExplorer.Current());
                Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(anEdge, First, Last);
                if(!edgecurve.IsNull())
                {
                    edgecurve->D0(First, pt0);
                    edgecurve->D0(Last,  pt1);
                    Vector3d v0(pt0.X(), pt0.Y(), pt0.Z());
                    Vector3d v1(pt1.X(), pt1.Y(), pt1.Z());
                    m_Segments.push_back({v0, v1});
                }
                nEdge++;
            }
            (void)nEdge;
        }
    }

    if(m_PickedPanelIndex<0)
    {
        m_vboWires.clear();
/*        m_DebugPts.clear();
        m_DebugVecs.clear(); */
    }
    update();
}



