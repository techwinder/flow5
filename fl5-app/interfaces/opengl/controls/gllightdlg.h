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

#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QRadioButton>
#include <QSettings>

#include <interfaces/opengl/views/light.h>

class gl3dView;
class ExponentialSlider;
class FloatEdit;


class GLLightDlg : public QDialog
{
    Q_OBJECT

    public:
        GLLightDlg(QWidget *pParent=nullptr);
        void apply();
        void readParams(void);
        void setDefaults();
        void setParams(void);
        void setgl3dView(gl3dView*pglView);

        QSize sizeHint() const override {return QSize(475, 475);}

        static double verticalAngle() {return s_VerticalAngle;}
        static double viewDistance()  {return s_ViewDistance;}
        static bool   isOrtho()       {return s_bOrtho;}
        static double refLength()     {return s_RefLength;}

        static void setVerticalAngle(double theta) {s_VerticalAngle=theta;}
        static void setViewDistance(double l)      {s_ViewDistance=l;}
        static void setOrtho(bool b)               {s_bOrtho=b;}
        static void setRefLength(double l)         {s_RefLength=l;}


        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);


    private:
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void setEnabled();
        void setLabels();

    public slots:
        void onChanged();
        void onDefaults();
        void onLight();
        void onViewProjection();
        void onButton(QAbstractButton *pButton);

    private:
        QSlider *m_pslRed, *m_pslGreen, *m_pslBlue;
        QSlider *m_pslMatShininess;
        ExponentialSlider *m_peslLightAmbient, *m_peslLightDiffuse, *m_peslLightSpecular;
        ExponentialSlider *m_peslXLight, *m_peslYLight, *m_peslZLight;
        ExponentialSlider *m_peslEyePos;

        QCheckBox *m_pchLight;
        QLabel *m_plabLightAmbient, *m_plabLightDiffuse, *m_plabLightSpecular;
        QLabel *m_plabLightRed, *m_plabLightGreen, *m_plabLightBlue;
        QLabel *m_plabMatShininess;
        QLabel *m_plabposXValue, *m_plabposYValue, *m_plabposZValue;
        QLabel *m_plabEyeDist;
        QDialogButtonBox *m_pButtonBox;

        FloatEdit *m_pdeConstantAttenuation , *m_pdeLinearAttenuation , *m_pdeQuadAttenuation ;

        QRadioButton *m_prbOrtho, *m_prbPerspective;
        FloatEdit *m_pdeVerticalAngle, *m_pdeViewDistance;

    private:
        gl3dView *m_pglView;

        static double s_VerticalAngle;
        static double s_ViewDistance;
        static bool s_bOrtho;

        static double s_RefLength;

        static QByteArray s_Geometry;
};




