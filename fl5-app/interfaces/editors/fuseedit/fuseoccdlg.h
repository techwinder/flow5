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
#include <QRadioButton>
#include <QAction>
#include <QSplitter>


#include <interfaces/editors/fuseedit/fusedlg.h>


class FuseOcc;
class PlainTextOutput;
class MesherWt;
class GMesherWt;

class FuseOccDlg : public FuseDlg
{
    Q_OBJECT

    public:
        FuseOccDlg(QWidget *pParent);
        ~FuseOccDlg() override;

        void initDialog(Fuse *pFuse) override;
        void connectSignals();

        static bool bfl5Mesher() {return s_bfl5Mesher;}
        static void setfl5Mesher(bool b) {s_bfl5Mesher=b;}

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
        void updateStdOutput(std::string const &strong) override;
        void updateOutput(QString const &strong) override;
        void outputPanelProperties(int panelindex);


    private slots:
        void onCenterViewOnPanel();
        void onCheckFreeEdges();
        void onCheckMesh();
        void onClearHighlighted();
        void onConnectTriangles();
        void onDoubleNodes();
        void onExportBodyToCADFile();
        void onFlipTessNormals();
        void onSelMesher();
        void onShapeFix();
        void onUpdateFuseView();

        void onScale() override;
        void onTranslate() override;
        void onRotate() override;



    private:
        FuseOcc *m_pFuseOcc;

        QSplitter *m_pHSplitter;

        PlainTextOutput *m_ppto;

        QPushButton *m_ppbShapeFix;

        QPushButton *m_ppbCheckMenuBtn;

        QRadioButton *m_prbfl5Mesher, *m_prbGMesher;

        QAction *m_pFlipTessNormals;

        /** @todo merge with fusexfl and maybe planexfl */
        QAction *m_pCheckMesh, *m_pClearHighlighted, *m_pCenterOnPanel;
        QAction *m_pCheckFreeEdges, *m_pCleanDoubleNode, *m_pConnectPanels;
        QAction *m_pRestoreFuseMesh;

        GMesherWt *m_pGMesherWt;
        MesherWt *m_pMesherWt;

        static bool s_bfl5Mesher;

        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;
};

