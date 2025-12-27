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

#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/panel4.h>
#include <interfaces/opengl/controls/colourlegend.h>


class IntEdit;
class FloatEdit;

class gl3dQuadField : public gl3dXflView
{
    Q_OBJECT
    public:
        gl3dQuadField(QWidget *pParent = nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;

        void glRenderView() override;
        void glMake3dObjects() override;

        bool intersectTheObject(Vector3d const&, Vector3d const&, Vector3d &) override {return false;}

        QSize sizeHint() const override {return QSize(1100,1000);}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:

        Vector3d velocity(Vector3d const &pt);
        void glMakeForces(QVector<Vector3d> const &nodes, const QVector<Vector3d> &Vectors, float scalef, QOpenGLBuffer &vbo);
        void makeColorRow(double t, double w, int ir, int iPlaneDir);
        void makeColorValues();
        void makeControls();
        void makeVelocityVectors();
        void readData();
        void resetQuad() {m_bResetQuad=true;}


    private slots:
        void onRefLength();
        void onVelScale();
        void onMakeView();
        void onPlanePosition(int pos);
        void onContourPlane(bool bChecked);
        void onLegendScale();

    private:
        bool m_bResetQuad;
        bool m_bResetContour;

        double m_ul;
        double m_wl;

        QVector<Vector3d> m_PointArray; // Size x Size x Size
        QVector<Vector3d> m_VectorArray; // Size x Size x Size
        Panel4 m_p4;

        QOpenGLBuffer m_vboForces;
        QOpenGLBuffer m_vboVectors;
        QOpenGLBuffer m_vboTriangles;
        QOpenGLBuffer m_vboBackgroundQuad;

        QOpenGLBuffer m_vboClrMap;
        QOpenGLBuffer m_vboContours;

        QFrame *m_pfrControls;

        QRadioButton *m_prbSource, *m_prbDoublet, *m_prbVortex;

        FloatEdit *m_pdeVelScale;
        FloatEdit *m_pdeRefLength;
        IntEdit *m_pieNu, *m_pieNw;

        QCheckBox *m_pchContourPlane;
        QComboBox *m_pcbPlaneDir;
        QCheckBox *m_pchAutoScale;
        FloatEdit *m_pdeVMin, *m_pdeVMax;

        QCheckBox *m_pchVectors;
        QComboBox *m_pcbVelDir;
        QSlider *m_pslPlanePos;

        ColourLegend m_LegendOverlay;

        //colormap and iso contours
        int m_nRows, m_nCols;
        QVector<Vector3d> m_Nodes;
        QVector<double> m_Values;

        static bool s_bShowVectors;
        static bool s_bShowPlane;
        static int s_iSource;
        static int s_iVelDir;
        static int s_iPlaneDir;
        static double s_PlanePos;
        static int s_Size_u, s_Size_w;
        static double s_VelScale;
        static double s_RefLength;

        static Quaternion s_ab_quat;
        static QByteArray s_Geometry;
};

