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

#include <QLabel>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/triangle3d.h>
#include <api/testpanels.h>
#include <api/panel3.h>
#include <api/panel4.h>

class FloatEdit;
class IntEdit;

class gl3dPanelField: public gl3dXflView
{
    Q_OBJECT
    public:
        gl3dPanelField(QWidget *pParent = nullptr);

        bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) override;

        void clearPanels() {m_Panel3.clear(); m_Panel4.clear(); m_bResetView=true;}

        void setPanel3(std::vector<Panel3> const &p3) {m_Panel3=p3; m_bResetView=true;}
        void setPanel4(std::vector<Panel4> const &p4) {m_Panel4=p4; m_bResetView=true;}
        void setPlotLine(Vector3d const &P0, Vector3d const &P1) {m_PlotLine.setNodes(P0, P1);}

        void setCore(Vortex::enumVortex vortexmodel, double cr) {m_VortexModel=vortexmodel, m_CoreRadius=cr;}
        void setSource(bool b) {m_bSource=b;}
        void setMethod(PANELMETHOD method) {m_Method=method;}

        void resetVectors() {m_bResetView=true;}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:

        void keyPressEvent(QKeyEvent *pEvent) override;

        void glRenderView() override;
        void glMake3dObjects() override;

        QSize sizeHint() const override {return QSize(1100,1000);}

        void resizeGL(int width, int height) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void makeVectorPlotLine();
        void glMakePlotLine();

        void paintVectors();
        void readData();
        void makeControls();

        void glMakeVectors();

    public slots:
        void onRefLength();
        void onMakeView();

    private:
        std::vector<Panel3> m_Panel3;
        std::vector<Panel4> m_Panel4;

        bool m_bSource;
        PANELMETHOD m_Method;

        Vortex::enumVortex m_VortexModel;
        double m_CoreRadius;

        Segment3d m_PlotLine;

        QVector<Vector3d> m_PointArray; // Size x Size x Size
        QVector<Vector3d> m_VectorArray; // Size x Size x Size

        QOpenGLBuffer m_vboVectors;
        QOpenGLBuffer m_vboPanels, m_vboEdges;
        QOpenGLBuffer m_vboPlotLine;

        bool m_bResetView;


        QLabel *m_pLabInfo;

        FloatEdit *m_pdeVelScale;
        FloatEdit *m_pdeRefLength;
        IntEdit *m_pieNw;

        QFrame *m_pFrame;

        static int s_nPoints;
        static double s_VelScale;
        static double s_RefLength;
        static Quaternion s_ab_quat;
};



