/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

#include <xfl/opengl/gl3dxflview.h>
#include <xflgeom/geom3d/slg3d.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflocc/occmeshparams.h>



class gl3dShapeView : public gl3dXflView
{
    public:
        gl3dShapeView(QWidget *pParent = nullptr);
        ~gl3dShapeView();

        void clearShape() {m_pShape=nullptr;}
        void setShape(TopoDS_Shape const &shape, OccMeshParams const& params);

        void setTriangles(QVector<Triangle3d> const &triangles, QVector<int> const &high) {m_Triangles=triangles; m_HighlightList=high;}
        void setPSLG(SLG3d const &pslg) {m_SLG=pslg;}

        void clearSLG() {m_SLG.clear();}
        void clearTriangles() {m_Triangles.clear(); m_HighlightList.clear();}
        void clearHighlighted() {m_HighlightList.clear();}
        void resetGeometry() {m_bResetglGeom=true;}

    private:
        void mouseMoveEvent(QMouseEvent *pEvent) override;

        void glRenderView() override;
        void glMake3dObjects() override;

        void paintWires();
        void paintShapeVertices();

        bool intersectTheObject(Vector3d const &AA, Vector3d const &B, Vector3d &I) override;

    protected:

        bool m_bResetglShape;
        bool m_bResetglGeom;

        TopoDS_Shape const *m_pShape;

        QOpenGLBuffer m_vboFaces;
        QVector<QOpenGLBuffer> m_vboWires;
        QOpenGLBuffer m_vboEdges;
        QVector<Vector3d>m_EdgeLabelPts;

        QOpenGLBuffer m_vboSLG, m_vboTriangles, m_vboNormals, m_vboTriangleEdges;
        QOpenGLBuffer m_vboHighlight;

        QVector<Triangle3d> m_Triangles;
        SLG3d m_SLG;

        QVector<int> m_HighlightList;

        OccMeshParams m_OccMeshParams;
};

