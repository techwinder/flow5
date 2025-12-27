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

#include <QDataStream>


#include <spline.h>


#include <mathelem.h>


Spline::Spline()
{
    m_theStyle = LineStyle(true, Line::SOLID, 2, {255,0,0}, Line::NOSYMBOL);

    m_bSingular = true; // until spline is made

    m_CtrlPt.clear();
    m_Weight.clear();
    m_iHighlight  = -10;
    m_iSelect     = -10;

    m_Output.clear();
    m_Output.resize(279); // even numbers are evil, odd numbers are acceptable, prime numbers are perfect

    m_bIsModified = false;

    m_bShowCtrlPts = true;
    m_bShowNormals = false;

    m_bClosed = false;
    m_bForcesymmetric = false;

    m_BunchType = NOBUNCH;
    m_BunchAmp = 0.0;
}


Spline::~Spline()
{
}


void Spline::makeDefaultControlPoints(bool bClosed, bool bTopHalfOnly)
{
    m_CtrlPt.clear();

    m_CtrlPt.push_back({1.0,0.0314});
    m_CtrlPt.push_back({0.5,0.09});
    m_CtrlPt.push_back({0.15,0.075});
    m_CtrlPt.push_back({0.0,0.0});
    if(!bTopHalfOnly)
    {
        m_CtrlPt.push_back({0.15,-0.075});
        m_CtrlPt.push_back({0.5,-0.09});
        m_CtrlPt.push_back({1.0,-0.0314});
        if(bClosed)
        {
            m_CtrlPt.front().y = 0;
            m_CtrlPt.back().y  = 0;
        }
    }

    m_Weight.clear();
    for(uint i=0; i<m_CtrlPt.size(); i++)
        m_Weight.push_back(1.0);
}


void Spline::duplicate(const Spline &spline)
{
    m_CtrlPt = spline.m_CtrlPt;
    m_Weight = spline.m_Weight;

    m_iHighlight  = spline.m_iHighlight;
    m_iSelect     = spline.m_iSelect;

    m_theStyle = spline.m_theStyle;

    m_bShowNormals = spline.m_bShowNormals;
    m_bSingular = spline.m_bSingular;
    m_bClosed = spline.m_bClosed;

    m_BunchType    = spline.m_BunchType;
    m_BunchAmp     = spline.m_BunchAmp;

}


void Spline::resetSpline()
{
    clearControlPoints();
    updateSpline();
}


void Spline::copysymmetric(Spline const &spline)
{
    m_CtrlPt.clear();
    for(uint ic=0; ic<spline.m_CtrlPt.size(); ic++)
    {
        m_CtrlPt.push_back(spline.m_CtrlPt.at(ic));
        m_CtrlPt[ic].y = -m_CtrlPt[ic].y;
    }
    m_Weight = spline.m_Weight;

    m_iHighlight  = spline.m_iHighlight;
    m_iSelect     = spline.m_iSelect;
}


double Spline::getY(double xinterp, bool bRelative) const
{
    if(bRelative && (xinterp<=0.0 || xinterp>=1.0)) return 0.0;
    double x = xinterp;
    if(bRelative) x = m_Output.front().x + xinterp*(m_Output.back().x - m_Output.front().x);

    for (int k=0; k<outputSize()-1; k++)
    {
        if (m_Output.at(k).x<m_Output.at(k+1).x  && m_Output.at(k).x<=x && x<=m_Output.at(k+1).x )
        {
            double y = (m_Output.at(k).y + (m_Output.at(k+1).y-m_Output.at(k).y)
                                          /(m_Output.at(k+1).x-m_Output.at(k).x)*(x-m_Output.at(k).x));
            return y;
        }
    }
    return 0.0;
}


void Spline::appendControlPoint(double x, double y, double w)
{
    appendControlPoint(Node2d(x,y),w);
}


void Spline::appendControlPoint(Node2d const &Pt, double w)
{
    m_CtrlPt.push_back(Pt);
    m_Weight.push_back(w);

    if(m_bForcesymmetric)
    {
        if(m_CtrlPt.size()>0) m_CtrlPt.front().y = m_CtrlPt.back().y =0;
    }

    setModified(true);
}


void Spline::resizeControlPoints(int nPts)
{
    if(nPts==int(m_CtrlPt.size())) return; // nothing to do
    //erase all points in between
    for(int ic=nPts-2; ic>0; ic--)
    {
        removeCtrlPoint(ic);
    }

    int nnew = nPts-2;
    for(int j=0; j<nnew; j++)
    {
        double tau = double(j+1)/double(nnew+1);
        Vector2d point = m_CtrlPt.front() + (m_CtrlPt.back()-m_CtrlPt.front())*tau;
        m_CtrlPt.insert(m_CtrlPt.begin()+j+1, point);
        double w = m_Weight.front() + tau*(m_Weight.back()-m_Weight.front());
        m_Weight.insert(m_Weight.begin()+j+1, w);
    }
    updateSpline();
}


void Spline::setCtrlPoints(std::vector<Node2d> const & ptList, double w)
{
    m_CtrlPt = ptList;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    m_Weight.resize(ptList.size());
    m_Weight.fill(w);
#else
    m_Weight.resize(ptList.size(), w);
#endif

    setModified(true);
}


void Spline::appendCtrlPoints(std::vector<Node2d> const & ptList, double w)
{
    m_CtrlPt.insert(m_CtrlPt.end(), ptList.begin(), ptList.end());
//    m_CtrlPt.push_back(ptList);
    for(uint i=0; i<ptList.size(); i++) m_Weight.push_back(w);

    setModified(true);
}


void Spline::appendCtrlPoints(std::vector<Node2d> const & ptList, std::vector<double> weightlist)
{
    m_CtrlPt.insert(m_CtrlPt.end(), ptList.begin(), ptList.end());
    m_Weight.insert(m_Weight.end(), weightlist.begin(), weightlist.end());
//    m_CtrlPt.push_back(ptList);
//    m_Weight.push_back(weightlist);

    setModified(true);
}


void Spline::insertCtrlPointAt(int iSel, double x, double y, double w)
{
    insertCtrlPointAt(iSel, Vector2d(x,y), w);
}


void Spline::insertCtrlPointAt(int iSel, const Node2d &pt, double w)
{
    m_CtrlPt.insert(m_CtrlPt.begin()+iSel, pt);
    m_Weight.insert(m_Weight.begin()+iSel, w);
}


void Spline::insertCtrlPointAt(int iSel)
{
    m_CtrlPt.insert(m_CtrlPt.begin()+iSel, Vector2d());
    m_Weight.insert(m_Weight.begin()+iSel, 1.0);

    if(iSel>0 && iSel<int(m_CtrlPt.size())-1)
    {
        m_CtrlPt[iSel] = (m_CtrlPt[iSel-1]+m_CtrlPt[iSel+1])/2.0;
        m_Weight[iSel] = (m_Weight[iSel-1]+m_Weight[iSel+1])/2.0;
    }
    else if (iSel==0)
    {
        m_CtrlPt[iSel] = m_CtrlPt[1]*0.95;
        m_Weight[iSel] = m_Weight[1];
    }
    else if (iSel==int(m_CtrlPt.size())-1)
    {
        m_CtrlPt[iSel] = m_CtrlPt[iSel-1]*1.05;
        m_Weight[iSel] = m_Weight[iSel-1];
    }
}


/**
 * Inserts a new point in the array of control points;
 * searches for the two closest adjacent points and inserts the new point in between
 * @param x the x-coordinate of the point to insert
 * @param y the y-coordinate of the point to insert
 * @return true unless the max number of points has been reached
 */
bool Spline::insertCtrlPoint(double x, double y, double w)
{
    int kc = 0;
    double dmax2=1.e10;
    double d1(0), d2(0);
    if(m_CtrlPt.size()<=1) return false;
    for (uint k=0; k<m_CtrlPt.size()-1; k++)
    {
        Node2d const &p1 = m_CtrlPt.at(k);
        Node2d const &p2 = m_CtrlPt.at(k+1);
        d1 = (p1.x-x)*(p1.x-x) + (p1.y-y)*(p1.y-y);
        d2 = (p2.x-x)*(p2.x-x) + (p2.y-y)*(p2.y-y);
        if(d1+d2<dmax2)
        {
            dmax2 = d1+d2;
            kc=k;
        }
    }
    m_CtrlPt.insert(m_CtrlPt.begin()+kc+1, Node2d(x,y));
    m_Weight.insert(m_Weight.begin()+kc+1, w);
    m_iSelect = kc+1;

    setModified(true);
    return true;
}


/**
* Removes a point from the array of control points, only if the remaining number of points is strictly greater than the spline's degree
* @param k the index of the control point to remove in the array
* @return false if the remaining number of points is equal or less than the spline's degree, true otherwise.
*/
bool Spline::removeCtrlPoint(int k)
{
    if (k>=0 && k<int(m_CtrlPt.size()))
    {
        m_CtrlPt.erase(m_CtrlPt.begin()+k);
        m_Weight.erase(m_Weight.begin()+k);
        setModified(true);
        return true;
    }
    return false;
}


/**
* Checks if an input point matches a control point
* @param Real the input point to compare with the control points
* @param zx the scaling factor of the x-scale, to withdraw from the input point @todo withdrawal to be performed from within the calling function.
* @param zy the scaling factor of the y-scale, to withdraw from the input point @todo withdrawal to be performed from within the calling function.
* @return the index of the first control point which matches, or -10 if none matches.
*/
int Spline::isCtrlPoint(double x, double y, double tolx, double toly) const
{
    for (int k=0; k<int(m_CtrlPt.size()); k++)
    {
        if(fabs(x-m_CtrlPt.at(k).x)<tolx && fabs(y-m_CtrlPt.at(k).y)<toly) return int(k);
    }
    return -10;
}


double Spline::xMin() const
{
    double xmin = 1.e10;
    for(int io=0; io<int(m_Output.size()); io++)
    {
        xmin=std::min(m_Output.at(io).x, xmin);
    }
    return xmin;
}


double Spline::xMax() const
{
    double xmax = -1.e10;
    for(int io=0; io<int(m_Output.size()); io++)
    {
        xmax=std::max(m_Output.at(io).x, xmax);
    }
    return xmax;
}


double Spline::yMin() const
{
    double ymin = 1.e10;
    for(int io=0; io<int(m_Output.size()); io++)
    {
        ymin=std::min(m_Output.at(io).y, ymin);
    }
    return ymin;
}


double Spline::yMax() const
{
    double ymax = -1.e10;
    for(int io=0; io<int(m_Output.size()); io++)
    {
        ymax=std::max(m_Output.at(io).y, ymax);
    }
    return ymax;
}


bool Spline::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int n(0), k(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    double dble(0);
    double x(0), y(0), w(0);
    int ArchiveFormat = 500001;
    // 500001 : first version of the new fl5 format

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        m_theStyle.serializeFl5(ar, bIsStoring);

        ar << m_bShowNormals;
        ar << int(m_Output.size());

        ar << int(m_CtrlPt.size());
        for (uint j=0; j<m_CtrlPt.size(); j++)
        {
            ar << m_CtrlPt.at(j).x << m_CtrlPt.at(j).y;
        }

        ar << int(m_Weight.size());
        for (uint j=0; j<m_Weight.size(); j++)
        {
            ar << m_Weight.at(j);
        }

        ar << m_bClosed;
        ar << m_bForcesymmetric;

        ar << m_BunchAmp;

        switch(m_BunchType)
        {
            default:
            case NOBUNCH:    n=0;        break;
            case UNIFORM:    n=1;        break;
            case SIGMOID:    n=2;        break;
            case DOUBLESIG:  n=3;        break;
        }
        ar << n << k; // ar << m_BunchDistrib;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        m_theStyle.serializeFl5(ar, bIsStoring);

        ar >> m_bShowNormals;
        ar >> n;  setOutputSize(n);

        ar >> n;
        m_CtrlPt.clear();
        for (int j=0; j<n; j++)
        {
            ar>>x>>y;
            m_CtrlPt.push_back(Node2d(x,y));
        }

        ar >> n;
        m_Weight.clear();
        for (int j=0; j<n; j++)
        {
            ar>>w;
            m_Weight.push_back(w);
        }

        ar >> m_bClosed;
        ar >> m_bForcesymmetric;

        ar >> m_BunchAmp;

        ar >> n  >> k;        // ar >> m_BunchDistrib;
        switch(n)
        {
            default:
            case 0: m_BunchType = NOBUNCH;    break;
            case 1: m_BunchType = UNIFORM;    break;
            case 2: m_BunchType = SIGMOID;    break;
            case 3: m_BunchType = DOUBLESIG;  break;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


void Spline::setPointWeight(int p, double w)
{
    if(p<int(m_Weight.size()))
    {
        m_Weight[p] = w;
        setModified(true);
    }
}


void Spline::setUniformWeights()
{
    m_Weight.resize(ctrlPointCount());
    for(int i=0; i<int(m_Weight.size()); i++) m_Weight[i]=1.0;
}



double Spline::closest(double xin, double yin, float precision) const
{
    //    int nsplit = m_Output.size();
    int nsplit = 300; // need to refine to differentiate top and bottom surfaces near the TE
    // find the two adjacent points
    double dist(0);
    double u(0);
    double dmax=1.e10;

    Vector2d pt;

    float umin=0.0;
    float umax=1.0;
    double uc = umin;
    double dt = (umax-umin)/double(nsplit-1);

    // refine progressively the interval
    int iter = 0;
    bool bImproved = true;
    do
    {
        u = umin;
        bImproved = false;
        for(int i=0; i<nsplit; i++)
        {
            pt = splinePoint(u);
            dist = sqrt((xin-pt.x)*(xin-pt.x)+(yin-pt.y)*(yin-pt.y));
            if(dist<dmax)
            {
                uc=u;
                dmax = dist;
                bImproved = true;
                if(dist<=precision) break;
            }
            u+=dt;
        }

        if(fabs(uc)<precision)     return 0.0;
        if(fabs(uc-1.0)<precision) return 1.0;

        // continue search around the best point
        umin = uc * 0.8;
        umax = uc * 1.2;
        dt =(umax-umin)/double(nsplit-1);
        iter++;
    }
    while(dist>precision && bImproved && iter<50);
    return uc;
}


void Spline::translate(double tx, double ty)
{
    for(int i=0; i<int(m_CtrlPt.size()); i++)
    {
        m_CtrlPt[i].translate(tx,ty);
    }
}


void Spline::setClosed(bool bClosed)
{
    m_bClosed=bClosed;
    if(m_bClosed) m_bForcesymmetric = false;
    if(bClosed)
    {
        if(m_CtrlPt.size()>0) m_CtrlPt.back() = m_CtrlPt.front();
        updateSpline();
        makeCurve();
    }
}


void Spline::setForcedsymmetric(bool bSym)
{
    m_bForcesymmetric = bSym;
    if(m_bForcesymmetric) m_bClosed = false;
    if(bSym)
    {
        if(m_CtrlPt.size()>0) m_CtrlPt.front().y = m_CtrlPt.back().y =0;
        updateSpline();
        makeCurve();
    }
}


#define RES 29
/**
 * Returns the approximate spline length from parameter t0 to t1
 */
double Spline::length(double t0, double t1) const
{
    double l=0;
    double u0=t0;
    double u1=0.0;
    Vector2d p0, p1;
    p0 = splinePoint(u0);
    for(int it=1; it<RES; it++)
    {
        u1 = t0 + double(it)/double(RES-1)*(t1-t0);
        p1 = splinePoint(u1);
        l += p0.distanceTo(p1);
//        u0 = u1;
    }
    return l;
}


void Spline::makeSplit(int m_nSegs, xfl::enumDistribution distrib, std::vector<double> &split) const
{
    if(m_nSegs<0) return;

    split.resize(m_nSegs+1);
    std::vector<double> psplit;
    xfl::getPointDistribution(psplit, m_nSegs, distrib);
    double l = length(0.0, 1.0);

    // split in g-space rather than p-space

    double u0 = 0.0;
    double u1 = 1.0;

    split.front() = u0;
    split.back()  = u1;

    // proceed using dichotomy
    double v0=0, v1=0;
    double tl=0;
    double eps = 0.0001;
    double seglength=0;
    Vector2d p0, p1;

    int iter=0;
    for(int i=1; i<m_nSegs; i++)
    {
        tl = l * psplit.at(i); // target length
        v0 = u0;
        v1 = u1;

//        p0 = splinePoint(v0);
//        p1 = splinePoint(v1);
        iter = 0;
        do
        {
            seglength = length(u0, (v0+v1)/2.0);
            if(seglength<tl)
            {
                v0 = (v0+v1)/2.0;
            }
            else
            {
                v1 = (v0+v1)/2.0;
            }
        }
        while(fabs(seglength-tl)>eps && iter++<20);
        split[i] = (v0+v1)/2.0;
    }
}


void Spline::scale(double ratio)
{
    for(int i=0; i<nCtrlPoints(); i++)
    {
        m_CtrlPt[i] *= ratio;
    }
    updateSpline();
    makeCurve();
}


Vector2d Spline::splineNormal(double u) const
{
    double  dx(0),  dy(0);
    splineDerivative(u, dx, dy);
    double norm = sqrt(dx*dx+dy*dy);

    return {dy/norm, -dx/norm};
}
