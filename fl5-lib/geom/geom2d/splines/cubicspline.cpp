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

#include <cubicspline.h>
#include <matrix.h>
#include <gaussquadrature.h>
#include <constants.h> 
#include <mathelem.h>
#include <node2d.h>


CubicSpline::CubicSpline() : Spline()
{
    m_SplineType = Spline::CUBIC;
}


/**
 * @brief CubicSpline::buildMatrix
 * @param size the number of control points
 * @param u spline parameter, varies between 0 and size; x(u=0)=x0, ... x(u=i)=xi, ... x(u=n-1)=xn-1
 * @param x the array of points
 * @param aij the matrix
 * @param RHS the right hand side
 */
void CubicSpline::buildMatrix(int size, int coordinate, double *aij, double *RHS)
{
    double ui(0), ui1(0);

    int i(0);
    for(i=0; i<nCtrlPoints()-2; i++)
    {
        ui  = m_segVal.at(i);
        ui1 = m_segVal.at(i+1);

        // first row : interpolation at 1st point
        aij[(4*i+0)*size+ i*4]   = ui * ui * ui;
        aij[(4*i+0)*size+ i*4+1] = ui * ui;
        aij[(4*i+0)*size+ i*4+2] = ui;
        aij[(4*i+0)*size+ i*4+3] = 1.0;
        if(coordinate==0) RHS[4*i + 0] = m_CtrlPt.at(i).x;
        else              RHS[4*i + 0] = m_CtrlPt.at(i).y;

        // second row : interpolation at 2nd point
        aij[(4*i+1)*size + i*4]   = ui1 * ui1 * ui1;
        aij[(4*i+1)*size + i*4+1] = ui1 * ui1;
        aij[(4*i+1)*size + i*4+2] = ui1;
        aij[(4*i+1)*size + i*4+3] = 1.0;
        if(coordinate==0) RHS[4*i + 1] = m_CtrlPt.at(i+1).x;
        else              RHS[4*i + 1] = m_CtrlPt.at(i+1).y;

        // third row : derivative continuity
        aij[(4*i+2)*size + i*4  ] =  3.0 * ui1 * ui1;
        aij[(4*i+2)*size + i*4+1] =  2.0 * ui1;
        aij[(4*i+2)*size + i*4+2] =  1.0;
        aij[(4*i+2)*size + i*4+3] =  0.0;
        aij[(4*i+2)*size + i*4+4] = -3.0 * ui1 * ui1;
        aij[(4*i+2)*size + i*4+5] = -2.0 * ui1;
        aij[(4*i+2)*size + i*4+6] = -1.0;
        aij[(4*i+2)*size + i*4+7] =  0.0;
        RHS[4*i + 2] = 0.0;

        // fourth row : 2nd derivative continuity
        aij[(4*i+3)*size + i*4  ] =  6.0 * ui1;
        aij[(4*i+3)*size + i*4+1] =  2.0;
        aij[(4*i+3)*size + i*4+2] =  0.0;
        aij[(4*i+3)*size + i*4+3] =  0.0;
        aij[(4*i+3)*size + i*4+4] = -6.0 * ui1;
        aij[(4*i+3)*size + i*4+5] = -2.0 ;
        aij[(4*i+3)*size + i*4+6] =  0.0;
        aij[(4*i+3)*size + i*4+7] =  0.0;
        RHS[4*i + 3] = 0.0;
    }

    ui  = m_segVal.at(i);
    ui1 = m_segVal.at(i+1);
    // last four rows
    // interpolation at 1st point
    aij[(4*i+0)*size + i*4]   = ui * ui * ui;
    aij[(4*i+0)*size + i*4+1] = ui * ui;
    aij[(4*i+0)*size + i*4+2] = ui;
    aij[(4*i+0)*size + i*4+3] = 1.0;
    if(coordinate==0) RHS[4*i + 0] = m_CtrlPt.at(i).x;
    else              RHS[4*i + 0] = m_CtrlPt.at(i).y;

    // interpolation at 2nd point
    aij[(4*i+1)*size + i*4]   = ui1 * ui1 * ui1;
    aij[(4*i+1)*size + i*4+1] = ui1 * ui1;
    aij[(4*i+1)*size + i*4+2] = ui1;
    aij[(4*i+1)*size + i*4+3] = 1.0;
    if(coordinate==0) RHS[4*i + 1] = m_CtrlPt.at(i+1).x;
    else              RHS[4*i + 1] = m_CtrlPt.at(i+1).y;

    // two "natural" spline conditions
    aij[(4*i+2)*size + 0    ] = 0.0;
    aij[(4*i+2)*size + 1    ] = 2.0;
    RHS[4*i + 2] = 0.0;

    aij[(4*i+3)*size + i*4+0] = 6.0 * ui1; /** @todo check value of ui1 */
    aij[(4*i+3)*size + i*4+1] = 2.0;
    RHS[4*i + 3] = 0.0;
}


bool CubicSpline::updateSpline()
{
    if(nCtrlPoints()<2)
    {
        m_bSingular = true;
        return false;
    }

    m_segVal.resize(nCtrlPoints());

    std::fill(m_segVal.begin(), m_segVal.end(), 0);
//    m_segVal.fill(0);

    // ideally we should calculate spline arc lengths, then reset spline parameters
    // we just approximate spline parameters with distance between control points

    m_segVal[0] = 0.0;

    // first step
    //    calculate arc lengths between control points
    //    we use the default uniform parameter value

    for(int i=1; i<nCtrlPoints(); i++) m_segVal[i] = double(i)/double(nCtrlPoints()-1);

    if(!solve())
    {
        m_bSingular = true;
        return false;
    }

    m_bSingular = false;

    return true;
}


/**
 * @brief Calculates the polynomial coefficients for each arc between consecutive control points.
 * Uses in input the array if arc segments lengths and the control points.
 * @return true if the coefficients have been successfully calculated, false otherwise.
 */
bool CubicSpline::solve()
{
    int size = (nCtrlPoints()-1)*4;
    std::vector<double> aij(size*size, 0);
    std::vector<double> RHS(size, 0);

    // Solve for x coefficients
    buildMatrix(size, 0, aij.data(), RHS.data());

    int info = matrix::solveLinearSystem(size, aij.data(), 1, RHS.data());


    if(info!=0)
    {
        return false;
    }

    m_cx = RHS;

    // Solve for y coefficients
//    aij.fill(0);
//    RHS.fill(0);
    std::fill(aij.begin(), aij.end(), 0);
    std::fill(RHS.begin(), RHS.end(), 0);

    buildMatrix(size, 1, aij.data(), RHS.data());
    info = matrix::solveLinearSystem(size, aij.data(), 1, RHS.data());
    if(info!=0) return false;

    m_cy = RHS;

    return true;
}


void CubicSpline::splineDerivative(double u, double &dx, double &dy) const
{
    dx=0.0;
    dy=0.0;
    if(nCtrlPoints()<2) return;

    for(int i=0; i<nCtrlPoints()-2; i++)
    {
        if(u<=m_segVal.at(i+1))
        {
            dx = 3.0*m_cx.at(i*4)*u*u + 2.0*m_cx.at(i*4+1)*u + m_cx.at(i*4+2);
            dy = 3.0*m_cy.at(i*4)*u*u + 2.0*m_cy.at(i*4+1)*u + m_cy.at(i*4+2);
            return;
        }
    }

    int i = nCtrlPoints()-2;
    dx = 3.0*m_cx.at(i*4)*u*u + 2.0*m_cx.at(i*4+1)*u + m_cx.at(i*4+2);
    dy = 3.0*m_cy.at(i*4)*u*u + 2.0*m_cy.at(i*4+1)*u + m_cy.at(i*4+2);
}


void CubicSpline::splineSecondDerivative(double u, double &d2x, double &d2y) const
{
    d2x=0.0;
    d2y=0.0;
    int i(0);

    bool bInt = false;
    for(i=0; i<nCtrlPoints()-2; i++)
    {
        if(u<=m_segVal.at(i+1))
        {
            d2x = 6.0*m_cx.at(i*4)*u + 2.0*m_cx.at(i*4+1);
            d2y = 6.0*m_cy.at(i*4)*u + 2.0*m_cy.at(i*4+1);
            bInt = true;
            break;
        }
    }
    if(!bInt)
    {
        d2x = 6.0*m_cx.at(i*4)*u + 2.0*m_cx.at(i*4+1);
        d2y = 6.0*m_cy.at(i*4)*u + 2.0*m_cy.at(i*4+1);
    }
}


void CubicSpline::computeArcLengths()
{
    GaussQuadrature GQ(5);

    m_ArcLengths.resize(nCtrlPoints()-1);

    // compute the length of each curve between control points
    double dx(0),dy(0);
    for(int i=0; i<nCtrlPoints()-1; i++)
    {
        double uLength = m_segVal.at(i+1)-m_segVal.at(i);
        m_ArcLengths[i] = 0.0;
        for(int jq=0; jq<GQ.order(); jq++)
        {
            double u = m_segVal.at(i) + GQ.xrel(jq) * uLength;
            splineDerivative(u, dx, dy);
            m_ArcLengths[i] +=  GQ.weight(jq) * sqrt(dx*dx + dy*dy) * uLength;
        }
    }
}


/**
 * @brief Calculates the spline's length between the spline parameters u0=0.0 and u1
 * @param u1 the end parameter
 * @return  the spline's arc length
 */
double CubicSpline::length(double u0, double u1) const
{
    if(fabs(u1)<PRECISION || u0>u1 || fabs(u1-u0)<PRECISION) return 0.0;
    if(m_ArcLengths.size()!=m_segVal.size()-1) return 0.0;

    GaussQuadrature GQ(7);

    u0*=m_segVal.back();
    u1*=m_segVal.back();

    // add all the segment lengths up to the one containing parameter u
    int k(0);
    double dx(0), dy(0);

    double length0 = 0.0;
    for(k=0; k<nCtrlPoints()-1; k++)
    {
        if(u0>m_segVal.at(k+1)) length0 += m_ArcLengths.at(k);
        else break;
    }

    // u0 is between k and k+1
    // calculate length between control point k and the point defined by u0
    for(int j=0; j<GQ.order(); j++)
    {
        // the change of variable is a change of origin and a scale by 1/2 since
        // u_i+1 - u_i = 1.0; and GQ points are defined in [-1,1]
        double xk = m_segVal.at(k) + GQ.xrel(j)* (u0-m_segVal.at(k));
        splineDerivative(xk, dx, dy);
        length0 +=  GQ.weight(j) * sqrt(dx*dx + dy*dy) *(u0-m_segVal.at(k)) ;
    }

    double length1 = 0.0;
    for(k=0; k<nCtrlPoints()-1; k++)
    {
        if(u1>m_segVal[k+1]) length1 += m_ArcLengths.at(k);
        else break;
    }

    // u1 is between k and k+1
    // calculate length between control point k and the point defined by u1
    for(int j=0; j<GQ.order(); j++)
    {
        // the change of variable is a change of origin and a scale by 1/2 since
        // u_i+1 - u_i = 1.0; and GQ points are defined in [-1,1]
        double xk = m_segVal.at(k) + GQ.xrel(j) * (u1-m_segVal.at(k));
        splineDerivative(xk, dx, dy);
        length1 +=  GQ.weight(j) * sqrt(dx*dx + dy*dy) *(u1-m_segVal.at(k));
    }
    return length1-length0;
}


void CubicSpline::makeCurve()
{
    if(m_bSingular) return;
    rePanel(outputSize());
}


double CubicSpline::curvature(double u) const
{
    if(nCtrlPoints()<2) return 0.0;

    double dx(0), dy(0), d2x(0), d2y(0);
    splineDerivative(u,dx,dy);
    splineSecondDerivative(u,d2x,d2y);
    double l = sqrt(dx*dx+dy*dy);
    return fabs(dx*d2y-dy*d2x)/l/l/l;
}


void CubicSpline::computeArcCurvatures(std::vector<double> &arccurvatures) const
{
    GaussQuadrature GQ(7);

    arccurvatures.resize(nCtrlPoints()-1);
    for(int i=0; i<nCtrlPoints()-1; i++)
    {
        double uLength = m_segVal.at(i+1)-m_segVal.at(i);
        arccurvatures[i] = 0.0;
        for(int jq=0; jq<GQ.order(); jq++)
        {
            double u = m_segVal.at(i)+ GQ.xrel(jq) * uLength;
            arccurvatures[i] +=  GQ.weight(jq) * curvature(u) * uLength;
        }
    }
}


double CubicSpline::totalLength() const
{
    if(nCtrlPoints()<2) return 0.0;

    double ll = 0.0;
    for(uint j=0; j<m_ArcLengths.size(); j++)
    {
        ll += m_ArcLengths.at(j);
    }
    return ll;
}


double CubicSpline::totalCurvature(double u0, double u1) const
{
    if(nCtrlPoints()<2) return 0.0;

    GaussQuadrature GQ(7);
    if(GQ.error())
    {
        qDebug("Error in Gaussian quadrature");
        return 0.0;
    }

    double curvatureInt = 0.0;
    for(int jq=0; jq<GQ.order(); jq++)
    {
        double u = u0+ GQ.xrel(jq) * (u1-u0);
        curvatureInt += GQ.weight(jq) * curvature(u) * (u1-u0);
    }
    return curvatureInt;
}


bool CubicSpline::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    Spline::serializeFl5(ar, bIsStoring);

    int n=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    double dble=0.0;


    if(bIsStoring)
    {
        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares; n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

        return true;
    }
    else
    {
        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;


        updateSpline();
        makeCurve();
        return true;
    }
}


/**
 * @brief CubicSpline::getPoint Searches for the point on the spline between spline parameters smin and smax which intersects segment AB
 */
bool CubicSpline::getPoint(bool bBefore, double sfrac, Vector2d const &A, Vector2d const &B, Vector2d &I) const
{
    double angleprecision = 0.0001; //rad
    assert(m_segVal.size()>0);
    assert(sfrac >0  && sfrac<m_segVal.back());
    double smin(0), smax(m_segVal.back());
//    if(bBefore) smax = tmin() + sfrac*(tmax()-tmin());
//    else        smin = tmin() + sfrac*(tmax()-tmin());;
    if(bBefore) smax = sfrac;
    else        smin = sfrac;

    Vector2d dir(B-A);
    
    Vector2d C, D;

    C = splinePoint(smin);
    D = splinePoint(smax);

    double s(0);

    int iter = 0;
    do
    {
        s = (smin+smax)/2.0;
        I = splinePoint(s);

        double dot = (I.x-A.x)*dir.x + (I.y-A.y)*dir.y;      // Dot product between [x1, y1] and [x2, y2]
        double det = (I.x-A.x)*dir.y - (I.y-A.y)*dir.x;      // Determinant
        double angle = atan2(det, dot);  // atan2(y, x) or atan2(sin, cos)
        if(angle>0) smin = s;
        else        smax = s;

        if(fabs(smax-smin)<angleprecision)
        {
            return true;
            break;
        }
        iter++;
    }while(iter<100);

    return false;
}


bool CubicSpline::approximate(int nCtrlPts, std::vector<Node2d> const& node)
{
    if(nCtrlPts<0)
    {
        //use them all
        m_CtrlPt.resize(node.size());
        for(uint i=0; i<node.size(); i++)
        {
            m_CtrlPt[i].set(node.at(i));
        }
    }
    else
    {
        clearControlPoints();
        if (nCtrlPts<3) nCtrlPts = 3;
        if (nCtrlPts>int(node.size())) nCtrlPts = int(node.size());
        double step = double(node.size()-1)/double(nCtrlPts-1);
        for(int i=0; i<nCtrlPts-1; i++)
        {
            int n = int(round(double(i)*step));
            appendControlPoint(node.at(n).x, node.at(n).y);
        }
        //force the last
        appendControlPoint(node.back().x, node.back().y);

    }

    if(!updateSpline()) return false;

    makeCurve();

    return true;
}


Vector2d CubicSpline::splinePoint(double u) const
{
    double x(0), y(0);

    if(nCtrlPoints()<2) return Vector2d();

    for(uint i=0; i<m_segVal.size()-1; i++)
    {
        if(u<=m_segVal.at(i+1))
        {
            x = m_cx.at(i*4)*u*u*u + m_cx.at(i*4+1)*u*u + m_cx.at(i*4+2)*u + m_cx.at(i*4+3);
            y = m_cy.at(i*4)*u*u*u + m_cy.at(i*4+1)*u*u + m_cy.at(i*4+2)*u + m_cy.at(i*4+3);
            return Vector2d(x,y);
        }
    }

//  return the last control point;
    x = m_CtrlPt.back().x;
    y = m_CtrlPt.back().y;
    return Vector2d(x,y);
}


void CubicSpline::rePanel(int N)
{
    if(nCtrlPoints()<=2) return;

    m_Output.clear();
    m_Output.resize(N);

    std::vector<double> length(nCtrlPoints());
    computeArcLengths();

    double l=0;

    for(uint i=0; i<m_ArcLengths.size(); i++) l+=m_ArcLengths.at(i);

    length[0] = 0.0;
    for(uint i=1; i<length.size(); i++) length[i] = length[i-1] + m_ArcLengths.at(i-1);
    for(uint i=1; i<length.size(); i++) length[i] *= 1.0/l;

    double u(0);
    for(int i=0; i<N; i++)
    {
        double t = double(i)/double(N-1);

        if     (m_BunchType==Spline::DOUBLESIG) u = doubleSigmoid(-m_BunchAmp, t);
        else if(m_BunchType==Spline::SIGMOID)   u = sigmoid(-m_BunchAmp, t);
        else                                    u = t; // UNIFORM length spacing

        for(uint j=0; j<m_segVal.size()-1; j++)
        {
            if(u<=length.at(j+1))
            {
                u = m_segVal.at(j) + (u-length.at(j))/(length.at(j+1)-length.at(j)) * (m_segVal.at(j+1)-m_segVal.at(j)); // space evenly
                m_Output[i].x = m_cx.at(j*4)*u*u*u + m_cx.at(j*4+1)*u*u + m_cx.at(j*4+2)*u + m_cx.at(j*4+3);
                m_Output[i].y = m_cy.at(j*4)*u*u*u + m_cy.at(j*4+1)*u*u + m_cy.at(j*4+2)*u + m_cy.at(j*4+3);
                break;;
            }
        }
    }

    //  return the last control point;
    m_Output.back().x = m_CtrlPt.back().x;
    m_Output.back().y = m_CtrlPt.back().y;
}









