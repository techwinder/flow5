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

#pragma once

#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/triangle3d.h>
#include <api/panel3.h>
#include <api/vorton.h>

class IntEdit;
class FloatEdit;
class Vortex;

class gl3dSingularity : public gl3dXflView
{
    Q_OBJECT

    public:
        gl3dSingularity(QWidget *pParent=nullptr);
        ~gl3dSingularity();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const&, Vector3d const &, Vector3d &) override {return false;}
        QSize sizeHint() const override {return QSize(1500, 900);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private slots:
        void onUpdateData();

    private:
        QRadioButton *m_prbSource, *m_prbVortex, *m_prbDoublet;

        IntEdit *m_pieNCircles;
        IntEdit *m_pieNArrows;
        FloatEdit *m_pfeArrowScale;
        FloatEdit *m_pfeInnerRadius;
        FloatEdit *m_pfeProgression;

        QOpenGLBuffer m_vboArrows;

        bool m_bResetArrows;

        Vortex *m_pVortex;

        static int s_NCircles;
        static int s_NArrows;
        static float s_ArrowLength;
        static float s_Radius;
        static float s_Progression;
};

