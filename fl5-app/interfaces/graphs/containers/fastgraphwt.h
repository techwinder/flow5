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

#include <QSplitter>

#include <api/linestyle.h>
#include <interfaces/graphs/containers/graphwt.h>

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

