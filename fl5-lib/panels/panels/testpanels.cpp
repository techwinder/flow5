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

#include <QDebug>



#include <panel3.h>
#include <panel4.h>

#include <mctriangle.h>
#include <triangle2d.h>
#include <triangle3d.h>



#include <testpanels.h>



void testBasisFunctions()
{
    Vector3d Vertice[3];
    Panel3 panel3;

    //equilateral triangle
    Vertice[0] = Vector3d(1.0, 0.0, 0.0);
    Vertice[1] = Vector3d(cos(2*PI/3), sin(2*PI/3), 0.0);
    Vertice[2] = Vector3d(cos(4*PI/3), sin(4*PI/3) , 0.0);

    panel3.setFrame(Vertice[0], Vertice[1], Vertice[2]);
    //make another distant random panel

    Vertice[0] = Vector3d( 0.5, 1.0, 1.5);
    Vertice[1] = Vector3d( 1.5, -1.0, 1.0);
    Vertice[2] = Vector3d( 1.0, 0.0, 1.0);

    Panel3 panelSource;
    panelSource.setFrame(Vertice[0], Vertice[1], Vertice[2]);

    Vector3d ptGlobal;
    double g[3];
    g[0]=1.0;  g[1]=0.0;   g[2]=0.0;
    panelSource.cartesianCoords(g, ptGlobal);
    Vector3d ptLocal = panelSource.globalToLocalPosition(ptGlobal);
    qDebug()<<panelSource.basis(ptLocal.x, ptLocal.y, 0)<<panelSource.basis(ptLocal.x, ptLocal.y, 1)<<panelSource.basis(ptLocal.x, ptLocal.y, 2);
}


void testPanel3_3()
{
    Vector3d Vertice[3];
    Vertice[0] = Vector3d(-1.0, 0.0, 0.0);
    Vertice[1] = Vector3d( 1.0, 0.0, 0.0);
    Vertice[2] = Vector3d( 0.0, 1.0, 0.0);

        Panel3 p3(Vertice[0], Vertice[1], Vertice[2]);
    double mu0=1.0, mu1=1.0, mu2=1.0;

    double phib[3], phiBasis;
    double phiq[3], phiGQ, phiNasa;
    Vector3d Vq[3], VGQ;
    Vector3d Vb[3], VBasis, VNasa;
    Vector3d pt(3.2, -1.7, 0.5);

    Panel3::setQuadratureOrder(8);
    Panel3::setNintcheuFataMethod(true);

    p3.sourceQuadraturePotential(pt, phiGQ);
    p3.sourceQuadratureVelocity(pt, VGQ);
    p3.sourcePotential(pt, false, phiBasis);
    p3.sourceVelocity(pt, false, VBasis);
    p3.sourceN4023Potential(pt, false, phiNasa, 0.0);
    p3.sourceN4023Velocity(pt, false, VNasa, 0.0);
    qDebug(" SOURCE             GQ              tw++      NASA");
    qDebug("       phi=   %13.7g   %13.7g   %13.7g", phiGQ, phiBasis, phiNasa);
    qDebug("       Vx =   %13.7g   %13.7g   %13.7g", VGQ.x, VBasis.x, VNasa.x);
    qDebug("       Vy =   %13.7g   %13.7g   %13.7g", VGQ.y, VBasis.y, VNasa.y);
    qDebug("       Vz =   %13.7g   %13.7g   %13.7g", VGQ.z, VBasis.z, VNasa.z);

    p3.doubletQuadraturePotential(pt, phiq);
    p3.doubletQuadratureVelocity(pt, Vq);
    phiGQ = phiq[0]*mu0 + phiq[1]*mu1 + phiq[2]*mu2;
    VGQ   =   Vq[0]*mu0 +   Vq[1]*mu1 +   Vq[2]*mu2;

    p3.doubletBasisPotential(pt, false, phib, true);
    phiBasis = phib[0]*mu0 + phib[1]*mu1 + phib[2]*mu2;

    p3.doubletBasisVelocity(pt, Vb, true);
    VBasis = Vb[0]*mu0 + Vb[1]*mu1 + Vb[2]*mu2;

    qDebug(" DOUBLET            GQ              tw++");
    qDebug("       phi=   %13.7g   %13.7g", phiGQ, phiBasis);
    qDebug("       Vx =   %13.7g   %13.7g", VGQ.x, VBasis.x);
    qDebug("       Vy =   %13.7g   %13.7g", VGQ.y, VBasis.y);
    qDebug("       Vz =   %13.7g   %13.7g", VGQ.z, VBasis.z);
}


void testPanel4()
{
    Vector3d Vertice[4];
    double s =1.0;
    Vertice[0] = Vector3d( 0.0, 0.0, 0.0);
    Vertice[1] = Vector3d(   s, 0.0, 0.0);
    Vertice[2] = Vector3d( 0.0,   s, 0.0);
    Vertice[3] = Vector3d(   s,   s, 0.0);

    Panel4 p4;
    p4.setPanelFrame(Vertice[0], Vertice[2], Vertice[1], Vertice[3]);

    double phi;;
    Vector3d V;
    Vector3d pt(0.2, 0.7, 0.5);

    int n = 100;
    double halfrange = 5.0;
    double delta = 2.0*halfrange/double(n);
    double x,y,z;
    x =  0.25;
    y =  0.33;
    z = 0.0000;

    double rff = Panel::RFF();
    Panel::setRFF(4.0);
    bool bSelf = false;
    for(double t=-halfrange; t<=halfrange*(1.0+PRECISION); t+=delta)
    {
        z = t;
        pt.set(x,y,z);
//        pt = p4.CoG() + p4.normal()*t;
//        bool bSelf = fabs(t)<1.e-6;
//        p4.sourceNASA4023(pt, bSelf, V, phi, 0.000001);
        p4.doubletN4023Velocity( pt, V,   0.000001, true);
        p4.doubletN4023Potential(pt, bSelf, phi, 0.000001, true);
        qDebug(" %13.7g  %13.7g  %13.7g  %13.7g  %13.7g ", t, phi, V.x, V.y, V.z);
    }
    //restore things as they were
    Panel::setRFF(rff);
}


void testVortex()
{
/*    Vector3d A(0.0, 0.0,     0.0);
    Vector3d B(0.0, 1.0,     0.0);
    Vector3d C(0.0, -0.3213, 0.5);
    double phi;
    Panel::vortexSegmentPotential(A,B,C,phi);
    qDebug("phi=%13.7f", phi);
    return;*/

    Vector3d Vertice[4];
    double s = 1.0;
    Vertice[0] = Vector3d( 0.0, 0.0, 0.0);
    Vertice[1] = Vector3d(   s, 0.0, 0.0);
    Vertice[2] = Vector3d( 0.0,   s, 0.0);
    Vertice[3] = Vector3d(   s,   s, 0.0);

    Panel4 p4;
    p4.setPanelFrame(Vertice[0], Vertice[2], Vertice[1], Vertice[3]);

    double phiNasa;
    Vector3d VNasa, VRing;
    Vector3d pt(0.2, 0.7, 0.5);

    int n = 10;
    double halfrange = 1.5;
    double delta = 2.0*halfrange/double(n);

    Vector3d u(0.1, 0.2, 0.3);
    double rff = Panel::RFF();
    Panel::setRFF(10000.0);

    for(double t=-halfrange; t<=halfrange*(1.0+PRECISION); t+=delta)
    {
//        z = t;
//        pt = Vector3d(x,y,z) + u*t;
        pt = p4.CoG() + p4.normal()*t;
        bool bSelf = fabs(t)<1.e-6;
        bSelf = false;
//        p4.sourceNASA4023(pt, bSelf, V, phi, 0.000001);
        p4.doubletN4023Potential(pt, bSelf, phiNasa, 0.000001, false);
        p4.doubletN4023Velocity( pt, VNasa,   0.000001, false);
        p4.doubletVortexVelocity(pt, VRing, 0.000001, false);

        //num differentiate
/*        point.set(pt.x+dl, pt.y, pt.z);
        p4.doubletVortex(point, bSelf, VRing, phip, 0.000001, false);
        point.set(pt.x-dl, pt.y, pt.z);
        p4.doubletVortex(point, bSelf, VRing, phim, 0.000001, false);
        VRing.x = (phip-phim)/2/dl;

        point.set(pt.x, pt.y+dl, pt.z);
        p4.doubletVortex(point, bSelf, VRing, phip, 0.000001, false);
        point.set(pt.x, pt.y-dl, pt.z);
        p4.doubletVortex(point, bSelf, VRing, phim, 0.000001, false);
        VRing.y = (phip-phim)/2/dl;

        point.set(pt.x, pt.y, pt.z+dl);
        p4.doubletVortex(point, bSelf, VRing, phip, 0.000001, false);
        point.set(pt.x, pt.y, pt.z-dl);
        p4.doubletVortex(point, bSelf, VRing, phim, 0.000001, false);
        VRing.z = (phip-phim)/2/dl;*/

//        qDebug(" %13.7f  %13.7g  %13.7g", t, phiNasa, phiRing/4.0/PI);
        qDebug(" %13.7f  %13.7f  %13.7f  %13.7f  %13.7f  %13.7f  %13.7f", t, VNasa.x, VNasa.y, VNasa.z,
                                                                             VRing.x, VRing.y, VRing.z);
    }
    //restore things as they were
    Panel::setRFF(rff);
}


void testPanel3_1()
{
    Vector3d vtx[4];
    vtx[0] = Vector3d( 0.0, 0.0, 0.0);
    vtx[1] = Vector3d( 1.0, 0.0, 0.0);
    vtx[2] = Vector3d( 1.0, 1.0, 0.0);
    vtx[3] = Vector3d( 0.0, 1.0, 0.0);

    Panel4 p4;
    p4.setPanelFrame(vtx[3], vtx[3], vtx[0], vtx[1]);

    Panel3 p3;
    p3.setFrame(vtx[0], vtx[1], vtx[3]);

    double phiNasa;
    double phi[3];
    Vector3d VNasa, VTW, V[3];
    Vector3d pt(0.2, 0.7, 0.5);

    int n = 100;
    double halfrange = 7;
    double delta = 2.0*halfrange/double(n);
    double x,y,z;

    x = 0.43;
    y = 0.4;
    z = 0.0;

    double rff = Panel::RFF();
    Panel::setRFF(3.0);

    for(double t=-halfrange; t<=halfrange*(1.001); t+=delta)
    {
        z = t;
        pt.set(x,y,z);

        bool bSelf = fabs(z)<0.001;
/*        p3.sourceNASA4023(pt, bSelf, VNasa, phiNasa, 0.000001);
        p3.sourcePotential(pt, bSelf, phiTW);
        p3.sourceVelocity(pt, bSelf, VTW);*/

        p3.doubletN4023Potential(pt, bSelf, phiNasa, 0.0000001);
        p3.doubletN4023Velocity( pt, bSelf, VNasa, 0.0000001);
        p3.doubletBasisPotential(pt, bSelf, phi, false);
        p3.doubletBasisVelocity(pt, V);
//        phiTW = phi[0]+phi[1]+phi[2];
        VTW = V[0]+V[1]+V[2];

        qDebug(" %13.7f  %13.7f  %13.7f  %13.7f  %13.7f  %13.7f  %13.7f ", t, VNasa.x, VNasa.y, VNasa.z, VTW.x, VTW.y, VTW.z);
//        qDebug(" %13.7f  %13.7f  %13.7f ",t, phiNasa, phiTW);
    }

    //restore things as they were
    Panel::setRFF(rff);
}


void testPanel2d()
{
    Vector2d vtx[3];
    Triangle2d t2d;

    vtx[0] = Vector2d(2.0, 1.0);
    vtx[1] = Vector2d(1.0, 1.0);
    vtx[2] = Vector2d(1.0, 0.0);
    t2d.setTriangle(vtx[0], vtx[1], vtx[2]);

//    Vector2d ptGlobal(1.6,1.0);
//    Vector2d ptLocal = t2d.frame().globalToLocalPosition(ptGlobal);
//    ptLocal.displayCoords();

//    int iVertex=-1;
    int iEdge=-1;
    bool bEdge=false;
    bEdge = t2d.isOnEdge(Vector2d(1.6, 1.0), iEdge); // true, 2
    qDebug()<<"isOnEdge"<<bEdge<<iEdge;
    bEdge = t2d.isOnEdge(Vector2d(1.6, 1.1), iEdge); // false
    qDebug()<<"isOnEdge"<<bEdge<<iEdge;
    bEdge = t2d.isOnEdge(Vector2d(1.6, 0.9), iEdge); // false
    qDebug()<<"isOnEdge"<<bEdge<<iEdge;
    bEdge = t2d.isOnEdge(Vector2d(1.6, 0.6), iEdge); // true, 1
    qDebug()<<"isOnEdge"<<bEdge<<iEdge;
    bEdge = t2d.isOnEdge(Vector2d(1.0, 0.7), iEdge); // true, 0
    qDebug()<<"isOnEdge"<<bEdge<<iEdge;

    qDebug()<<"--";
    // check is inside
    qDebug()<<"  hasInside"<<t2d.isInside(0.0,0.0); // false
    qDebug()<<"  hasInside"<<t2d.isInside(1.4,0.0); // false
    qDebug()<<"  hasInside"<<t2d.isInside(1.4,1.2); // false
    qDebug()<<"  hasInside"<<t2d.isInside(0.8,0.5); // false
    qDebug()<<"  hasInside"<<t2d.isInside(2.2,0.5); // false
    qDebug()<<"  hasInside"<<t2d.isInside(1.4,0.8); // true
    qDebug()<<"  hasInside"<<t2d.isInside(1.99,0.995); // true
    qDebug()<<"  hasInside"<<t2d.isInside(1.01,0.995); // true
    qDebug()<<"  hasInside"<<t2d.isInside(1.01,1.005); // false
    qDebug()<<"  hasInside"<<t2d.isInside(1.01,0.015); // true
    qDebug()<<"--";

/*    TRIANGLE::enumPointPosition pos;
    pos = t2d.pointPosition(0.0,0.0, iVertex, iEdge); // OUTSIDE
    pos = t2d.pointPosition(1.4,0.0, iVertex, iEdge); // OUTSIDE
    pos = t2d.pointPosition(1.4,1.2, iVertex, iEdge); // OUTSIDE
    pos = t2d.pointPosition(0.8,0.5, iVertex, iEdge); // OUTSIDE
    pos = t2d.pointPosition(2.2,0.5, iVertex, iEdge); // OUTSIDE
    pos = t2d.pointPosition(1.4,0.8, iVertex, iEdge); // INSIDE
    pos = t2d.pointPosition(1.99,0.995, iVertex, iEdge); // INSIDE
    pos = t2d.pointPosition(1.01,0.995, iVertex, iEdge); // INSIDE
    pos = t2d.pointPosition(1.01,1.005, iVertex, iEdge); // OUTSIDE
    pos = t2d.pointPosition(1.01,0.015, iVertex, iEdge); // INSIDE

    pos = t2d.pointPosition(vtx[0].x,vtx[0].y, iVertex, iEdge); // VTX0
    pos = t2d.pointPosition(vtx[1].x,vtx[1].y, iVertex, iEdge); // VTX1
    pos = t2d.pointPosition(vtx[2].x,vtx[2].y, iVertex, iEdge); // VTX2

    pos = t2d.pointPosition(1.6, 0.6, iVertex, iEdge); // EDGE1
    pos = t2d.pointPosition(1.0, 0.3, iVertex, iEdge); // EDGE0
    pos = t2d.pointPosition(1.6, 1.0, iVertex, iEdge); // EDGE2*/

}


void testPanelNF1(bool bPositive)
{
    Vector3d Vertice[3];
    Vertice[0] = Vector3d( 1.0, 1.0, 0.0);
    Vertice[1] = Vector3d( 3.0, 1.0, 0.0);
    Vertice[2] = Vector3d( 1.0, 2.0, 0.0);

    Panel3 p3;
    if(bPositive)
    {
        qDebug("-----Testing positive orientation-----");
                p3.setFrame(Vertice[0], Vertice[1], Vertice[2]);
    }
    else
    {
        qDebug("-----Testing negative orientation-----");
                p3.setFrame(Vertice[0], Vertice[1], Vertice[2]); /// assert false
    }

    Vector3d ptGlobal(2.0, 1.5, 1.0);

    double G1[3], G3[6], G5[6];
    memset(G1, 0, 3*sizeof(double));
    memset(G3, 0, 6*sizeof(double));
    memset(G5, 0, 6*sizeof(double));
    p3.quadratureIntegrals(ptGlobal, G1, G3, G5);

    double MC1[3], MC3[6], MC5[6];
    memset(MC1, 0, 3*sizeof(double));
    memset(MC3, 0, 6*sizeof(double));
    memset(MC5, 0, 6*sizeof(double));
    p3.computeMCIntegrals(ptGlobal, false, MC1, MC3, MC5, true);

    double NF1[3], NF3[6], NF5[6];
    memset(NF1, 0, 3*sizeof(double));
    memset(NF3, 0, 6*sizeof(double));
    memset(NF5, 0, 6*sizeof(double));
    p3.computeNFIntegrals(ptGlobal, NF1, NF3, NF5, true);

    qDebug("1/r:      /            GQ            MC               NF");
    for(int i=0; i<3; i++)
        qDebug("    I1[%d] %15.7g  %15.7g  %15.7g", i, G1[i], MC1[i], NF1[i]);

    qDebug("1/r³:");
    for(int i=0; i<6; i++)
        qDebug("    I3[%d] %15.7g  %15.7g  %15.7g", i, G3[i], MC3[i], NF3[i]);

    qDebug("1/r⁵:");
    for(int i=0; i<6; i++)
        qDebug("    I5[%d] %15.7g  %15.7g  %15.7g", i, G5[i], MC5[i], NF5[i]);
    qDebug(" ");
}


void testPanel5()
{
    Vector3d Vertice[3];
    //global coordinates
    Vertice[0] = Vector3d( 0.0750000000000001, -0.2, -0.00263046620951234);
    Vertice[1] = Vector3d( 0.1, -0.2, 0.0);
    Vertice[2] = Vector3d( 0.0750000000000001, -0.1, -0.0026304662095123438);

        Panel3 p3(Vertice[0], Vertice[1], Vertice[2]); // negative orientation?!
//    p3.setNormal(p3.normal()*(-1.0));
    Vector3d ptGlobal(0.0833333, -0.2, 0.00087900577216089358);

    double G1[3], G3[6], G5[6];
    memset(G1, 0, 3*sizeof(double));
    memset(G3, 0, 6*sizeof(double));
    memset(G5, 0, 6*sizeof(double));
    p3.quadratureIntegrals(ptGlobal, G1, G3, G5);

    double MC1[3], MC3[6], MC5[6];
    memset(MC1, 0, 3*sizeof(double));
    memset(MC3, 0, 6*sizeof(double));
    memset(MC5, 0, 6*sizeof(double));
    p3.computeMCIntegrals(ptGlobal, false, MC1, MC3, MC5, false);

    double NF1[3], NF3[6], NF5[6];
    memset(NF1, 0, 3*sizeof(double));
    memset(NF3, 0, 6*sizeof(double));
    memset(NF5, 0, 6*sizeof(double));
    p3.computeNFIntegrals(ptGlobal, NF1, NF3, NF5, false);

    qDebug("1/r:      /            GQ            MC               NF");
    for(int i=0; i<3; i++)
        qDebug("    I1[%d] %15.7g  %15.7g  %15.7g", i, G1[i], MC1[i], NF1[i]);

    qDebug("1/r³:     /            GQ              MC                 NF");
    for(int i=0; i<6; i++)
        qDebug("    I3[%d] %15.7g  %15.7g  %15.7g", i, G3[i], MC3[i], NF3[i]);

    qDebug("1/r⁵:     /            GQ              MC                 NF");
    for(int i=0; i<6; i++)
        qDebug("    I5[%d] %15.7g  %15.7g  %15.7g", i, G5[i], MC5[i], NF5[i]);
    qDebug(" ");
    qDebug(" ");
}


double potentialP3(const Vector3d &pt, const std::vector<Panel3> &panel3, PANELMETHOD Method, bool bSource, double CoreRadius)
{
    double phib[]{0,0,0};
    double phi(0), phi_tot(0);

    for(uint i3=0; i3<panel3.size(); i3++)
    {
        Panel3 const &p3 = panel3.at(i3);
        if(Method==BASIS)
        {
            if(bSource) p3.sourcePotential(pt, false, phi); //not defined
            else
            {
                p3.doubletBasisPotential(pt, false, phib, true);
                phi = phib[0] + phib[1] + phib[2];
            }
        }
        else if(Method==NASA4023)
        {
            if(bSource) p3.sourceN4023Potential( pt, false, phi, CoreRadius);
            else        p3.doubletN4023Potential(pt, false, phi, CoreRadius, true);
        }
        phi_tot += phi;
    }

    return phi_tot;
}


double potentialP4(const Vector3d &pt, const std::vector<Panel4> &panel4, PANELMETHOD Method, bool bSource, double CoreRadius)
{
    double phi(0), phi_tot(0);

    for(uint i4=0; i4<panel4.size(); i4++)
    {
        Panel4 const &p4 = panel4.at(i4);
        if(Method==NASA4023)
        {
            if(bSource) p4.sourceN4023Potential( pt,        phi, CoreRadius);
            else        p4.doubletN4023Potential(pt, false, phi, CoreRadius, true);
        }
        else phi = 0;
        phi_tot += phi;
    }

    return phi_tot;
}


Vector3d velocityP3(Vector3d const &pt, std::vector<Panel3> const &panel3,
                    PANELMETHOD Method, bool bSource, double CoreRadius, Vortex::enumVortex VortexModel)
{
    Vector3d V, vel, Vb[3];

    for(uint i3=0; i3<panel3.size(); i3++)
    {
        Panel3 const &p3 = panel3.at(i3);
        if(bSource)
        {
            switch(Method)
            {
                case BASIS:     p3.sourceVelocity(pt, false, vel);                   break;
                case NASA4023:  p3.sourceN4023Velocity(pt, false, vel, CoreRadius);  break;
                case VORTEX:    vel.set(0,0,0);  break;
                default: vel.set(0,0,0);  break;
            }
        }
        else
        {
            switch(Method)
            {
                case BASIS:
                {
                    p3.doubletBasisVelocity(pt, Vb, true);
                    vel = Vb[0] + Vb[1] + Vb[2];
                    break;
                }
                case NASA4023:  p3.doubletN4023Velocity(pt, false, vel, CoreRadius);               break;
                case VORTEX:    p3.doubletVortexVelocity(pt, vel, CoreRadius, VortexModel, true);  break;
                default: vel.set(0,0,0);  break;
            }
        }
        V += vel;
    }

    return V;
}


Vector3d velocityP4(Vector3d const &pt, std::vector<Panel4> const &panel4, PANELMETHOD Method, bool bSource, double CoreRadius)
{
    Vector3d V, vel;

    for(uint i4=0; i4<panel4.size(); i4++)
    {
        Panel4 const &p4 = panel4.at(i4);
        if(bSource)
        {
            switch(Method)
            {
                case BASIS:     vel.set(0,0,0);  break;
                case NASA4023:  p4.sourceN4023Velocity(pt, false, vel, CoreRadius);  break;
                case VORTEX:    vel.set(0,0,0);  break;
                default: vel.set(0,0,0);  break;
            }
        }
        else
        {
            switch(Method)
            {
                case BASIS:     vel.set(0,0,0);  break;
                case NASA4023:  p4.doubletN4023Velocity(pt, vel, CoreRadius);   break;
                case VORTEX:    p4.doubletVortexVelocity(pt, vel, CoreRadius);  break;
                default: vel.set(0,0,0);  break;
            }
        }
        V += vel;
    }

    return V;
}





