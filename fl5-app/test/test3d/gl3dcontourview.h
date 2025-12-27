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

#include <QComboBox>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/triangle3d.h>

class IntEdit;
class FloatEdit;
class gl3dContourView : public gl3dXflView
{
    Q_OBJECT
    public:
        gl3dContourView(QWidget *pParent = nullptr);

        void glRenderView() override;
        void glMake3dObjects() override;

        bool intersectTheObject(Vector3d const &A, Vector3d const &B, Vector3d &I) override;

        QSize sizeHint() const override {return QSize(1200, 950);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void keyPressEvent(QKeyEvent *pEvent) override;

        void resetView() {m_bResetView=true;}


        void hideSegment() {m_bShowSegment=false;}
        void hidePoints() {m_bShowPoints=false;}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glMakeTriangleContoursOnGrid(QOpenGLBuffer &vbo, int nrows, int ncols, QVector<Vector3d> const&nodes, QVector<double> const &values) const;

    private slots:
        void onChanged();

    private:

        QSlider *m_psPosition;
        QComboBox *m_pcbPlaneDir;
        IntEdit *m_pieNRows;

        bool m_bResetView;
        bool m_bShowSegment;
        bool m_bShowPoints;

        QOpenGLBuffer m_vboTriangles, m_vboTriangleEdges;
        QOpenGLBuffer m_vboSegment;
        QOpenGLBuffer m_vboClrMap;
        QOpenGLBuffer m_vboContours;

        static int s_NRows;
};

