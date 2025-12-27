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
#include <QCheckBox>
#include <QSlider>
#include <QSplitter>
#include <QSettings>
#include <QDialogButtonBox>

class FuseXfl;
class IntEdit;
class gl3dFuseView;
class gl3dGeomControls;

class FlatFaceConverterDlg : public QDialog
{
    Q_OBJECT

    public:
        FlatFaceConverterDlg(QWidget*pWidget);
        ~FlatFaceConverterDlg();

        void initDialog(FuseXfl const *pFuseXfl);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        FuseXfl *flatFaceFuse() const {return m_pFlatFaceFuse;}

        int nx() const {return s_Nx;}
        int nh() const {return s_Nh;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onFlatFace();

    private:
        FuseXfl const *m_pFuseXfl;
        FuseXfl *m_pFlatFaceFuse;

        gl3dFuseView *m_pglFuseView;
        gl3dGeomControls *m_pglControls;

        IntEdit *m_pieFlatFaceNx, *m_pieFlatFaceNh;
        QSlider *m_pslBunchDist, *m_pslBunchAmp;


        QDialogButtonBox *m_pButtonBox;
        QSplitter *m_pHSplitter;

        static int s_Nx, s_Nh;
        static double s_BunchDist, s_BunchAmp;

        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;
};

