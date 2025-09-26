/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xflcore/xflsvgwriter.h>

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

