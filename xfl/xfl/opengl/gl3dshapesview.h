/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <xfl/opengl/gl3dxflview.h>
#include <xflgeom/geom3d/slg3d.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflgeom/geom3d/nurbssurface.h>
#include <xflocc/occmeshparams.h>


class gl3dShapesView : public gl3dXflView
{
    public:
        gl3dShapesView(QWidget *pParent = nullptr);
        ~gl3dShapesView();

        void setShapes(QVector<TopoDS_Shape> const &shapes);
        void setTriangles(QVector<Triangle3d> const &triangles) {m_Triangles=triangles; m_bResetglTriangles=true;}
        void clearTriangles() {m_Triangles.clear(); m_bResetglTriangles=true;}

        int highlighted() const {return m_HighlightedPart.size();}
        void setHighlighted(int iShape) {m_HighlightedPart.append(iShape);}
        void clearHighlighted() {m_HighlightedPart.clear();}

        void setPSLG(SLG3d const &pslg) {m_SLG=pslg;}
        void clearSLG() {m_SLG.clear();}

        void setNurbs(NURBSSurface const &nurbs) {m_Nurbs.copy(nurbs);}

        void resetShapes();
        void resetTriangles() {m_bResetglTriangles = true;}
        void resetNurbs() {m_bResetglNurbs= true;}

        void setHighlightedEdge(TopoDS_Edge const &Edge);
        void clearHighlightedEdge() {m_vboHighEdge.destroy();}


        OccMeshParams const &occMeshParams() const {return m_OccMeshParams;}
        void setOccMeshParams(OccMeshParams const &params) {m_OccMeshParams=params;}

    private:
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const &AA, Vector3d const &B, Vector3d &I) override;

        void updatePartFrame();
        void onPartSelClicked() override;

    protected:

        bool m_bResetglShape, m_bResetglTriangles, m_bResetglNurbs;
        QVector<int> m_HighlightedPart;
        QVector<TopoDS_Shape> const *m_pShapes;
        QVector<bool> m_bVisible;

        QVector<Triangle3d> m_Triangles;
        SLG3d m_SLG;

        NURBSSurface m_Nurbs;

        OccMeshParams m_OccMeshParams;


        QVector<QOpenGLBuffer> m_vboShapes;
        QVector<QOpenGLBuffer> m_vboEdges;
        QVector<Vector3d> m_EdgeLabelPts;

        QOpenGLBuffer m_vboTriangles, m_vboTriangleEdges, m_vboSLG;
        QOpenGLBuffer m_vboHighEdge, m_vboPickedEdge;

};

