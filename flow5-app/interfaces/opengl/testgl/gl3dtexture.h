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

#include <QPushButton>
#include <QCheckBox>
#include<QLabel>
#include <QSlider>

#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>

class Triangle3d;
class IntEdit;
class FloatEdit;

class gl3dTexture : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dTexture(QWidget *pParent = nullptr);
        ~gl3dTexture();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;
        void initializeGL() override;
        void glMake3dObjects() override;
        void glRenderView() override;
        void glMakeTexSphere(int nSplits);
        void glMakeTexSphere(int nLong, int nLat);
        void cartesianToSpherical(Vector3d const &position, float &theta, float &phi);


        void updateBtns(bool bStart);

        void runAttractor(QWidget *pParent);

        void processImgBlock(int rf, int rl);

        double fx(double x, double y) const;
        double fy(double x, double y) const;

    private slots:
        void onContinue();
        void onClear();
        void updateImg();
        void onReadParams();
        void onResizeImage();
        void onOpenImg();
        void onSaveImg();
        void onTaskFinished();

    signals:
        void taskFinished();


    private:

        QImage *m_pImg;

        QOpenGLTexture *m_pglTexture = nullptr;
        QOpenGLBuffer m_vboTexSphere;

        bool m_bResetTexture;

        bool m_bCancel = true;
        bool m_bIsRunning = false;
        bool m_bUpdating;
        int m_NSteps = 0;
        QVector<ushort> m_Occupancy;

        QPushButton *m_ppbSaveImg;
        QPushButton *m_ppbStart, *m_ppbClear;
        IntEdit *m_pieWidth, *m_pieHeight;
        FloatEdit *m_pfeTimeOut, *m_pfeUpdate;
        QLabel *m_plabMaxOcc;
        IntEdit *m_pieMaxOcc;
        FloatEdit *m_pfea, *m_pfeb;
        FloatEdit *m_pfec, *m_pfed;

        QSlider *m_pslRed, *m_pslGreen, *m_pslBlue;
        QLabel *m_plabInfo;

        QString s_LastFileName;


        static float s_red, s_green, s_blue;
        static double s_a, s_b;
        static double s_c, s_d;

        static float s_TimeOut, s_UpdatePeriod;
        static int s_MaxOccupancy;

        static QSize s_ImgSize;
        static QByteArray s_Geometry;

};

#include <QEvent>

const QEvent::Type OCCUPANCY_EVENT         = static_cast<QEvent::Type>(QEvent::User + 201);

class OccupancyEvent : public QEvent
{
    public:
        OccupancyEvent(): QEvent(OCCUPANCY_EVENT)
        {
        }

        void setElapsed(float t) {m_ElapsedTime = t;}
        float elapsed() const {return m_ElapsedTime;}

        void setTau(QVector<ushort> const &states)    {m_Occupancy=states;}

        QVector<ushort> const & newStates() const {return m_Occupancy;}

        void setFinished() {m_bIsFinished=true;}
        bool isFinished() const {return m_bIsFinished;}

    private:

        QVector<ushort> m_Occupancy;
        bool m_bIsFinished = false;
        float m_ElapsedTime = 0; //s
};
