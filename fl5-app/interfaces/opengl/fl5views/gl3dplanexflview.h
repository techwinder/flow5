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

#include <interfaces/opengl/fl5views/gl3dxflview.h>



class gl3dPlaneXflView : public gl3dXflView
{
	Q_OBJECT
    public:
        gl3dPlaneXflView(QWidget *pParent = nullptr);
        ~gl3dPlaneXflView();

        void setPlane(PlaneXfl *pPlane);

        void resetgl3dPlane() {m_bResetglPlane=true;}
        void resetgl3dFuse()  {m_bResetglFuse=true;}
        void resetgl3dP3Sel() {m_bResetP3Select=true;}

        bool pickTriPanel(QPoint const &point, bool &bFuse, Vector3d &I);
        void resetPickedNodes() override;
        int P3Node(int idx) {return m_iP3Node[idx%3];}
        void setP3Node(int idx, int iVal) {m_iP3Node[idx%3] =iVal;}
        void shiftP3Nodes();

        bool isP3Selecting()  const {return m_bP3Select;}
        void setP3Selecting(bool bSel) {m_bP3Select=bSel; if(!bSel) m_PickType=xfl::NOPICK;}
        void clearP3Selection() {m_P3Selection.clear();}
        int p3SelectionCount() const {return  m_P3Selection.size();}
        int p3SelectionAt(int idx) const {return  m_P3Selection.at(idx);}
        void p3SelRemove(int idx) {if(idx>=0 && idx<m_P3Selection.size()) m_P3Selection.removeAt(idx);}
        void glMakeP3Sel();

        static void highlightSelectedPart(bool bHighlight) {s_bHighlightSelectedPart=bHighlight;}
        static bool isHighlightingPart() {return s_bHighlightSelectedPart;}

    protected:
        void glMake3dObjects() override;
        void glRenderView() override;

        bool intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I) override;

        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        const Part *intersectPart(const Vector3d &AA,  Vector3d &U, Vector3d &I);

        virtual void paintMeshPanels();

    protected slots:
        void onPartSelClicked() override;


    protected:
        PlaneXfl *m_pPlaneXfl;

        bool m_bResetglPlane;
        bool m_bResetglFuse;

        bool m_bIsSelecting;
        QVector<int> m_P3Selection;
        int m_iP3Node[3];
        int m_iStrip;

        static bool s_bHighlightSelectedPart;

        QVector<QOpenGLBuffer> m_vboWingSurface, m_vboWingOutline;
        QVector<QOpenGLBuffer> m_vboTriMesh;
        QVector<QOpenGLBuffer> m_vboTriEdges;

        QOpenGLBuffer m_vboFrames;
        QVector<QOpenGLBuffer> m_vboBodyOutline;
        QVector<QOpenGLBuffer> m_vboFuseTriangulation;

        bool m_bResetP3Select;
        QOpenGLBuffer m_vboP3Select;
};

