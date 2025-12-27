/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QVector>

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Poly.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

#define _MATH_DEFINES_DEFINED

#include "gl_occ.h"
#include <api/occ_globals.h>

#define WIRERES 77
void gl::glMakeWire(TopoDS_Wire const &wire, QOpenGLBuffer &vbo)
{
    if(wire.IsNull())
    {
        vbo.destroy();
        return;
    }

    Standard_Real First=0,Last=0;
    gp_Pnt pt0, pt1;
    int nEdge=0;

    TopExp_Explorer WireExplorer;

    QVector<float> OutlineVertexArray;

    for (WireExplorer.Init(wire, TopAbs_EDGE); WireExplorer.More(); WireExplorer.Next())
    {
        TopoDS_Edge const &anEdge = TopoDS::Edge(WireExplorer.Current());
        Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(anEdge, First, Last);
        if(!edgecurve.IsNull())
        {
            for(int ji=0; ji<=WIRERES; ji++)
            {
                int i=ji;
                if(anEdge.Orientation()==TopAbs_REVERSED) i = WIRERES-ji;

                double df0 = double(i)  /(WIRERES);
                double df1 = double(i+1)/(WIRERES);
                double u0 = First + df0 * (Last-First);
                double u1 = First + df1 * (Last-First);
                edgecurve->D0(u0, pt0);
                edgecurve->D0(u1, pt1);
                OutlineVertexArray.push_back(float(pt0.X()));
                OutlineVertexArray.push_back(float(pt0.Y()));
                OutlineVertexArray.push_back(float(pt0.Z()));
            }
        }

        nEdge++;
    }
    (void)nEdge;
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), OutlineVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::glMakeShapeTriangles(TopoDS_Shape const &shape, OccMeshParams const& params, QOpenGLBuffer &vbo)
{
    TopExp_Explorer shapeExplorer;
    QVector<float> meshvertexarray;
    gp_Dir N1,N2,N3;
    int nFace = 0;
    for (shapeExplorer.Init(shape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Face aFace = TopoDS::Face(shapeExplorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);
        if(hTriangulation.IsNull())
        {
            // make the triangulation on the fly
//            BRepMesh_IncrementalMesh(aFace, 0.1);
            if(params.isRelativeDeflection())
                BRepMesh_IncrementalMesh(aFace, params.deflectionAbsolute(), params.isRelativeDeflection(),
                                         params.angularDeviation()*PI/180.0, true);
            else
                BRepMesh_IncrementalMesh(aFace, params.deflectionRelative(), params.isRelativeDeflection(),
                                         params.angularDeviation()*PI/180.0, true);
        }
        hTriangulation = BRep_Tool::Triangulation(aFace, location);

        if(!hTriangulation.IsNull())
        {
            Poly::ComputeNormals(hTriangulation);

//            const TColgp_Array1OfPnt& nodes = hTriangulation->Nodes();
//            const Poly_Array1OfTriangle& triangles = hTriangulation->Triangles();

            for (int i=1; i<=hTriangulation->NbTriangles();  i++)
            {
                const Poly_Triangle& tri = hTriangulation->Triangle(i);
                const Vector3d p1 = Vector3d(hTriangulation->Node(tri(1)).X(), hTriangulation->Node(tri(1)).Y(), hTriangulation->Node(tri(1)).Z());
                Vector3d p2 = Vector3d(hTriangulation->Node(tri(2)).X(), hTriangulation->Node(tri(2)).Y(), hTriangulation->Node(tri(2)).Z());
                Vector3d p3 = Vector3d(hTriangulation->Node(tri(3)).X(), hTriangulation->Node(tri(3)).Y(), hTriangulation->Node(tri(3)).Z());

                N1 = hTriangulation->Normal(tri(1));
                N2 = hTriangulation->Normal(tri(2));
                N3 = hTriangulation->Normal(tri(3));

                if(aFace.Orientation()==TopAbs_REVERSED)
                {
                    Vector3d tmp = p3;
                    p3 = p2;
                    p2 = tmp;
                    N1.Reverse();
                    N2.Reverse();
                    N3.Reverse();
                }

                meshvertexarray.push_back(p1.xf());
                meshvertexarray.push_back(p1.yf());
                meshvertexarray.push_back(p1.zf());
                meshvertexarray.push_back(float(N1.X()));
                meshvertexarray.push_back(float(N1.Y()));
                meshvertexarray.push_back(float(N1.Z()));

                meshvertexarray.push_back(p2.xf());
                meshvertexarray.push_back(p2.yf());
                meshvertexarray.push_back(p2.zf());
                meshvertexarray.push_back(float(N2.X()));
                meshvertexarray.push_back(float(N2.Y()));
                meshvertexarray.push_back(float(N2.Z()));

                meshvertexarray.push_back(p3.xf());
                meshvertexarray.push_back(p3.yf());
                meshvertexarray.push_back(p3.zf());
                meshvertexarray.push_back(float(N3.X()));
                meshvertexarray.push_back(float(N3.Y()));
                meshvertexarray.push_back(float(N3.Z()));
            }
        }

        nFace++;
    }
    (void)nFace;
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), meshvertexarray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::glMakeEdges(TopoDS_Shape const &Shape, QOpenGLBuffer &vboEdges, QVector<Vector3d> &labelpoints)
{
    //could also use BRep_Tool::PolygonOnSurface
    std::string strange;
    TopoDS_ListOfShape theEdges;

    occ::findEdges(Shape, theEdges, strange);

    glMakeEdges(theEdges, Vector3d(), vboEdges, labelpoints);
}


#define EDGERES 37

void gl::glMakeEdges(TopoDS_ListOfShape const &theEdges, Vector3d const &T, QOpenGLBuffer &vboEdges, QVector<Vector3d> &labelpoints)
{
    if(vboEdges.isCreated()) vboEdges.destroy();

    Standard_Real First=0,Last=0;
    gp_Pnt pt0, pt1;

    QVector<float> OutlineVertexArray;
    int nEdge=0;
    for(TopTools_ListIteratorOfListOfShape EdgeIt(theEdges); EdgeIt.More(); EdgeIt.Next())
    {
        TopoDS_Edge const &anEdge = TopoDS::Edge(EdgeIt.Value());

        Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(anEdge, First, Last);
        if(!edgecurve.IsNull())
        {
            for(int i=0; i<EDGERES; i++)
            {
                double df0 = double(i)  /(EDGERES);
                double df1 = double(i+1)/(EDGERES);
                double u0 = First + df0 * (Last-First);
                double u1 = First + df1 * (Last-First);
                edgecurve->D0(u0, pt0);
                edgecurve->D0(u1, pt1);
                OutlineVertexArray.push_back(float(pt0.X() + T.x));
                OutlineVertexArray.push_back(float(pt0.Y() + T.y));
                OutlineVertexArray.push_back(float(pt0.Z() + T.z));
                OutlineVertexArray.push_back(float(pt1.X() + T.x));
                OutlineVertexArray.push_back(float(pt1.Y() + T.y));
                OutlineVertexArray.push_back(float(pt1.Z() + T.z));
                if(i==int(EDGERES/2))
                {
                    labelpoints.append({pt0.X(), pt0.Y(), pt0.Z()});
                }
            }
        }
        else
        {
//            qDebug()<<"Discarding null edge" << nEdge;
        }

        nEdge++;
    }
    (void)nEdge;

    vboEdges.create();
    vboEdges.bind();
    vboEdges.allocate(OutlineVertexArray.data(), OutlineVertexArray.size() * int(sizeof(GLfloat)));
    vboEdges.release();
}


void gl::glMakeEdge(const TopoDS_Edge &Edge, QOpenGLBuffer &vboEdge)
{
    if(vboEdge.isCreated()) vboEdge.destroy();

    //could also use BRep_Tool::PolygonOnSurface

    Standard_Real First=0, Last=0;
    gp_Pnt pt0, pt1;

    QVector<float> OutlineVertexArray;

    Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(Edge, First, Last);
    if(!edgecurve.IsNull())
    {
        for(int i=0; i<EDGERES; i++)
        {
            double df0 = double(i)  /(EDGERES);
            double df1 = double(i+1)/(EDGERES);
            double u0 = First + df0 * (Last-First);
            double u1 = First + df1 * (Last-First);
            edgecurve->D0(u0, pt0);
            edgecurve->D0(u1, pt1);
            OutlineVertexArray.push_back(float(pt0.X()));
            OutlineVertexArray.push_back(float(pt0.Y()));
            OutlineVertexArray.push_back(float(pt0.Z()));
            OutlineVertexArray.push_back(float(pt1.X()));
            OutlineVertexArray.push_back(float(pt1.Y()));
            OutlineVertexArray.push_back(float(pt1.Z()));
        }
    }

    vboEdge.create();
    vboEdge.bind();
    vboEdge.allocate(OutlineVertexArray.data(), OutlineVertexArray.size() * int(sizeof(GLfloat)));
    vboEdge.release();
}


void gl::glMakeShellOutline(const TopoDS_ListOfShape &shapes, const Vector3d &position, QOpenGLBuffer &vbo, int nWireRes)
{
    if(shapes.Extent()<=0)
    {
        vbo.destroy();
        return;
    }

    std::string strange;

    //OUTLINE
    QVector<float> OutlineVertexArray;

    TopoDS_ListIteratorOfListOfShape iterator;
    Standard_Real First,Last;
    gp_Pnt pt0, pt1;

    int iShape=0;
    for (iterator.Initialize(shapes); iterator.More(); iterator.Next()) // for each of the fuse (cut) shells
    {
        TopExp_Explorer shapeExplorer;
        int iFace=0;
        // for each face of the shape
        for (shapeExplorer.Init(iterator.Value(), TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Face face = TopoDS::Face(shapeExplorer.Current());

            TopoDS_Wire theOuterWire;
            TopoDS_ListOfShape theInnerWires;
            occ::findWires(face, theOuterWire, theInnerWires, strange);
            theInnerWires.Append(theOuterWire);

            int iWire=0;
            // for each of the face's wire
            for(TopTools_ListIteratorOfListOfShape WireIt(theInnerWires); WireIt.More(); WireIt.Next())
            {
                TopExp_Explorer WireExplorer;

                int iEdge=0;
                //for each edge of the wire
                for (WireExplorer.Init(WireIt.Value(), TopAbs_EDGE); WireExplorer.More(); WireExplorer.Next())
                {
                    TopoDS_Edge const &anEdge = TopoDS::Edge(WireExplorer.Current());
                    Handle(Geom_Curve) edgecurve = BRep_Tool::Curve(anEdge, First, Last);
                    if(!edgecurve.IsNull())
                    {
                        for(int ji=0; ji<nWireRes; ji++)
                        {
                            int i=ji;
                            if(anEdge.Orientation()==TopAbs_REVERSED) i = nWireRes-ji-1;

                            double df0 = double(i)  /double(nWireRes);
                            double df1 = double(i+1)/double(nWireRes);
                            double u0 = First + df0 * (Last-First);
                            double u1 = First + df1 * (Last-First);
                            edgecurve->D0(u0, pt0);
                            edgecurve->D0(u1, pt1);
                            OutlineVertexArray.push_back(position.xf() + float(pt0.X()));
                            OutlineVertexArray.push_back(position.yf() + float(pt0.Y()));
                            OutlineVertexArray.push_back(position.zf() + float(pt0.Z()));
                            OutlineVertexArray.push_back(position.xf() + float(pt1.X()));
                            OutlineVertexArray.push_back(position.yf() + float(pt1.Y()));
                            OutlineVertexArray.push_back(position.zf() + float(pt1.Z()));
                        }
                    }
                    iEdge++;
                }
                (void)iEdge;
                iWire++;
            }
            (void)iWire;
            iFace++;
        }
        (void)iFace;
        iShape++;
    }
    (void)iShape;
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), OutlineVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}

