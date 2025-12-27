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

#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>

#include <edgesplit.h>

#include <occ_globals.h>

EdgeSplit::EdgeSplit()
{
    m_nSegs=3;
    m_Distrib = xfl::UNIFORM;
    xfl::getPointDistribution(m_Split, m_nSegs, m_Distrib);
}


/** Finds the p-space parameters so that the points are spaced in g-space according to the specified distribution */
void EdgeSplit::makeSplit(TopoDS_Edge const &edge)
{
    if(m_nSegs<0) return;

    m_Split.resize(m_nSegs+1);
    std::vector<double> psplit;
    xfl::getPointDistribution(psplit, m_nSegs, distrib());
    double l = occ::edgeLength(edge);

    // split in g-space rather than p-space
    BRepAdaptor_Curve CA;
    CA.Initialize(edge);
    double u0 = CA.FirstParameter();
    double u1 = CA.LastParameter();

    m_Split.front() = u0;
    m_Split.back()  = u1;

    // proceed using dichotomy
    double v0=0, v1=0;
    double tl=0;
    double eps = 0.0001;
    double length=0;
    gp_Pnt P0, P1;
    int iter=0;
    for(int i=1; i<m_nSegs; i++)
    {
        tl = l * psplit.at(i); // target length
        v0 = u0;
        v1 = u1;
        length = l;
        CA.D0(v0, P0);
        CA.D0(v1, P1);
        iter = 0;
        do
        {
            length = occ::edgeLength(edge, u0, (v0+v1)/2.0);
            if(length<tl)
            {
                v0 = (v0+v1)/2.0;
            }
            else
            {
                v1 = (v0+v1)/2.0;
            }
        }
        while(fabs(length-tl)>eps && iter++<20);
        m_Split[i] = (v0+v1)/2.0;
    }
}


void EdgeSplit::makeUniformSplit(TopoDS_Edge const &Edge, double maxlength)
{
    m_Split.clear();
    double length = occ::edgeLength(Edge);
    int nSegs = std::max(1, int(std::round(length/maxlength)));

    BRepAdaptor_Curve curveAdaptor;
    curveAdaptor.Initialize(Edge);

    GCPnts_UniformAbscissa uniformAbscissa;
    uniformAbscissa.Initialize(curveAdaptor, nSegs+1);

    if(uniformAbscissa.IsDone())
    {
        int NPoints = uniformAbscissa.NbPoints();
        for(int i=1; i<=NPoints; i++)
        {
            double param = uniformAbscissa.Parameter(i);
            m_Split.push_back(param);
        }
    }

    if(m_Split.size()<2)
    {
        // NPoints may be 1 if nSegs=1
        m_Split.resize(2);
        m_Split.front() = curveAdaptor.FirstParameter();
        m_Split.back()  = curveAdaptor.LastParameter();
    }
}


void EdgeSplit::makeNodes(TopoDS_Edge const &Edge, std::vector<Node> &nodes)
{
    BRepAdaptor_Curve CA;
    CA.Initialize(Edge);

    gp_Pnt pt;
    for(uint i=0; i<m_Split.size(); i++)
    {
        CA.D0(m_Split.at(i), pt);
        nodes.push_back({pt.X(), pt.Y(), pt.Z()});
    }
}


void EdgeSplit::serialize(QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring)
    {
        ar << m_nSegs;
        switch(m_Distrib)
        {
            default:
            case xfl::UNIFORM:    ar<<0;   break;
            case xfl::COSINE:     ar<<1;   break;
            case xfl::SINE:       ar<<2;   break;
            case xfl::INV_SINE:   ar<<3;   break;
            case xfl::INV_SINH:   ar<<4;   break;
            case xfl::TANH:       ar<<5;   break;
            case xfl::EXP:        ar<<6;   break;
            case xfl::INV_EXP:    ar<<7;   break;
        }
    }
    else
    {
        ar >> m_nSegs;
        if(m_nSegs<0) m_nSegs=3; // cleaning up pas errors
        int k=0;
        ar>>k;
        switch (k)
        {
            case 0:   m_Distrib=xfl::UNIFORM;    break;
            case 1:   m_Distrib=xfl::COSINE;     break;
            case 2:   m_Distrib=xfl::SINE;       break;
            case 3:   m_Distrib=xfl::INV_SINE;   break;
            case 4:   m_Distrib=xfl::INV_SINH;   break;
            case 5:   m_Distrib=xfl::TANH;       break;
            case 6:   m_Distrib=xfl::EXP;        break;
            case 7:   m_Distrib=xfl::INV_EXP;    break;
        }
    }
}


