/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QMap>

#include <xflcore/linestyle.h>
#include <xflcore/fontstruct.h>


class Graph;
class MainFrame;
class XPlane;
class XDirect;
class LegendBtn;
class XflObject;
class Curve;



class LegendWt : public QWidget
{
    Q_OBJECT

    public:
        LegendWt(QWidget *pParent = nullptr);


        void setGraph(Graph*pGraph){m_pGraph = pGraph;}

        virtual void makeLegend(bool bHighlight);



    protected:
        void makeGraphLegendBtns(bool bHighlight);

    private slots:
        void onClickedCurveBtn();
        void onRightClickedCurveBtn(LineStyle);
        void onClickedCurveLine(LineStyle ls);

        void onDeleteActiveCurve();
        void onRenameActiveCurve();

    signals:
        void updateGraphs();

    protected:
        QMap<LegendBtn*, XflObject *> m_XflObjectMap;
        QMap<LegendBtn*, Curve*> m_CurveMap;
        Graph *m_pGraph;
        Curve *m_pActiveCurve;

};


