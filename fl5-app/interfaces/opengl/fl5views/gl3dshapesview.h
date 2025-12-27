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

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/mesh/slg3d.h>
#include <api/triangle3d.h>
#include <api/nurbssurface.h>
#include <api/occmeshparams.h>


class gl3dShapesView : public gl3dXflView
{
    public:
        gl3dShapesView(QWidget *pParent = nullptr);
        ~gl3dShapesView();

        void setShapes(QVector<TopoDS_Shape> const &shapes);
        void setTriangles(std::vector<Triangle3d> const &triangles) {m_Triangles=triangles; m_bResetglTriangles=true;}
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

        std::vector<Triangle3d> m_Triangles;
        SLG3d m_SLG;

        NURBSSurface m_Nurbs;

        OccMeshParams m_OccMeshParams;


        QVector<QOpenGLBuffer> m_vboShapes;
        QVector<QOpenGLBuffer> m_vboEdges;
        QVector<Vector3d> m_EdgeLabelPts;

        QOpenGLBuffer m_vboTriangles, m_vboTriangleEdges, m_vboSLG;
        QOpenGLBuffer m_vboHighEdge, m_vboPickedEdge;

};

