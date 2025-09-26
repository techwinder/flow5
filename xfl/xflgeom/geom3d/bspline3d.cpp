/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include "bspline3d.h"
#include <xflgeom/geom_globals/geom_global.h>
#include <xflmath/mathelem.h>
#include <xflmath/matrix.h>

BSpline3d::BSpline3d()
{
    m_bSingular = true; // until spline is made

    m_theStyle.m_Color = QColor(75,155,205);
    m_theStyle.m_Width = 2;

    m_CtrlPt.clear();
    m_Weight.clear();
    m_iHighlight  = -10;
    m_iSelect     = -10;

    m_Output.clear();
    m_Output.resize(79);

    m_bIsModified = false;

    m_bShowCtrlPts = true;
    m_bShowNormals = false;

    m_bClosed = false;
    m_bForcesymmetric = false;

    m_BunchAmp = 0.0;
    m_BunchDist = 0.5;

    m_degree = 3;

    // make default spline
    m_CtrlPt.append({ 0.0, 0.0,  0.0});
    m_CtrlPt.append({ 1.0, 1.0,  0.0});
    m_CtrlPt.append({ 2.5, 2.0,  1.0});
    m_CtrlPt.append({ 3.5, 2.0,  1.5});
    m_CtrlPt.append({ 3.5, 0.0,  0.5});
    m_CtrlPt.append({ 1.7, -2.1, -0.3});
    m_CtrlPt.append({-1.3, -2.5, 0.7});
    m_CtrlPt.append({-1.0, -1.0, 1.5});
    m_CtrlPt.append({-0.7, -0.0, 1.7});
    m_CtrlPt.append({-0.3,  1.5, 1.5});
    m_CtrlPt.append({-0.0,  2.5, 0.0});

    m_Weight.resize(m_CtrlPt.size());
    m_Weight.fill(1.0);

    splineKnots();
    updateSpline();
    makeCurve();
}


void BSpline3d::appendControlPoint(double x, double y, double z, double w)
{
     appendControlPoint(Vector3d(x,y,z),w);
}


void BSpline3d::appendControlPoint(Vector3d const&Pt, double w)
{
    m_CtrlPt.push_back(Pt);
    m_Weight.push_back(w);

    if(m_bForcesymmetric)
    {
        if(m_CtrlPt.size()>0) m_CtrlPt.first().y = m_CtrlPt.last().y =0;
    }

    setModified(true);
}


void BSpline3d::resizeControlPoints(int nPts)
{
    if(nPts==int(m_CtrlPt.size())) return; // nothing to do
    //erase all points in between
    for(int ic=m_CtrlPt.size()-2; ic>0; ic--)
    {
        removeCtrlPoint(ic);
    }

    int nnew = nPts-2;
    for(int j=0; j<nnew; j++)
    {
        double tau = double(j+1)/double(nnew+1);
        Vector3d point = m_CtrlPt.front() + (m_CtrlPt.back()-m_CtrlPt.front())*tau;
        m_CtrlPt.insert(m_CtrlPt.begin()+j+1, point);
        double w = m_Weight.front() + tau*(m_Weight.back()-m_Weight.front());
        m_Weight.insert(m_Weight.begin()+j+1, w);
    }
    updateSpline();
}


void BSpline3d::setCtrlPoints(const QVector<Vector3d> &ptList, double w)
{
    m_CtrlPt = ptList;

    m_Weight.clear();
    for(int i=0; i<ptList.size(); i++)
        m_Weight.push_back(w);

    setModified(true);
}


void BSpline3d::appendCtrlPoints(QVector<Vector3d> const & ptList, double w)
{
    m_CtrlPt.append(ptList);
    for(int i=0; i<ptList.size(); i++) m_Weight.push_back(w);

    setModified(true);
}


void BSpline3d::appendCtrlPoints(QVector<Vector3d> const & ptList, QVector<double> weightlist)
{
    m_CtrlPt.append(ptList);
    m_Weight.append(weightlist);

    setModified(true);
}


void BSpline3d::insertCtrlPointAt(int iSel, double x, double y, double z, double w)
{
    insertCtrlPointAt(iSel, Vector3d(x,y,z), w);
}


void BSpline3d::insertCtrlPointAt(int iSel, Vector3d pt, double w)
{
    m_CtrlPt.insert(m_CtrlPt.begin()+iSel, pt);
    m_Weight.insert(m_Weight.begin()+iSel, w);
}


void BSpline3d::insertCtrlPointAt(int iSel)
{
    m_CtrlPt.insert(m_CtrlPt.begin()+iSel, Vector3d());
    m_Weight.insert(m_Weight.begin()+iSel, 1.0);

    if(iSel>0 && iSel<m_CtrlPt.size()-1)
    {
        m_CtrlPt[iSel] = (m_CtrlPt[iSel-1]+m_CtrlPt[iSel+1])/2.0;
        m_Weight[iSel] = (m_Weight[iSel-1]+m_Weight[iSel+1])/2.0;
    }
    else if (iSel==0)
    {
        m_CtrlPt[iSel] = m_CtrlPt[1]*0.95;
        m_Weight[iSel] = m_Weight[1];
    }
    else if (iSel==m_CtrlPt.size()-1)
    {
        m_CtrlPt[iSel] = m_CtrlPt[iSel-1]*1.05;
        m_Weight[iSel] = m_Weight[iSel-1];
    }
}


bool BSpline3d::removeCtrlPoint(int k)
{
    if (k>=0 && k<m_CtrlPt.size())
    {
        m_CtrlPt.erase(m_CtrlPt.begin()+k);
        m_Weight.erase(m_Weight.begin()+k);
        setModified(true);
        return true;
    }
    return false;
}


int BSpline3d::isCtrlPoint(double x, double y, double tolx, double toly) const
{
    for (int k=0; k<m_CtrlPt.size(); k++)
    {
        if(fabs(x-m_CtrlPt.at(k).x)<tolx && fabs(y-m_CtrlPt.at(k).y)<toly) return int(k);
    }
    return -10;
}


void BSpline3d::resetSpline()
{
    m_CtrlPt.clear();
    m_Weight.clear();
    splineKnots();
}


bool BSpline3d::updateSpline()
{
    m_bSingular = splineKnots();
    return m_bSingular;
}


bool BSpline3d::splineKnots()
{
    if(m_CtrlPt.size()<2)
    {
        m_knot.clear();
        return false;
    }
    int iDegree = std::min(m_degree, int(m_CtrlPt.size()));

    int nKnots  = iDegree + m_CtrlPt.size() + 1;
    m_knot.resize(nKnots);

    for (int j=0; j<nKnots; j++)
    {
        if (j<iDegree+1)  m_knot[j] = 0.0; //double(j)/1000.0;
        else
        {
            if(j<m_CtrlPt.size())
            {
                double a = double(j-iDegree);
                double b = double(nKnots-2*iDegree-1);
                if(fabs(b)>0.0) m_knot[j] = a/b;
                else            m_knot[j] = 1.0;
            }
            else m_knot[j] = 1.0; //+double(j-m_CtrlPt.size()-1.0)/1000.0;
        }
    }
    return true;
}


/**
* Calculates the spline's output points
*/
void BSpline3d::makeCurve()
{
    double t=0.0, increment=0.0;

    if (m_CtrlPt.size()>=2)
    {
        t = 0;
        increment = 1.0/double(outputSize() - 1);

        for (int j=0; j<outputSize(); j++)
        {
            double u = bunchedParameter(m_BunchDist, m_BunchAmp, t);
            splinePoint(u, m_Output[j]);
            t += increment;
        }
        m_Output.back() = m_CtrlPt.back();
    }
    m_bSingular = false;
}


void BSpline3d::splinePoint(double t, Vector3d &pt) const
{
    double w=0.0;
    pt.reset();
    for (int i=0; i<m_CtrlPt.size(); i++)
    {
        double b = basis(i, m_degree, t, m_knot.data()) * m_Weight[i];
        pt.x += m_CtrlPt[i].x * b;
        pt.y += m_CtrlPt[i].y * b;
        pt.z += m_CtrlPt[i].z * b;
        w += b;
    }
    pt.x *= 1.0/w;
    pt.y *= 1.0/w;
    pt.z *= 1.0/w;
}


/** returns the vector (dC.x/dt, dC.y/dt) */
void BSpline3d::splineDerivative(double t, Vector3d &der) const
{
    int m = ctrlPointCount();
    int n = m_degree;
    der.reset();

    for(int i=0; i<m; i++)
    {
        double k0 = m_knot[i+n]-m_knot[i];
        double k1 = m_knot[i+n+1]-m_knot[i+1];
        double n0=0.0, n1=0.0;
        double np=0.0;

        if(fabs(k0)>0.0)
        {
            n0 = basis(i, n-1, t, m_knot.data());
            np += n/k0*n0;
        }
        if(fabs(k1)>0.0)
        {
            n1 = basis(i+1, n-1, t, m_knot.data());
            np -= n/k1*n1;
        }
        der.x += m_CtrlPt[i].x * np;
        der.y += m_CtrlPt[i].y * np;
        der.z += m_CtrlPt[i].z * np;
    }
}


void BSpline3d::splineDerivative(BSpline3d &der) const
{
    QVector<Vector3d> Q(m_CtrlPt.size()-1);

    int p = m_degree;
    for(int i=0; i<Q.size(); i++)
    {
        Q[i] = (m_CtrlPt[i+1]-m_CtrlPt[i]) * double(p)/(double(m_knot[i+p+1]-m_knot[i]));
    }

    der.setCtrlPoints(Q);
    der.setDegree(p-1);
    der.splineKnots();

    QVector<double> knots = m_knot;
    knots.pop_front();
    knots.pop_back();
    der.setKnots(knots);
 }


void BSpline3d::setUniformWeights()
{
    m_Weight.resize(ctrlPointCount());
    for(int i=0; i<m_Weight.size(); i++) m_Weight[i]=1.0;
}


/**
 * Given a set of n+1 data points, D0 , D1, ..., Dn, a degree p, and a
 * number h, where n > h ≥ p ≥ 1, finds a B-spline curve of degree p defined
 * by nPts control points, that satisfies the following conditions:
 *   1. The curve contains the first and last data points
 *   2. The curve approximates the data polygon in the sense of least square.
*/
bool BSpline3d::approximate(int degree, int nPts, const QVector<Vector3d> &pts)
{
    // p=degree
    // h=number of control points to build
    // n > h >= p >= 1

    int p = degree;
    int h = nPts-1;
    const int n = pts.size()-1 ;
    if(p==0 || p>h)           {m_bSingular=true;    return false;}
    if(h>=n)                  {m_bSingular=true;    return false;}

    m_degree = degree;
    m_CtrlPt.resize(h+1);

    setUniformWeights();

    m_CtrlPt.front().set(pts.front());
    m_CtrlPt.back().set(pts.back());
    splineKnots();

    // the resulting curves passes through the first and last input points
    // build the intermediate points
    QVector<double> Nij((h-1)*(n-1), 0);
    QVector<double> tNij((h-1)*(n-1), 0);
    QVector<double> tk(n+1, 0);
    int nCols = h-1;
    int nRows = n-1;

    for(int k=0; k<=n; k++) tk[k] = double(k)/double(n); // n+1 values

    for(int i=1; i<n; i++)
    {
        int r=i-1; // row index
        for(int j=1; j<h; j++)
        {
            int c=j-1; // col index
            double b = basis(j, p, tk[i], m_knot.data());
            Nij[r*nCols+c]  = b;
            tNij[c*nRows+r] = b;
        }
    }

    QVector<double> NtN(nRows*nRows);
    matMult(tNij.data(), Nij.data(), NtN.data(), nCols, nRows, nCols, 1);

    //make the RHS
    QVector<double> Q(3*(h-1));
    for(int i=1; i<h; i++)
    {
        int r = i-1;
        Q[        r] = 0.0;
        Q[  (h-1)+r] = 0.0;
        Q[2*(h-1)+r] = 0.0;
        for(int k=1; k<n; k++)
        {
            double b = basis(i, p, tk[k], m_knot.data());
            double Qkx = pts[k].x - basis(0, p, tk[k], m_knot.data()) * pts.front().x - basis(h, p, tk[k], m_knot.data()) * pts.back().x;
            double Qky = pts[k].y - basis(0, p, tk[k], m_knot.data()) * pts.front().y - basis(h, p, tk[k], m_knot.data()) * pts.back().y;
            double Qkz = pts[k].z - basis(0, p, tk[k], m_knot.data()) * pts.front().z - basis(h, p, tk[k], m_knot.data()) * pts.back().z;
            Q[        r] += b*Qkx;
            Q[  (h-1)+r] += b*Qky;
            Q[2*(h-1)+r] += b*Qkz;
        }
    }

    int res = solveLinearSystem(h-1, NtN.data(), 3, Q.data());

    if(res!=0)
    {
        m_bSingular = true;
        return false;
    }

    for(int i=1; i<h; i++)
    {
        int r=i-1;
        m_CtrlPt[i].set(Q[r], Q[(h-1)+r], Q[2*(h-1)+r]);
    }
    updateSpline();
    makeCurve();

    return m_bSingular;
}





