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

#include <QDataStream>

#include <TopoDS_Edge.hxx>

#include <mathelem.h>
#include <node.h>

#include <fl5lib_global.h>


class FL5LIB_EXPORT EdgeSplit
{
    public:
        EdgeSplit();

        int nSegs() const {return m_nSegs;}
        xfl::enumDistribution distrib() const {return m_Distrib;}
        std::vector<double> const &split() const {return m_Split;}

        void setSplit(int n, xfl::enumDistribution dist) {m_nSegs=n; m_Distrib=dist;}
        void setNSegs(int n) {m_nSegs=n;}
        void setDistrib(xfl::enumDistribution dist) {m_Distrib=dist;}


        void makeSplit(TopoDS_Edge const &edge);
        void makeUniformSplit(TopoDS_Edge const &Edge, double maxlength);
        void makeNodes(TopoDS_Edge const &Edge, std::vector<Node> &nodes);

        void serialize(QDataStream &ar, bool bIsStoring);

    private:
        int m_nSegs; /**< the number of triangles on the edge = 1 less than the number of vertices */
        xfl::enumDistribution m_Distrib; /**< the type of vertex distribution on this edge */
        std::vector<double> m_Split; /**< edge parameters in p-space */
};

