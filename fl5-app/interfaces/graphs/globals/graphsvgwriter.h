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


#include <core/xflsvgwriter.h>

class Graph;
class GraphWt;
struct FontStruct;

class GraphSVGWriter : public XflSvgWriter
{
    public:
        GraphSVGWriter(QFile &XFile);
        void writeGraph(GraphWt const*pGraphWt, Graph const *pGraph);

        static void setFillBackground(bool bFill) {s_bFillBackground=bFill;}
        static bool bFillBackground() {return s_bFillBackground;}

    private:
        void writeXAxis();
        void writeYAxis(int iy);
        void writeXMajGrid();
        void writeYMajGrid(int iy);
        void writeXMinGrid();
        void writeYMinGrid();
        void writeCurve(int nIndex);
        void writeTitles(QRectF const &graphrect, const QColor &clr);
        void writeLegend(QPointF const &Place, const QFont &LegendFont, QColor const &LegendColor);

    private:
        GraphWt const *m_pGraphWt;
        Graph   const *m_pGraph;

        static bool s_bFillBackground;
};

