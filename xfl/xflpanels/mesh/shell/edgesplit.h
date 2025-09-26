/****************************************************************************

  flow5 application
  Copyright (C) Andre Deperrois 
  All rights reserved.

*****************************************************************************/

#pragma once

#include <TopoDS_Edge.hxx>

#include <xflmath/mathelem.h>
#include <xflgeom/geom3d/node.h>


class EdgeSplit
{
    public:
        EdgeSplit();

        int nSegs() const {return m_nSegs;}
        xfl::enumDistribution distrib() const {return m_Distrib;}
        QVector<double> const &split() const {return m_Split;}

        void setSplit(int n, xfl::enumDistribution dist) {m_nSegs=n; m_Distrib=dist;}
        void setNSegs(int n) {m_nSegs=n;}
        void setDistrib(xfl::enumDistribution dist) {m_Distrib=dist;}


        void makeSplit(TopoDS_Edge const &edge);
        void makeUniformSplit(TopoDS_Edge const &Edge, double maxlength);
        void makeNodes(TopoDS_Edge const &Edge, QVector<Node> &nodes);

        void serialize(QDataStream &ar, bool bIsStoring);

    private:
        int m_nSegs; /**< the number of triangles on the edge = 1 less than the number of vertices */
        xfl::enumDistribution m_Distrib; /**< the type of vertex distribution on this edge */
        QVector<double> m_Split; /**< edge parameters in p-space */
};

