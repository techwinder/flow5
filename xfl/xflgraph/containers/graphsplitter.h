/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QSplitter>



class Graph;
class GraphWt;
class CurveModel;

class GraphSplitter : public QSplitter
{
    Q_OBJECT

    public:
        GraphSplitter(QVector<Graph*> &graphlist, QWidget *parent = nullptr);

        GraphWt *graphWt(int ig) {return m_GraphWt.at(ig);}
        QSize sizeHint() const override {return QSize(1100,900);}

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;

    public slots:
        void onTopBLSplitterMoved();
        void onBotBLSplitterMoved();
        void onResetSplitters();

    signals:
        void graphChanged(int iGraph);

    private:
        void setupLayout();

    private:

        QVector<Graph*>   *m_pGraphList;
        QVector<GraphWt*> m_GraphWt;

        QSplitter *m_pTopSplitter, *m_pBotSplitter;
};

