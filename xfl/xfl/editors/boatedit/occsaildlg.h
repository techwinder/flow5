/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

