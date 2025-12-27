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
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

#include <interfaces/editors/fuseedit/fusedlg.h>

#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>

class FuseStl;
class FloatEdit;
class IntEdit;
class PlainTextOutput;

class FuseStlDlg : public FuseDlg
{
    Q_OBJECT

    public:
        FuseStlDlg(QWidget *pParent);
        void initDialog(Fuse *pFuse) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();

        void updateOutput(QString const &msg) override;

        void enableControls(bool bEnable);
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void updateProperties(bool bFull=false) override;

    private slots:
        void onSplitMeshPanels();
        void onRestoreDefaultMesh();

    private:
        QPushButton *m_ppbSplitTriangles, *m_ppbRestoreDefaultMesh;

        FloatEdit *m_pdeMaxEdgeLength;
        IntEdit *m_pieMaxPanelCount;

        PlainTextOutput *m_pptoOutput;

        FuseStl *m_pFuseStl;

        static QByteArray s_Geometry;
};


