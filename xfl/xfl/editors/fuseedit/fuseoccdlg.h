/****************************************************************************

    flow5
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

#include <QAction>
#include <QSplitter>


#include <xfl/editors/fuseedit/fusedlg.h>


class FuseOcc;
class PlainTextOutput;
class MesherWt;

class FuseOccDlg : public FuseDlg
{
    Q_OBJECT

    public:
        FuseOccDlg(QWidget *pParent);
        ~FuseOccDlg() override;

        void initDialog(Fuse *pFuse) override;
        void connectSignals();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void showEvent(QShowEvent *pEvent) override;
        void updateProperties(bool bFull=false) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

    private:
        void createActions();
        void setupLayout();
        void updateOutput(QString const &strong) override;
        void outputPanelProperties(int panelindex);


    private slots:
        void onExportBodyToCADFile();
        void onUpdateFuseView();
        void onShapeFix();

        void onConnectTriangles();
        void onCheckFreeEdges();
        void onClearHighlighted();
        void onDoubleNodes();
        void onCheckMesh();
        void onCenterViewOnPanel();
        void onFlipTessNormals();

        void onScale() override;
        void onTranslate() override;
        void onRotate() override;



    private:
        FuseOcc *m_pFuseOcc;

        QSplitter *m_pHSplitter;

        PlainTextOutput *m_pptoOutput;

        QPushButton *m_ppbShapeFix;

        QPushButton *m_ppbCheckMenuBtn;


        QAction *m_pFlipTessNormals;

        /** @todo merge with fusexfl and maybe planexfl */
        QAction *m_pCheckMesh, *m_pClearHighlighted, *m_pCenterOnPanel;
        QAction *m_pCheckFreeEdges, *m_pCleanDoubleNode, *m_pConnectPanels;
        QAction *m_pRestoreFuseMesh;


        MesherWt *m_pFuseMesherWt;

        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;
};

