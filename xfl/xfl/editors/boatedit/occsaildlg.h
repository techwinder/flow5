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

#include <xfl/editors/boatedit/externalsaildlg.h>

class MesherWt;

class OccSailDlg: public ExternalSailDlg
{
    Q_OBJECT
    public:
        OccSailDlg(QWidget *pParent);
        void initDialog(Sail *pSail) override;
        void customEvent(QEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    private:
        void setupLayout();
        void connectSignals();

        void initMesher();
        void updateEdgeNodes();
        void makeEdgeNodes(QVector<Node> &nodes) override;

    private slots:
        void onUpdateSailView();
        void onFlipTessNormals();
        void onTessellation();
        void onShapes();
        void onTabChanged(int);

        void onPickEdge(bool bPick) override;
        void onPickedEdge(int iFace, int iEdge) override;
        void onMakeEdgeSplits() override;

    private:

        MesherWt *m_pMesherWt;
        QAction *m_pTessSettings, *m_pFlipTessNormals;

};

