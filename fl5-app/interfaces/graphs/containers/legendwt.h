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

#include <QWidget>
#include <QMap>

#include <api/linestyle.h>
#include <core/fontstruct.h>


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


