/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QThread>
#include <QDataStream>

#include "bspline.h"
#include <xflmath/constants.h>
#include <xflmath/matrix.h>
#include <xflmath/mathelem.h>
#include <xflgeom/geom_globals/geom_global.h>

BSpline::BSpline() : Spline()
{
    m_SplineType = Spline::BSPLINE;
    m_degree = 3;
}


void BSpline::makeCurve()
{
    if(m_Output.size()<=0) m_Output.resize(279);

    if (m_CtrlPt.size()>=2)
    {
        double t = 0;
        double u = 0;
        double increment = 1.0/double(outputSize() - 1);

        for (int j=0; j<m_Output.size(); j++)
        {
//            double u = bunchedParameter(m_BunchDistrib, m_BunchAmp, t);
            switch(m_BunchType)
            {
                case Spline::NOBUNCH:
                case Spline::UNIFORM:
                    u = t;
                    break;
                case Spline::SIGMOID:
                    u = sigmoid(-m_BunchAmp, t);
                    break;
                case Spline::DOUBLESIG:
                    u = doubleSigmoid(-m_BunchAmp, t);
                    break;
            }

            splinePoint(u, m_Output[j].x, m_Output[j].y);
            t += increment;
        }
        m_Output.back() = m_CtrlPt.back();
    }
    else
    {
        m_Output.clear();
    }
    m_bSingular = false;
}


void BSpline::splinePoint(double t, double &x, double &y) const
{
    double w(0);
    x=y=0.0;
    t=std::min(t, 0.99999);
    for (int i=0; i<m_CtrlPt.size(); i++)
    {
        double b = basis(i, m_degree, t, m_knot.data()) * m_Weight[i];
        x += m_CtrlPt.at(i).x * b;
        y += m_CtrlPt.at(i).y * b;
        w += b;
    }
    if(w>0)
    {
        x *= 1.0/w;
        y *= 1.0/w;
    }
}


/** returns the vector (dC.x/dt, dC.y/dt) */
void BSpline::splineDerivative(double t, double &dx, double &dy) const
{
    double np(0);
    dx=0.0;
    dy=0.0;
    t=std::min(t, 0.99999);
    for(int i=0; i<ctrlPointCount(); i++)
    {
        np = basisDerivative(i, m_degree, t, m_knot.constData());
        dx += m_CtrlPt[i].x * np;
        dy += m_CtrlPt[i].y * np;
    }
}


/**
 *Generates an array of standard knot values for this spline.
 */
bool BSpline::splineKnots()
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


void BSpline::resetSpline()
{
    m_CtrlPt.clear();
    m_Weight.clear();
    splineKnots();
}


bool BSpline::updateSpline()
{
    m_bSingular = !splineKnots();
    return m_bSingular;
}


void BSpline::duplicate(Spline const &spline)
{
    Spline::duplicate(spline); //call base class method
    m_degree = spline.degree();
}


bool BSpline::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int n=0;
    // 500001 : first version of new fl5 format

    if(bIsStoring)
    {
        Spline::serializeFl5(ar, bIsStoring);
        ar << m_degree;

        // dynamic space allocation for the future storage of more data, without need to change the format
        n=0;
        ar << n << n;

        return true;
    }
    else
    {
        resetSpline();
        Spline::serializeFl5(ar, bIsStoring);
        ar >> m_degree;

        // space allocation
        ar >> n >> n;

        updateSpline();
        makeCurve();
        return true;
    }
}


void BSpline::copySymmetric(BSpline const &bspline)
{
    Spline::copysymmetric(bspline); //call base class method
    m_degree     = bspline.degree();
    m_Weight.clear();
    m_Weight.append(bspline.m_Weight);
}


/**
 * @brief Creates a Cubic B-Spline from the interpolation of nPts points;
 * @param nPts the number of points to interpolate
 * @param pt a pointer to the array of points to interpolate.
 */
void BSpline::fromInterpolation(int N, Vector2d const *pt)
{
    // to interpolate (n +1) data points, one needs (n +7) knots
    // t0, t1, ..., t_n+6 , for a uniform cubic B-spline interpolating curve
    if(m_knot.size()<N+6) m_knot.resize(N+6);
    m_knot[0] = m_knot[1] = m_knot[2] = m_knot[3] = 0.0;
    for(int i=3; i<N+2; i++) m_knot[i]=i-3;
    m_knot[N+2] = m_knot[N+3] = m_knot[N+4] = m_knot[N+5] = N-1;

    // to interpolate (n +1) points, one needs (n +3) control points,
    // P_0, P_1, ..., P_n+2, for a uniform cubic B-spline interpolating curve
    if(m_CtrlPt.size()<N+2)
    {
        m_CtrlPt.resize(N+2);
    }
    m_Weight.clear();
    for(int i=0; i<N+2; i++) m_Weight.push_back(1.0);


    // make the matrix
    // int n=N-1;
    QVector<double>aij(N*N, 0);

    aij[0]   = 3.0;       aij[1]   = -1.0;
    aij[N] = 1.0/4.0;     aij[N+1] = 7.0/12.0;   aij[N+2] = 1.0/6.0;
    for(int i=2; i<N-2; i++)
    {
        aij[i*N+i-1] = 1.0/6.0;     aij[i*N+i] = 2.0/3.0;     aij[i*N+i+1] = 1.0/6.0;
    }
    aij[(N-2)*N+N-3] = 1.0/6.0;     aij[(N-2)*N+N-2] = 7.0/12.0;   aij[(N-2)*N+N-1] = 1.0/4.0;
    aij[(N-1)*N+N-2]       = -1.0;    aij[(N-1)*N+N-1] = 3.0;

    //    make the RHS
    QVector<double> RHS(2*N, 0);

    RHS[0]   = 2.0*pt[0].x;
    RHS[N+0] = 2.0*pt[0].y;
    for(int i=1; i<N-1; i++)
    {
        RHS[i]   = pt[i].x;
        RHS[N+i] = pt[i].y;
    }
    RHS[N-1]   = 2.0*pt[N-1].x;
    RHS[N+N-1] = 2.0*pt[N-1].y;

    //solve
    int info = solveLinearSystem(N, aij.data(), 2, RHS.data());
    if(info!=0)
    {
        m_bSingular=true;
        return;
    }
//    Gauss(aij, N, RHS, 2, bCancel); /** @todo is a band matrix, no need for pivoting */

    m_CtrlPt[0]   = pt[0];
    m_CtrlPt[N+1] = pt[N-1];
    for(int i=1; i<N+1; i++)
    {
        m_CtrlPt[i].x = RHS[i-1];
        m_CtrlPt[i].y = RHS[i+N-1];
    }

    updateSpline();
    makeCurve();
}


/**
 * Given a set of n+1 data points, D0 , D1, ..., Dn, a degree p, and a
 * number h, where n > h ≥ p ≥ 1, finds a B-spline curve of degree p defined
 * by nPts control points, that satisfies the following conditions:
 *   1. The curve contains the first and last data points
 *   2. The curve approximates the data polygon in the sense of least square.
*/
bool BSpline::approximate(int degree, int nPts, QVector<Vector2d> const& node)
{
    // p=degree
    // h=number of control points to build
    // n > h >= p >= 1

    int p = degree;
    int h = nPts-1;
    const int n = node.size()-1 ;
    if(p==0 || p>h)         {m_bSingular=true;    return false;}
    if(h>=n)                {m_bSingular=true;    return false;}

    m_degree = degree;
    m_CtrlPt.resize(h+1);

    setUniformWeights();

    m_CtrlPt.front().set(node.front());
    m_CtrlPt.back().set(node.back());
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
    QVector<double> Q(2*(h-1));
    for(int i=1; i<h; i++)
    {
        int r = i-1;
        Q[r]      = 0.0;
        Q[(h-1)+r]= 0.0;
        for(int k=1; k<n; k++)
        {
            double b = basis(i, p, tk[k], m_knot.data());
            double Qkx = node.at(k).x - basis(0, p, tk[k], m_knot.data()) * node.front().x - basis(h, p, tk[k], m_knot.data()) * node.back().x;
            double Qky = node.at(k).y - basis(0, p, tk[k], m_knot.data()) * node.front().y - basis(h, p, tk[k], m_knot.data()) * node.back().y;
            Q[r]       += b*Qkx;
            Q[(h-1)+r] += b*Qky;
        }
    }

//solveLinearSystem(int rank, double *M, int nrhs, double *rhs, int nThreads)
Q_ASSERT(Q.size()>=2);
    int res = solveLinearSystem(h-1, NtN.data(), 2, Q.data());

    if(res!=0)
    {
        m_bSingular = true;
        return false;
    }

    for(int i=1; i<h; i++)
    {
        int r=i-1;
        m_CtrlPt[i].set(Q[r], Q[(h-1)+r]);
    }
    updateSpline();
    makeCurve();

    return true;
}


void BSpline::testApproximation()
{
    BSpline bs;

    QVector<Vector2d> node = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 3.0}, {3.0,4.5}, {4.0,3.2}, {5.0,2.0}, {6.0,1.0}};

    for(int i=0; i<node.size(); i++)
        qDebug(" in  %11.5g  %11.5g", node[i].x, node[i].y);

    int degree=3;
    int nPts=4;
    if(bs.approximate(degree, nPts, node))
    {
        for(int i=0; i<bs.ctrlPointCount(); i++)
            qDebug(" ctrl %11.5g  %11.5g", bs.m_CtrlPt[i].x, bs.m_CtrlPt[i].y);

        for(int i=0; i<bs.outputSize(); i++)
            qDebug(" out %11.5g  %11.5g", bs.m_Output[i].x, bs.m_Output[i].y);
    }
    else
        qDebug("Error making spline approximation");
}


/**
 * Given a set of data points (x0,y0), returns a smoothed set of points (x0,y).
 * The input points are required to be in x-crescending order.
 * The first and last points of the output curve are the same as those
 * of the input curve.
 * The smoothness is controlled by the spline degree and the number of
 * generated control points. The higher the degree and the lower the nunmber
 * of control points, the smoother the output curve will be.
 * */
bool BSpline::smoothFunction(int deg, int npts, QVector<double> const& x0, QVector<double> const& y0, QVector<double>& y)
{
    int nInput = x0.size();
    if(x0.size()!=y0.size())    return false;
    if(nInput<5)                return false;

    QVector<Vector2d> node(x0.size());
    for(int i=1; i<x0.size(); i++)
    {
        if(x0[i-1]>x0[i])       return false;

        node[i] = Node2d(x0.at(i), y0.at(i));
    }
    y.resize(nInput);

    BSpline bs;
    bs.approximate(deg, npts, node);
    if(bs.m_bSingular) return false;

    //use dichotomy to find approximations of output y;
    Vector2d pt0, pt1, pt;

    for(int i=1; i<nInput-1; i++)
    {
        double x = x0[i];
        double t0=0.0;
        double t1=1.0;
        bs.splinePoint(t0, pt0.x, pt0.y);
        bs.splinePoint(t1, pt1.x, pt1.y);
        double t=0.5;
        int iter=0;
        do
        {
            t=(t0+t1)/2.0;
            bs.splinePoint(t, pt.x, pt.y);
            if(pt.x<x) {t0=t;  pt0=pt;}
            else       {t1=t;  pt1=pt;}
            if(fabs(t1-t0)<1.e-5)
            {
                y[i] = pt.y;
                break;
            }
            iter++;
        }while(iter<100);

        if(iter>=100)
        {
            bs.m_bSingular = true;    return false;
        }
    }
    y.front() = y0.front();
    y.back()  = y0.back();

    return true;
}



/**
 * @brief sgsmooth::testSmooth
 * @param order 5, 7 or 9
 */
void BSpline::testSmooth()
{
    int N = 100;

    QVector<double> x0(N);
    QVector<double> y0(N);
    for(int i=0; i<N; i++)
    {
        x0[i] =  double(i)/double(N-1);
        y0[i] = 1.0*cos(x0[i]*2.0*PI);
        y0[i] += 2.0*sin(x0[i]*5.0);
        y0[i] += 0.3*(double(rand())/double(RAND_MAX)-0.5);
    }

    QVector<double> y;

    BSpline bs;
    bs.smoothFunction(5, N/2, x0, y0, y);

    if(bs.isSingular())
    {
        qDebug(" Smoothing failed");
        return;
    }

    qDebug("         x0            y0           y");
    for(int i=0; i<N; i++)
    {
        qDebug(" %13.5g  %13.5g  %13.5g", x0[i], y0[i], y[i]);
    }
}

/*

void BSpline::makePointWeights()
{
    uint n = m_Weight.size();

    // parabola parameters
    // w = amp*(alpha (x_i)² + beta)
    double alpha = -1.0+2.0*m_BunchDist;
    double beta = 1.0-m_BunchDist;
    alpha = m_BunchDist;
    beta=0.;

//    qDebug(" weighhts:  %11.5g  %11.5g  %11.5g  %11.5g", m_WeightAmp, m_WeightDist, alpha, beta);
    for(uint i=0; i<n; i++)
    {
        double xi = 1.0 -2.0*double(n-1-i)/double(n-1);
        m_Weight[i] = 1.0+ m_BunchAmp*(alpha *xi*xi + beta);
//        qDebug("  %7.2f   w=%9.5f", xi, m_Weight[i]);
    }
}
*/


/** returns the curve parameter corresponding to the point closest to the input point */
/*
double BSpline::closest(double x, double y)
{
    // find the two adjacent points
    double t0=0.0;
    double t1=0.0;
    double dt = 1.0/(m_Output.size()-1);
    double d0=0, d1=0;
    double x0=0, y0=0, x1=0, y1=0;
    splinePoint(t0, x0, y0);
    d0 = (x-x0)*(x-x0)+(y-y0)*(y-y0);

    double dmax=1.e10;
    double tc = t0;
    for(uint i=1; i<m_Output.size(); i++)
    {
        t1 = t0+dt;
        if(t1>1.0001) break;
        splinePoint(t1,x1,y1);
        d1 = (x-x1)*(x-x1)+(y-y1)*(y-y1);
        if(d0+d1<dmax)
        {
            tc=t0;
            dmax = d0+d1;
        }
        t0=t1;
        d0=d1;
    }

    // finish by dichotomy to find the spline parameter which minimizes the distance
    double d=0;

    Vector2d pt;
    t0 = tc;
    t1 = tc+dt;
    splinePoint(t0, x0, y0);
    splinePoint(t1, x1, y1);
    d0 = (x-x0)*(x-x0)+(y-y0)*(y-y0);
    d1 = (x-x1)*(x-x1)+(y-y1)*(y-y1);
    double t=(t0+t1)/2.0;
    uint iter=0;
    do
    {
        t=(t0+t1)/2.0;
        splinePoint(t, pt.x, pt.y);
        d = (x-pt.x)*(x-pt.x)+(y-pt.y)*(y-pt.y);

        if(d0>d1)
        {
            t0=t;
            d0=d;
        }
        else
        {
            t1=t;
            d1=d;
        }

        if(fabs(t1-t0)<1.e-5)
        {
            break;
        }
        iter++;
    }while(iter<100);

    if(iter>=100)
    {
        return 0.0;
    }

    return t;
}*/
