/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QSplitter>

#include <xflcore/linestyle.h>
#include <xflgraph/containers/graphwt.h>

class Graph;
class GraphWt;
class CPTableView;
class ActionItemModel;
class XflDelegate;

class FastGraphWt : public QFrame
{
    Q_OBJECT
    public:
        FastGraphWt(QWidget *pParent=nullptr);
        ~FastGraphWt();

        QSize sizeHint() const override {return QSize(1100, 900);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pevent) override;

        Graph *graph() {return m_pGraphWt->graph();}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onMakeGraph();
        void onSplitterMoved();
        void onGraphChanged();

    private:
        void setupLayout();
        void connectSignals();

    private:
        GraphWt *m_pGraphWt;
        Graph *m_pGraph;

        CPTableView *m_pcptData;
        ActionItemModel *m_pDataModel;
        XflDelegate *m_pDataDelegate;

        QSplitter *m_pHSplitter;



    private:
        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;

        static LineStyle s_LS[5];
};

