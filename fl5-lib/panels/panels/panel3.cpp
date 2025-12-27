/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QString>


#include <panel3.h>

#include <constants.h>
#include <geom_global.h>
#include <matrix.h>
#include <mctriangle.h>
#include <node.h>
#include <triangle3d.h>
#include <units.h>
#include <utils.h>
#include <vortex.h>

GQTriangle Panel3::s_gq;
int Panel3::s_iQuadratureOrder = 5;
double Panel3::s_Quality = 1.414;
bool Panel3::s_bUseNintcheuFata = true;



Panel3::Panel3() : Panel(), m_bNullTriangle{true}, m_bIsLeftPanel{false}, m_bFromSTL{false}, m_iOppositeIndex{-1},
   m_SignedArea{0}, m_Angle{0,0,0}, bx{0,0,0}, by{0,0,0}, m_mu{0,0,0}, m_beta{0,0,0}
{
    m_S[0].setIndex(-1);
    m_S[1].setIndex(-1);
    m_S[2].setIndex(-1);
    m_Neighbour[0]=m_Neighbour[1]=m_Neighbour[2]=-1;
    memset(m_gmat, 0, 9*sizeof(double));
}


/** The constructor with triangle vertices */
Panel3::Panel3(const Node &S0, const Node &S1, const Node &S2) : Panel()
{
    initialize();

    setFrame(S0,S1,S2);
}


Panel3::Panel3(const Triangle3d &triangle, xfl::enumSurfacePosition pos) : Panel()
{
    initialize();

    setFrame(triangle.vertexAt(0), triangle.vertexAt(1), triangle.vertexAt(2));
    m_S[0].setIndex(triangle.nodeIndex(0));
    m_S[1].setIndex(triangle.nodeIndex(1));
    m_S[2].setIndex(triangle.nodeIndex(2));
    m_Pos = pos;
}


/** constructs the triangle's properties from its vertices; leave the orientation unchanged */
void Panel3::setFrame(Node const &S0, Node const &S1, Node const &S2)
{
    m_S[0].setNode(S0);
    m_S[1].setNode(S1);
    m_S[2].setNode(S2);

    setFrame();
}


void Panel3::setFrame(std::vector<Node> const& nodelist, int index0, int index1, int index2)
{
    m_S[0].setNode(nodelist[index0]);
    m_S[1].setNode(nodelist[index1]);
    m_S[2].setNode(nodelist[index2]);

    m_S[0].setIndex(index0);
    m_S[1].setIndex(index1);
    m_S[2].setIndex(index2);
    setFrame();
}


void Panel3::setFrame(Triangle3d const &triangle, xfl::enumSurfacePosition pos)
{
    m_S[0].set(triangle.vertexAt(0));
    m_S[1].set(triangle.vertexAt(1));
    m_S[2].set(triangle.vertexAt(2));

    m_Pos = pos;

    setFrame();
}


/** Constructs the triangle's properties from the pre-set vertices and the orientation */
void Panel3::setFrame()
{
    m_Edge[0].setNodes(m_S[1],m_S[2]);
    m_Edge[1].setNodes(m_S[2],m_S[0]);
    m_Edge[2].setNodes(m_S[0],m_S[1]);

    m_CoG_g = (m_S[0]+m_S[1]+m_S[2])/3.0;

    // Set the critical edge length to 0.0001 m = 0.1mm
    // This is more than enough for potential flow anaysis
    // Note that this is not the same as LENGTHPRECISION
    // because intermediate calculations, if any,
    // need to be performed with higher accuracy
    if(m_Edge[0].length()<LENGTHPRECISION || m_Edge[1].length()<LENGTHPRECISION || m_Edge[2].length()<LENGTHPRECISION)
    {
        // one null side
        m_Area = m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }

    m_bNullTriangle = false;

    m_Normal = m_Edge[1].segment() * m_Edge[2].segment();

    m_Area = m_Normal.norm()/2.0;

/*    if (    (m_Edge[0].segment()*m_Edge[1].segment()).norm()<AREAPRECISION ||
            (m_Edge[1].segment()*m_Edge[2].segment()).norm()<AREAPRECISION ||
            (m_Edge[2].segment()*m_Edge[0].segment()).norm()<AREAPRECISION)
    {
        // the three sides are colinear, flat triangle
        m_Area = m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }*/
    m_Normal.normalize();

    m_MaxSize = m_Edge[0].length();
    m_MaxSize = std::max(m_Edge[1].length(), m_MaxSize);
    m_MaxSize = std::max(m_Edge[2].length(), m_MaxSize);


    // compute the three internal angles
    double cost0 = m_Edge[1].segment().dot(m_Edge[2].oppSegment()) / m_Edge[1].length() / m_Edge[2].length();
    double sint0 = (m_Edge[2].segment()*m_Edge[1].oppSegment()).dot(m_Normal) / m_Edge[1].length() / m_Edge[2].length();
    m_Angle[0] = atan2( sint0, cost0) * 180.0/PI;

    double cost1 = m_Edge[0].segment().dot(m_Edge[2].oppSegment()) / m_Edge[0].length() / m_Edge[2].length();
    double sint1 = (m_Edge[0].segment()*m_Edge[2].oppSegment()).dot(m_Normal) / m_Edge[0].length() / m_Edge[2].length();
    m_Angle[1] = atan2( sint1, cost1) * 180.0/PI;

    m_Angle[2] = 180.0 - m_Angle[1] - m_Angle[0];

    if (fabs(m_Angle[0])<ANGLEPRECISION || fabs(m_Angle[1])<ANGLEPRECISION || fabs(m_Angle[2])<ANGLEPRECISION )
    {
        // the three sides are colinear, flat triangle
        m_Area = m_SignedArea = 0.0;
        m_bNullTriangle = true;
        return;
    }

    //define the local frame of reference
    // S01 defines the x-axis
    m_l = m_Edge[2].segment().normalized();
    m_m = m_Normal*m_l;

    // for now, set the origin at the CoG
    // to compare integrals with MC's method, set the origin at vertex 0
    m_O.set(m_CoG_g);
    //    O = Sl[0];

    m_CF.setOrigin(m_O);
    m_CF.setIJK(m_l,m_m,m_Normal);

    m_Sl[0].set(m_CF.globalToLocal(m_S[0]-m_O));
    m_Sl[1].set(m_CF.globalToLocal(m_S[1]-m_O));
    m_Sl[2].set(m_CF.globalToLocal(m_S[2]-m_O));
    m_CoG_l.set(m_CF.globalToLocal(m_CoG_g-m_O));

    // calculate the matrix to transform local coordinates in homogeneous barycentric coordinates
    m_gmat[0] = 1.0;     m_gmat[1] = m_Sl[0].x;     m_gmat[2] = m_Sl[0].y;
    m_gmat[3] = 1.0;     m_gmat[4] = m_Sl[1].x;     m_gmat[5] = m_Sl[1].y;
    m_gmat[6] = 1.0;     m_gmat[7] = m_Sl[2].x;     m_gmat[8] = m_Sl[2].y;
    matrix::transpose33(m_gmat);
    matrix::invert33(m_gmat);

    m_S01l.set(globalToLocal(m_Edge[2].segment()));
    m_S02l.set(globalToLocal(m_Edge[1].segment()));
    m_S12l.set(globalToLocal(m_Edge[0].segment()));

    m_SignedArea = ((m_Sl[1].x-m_Sl[0].x)*(m_Sl[2].y-m_Sl[0].y) - (m_Sl[2].x-m_Sl[0].x)*(m_Sl[1].y-m_Sl[0].y)) / 2.0;
    assert(m_SignedArea>0.0); // positive orientations only
//    if(m_SignedArea<0.0 || !m_bPositiveOrientation)  qDebug(" index=%3d   %1d  %g", m_index, m_bPositiveOrientation, m_SignedArea);

    // calculate the integrals of x.b_i(x,y) and y.b_i(x,y) - needed later for distant field approximation
    GQTriangle gq(5); // x.basis functions are second order
    double integrand_x=0, integrand_y=0;
    double sum_x[]{0.0,0.0,0.0}, sum_y[] {0.0,0.0,0.0};

    sum_x[0]=sum_x[1]=sum_x[2]=0.0;
    sum_y[0]=sum_y[1]=sum_y[2]=0.0;

    for(int i=0; i<gq.nPoints(); i++)
    {
        double x = m_Sl[0].x*(1.0-gq.points().at(i).x-gq.points().at(i).y) + m_Sl[1].x*gq.points().at(i).x + m_Sl[2].x*gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-gq.points().at(i).x-gq.points().at(i).y) + m_Sl[1].y*gq.points().at(i).x + m_Sl[2].y*gq.points().at(i).y;

        for(int l=0; l<3; l++)
        {
            integrand_x = gq.weights().at(i) * x*basis(x,y,l);
            sum_x[l] += integrand_x;

            integrand_y = gq.weights().at(i) * y*basis(x,y,l);
            sum_y[l] += integrand_y;

        }
    }
    //multiply the result by the Jacobian of the transformation from local to normalized |J|=2xArea
    for(int l=0; l<3; l++)
    {
        bx[l] = sum_x[l] * 2.0*m_SignedArea;
        by[l] = sum_y[l] * 2.0*m_SignedArea;
    }
}


void Panel3::makeGQCoeffs()
{
    s_gq.makeCoeffs(s_iQuadratureOrder);
}


/** Initializes member variables to zero */
void Panel3::initialize()
{
    m_bNullTriangle = true;
    m_Angle[0] = m_Angle[1] = m_Angle[2] = 0.0;
    m_SignedArea  = 0.0;
    m_mu[0] = m_mu[1] = m_mu[2] = 1.0;
    m_beta[0] = 1.0; m_beta[1]=0.0; m_beta[2]=0.0;

    m_S[0].setIndex(-1);
    m_S[1].setIndex(-1);
    m_S[2].setIndex(-1);
    clearConnections();

    memset(m_gmat, 0, 9*sizeof(double));

    m_bIsLeftPanel = false;
    m_bFromSTL = false;
    m_iOppositeIndex = -1;
}


void Panel3::makeXZsymmetric()
{
    m_S[0].y = -m_S[0].y;

    double S1x = m_S[1].x;
    double S1y = m_S[1].y;
    double S1z = m_S[1].z;

    m_S[1].x =  m_S[2].x;
    m_S[1].y = -m_S[2].y;
    m_S[1].z =  m_S[2].z;

    m_S[2].x =  S1x;
    m_S[2].y = -S1y;
    m_S[2].z =  S1z;

    setFrame();
}


void Panel3::scalePanel(double sx, double sy, double sz)
{
    m_S[0].x *= sx;
    m_S[0].y *= sy;
    m_S[0].z *= sz;

    m_S[1].x *= sx;
    m_S[1].y *= sy;
    m_S[1].z *= sz;

    m_S[2].x *= sx;
    m_S[2].y *= sy;
    m_S[2].z *= sz;

    setFrame();
}


/**
 * DEBUG only
 * @brief Calculates the panel influence of a uniform source function at a field point using Gauss-Legendre quadrature
 * @param phi the potential at the field point
 * @param Pt the field point where the influence is calculated, in global coordinates
 */
void Panel3::sourceQuadraturePotential(Vector3d ptGlobal, double &phi) const
{
    Vector3d ptL = globalToLocalPosition(ptGlobal);
    double sumPhi = 0.0;

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        double r = sqrt((ptL.x-x)*(ptL.x-x) + (ptL.y-y)*(ptL.y-y) + ptL.z*ptL.z);

        sumPhi +=   (-1.0 /r) * s_gq.weights().at(i);
    }
    phi = sumPhi * fabs(m_SignedArea);
}



/**
 * DEBUG only
 * @brief Calculates the panel influence of a uniform source function at a field point using Gauss-Legendre quadrature
 * @param phi the potential at the field point
 * @param Pt the field point where the influence is calculated, in global coordinates
 */
void Panel3::sourceQuadratureVelocity(Vector3d ptGlobal, Vector3d &V) const
{
    Vector3d ptL = globalToLocalPosition(ptGlobal);
    Vector3d sumV(0.0,0,0);

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        double r = sqrt((ptL.x-x)*(ptL.x-x) + (ptL.y-y)*(ptL.y-y) + ptL.z*ptL.z);

        double r3 = r*r*r;
        sumV.x +=   (ptL.x /r3) * s_gq.weights().at(i);
        sumV.y +=   (ptL.y /r3) * s_gq.weights().at(i);
        sumV.z +=   (ptL.z /r3) * s_gq.weights().at(i);
    }
    V = sumV * fabs(m_SignedArea);
}


/**
 * @brief Calculates the elementary integrals at a field point using M. Carley's procedure.
 * cf. Potential integrals on triangles, Michael Carley, January 25, 2012. <br>
 *   \f$ G1[0] = \int  1    /R   \: dS \f$ <br>
 *   \f$ G1[1] = \int  x    /R   \: dS \f$ <br>
 *   \f$ G1[2] = \int  y    /R   \: dS \f$ <br>
 *   \f$ G3[0] = \int  1    /R^3 \: dS \f$ <br>
 *   \f$ G3[1] = \int  x    /R^3 \: dS \f$ <br>
 *   \f$ G3[2] = \int  y    /R^3 \: dS \f$ <br>
 *   \f$ G3[3] = \int  x^2  /R^3 \: dS \f$ <br>
 *   \f$ G3[4] = \int  x.y  /R^3 \: dS \f$ <br>
 *   \f$ G3[5] = \int  y^2  /R^3 \: dS \f$ <br>
 *   \f$ G5[0] = \int  1    /R^5 \: dS \f$ <br>
 *   \f$ G5[1] = \int  x    /R^5 \: dS \f$ <br>
 *   \f$ G5[2] = \int  y    /R^5 \: dS \f$ <br>
 *   \f$ G5[3] = \int  x^2  /R^5 \: dS \f$ <br>
 *   \f$ G5[4] = \int  x.y  /R^5 \: dS \f$ <br>
 *   \f$ G5[5] = \int  y^2  /R^5 \: dS \f$ <br>
 * @param Pt the field point where the influence is calculated, in local coordinates
 * @param bSelf true if the Pt lies in the triangle's plane
 * @return the value of the integral at point Pt
 */
void Panel3::computeMCIntegrals(const Vector3d &FieldPtGlobal, bool bInPlane, double *G1, double *G3, double *G5, bool bGradients)const
{

    MCTriangle trP01, trP12, trP20;
    Vector3d Normal(0.0,0.0,1.0);

    Vector3d ptlocal;
    m_CF.globalToLocalPosition(FieldPtGlobal, ptlocal);

    // construct 3 triangles i.a.w. MC figure 2
    // get the integrals of table 1
    trP01.setTriangle(ptlocal, m_Sl[0], m_Sl[1], Normal);    // = triangle 012
    if(trP01.isValid()) trP01.integrate(ptlocal, bInPlane, G1, G3, G5, bGradients);

    trP12.setTriangle(ptlocal, m_Sl[1], m_Sl[2], Normal);    // = triangle 023
    if(trP12.isValid()) trP12.integrate(ptlocal, bInPlane, G1, G3, G5, bGradients);

    trP20.setTriangle(ptlocal, m_Sl[2], m_Sl[0], Normal);    // = triangle 031
    if(trP20.isValid()) trP20.integrate(ptlocal, bInPlane, G1, G3, G5, bGradients);
}


void Panel3::rotate(Vector3d const &HA, Quaternion &Qt)
{
    Vector3d VTemp;
    for(int i=0; i<3; i++)
    {
        VTemp.x = m_S[i].x - HA.x;
        VTemp.y = m_S[i].y - HA.y;
        VTemp.z = m_S[i].z - HA.z;
        Qt.conjugate(VTemp);
        m_S[i].x = VTemp.x + HA.x;
        m_S[i].y = VTemp.y + HA.y;
        m_S[i].z = VTemp.z + HA.z;
    }
    setFrame();
}


void Panel3::rotate(Vector3d const &HA, Vector3d const &Axis, double angle)
{
    for(int i=0; i<3; i++)
    {
        m_S[i].rotate(HA, Axis, angle);
    }
    setFrame();
}


std::string Panel3::properties(bool bLong) const
{
    QString props, strange;

    props = QString::asprintf("Triangular Panel %d\n", m_index);

    if(m_bNullTriangle) props +="   ***** NULL triangle *****\n";


    for(int in=0; in<3; in++)
    {
        strange = QString::asprintf("  Node(%4d) = (%9.3f, %9.3f, %9.3f) ", m_S[in].index(),
                                    round(m_S[in].x*Units::mtoUnit()*1000.0)/1000.0,
                                    round(m_S[in].y*Units::mtoUnit()*1000.0)/1000.0,
                                    round(m_S[in].z*Units::mtoUnit()*1000.0)/1000.0);
        strange += Units::lengthUnitQLabel() + "\n";
        props += strange;
    }

    strange = QString::asprintf("  Normal  = (%9.3f, %9.3f, %9.3f)\n", m_Normal.x, m_Normal.y, m_Normal.z);
    props += strange;

    strange = QString::asprintf("  CoG     = (%9.3f, %9.3f, %9.3f) ",
                                round(m_CoG_g.x*Units::mtoUnit()*1000.0)/1000.0,
                                round(m_CoG_g.y*Units::mtoUnit()*1000.0)/1000.0,
                                round(m_CoG_g.z*Units::mtoUnit()*1000.0)/1000.0);
    strange += Units::lengthUnitQLabel() + "\n";
    props += strange;

    strange = QString::asprintf("  Area    = %.5g ", m_SignedArea*Units::m2toUnit());
    props += strange + Units::areaUnitQLabel() + "\n";

    strange = QString::asprintf("  Angles  = (%5.1f, %5.1f, %5.1f) ", m_Angle[0], m_Angle[1], m_Angle[2]);
    props += strange + DEGch + "\n";

    strange = QString::asprintf("  Edges   = (%9.3f, %9.3f, %9.3f) ",
                                round(edge(0).length()*Units::mtoUnit()*1000.0)/1000.0,
                                round(edge(1).length()*Units::mtoUnit()*1000.0)/1000.0,
                                round(edge(2).length()*Units::mtoUnit()*1000.0)/1000.0);
    strange += Units::lengthUnitQLabel();
    props += strange;

    if(isSkinny())
    {
        props += "\n";
        double q=0,r=0,e=0;
        q = qualityFactor(r,e);
        props += "  Triangle is skinny:\n";
        strange = QString::asprintf("    quality factor = %7g > %7g\n", q, s_Quality);
        props += strange;
        strange = QString::asprintf("    circumradius   = %7g ", r*Units::mtoUnit());
        strange += Units::lengthUnitQLabel();
        props += strange;
    }


    props += "\n";
    switch(m_Pos)
    {
        case xfl::BOTSURFACE:  strange = "  BOTTOM SURFACE\n";   break;
        case xfl::MIDSURFACE:  strange = "  MIDDLE SURFACE\n";   break;
        case xfl::TOPSURFACE:  strange = "  TOP SURFACE\n";      break;
        case xfl::SIDESURFACE: strange = "  SIDE SURFACE\n";     break;
        case xfl::FUSESURFACE: strange = "  FUSE SURFACE\n";     break;
        case xfl::WAKESURFACE: strange = "  WAKE PANEL\n";       break;
        case xfl::NOSURFACE:   strange = "  NO SURFACE\n";       break;
    }
    props += strange;

    strange = QString::asprintf("  Surface index= %d\n", m_SurfaceIndex);
    props += strange;

    strange = QString::asprintf("  Vertex indexes:  %4d  %4d  %4d\n", m_S[0].index(), m_S[1].index(), m_S[2].index());
    props += strange;

    strange = QString::asprintf("  Neighbours:      %4d  %4d  %4d\n", m_Neighbour[0], m_Neighbour[1], m_Neighbour[2]);
    props += strange;
    strange = QString::asprintf("                   PU=%d  PD=%d  PL=%d  PR=%d\n", m_iPU, m_iPD, m_iPL, m_iPR);
    props += strange;

    if(isPositiveOrientation()) props += "  Positive orientation";
    else                        props += "  Negative orientation";

    if(isTrailing())
    {
        props += "\n";
        props += "  Panel is trailing:\n";
        strange = QString::asprintf("    Downstream wake panel  index = %d\n", m_iWake);
        props += strange;
        strange = QString::asprintf("    Downstream wake column index = %d", m_iWakeColumn);
        props += strange;
    }
    else if (isLeading())
    {
        props += "\n";
        props += "  Panel is leading";
    }
    if(m_bIsTrailing)
    {
        props +="\n";
        strange = QString::asprintf("  Opposite index = %d", m_iOppositeIndex);
        props += strange;
    }

    if(!bLong) return props.toStdString();

    props += "\n  Local frame:\n";
    strange = QString::asprintf("    l      = (%7.2f, %7.2f, %7.2f)\n", m_l.x, m_l.y, m_l.z);
    props += strange;
    strange = QString::asprintf("    m      = (%7.2f, %7.2f, %7.2f)\n", m_m.x, m_m.y, m_m.z);
    props += strange;
    strange = QString::asprintf("    Normal = (%7.2f, %7.2f, %7.2f)", m_Normal.x, m_Normal.y, m_Normal.z);
    props += strange;

    return props.toStdString();
}


void Panel3::translate(double tx, double ty, double tz)
{
    m_S[0].translate(tx, ty, tz);
    m_S[1].translate(tx, ty, tz);
    m_S[2].translate(tx, ty, tz);
    setFrame();
}


/**
 * DEBUG ONLY
 */
void Panel3::quadratureIntegrals(Vector3d Pt, double *I1, double *I3, double *I5) const
{
    Vector3d Ptl = globalToLocalPosition(Pt);
    Vector3d R;
    double sumPotential[3];
    sumPotential[0] = sumPotential[1] = sumPotential[2] = 0.0;
    GQTriangle gq(8);

    I1[0]=I1[1]=I1[2]=0.0;
    I3[0]=I3[1]=I3[2]=I3[3]=I3[4]=I3[5]=0.0;
    I5[0]=I5[1]=I5[2]=I5[3]=I5[4]=I5[5]=0.0;

    for(uint i=0; i<gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-gq.points().at(i).x-gq.points().at(i).y) + m_Sl[1].x*gq.points().at(i).x + m_Sl[2].x*gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-gq.points().at(i).x-gq.points().at(i).y) + m_Sl[1].y*gq.points().at(i).x + m_Sl[2].y*gq.points().at(i).y;
        //qDebug(" %13.7f   %13.7f",x,y);
        R = Ptl - Vector3d(x,y,0.0);
        double r  = R.norm();
        double r3 = r*r*r;
        double r5 = r*r*r*r*r;

        I1[0] +=  (1/r) * gq.weights().at(i);
        I1[1] +=  (x/r) * gq.weights().at(i);
        I1[2] +=  (y/r) * gq.weights().at(i);

        I3[0] +=  (1/r3)   * gq.weights().at(i);
        I3[1] +=  (x/r3)   * gq.weights().at(i);
        I3[2] +=  (y/r3)   * gq.weights().at(i);
        I3[3] +=  (x*x/r3) * gq.weights().at(i);
        I3[4] +=  (x*y/r3) * gq.weights().at(i);
        I3[5] +=  (y*y/r3) * gq.weights().at(i);

        I5[0] +=  (1/r5)   * gq.weights().at(i);
        I5[1] +=  (x/r5)   * gq.weights().at(i);
        I5[2] +=  (y/r5)   * gq.weights().at(i);
        I5[3] +=  (x*x/r5) * gq.weights().at(i);
        I5[4] +=  (x*y/r5) * gq.weights().at(i);
        I5[5] +=  (y*y/r5) * gq.weights().at(i);
    }

    for(int i=0; i<3; i++) I1[i] *= fabs(m_SignedArea);
    for(int i=0; i<6; i++) I3[i] *= fabs(m_SignedArea);
    for(int i=0; i<6; i++) I5[i] *= fabs(m_SignedArea);
}



void Panel3::computeNFIntegrals_ref(Vector3d const &FieldPt, double *G1, double *G3, double *G5, bool bGradients) const
{
    // integrals in "NF" coordinate system with origin at field point and axis along first edge
    double N1[3], N3[6], N5[6];
    memset(N1, 0, 3*sizeof(double));
    memset(N3, 0, 6*sizeof(double));
    memset(N5, 0, 6*sizeof(double));

    Vector3d R = FieldPt-m_S[0];

    // get the projection of the field point on the triangle's plane
    double eta = R.dot(m_Normal);

    if(fabs(eta)<INPLANEPRECISION)
    {
        // method not applicable for in-plane field points:
        return;
    }

    Vector3d X = R - m_Normal*eta;
    Vector3d Xl = m_CF.globalToLocalPosition(X);
    //  introduce in R³ a local coordinate system {x; ξ, ζ, η} associated with Eq
    CartesianFrame NF(FieldPt, m_l, m_m, m_Normal);

    // implement eq.15
    Vector3d r[3];
    r[0] = NF.globalToLocalPosition(m_S[0]);
    r[1] = NF.globalToLocalPosition(m_S[1]);
    r[2] = NF.globalToLocalPosition(m_S[2]);

    // calculate vertex angles
    double theta[3], det, dot;
    Vector3d u0, u1, u2;

    // first angle
    u0 = m_Edge[2].unitDir();
    u1 = m_Edge[1].unitDir() * (-1.0);
    dot = u0.x*u1.x + u0.y*u1.y;
    det = u0.x*u1.y - u0.y*u1.x;
    theta[0] = atan2(det, dot);

    // second angle
    u1 = m_Edge[0].unitDir();
    u0 = m_Edge[2].unitDir() * (-1.0);
    dot = u1.x*u0.x + u1.y*u0.y;
    det = u1.x*u0.y - u1.y*u0.x;
    theta[1] = atan2(det, dot);

    // third angle
    u2 = m_Edge[1].unitDir();
    u1 = m_Edge[0].unitDir() * (-1.0);
    dot = u2.x*u1.x + u2.y*u1.y;
    det = u2.x*u1.y - u2.y*u1.x;
    theta[2] = atan2(det, dot);
    // check that sum of internal angles = PI
    //    qDebug(" anglesum = PI = %13.7f", theta[0]+theta[1]+theta[2]);

    // implement eq. 21
    double alfa[3], cosa[3], sina[3];
    alfa[0]=0.0;              cosa[0]=1.0;               sina[0]=0.0;
    alfa[1]=PI-theta[1];      cosa[1]=cos(alfa[1]);      sina[1]=sin(alfa[1]);
    alfa[2]=PI+theta[0];      cosa[2]=cos(alfa[2]);      sina[2]=sin(alfa[2]);

    // make edge-aligned cartesian frames centered on field point
    CartesianFrame NFe[3];
    NFe[0].setFrame(FieldPt, m_Edge[2].unitDir(), m_Normal*m_Edge[2].unitDir(), m_Normal);
    NFe[1].setFrame(FieldPt, m_Edge[0].unitDir(), m_Normal*m_Edge[0].unitDir(), m_Normal);
    NFe[2].setFrame(FieldPt, m_Edge[1].unitDir(), m_Normal*m_Edge[1].unitDir(), m_Normal);

    // check eq. 22
    /*    Vector3d ptGlobal(-2,7,3); // random point
    Vector3d ptl = CF0.globalToLocalPosition(ptGlobal);
    Vector3d l0 = CFe[0].globalToLocalPosition(ptGlobal);
    Vector3d l1 = CFe[1].globalToLocalPosition(ptGlobal);
    Vector3d l2 = CFe[2].globalToLocalPosition(ptGlobal);
    qDebug("  %13.7f  %13.7f  %13.7f  %13.7f", l0.x, l0.y, ptl.x*cos(alfa[0]) + ptl.y*sin(alfa[0]), -ptl.x*sin(alfa[0]) + ptl.y*cos(alfa[0]));
    qDebug("  %13.7f  %13.7f  %13.7f  %13.7f", l1.x, l1.y, ptl.x*cos(alfa[1]) + ptl.y*sin(alfa[1]), -ptl.x*sin(alfa[1]) + ptl.y*cos(alfa[1]));
    qDebug("  %13.7f  %13.7f  %13.7f  %13.7f", l2.x, l2.y, ptl.x*cos(alfa[2]) + ptl.y*sin(alfa[2]), -ptl.x*sin(alfa[2]) + ptl.y*cos(alfa[2]));
*/

    // check eq. 23
    double rho[4];
    Vector3d vl[3][4];
    for(int i=0; i<3; i++)
    {
        NFe[i].globalToLocalPosition(m_S[0], vl[i][0]);
        NFe[i].globalToLocalPosition(m_S[1], vl[i][1]);
        NFe[i].globalToLocalPosition(m_S[2], vl[i][2]);
        vl[i][3] = vl[i][0];
        rho[0] = vl[i][0].norm();
        rho[1] = vl[i][1].norm();
        rho[2] = vl[i][2].norm();
        //        qDebug("  %13.7f  %13.7f  %13.7f ", rho[0], rho[1], rho[2]); // distance is independant of the reference frame
    }
    rho[3]=rho[0];

    double q[4];
    for(int i=0; i<3; i++)  q[i] = vl[i][i].y;
    q[3] = q[0];
    /*    // check eq. 24
    qDebug("q0=  %13.7f  %13.7f  ", q[0], vl[0][1].y); // q00=q01 = q1
    qDebug("q1=  %13.7f  %13.7f  ", q[1], vl[1][2].y); // q11=q12 = q2
    qDebug("q2=  %13.7f  %13.7f  ", q[2], vl[2][0].y); // q22=q00 = q3 */

    double rho_t[3],d[4];
    for(int i=0; i<3; i++)
    {
        d[i]     = q[i]*q[i] + eta*eta;
        rho_t[i] = rho[i] - rho[i+1];
    }
    d[3]=d[0];

    // implement eq. 25
    double chi[3], delta[3], L[3], gamma[3];
    for(int i=0; i<3; i++)
    {
        double pii  = vl[i][i].x;
        double pii1 = vl[i][i+1].x;

        gamma[i]  = atan2(-2.0*pii  *q[i]*eta*rho[i],   (q[i]*q[i]*rho[i]*rho[i]     -pii*pii  *eta*eta));
        gamma[i] -= atan2(-2.0*pii1 *q[i]*eta*rho[i+1], (q[i]*q[i]*rho[i+1]*rho[i+1] -pii1*pii1*eta*eta));

        chi[i]     = log(pii+rho[i]) - log(pii1+rho[i+1]);

        delta[i]   = pii/rho[i] - pii1/rho[i+1];
        L[i]       = 1.0/rho[i] - 1.0/rho[i+1];
    }

    // implement eq. 26
    double thet0=0;
    int iVertex, iEdge;
    //most probable first
    TRIANGLE::enumPointPosition pos = pointPosition(Xl.x, Xl.y, iVertex, iEdge);
    switch(pos)
    {
        case TRIANGLE::Inside:
        {
            thet0 = 2.0*PI;
            break;
        }
        case TRIANGLE::OnVertex:
        {
            thet0 = theta[iVertex];
            break;
        }
        case TRIANGLE::OnEdge:
        {
            thet0 = PI;
            break;
        }
        case TRIANGLE::Outside:
        {
            thet0 = 0.0;
            break;
        }
    }

    // implement eq. 27
    double sign = (eta>=0.0) ? 1.0 : -1.0;
    double thetx = 0.5*(gamma[0]+gamma[1]+gamma[2]) + sign *thet0;

    if(G1)
    {
        // implement eq. 28
        N1[0] = -eta*thetx;                                     // I1
        for(int i=0; i<3; i++) N1[0] += q[i]*chi[i];

        N1[1] = N1[2] = 0.0;                                           // I1x, I1y
        for(int i=0; i<3; i++)
        {
            N1[1] += q[i]*rho_t[i]*cosa[i] - d[i]*chi[i]*sina[i];
            N1[2] += q[i]*rho_t[i]*sina[i] + d[i]*chi[i]*cosa[i];
        }
        N1[1] *= 0.5;
        N1[2] *= 0.5;
    }


    if(G3)
    {
        // implement eq. 29
        N3[0] = 1.0/eta * thetx;                       // I3
        N3[1] = N3[2] = 0.0;                           // I3x, I3y
        for(int i=0; i<3; i++)
        {
            N3[1] += chi[i]*sina[i];
            N3[2] -= chi[i]*cosa[i];
        }

        if(bGradients)
        {
            // implement eq. 30
            N3[3]=N3[4]=N3[5]=0.0;                         // I3xx, I3xy, I3yy
            for(int i=0; i<3; i++)
            {
                N3[3] += (q[i]*chi[i]*cosa[i] + rho_t[i]*sina[i]) * cosa[i];  // I3xx
                N3[4] += (q[i]*chi[i]*cosa[i] + rho_t[i]*sina[i]) * sina[i];  // I3xy
                N3[5] += (q[i]*chi[i]*sina[i] - rho_t[i]*cosa[i]) * sina[i];  // I3yy
            }
            N3[3] -= eta*thetx;
            N3[5] -= eta*thetx;
        }
    }

    if(G5)
    {
        // implement eq. 31
        N5[0]=N5[1]=N5[2]=0.0;                         // I5, I5x, I5y
        for(int i=0; i<3; i++)
        {
            N5[0]  += q[i]/d[i]*delta[i];    // I5
            N5[1] += delta[i]/d[i]*sina[i];  // I5x
            N5[2] -= delta[i]/d[i]*cosa[i];  // I5y
        }
        N5[0] = N5[0]/3.0/eta/eta + 1.0/3.0/eta/eta/eta*thetx;
        N5[1] *= 1.0/3.0;
        N5[2] *= 1.0/3.0;

        if(bGradients)
        {
            // implement eq. 32
            N5[3]=N5[4]=N5[5]=0.0;
            for(int i=0; i<3; i++)
            {
                N5[3] += (L[i]*cosa[i] + q[i]/d[i]*delta[i]*sina[i]) *sina[i];  // I5xx
                N5[4] += (L[i]*sina[i] - q[i]/d[i]*delta[i]*cosa[i]) *sina[i];  // I5xy
                N5[5] += (L[i]*sina[i] - q[i]/d[i]*delta[i]*cosa[i]) *cosa[i];  // I5yy
            }
            N5[3]  = N5[3] * (-1./3.) + thetx/3.0/eta;
            N5[4] *= -1.0/3.0;
            N5[5]  = N5[5] * ( 1./3.) + thetx/3.0/eta;
        }
    }


    // Change origin from field point to panel's origin
    // first angle
    u0 = m_Edge[2].unitDir();
    double psi = atan2(m_l.y, m_l.x);
    psi = 0.0;
    //    double psi_deg = psi *180.0/PI;
    double cosp = cos(psi);
    double sinp = sin(psi);
    double dx = FieldPt.x - m_CoG_g.x;
    double dy = FieldPt.y - m_CoG_g.y;

    // Convert the integrals into the panel's reference frame

    /*    if(G1)
    {
        G1[0] = N1[0];
        G1[1] = N1[1] + dx*N1[0];
        G1[2] = N1[2] + dy*N1[0];
    }
    if(G3)
    {
        G3[0] = N3[0];                                // is Int(1/R³)
        G3[1] = N3[1] + dx*N3[0];                     // is Int(x/R³)
        G3[2] = N3[2] + dy*N3[0];                     // is Int(y/R³)
        if(bGradients)
        {
            G3[3] = dx*dx*N3[0] + 2.0*dx*N3[1]            + N3[3];   // is Int(xx/R³)
            G3[4] = dx*dy*N3[0] +     dx*N3[2] + dy*N3[1] + N3[4];   // is Int(xy/R³)
            G3[5] = dy*dy*N3[0] + 2.0*dy*N3[2]            + N3[5];   // is Int(yy/R³)
        }
    }
    if(G5)
    {
        G5[0] = N5[0];
        G5[1] = N5[1] + dx*N5[0];
        G5[2] = N5[2] + dy*N5[0];
        if(bGradients)
        {
            G5[3] = dx*dx*N5[0] + 2.0*dx*N5[1]            + N5[3];
            G5[4] = dx*dy*N5[0] +     dx*N5[2] + dy*N5[1] + N5[4];
            G5[5] = dy*dy*N5[0] + 2.0*dy*N5[2]            + N5[5];
        }
    }
*/
    if(G1)
    {
        G1[0]  = N1[0];                                // is Int(1/R)
        G1[1]  = cosp*N1[1] - sinp*N1[2] + dx*N1[0];   // is Int(x/R)
        G1[2]  = sinp*N1[1] + cosp*N1[2] + dy*N1[0];   // is Int(y/R)
    }
    if(G3)
    {
        G3[0]  = N3[0];                                // is Int(1/R³)
        G3[1]  = cosp*N3[1] - sinp*N3[2] + dx*N3[0];   // is Int(x/R³)
        G3[2]  = sinp*N3[1] + cosp*N3[2] + dy*N3[0];   // is Int(y/R³)

        if(bGradients)
        {
            //second order: xx/R³, xy/R³, yy/R³
            G3[3] = dx*dx*N3[0] + cosp*cosp*N3[3] + sinp*sinp*N3[5] + 2.0*dx*cosp*N3[1] - 2.0*dx*sinp*N3[2] - 2.0*sinp*cosp*N3[4];
            G3[4] = dx*dy*N3[0] + (dx*sinp+dy*cosp)*N3[1] + (dx*cosp-dy*sinp)*N3[2] + sinp*cosp*N3[3]    + (cosp*cosp-sinp*sinp)*N3[4] - sinp*cosp*N3[5];
            G3[5] = dy*dy*N3[0] + sinp*sinp*N3[3] + cosp*cosp*N3[5] + 2.0*dy*sinp*N3[1] + 2.0*dy*cosp*N3[2] + 2.0*sinp*cosp*N3[4];
        }
    }

    if(G5)
    {
        G5[0]  = N5[0];                                // is Int(1/R⁵)
        G5[1]  = cosp*N5[1] - sinp*N5[2] + dx*N5[0];   // is Int(x/R⁵)
        G5[2]  = sinp*N5[1] + cosp*N5[2] + dy*N5[0];   // is Int(y/R⁵)

        if(bGradients)
        {
            //second order: xx/r⁵, xy/r⁵, yy/r⁵
            G5[3] = dx*dx*N5[0] + cosp*cosp*N5[3] + sinp*sinp*N5[5] + 2.0*dx*cosp*N5[1] - 2.0*dx*sinp*N5[2] - 2.0*sinp*cosp*N5[4];
            G5[4] = dx*dy*N5[0] + (dx*sinp+dy*cosp)*N5[1] + (dx*cosp-dy*sinp)*N5[2] + sinp*cosp*N5[3]    + (cosp*cosp-sinp*sinp)*N5[4] - sinp*cosp*N5[5];
            G5[5] = dy*dy*N5[0] + sinp*sinp*N5[3] + cosp*cosp*N5[5] + 2.0*dy*sinp*N5[1] + 2.0*dy*cosp*N5[2] + 2.0*sinp*cosp*N5[4];
        }
    }
}


/**
 * Implements the method given in
 *    "Explicit expressions for 3D boundary integrals in potential theory"
 *    S. Nintcheu Fata
 *    INTERNATIONAL JOURNAL FOR NUMERICAL METHODS IN ENGINEERING
 *    Int. J. Numer. Meth. Engng 2009; 78:32–47
 *
 * Note:
 *    - Used only for off-plane evaluations, since the method is singular for z-->0
 *    - The method is unstable for field points with orthogonal projection on the
 *      edge due to atan2 quadrant issues: double numerical error causes the field
 *      point to lie randomly on either side of the edge, and the difference of the
 *      two atan2 is also random. To prevent this, such points are moved slightly
 *      outside the triangle. Since the off-plane integrals are space-continuous and
 *      smooth, this has negligible impact on results.
 *
 * @todo optimize for speed
 */
void Panel3::computeNFIntegrals(Vector3d const &FieldPtGlobal, double *G1, double *G3, double *G5, bool bGradients) const
{
     double N1[] = {0,0,0};
     double N3[] = {0,0,0,0,0,0};
     double N5[] = {0,0,0,0,0,0};

     Vector3d FieldPtLocal;
     m_CF.globalToLocalPosition(FieldPtGlobal, FieldPtLocal);
     Vector3d EdgeL[3];
     Vector3d NormalL;
     Vector3d u[4];

     //unneeded
     Vector3d R(FieldPtLocal-m_Sl[0]);

     // make normalized local edges
     EdgeL[0].x = m_Sl[1].x-m_Sl[0].x;
     EdgeL[0].y = m_Sl[1].y-m_Sl[0].y;
     EdgeL[0].z = m_Sl[1].z-m_Sl[0].z;
     EdgeL[1].x = m_Sl[2].x-m_Sl[1].x;
     EdgeL[1].y = m_Sl[2].y-m_Sl[1].y;
     EdgeL[1].z = m_Sl[2].z-m_Sl[1].z;
     EdgeL[2].x = m_Sl[0].x-m_Sl[2].x;
     EdgeL[2].y = m_Sl[0].y-m_Sl[2].y;
     EdgeL[2].z = m_Sl[0].z-m_Sl[2].z;

     NormalL.set(0,0, 1.0);

     for(int i=0; i<3; i++)    u[i].set(EdgeL[i].normalized());
     u[3].set(u[0]);

     // make edge-aligned cartesian frames centered on field point
     CartesianFrame NFe[3];
     for(int i=0; i<3; i++)
         NFe[i].setFrame(FieldPtLocal, u[i], Vector3d(-u[i].y*NormalL.z, u[i].x*NormalL.z, 0.0), NormalL);

     int iVertex=-1, iEdge=-1;
     TRIANGLE::enumPointPosition pointposition = pointPosition(FieldPtLocal.x, FieldPtLocal.y, iVertex, iEdge);
     // if the point falls on a vertex or on an edge, move it slightly
     // outside to prevent atan2 quadrant issues.
     // Since this is an off-plane calculation, the field point is not on
     // the edge itself and the integrals are continuous in space.
     // In-plane integrals have been redirected to MC method.
     if(pointposition==TRIANGLE::OnVertex)
     {
         // Move the point radially outward from the CoG
         // The translation length depends on the panel's size measured by the
         // distance of the CoG to the vertex
         FieldPtLocal = m_CoG_l+ (m_Sl[iVertex]-m_CoG_l)* 1.00001;
         pointposition = pointPosition(FieldPtLocal.x, FieldPtLocal.y, iVertex, iEdge);
         assert(pointposition==TRIANGLE::Outside);
     }
     else if(pointposition==TRIANGLE::OnEdge)
     {
         // Move the point radially outward along the edge's perpendicular vector
         // The translation length depends on the panel's size measured by the
         // distance of the CoG to the edge
         //        FieldPtLocal += NFe[iEdge].J * (-0.0001);
         double characteristiclength = geom::distanceToLine3d(m_Edge[iEdge].vertexAt(0), m_Edge[iEdge].vertexAt(1), m_CoG_g);
         FieldPtLocal += NFe[iEdge].Jdir() * characteristiclength * -0.001;
         NFe[iEdge].setOrigin(FieldPtLocal);
         pointposition = pointPosition(FieldPtLocal.x, FieldPtLocal.y, iVertex, iEdge);
         assert(pointposition==TRIANGLE::Outside);
     }

     // get the projection of the field point on the triangle's plane
     double eta = R.dot(NormalL);

     if(fabs(eta)<INPLANEPRECISION)
     {
         // method not applicable for in-plane field points
         return;
     }

     // unneeded: implement eq.15
     /*    Vector3d r[3];
     r[0] = Sl[0]-FieldPtLocal;
     r[1] = Sl[1]-FieldPtLocal;
     r[2] = Sl[2]-FieldPtLocal;*/

     // calculate vertex angles
     double theta[3], det, dot;
     dot =  u[0].x*(-u[2].x) + u[0].y*(-u[2].y);
     det =  u[0].x*(-u[2].y) - u[0].y*(-u[2].x);
     theta[0] = atan2(det, dot);

     dot =  u[1].x*(-u[0].x) + u[1].y*(-u[0].y);
     det =  u[1].x*(-u[0].y) - u[1].y*(-u[0].x);
     theta[1] = atan2(det, dot);

     dot =  u[2].x*(-u[1].x) + u[2].y*(-u[1].y);
     det =  u[2].x*(-u[1].y) - u[2].y*(-u[1].x);
     theta[2] = atan2(det, dot);

     for(int i=0; i<3; i++)
     {
         assert(theta[i]>=0.0);
     }
     // check that sum of internal angles = PI
     //    qDebug(" theta[] =  %13.7f  %13.7f  %13.7f", theta[0], theta[1], theta[2]);

     assert(fabs(theta[0]+theta[1]+theta[2]-PI)<0.0001);

     // implement eq. 21
     double alfa[3], cosa[3], sina[3];
     alfa[0]=0.0;              cosa[0]=1.0;               sina[0]=0.0;
     alfa[1]=PI-theta[1];      cosa[1]=cos(alfa[1]);      sina[1]=sin(alfa[1]);
     alfa[2]=PI+theta[0];      cosa[2]=cos(alfa[2]);      sina[2]=sin(alfa[2]);

     /*
     //  unneeded: introduce in R³ a local coordinate system {x; ξ, ζ, η} associated with the triangle E_q
     CartesianFrame NF(FieldPtLocal, u0, Vector3d(-u0.y, +u0.x, 0.0), NormalL);
     // check eq. 22
     Vector3d ptGlobal(-2,7,3); // random point
     Vector3d ptl = NF.globalToLocalPosition(ptGlobal);
     Vector3d l0 = NFe[0].globalToLocalPosition(ptGlobal);
     Vector3d l1 = NFe[1].globalToLocalPosition(ptGlobal);
     Vector3d l2 = NFe[2].globalToLocalPosition(ptGlobal);
     qDebug("  %13.7f  %13.7f  %13.7f  %13.7f", l0.x, l0.y, ptl.x*cos(alfa[0]) + ptl.y*sin(alfa[0]), -ptl.x*sin(alfa[0]) + ptl.y*cos(alfa[0]));
     qDebug("  %13.7f  %13.7f  %13.7f  %13.7f", l1.x, l1.y, ptl.x*cos(alfa[1]) + ptl.y*sin(alfa[1]), -ptl.x*sin(alfa[1]) + ptl.y*cos(alfa[1]));
     qDebug("  %13.7f  %13.7f  %13.7f  %13.7f", l2.x, l2.y, ptl.x*cos(alfa[2]) + ptl.y*sin(alfa[2]), -ptl.x*sin(alfa[2]) + ptl.y*cos(alfa[2]));*/


     // check eq. 23
     double rho[] = {0,0,0,0};
     Vector3d vl[3][4];
     for(int i=0; i<3; i++)
     {
         NFe[i].globalToLocalPosition(m_Sl[0], vl[i][0]);
         NFe[i].globalToLocalPosition(m_Sl[1], vl[i][1]);
         NFe[i].globalToLocalPosition(m_Sl[2], vl[i][2]);
         vl[i][3] = vl[i][0];
         rho[0] = vl[i][0].norm();
         rho[1] = vl[i][1].norm();
         rho[2] = vl[i][2].norm();
         //        qDebug("  %13.7f  %13.7f  %13.7f ", rho[0], rho[1], rho[2]); // check that distance is independant of the reference frame
     }
     rho[3]=rho[0];

     double q[] = {0,0,0,0};
     for(int i=0; i<3; i++)  q[i] = vl[i][i].y;
     q[3] = q[0];
     // check eq. 24
     /*    qDebug("  q0=  %13.7f  %13.7f  ", q[0], vl[0][1].y); // q00=q01 = q1
     qDebug("  q1=  %13.7f  %13.7f  ", q[1], vl[1][2].y); // q11=q12 = q2
     qDebug("  q2=  %13.7f  %13.7f  %13.7f  ", q[2], vl[2][0].y, vl[2][3].y); // q22=q20=q23= q3*/

     double rho_t[] = {0,0,0};
     double d[] = {0,0,0,0};
     for(int i=0; i<3; i++)
     {
         d[i]     = q[i]*q[i] + eta*eta;
         rho_t[i] = rho[i] - rho[i+1];
     }
     d[3]=d[0];

     // implement eq. 25
     double chi[3], delta[3], L[3], gamma[3];

     for(int i=0; i<3; i++)
     {
         double pii  = vl[i][i].x;
         double pii1 = vl[i][i+1].x;

         gamma[i]  = atan2(-2.0*pii  *q[i]*eta*rho[i],   (q[i]*q[i]*rho[i]*rho[i]     -pii*pii  *eta*eta));
         gamma[i] -= atan2(-2.0*pii1 *q[i]*eta*rho[i+1], (q[i]*q[i]*rho[i+1]*rho[i+1] -pii1*pii1*eta*eta));

         chi[i]     = log(pii+rho[i]) - log(pii1+rho[i+1]);

         delta[i]   = pii/rho[i] - pii1/rho[i+1];
         L[i]       = 1.0/rho[i] - 1.0/rho[i+1];
     }

     // implement eq. 26
     double thet0=0;
     switch(pointposition)
     {
         case TRIANGLE::Inside:
         {
             thet0 = 2.0*PI;
             break;
         }
         case TRIANGLE::OnVertex:
         {
             thet0 = theta[iVertex];
             break;
         }
         case TRIANGLE::OnEdge:
         {
             thet0 = PI;
             thet0 = 0.0;
             break;
         }
         case TRIANGLE::Outside:
         {
             thet0 = 0.0;
             break;
         }
     }

     // implement eq. 27
     double sign = (eta>=0.0) ? 1.0 : -1.0;
     double thetx = 0.5*(gamma[0]+gamma[1]+gamma[2]) + sign *thet0;

     if(G1)
     {
         // implement eq. 28
         N1[0] = -eta*thetx;                                     // I1
         for(int i=0; i<3; i++) N1[0] += q[i]*chi[i];

         N1[1] = N1[2] = 0.0;                                    // I1x, I1y
         for(int i=0; i<3; i++)
         {
             N1[1] += q[i]*rho_t[i]*cosa[i] - d[i]*chi[i]*sina[i];
             N1[2] += q[i]*rho_t[i]*sina[i] + d[i]*chi[i]*cosa[i];
         }
         N1[1] *= 0.5;
         N1[2] *= 0.5;
     }

     if(G3)
     {
         // implement eq. 29
         N3[0] = 1.0/eta * thetx;                       // I3
         N3[1] = N3[2] = 0.0;                           // I3x, I3y
         for(int i=0; i<3; i++)
         {
             N3[1] += chi[i]*sina[i];
             N3[2] -= chi[i]*cosa[i];
         }

         if(bGradients)
         {
             // implement eq. 30
             N3[3]=N3[4]=N3[5]=0.0;                         // I3xx, I3xy, I3yy
             for(int i=0; i<3; i++)
             {
                 N3[3] += (q[i]*chi[i]*cosa[i] + rho_t[i]*sina[i]) * cosa[i];  // I3xx
                 N3[4] += (q[i]*chi[i]*cosa[i] + rho_t[i]*sina[i]) * sina[i];  // I3xy
                 N3[5] += (q[i]*chi[i]*sina[i] - rho_t[i]*cosa[i]) * sina[i];  // I3yy
             }
             N3[3] -= eta*thetx;
             N3[5] -= eta*thetx;
         }
     }

     if(G5)
     {
         // implement eq. 31
         N5[0]=N5[1]=N5[2]=0.0;                         // I5, I5x, I5y
         for(int i=0; i<3; i++)
         {
             N5[0]  += q[i]/d[i]*delta[i];    // I5
             N5[1] += delta[i]/d[i]*sina[i];  // I5x
             N5[2] -= delta[i]/d[i]*cosa[i];  // I5y
         }
         N5[0] = N5[0]/3.0/eta/eta + 1.0/3.0/eta/eta/eta*thetx;
         N5[1] *= 1.0/3.0;
         N5[2] *= 1.0/3.0;

         if(bGradients)
         {
             // implement eq. 32
             N5[3]=N5[4]=N5[5]=0.0;
             for(int i=0; i<3; i++)
             {
                 N5[3] += (L[i]*cosa[i] + q[i]/d[i]*delta[i]*sina[i]) *sina[i];  // I5xx
                 N5[4] += (L[i]*sina[i] - q[i]/d[i]*delta[i]*cosa[i]) *sina[i];  // I5xy
                 N5[5] += (L[i]*sina[i] - q[i]/d[i]*delta[i]*cosa[i]) *cosa[i];  // I5yy
             }
             N5[3]  = N5[3] * (-1./3.) + thetx/3.0/eta;
             N5[4] *= -1.0/3.0;
             N5[5]  = N5[5] * ( 1./3.) + thetx/3.0/eta;
         }
     }


     // Change origin from field point to panel's origin
     // No rotation to apply since x axis is the same as the panel's "l"  direction
     double psi = .0;
     double cosp = cos(psi);
     double sinp = sin(psi);
     double dx = FieldPtLocal.x - m_CoG_l.x;
     double dy = FieldPtLocal.y - m_CoG_l.y;

     // Convert the integrals into the panel's reference frame
     if(G1)
     {
         G1[0]  = N1[0];                                // is Int(1/R)
         G1[1]  = dx*N1[0] + cosp*N1[1] - sinp*N1[2];   // is Int(x/R)
         G1[2]  = dy*N1[0] + sinp*N1[1] + cosp*N1[2];   // is Int(y/R)
     }
     if(G3)
     {
         G3[0]  = N3[0];                                // is Int(1/R³)
         G3[1]  = dx*N3[0] + cosp*N3[1] - sinp*N3[2];   // is Int(x/R³)
         G3[2]  = dy*N3[0] + sinp*N3[1] + cosp*N3[2];   // is Int(y/R³)

         if(bGradients)
         {
             //second order: xx/R³, xy/R³, yy/R³
             G3[3] = dx*dx*N3[0] + cosp*cosp*N3[3] + sinp*sinp*N3[5] + 2.0*dx*cosp*N3[1] - 2.0*dx*sinp*N3[2] - 2.0*sinp*cosp*N3[4];
             G3[4] = dx*dy*N3[0] + (dx*sinp+dy*cosp)*N3[1] + (dx*cosp-dy*sinp)*N3[2] + sinp*cosp*N3[3]    + (cosp*cosp-sinp*sinp)*N3[4] - sinp*cosp*N3[5];
             G3[5] = dy*dy*N3[0] + sinp*sinp*N3[3] + cosp*cosp*N3[5] + 2.0*dy*sinp*N3[1] + 2.0*dy*cosp*N3[2] + 2.0*sinp*cosp*N3[4];
         }
     }

     if(G5)
     {
         G5[0]  = N5[0];                                // is Int(1/R⁵)
         G5[1]  = dx*N5[0] + cosp*N5[1] - sinp*N5[2];   // is Int(x/R⁵)
         G5[2]  = dy*N5[0] + sinp*N5[1] + cosp*N5[2];   // is Int(y/R⁵)

         if(bGradients)
         {
             //second order: xx/r⁵, xy/r⁵, yy/r⁵
             G5[3] = dx*dx*N5[0] + cosp*cosp*N5[3] + sinp*sinp*N5[5] + 2.0*dx*cosp*N5[1] - 2.0*dx*sinp*N5[2] - 2.0*sinp*cosp*N5[4];
             G5[4] = dx*dy*N5[0] + (dx*sinp+dy*cosp)*N5[1] + (dx*cosp-dy*sinp)*N5[2] + sinp*cosp*N5[3]    + (cosp*cosp-sinp*sinp)*N5[4] - sinp*cosp*N5[5];
             G5[5] = dy*dy*N5[0] + sinp*sinp*N5[3] + cosp*cosp*N5[5] + 2.0*dy*sinp*N5[1] + 2.0*dy*cosp*N5[2] + 2.0*sinp*cosp*N5[4];
         }
     }
}


void Panel3::copyConnections(Panel3 const &refp3)
{
    for(int ip=0; ip<3; ip++) m_Neighbour[ip] = refp3.neighbour(ip);
    for(int in=0; in<3; in++) m_S[in].setTriangleIndexes(refp3.m_S[in].triangleIndexes());
}



/*
 *                N0
 *               / \
 *              /   \
 *      edge2  /     \  edge1
 *            /       \
 *           /         \
 *          /           \
 *         /             \
 *       N1---------------N2
 *              edge0
 */
void Panel3::setNeighbour(int iTriangle, int iEdge)
{
    for(int i=0; i<3; i++)
        if(m_Neighbour[i]==iTriangle) return; //already in the list - occurs when A has already been connected to B and attempting to connect B to A

    if(iEdge<0 || iEdge>=3) return; // something went wrong somewhere

    m_Neighbour[iEdge]=iTriangle;

    if(iEdge==0)
    {
        m_S[1].addTriangleIndex(iTriangle);
        m_S[2].addTriangleIndex(iTriangle);
    }
    else if(iEdge==1)
    {
        m_S[0].addTriangleIndex(iTriangle);
        m_S[2].addTriangleIndex(iTriangle);
    }
    else if(iEdge==2)
    {
        m_S[0].addTriangleIndex(iTriangle);
        m_S[1].addTriangleIndex(iTriangle);
    }
}


Node const &Panel3::leftTrailingNode() const
{
    if(isWingPanel())
    {
        /*
         *                N0
         *               / \
         *              /   \
         *      edge2  /     \  edge1
         *            /       \
         *           /         \
         *          /           \
         *         /             \
         *       N1---------------N2
         *              edge0
         */
        if      (isTopPanel()) return m_S[1];
        else if (isBotPanel()) return m_S[2];
        else                   return m_S[1]; // mid panel
    }
    else // if(isWakePanel())
    {
        //     left wing                 right wing
        //   2           1              2          1
        //   _____________             _____________
        //   | up       /|1           2|\    up    |
        //   | left   /  |             |  \  right |
        //   |      /    |             |    \      |
        //   |    /      |             |      \    |
        //   |  /  right |             | left   \  |
        //   |/    down  |             | down     \| 0
        // 0 _____________             _____________
        //   2           0             0           1
        //
        if(isLeftWingPanel()) return m_S[2];
        else                  return m_S[0];
    }
}


Node const &Panel3::rightTrailingNode() const
{
    if(isWingPanel())
    {
        /*
         *                N0
         *               / \
         *              /   \
         *      edge2  /     \  edge1
         *            /       \
         *           /         \
         *          /           \
         *         /             \
         *       N1---------------N2
         *              edge0
         */
        if      (isTopPanel()) return m_S[2];
        else if (isBotPanel()) return m_S[1];
        else                   return m_S[2]; // Mid panel

    }
    else // if(isWakePanel())
    {
        //     left wing                 right wing
        //   2           1              2          1
        //   _____________             _____________
        //   | up       /|1           2|\    up    |
        //   | left   /  |             |  \  right |
        //   |      /    |             |    \      |
        //   |    /      |             |      \    |
        //   |  /  right |             | left   \  |
        //   |/    down  |             | down     \| 0
        // 0 _____________             _____________
        //   2           0             0           1
        //
        if(isLeftWingPanel()) return m_S[0];
        else                  return m_S[1];
    }
}


bool Panel3::isTrailingEdgeNode(int iv) const
{
    if(!m_bIsTrailing) return false;
    return (iv==1) || (iv==2);
}


void Panel3::doubletVortexVelocity(Vector3d const &C, Vector3d &V, double coreradius, Vortex::enumVortex vortexmodel, bool bUseRFF) const
{
    Vector3d velseg;
    Vector3d PJK, T1;
    V.x=0.0; V.y=0.0; V.z=0.0;

    PJK.x = C.x - m_CoG_g.x;
    PJK.y = C.y - m_CoG_g.y;
    PJK.z = C.z - m_CoG_g.z;

    double PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    double pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(bUseRFF && pjk>s_RFF*m_MaxSize)
    {
        // use far-field formula
        T1.x = PJK.x*3.0*PN - m_Normal.x*pjk*pjk;
        T1.y = PJK.y*3.0*PN - m_Normal.y*pjk*pjk;
        T1.z = PJK.z*3.0*PN - m_Normal.z*pjk*pjk;
        double pjk5 = pjk *pjk *pjk *pjk *pjk;
        V.x  = T1.x * m_Area /pjk5;
        V.y  = T1.y * m_Area /pjk5;
        V.z  = T1.z * m_Area /pjk5;
        return;
    }

    for (int i=0; i<3; i++)
    {
        velseg = vortexInducedVelocity(m_S[i%3], m_S[(i+1)%3], C, coreradius, vortexmodel);
        V += velseg;
    }

    V.x *= 4.0*PI;
    V.y *= 4.0*PI;
    V.z *= 4.0*PI;
}


/**
 * Checks if a point lies on a panel's edge, within the CoreRadius length
 */
bool Panel3::isEdgePoint(Vector3d const &PtGlobal) const
{
    Vector3d const *pS[4];
    pS[0] = m_S;
    pS[1] = m_S+1;
    pS[2] = m_S+2;
    pS[3] = m_S;

    for(int i=0; i<3; i++)
    {
        Vector3d E = *pS[i+1] - *pS[i];
        double e = E.norm();
        Vector3d R0 = PtGlobal - *pS[i];
        Vector3d R1 = PtGlobal - *pS[i+1];

        //check that the dot product is negative
        if(R0.dot(R1)<=0.0)
        {
            Vector3d u = E/e;
            Vector3d H;
            double projection = R0.dot(u);

            H.x = R0.x - u.x * projection;
            H.y = R0.y - u.y * projection;
            H.z = R0.z - u.z * projection;
            double r = sqrt(H.x*H.x + H.y*H.y + H.z*H.z);
            if(r<Vortex::coreRadius()) return true;
        }
    }

    return false;
}


void Panel3::doubletN4023Velocity(Vector3d const &C, bool , Vector3d &V, double coreradius, bool bUseRFF) const
{
    Vector3d T1;
    double ax(0), ay(0), az(0);
    double bx(0), by(0), bz(0);
    double sx(0), sy(0), sz(0);
    double hx(0), hy(0), hz(0);
    double A(0), B(0), GL(0);

    V.set(0.0,0.0,0.0);

    double PJKx = C.x - m_CoG_g.x;
    double PJKy = C.y - m_CoG_g.y;
    double PJKz = C.z - m_CoG_g.z;

    double PN  = PJKx*m_Normal.x + PJKy*m_Normal.y + PJKz*m_Normal.z;
    double pjk = sqrt(PJKx*PJKx + PJKy*PJKy + PJKz*PJKz);

    if(bUseRFF && pjk> s_RFF*m_MaxSize)
    {
        // use far-field formula

        double pjk2=pjk*pjk;
        double pjk5=pjk*pjk*pjk*pjk2;

        T1.x = PJKx*3.0*PN - m_Normal.x*pjk2;
        T1.y = PJKy*3.0*PN - m_Normal.y*pjk2;
        T1.z = PJKz*3.0*PN - m_Normal.z*pjk2;

        V.x  = T1.x * m_Area /pjk5;
        V.y  = T1.y * m_Area /pjk5;
        V.z  = T1.z * m_Area /pjk5;

        return;
    }

    for (int i=0; i<3; i++)
    {
        ax  = C.x - m_S[i].x;
        ay  = C.y - m_S[i].y;
        az  = C.z - m_S[i].z;
        bx  = C.x - m_S[(i+1)%3].x;
        by  = C.y - m_S[(i+1)%3].y;
        bz  = C.z - m_S[(i+1)%3].z;
        sx  = m_S[(i+1)%3].x - m_S[i].x;
        sy  = m_S[(i+1)%3].y - m_S[i].y;
        sz  = m_S[(i+1)%3].z - m_S[i].z;
        A    = sqrt(ax*ax + ay*ay + az*az);
        B    = sqrt(bx*bx + by*by + bz*bz);

        //get the distance of the TestPoint to the panel's side
        hx =  ay*sz - az*sy;
        hy = -ax*sz + az*sx;
        hz =  ax*sy - ay*sx;

        if(m_S[i].isSame(m_S[(i+1)%3]))
        {
            //no contribution
        }
        else if(A<coreradius|| B<coreradius)
        {
            //no contribution
        }
        else if (sqrt(hx*hx+hy*hy+hz*hz)<=coreradius && ax*sx+ay*sy+az*sz>=0.0 && bx*sx+by*sy+bz*sz<=0.0)
        {
            //no contribution
        }
        else
        {
            // next the induced velocity
            hx =  ay*bz - az*by;
            hy = -ax*bz + az*bx;
            hz =  ax*by - ay*bx;
            GL = ((A+B) /A/B/ (A*B + ax*bx+ay*by+az*bz));

            V.x += hx * GL;
            V.y += hy * GL;
            V.z += hz * GL;
        }
    }
}


void Panel3::doubletN4023Potential(Vector3d const &C, bool bSelf, double &phi, double coreradius, bool bUseRFF) const
{
    Vector3d PJK, a, b, s, h;
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);

    if(bSelf)
    {
        // INTERNAL Dirichlet BC
        phi  = +2.0*PI;
        return;
    }


    phi = 0.0;

    PJK.x = C.x - m_CoG_g.x;
    PJK.y = C.y - m_CoG_g.y;
    PJK.z = C.z - m_CoG_g.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(bUseRFF && pjk> s_RFF*m_MaxSize)
    {
        // use far-field formula
        phi = -PN * m_Area /(pjk*pjk*pjk);
        return;
    }

    for (int i=0; i<3; i++)
    {
        a.x  = C.x - m_S[i].x;
        a.y  = C.y - m_S[i].y;
        a.z  = C.z - m_S[i].z;
        b.x  = C.x - m_S[(i+1)%3].x;
        b.y  = C.y - m_S[(i+1)%3].y;
        b.z  = C.z - m_S[(i+1)%3].z;
        s.x  = m_S[(i+1)%3].x - m_S[i].x;
        s.y  = m_S[(i+1)%3].y - m_S[i].y;
        s.z  = m_S[(i+1)%3].z - m_S[i].z;
        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        SM   = s.x*m_m.x + s.y*m_m.y + s.z*m_m.z;
        SL   = s.x*m_l.x + s.y*m_l.y + s.z*m_l.z;
        AM   = a.x*m_m.x + a.y*m_m.y + a.z*m_m.z;
        AL   = a.x*m_l.x + a.y*m_l.y + a.z*m_l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        //first the potential
        if(m_S[i].isSame(m_S[(i+1)%3]))
        {
            CJKi = 0.0;
            //no contribution to speed either
        }
        else if (((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= coreradius*coreradius)
                 && a.x*s.x+a.y*s.y+a.z*s.z>=0.0
                 && b.x*s.x+b.y*s.y+b.z*s.z<=0.0)
        {
            CJKi = 0.0;//speed is singular at panel edge, the value of the potential is unknown
        }
        else
        {
            if(fabs(PN)<INPLANEPRECISION)
            {
                CJKi = 0.0;
            }
            else
            {
                RNUM = SM*PN * (B*PA-A*PB);
                DNOM = PA*PB + PN*PN*A*B*SM*SM;
                CJKi = atan2(RNUM,DNOM);
            }
        }
        phi -= CJKi;
    }
}


void Panel3::sourceN4023Potential(Vector3d const &C, bool , double &phi, double coreradius) const
{
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double sign(1.0), Sk(0), GL(0);
    Vector3d PJK, a, b, s, T1, T2, h;

    phi = 0.0;

    PJK.x = C.x - m_CoG_g.x;
    PJK.y = C.y - m_CoG_g.y;
    PJK.z = C.z - m_CoG_g.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);


    if(pjk> s_RFF*m_MaxSize)
    {
        // use far-field formula
        phi = -m_Area /pjk;
        return;
    }

    for (int i=0; i<3; i++)
    {
        a.x  = C.x - m_S[i].x;
        a.y  = C.y - m_S[i].y;
        a.z  = C.z - m_S[i].z;

        b.x  = C.x - m_S[(i+1)%3].x;
        b.y  = C.y - m_S[(i+1)%3].y;
        b.z  = C.z - m_S[(i+1)%3].z;

        s.x  = m_S[(i+1)%3].x - m_S[i].x;
        s.y  = m_S[(i+1)%3].y - m_S[i].y;
        s.z  = m_S[(i+1)%3].z - m_S[i].z;

        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        Sk   = sqrt(s.x*s.x + s.y*s.y + s.z*s.z);
        SM   = s.x*m_m.x + s.y*m_m.y + s.z*m_m.z;
        SL   = s.x*m_l.x + s.y*m_l.y + s.z*m_l.z;
        AM   = a.x*m_m.x + a.y*m_m.y + a.z*m_m.z;
        AL   = a.x*m_l.x + a.y*m_l.y + a.z*m_l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        if(m_S[i].isSame(m_S[(i+1)%3]))
        {
            //no contribution from this side
            CJKi = 0.0;
        }
        else if ((((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= coreradius*coreradius) && a.x*s.x+a.y*s.y+a.z*s.z>=0.0 && b.x*s.x+b.y*s.y+b.z*s.z<=0.0) ||
                 A < coreradius || B < coreradius)
        {
            //if lying on the panel's side... no contribution
            CJKi = 0.0;
        }
        else
        {
            if(fabs(A+B-Sk)>0.0) GL = 1.0/Sk * log(fabs((A+B+Sk)/(A+B-Sk)));
            else                 GL = 0.0;

            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;

            if(fabs(PN)<INPLANEPRECISION)
            {
                // side contribution is >0 if the point is on the panel's right side
                sign = 1.0;
                if(DNOM<0.0)
                {
                    if(PN>0.0)    CJKi =  PI * sign;
                    else        CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)    CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan2(RNUM, DNOM);
            }

            phi += Al*GL - PN*CJKi;
        }
    }
    phi = -phi;
}


/** returns F/(rho.gamma) */
Vector3d Panel3::vortexForce(Vector3d const &wind) const
{
    Vector3d force;
    Vector3d s;

    for (int i=0; i<3; i++)
    {
        s.x  = m_S[(i+1)%3].x - m_S[i].x;
        s.y  = m_S[(i+1)%3].y - m_S[i].y;
        s.z  = m_S[(i+1)%3].z - m_S[i].z;

        force += wind*s; // vortex = rho.v*Gamma
    }

    return force;
}



void Panel3::sourceN4023Velocity(Vector3d const &C, bool bSelf, Vector3d &Vel, double coreradius) const
{
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double Sk(0), GL(0);

    double ax(0), ay(0), az(0);
    double bx(0), by(0), bz(0);
    double sx(0), sy(0), sz(0);
    double hx(0), hy(0), hz(0);
    double T1x(0), T1y(0), T1z(0);
    double T2x(0), T2y(0), T2z(0);
    Vector3d PJK;


    Vel.x=0.0; Vel.y=0.0; Vel.z=0.0;

    PJK.x = C.x - m_CoG_g.x;
    PJK.y = C.y - m_CoG_g.y;
    PJK.z = C.z - m_CoG_g.z;

    PN  = PJK.x*m_Normal.x + PJK.y*m_Normal.y + PJK.z*m_Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);


    if(pjk> s_RFF*m_MaxSize)
    {
        // use far-field formula
        Vel.x = PJK.x * m_Area/pjk/pjk/pjk;
        Vel.y = PJK.y * m_Area/pjk/pjk/pjk;
        Vel.z = PJK.z * m_Area/pjk/pjk/pjk;
        return;
    }

    for (int i=0; i<3; i++)
    {
        ax  = C.x - m_S[i].x;
        ay  = C.y - m_S[i].y;
        az  = C.z - m_S[i].z;

        bx  = C.x - m_S[(i+1)%3].x;
        by  = C.y - m_S[(i+1)%3].y;
        bz  = C.z - m_S[(i+1)%3].z;

        sx  = m_S[(i+1)%3].x - m_S[i].x;
        sy  = m_S[(i+1)%3].y - m_S[i].y;
        sz  = m_S[(i+1)%3].z - m_S[i].z;

        A    = sqrt(ax*ax + ay*ay + az*az);
        B    = sqrt(bx*bx + by*by + bz*bz);
        Sk   = sqrt(sx*sx + sy*sy + sz*sz);
        SM   = sx*m_m.x + sy*m_m.y + sz*m_m.z;
        SL   = sx*m_l.x + sy*m_l.y + sz*m_l.z;
        AM   = ax*m_m.x + ay*m_m.y + az*m_m.z;
        AL   = ax*m_l.x + ay*m_l.y + az*m_l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        hx =  ay*sz - az*sy;
        hy = -ax*sz + az*sx;
        hz =  ax*sy - ay*sx;

        if(m_S[i].isSame(m_S[(i+1)%3]))
        {
            //no contribution from this side
            CJKi = 0.0;
        }
        else if ((((hx*hx+hy*hy+hz*hz)/(sx*sx+sy*sy+sz*sz) <= coreradius*coreradius) && ax*sx+ay*sy+az*sz>=0.0 && bx*sx+by*sy+bz*sz<=0.0) ||
                 A < coreradius || B < coreradius)
        {
            //if lying on the panel's side... no contribution
            CJKi = 0.0;
        }
        else
        {
            //first the potential
            if(fabs(A+B-Sk)>0.0) GL = 1.0/Sk * log(fabs((A+B+Sk)/(A+B-Sk)));
            else                 GL = 0.0;

            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;

            if(fabs(PN)<INPLANEPRECISION)
            {
                // side contribution is >0 if the point is on the panel's right side
                if(DNOM<0.0)
                {
                    if(PN>0.0)    CJKi =  PI;
                    else        CJKi = -PI;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)    CJKi =  PI/2.0;
                    else        CJKi = -PI/2.0;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan2(RNUM, DNOM);
            }

            T1x   = m_l.x      * SM*GL;
            T1y   = m_l.y      * SM*GL;
            T1z   = m_l.z      * SM*GL;
            T2x   = m_m.x      * SL*GL;
            T2y   = m_m.y      * SL*GL;
            T2z   = m_m.z      * SL*GL;

            Vel.x   += m_Normal.x * CJKi + T1x - T2x;
            Vel.y   += m_Normal.y * CJKi + T1y - T2y;
            Vel.z   += m_Normal.z * CJKi + T1z - T2z;
        }
    }

    if(fabs(PN)<INPLANEPRECISION) Vel.z = 0.0;

    if(bSelf)
    {
        // The normal component of the velocity is discontinuous in the normal's direction.
        // Force the value on the outer surface to be consistent with the external BC
        Vel.z = m_Normal.z * 2.0 * PI;
    }
}


/**
 * @brief Calculates the panel influence of a source density at a field point using Michael Carley's method
 * @param phi the potential at the field point
 * @param Pt the field point where the influence is calculated, in global coordinates
 */
void Panel3::sourcePotential(Vector3d const &Pt, bool bSelf, double &phi) const
{
    Vector3d Ptl = globalToLocalPosition(Pt);

    double r = sqrt(Ptl.x*Ptl.x+Ptl.y*Ptl.y+Ptl.z*Ptl.z);
    if(r>s_RFF*m_MaxSize)
    {
        phi = -m_Area/r;
        return;
    }

    double I1[3];
    memset(I1, 0, 3*sizeof(double));

    bool bInPlane = fabs(Ptl.z)<INPLANEPRECISION;
    if(s_bUseNintcheuFata && !bInPlane)
    {
        // use S. Nintcheu Fata's method
        // if in-plane fall back on M. Carley's method
        computeNFIntegrals(Pt, I1, nullptr, nullptr, false);
    }
    else
    {
        // use M. Carley's method
        computeMCIntegrals(Pt, bSelf, I1, nullptr, nullptr, false);
    }

    phi = -I1[0];
}


/**
 * @brief Calculates the panel influence of a source density at a field point
 * @param Pt the field point where the influence is calculated, in global coordinates
 * @param Velocity the velocity at the field point, in the global reference frame
 */
void Panel3::sourceVelocity(Vector3d const& Pt, bool bSelf, Vector3d &Velocity) const
{
    Vector3d Ptl = globalToLocalPosition(Pt);

    if(bSelf)
    {
        // With the normal pointing outwards,
        //   - exterior limit to Vn =  2.pi.n
        //   - interior limit is Vn = -2.pi.n
        // Exterior Neumann BC:
        Velocity = m_Normal *(+2.0*PI);;
        return;
    }

    double r = sqrt(Ptl.x*Ptl.x+Ptl.y*Ptl.y+Ptl.z*Ptl.z);
    if(r>s_RFF*m_MaxSize)
    {
        Vector3d vel;
        double invr3 = 1.0/(r*r*r);
        vel.x = Ptl.x*m_Area * invr3;
        vel.y = Ptl.y*m_Area * invr3;
        vel.z = Ptl.z*m_Area * invr3;
        localToGlobal(vel, Velocity);
        return;
    }

    double I3[6];
    memset(I3, 0, 6*sizeof(double));

    bool bInPlane = fabs(Ptl.z)<INPLANEPRECISION;
    if(s_bUseNintcheuFata && !bInPlane)
    {
        // use S. Nintcheu Fata's method
        // if in-plane fall back on M. Carley's method
        computeNFIntegrals(Pt, nullptr, I3, nullptr, false);
    }
    else
    {
        // use M. Carley's method
        computeMCIntegrals(Pt, bInPlane, nullptr, I3, nullptr, false);
    }

    Vector3d vel;
    vel.x = Ptl.x*I3[0]-I3[1];
    vel.y = Ptl.y*I3[0]-I3[2];
    vel.z = Ptl.z*I3[0];

    localToGlobal(vel, Velocity);
}



/**
 * @brief Returns the value of the potential induced by the basis functions with unit doublet density.
 * @param Pt the field point, in global coordinates
 * @param phi the potential induced by the basis function 0 with unit doublet density
 * @param bSelf true if calculating a panel's influence on itself
 */
void Panel3::doubletBasisPotential(Vector3d const &ptGlobal, bool bSelf, double *phi, bool bUseRFF) const
{
    double G3[] = {0,0,0,0,0,0};

    Vector3d Ptl;
    globalToLocalPosition(ptGlobal.x, ptGlobal.y, ptGlobal.z, Ptl.x, Ptl.y, Ptl.z);

    double r = sqrt(Ptl.x*Ptl.x+Ptl.y*Ptl.y+Ptl.z*Ptl.z);

    if(bSelf)
    {
        // Potential is undefined on the panel itself
        // Value is evaluated on the internal face, i.e. opposite the Normal
        // to be consistent with the INTERNAL Dirichlet BC
        //
        phi[0] = 2.0*PI*basis(Ptl.x, Ptl.y, 0);
        phi[1] = 2.0*PI*basis(Ptl.x, Ptl.y, 1);
        phi[2] = 2.0*PI*basis(Ptl.x, Ptl.y, 2);
        return;
    }

    if(fabs(Ptl.z)<INPLANEPRECISION)
    {
        // the doublet potential is zero in the panel's plane outside the panel
        phi[0] = phi[1] = phi[2] = 0.0;
        return;
    }

    if(bUseRFF && r>s_RFF*m_MaxSize)
    {
        phi[0]=phi[1]=phi[2] = -Ptl.z/r/r/r *m_Area/3.0;
        return;
    }

    double detA = 2.0*m_Area; // no need to recalculate
    double J03[] = {0,0,0};

    if(s_bUseNintcheuFata)
    {
        // use S. Nintcheu Fata's method
        // In-Plane Potential has been addressed above

        computeNFIntegrals(ptGlobal, nullptr, G3, nullptr, false);
/*        if(s_bTest)
        {
            double MC3[] = {0,0,0,0,0,0};
            computeMCIntegrals(ptGlobal, false, nullptr, MC3, nullptr, false);
            for(int i=0; i<6; i++) qDebug("  MC=%13.7g  NF=%13.7g", MC3[i], G3[i]);
        }*/
    }
    else
    {
        computeMCIntegrals(ptGlobal, false, nullptr, G3, nullptr, false);
    }

    // Apply basis functions transformations
    J03[0] =   (m_Sl[1].x*m_Sl[2].y-m_Sl[1].y*m_Sl[2].x) * G3[0] - (m_Sl[2].y-m_Sl[1].y)*G3[1] + (m_Sl[2].x-m_Sl[1].x)*G3[2];
    J03[1] = -((m_Sl[0].x*m_Sl[2].y-m_Sl[0].y*m_Sl[2].x) * G3[0] - (m_Sl[2].y-m_Sl[0].y)*G3[1] + (m_Sl[2].x-m_Sl[0].x)*G3[2]);
    J03[2] =   (m_Sl[0].x*m_Sl[1].y-m_Sl[0].y*m_Sl[1].x) * G3[0] - (m_Sl[1].y-m_Sl[0].y)*G3[1] + (m_Sl[1].x-m_Sl[0].x)*G3[2];

    J03[0] *= 1.0/detA;
    J03[1] *= 1.0/detA;
    J03[2] *= 1.0/detA;

    // Compute the potential induced by each unit density basis function
    phi[0] = -Ptl.z * J03[0];
    phi[1] = -Ptl.z * J03[1];
    phi[2] = -Ptl.z * J03[2];
}



/**
 * @brief returns the velocity vector induced by the basis functions with unit doublet weights.
 * @param Pt the field point, in global coordinates
 * @param V0 the velocity vector induced by basis function 0  at the field point, in global coordinates
 * @param V1 the velocity vector induced by basis function 1  at the field point, in global coordinates
 * @param V2 the velocity vector induced by basis function 2  at the field point, in global coordinates
 */
void Panel3::doubletBasisVelocity(Vector3d const &ptGlobal, Vector3d *V, bool bUseRFF) const
{
    Vector3d Vl[3];
    double G3[6], G5[6];
    memset(G3, 0, 6*sizeof(double));
    memset(G5, 0, 6*sizeof(double));
    Vector3d Ptl = globalToLocalPosition(ptGlobal);
    double r = sqrt(Ptl.x*Ptl.x+Ptl.y*Ptl.y+Ptl.z*Ptl.z);

    if(bUseRFF && r>s_RFF*m_MaxSize)
    {
        double invr3 = 1.0/r/r/r;
        double invr5 = invr3/r/r;
        Vl[0].x = Ptl.z*invr5*(Ptl.x*m_Area-bx[0]);
        Vl[0].y = Ptl.z*invr5*(Ptl.y*m_Area-by[0]);
        Vl[0].z = (-invr3+ 3.0*Ptl.z*Ptl.z*invr5) *m_Area/3.0;

        Vl[1].x = Ptl.z*invr5*(Ptl.x*m_Area-bx[1]);
        Vl[1].y = Ptl.z*invr5*(Ptl.y*m_Area-by[1]);
        Vl[1].z = Vl[0].z;

        Vl[2].x = Ptl.z*invr5*(Ptl.x*m_Area-bx[2]);
        Vl[2].y = Ptl.z*invr5*(Ptl.y*m_Area-by[2]);
        Vl[2].z = Vl[0].z;

        V[0] = localToGlobal(Vl[0]);
        V[1] = localToGlobal(Vl[1]);
        V[2] = localToGlobal(Vl[2]);
        return;
    }

    double detA = 2.0*m_Area; // no need to recalculate
    double J03[]={0,0,0};
    double J05[]={0,0,0};
    double Jx5[]={0,0,0}, Jy5[]={0,0,0};

    bool bInPlane = fabs(Ptl.z)<INPLANEPRECISION;
    if(s_bUseNintcheuFata && !bInPlane)
    {
        // use S. Nintcheu Fata's method
        // if in-plane fall back on M. Carley's method
        computeNFIntegrals(ptGlobal, nullptr, G3, G5, true);
    }
    else
    {
        computeMCIntegrals(ptGlobal, bInPlane, nullptr, G3, G5, true);
    }

    // Apply basis functions transformations
    J03[0] =   (m_Sl[1].x*m_Sl[2].y-m_Sl[1].y*m_Sl[2].x) * G3[0] - (m_Sl[2].y-m_Sl[1].y)*G3[1] + (m_Sl[2].x-m_Sl[1].x)*G3[2];
    J03[1] = -((m_Sl[0].x*m_Sl[2].y-m_Sl[0].y*m_Sl[2].x) * G3[0] - (m_Sl[2].y-m_Sl[0].y)*G3[1] + (m_Sl[2].x-m_Sl[0].x)*G3[2]);
    J03[2] =   (m_Sl[0].x*m_Sl[1].y-m_Sl[0].y*m_Sl[1].x) * G3[0] - (m_Sl[1].y-m_Sl[0].y)*G3[1] + (m_Sl[1].x-m_Sl[0].x)*G3[2];

    J05[0] =   (m_Sl[1].x*m_Sl[2].y-m_Sl[1].y*m_Sl[2].x) * G5[0] - (m_Sl[2].y-m_Sl[1].y)*G5[1] + (m_Sl[2].x-m_Sl[1].x)*G5[2];
    J05[1] = -((m_Sl[0].x*m_Sl[2].y-m_Sl[0].y*m_Sl[2].x) * G5[0] - (m_Sl[2].y-m_Sl[0].y)*G5[1] + (m_Sl[2].x-m_Sl[0].x)*G5[2]);
    J05[2] =   (m_Sl[0].x*m_Sl[1].y-m_Sl[0].y*m_Sl[1].x) * G5[0] - (m_Sl[1].y-m_Sl[0].y)*G5[1] + (m_Sl[1].x-m_Sl[0].x)*G5[2];

    Jx5[0] =   (m_Sl[1].x*m_Sl[2].y-m_Sl[1].y*m_Sl[2].x) * G5[1] - (m_Sl[2].y-m_Sl[1].y)*G5[3] + (m_Sl[2].x-m_Sl[1].x)*G5[4];
    Jx5[1] = -((m_Sl[0].x*m_Sl[2].y-m_Sl[0].y*m_Sl[2].x) * G5[1] - (m_Sl[2].y-m_Sl[0].y)*G5[3] + (m_Sl[2].x-m_Sl[0].x)*G5[4]);
    Jx5[2] =   (m_Sl[0].x*m_Sl[1].y-m_Sl[0].y*m_Sl[1].x) * G5[1] - (m_Sl[1].y-m_Sl[0].y)*G5[3] + (m_Sl[1].x-m_Sl[0].x)*G5[4];

    Jy5[0] =   (m_Sl[1].x*m_Sl[2].y-m_Sl[1].y*m_Sl[2].x) * G5[2] - (m_Sl[2].y-m_Sl[1].y)*G5[4] + (m_Sl[2].x-m_Sl[1].x)*G5[5];
    Jy5[1] = -((m_Sl[0].x*m_Sl[2].y-m_Sl[0].y*m_Sl[2].x) * G5[2] - (m_Sl[2].y-m_Sl[0].y)*G5[4] + (m_Sl[2].x-m_Sl[0].x)*G5[5]);
    Jy5[2] =   (m_Sl[0].x*m_Sl[1].y-m_Sl[0].y*m_Sl[1].x) * G5[2] - (m_Sl[1].y-m_Sl[0].y)*G5[4] + (m_Sl[1].x-m_Sl[0].x)*G5[5];

    J03[0] *= 1.0/detA;
    J03[1] *= 1.0/detA;
    J03[2] *= 1.0/detA;

    J05[0] *= 1.0/detA;      Jx5[0] *= 1.0/detA;      Jy5[0] *= 1.0/detA;
    J05[1] *= 1.0/detA;      Jx5[1] *= 1.0/detA;      Jy5[1] *= 1.0/detA;
    J05[2] *= 1.0/detA;      Jx5[2] *= 1.0/detA;      Jy5[2] *= 1.0/detA;

    // Compute the velocity induced by each unit density basis function
    Vl[0].x = 3.0 * Ptl.z * (Ptl.x*J05[0]-Jx5[0]);
    Vl[0].y = 3.0 * Ptl.z * (Ptl.y*J05[0]-Jy5[0]);
    Vl[0].z = -J03[0] + 3.0 * Ptl.z*Ptl.z * J05[0];

    Vl[1].x = 3.0 * Ptl.z * (Ptl.x*J05[1]-Jx5[1]);
    Vl[1].y = 3.0 * Ptl.z * (Ptl.y*J05[1]-Jy5[1]);
    Vl[1].z = -J03[1] + 3.0 * Ptl.z*Ptl.z * J05[1];

    Vl[2].x = 3.0 * Ptl.z * (Ptl.x*J05[2]-Jx5[2]);
    Vl[2].y = 3.0 * Ptl.z * (Ptl.y*J05[2]-Jy5[2]);
    Vl[2].z = -J03[2] + 3.0 * Ptl.z*Ptl.z * J05[2];

    V[0] = localToGlobal(Vl[0]);
    V[1] = localToGlobal(Vl[1]);
    V[2] = localToGlobal(Vl[2]);
}


/**
 * @brief Calculates the potential induced by the panel's unit basis functions at a field
 * point Pt using Gauss-Legendre quadrature.
 * @param phi a pointer to the array of potential values at the field point
 * @param Pt the field point where the influence is calculated, in global coordinates
 */
void Panel3::doubletQuadraturePotential(Vector3d Pt, double *phi) const
{
    Vector3d Ptl = globalToLocalPosition(Pt);
    Vector3d r;
    double sumPotential[3];
    sumPotential[0] = sumPotential[1] = sumPotential[2] = 0.0;

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        r = Ptl - Vector3d(x,y,0.0);
        double dist = r.norm();
        /*        r.x = Ptl.x - x;
        r.y = Ptl.y - y;
        r.z = Ptl.z;
        double dist = sqrt(r.x*r.x+r.y*r.y+r.z*r.z);*/
        double r3 = dist*dist*dist;

        sumPotential[0] +=  basis(x,y,0) * (-Ptl.z /r3) * s_gq.weights().at(i);
        sumPotential[1] +=  basis(x,y,1) * (-Ptl.z /r3) * s_gq.weights().at(i);
        sumPotential[2] +=  basis(x,y,2) * (-Ptl.z /r3) * s_gq.weights().at(i);
    }
    phi[0] = sumPotential[0] * fabs(m_SignedArea);
    phi[1] = sumPotential[1] * fabs(m_SignedArea);
    phi[2] = sumPotential[2] * fabs(m_SignedArea);
}



/**
 * @brief Calculates the velocity induced by the panel's unit basis functions at a field
 * point Pt using Gauss-Legendre quadrature.
 * @param V a pointer to the array of velocity vectors at the field point
 * @param Pt the field point where the influence is calculated, in global coordinates
 */
void Panel3::doubletQuadratureVelocity(Vector3d Pt, Vector3d *V) const
{
    if(!V) return;
    Vector3d Ptl = globalToLocalPosition(Pt);
    Vector3d r;
    double b[]{0,0,0};
    Vector3d sumvelocity[3];

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        r = Ptl - Vector3d(x,y,0.0);
        double dist = r.norm();
        /*        r.x = Ptl.x - x;
        r.y = Ptl.y - y;
        r.z = Ptl.z;
        double dist = sqrt(r.x*r.x+r.y*r.y+r.z*r.z);*/
        double r3 = dist*dist*dist;
        double r5 = r3*dist*dist;
        b[0] =  basis(x,y,0);
        b[1] =  basis(x,y,1);
        b[2] =  basis(x,y,2);

        for(int l=0; l<3; l++)
        {
            sumvelocity[l].x  +=  3.0 * b[l] * (Ptl.x-x)* Ptl.z/r5       * s_gq.weights().at(i);
            sumvelocity[l].y  +=  3.0 * b[l] * (Ptl.y-y)* Ptl.z/r5       * s_gq.weights().at(i);
            sumvelocity[l].z  += b[l] *(-1.0/r3 + 3.0*Ptl.z*Ptl.z/r5 )   * s_gq.weights().at(i);
        }
    }
    for(int l=0; l<3; l++)
    {
        Vector3d localVelocity = sumvelocity[l]  * fabs(m_SignedArea);
        V[l] = localToGlobal(localVelocity);
    }
}

/**
* Finds the intersection point of a ray with the panel.
* The ray is defined by a point and a direction vector.
*@param A the ray's origin
*@param U the ray's direction
*@param I the intersection point
*/
bool Panel3::intersect(Vector3d const &A, Vector3d const &U, Vector3d &I) const
{
    if(m_bNullTriangle) return false;

    /*    double Nx =  (m_Vertex[1].y-m_Vertex[0].y) * (m_Vertex[2].z-m_Vertex[0].z) - (m_Vertex[1].z-m_Vertex[0].z) * (m_Vertex[2].y-m_Vertex[0].y);
    double Ny = -(m_Vertex[1].x-m_Vertex[0].x) * (m_Vertex[2].z-m_Vertex[0].z) + (m_Vertex[1].z-m_Vertex[0].z) * (m_Vertex[2].x-m_Vertex[0].x);
    double Nz =  (m_Vertex[1].x-m_Vertex[0].x) * (m_Vertex[2].y-m_Vertex[0].y) - (m_Vertex[1].y-m_Vertex[0].y) * (m_Vertex[2].x-m_Vertex[0].x);*/
    double Nx = m_Normal.x;
    double Ny = m_Normal.y;
    double Nz = m_Normal.z;

    double r = (m_CoG_g.x-A.x)*Nx + (m_CoG_g.y-A.y)*Ny + (m_CoG_g.z-A.z)*Nz ;
    double s = U.x*Nx + U.y*Ny + U.z*Nz;

    double dist = 10000.0;

    if(fabs(s)>0.0)
    {
        dist = r/s;
        I.x = A.x + U.x * dist;
        I.y = A.y + U.y * dist;
        I.z = A.z + U.z * dist;

        double g[] = {0,0,0};
        double xl=0, yl=0, zl=0;

        globalToLocalPosition(I.x, I.y, I.z, xl, yl, zl);
        barycentricCoords(xl, yl, g);
        return 0<=g[0] && g[0]<=1.0 && 0<=g[1] && g[1]<=1.0 && 0<=g[2] && g[2]<=1.0;
    }
    return false;
}


/**
 * Returns the scalar product of the potential induced by the uniform source density
 * on the sourcepanel with the basis functions of this panel.
 */
void Panel3::scalarProductSourcePotential(const Panel3 &SourcePanel, bool bSelf, double *sp) const
{
    Vector3d ptGlobal;
    double phiSource(0);
    double integrand(0);
    double sum[]{0,0,0};
    double x(0), y(0);

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        //convert the local panel point to global coordinates
        localToGlobalPosition(x,y,0.0, ptGlobal.x, ptGlobal.y, ptGlobal.z);

        //get the influence of the source panel at this point
        //        pSourcePanel->sourcePotential(ptGlobal, bSelf, phiSource);
        SourcePanel.sourceN4023Potential(ptGlobal, bSelf, phiSource, 0.0);

        //scalar product with basis function
        for(int l=0; l<3; l++)
        {
            integrand = phiSource * basis(x,y,l) * s_gq.weights().at(i);
            sum[l] += integrand;
        }
    }

    for(int l=0; l<3; l++)
        sp[l] = sum[l]*m_Area;
}


/**
 * Returns the scalar product of velocity.dot(normal) induced by the uniform source density
 * on the sourcepanel with the basis functions of this panel.
 */
void Panel3::scalarProductSourceVelocity(Panel3 const &SourcePanel, bool bSelf, double *sp) const
{
    if(!sp) return;

    Vector3d ptGlobal;
    Vector3d Vel;
    double integrand(0);
    double sum[]{0,0,0};

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        //convert the local panel point to global coordinates
        localToGlobalPosition(x,y,0.0, ptGlobal.x, ptGlobal.y, ptGlobal.z);

        //get the influence of the source panel at this point
        SourcePanel.sourceVelocity(ptGlobal, bSelf, Vel);
        //        SourcePanel.sourceN4023Velocity(ptGlobal, bSelf, Vel, 0.0);

        //scalar product with basis function
        for(int l=0; l<3; l++)
        {
            integrand = -Vel.dot(m_Normal) * basis(x,y,l) * s_gq.weights().at(i);
            sum[l] += integrand;
        }
    }

    for(int l=0; l<3; l++)
        sp[l] = sum[l]*m_Area;
}


/**
 * Returns the scalar product of the basis functions of pDoubletPanel with the basis functions
 * of this panel using Gaussian quadrature.
 */
void Panel3::scalarProductDoubletPotential(Panel3 const &DoubletPanel, bool bSelf, double *sp) const
{
    Vector3d ptGlobal;
    double integrand(0);
    double sum[]{0,0,0,0,0,0,0,0,0};
    double phi[]{0,0,0};

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        localToGlobalPosition(x,y,0.0, ptGlobal.x, ptGlobal.y, ptGlobal.z);
        DoubletPanel.doubletBasisPotential(ptGlobal, bSelf, phi, true);

        for(int k=0; k<3; k++)
        {
            for(int l=0; l<3; l++)
            {
                integrand = phi[l] * basis(x,y,k) * s_gq.weights().at(i);
                sum[3*k+l] += integrand;
            }
        }
    }

    for(int i=0; i<9; i++)    sp[i] = sum[i] *m_Area;
}



/**
 * Returns the scalar product of the basis functions of pDoubletPanel with the basis functions
 * of this panel using Gaussian quadrature.
 */
void Panel3::scalarProductDoubletVelocity(const Panel3 &DoubletPanel, double *sp) const
{
    Vector3d ptGlobal;
    Vector3d V[3];

    double integrand(0);
    double sum[]{0,0,0,0,0,0,0,0,0};

    for(uint i=0; i<s_gq.points().size(); i++)
    {
        double x = m_Sl[0].x*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].x*s_gq.points().at(i).x + m_Sl[2].x*s_gq.points().at(i).y;
        double y = m_Sl[0].y*(1.0-s_gq.points().at(i).x-s_gq.points().at(i).y) + m_Sl[1].y*s_gq.points().at(i).x + m_Sl[2].y*s_gq.points().at(i).y;

        localToGlobalPosition(x,y,0.0, ptGlobal.x, ptGlobal.y, ptGlobal.z);
        DoubletPanel.doubletBasisVelocity(ptGlobal, V, true);

        for(int k=0; k<3; k++)
        {
            for(int l=0; l<3; l++)
            {
                integrand = V[l].dot(m_Normal) * basis(x,y,k) * s_gq.weights().at(i);
                sum[3*k+l] += integrand;
            }
        }
    }

    for(int i=0; i<9; i++)    sp[i] = sum[i] *m_Area;
}


void Panel3::moveVertex(int iv, Vector3d const&pos)
{
    if(iv<0||iv>2) return;
    m_S[iv].setPosition(pos);
    setFrame();
}


void Panel3::splitAtPt(Node const &ptinside, std::vector<Panel3> &splitpanels) const
{
    splitpanels.clear();
    Panel3 p30(ptinside, m_S[1], m_S[2]);
    p30.setSurfacePosition(m_Pos);
    if(!p30.isNullTriangle()) splitpanels.push_back(p30);

    Panel3 p31(ptinside, m_S[2], m_S[0]);
    p31.setSurfacePosition(m_Pos);
    if(!p31.isNullTriangle()) splitpanels.push_back(p31);

    Panel3 p32(ptinside, m_S[0], m_S[1]);
    p32.setSurfacePosition(m_Pos);
    if(!p32.isNullTriangle()) splitpanels.push_back(p32);
}


double Panel3::width() const
{
    double w(0.0);
    for(int i=0; i<3; i++)
    {
        double d = sqrt(m_Edge[i].segment().y*m_Edge[i].segment().y + m_Edge[i].segment().z*m_Edge[i].segment().z);
        w = std::max(w, d);
    }
    return w;
}


double Panel3::qualityFactor(double &r, double &shortestEdge) const
{
    double a = m_Edge[0].length();
    double b = m_Edge[1].length();
    double c = m_Edge[2].length();
    shortestEdge = a;
    shortestEdge = std::min(shortestEdge, b);
    shortestEdge = std::min(shortestEdge, c);

    // find circumradius
    r = a*b*c / sqrt((a+b+c)*(b+c-a)*(c+a-b)*(a+b-c));

    return r/shortestEdge;
}


double Panel3::minAngle() const
{
    double angle = fabs(m_Angle[0]);
    angle = std::min(angle, m_Angle[1]);
    angle = std::min(angle, m_Angle[2]);
    return angle;
}


double Panel3::minEdgeLength() const
{
    double l(1.e10);
    for (int ie=0; ie<3; ie++)
    {
        l = std::min(l, edge(ie).length());
    }
    return l;
}


double Panel3::maxEdgeLength() const
{
    double l(0);
    for (int ie=0; ie<3; ie++)
    {
        l = std::max(l, edge(ie).length());
    }
    return l;
}




