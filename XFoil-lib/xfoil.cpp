/****************************************************************************

    XFoil Class
    Copyright (C) 2000 Mark Drela
    André Deperrois techwinder@users.sourceforge.net - translation to C - 2003

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#
#include <cstring>

#include "xfoil.h"

#define PI 3.141592654
#define EPSILON 1.e-6

bool XFoil::s_bCancel = false;
bool XFoil::s_bFullReport = false;
double XFoil::vaccel = 0.01;

XFoil::XFoil()
{
    //------ primary dimensioning limit parameters

    //------ derived dimensioning limit parameters
    //    nax=800;//number of points in stored polar
    //    npx=8;//number of polars and reference polars
    //    nfx=128;// number of points in one reference polar
    //    ncom = 73;

    // imx   number of complex mapping coefficients  cn
    m_bTrace = false;
    m_ctrl = 0.0;


    sccon = 5.6;
    gacon = 6.70;
    gbcon = 0.75;
    gbc0  = 0.60;
    gbc1  = 0.40;
    gccon = 18.0;
    dlcon =  0.9;
    ctcon = 0.01485111754659538130244; //(ctcon = 0.5/(gacon**2 * gbcon))
    angtol = 40.0;

    // fortran seems to initializes variables to 0
    mvisc = 0.0;

    //initialize transition parameters until user changes them
    acrit     = 9.0;
    xstrip[1] = 1.0;
    xstrip[2] = 1.0;

    //intialize analysis parameter  until user changes them
    //---- default paneling parameters
    npan = 140;
    cvpar = 1.0;
    cterat = 0.15;
    ctrrat = 0.2;

    //---- default paneling refinement zone x/c endpoints
    xsref1 = 1.0;
    xsref2 = 1.0;
    xpref1 = 1.0;
    xpref2 = 1.0;

    //---- initialize freestream mach number to zero
    matyp = 1;
    minf1 = 0.0;

    //---- drop tolerance for bl system solver
    vaccel = 0.01;
    //---- default viscous parameters
    retyp = 1;
    reinf1 = 0.0;

    initialize();
}

XFoil::~XFoil()
{
}


/** ---------------------------------------------------
 *      variable initialization/default routine.
 * --------------------------------------------------- */
bool XFoil::initialize()
{
    hopi = 0.50/PI;
    qopi = 0.25/PI;
    dtor = PI/180.0;

    n=0;// so that current airfoil is not initialized

    memset(Hk,     0, sizeof(Hk));
    memset(RTheta, 0, sizeof(RTheta));

    memset(aij,    0, sizeof(aij));
    memset(aijpiv, 0, sizeof(aijpiv));
    memset(apanel, 0, sizeof(apanel));
    memset(bij,    0, sizeof(bij));
    memset(blsav,  0, sizeof(blsav));
    memset(cij,    0, sizeof(cij));
    memset(cpi,    0, sizeof(cpi));
    memset(cpv,    0, sizeof(cpv));
    memset(ctau,   0, sizeof(ctau));
    memset(ctq,    0, sizeof(ctq));
    memset(delt,   0, sizeof(delt));
    memset(dij,    0, sizeof(dij));
    memset(dis,    0, sizeof(dis));
    memset(dq,     0, sizeof(dq));
    memset(dqdg,   0, sizeof(dqdg));
    memset(dqdm,   0, sizeof(dqdm));
    memset(dstr,   0, sizeof(dstr));
    memset(dzdg,   0, sizeof(dzdg));
    memset(dzdm,   0, sizeof(dzdm));
    memset(dzdn,   0, sizeof(dzdn));
    memset(gam,    0, sizeof(gam));
    memset(gam_a,  0, sizeof(gam_a));
    memset(gamu,   0, sizeof(gamu));
    memset(guxd,   0, sizeof(guxd));
    memset(guxq,   0, sizeof(guxq));
    memset(iblte,  0, sizeof(iblte));
    memset(ipan,   0, sizeof(ipan));
    memset(isys,   0, sizeof(isys));
    memset(itran,  0, sizeof(itran));
    memset(itran,  0, sizeof(itran));
    memset(mass,   0, sizeof(mass));
    memset(nbl,    0, sizeof(nbl));
    memset(nx,     0, sizeof(nx));
    memset(ny,     0, sizeof(ny));
    memset(q,      0, sizeof(q));
    memset(qf0,    0, sizeof(qf0));
    memset(qf1,    0, sizeof(qf1));
    memset(qf2,    0, sizeof(qf2));
    memset(qf3,    0, sizeof(qf3));
    memset(qinv,   0, sizeof(qinv));
    memset(qinv_a, 0, sizeof(qinv_a));
    memset(qinvu,  0, sizeof(qinvu));
    memset(qvis,   0, sizeof(qvis));
    memset(s,      0, sizeof(x));
    memset(sb,     0, sizeof(xb));
    memset(sig,    0, sizeof(sig));
    memset(snew,   0, sizeof(snew));
    memset(tau,    0, sizeof(tau));
    memset(thet,   0, sizeof(thet));
    memset(uedg,   0, sizeof(uedg));
    memset(uinv,   0, sizeof(uinv));
    memset(uslp,   0, sizeof(uslp));
    memset(va,     0, sizeof(va));
    memset(vb,     0, sizeof(vb));
    memset(vdel,   0, sizeof(vdel));
    memset(vm,     0, sizeof(vm));
    memset(vs1,    0, sizeof(vs1));
    memset(vs2,    0, sizeof(vs2));
    memset(vsm,    0, sizeof(vsm));
    memset(vsr,    0, sizeof(vsr));
    memset(vsrez,  0, sizeof(vsrez));
    memset(vsx,    0, sizeof(vsx));
    memset(vti,    0, sizeof(vti));
    memset(vz,     0, sizeof(vz));
    memset(w1,     0, sizeof(w1));
    memset(w2,     0, sizeof(w2));
    memset(w3,     0, sizeof(w3));
    memset(w4,     0, sizeof(w4));
    memset(w5,     0, sizeof(w5));
    memset(w6,     0, sizeof(w6));
    memset(w7,     0, sizeof(w7));
    memset(w8,     0, sizeof(w8));
    memset(wgap,   0, sizeof(wgap));
    memset(x,      0, sizeof(x));
    memset(xb,     0, sizeof(xb));
    memset(xbl,    0, sizeof(xbl));
    memset(xbp,    0, sizeof(xbp));
    memset(xp,     0, sizeof(xp));
    memset(xssi,   0, sizeof(xssi));
    memset(y,      0, sizeof(y));
    memset(yb,     0, sizeof(yb));
    memset(ybp,    0, sizeof(ybp));
    memset(yp,     0, sizeof(yp));

    m_nSide1 = m_nSide2 = 0;
    //mdes
    memset(wc,     0, sizeof(wc));
    memset(sc,     0, sizeof(sc));
    memset(scold,  0, sizeof(scold));
    memset(xcold,  0, sizeof(xcold));
    memset(ycold,  0, sizeof(ycold));
    memset(sspec,  0, sizeof(sspec));
    memset(xspoc,  0, sizeof(xspoc));
    memset(yspoc,  0, sizeof(yspoc));
    memset(qgamm,  0, sizeof(qgamm));
    memset(qspec,  0, sizeof(qspec));
    memset(qspecp, 0, sizeof(qspecp));
    memset(alqsp,  0, sizeof(alqsp));
    memset(clqsp,  0, sizeof(clqsp));
    memset(cmqsp,  0, sizeof(cmqsp));

    memset(xcam,  0, IQX*sizeof(double));
    memset(ycam,  0, IQX*sizeof(double));
    memset(xthk,  0, IQX*sizeof(double));
    memset(ythk,  0, IQX*sizeof(double));
    memset(ycamp, 0, IQX*sizeof(double));
    memset(ythkp, 0, IQX*sizeof(double));
    ncam = nthk = 0;

    agte = 0.0;
    ag0 = 0.0;
    qim0 = 0.0;
    qimold = 0.0;
    ssple = 0.0;
    dwc = 0.0;
    algam = 0.0;
    clgam = 0.0;
    cmgam = 0.0;

    niterq = 6;

    //---- default cp/cv (air)
    gamma = 1.4;
    gamm1 = gamma - 1.0;

    //---- set unity freestream speed
    qinf = 1.0;

    psio = 0.0;

    cl = 0.0;
    cm = 0.0;
    cd = 0.0;

    sigte = 0.0;
    gamte = 0.0;
    //    sigte_a = 0.0;
    //    gamte_a = 0.0;

    nsp = 0;
    nqsp = 0;

    awake = 0.0;
    avisc = 0.0;

    //    kimage = 1;
    yimage = -10.0;
    limage = false;

    liqset = false; //???
    lgamu  = false;
    lqinu  = false;//???
    lvisc  = false;
    lwake  = false;
    //    lpacc  = false;
    lblini = false;
    lipan  = false;
    lqaij  = false;
    ladij  = false;
    lwdij  = false;
    lcpxx  = false;
    //    lqvdes = false;
    lqspec = false;
    //    lqrefl = false;
    lvconv = false;
    //    lcpref = false;
    //    lforef = false;
    //    lpfile = false;
    //    lpfilx = false;
    //    lppsho = false;
    leiw   = false;
    lscini = false;

    //    lclip  = false;
    //    lvlab  = true;
    //    lcminp = false;
    //    lhmomp = false;

    //    lcurs  = true;
    //    lland  = true;
    lgsame = false;

    //    lgparm = true;
    //    lplcam = false;

    sharp = false;
    lalfa = false;
    lbflap = false;
    lflap = false;
    trforc = false;
    simi = false;
    tran = false;
    turb = false;
    wake = false;
    trfree = false;
    tforce[0] =false;
    tforce[1] =false;
    tforce[2] =false;

    thickb = 0.0;
    cambrb = 0.0;

    //---- input airfoil will not be normalized
    //    lnorm = false;

    //---- airfoil will not be forced symmetric
    lqsym = false;
    //    lgsym = false;

    //---- endpoint slopes will be matched
    lqslop = true;
    //    lgslop = true;
    //    lcslop = true;

    //---- buffer and current airfoil flap hinge coordinates
    xbf = 0.0;
    ybf = 0.0;
    xof = 0.0;
    yof = 0.0;

    //    ncpref = 0;
    //                                       n
    //---- circle plane array size (largest 2  + 1 that will fit array size)
    double ann = log(double((2*IQX)-1))/log(2.0);
    int nn = int( ann + 0.00001 );
    int tmp = 1;

    for (int l=0; l<nn; l++){
        tmp = 2*tmp;
    }
    nc1 = tmp + 1;
    //    nc1 = (int)pow(2,nn) + 1;
    if(nc1 > ICX) {
        tmp = 1;
        for (int l=0; l<nn-1; l++){
            tmp = 2*tmp;
        }
        nc1 = tmp+1;
        //        nc1 = pow(2,(nn-1)) + 1; //257 instead of ICX in original source code
    }

    //---- default cm reference location
    xcmref = 0.25;
    ycmref = 0.0;

    xoctr[1] = 1.0;
    xoctr[2] = 1.0;
    yoctr[1] = 0.0;
    yoctr[2] = 0.0;
    waklen = 1.0;

    //added techwinder : no wake yet
    nw = 0;

    //added techwinder : no flap yet
    hmom = 0.0;
    hfx  = 0.0;
    hfy  = 0.0;

    //added techwinder : fortran initializes to 0
    imxbl  = 0;
    ismxbl = 0;
    ist = 0;
    nb =0;

    iacqsp = 1;

    dwte = 0.0;
    qinfbl = 0.0;
    tkbl = 0.0;
    tkbl_ms = 0.0;
    rstbl = 0.0;
    rstbl_ms = 0.0;
    hstinv = 0.0;
    hstinv_ms = 0.0;
    reybl = 0.0;
    reybl_ms = 0.0;
    reybl_re = 0.0;
    gambl = 0.0;
    gm1bl = 0.0;
    hvrat = 0.0;
    bule = 0.0;
    xiforc = 0.0;
    amcrit = 0.0;
    x2 = 0.0;
    u2 = 0.0;
    theta2 = 0.0;
    d2 = 0.0;
    s2 = 0.0;
    ampl2 = 0.0;
    u2_uei = 0.0;
    u2_ms = 0.0;
    dw2 = 0.0;
    h2 = 0.0;
    h2_t2 = 0.0;
    h2_d2 = 0.0;
    m2 = 0.0;
    m2_u2 = 0.0;
    m2_ms = 0.0;
    r2 = 0.0;
    r2_u2 = 0.0;
    r2_ms = 0.0;
    v2 = 0.0;
    v2_u2 = 0.0;
    v2_ms = 0.0;
    v2_re = 0.0;
    hk2 = 0.0;
    hk2_u2 = 0.0;
    hk2_t2 = 0.0;
    hk2_d2 = 0.0;
    hk2_ms = 0.0;
    hs2 = 0.0;
    hs2_u2 = 0.0;
    hs2_t2 = 0.0;
    hs2_d2 = 0.0;
    hs2_ms = 0.0;
    hs2_re = 0.0;
    hc2 = 0.0;
    hc2_u2 = 0.0;
    hc2_t2 = 0.0;
    hc2_d2 = 0.0;
    hc2_ms = 0.0;
    rt2 = 0.0;
    rt2_u2 = 0.0;
    rt2_t2 = 0.0;
    rt2_ms = 0.0;
    rt2_re = 0.0;
    cf2 = 0.0;
    cf2_u2 = 0.0;
    cf2_t2 = 0.0;
    cf2_d2 = 0.0;
    cf2_ms = 0.0;
    cf2_re = 0.0;
    di2 = 0.0;
    di2_u2 = 0.0;
    di2_t2 = 0.0;
    di2_d2 = 0.0;
    di2_s2 = 0.0;
    di2_ms = 0.0;
    di2_re = 0.0;
    us2 = 0.0;
    us2_u2 = 0.0;
    us2_t2 = 0.0;
    us2_d2 = 0.0;
    us2_ms = 0.0;
    us2_re = 0.0;
    cq2 = 0.0;
    cq2_u2 = 0.0;
    cq2_t2 = 0.0;
    cq2_d2 = 0.0;
    cq2_ms = 0.0;
    cq2_re = 0.0;
    de2 = 0.0;
    de2_u2 = 0.0;
    de2_t2 = 0.0;
    de2_d2 = 0.0;
    de2_ms = 0.0;
    x1 = 0.0;
    u1 = 0.0;
    theta1 = 0.0;
    d1 = 0.0;
    s1 = 0.0;
    ampl1 = 0.0;
    u1_uei = 0.0;
    u1_ms = 0.0;
    dw1 = 0.0;
    h1 = 0.0;
    h1_t1 = 0.0;
    h1_d1 = 0.0;
    m1 = 0.0;
    m1_u1 = 0.0;
    m1_ms = 0.0;
    r1 = 0.0;
    r1_u1 = 0.0;
    r1_ms = 0.0;
    v1 = 0.0;
    v1_u1 = 0.0;
    v1_ms = 0.0;
    v1_re = 0.0;
    hk1 = 0.0;
    hk1_u1 = 0.0;
    hk1_t1 = 0.0;
    hk1_d1 = 0.0;
    hk1_ms = 0.0;
    hs1 = 0.0;
    hs1_u1 = 0.0;
    hs1_t1 = 0.0;
    hs1_d1 = 0.0;
    hs1_ms = 0.0;
    hs1_re = 0.0;
    hc1 = 0.0;
    hc1_u1 = 0.0;
    hc1_t1 = 0.0;
    hc1_d1 = 0.0;
    hc1_ms = 0.0;
    rt1 = 0.0;
    rt1_u1 = 0.0;
    rt1_t1 = 0.0;
    rt1_ms = 0.0;
    rt1_re = 0.0;
    cf1 = 0.0;
    cf1_u1 = 0.0;
    cf1_t1 = 0.0;
    cf1_d1 = 0.0;
    cf1_ms = 0.0;
    cf1_re = 0.0;
    di1 = 0.0;
    di1_u1 = 0.0;
    di1_t1 = 0.0;
    di1_d1 = 0.0;
    di1_s1 = 0.0;
    di1_ms = 0.0;
    di1_re = 0.0;
    us1 = 0.0;
    us1_u1 = 0.0;
    us1_t1 = 0.0;
    us1_d1 = 0.0;
    us1_ms = 0.0;
    us1_re = 0.0;
    cq1 = 0.0;
    cq1_u1 = 0.0;
    cq1_t1 = 0.0;
    cq1_d1 = 0.0;
    cq1_ms = 0.0;
    cq1_re = 0.0;
    de1 = 0.0;
    de1_u1 = 0.0;
    de1_t1 = 0.0;
    de1_d1 = 0.0;
    de1_ms = 0.0;
    xsf = 0.0;
    ysf = 0.0;
    cdp = 0.0;
    cdf = 0.0;
    alfa = 0.0;
    amax = 0.0;
    adeg = 0.0;
    rmxbl = 0.0;
    rmsbl = 0.0;
    rlx = 0.0;
    ante  = 0.0;
    ddef = 0.0;
    cpmn = 0.0;
    clspec = 0.0;
    minf = 0.0;
    reinf = 0.0;
    minf_cl = 0.0;
    reinf_cl = 0.0;

    sble = 0.0;
    chordb = 0.0;
    areab = 0.0;
    radble = 0.0;
    angbte = 0.0;
    ei11ba = 0.0;
    ei22ba = 0.0;
    apx1ba = 0.0;
    apx2ba = 0.0;
    ei11bt = 0.0;
    ei22bt = 0.0;
    apx1bt = 0.0;
    apx2bt = 0.0;
    sle = 0.0;
    xle = 0.0;
    yle = 0.0;
    xte = 0.0;
    yte = 0.0;
    chord = 0.0;
    ch = 0.0;
    cl_alf = 0.0;
    cl_msq = 0.0;
    cosa = 0.0;
    sina = 0.0;
    tklam = 0.0;
    tkl_msq = 0.0;
    cpstar = 0.0;
    qstar = 0.0;
    cpmni = 0.0;
    cpmnv = 0.0;
    xcpmni = 0.0;
    xcpmnv = 0.0;
    arad = 0.0;
    sst = 0.0;
    sst_go = 0.0;
    sst_gp = 0.0;
    dste = 0.0;
    aste = 0.0;

    qtan1 = 0.0;
    qtan2 = 0.0;
    z_qinf = 0.0;
    z_alfa = 0.0;
    z_qdof0 = 0.0;
    z_qdof1 = 0.0;
    z_qdof2 = 0.0;
    z_qdof3 = 0.0;
    cfm = 0.0;
    cfm_ms = 0.0;
    cfm_re = 0.0;
    cfm_u1 = 0.0;
    cfm_t1 = 0.0;
    cfm_d1 = 0.0;
    cfm_u2 = 0.0;
    cfm_t2 = 0.0;
    cfm_d2 = 0.0;
    xt = 0.0;
    xt_a1 = 0.0;
    xt_ms = 0.0;
    xt_re = 0.0;
    xt_xf = 0.0;
    xt_x1 = 0.0;
    xt_t1 = 0.0;
    xt_d1 = 0.0;
    xt_u1 = 0.0;
    xt_x2 = 0.0;
    xt_t2 = 0.0;
    xt_d2 = 0.0;
    xt_u2 = 0.0;

    npan = 140;
    cvpar = 1.0;
    cterat = 0.15;
    ctrrat = 0.2;

    //---- default paneling refinement zone x/c endpoints
    xsref1 = 1.0;
    xsref2 = 1.0;
    xpref1 = 1.0;
    xpref2 = 1.0;

    //---- drop tolerance for bl system solver
    vaccel = 0.01;



    //---- set minf, reinf, based on current cl-dependence
    mrcl(1.0, minf_cl, reinf_cl);

    //---- set various compressibility parameters from minf
    comset();

    return true;
}

bool XFoil::abcopy()
{
    int i(0);
    if(nb<=1)
    {
        writeString("abcopy: buffer airfoil not available");
        return false;
    }
    else if(nb>IQX-2)
    {
        std::stringstream ss;
        ss <<"Excessive number of airfoil points surface points: " << nb <<"\n";
        ss <<"Current airfoil cannot be set\n";
        writeString(ss.str());
        return false;
    }
    if(n!=nb) lblini = false;

    n = nb;
    for (i=1; i<=n; i++){
        x[i] = xb[i];
        y[i] = yb[i];
    }
    lgsame = true;

    if(lbflap)
    {
        xof = xbf;
        yof = ybf;
        lflap = true;
    }

    //---- strip out doubled points
    i = 1;

    while (i<n)
    {
        i++;
        if(fabs(x[i-1]-x[i])<EPSILON && fabs(y[i-1]-y[i])<EPSILON)
        {
            for (int j=i; j<=n-1; j++){
                x[j] = x[j+1];
                y[j] = y[j+1];
            }
            n = n-1;
        }
    }

    scalc(x,y,s,n);
    segspl(x,xp,s,n);
    segspl(y,yp,s,n);
    ncalc(x,y,s,n,nx,ny);
    lefind(sle,x,xp,y,yp,s,n);
    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);
    chord  = sqrt( (xte-xle)*(xte-xle) + (yte-yle)*(yte-yle) );
    tecalc();
    apcalc();

    lgamu = false;
    lqinu = false;
    lwake = false;
    lqaij = false;
    ladij = false;
    lwdij = false;
    lipan = false;
    lvconv = false;
    //    lscini = false;

    //   write(*,1200) n
    // 1200 format(/' current airfoil nodes set from buffer airfoil nodes (', i4,' )')

    return true;
}


/** ---------------------------------------------------------------
 *      calculates geometric properties of shape x,y
 *
 *      input:
 *        n      number of points
 *        x(.)   shape coordinate point arrays
 *        y(.)
 *        t(.)   skin-thickness array, used only if itype = 2
 *        itype  = 1 ...   integration is over whole area  dx dy
 *               = 2 ...   integration is over skin  area   t ds
 *
 *      output:
 *        xcen,ycen  centroid location
 *        ei11,ei22  principal moments of inertia
 *        apx1,apx2  principal-axis angles
 * ---------------------------------------------------------------*/
bool XFoil::aecalc(int n, double x[], double y[], double t[], int itype, double &area,
                   double &xcen, double &ycen, double &ei11, double &ei22,
                   double &apx1, double &apx2)
{
    double sint(0), aint(0), xint(0), yint(0), xxint(0), yyint(0), xyint(0);
    double eixx(0), eiyy(0), eixy(0), eisq(0);
    double dx(0), dy(0), xa(0), ya(0), ta(0), ds(0), da(0), c1(0), c2(0), sgn(0);
    int ip(0), io(0);

    sint  = 0.0;
    aint  = 0.0;
    xint  = 0.0;
    yint  = 0.0;
    xxint = 0.0;
    xyint = 0.0;
    yyint = 0.0;

    for (io = 1; io<= n; io++)
    {
        if(io==n) ip = 1;
        else ip = io + 1;


        dx =  x[io] - x[ip];
        dy =  y[io] - y[ip];
        xa = (x[io] + x[ip])*0.50;
        ya = (y[io] + y[ip])*0.50;
        ta = (t[io] + t[ip])*0.50;

        ds = sqrt(dx*dx + dy*dy);
        sint = sint + ds;

        if(itype==1)
        {
            //-------- integrate over airfoil cross-section
            da = ya*dx;
            aint  = aint  +       da;
            xint  = xint  + xa   *da;
            yint  = yint  + ya   *da/2.0;
            xxint = xxint + xa*xa*da;
            xyint = xyint + xa*ya*da/2.0;
            yyint = yyint + ya*ya*da/3.0;
        }
        else
        {
            //-------- integrate over skin thickness
            da = ta*ds;
            aint  = aint  +       da;
            xint  = xint  + xa   *da;
            yint  = yint  + ya   *da;
            xxint = xxint + xa*xa*da;
            xyint = xyint + xa*ya*da;
            yyint = yyint + ya*ya*da;
        }
    }

    area = aint;

    if(aint == 0.0)
    {
        xcen  = 0.0;
        ycen  = 0.0;
        ei11  = 0.0;
        ei22  = 0.0;
        apx1 = 0.0;
        apx2 = atan2(1.0,0.0);
        return false;
    }

    //---- calculate centroid location
    xcen = xint/aint;
    ycen = yint/aint;

    //---- calculate inertias
    eixx = yyint - (ycen)*(ycen)*aint;
    eixy = xyint - (xcen)*(ycen)*aint;
    eiyy = xxint - (xcen)*(xcen)*aint;

    //---- set principal-axis inertias, ei11 is closest to "up-down" bending inertia
    eisq  = 0.25*(eixx - eiyy)*(eixx - eiyy)  + eixy*eixy;
    sgn = sign(1.0 , eiyy-eixx );
    ei11 = 0.5*(eixx + eiyy) - sgn*sqrt(eisq);
    ei22 = 0.5*(eixx + eiyy) + sgn*sqrt(eisq);

    if(ei11==0.0 || ei22==0.0)
    {
        //----- vanishing section stiffness
        apx1 = 0.0;
        apx2 = atan2(1.0,0.0);
    }
    else
    {
        if(eisq/((ei11)*(ei22)) < pow((0.001*sint),4.0))
        {
            //----- rotationally-invariant section (circle, square, etc.)
            apx1 = 0.0;
            apx2 = atan2(1.0,0.0);
        }
        else
        {
            //----- normal airfoil section
            c1 = eixy;
            s1 = eixx-ei11;

            c2 = eixy;
            s2 = eixx-ei22;

            if(fabs(s1)>fabs(s2)) {
                apx1 = atan2(s1,c1);
                apx2 = apx1 + 0.5*PI;
            }
            else{
                apx2 = atan2(s2,c2);
                apx1 = apx2 - 0.5*PI;
            }

            if(apx1<-0.5*PI) apx1 = apx1 + PI;
            if(apx1>+0.5*PI) apx1 = apx1 - PI;
            if(apx2<-0.5*PI) apx2 = apx2 + PI;
            if(apx2>+0.5*PI) apx2 = apx2 - PI;

        }
    }

    return true;
}


double XFoil::aint(double number)
{
    if(number>=0) return double( int( number));
    else          return double(-int(-number));
}


bool XFoil::apcalc()
{
    double sx(0), sy(0);
    int i(0), ip(0);

    //---- set angles of airfoil panels
    for (i=1; i<=n-1; i++)
    {
        sx = x[i+1] - x[i];
        sy = y[i+1] - y[i];
        if(sx==0.0 && sy==0.0) apanel[i] = atan2(-ny[i], -nx[i]);
        else                   apanel[i] = atan2(sx, -sy );
    }

    //---- TE panel
    i = n;
    ip = 1;
    if(sharp) apanel[i] = PI;
    else
    {
        sx = x[ip] - x[i];
        sy = y[ip] - y[i];
        apanel[i] = atan2( -sx , sy ) + PI;
    }

    return true;
}


/** -------------------------------------------------------------
 *     atan2 function with branch cut checking.
 *
 *     increments position angle of point x,y from some previous
 *     value thold due to a change in position, ensuring that the
 *     position change does not cross the atan2 branch cut
 *     (which is in the -x direction).  for example:
 *
 *       atanc( -1.0 , -1.0 , 0.75*PI )  returns  1.25*PI , whereas
 *       atan2( -1.0 , -1.0 )            returns  -.75*PI .
 *
 *     typically, atanc is used to fill an array of angles:
 *
 *        theta(1) = atan2( y(1) , x(1) )
 *        do i=2, n
 *          theta[i] = atanc( y[i] , x[i] , theta(i-1) )
 *        end do
 *
 *     this will prevent the angle array theta(i) from jumping by
 *     +/- 2 pi when the path x(i),y(i) crosses the negative x axis.
 *
 *     input:
 *       x,y     point position coordinates
 *       thold   position angle of nearby point
 *
 *     output:
 *       atanc   position angle of x,y
 * -------------------------------------------------------------- */
double XFoil::atanc(double y, double x, double thold)
{
    double tpi(0), thnew(0), dthet(0), dtcorr(0);
    tpi = 6.2831853071795864769;

    //---- set new position angle, ignoring branch cut in atan2 function for now

    thnew = atan2( y , x );
    dthet = thnew - thold;

    //---- angle change cannot exceed +/- pi, so get rid of any multiples of 2 pi
    dtcorr = dthet - tpi*int( (dthet + sign(PI,dthet))/tpi );

    //---- set correct new angle
    return thold + dtcorr;

}


/** ----------------------------------------------------------
 *      returns average amplification ax over interval 1..2
 * ----------------------------------------------------------- */
bool XFoil::axset(double hk1, double t1, double rt1, double a1,
                  double hk2, double t2, double rt2, double a2,
                  double acrit, double &ax,
                  double &ax_hk1, double &ax_t1, double &ax_rt1, double &ax_a1,
                  double &ax_hk2, double &ax_t2, double &ax_rt2, double &ax_a2)
{
    //
    //==========================
    //---- 2nd-order
    double ax1(0), ax2(0), ax1_hk1(0), ax1_t1(0), ax1_rt1(0);
    double ax2_hk2(0), ax2_t2(0), ax2_rt2(0), axsq(0);
    double axa(0), axa_ax1(0), axa_ax2(0);
    double exn(0), exn_a1(0), exn_a2(0), dax(0), dax_a1(0), dax_a2(0), dax_t1(0), dax_t2(0);
    double f_arg(0);//ex arg

    dampl(hk1, t1, rt1, ax1, ax1_hk1, ax1_t1, ax1_rt1);
    dampl(hk2, t2, rt2, ax2, ax2_hk2, ax2_t2, ax2_rt2);

    //---- rms-average version (seems a little better on coarse grids)
    axsq = 0.5*(ax1*ax1 + ax2*ax2);
    if(axsq <= 0.0)
    {
        axa = 0.0;
        axa_ax1 = 0.0;
        axa_ax2 = 0.0;
    }
    else
    {
        axa = sqrt(axsq);
        axa_ax1 = 0.5*ax1/axa;
        axa_ax2 = 0.5*ax2/axa;
    }

    //----- small additional term to ensure  dn/dx > 0  near  n = ncrit
    f_arg = std::min(20.0*(acrit-0.5*(a1+a2)) , 20.0);
    if(f_arg<=0.0)
    {
        exn    = 1.0;
        exn_a1 = 0.0;
        exn_a2 = 0.0;
    }
    else
    {
        exn    = exp(-f_arg);
        exn_a1 =  20.0*0.5*exn;
        exn_a2 =  20.0*0.5*exn;
    }

    dax    = exn    * 0.002/(t1+t2);
    dax_a1 = exn_a1 * 0.002/(t1+t2);
    dax_a2 = exn_a2 * 0.002/(t1+t2);
    dax_t1 = -dax/(t1+t2);
    dax_t2 = -dax/(t1+t2);

    //==========================

    ax     = axa             + dax;
    ax_hk1 = axa_ax1*ax1_hk1;
    ax_t1  = axa_ax1*ax1_t1  + dax_t1;
    ax_rt1 = axa_ax1*ax1_rt1;
    ax_a1  =                   dax_a1;

    ax_hk2 = axa_ax2*ax2_hk2;
    ax_t2  = axa_ax2*ax2_t2  + dax_t2;
    ax_rt2 = axa_ax2*ax2_rt2;
    ax_a2  =                   dax_a2;

    return true;
}


/** -----------------------------------------------------------
 *     sets up the newton system coefficients and residuals
 *
 *         ityp = 0 :  similarity station
 *         ityp = 1 :  laminar interval
 *         ityp = 2 :  turbulent interval
 *         ityp = 3 :  wake interval
 *
 *      this routine knows nothing about a transition interval,
 *      which is taken care of by trdif.
 * ------------------------------------------------------------ */
bool XFoil::bldif(int ityp)
{
    int k(0),l(0);
    double hupwt(0),hdcon(0), hl(0), hd_hk1(0),hd_hk2(0), hlsq(0), ehh(0);
    double upw(0), upw_hl(0), upw_hd(0), upw_hk1(0), upw_hk2(0), upw_u1(0), upw_t1(0), upw_d1(0);
    double upw_u2(0), upw_t2(0), upw_d2(0), upw_ms(0);
    double dxi(0), slog(0), scc(0), scc_usa(0);
    double ax(0), ax_t1(0), ax_rt1(0), ax_a1(0), ax_hk2(0), ax_t2(0), ax_rt2(0), ax_a2(0);
    double rezc(0), z_ax(0), hr(0), hr_hka(0), hr_rta(0);
    double hl_hk1(0), hl_hk2(0), ax_hk1(0), sa(0), cqa(0), cfa(0), hka(0), usa(0), rta(0), dea(0), da(0), ald(0);
    double gcc(0), hkc(0), hkc_hka(0), hkc_rta(0), rezt(0), rezh(0);
    double btmp(0), hwa(0), ha(0), ma(0), xa(0), ta(0), xlog(0), ulog(0), tlog(0), hlog(0), ddlog(0);
    double z_cfx(0), z_ha(0), z_hwa(0), z_ma(0), z_xl(0), z_tl(0), z_cfm(0), z_t1(0), z_t2(0);
    double z_dix(0), z_hca(0), z_hl(0), z_hs1(0), z_hs2(0), z_di1(0), z_di2(0);
    double z_cfa(0), z_hka(0), z_da(0), z_sl(0), z_ul(0), z_dxi(0), z_usa(0), z_cqa(0), z_sa(0), z_dea(0);
    double z_upw(0), z_de1(0), z_de2(0), z_us1(0), z_us2(0), z_d1(0), z_d2(0), z_u1(0), z_u2(0), z_x1(0), z_x2(0);
    double z_s1(0), z_s2(0), z_cq1(0), z_cq2(0), z_cf1(0), z_cf2(0), z_hk1(0), z_hk2(0);
    double cfx(0), cfx_xa(0), cfx_ta(0), cfx_x1(0), cfx_x2(0), cfx_t1(0), cfx_t2(0), cfx_cf1(0), cfx_cf2(0), cfx_cfm(0);
    double xot1(0), xot2(0), hca(0), hsa(0), dix(0), dix_upw(0), cfx_upw(0);
    double uq(0), uq_hka(0), uq_rta(0), uq_cfa(0), uq_da(0), uq_upw(0), uq_t1(0), uq_t2(0);
    double uq_d1(0), uq_d2(0), uq_u1(0), uq_u2(0), uq_ms(0), uq_re(0);
    double f_arg(0);// ex arg

    //    double scc_us1, scc_us2;

    if(ityp==0)
    {
        //----- similarity logarithmic differences  (prescribed)
        xlog = 1.0;
        ulog = bule;
        tlog = 0.5*(1.0 - bule);
        hlog = 0.0;
        ddlog = 0.0;
    }
    else
    {
        //----- usual logarithmic differences
        xlog = log(x2/x1);
        ulog = log(u2/u1);
        tlog = log(theta2/theta1);
        hlog = log(hs2/hs1);
        ddlog = 1.0;
    }

    for (k=1; k<=4;k++)
    {
        vsrez[k] = 0.0;
        vsm[k] = 0.0;
        vsr[k] = 0.0;
        vsx[k] = 0.0;
        for (l=1;l<=5;l++)
        {
            vs1[k][l] = 0.0;
            vs2[k][l] = 0.0;
        }
    }

    //---- set triggering constant for local upwinding
    hupwt = 1.0;

    hdcon  =  5.0*hupwt/hk2/hk2;
    hd_hk1 =  0.0;
    hd_hk2 = -hdcon*2.0/hk2;

    //---- use less upwinding in the wake
    if(ityp==3)
    {
        hdcon  =  hupwt/hk2/hk2;
        hd_hk1 =  0.0;
        hd_hk2 = -hdcon*2.0/hk2;
    }
    //
    //---- local upwinding is based on local change in  log(hk-1)
    //-    (mainly kicks in at transition)
    f_arg = fabs((hk2-1.0)/(hk1-1.0));
    hl = log(f_arg);
    hl_hk1 = -1.0/(hk1-1.0);
    hl_hk2 =  1.0/(hk2-1.0);

    hlsq = std::min(hl*hl, 15.0);
    ehh = exp(-hlsq*hdcon);
    upw = 1.0 - 0.5*ehh;
    upw_hl =        ehh * hl  *hdcon;
    upw_hd =    0.5*ehh * hlsq;

    upw_hk1 = upw_hl*hl_hk1 + upw_hd*hd_hk1;
    upw_hk2 = upw_hl*hl_hk2 + upw_hd*hd_hk2;

    upw_u1 = upw_hk1*hk1_u1;
    upw_t1 = upw_hk1*hk1_t1;
    upw_d1 = upw_hk1*hk1_d1;
    upw_u2 = upw_hk2*hk2_u2;
    upw_t2 = upw_hk2*hk2_t2;
    upw_d2 = upw_hk2*hk2_d2;
    upw_ms = upw_hk1*hk1_ms + upw_hk2*hk2_ms;


    if(ityp==0) {

        //***** le point -->  set zero amplification factor
        vs2[1][1] = 1.0;
        vsr[1]   = 0.0;
        vsrez[1] = -ampl2;
    }
    else
    {
        if(ityp==1)
        {
            //***** laminar part -->  set amplification equation
            //----- set average amplification ax over interval x1..x2

            axset(hk1, theta1, rt1, ampl1,
                  hk2, theta2, rt2, ampl2,
                  amcrit, ax,
                  ax_hk1, ax_t1, ax_rt1, ax_a1,
                  ax_hk2, ax_t2, ax_rt2, ax_a2 );

            rezc = ampl2 - ampl1 - ax*(x2-x1);
            z_ax = -(x2-x1);

            vs1[1][1] = z_ax* ax_a1  -  1.0;
            vs1[1][2] = z_ax*(ax_hk1*hk1_t1 + ax_t1 + ax_rt1*rt1_t1);
            vs1[1][3] = z_ax*(ax_hk1*hk1_d1                        );
            vs1[1][4] = z_ax*(ax_hk1*hk1_u1         + ax_rt1*rt1_u1);
            vs1[1][5] =  ax;
            vs2[1][1] = z_ax* ax_a2  +  1.0;
            vs2[1][2] = z_ax*(ax_hk2*hk2_t2 + ax_t2 + ax_rt2*rt2_t2);
            vs2[1][3] = z_ax*(ax_hk2*hk2_d2                        ) ;
            vs2[1][4] = z_ax*(ax_hk2*hk2_u2         + ax_rt2*rt2_u2);
            vs2[1][5] = -ax;
            vsm[1]   = z_ax*(ax_hk1*hk1_ms         + ax_rt1*rt1_ms    + ax_hk2*hk2_ms         + ax_rt2*rt2_ms);
            vsr[1]   = z_ax*(                        ax_rt1*rt1_re    + ax_rt2*rt2_re);
            vsx[1]   = 0.0;
            vsrez[1] = -rezc;
        }
        else
        {

            //***** turbulent part -->  set shear lag equation

            sa  = (1.0-upw)*s1  + upw*s2;
            cqa = (1.0-upw)*cq1 + upw*cq2;
            cfa = (1.0-upw)*cf1 + upw*cf2;
            hka = (1.0-upw)*hk1 + upw*hk2;

            usa = 0.5*(us1 + us2);
            rta = 0.5*(rt1 + rt2);
            dea = 0.5*(de1 + de2);
            da  = 0.5*(d1  + d2 );

            if(ityp==3) ald = dlcon;//------ increased dissipation length in wake (decrease its reciprocal)
            else  ald = 1.0;

            //----- set and linearize  equilibrium 1/ue due/dx   ...  new  12 oct 94
            if(ityp==2)
            {
                gcc = gccon;
                hkc     = hka - 1.0 - gcc/rta;
                hkc_hka = 1.0;
                hkc_rta =  gcc/rta/rta;
                if(hkc < 0.01)
                {
                    hkc = 0.01;
                    hkc_hka = 0.0;
                    hkc_rta = 0.0;
                }
            }
            else
            {
                gcc = 0.0;
                hkc = hka - 1.0;
                hkc_hka = 1.0;
                hkc_rta = 0.0;
            }

            hr     = hkc     / (gacon*ald*hka);
            hr_hka = hkc_hka / (gacon*ald*hka) - hr / hka;
            hr_rta = hkc_rta / (gacon*ald*hka);

            uq     = (0.5*cfa - hr*hr) / (gbcon*da);
            uq_hka =   -2.0*hr*hr_hka  / (gbcon*da);
            uq_rta =   -2.0*hr*hr_rta  / (gbcon*da);
            uq_cfa =   0.5             / (gbcon*da);
            uq_da  = -uq/da;
            uq_upw = uq_cfa*(cf2-cf1) + uq_hka*(hk2-hk1);

            uq_t1 = (1.0-upw)*(uq_cfa*cf1_t1 + uq_hka*hk1_t1) + uq_upw*upw_t1;
            uq_d1 = (1.0-upw)*(uq_cfa*cf1_d1 + uq_hka*hk1_d1) + uq_upw*upw_d1;
            uq_u1 = (1.0-upw)*(uq_cfa*cf1_u1 + uq_hka*hk1_u1) + uq_upw*upw_u1;
            uq_t2 =       upw *(uq_cfa*cf2_t2 + uq_hka*hk2_t2) + uq_upw*upw_t2;
            uq_d2 =       upw *(uq_cfa*cf2_d2 + uq_hka*hk2_d2) + uq_upw*upw_d2;
            uq_u2 =       upw *(uq_cfa*cf2_u2 + uq_hka*hk2_u2) + uq_upw*upw_u2;
            uq_ms = (1.0-upw)*(uq_cfa*cf1_ms + uq_hka*hk1_ms) + uq_upw*upw_ms
                    +       upw *(uq_cfa*cf2_ms + uq_hka*hk2_ms);
            uq_re = (1.0-upw)* uq_cfa*cf1_re + upw * uq_cfa*cf2_re;

            uq_t1 = uq_t1             + 0.5*uq_rta*rt1_t1;
            uq_d1 = uq_d1 + 0.5*uq_da;
            uq_u1 = uq_u1             + 0.5*uq_rta*rt1_u1;
            uq_t2 = uq_t2             + 0.5*uq_rta*rt2_t2;
            uq_d2 = uq_d2 + 0.5*uq_da;
            uq_u2 = uq_u2             + 0.5*uq_rta*rt2_u2;
            uq_ms = uq_ms             + 0.5*uq_rta*rt1_ms            + 0.5*uq_rta*rt2_ms;
            uq_re = uq_re             + 0.5*uq_rta*rt1_re            + 0.5*uq_rta*rt2_re;

            scc = sccon*1.333/(1.0+usa);
            scc_usa = -scc/(1.0+usa);

            //            scc_us1 = scc_usa*0.5;
            //            scc_us2 = scc_usa*0.5;

            slog = log(s2/s1);
            dxi = x2 - x1;

            rezc = scc*(cqa - sa*ald)*dxi- dea*2.0*  slog + dea*2.0*(uq*dxi - ulog);

            z_cfa = dea*2.0*uq_cfa*dxi;
            z_hka = dea*2.0*uq_hka*dxi;
            z_da  = dea*2.0*uq_da *dxi;
            z_sl = -dea*2.0;
            z_ul = -dea*2.0;
            z_dxi = scc    *(cqa - sa*ald)     + dea*2.0*uq;
            z_usa = scc_usa*(cqa - sa*ald)*dxi;
            z_cqa = scc*dxi;
            z_sa = -scc*dxi*ald;
            z_dea = 2.0*(uq*dxi - ulog - slog);

            z_upw = z_cqa*(cq2-cq1) + z_sa *(s2 -s1 )+ z_cfa*(cf2-cf1) + z_hka*(hk2-hk1);
            z_de1 = 0.5*z_dea;
            z_de2 = 0.5*z_dea;
            z_us1 = 0.5*z_usa;
            z_us2 = 0.5*z_usa;
            z_d1  = 0.5*z_da;
            z_d2  = 0.5*z_da;
            z_u1  =                 - z_ul/u1;
            z_u2  =                   z_ul/u2;
            z_x1  = -z_dxi;
            z_x2  =  z_dxi;
            z_s1  = (1.0-upw)*z_sa  - z_sl/s1;
            z_s2  =       upw *z_sa  + z_sl/s2;
            z_cq1 = (1.0-upw)*z_cqa;
            z_cq2 =       upw *z_cqa;
            z_cf1 = (1.0-upw)*z_cfa;
            z_cf2 =       upw *z_cfa;
            z_hk1 = (1.0-upw)*z_hka;
            z_hk2 =       upw *z_hka;

            vs1[1][1] = z_s1;
            vs1[1][2] =        z_upw*upw_t1 + z_de1*de1_t1 + z_us1*us1_t1;
            vs1[1][3] = z_d1 + z_upw*upw_d1 + z_de1*de1_d1 + z_us1*us1_d1;
            vs1[1][4] = z_u1 + z_upw*upw_u1 + z_de1*de1_u1 + z_us1*us1_u1;
            vs1[1][5] = z_x1;
            vs2[1][1] = z_s2;
            vs2[1][2] =        z_upw*upw_t2 + z_de2*de2_t2 + z_us2*us2_t2;
            vs2[1][3] = z_d2 + z_upw*upw_d2 + z_de2*de2_d2 + z_us2*us2_d2;
            vs2[1][4] = z_u2 + z_upw*upw_u2 + z_de2*de2_u2 + z_us2*us2_u2;
            vs2[1][5] = z_x2;
            vsm[1]   =        z_upw*upw_ms + z_de1*de1_ms + z_us1*us1_ms+ z_de2*de2_ms + z_us2*us2_ms;

            vs1[1][2] = vs1[1][2] + z_cq1*cq1_t1 + z_cf1*cf1_t1 + z_hk1*hk1_t1;
            vs1[1][3] = vs1[1][3] + z_cq1*cq1_d1 + z_cf1*cf1_d1 + z_hk1*hk1_d1;
            vs1[1][4] = vs1[1][4] + z_cq1*cq1_u1 + z_cf1*cf1_u1 + z_hk1*hk1_u1;

            vs2[1][2] = vs2[1][2] + z_cq2*cq2_t2 + z_cf2*cf2_t2 + z_hk2*hk2_t2;
            vs2[1][3] = vs2[1][3] + z_cq2*cq2_d2 + z_cf2*cf2_d2 + z_hk2*hk2_d2;
            vs2[1][4] = vs2[1][4] + z_cq2*cq2_u2 + z_cf2*cf2_u2 + z_hk2*hk2_u2;

            vsm[1]   = vsm[1]   + z_cq1*cq1_ms + z_cf1*cf1_ms + z_hk1*hk1_ms + z_cq2*cq2_ms + z_cf2*cf2_ms + z_hk2*hk2_ms;
            vsr[1]   =            z_cq1*cq1_re + z_cf1*cf1_re                + z_cq2*cq2_re + z_cf2*cf2_re;
            vsx[1]   = 0.0;
            vsrez[1] = -rezc;
        }
    }//endif

    //**** set up momentum equation
    ha = 0.5*(h1 + h2);
    ma = 0.5*(m1 + m2);
    xa = 0.5*(x1 + x2);
    ta = 0.5*(theta1 + theta2);
    hwa = 0.5*(dw1/theta1 + dw2/theta2);

    //---- set cf term, using central value cfm for better accuracy in drag
    cfx     = 0.50*cfm*xa/ta  +  0.25*(cf1*x1/theta1 + cf2*x2/theta2);
    cfx_xa  = 0.50*cfm   /ta;
    cfx_ta  = -.50*cfm*xa/ta/ta;

    cfx_x1  = 0.25*cf1   /theta1     + cfx_xa*0.5;
    cfx_x2  = 0.25*cf2   /theta2     + cfx_xa*0.5;
    cfx_t1  = -.25*cf1*x1/theta1/theta1  + cfx_ta*0.5;
    cfx_t2  = -.25*cf2*x2/theta2/theta2  + cfx_ta*0.5;
    cfx_cf1 = 0.25*    x1/theta1;
    cfx_cf2 = 0.25*    x2/theta2;
    cfx_cfm = 0.50*    xa/ta;

    btmp = ha + 2.0 - ma + hwa;

    rezt  = tlog + btmp*ulog - xlog*0.5*cfx;
    z_cfx = -xlog*0.5;
    z_ha  =  ulog;
    z_hwa =  ulog;
    z_ma  = -ulog;
    z_xl  =-ddlog * 0.5*cfx;
    z_ul  = ddlog * btmp;
    z_tl  = ddlog;

    z_cfm = z_cfx*cfx_cfm;
    z_cf1 = z_cfx*cfx_cf1;
    z_cf2 = z_cfx*cfx_cf2;

    z_t1 = -z_tl/theta1 + z_cfx*cfx_t1 + z_hwa*0.5*(-dw1/theta1/theta1);
    z_t2 =  z_tl/theta2 + z_cfx*cfx_t2 + z_hwa*0.5*(-dw2/theta2/theta2);
    z_x1 = -z_xl/x1 + z_cfx*cfx_x1;
    z_x2 =  z_xl/x2 + z_cfx*cfx_x2;
    z_u1 = -z_ul/u1;
    z_u2 =  z_ul/u2;

    vs1[2][2] = 0.5*z_ha*h1_t1 + z_cfm*cfm_t1 + z_cf1*cf1_t1 + z_t1;
    vs1[2][3] = 0.5*z_ha*h1_d1 + z_cfm*cfm_d1 + z_cf1*cf1_d1;
    vs1[2][4] = 0.5*z_ma*m1_u1 + z_cfm*cfm_u1 + z_cf1*cf1_u1 + z_u1;
    vs1[2][5] =                                                z_x1;
    vs2[2][2] = 0.5*z_ha*h2_t2 + z_cfm*cfm_t2 + z_cf2*cf2_t2 + z_t2;
    vs2[2][3] = 0.5*z_ha*h2_d2 + z_cfm*cfm_d2 + z_cf2*cf2_d2;
    vs2[2][4] = 0.5*z_ma*m2_u2 + z_cfm*cfm_u2 + z_cf2*cf2_u2 + z_u2;
    vs2[2][5] =                                                z_x2;

    vsm[2]   = 0.5*z_ma*m1_ms + z_cfm*cfm_ms + z_cf1*cf1_ms
            + 0.5*z_ma*m2_ms + z_cf2*cf2_ms;
    vsr[2]   =                   z_cfm*cfm_re + z_cf1*cf1_re
            + z_cf2*cf2_re;
    vsx[2]   = 0.0;
    vsrez[2] = -rezt;

    //**** set up shape parameter equation

    xot1 = x1/theta1;
    xot2 = x2/theta2;

    ha  = 0.5*(h1  + h2 );
    hsa = 0.5*(hs1 + hs2);
    hca = 0.5*(hc1 + hc2);
    hwa = 0.5*(dw1/theta1 + dw2/theta2);

    dix = (1.0-upw)*di1*xot1 + upw*di2*xot2;
    cfx = (1.0-upw)*cf1*xot1 + upw*cf2*xot2;
    dix_upw = di2*xot2 - di1*xot1;
    cfx_upw = cf2*xot2 - cf1*xot1;

    btmp = 2.0*hca/hsa + 1.0 - ha - hwa;

    rezh  = hlog + btmp*ulog + xlog*(0.5*cfx-dix);
    z_cfx =  xlog*0.5;
    z_dix = -xlog;
    z_hca = 2.0*ulog/hsa;
    z_ha  = -ulog;
    z_hwa = -ulog;
    z_xl  = ddlog * (0.5*cfx-dix);
    z_ul  = ddlog * btmp;
    z_hl  = ddlog;

    z_upw = z_cfx*cfx_upw + z_dix*dix_upw;

    z_hs1 = -hca*ulog/hsa/hsa - z_hl/hs1;
    z_hs2 = -hca*ulog/hsa/hsa + z_hl/hs2;

    z_cf1 = (1.0-upw)*z_cfx*xot1;
    z_cf2 =      upw *z_cfx*xot2;
    z_di1 = (1.0-upw)*z_dix*xot1;
    z_di2 =      upw *z_dix*xot2;

    z_t1 = (1.0-upw)*(z_cfx*cf1 + z_dix*di1)*(-xot1/theta1);
    z_t2 =      upw *(z_cfx*cf2 + z_dix*di2)*(-xot2/theta2);
    z_x1 = (1.0-upw)*(z_cfx*cf1 + z_dix*di1)/ theta1        - z_xl/x1;
    z_x2 =      upw *(z_cfx*cf2 + z_dix*di2)/ theta2        + z_xl/x2;
    z_u1 =                                              - z_ul/u1;
    z_u2 =                                                z_ul/u2;

    z_t1 = z_t1 + z_hwa*0.5*(-dw1/theta1/theta1);
    z_t2 = z_t2 + z_hwa*0.5*(-dw2/theta2/theta2);

    vs1[3][1] =                               z_di1*di1_s1;
    vs1[3][2] = z_hs1*hs1_t1 + z_cf1*cf1_t1 + z_di1*di1_t1 + z_t1;
    vs1[3][3] = z_hs1*hs1_d1 + z_cf1*cf1_d1 + z_di1*di1_d1;
    vs1[3][4] = z_hs1*hs1_u1 + z_cf1*cf1_u1 + z_di1*di1_u1 + z_u1;
    vs1[3][5] =                                              z_x1;
    vs2[3][1] =                               z_di2*di2_s2;
    vs2[3][2] = z_hs2*hs2_t2 + z_cf2*cf2_t2 + z_di2*di2_t2 + z_t2;
    vs2[3][3] = z_hs2*hs2_d2 + z_cf2*cf2_d2 + z_di2*di2_d2;
    vs2[3][4] = z_hs2*hs2_u2 + z_cf2*cf2_u2 + z_di2*di2_u2 + z_u2;
    vs2[3][5] =                                              z_x2;
    vsm[3]   =   z_hs1*hs1_ms + z_cf1*cf1_ms + z_di1*di1_ms
               + z_hs2*hs2_ms + z_cf2*cf2_ms + z_di2*di2_ms;
    vsr[3]   =   z_hs1*hs1_re + z_cf1*cf1_re + z_di1*di1_re
               + z_hs2*hs2_re + z_cf2*cf2_re + z_di2*di2_re;

    vs1[3][2] = vs1[3][2] + 0.5*(z_hca*hc1_t1+z_ha*h1_t1) + z_upw*upw_t1;
    vs1[3][3] = vs1[3][3] + 0.5*(z_hca*hc1_d1+z_ha*h1_d1) + z_upw*upw_d1;
    vs1[3][4] = vs1[3][4] + 0.5*(z_hca*hc1_u1           ) + z_upw*upw_u1;
    vs2[3][2] = vs2[3][2] + 0.5*(z_hca*hc2_t2+z_ha*h2_t2) + z_upw*upw_t2;
    vs2[3][3] = vs2[3][3] + 0.5*(z_hca*hc2_d2+z_ha*h2_d2) + z_upw*upw_d2;
    vs2[3][4] = vs2[3][4] + 0.5*(z_hca*hc2_u2           ) + z_upw*upw_u2;

    vsm[3]   = vsm[3]   + 0.5*(z_hca*hc1_ms) + z_upw*upw_ms
                        + 0.5*(z_hca*hc2_ms);

    vsx[3]   = 0.0;
    vsrez[3] = -rezh;

    return true;
}


bool XFoil::blkin()
{
    //----------------------------------------------------------
    //     calculates turbulence-independent secondary "2"
    //     variables from the primary "2" variables.
    //----------------------------------------------------------
    double tr2(0), herat(0), he_u2(0), he_ms(0), v2_he(0), hk2_h2(0), hk2_m2(0);

    //---- set edge mach number ** 2
    m2    = u2*u2*hstinv / (gm1bl*(1.0 - 0.5*u2*u2*hstinv));
    tr2   = 1.0 + 0.5*gm1bl*m2;
    m2_u2 = 2.0*m2*tr2/u2;
    m2_ms = u2*u2*tr2 / (gm1bl*(1.0 - 0.5*u2*u2*hstinv))* hstinv_ms;

    //---- set edge density (isentropic relation)
    r2    = rstbl   *pow(tr2,(-1.0/gm1bl));
    r2_u2 = -r2/tr2 * 0.5*m2_u2;
    r2_ms = -r2/tr2 * 0.5*m2_ms+ rstbl_ms*pow(tr2,(-1.0/gm1bl));

    //---- set shape parameter
    h2    =  d2/theta2;
    h2_d2 = 1.0/theta2;
    h2_t2 = -h2/theta2;

    //---- set edge static/stagnation enthalpy
    herat = 1.0 - 0.5*u2*u2*hstinv;
    he_u2 =     -        u2*hstinv;
    he_ms =     - 0.5*u2*u2*hstinv_ms;
    //---- set molecular viscosity
    v2 = sqrt(herat*herat*herat) * (1.0+hvrat)/(herat+hvrat)/reybl;
    v2_he = v2*(1.5/herat - 1.0/(herat+hvrat));

    v2_u2 =                        v2_he*he_u2;
    v2_ms = -v2/reybl * reybl_ms + v2_he*he_ms;
    v2_re = -v2/reybl * reybl_re;

    //---- set kinematic shape parameter
    hkin(h2, m2, hk2, hk2_h2, hk2_m2 );

    hk2_u2 =                hk2_m2*m2_u2;
    hk2_t2 = hk2_h2*h2_t2;
    hk2_d2 = hk2_h2*h2_d2;
    hk2_ms =                hk2_m2*m2_ms;

    //---- set momentum thickness reynolds number
    rt2    = r2*u2*theta2/v2;
    rt2_u2 = rt2*(1.0/u2 + r2_u2/r2 - v2_u2/v2);
    rt2_t2 = rt2/theta2;
    rt2_ms = rt2*(         r2_ms/r2 - v2_ms/v2);
    rt2_re = rt2*(                  - v2_re/v2);

    return true;
}


/** ----------------------------------------------------
 *     calculates midpoint skin friction cfm
 *
 *      ityp = 1 :  laminar
 *      ityp = 2 :  turbulent
 *      ityp = 3 :  turbulent wake
 * -----------------------------------------------------*/
bool XFoil::blmid(int ityp)
{
    double hka(0), rta(0), ma(0), cfm_rta(0), cfm_ma(0);
    double cfml(0), cfml_hka(0), cfml_rta(0), cfml_ma(0), cfm_hka(0);

    //---- set similarity variables if not defined
    if(simi)
    {
        hk1    = hk2;
        hk1_t1 = hk2_t2;
        hk1_d1 = hk2_d2;
        hk1_u1 = hk2_u2;
        hk1_ms = hk2_ms;
        rt1    = rt2;
        rt1_t1 = rt2_t2;
        rt1_u1 = rt2_u2;
        rt1_ms = rt2_ms;
        rt1_re = rt2_re;
        m1    = m2;
        m1_u1 = m2_u2;
        m1_ms = m2_ms;
    }

    //---- define stuff for midpoint cf
    hka = 0.5*(hk1 + hk2);
    rta = 0.5*(rt1 + rt2);
    ma  = 0.5*(m1  + m2 );

    //---- midpoint skin friction coefficient  (zero in wake)
    if(ityp==3)
    {
        cfm     = 0.0;
        cfm_hka = 0.0;
        cfm_rta = 0.0;
        cfm_ma  = 0.0;
        cfm_ms  = 0.0;
    }
    else
    {
        if(ityp==1) cfl(hka, rta, cfm, cfm_hka, cfm_rta, cfm_ma );
        else {
            cft(hka, rta, ma, cfm, cfm_hka, cfm_rta, cfm_ma );
            cfl(hka, rta, cfml, cfml_hka, cfml_rta, cfml_ma);
            if(cfml>cfm) {
                //ccc      write(*,*) 'cft cfl rt hk:', cfm, cfml, rta, hka, 0.5*(x1+x2)
                cfm     = cfml;
                cfm_hka = cfml_hka;
                cfm_rta = cfml_rta;
                cfm_ma  = cfml_ma;
            }
        }
    }
    cfm_u1 = 0.5*(cfm_hka*hk1_u1 + cfm_ma*m1_u1 + cfm_rta*rt1_u1);
    cfm_t1 = 0.5*(cfm_hka*hk1_t1 +                cfm_rta*rt1_t1);
    cfm_d1 = 0.5*(cfm_hka*hk1_d1                                );

    cfm_u2 = 0.5*(cfm_hka*hk2_u2 + cfm_ma*m2_u2 + cfm_rta*rt2_u2);
    cfm_t2 = 0.5*(cfm_hka*hk2_t2 +                cfm_rta*rt2_t2);
    cfm_d2 = 0.5*(cfm_hka*hk2_d2                                );

    cfm_ms = 0.5*(cfm_hka*hk1_ms + cfm_ma*m1_ms + cfm_rta*rt1_ms
                  + cfm_hka*hk2_ms + cfm_ma*m2_ms + cfm_rta*rt2_ms);
    cfm_re = 0.5*(                                cfm_rta*rt1_re
                                                  + cfm_rta*rt2_re);

    return true;
}


/** ----------------------------------------------------------
 *     set bl primary "2" variables from parameter list
 *  ---------------------------------------------------------- */
bool XFoil::blprv(double xsi, double ami, double cti, double thi,
                  double dsi, double dswaki, double uei)
{
    x2 = xsi;
    ampl2 = ami;
    s2  = cti;
    theta2  = thi;
    d2  = dsi - dswaki;
    dw2 = dswaki;

    u2 = uei*(1.0-tkbl) / (1.0 - tkbl*(uei/qinfbl)*(uei/qinfbl));
    u2_uei = (1.0 + tkbl*(2.0*u2*uei/qinfbl/qinfbl - 1.0))
            / (1.0 - tkbl*(uei/qinfbl)*(uei/qinfbl));
    u2_ms  = (u2*(uei/qinfbl)*(uei/qinfbl)  -  uei)*tkbl_ms
            / (1.0 - tkbl*(uei/qinfbl)*(uei/qinfbl));
    return true;

}



/** -----------------------------------------------------------------
 *      custom solver for coupled viscous-inviscid newton system:
 *
 *        a  |  |  .  |  |  .  |    d       r       s
 *        b  a  |  .  |  |  .  |    d       r       s
 *        |  b  a  .  |  |  .  |    d       r       s
 *        .  .  .  .  |  |  .  |    d   =   r - dre s
 *        |  |  |  b  a  |  .  |    d       r       s
 *        |  z  |  |  b  a  .  |    d       r       s
 *        .  .  .  .  .  .  .  |    d       r       s
 *        |  |  |  |  |  |  b  a    d       r       s
 *
 *       a, b, z  3x3  blocks containing linearized bl equation coefficients
 *       |        3x1  vectors containing mass defect influence
 *                     coefficients on ue
 *       d        3x1  unknown vectors (newton deltas for ctau][ theta][ m)
 *       r        3x1  residual vectors
 *       s        3x1  re influence vectors
 * ------------------------------------------------------------------ */
bool XFoil::blsolve()
{
    int iv(0), kv(0), ivp(0), k(0), l(0), ivte1(0), ivz(0);
    double pivot(0), vtmp(0), vtmp1(0), vtmp2(0), vtmp3(0);

    ivte1 = isys[iblte[1]][1];
    //
    for (iv=1; iv<=nsys; iv++)
    {
        //
        ivp = iv + 1;
        //
        //====== invert va[iv] block
        //
        //------ normalize first row
        pivot = 1.0 / va[1][1][iv];
        va[1][2][iv] *= pivot;
        for (l=iv;l<= nsys;l++) vm[1][l][iv] *= pivot;
        vdel[1][1][iv] *= pivot;
        vdel[1][2][iv] *= pivot;
        //
        //------ eliminate lower first column in va block
        for (k=2; k<=3; k++)
        {
            vtmp = va[k][1][iv];
            va[k][2][iv] -= vtmp*va[1][2][iv];
            for (int l=iv; l<=nsys; l++) vm[k][l][iv] -= vtmp*vm[1][l][iv];
            vdel[k][1][iv] -= vtmp*vdel[1][1][iv];
            vdel[k][2][iv] -= vtmp*vdel[1][2][iv];
        }
        //
        //------ normalize second row
        pivot = 1.0 / va[2][2][iv];
        for (l=iv; l<= nsys; l++) vm[2][l][iv] *=pivot;
        vdel[2][1][iv] *= pivot;
        vdel[2][2][iv] *= pivot;
        //
        //------ eliminate lower second column in va block
        k = 3;
        vtmp = va[k][2][iv];
        for (l=iv; l<=nsys; l++) vm[k][l][iv] -= vtmp*vm[2][l][iv];
        vdel[k][1][iv] -= vtmp*vdel[2][1][iv];
        vdel[k][2][iv] -= vtmp*vdel[2][2][iv];

        //------ normalize third row
        pivot = 1.0/vm[3][iv][iv];
        for (l=ivp; l<=nsys; l++) vm[3][l][iv] *= pivot;
        vdel[3][1][iv] *= pivot;
        vdel[3][2][iv] *= pivot;
        //
        //
        //------ eliminate upper third column in va block
        vtmp1 = vm[1][iv][iv];
        vtmp2 = vm[2][iv][iv];
        for(l=ivp;l<= nsys;l++)
        {
            vm[1][l][iv] -= vtmp1*vm[3][l][iv];
            vm[2][l][iv] -= vtmp2*vm[3][l][iv];
        }
        vdel[1][1][iv] -= vtmp1*vdel[3][1][iv];
        vdel[2][1][iv] -= vtmp2*vdel[3][1][iv];
        vdel[1][2][iv] -= vtmp1*vdel[3][2][iv];
        vdel[2][2][iv] -= vtmp2*vdel[3][2][iv];

        //------ eliminate upper second column in va block
        vtmp = va[1][2][iv];
        for (l=ivp; l<=nsys;l++) vm[1][l][iv] -= vtmp*vm[2][l][iv];

        vdel[1][1][iv] -= vtmp*vdel[2][1][iv];
        vdel[1][2][iv] -= vtmp*vdel[2][2][iv];


        if(iv!=nsys)
        {
            //====== eliminate vb(iv+1) block][ rows  1 -> 3
            for (k=1; k<= 3;k++)
            {
                vtmp1 = vb[k][ 1][ivp];
                vtmp2 = vb[k][ 2][ivp];
                vtmp3 = vm[k][iv][ivp];
                for(l=ivp; l<= nsys;l++) vm[k][l][ivp] -= (vtmp1*vm[1][l][iv]+ vtmp2*vm[2][l][iv]+vtmp3*vm[3][l][iv]);
                vdel[k][1][ivp] -= (vtmp1*vdel[1][1][iv]+vtmp2*vdel[2][1][iv]+ vtmp3*vdel[3][1][iv]);
                vdel[k][2][ivp] -= (vtmp1*vdel[1][2][iv]+vtmp2*vdel[2][2][iv]+ vtmp3*vdel[3][2][iv]);
            }

            if(iv==ivte1)
            {
                //------- eliminate vz block
                ivz = isys[iblte[2]+1][2];
                //
                for(k=1;k<=3;k++)
                {
                    vtmp1 = vz[k][1];
                    vtmp2 = vz[k][2];
                    for (l=ivp;l<= nsys;l++)
                    {
                        vm[k][l][ivz] -=(vtmp1*vm[1][l][iv]+ vtmp2*vm[2][l][iv]);
                    }
                    vdel[k][1][ivz] -= (vtmp1*vdel[1][1][iv]+ vtmp2*vdel[2][1][iv]);
                    vdel[k][2][ivz] -= (vtmp1*vdel[1][2][iv]+ vtmp2*vdel[2][2][iv]);
                }
            }

            if(ivp!=nsys)
            {
                //====== eliminate lower vm column
                for(kv=iv+2; kv<= nsys;kv++)
                {
                    vtmp1 = vm[1][iv][kv];
                    vtmp2 = vm[2][iv][kv];
                    vtmp3 = vm[3][iv][kv];

                    if(fabs(vtmp1)>vaccel)
                    {
                        for(l=ivp;l<= nsys;l++) vm[1][l][kv] -= vtmp1*vm[3][l][iv];
                        vdel[1][1][kv] -= vtmp1*vdel[3][1][iv];
                        vdel[1][2][kv] -= vtmp1*vdel[3][2][iv];
                    }

                    if(fabs(vtmp2)>vaccel)
                    {
                        for (l=ivp;l<=nsys;l++) vm[2][l][kv] -= vtmp2*vm[3][l][iv];
                        vdel[2][1][kv] -= vtmp2*vdel[3][1][iv];
                        vdel[2][2][kv] -= vtmp2*vdel[3][2][iv];
                    }

                    if(fabs(vtmp3)>vaccel)
                    {
                        for(l=ivp;l<=nsys;l++) vm[3][l][kv] -= vtmp3*vm[3][l][iv];
                        vdel[3][1][kv] -= vtmp3*vdel[3][1][iv];
                        vdel[3][2][kv] -= vtmp3*vdel[3][2][iv];
                    }

                }
            }
        }
    }//1000


    for (iv=nsys; iv>=2; iv--)
    {
        //------ eliminate upper vm columns
        vtmp = vdel[3][1][iv];
        for (kv=iv-1; kv>=1;kv--)
        {
            vdel[1][1][kv] -= vm[1][iv][kv]*vtmp;
            vdel[2][1][kv] -= vm[2][iv][kv]*vtmp;
            vdel[3][1][kv] -= vm[3][iv][kv]*vtmp;
        }
        vtmp = vdel[3][2][iv];
        for (kv=iv-1; kv>=1;kv--)
        {
            vdel[1][2][kv] -= vm[1][iv][kv]*vtmp;
            vdel[2][2][kv] -= vm[2][iv][kv]*vtmp;
            vdel[3][2][kv] -= vm[3][iv][kv]*vtmp;
        }
    }
    return true;
}


/** ------------------------------------------------------------------
 *
 *      sets up the bl newton system governing the current interval:
 *
 *      |       ||da1|     |       ||da2|       |     |
 *      |  vs1  ||dt1|  +  |  vs2  ||dt2|   =   |vsrez|
 *      |       ||dd1|     |       ||dd2|       |     |
 *               |du1|              |du2|
 *               |dx1|              |dx2|
 *
 *         3x5    5x1         3x5    5x1          3x1
 *
 *      the system as shown corresponds to a laminar station
 *      if tran, then  ds2  replaces  da2
 *      if turb, then  ds1, ds2  replace  da1, da2
 *
 * ------------------------------------------------------------------ */
bool XFoil::blsys()
{
    double res_u1(0), res_u2(0), res_ms(0);
    int k(0), l(0);

    //---- calculate secondary bl variables and their sensitivities
    if(wake)
    {
        blvar(3);
        blmid(3);
    }
    else
    {
        if(turb || tran)
        {
            blvar(2);
            blmid(2);
        }
        else
        {
            blvar(1);
            blmid(1);
        }
    }

    //---- for the similarity station, "1" and "2" variables are the same
    if(simi) {
        //        for(int icom=1;icom<= ncom;icom++) com1[icom] = com2[icom];
        stepbl();

    }

    //---- set up appropriate finite difference system for current interval
    if(tran)
        trdif();
    else if(simi)
        bldif(0);
    else if(!turb)
        bldif(1);
    else if(wake)
        bldif(3);
    else if(turb)
        bldif(2);


    if(simi)
    {
        //----- at similarity station, "1" variables are really "2" variables
        for (k=1; k<= 4;k++)
        {
            for(l=1; l<= 5; l++){
                vs2[k][l] = vs1[k][l] + vs2[k][l];
                vs1[k][l] = 0.0;
            }
        }
    }

    //---- change system over into incompressible uei and mach
    for(k=1;k<= 4;k++)
    {
        //------ residual derivatives wrt compressible uec
        res_u1 = vs1[k][4];
        res_u2 = vs2[k][4];
        res_ms = vsm[k];

        //------ combine with derivatives of compressible  u1,u2 = uec(uei m)
        vs1[k][4] = res_u1*u1_uei;
        vs2[k][4] =                res_u2*u2_uei;
        vsm[k]   = res_u1*u1_ms + res_u2*u2_ms  + res_ms;
    }
    return true;
}




/** ----------------------------------------------------
 *      calculates all secondary "2" variables from
 *      the primary "2" variables x2, u2, t2, d2, s2.
 *      also calculates the sensitivities of the
 *      secondary variables wrt the primary variables.
 *
 *       ityp = 1 :  laminar
 *       ityp = 2 :  turbulent
 *       ityp = 3 :  turbulent wake
 * ---------------------------------------------------- */
bool XFoil::blvar(int ityp)
{
    double hs2_hk2(0), hs2_rt2(0), hs2_m2(0);
    double hc2_hk2(0), hc2_m2(0), us2_hs2(0), us2_hk2(0), us2_h2(0);
    double gcc(0), hkc(0), hkc_hk2(0), hkc_rt2(0), hkb(0), usb(0);
    double cq2_hs2(0), cq2_us2(0), cq2_hk2(0);
    double cq2_rt2(0), cq2_h2(0), cf2_hk2(0), cf2_rt2(0), cf2_m2(0);
    double cf2l(0), cf2l_hk2(0), cf2l_rt2(0), cf2l_m2(0);
    double di2_hk2(0), di2_rt2(0), cf2t(0), cf2t_hk2(0);
    double cf2t_rt2(0), cf2t_m2(0), cf2t_u2(0), cf2t_t2(0);
    double cf2t_d2(0), cf2t_ms(0), cf2t_re(0), di2_hs2(0);
    double di2_us2(0), di2_cf2t(0), hmin(0), hm_rt2(0);
    double grt(0), fl(0), fl_hk2(0), fl_rt2(0), tfl(0);
    double dfac(0), df_fl(0), df_hk2(0), df_rt2(0), dd(0), dd_hs2(0);
    double dd_us2(0), dd_s2(0), dd_rt2(0), di2l(0), di2l_hk2(0), di2l_rt2(0), de2_hk2(0), hdmax(0);


    //    double gbcon, gccon, ctcon, hkc2;//were are they initialized?
    if(ityp==3) hk2 = std::max(hk2,1.00005);
    if(ityp!=3) hk2 = std::max(hk2,1.05000);

    //---- density thickness shape parameter     ( h** )
    hct( hk2, m2, hc2, hc2_hk2, hc2_m2 );
    hc2_u2 = hc2_hk2*hk2_u2 + hc2_m2*m2_u2;
    hc2_t2 = hc2_hk2*hk2_t2;
    hc2_d2 = hc2_hk2*hk2_d2;
    hc2_ms = hc2_hk2*hk2_ms + hc2_m2*m2_ms;

    //---- set ke thickness shape parameter from  h - h*  correlations
    if(ityp==1) hsl(hk2, hs2, hs2_hk2, hs2_rt2, hs2_m2 );
    else hst(hk2, rt2, m2, hs2, hs2_hk2, hs2_rt2, hs2_m2 );


    hs2_u2 = hs2_hk2*hk2_u2 + hs2_rt2*rt2_u2 + hs2_m2*m2_u2;
    hs2_t2 = hs2_hk2*hk2_t2 + hs2_rt2*rt2_t2;
    hs2_d2 = hs2_hk2*hk2_d2;
    hs2_ms = hs2_hk2*hk2_ms + hs2_rt2*rt2_ms + hs2_m2*m2_ms;
    hs2_re =                  hs2_rt2*rt2_re;

    //---- normalized slip velocity  us
    us2     = 0.5*hs2*( 1.0 - (hk2-1.0)/(gbcon*h2) );
    us2_hs2 = 0.5  *  ( 1.0 - (hk2-1.0)/(gbcon*h2) );
    us2_hk2 = 0.5*hs2*(      -  1.0     /(gbcon*h2) );
    us2_h2  = 0.5*hs2*         (hk2-1.0)/(gbcon*h2*h2);

    us2_u2 = us2_hs2*hs2_u2 + us2_hk2*hk2_u2;
    us2_t2 = us2_hs2*hs2_t2 + us2_hk2*hk2_t2 + us2_h2*h2_t2;
    us2_d2 = us2_hs2*hs2_d2 + us2_hk2*hk2_d2 + us2_h2*h2_d2;
    us2_ms = us2_hs2*hs2_ms + us2_hk2*hk2_ms;
    us2_re = us2_hs2*hs2_re;

    if(ityp<=2 && us2>0.95)
    {
        //       write(*,*) 'blvar: us clamped:', us2
        us2 = 0.98;
        us2_u2 = 0.0;
        us2_t2 = 0.0;
        us2_d2 = 0.0;
        us2_ms = 0.0;
        us2_re = 0.0;
    }

    if(ityp==3 && us2>0.99995)
    {
        //       write(*,*) 'blvar: wake us clamped:', us2
        us2 = 0.99995;
        us2_u2 = 0.0;
        us2_t2 = 0.0;
        us2_d2 = 0.0;
        us2_ms = 0.0;
        us2_re = 0.0;
    }

    //---- equilibrium wake layer shear coefficient (ctau)eq ** 1/2
    //   ...  new  12 oct 94
    gcc = 0.0;
    hkc = hk2 - 1.0;
    hkc_hk2 = 1.0;
    hkc_rt2 = 0.0;
    if(ityp==2)
    {
        gcc = gccon;
        hkc     = hk2 - 1.0 - gcc/rt2;
        hkc_hk2 = 1.0;
        hkc_rt2 =             gcc/rt2/rt2;
        if(hkc < 0.01)
        {
            hkc = 0.01;
            hkc_hk2 = 0.0;
            hkc_rt2 = 0.0;
        }
    }

    hkb = hk2 - 1.0;
    usb = 1.0 - us2;
    cq2     =
            sqrt( ctcon*hs2*hkb*hkc*hkc / (usb*h2*hk2*hk2) );
    cq2_hs2 = ctcon    *hkb*hkc*hkc / (usb*h2*hk2*hk2)       * 0.5/cq2;
    cq2_us2 = ctcon*hs2*hkb*hkc*hkc / (usb*h2*hk2*hk2) / usb * 0.5/cq2;
    cq2_hk2 = ctcon*hs2    *hkc*hkc / (usb*h2*hk2*hk2)       * 0.5/cq2
            - ctcon*hs2*hkb*hkc*hkc / (usb*h2*hk2*hk2*hk2) * 2.0 * 0.5/cq2
            + ctcon*hs2*hkb*hkc     / (usb*h2*hk2*hk2) * 2.0 * 0.5/cq2
            *hkc_hk2;
    cq2_rt2 = ctcon*hs2*hkb*hkc    / (usb*h2*hk2*hk2) * 2.0 * 0.5/cq2
            *hkc_rt2;
    cq2_h2  =-ctcon*hs2*hkb*hkc*hkc / (usb*h2*hk2*hk2) / h2  * 0.5/cq2;

    cq2_u2 = cq2_hs2*hs2_u2 + cq2_us2*us2_u2 + cq2_hk2*hk2_u2;
    cq2_t2 = cq2_hs2*hs2_t2 + cq2_us2*us2_t2 + cq2_hk2*hk2_t2;
    cq2_d2 = cq2_hs2*hs2_d2 + cq2_us2*us2_d2 + cq2_hk2*hk2_d2;
    cq2_ms = cq2_hs2*hs2_ms + cq2_us2*us2_ms + cq2_hk2*hk2_ms;
    cq2_re = cq2_hs2*hs2_re + cq2_us2*us2_re;

    cq2_u2 = cq2_u2                + cq2_rt2*rt2_u2;
    cq2_t2 = cq2_t2 + cq2_h2*h2_t2 + cq2_rt2*rt2_t2;
    cq2_d2 = cq2_d2 + cq2_h2*h2_d2;
    cq2_ms = cq2_ms                + cq2_rt2*rt2_ms;
    cq2_re = cq2_re                + cq2_rt2*rt2_re;

    //---- set skin friction coefficient
    if(ityp==3)
    {
        //----- wake
        cf2     = 0.0;
        cf2_hk2 = 0.0;
        cf2_rt2 = 0.0;
        cf2_m2  = 0.0;
    }
    else
    {
        if(ityp==1)
            //----- laminar
            cfl(hk2, rt2, cf2, cf2_hk2, cf2_rt2, cf2_m2);
        else
        {
            //----- turbulent
            cft(hk2, rt2, m2, cf2, cf2_hk2, cf2_rt2, cf2_m2);
            cfl(hk2, rt2, cf2l, cf2l_hk2, cf2l_rt2, cf2l_m2);
            if(cf2l>cf2)
            {
                //------- laminar cf is greater than turbulent cf -- use laminar
                //-       (this will only occur for unreasonably small rtheta)
                cf2     = cf2l;
                cf2_hk2 = cf2l_hk2;
                cf2_rt2 = cf2l_rt2;
                cf2_m2  = cf2l_m2;
            }
        }
    }

    cf2_u2 = cf2_hk2*hk2_u2 + cf2_rt2*rt2_u2 + cf2_m2*m2_u2;
    cf2_t2 = cf2_hk2*hk2_t2 + cf2_rt2*rt2_t2;
    cf2_d2 = cf2_hk2*hk2_d2;
    cf2_ms = cf2_hk2*hk2_ms + cf2_rt2*rt2_ms + cf2_m2*m2_ms;
    cf2_re =                  cf2_rt2*rt2_re;

    //---- dissipation function    2 cd / h*
    if(ityp==1)
    {

        //----- laminar
        dil( hk2, rt2, di2, di2_hk2, di2_rt2 );

        di2_u2 = di2_hk2*hk2_u2 + di2_rt2*rt2_u2;
        di2_t2 = di2_hk2*hk2_t2 + di2_rt2*rt2_t2;
        di2_d2 = di2_hk2*hk2_d2;
        di2_s2 = 0.0;
        di2_ms = di2_hk2*hk2_ms + di2_rt2*rt2_ms;
        di2_re =                  di2_rt2*rt2_re;
    }
    else
    {
        if(ityp==2)
        {
            //----- turbulent wall contribution
            cft(hk2, rt2, m2, cf2t, cf2t_hk2, cf2t_rt2, cf2t_m2);
            cf2t_u2 = cf2t_hk2*hk2_u2 + cf2t_rt2*rt2_u2 + cf2t_m2*m2_u2;
            cf2t_t2 = cf2t_hk2*hk2_t2 + cf2t_rt2*rt2_t2;
            cf2t_d2 = cf2t_hk2*hk2_d2;
            cf2t_ms = cf2t_hk2*hk2_ms + cf2t_rt2*rt2_ms + cf2t_m2*m2_ms;
            cf2t_re =                   cf2t_rt2*rt2_re;

            di2      =  ( 0.5*cf2t*us2 ) * 2.0/hs2;
            di2_hs2  = -( 0.5*cf2t*us2 ) * 2.0/hs2/hs2;
            di2_us2  =  ( 0.5*cf2t     ) * 2.0/hs2;
            di2_cf2t =  ( 0.5     *us2 ) * 2.0/hs2;

            di2_s2 = 0.0;
            di2_u2 = di2_hs2*hs2_u2 + di2_us2*us2_u2 + di2_cf2t*cf2t_u2;
            di2_t2 = di2_hs2*hs2_t2 + di2_us2*us2_t2 + di2_cf2t*cf2t_t2;
            di2_d2 = di2_hs2*hs2_d2 + di2_us2*us2_d2 + di2_cf2t*cf2t_d2;
            di2_ms = di2_hs2*hs2_ms + di2_us2*us2_ms + di2_cf2t*cf2t_ms;
            di2_re = di2_hs2*hs2_re + di2_us2*us2_re + di2_cf2t*cf2t_re;

            //----- set minimum hk for wake layer to still exist
            grt = log(rt2);
            hmin = 1.0 + 2.1/grt;
            hm_rt2 = -(2.1/grt/grt) / rt2;

            //----- set factor dfac for correcting wall dissipation for very low hk
            fl = (hk2-1.0)/(hmin-1.0);
            fl_hk2 =   1.0/(hmin-1.0);
            fl_rt2 = ( -fl/(hmin-1.0) ) * hm_rt2;

            tfl = tanh(fl);
            dfac  = 0.5 + 0.5* tfl;
            df_fl =       0.5*(1.0 - tfl*tfl);

            df_hk2 = df_fl*fl_hk2;
            df_rt2 = df_fl*fl_rt2;

            di2_s2 = di2_s2*dfac;
            di2_u2 = di2_u2*dfac + di2*(df_hk2*hk2_u2 + df_rt2*rt2_u2);
            di2_t2 = di2_t2*dfac + di2*(df_hk2*hk2_t2 + df_rt2*rt2_t2);
            di2_d2 = di2_d2*dfac + di2*(df_hk2*hk2_d2                );
            di2_ms = di2_ms*dfac + di2*(df_hk2*hk2_ms + df_rt2*rt2_ms);
            di2_re = di2_re*dfac + di2*(                df_rt2*rt2_re);
            di2    = di2   *dfac;
        }
        else
        {

            //----- zero wall contribution for wake
            di2    = 0.0;
            di2_s2 = 0.0;
            di2_u2 = 0.0;
            di2_t2 = 0.0;
            di2_d2 = 0.0;
            di2_ms = 0.0;
            di2_re = 0.0;

        }
    }
    //---- add on turbulent outer layer contribution
    if(ityp!=1)
    {

        dd     =  s2*s2 *  (0.995-us2) * 2.0/hs2;
        dd_hs2 = -s2*s2 *  (0.995-us2) * 2.0/hs2/hs2;
        dd_us2 = -s2*s2               * 2.0/hs2;
        dd_s2  =  s2*2.0* (0.995-us2) * 2.0/hs2;

        di2    = di2    + dd;
        di2_s2 =          dd_s2;
        di2_u2 = di2_u2 + dd_hs2*hs2_u2 + dd_us2*us2_u2;
        di2_t2 = di2_t2 + dd_hs2*hs2_t2 + dd_us2*us2_t2;
        di2_d2 = di2_d2 + dd_hs2*hs2_d2 + dd_us2*us2_d2;
        di2_ms = di2_ms + dd_hs2*hs2_ms + dd_us2*us2_ms;
        di2_re = di2_re + dd_hs2*hs2_re + dd_us2*us2_re;

        //----- add laminar stress contribution to outer layer cd
        dd     =  0.15*(0.995-us2)*(0.995-us2) / rt2  * 2.0/hs2;
        dd_us2 = -0.15*(0.995-us2)*2.0 / rt2  * 2.0/hs2;
        dd_hs2 = -dd/hs2;
        dd_rt2 = -dd/rt2;

        di2    = di2    + dd;
        di2_u2 = di2_u2 + dd_hs2*hs2_u2 + dd_us2*us2_u2 + dd_rt2*rt2_u2;
        di2_t2 = di2_t2 + dd_hs2*hs2_t2 + dd_us2*us2_t2 + dd_rt2*rt2_t2;
        di2_d2 = di2_d2 + dd_hs2*hs2_d2 + dd_us2*us2_d2;
        di2_ms = di2_ms + dd_hs2*hs2_ms + dd_us2*us2_ms + dd_rt2*rt2_ms;
        di2_re = di2_re + dd_hs2*hs2_re + dd_us2*us2_re + dd_rt2*rt2_re;

    }

    if(ityp==2)
    {
        dil( hk2, rt2, di2l, di2l_hk2, di2l_rt2 );

        if(di2l>di2)
        {
            //------- laminar cd is greater than turbulent cd -- use laminar
            //-       (this will only occur for unreasonably small rtheta)
            di2    = di2l;
            di2_s2 = 0.0;
            di2_u2 = di2l_hk2*hk2_u2 + di2l_rt2*rt2_u2;
            di2_t2 = di2l_hk2*hk2_t2 + di2l_rt2*rt2_t2;
            di2_d2 = di2l_hk2*hk2_d2;
            di2_ms = di2l_hk2*hk2_ms + di2l_rt2*rt2_ms;
            di2_re =                   di2l_rt2*rt2_re;
        }
    }

    if(ityp==3)
    {
        //------ laminar wake cd
        dilw( hk2, rt2, di2l, di2l_hk2, di2l_rt2 );
        if(di2l > di2)
        {
            //------- laminar wake cd is greater than turbulent cd -- use laminar
            //-       (this will only occur for unreasonably small rtheta)
            di2    = di2l;
            di2_s2 = 0.0;
            di2_u2 = di2l_hk2*hk2_u2 + di2l_rt2*rt2_u2;
            di2_t2 = di2l_hk2*hk2_t2 + di2l_rt2*rt2_t2;
            di2_d2 = di2l_hk2*hk2_d2;
            di2_ms = di2l_hk2*hk2_ms + di2l_rt2*rt2_ms;
            di2_re =                   di2l_rt2*rt2_re;
        }
    }

    if(ityp==3)
    {
        //----- double dissipation for the wake (two wake halves)
        di2    = di2   *2.0;
        di2_s2 = di2_s2*2.0;
        di2_u2 = di2_u2*2.0;
        di2_t2 = di2_t2*2.0;
        di2_d2 = di2_d2*2.0;
        di2_ms = di2_ms*2.0;
        di2_re = di2_re*2.0;
    }

    //---- bl thickness (delta) from simplified green's correlation
    de2     = (3.15 + 1.72/(hk2-1.0)   )*theta2  +  d2;
    de2_hk2 = (     - 1.72/(hk2-1.0)/(hk2-1.0))*theta2;

    de2_u2 = de2_hk2*hk2_u2;
    de2_t2 = de2_hk2*hk2_t2 + (3.15 + 1.72/(hk2-1.0));
    de2_d2 = de2_hk2*hk2_d2 + 1.0;
    de2_ms = de2_hk2*hk2_ms;


    hdmax = 12.0;
    if(de2 > hdmax*theta2)
    {
        de2    = hdmax*theta2;
        de2_u2 =  0.0;
        de2_t2 = hdmax;
        de2_d2 =  0.0;
        de2_ms =  0.0;
    }

    return true;
}



bool XFoil::cang(double x[], double y[], int n, int &imax, double &amax)
{
    //-------------------------------------------------------------------
    double dx1(0), dx2(0), dy1(0), dy2(0), crossp(0), angl(0);
    amax = 0.0;
    imax = 1;

    //---- go over each point, calculating corner angle
    for (int i=2; i<=n-1; i++){
        dx1 = x[i] - x[i-1];
        dy1 = y[i] - y[i-1];
        dx2 = x[i] - x[i+1];
        dy2 = y[i] - y[i+1];

        //------ allow for doubled points
        if(dx1==0.0 && dy1==0.0) {
            dx1 = x[i] - x[i-2];
            dy1 = y[i] - y[i-2];
        }
        if(dx2==0.0 && dy2==0.0) {
            dx2 = x[i] - x[i+2];
            dy2 = y[i] - y[i+2];
        }

        crossp = (dx2*dy1 - dy2*dx1)
                / sqrt((dx1*dx1 + dy1*dy1) * (dx2*dx2 + dy2*dy2));
        angl = asin(crossp)*(180.0/PI);
        if(fabs(angl) > fabs(amax))
        {
            amax = angl;
            imax = i;
        }
    }
    return true;
}


bool XFoil::cdcalc()
{
    double thwake(0), urat(0), uewake(0), shwake(0), dx(0);
    int i(0),im(0),is(0),ibl(0);
    double sa, ca;
    sa = sin(alfa);
    ca = cos(alfa);

    if(lvisc && lblini)
    {

        //---- set variables at the end of the wake
        thwake = thet[nbl[2]][2];
        urat   = uedg[nbl[2]][2]/qinf;
        uewake = uedg[nbl[2]][2] * (1.0-tklam) / (1.0 - tklam*urat*urat);
        shwake = dstr[nbl[2]][2]/thet[nbl[2]][2];

        //---- extrapolate wake to downstream infinity using squire-young relation
        //      (reduces errors of the wake not being long enough)
        cd = 2.0*thwake * pow((uewake/qinf),(0.5*(5.0+shwake)));
    }
    else
    {
        cd = 0.0;
    }

    //--- calculate friction drag coefficient
    cdf = 0.0;
    for (is=1; is<=2;is++)
    {
        for(ibl=3;ibl<= iblte[is]; ibl++)
        {
            i  = ipan[ibl][is];
            im = ipan[ibl-1][is];
            dx = (x[i] - x[im])*ca + (y[i] - y[im])*sa;
            cdf = cdf + 0.5*(tau[ibl][is]+tau[ibl-1][is])*dx * 2.0/qinf/qinf;
        }
    }

    return true;
}


/** ---- laminar skin friction function  ( Cf )    ( from Falkner-Skan )*/
bool XFoil::cfl(double hk, double rt, double &cf, double &cf_hk, double &cf_rt, double &cf_msq)
{
    double tmp(0);
    if(hk<5.5)
    {
        tmp = (5.5-hk)*(5.5-hk)*(5.5-hk) / (hk+1.0);
        cf    = ( 0.0727*tmp                      - 0.07       )/rt;
        cf_hk = ( -.0727*tmp*3.0/(5.5-hk) - 0.0727*tmp/(hk+1.0))/rt;
    }
    else
    {
        tmp = 1.0 - 1.0/(hk-4.5);
        cf    = ( 0.015*tmp*tmp      - 0.07  ) / rt;
        cf_hk = ( 0.015*tmp*2.0/(hk-4.5)/(hk-4.5) ) / rt;
    }
    cf_rt = -cf/rt;
    cf_msq = 0.0;

    return true;
}

/** ---- turbulent skin friction function  ( Cf )    (Coles) */
bool XFoil::cft(double hk, double rt, double msq, double &cf, double &cf_hk, double &cf_rt, double &cf_msq)
{
    double gam(0), gm1(0), f_arg(0), fc(0), grt(0), gex(0), thk(0), cfo(0);
    gam =1.4;


    gm1 = gam - 1.0;
    fc  = sqrt(1.0 + 0.5*gm1*msq);
    grt = log(rt/fc);
    grt = std::max(grt,3.0);

    gex = -1.74 - 0.31*hk;

    f_arg = -1.33*hk;
    f_arg = std::max(-20.0, f_arg );

    thk = tanh(4.0 - hk/0.875);

    cfo =  0.3*exp(f_arg) * pow((grt/2.3026),gex);
    cf     = ( cfo  +  0.00011*(thk-1.0) ) / fc;
    cf_hk  = (-1.33*cfo - 0.31*log(grt/2.3026)*cfo
              - 0.00011*(1.0-thk*thk) / 0.875    ) / fc;
    cf_rt  = gex*cfo/(fc*grt) / rt;
    cf_msq = gex*cfo/(fc*grt) * (-0.25*gm1/fc/fc) - 0.25*gm1*(cf)/fc/fc;

    return true;
}


/** -----------------------------------------------------------
 *       integrates surface pressures to get cl and cm.
 *       integrates skin friction to get cdf.
 *       calculates dcl/dalpha for prescribed-cl routines.
 *-----------------------------------------------------------*/
bool XFoil::clcalc(double xref, double yref)
{
    //techwinder addition : calculate XCp position

    //---- moment-reference coordinates
    //ccc       xref = 0.25
    //ccc       yref = 0.

    double beta(0), beta_msq(0), bfac(0), bfac_msq(0), cginc(0);
    double cpi_gam(0), cpc_cpi(0);
    double dx(0), dy(0), dg(0), ax(0), ay(0), ag(0), dx_alf(0), ag_alf(0), ag_msq(0);
    double cpg1(0), cpg1_msq(0), cpg1_alf(0), cpg2(0), cpg2_msq(0), cpg2_alf(0);
    double sa, ca;
    sa = sin(alfa);
    ca = cos(alfa);

    xcp = 0.0;

    beta     = sqrt(1.0 - minf*minf);
    beta_msq = -0.5/beta;

    bfac     = 0.5*minf*minf / (1.0 + beta);
    bfac_msq = 0.5/ (1.0 + beta)- bfac/ (1.0 + beta) * beta_msq;

    cl = 0.0;
    cm = 0.0;

    cdp = 0.0;

    cl_alf = 0.0;
    cl_msq = 0.0;

    int i = 1;
    cginc = 1.0 - (gam[i]/qinf)*(gam[i]/qinf);
    cpg1     = cginc/(beta + bfac*cginc);
    cpg1_msq = -cpg1/(beta + bfac*cginc)*(beta_msq + bfac_msq*cginc);

    cpi_gam = -2.0*gam[i]/qinf/qinf;
    cpc_cpi = (1.0 - bfac*cpg1)/ (beta + bfac*cginc);
    cpg1_alf = cpc_cpi*cpi_gam*gam_a[i];

    for (i=1; i<=n; i++)
    {
        int ip = i+1;
        if(i==n) ip = 1;

        cginc      = 1.0 - (gam[ip]/qinf)*(gam[ip]/qinf);
        cpg2       = cginc/(beta + bfac*cginc);
        cpg2_msq   = -cpg2/(beta + bfac*cginc)*(beta_msq + bfac_msq*cginc);

        cpi_gam    = -2.0*gam[ip]/qinf/qinf;
        cpc_cpi    = (1.0 - bfac*cpg2)/ (beta + bfac*cginc);
        cpg2_alf   = cpc_cpi*cpi_gam*gam_a[ip];

        dx = (x[ip] - x[i])*ca + (y[ip] - y[i])*sa;
        dy = (y[ip] - y[i])*ca - (x[ip] - x[i])*sa;
        dg = cpg2 - cpg1;

        ax = (0.5*(x[ip]+x[i])-xref)*ca + (0.5*(y[ip]+y[i])-yref)*sa;
        ay = (0.5*(y[ip]+y[i])-yref)*ca - (0.5*(x[ip]+x[i])-xref)*sa;
        ag = 0.5*(cpg2 + cpg1);

        dx_alf = -(x[ip] - x[i])*sa + (y[ip] - y[i])*ca;
        ag_alf = 0.5*(cpg2_alf + cpg1_alf);
        ag_msq = 0.5*(cpg2_msq + cpg1_msq);

        cl       = cl     + dx* ag;
        cdp    = cdp    - dy* ag;
        cm       = cm     - dx*(ag*ax + dg*dx/12.0) - dy*(ag*ay + dg*dy/12.0);

        xcp += dx*ag*(x[ip]+x[i])/2.0;

        cl_alf = cl_alf + dx*ag_alf + ag*dx_alf;
        cl_msq = cl_msq + dx*ag_msq;

        cpg1 = cpg2;
        cpg1_alf = cpg2_alf;
        cpg1_msq = cpg2_msq;
    }

    if(fabs(cl)>0.0)     xcp/= cl;
    else                xcp = 0.0;

    return true;
}




bool XFoil::comset()
{
    //---- set karman-tsien parameter tklam
    double beta(0), beta_msq(0);
    beta = sqrt(1.0 - minf*minf);
    beta_msq = -0.5/beta;

    tklam   = minf*minf / (1.0 + beta)/ (1.0 + beta);
    tkl_msq =     1.0 / (1.0 + beta)/ (1.0 + beta)
            - 2.0*tklam/ (1.0 + beta) * beta_msq;

    //---- set sonic pressure coefficient and speed
    if(minf==0.0) {
        cpstar = -999.0;
        qstar = 999.0;
    }
    else{
        cpstar = 2.0 / (gamma*minf*minf) *
                (pow(((1.0 + 0.5*gamm1*minf*minf)/(1.0 + 0.5*gamm1)),(gamma/gamm1))
                 - 1.0 );
        qstar = qinf/minf * sqrt( (1.0 + 0.5*gamm1*minf*minf)
                                  /(1.0 + 0.5*gamm1        ) );
    }

    return true;
}


/** ---------------------------------------------
 *      sets compressible cp from speed.
 * ---------------------------------------------- */
bool XFoil::cpcalc(int n, double q[], double qinf, double minf, double cp[])
{
    bool denneg(false);
    double cpinc(0), den(0), beta(0), bfac(0);

    beta = sqrt(1.0 - minf*minf);
    bfac = 0.5*minf*minf / (1.0 + beta);

    denneg = false;

    for (int i=1; i<=n; i++)
    {
        cpinc = 1.0 - (q[i]/qinf)*(q[i]/qinf);
        den = beta + bfac*cpinc;
        cp[i] = cpinc / den;
        if(den <= 0.0) denneg = true;
    }

    if(denneg)
    {
        std::string str("CpCalc: local speed too larger\n Compressibility corrections invalid\n");
        writeString(str, true);
        return false;
    }

    return true;
}


void XFoil::writeString(const std::string &str, bool bFullReport)
{
    if(!bFullReport && !s_bFullReport) return;
    m_Report.append(str);
}


/** -----------------------------------------------
 *      calculates curvature of splined 2-d curve |
 *      at s = ss                                 |
 *                                                |
 *      s        arc length array of curve        |
 *      x, y     coordinate arrays of curve       |
 *      xs,ys    derivative arrays                |
 *               (calculated earlier by spline)   |
 * ------------------------------------------------ */
double XFoil::curv(double ss, double x[], double xs[], double y[], double ys[], double s[], int n)
{
    int ilow(0), i(0), imid(0);
    double crv(0), ds(0), t(0), cx1(0), cx2(0), xd(0), xdd(0), cy1(0), cy2(0), yd(0), ydd(0), sd(0);

    ilow = 1;
    i = n;

stop10:
    if(i-ilow<=1) goto stop11;
    imid = (i+ilow)/2;
    if(ss < s[imid]) i = imid;
    else ilow = imid;
    goto stop10;

stop11:

    ds = s[i] - s[i-1];
    t = (ss - s[i-1]) / ds;

    cx1 = ds*xs[i-1] - x[i] + x[i-1];
    cx2 = ds*xs[i]   - x[i] + x[i-1];
    xd = x[i] - x[i-1] + (1.0-4.0*t+3.0*t*t)*cx1 + t*(3.0*t-2.0)*cx2;
    xdd = (6.0*t-4.0)*cx1 + (6.0*t-2.0)*cx2;

    cy1 = ds*ys[i-1] - y[i] + y[i-1];
    cy2 = ds*ys[i]   - y[i] + y[i-1];
    yd = y[i] - y[i-1] + (1.0-4.0*t+3.0*t*t)*cy1 + t*(3.0*t-2.0)*cy2;
    ydd = (6.0*t-4.0)*cy1 + (6.0*t-2.0)*cy2;

    sd = sqrt(xd*xd + yd*yd);
    sd = std::max(sd,0.001*ds);

    crv = (xd*ydd - yd*xdd) / sd/ sd/ sd;

    return crv;
}


/** --------------------------------------------------
 *      calculates d2x/ds2(ss)                       |
 *      xs array must have been calculated by spline |
 * --------------------------------------------------- */
double XFoil::d2val(double ss, double x[], double xs[], double s[], int n)
{
    int i(0), imid(0), ilow(0);
    double ds(0), t(0), cx1(0), cx2(0), dtwoval(0);

    ilow = 1;
    i = n;
stop10:
    if(i-ilow<=1) goto stop11;
    imid = (i+ilow)/2;
    if(ss < s[imid]) i = imid;
    else ilow = imid;
    goto stop10;

stop11:
    ds = s[i] - s[i-1];
    t = (ss - s[i-1]) / ds;
    cx1 = ds*xs[i-1] - x[i] + x[i-1];
    cx2 = ds*xs[i]   - x[i] + x[i-1];
    dtwoval = (6.0*t-4.0)*cx1 + (6.0*t-2.0)*cx2;
    dtwoval = dtwoval/ds/ds;
    return dtwoval;
}


/** ==============================================================
 *      amplification rate routine for envelope e^n method.
 *      reference:
 *                 Drela, M., Giles, M.,
 *                "Viscous/inviscid analysis of transonic and
 *                 low reynolds number airfoils",
 *                 AIAA journal, oct. 1987.
 *
 *      new version.   March 1991       (latest bug fix  july 93)
 *           - m(h) correlation made more accurate up to h=20
 *           - for h > 5, non-similar profiles are used
 *             instead of falkner-skan profiles.  these
 *             non-similar profiles have smaller reverse
 *             velocities, are more representative of typical
 *             separation bubble profiles.
 * --------------------------------------------------------------
 *
 *      input :   hk     kinematic shape parameter
 *                th     momentum thickness
 *                Rt     momentum-thickness reynolds number
 *
 *      output:   ax     envelope spatial amplification rate
 *                ax_(.) sensitivity of ax to parameter (.)
 *
 *
 *      usage: the log of the envelope amplitude n(x) is
 *             calculated by integrating ax (= dn/dx) with
 *             respect to the streamwise distance x.
 *                       x
 *                      /
 *               n(x) = | ax(h(x),th(x),Rth(x)) dx
 *                      /
 *                       0
 *             the integration can be started from the leading
 *             edge since ax will be returned as zero when rt
 *             is below the critical rtheta.  transition occurs
 *             when n(x) reaches ncrit (ncrit= 9 is "standard").
 * ============================================================== */
bool XFoil::dampl(double hk, double th, double rt, double &ax, double &ax_hk, double &ax_th, double &ax_rt)
{
    double dgr = 0.08;

    double hmi(0), hmi_hk(0), aa(0), aa_hk(0), bb(0), bb_hk(0), grcrit(0), grc_hk(0), gr(0), gr_rt(0);
    double rnorm(0), rn_hk(0), rn_rt(0), rfac(0), rfac_hk(0), rfac_rt(0);
    double rfac_rn(0), arg_hk(0),ex(0), f_arg(0), ex_hk(0);
    double af(0), af_hmi(0), af_hk(0), dadr(0), dadr_hk(0);


    hmi = 1.0/(hk - 1.0);
    hmi_hk = -hmi*hmi;

    //---- log10(critical rth) - h   correlation for falkner-skan profiles
    aa    = 2.492*pow(hmi,0.43);
    aa_hk =   (aa/hmi)*0.43 * hmi_hk;
    bb    = tanh(14.0*hmi - 9.24);
    bb_hk = (1.0 - bb*bb) * 14.0 * hmi_hk;
    grcrit = aa    + 0.7*(bb + 1.0);
    grc_hk = aa_hk + 0.7* bb_hk;
    gr = log10(rt);
    gr_rt = 1.0 / (2.3025851*rt);
    if(gr < grcrit-dgr)
    {

        //----- no amplification for rtheta < rcrit
        ax    = 0.0;
        ax_hk = 0.0;
        ax_th = 0.0;
        ax_rt = 0.0;
    }
    else
    {

        //----- set steep cubic ramp used to turn on ax smoothly as rtheta
        //-     exceeds rcrit (previously, this was done discontinuously).
        //-     the ramp goes between  -dgr < log10(rtheta/rcrit) < dgr

        rnorm = (gr - (grcrit-dgr)) / (2.0*dgr);
        rn_hk =     -  grc_hk       / (2.0*dgr);
        rn_rt =  gr_rt              / (2.0*dgr);

        if(rnorm >= 1.0) {
            rfac    = 1.0;
            rfac_hk = 0.0;
            rfac_rt = 0.0;
        }
        else{
            rfac    = 3.0*rnorm*rnorm - 2.0*rnorm*rnorm*rnorm;
            rfac_rn = 6.0*rnorm    - 6.0*rnorm*rnorm;

            rfac_hk = rfac_rn*rn_hk;
            rfac_rt = rfac_rn*rn_rt;
        }

        //----- amplification envelope slope correlation for falkner-skan
        f_arg  = 3.87*hmi    - 2.52;
        arg_hk = 3.87*hmi_hk;

        ex    = exp(-f_arg*f_arg);
        ex_hk = ex * (-2.0*f_arg*arg_hk);

        dadr    = 0.028*(hk-1.0) - 0.0345*ex;
        dadr_hk = 0.028          - 0.0345*ex_hk;

        //----- new m(h) correlation    1 march 91
        af = -0.05 + 2.7*hmi -  5.5*hmi*hmi + 3.0*hmi*hmi*hmi;
        af_hmi =     2.7     - 11.0*hmi    + 9.0*hmi*hmi;
        af_hk = af_hmi*hmi_hk;

        ax    =   (af   *dadr/th                ) * rfac;
        ax_hk =   (af_hk*dadr/th + af*dadr_hk/th) * rfac
                + (af   *dadr/th                ) * rfac_hk;
        ax_th = - (ax)/th;
        ax_rt =   (af   *dadr/th                ) * rfac_rt;

    }

    return true;
}



/** --------------------------------------------------
 *       calculates dx/ds(ss)                         |
 *       xs array must have been calculated by spline |
 * -------------------------------------------------- */
double XFoil::deval(double ss, double x[], double xs[], double s[], int n)
{
    int ilow(0), i(0), imid(0);
    double ds(0), t(0), cx1(0), cx2(0), deval(0);

    ilow = 1;
    //    i = nc;
    i = n; ///techwinder modified


    while(i-ilow>1)
    {
        imid = (i+ilow)/2;
        if(ss < s[imid]) i = imid;
        else ilow = imid;
    }

    ds = s[i] - s[i-1];
    t = (ss - s[i-1]) / ds;
    cx1 = ds*xs[i-1] - x[i] + x[i-1];
    cx2 = ds*xs[i]     - x[i] + x[i-1];
    deval = x[i] - x[i-1] + (1.0-4.0*t+3.0*t*t)*cx1
            + t*(3.0*t-2.0)*cx2;
    deval = deval/ds;
    return deval;
}


/** Laminar dissipation function  ( 2 Cd/H* )     (from Falkner-Skan)*/
bool XFoil::dil(double hk, double rt, double &di, double &di_hk, double &di_rt)
{
    if(hk<4.0)
    {
        di    = ( 0.00205   *  pow((4.0-hk),5.5) + 0.207 ) / rt;
        di_hk = ( -.00205*5.5*pow((4.0-hk),4.5)         ) / rt;
    }
    else
    {
        double hkb = hk - 4.0;
        double den = 1.0 + 0.02*hkb*hkb;
        di    = ( -.0016  *  hkb*hkb  /den   + 0.207              ) / rt;
        di_hk = ( -.0016*2.0*hkb*(1.0/den - 0.02*hkb*hkb/den/den) ) / rt;
    }
    di_rt = -(di)/rt;

    return true;
}



bool XFoil::dilw(double hk, double rt, double &di, double &di_hk, double &di_rt)
{
    //    double msq = 0.0;
    double hs(0), hs_hk(0), hs_rt(0), hs_msq(0);

    hsl(hk, hs, hs_hk, hs_rt, hs_msq);
    //---- laminar wake dissipation function  ( 2 cd/h* )
    double rcd    =  1.10 * (1.0 - 1.0/hk)* (1.0 - 1.0/hk) / hk;
    double rcd_hk = -1.10 * (1.0 - 1.0/hk)*2.0/hk/hk/hk- rcd/hk;

    di    = 2.0*rcd   /(hs*rt);
    di_hk = 2.0*rcd_hk/(hs*rt) - ((di)/hs)*hs_hk;
    di_rt = -(di)/rt         - ((di)/hs)*hs_rt;

    return true;
}


bool XFoil::dslim(double &dstr, double thet, double msq, double hklim)
{
    double h(0), hk(0), hk_h(0), hk_m(0), dh(0);
    h = (dstr)/thet;

    hkin(h, msq, hk, hk_h, hk_m);

    dh = std::max(0.0 , hklim-hk ) / hk_h;
    dstr = (dstr) + dh*thet;

    return true;
}


/** ------------------------------------------------
 *     finds minimum cp on dist for cavitation work
 * ------------------------------------------------ */
bool XFoil::fcpmin()
{
    xcpmni = x[1];
    xcpmnv = x[1];
    cpmni = cpi[1];
    cpmnv = cpv[1];

    for (int i=2; i<=n+nw; i++)
    {
        if(cpi[i] < cpmni)
        {
            xcpmni = x[i];
            cpmni = cpi[i];
        }
        if(cpv[i] < cpmnv)
        {
            xcpmnv = x[i];
            cpmnv = cpv[i];
        }
    }


    if (lvisc) cpmn = cpmnv;
    else
    {
        cpmn = cpmni;
        cpmnv = cpmni;
        xcpmnv = xcpmni;
    }

    return true;
}



bool XFoil::gamqv()
{
    int i;
    for (i=1; i<=n; i++)
    {
        gam[i]   = qvis[i];
        gam_a[i] = qinv_a[i];
    }

    return true;
}



/**
  *   Solves general nxn system in nn unknowns
  *   with arbitrary number (nrhs) of righthand sides.
  *   assumes system is invertible...
  *    ...if it isn't, a divide by zero will result.
  *
  *   z is the coefficient matrix...
  *     ...destroyed during solution process.
  *   r is the righthand side(s)...
  *     ...replaced by the solution vector(s).
  *
  *                              mark drela  1984
  */
bool XFoil::Gauss(int nn, double z[][6], double r[5]){
    // dimension z(nsiz,nsiz), r(nsiz,nrhs)

    int loc(0);
    int np(0), nnpp(0), nt(0), nx(0), k(0);

    double temp(0), ztmp(0), pivot(0);

    for (np=1; np<=nn-1; np++){
        nnpp = np+1;
        //------ find max pivot index nx
        nx = np;
        for (nt =nnpp; nt<=nn; nt++){
            if (fabs(z[nt][np])>fabs(z[nx][np])) nx = nt;
        }

        pivot = 1.0/z[nx][np];

        //------ switch pivots
        z[nx][np] = z[np][np];

        //------ switch rows & normalize pivot row
        for (loc = nnpp; loc<=nn; loc++){
            temp = z[nx][loc]*pivot;
            z[nx][loc] = z[np][loc];
            z[np][loc] = temp;
        }

        temp = r[nx]*pivot;
        r[nx] = r[np];
        r[np] = temp;

        //------ forward eliminate everything
        for (k = nnpp; k<=nn; k++){
            ztmp = z[k][np];
            for (loc=nnpp; loc<=nn;loc++) z[k][loc] = z[k][loc] - ztmp*z[np][loc];
            r[k] = r[k] - ztmp*r[np];
        }
    }

    //---- solve for last row
    r[nn] = r[nn]/z[nn][nn];

    //---- back substitute everything
    for (np=nn-1; np>= 1; np--){
        nnpp = np+1;
        for(k=nnpp; k<= nn;k++)
            r[np] = r[np] - z[np][k]*r[k];
    }

    return true;
}

/** *****************************************************
  *                                                     *
  *   solves general nxn system in nn unknowns          *
  *    with arbitrary number (nrhs) of righthand sides. *
  *   assumes system is invertible...                   *
  *    ...if it isn't, a divide by zero will result.    *
  *                                                     *
  *   z is the coefficient matrix...                    *
  *     ...destroyed during solution process.           *
  *   r is the righthand side(s)...                     *
  *     ...replaced by the solution vector(s).          *
  *                                                     *
  *                              mark drela  1984       *
  ****************************************************** */
bool XFoil::Gauss(int nn, double z[IQX][IQX], double r[IQX]){
    // techwinder : only one rhs is enough ! nrhs = 1
    // dimension z(nsiz,nsiz), r(nsiz,nrhs)

    int loc(0);
    int np(0), nnpp(0), nt(0), nx(0), k(0);

    double temp(0), ztmp(0), pivot(0);

    for (np=1; np<=nn-1; np++)
    {
        nnpp = np+1;
        //------ find max pivot index nx
        nx = np;
        for (nt =nnpp; nt<=nn; nt++)
        {
            if (fabs(z[nt][np])>fabs(z[nx][np])) nx = nt;
        }

        pivot = 1.0/z[nx][np];

        //------ switch pivots
        z[nx][np] = z[np][np];

        //------ switch rows & normalize pivot row
        for (loc = nnpp; loc<=nn; loc++){
            temp = z[nx][loc]*pivot;
            z[nx][loc] = z[np][loc];
            z[np][loc] = temp;
        }

        temp = r[nx]*pivot;
        r[nx] = r[np];
        r[np] = temp;

        //------ forward eliminate everything
        for (k = nnpp; k<=nn; k++){
            ztmp = z[k][np];
            for (loc=nnpp; loc<=nn;loc++) z[k][loc] = z[k][loc] - ztmp*z[np][loc];
            r[k] = r[k] - ztmp*r[np];
        }
    }

    //---- solve for last row
    r[nn] = r[nn]/z[nn][nn];

    //---- back substitute everything
    for (np=nn-1; np>= 1; np--){
        nnpp = np+1;
        for(k=nnpp; k<= nn;k++)
            r[np] = r[np] - z[np][k]*r[k];
    }

    return true;
}


/** ------------------------------------------------------
 *      sets geometric parameters for airfoil shape
 * ------------------------------------------------------ */
bool XFoil::geopar(double x[], double xp[], double y[], double yp[], double s[],
                   int n, double t[], double &sle, double  &chord,
                   double &area, double &radle, double &angte,
                   double &ei11a, double &ei22a, double &apx1a, double &apx2a,
                   double &ei11t, double &ei22t, double &apx1t, double &apx2t)
{
    double chsq(0), curvle(0), ang1(0), ang2(0), xcena(0), ycena(0), slen(0), xcent(0), ycent(0);

    lefind(sle,x,xp,y,yp,s,n);

    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);

    chsq = (xte-xle)*(xte-xle) + (yte-yle)*(yte-yle);
    chord = sqrt(chsq);

    curvle = curv(sle,x,xp,y,yp,s,n);

    radle = 0.0;
    if(fabs(curvle) > 0.001*(s[n]-s[1])) radle = 1.0 / curvle;

    ang1 = atan2( -yp[1] , -xp[1] );
    ang2 = atanc(  yp[n] ,  xp[n] , ang1 );
    angte = ang2 - ang1;

    for (int i=1; i<=n; i++) t[i] = 1.0;

    aecalc(n,x,y,t, 1, area,xcena,ycena,ei11a,ei22a,apx1a,apx2a);
    aecalc(n,x,y,t, 2, slen,xcent,ycent,ei11t,ei22t,apx1t,apx2t);

    // techwinder 22/18/01 updated source code i.a.w. XFoil 6.99
    //--- old, approximate thickness,camber routine (on discrete points only)
    tccalc(x,xp,y,yp,s,n, thick, xthick, cambr, xcambr);
    //--- more accurate thickness and camber estimates
/*    getcam(xcam,ycam,ncam,xthk,ythk,nthk,x,xp,y,yp,s,n );
    getmax(xcam,ycam,ycamp,ncam,xcambr,cambr);
    getmax(xthk,ythk,ythkp,nthk,xthick,thick);*/
    thick = 2.0*thick;

    thickb = thick;
    cambrb = cambr;

    //      write(*,1000) thick,xthick,cambr,xcambr
    // 1000 format( ' max thickness = ',f12.6,'  at x = ',f7.3,/' max camber    = ',f12.6,'  at x = ',f7.3)

    return true;
}



void XFoil::tccalc(double x[], double xp[], double y[], double yp[], double s[],
                  int n, double &thick, double &xthick, double &cambr, double &xcambr)
{
//---------------------------------------------------------------
//    calculates max thickness and camber at airfoil points
//
//    note: this routine does not find the maximum camber or
//          thickness exactly as it only looks at discrete points
//
//    input:
//      n      number of points
//      x(.)   shape coordinate point arrays
//      y(.)
//
//    output:
//      thick  max thickness
//      cambr  max camber
//---------------------------------------------------------------
    lefind(sle,x,xp,y,yp,s,n);
    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);
    chord = sqrt((xte-xle)*(xte-xle) + (yte-yle)*(yte-yle));

//---- set unit chord-line vector
    double dxc = (xte-xle) / chord;
    double dyc = (yte-yle) / chord;

    thick  = 0.0;
    xthick = 0.0;
    cambr  = 0.0;
    xcambr = 0.0;

//---- go over each point, finding the y-thickness and camber
    for(int i=1; i<=n; i++)
    {
//        double xbar = (x[i]-xle)*dxc + (y[i]-yle)*dyc;
        double ybar = (y[i]-yle)*dxc - (x[i]-xle)*dyc;

//------ set point on the opposite side with the same chord x value
        double sopp(0);
        sopps(sopp, s[i], x,xp,y,yp,s,n, sle);
        double xopp = seval(sopp,x,xp,s,n);
        double yopp = seval(sopp,y,yp,s,n);

        double ybarop = (yopp-yle)*dxc - (xopp-xle)*dyc;

        double yc = 0.5*(ybar+ybarop);
        double yt =  abs(ybar-ybarop);

        if(abs(yc)>abs(cambr))
        {
            cambr = yc;
            xcambr = xopp;
        }
        if(abs(yt)>abs(thick))
        {
            thick = yt;
            xthick = xopp;
        }
    }
}


/** ------------------------------------------------------
 *      finds camber and thickness
 *      distribution for input airfoil
 * ------------------------------------------------------ */
void XFoil::getcam(double xcm[], double ycm[], int &ncm, double xtk[], double ytk[], int &ntk,
                   double x[], double xp[], double y[], double yp[],double s[], int n)
{
    double sl(0), xl(0), yl(0), sopp(0), xopp(0), yopp(0), tol(0);

    xlfind(sl,x,xp,y,yp,s,n);
    xl = seval(sl,x,xp,s,n);
    yl = seval(sl,y,yp,s,n);

    //---- go over each point, finding opposite points, getting camber and thickness
    for (int i=1; i<=n; i++)
    {
        //------ coordinates of point on the opposite side with the same x value
        sopps(sopp, s[i], x,xp,y,yp,s,n,sl);
        xopp = seval(sopp,x,xp,s,n);
        yopp = seval(sopp,y,yp,s,n);

        // techwinder 22/18/01 updated source code i.a.w. XFoil 6.99
        if(i==1)
        {
            xopp = x[n];
            yopp = y[n];
        }
        else if(i==n)
        {
            xopp = x[1];
            yopp = y[1];
        } // end techwinder insertion

        //------ get camber and thickness
        xcm[i] = 0.5*(x[i]+xopp);
        ycm[i] = 0.5*(y[i]+yopp);
        xtk[i] = 0.5*(x[i]+xopp);
        ytk[i] = 0.5*(y[i]-yopp);
        ytk[i] = fabs(ytk[i]);
//        if (xopp.gt.0.9) then
//         write(*,*) 'cm i,x,y ',i,xcm(i),ycm(i)
//         write(*,*) 'tk i,x,y ',i,xtk(i),ytk(i)
//        endif
    }

    //---- tolerance for nominally identical points
    // jx-mod
    //    tol = 0.001 * (s[n]-s[1]);     ! Bad bug -- was losing x=1.0 point - see org. xfoil
    tol = 1.e-5 * (s[n]-s[1]);

    //---- sort the camber points
    ncm = n+1;
    xcm[n+1] = xl;
    ycm[n+1] = yl;
    sortol(tol,ncm,xcm,ycm);

    //--- reorigin camber from le so camberlines start at y=0  4/24/01 hhy
    //    policy now to generate camber independent of y-offsets
    yof = ycm[1];
    for (int i=1; i<=ncm; i++)
    {
        ycm[i] -= yof;
    }

    //---- sort the thickness points

    ntk = n+1;
    xtk[n+1] = xl;
    ytk[n+1] = 0.0;
    sortol(tol,ntk,xtk,ytk);
}


/** ------------------------------------------------
 *      calculates camber or thickness highpoint
 *      and x position
 * ------------------------------------------------ */
void XFoil::getmax(double x[],double y[], double yp[], int n, double &xmax, double &ymax)
{
    double xlen(0), xtol(0);
    double xmax0(0), ymax0(0), ddx(0), res(0), resp(0), dx(0);

    ddx = 0.0;

    xlen = x[n] - x[1];
    xtol = xlen * 0.00001;

    segspl(y,yp,x,n);

    //---- get approx max point and rough interval size
    ymax0 = y[1];
    xmax0 = x[1];
    for (int i=2; i<=n; i++)
    {
        if (fabs(y[i])>fabs(ymax0))
        {
            ymax0 = y[i];
            xmax0 = 0.5*(x[i-1] + x[i]);
            ddx   = 0.5*fabs(x[i+1] - x[i-1]);
        }
    }
    xmax = xmax0;

    //---- do a newton loop to refine estimate
    bool bConv =false;
    for (int iter=1; iter<= 10; iter++)
    {
        ymax  = seval(xmax,y,yp,x,n);
        res   = deval(xmax,y,yp,x,n);
        resp  = d2val(xmax,y,yp,x,n);
        if (fabs(xlen*resp) < 1.0e-6)
        {
            bConv = true;
            break;//go to 20
        }
        dx = -res/resp;
        dx = sign(std::min(0.5*ddx,fabs(dx)), dx);
        xmax += dx;
        if(fabs(dx) < xtol)
        {
            bConv = true;
            break;//go to 20
        }
    }
    //      write(*,*)  'getmax: newton iteration for max camber/thickness failed.'
    if(!bConv)
    {
        ymax = ymax0;
        xmax = xmax0;
    }
}

/** ------------------------------------------------------
 *      locates leftmost (minimum x) point location sle
 *
 *      the defining condition is
 *
 *       x' = 0     at  s = sle
 *
 *      i.e. the surface tangent is vertical
 * ------------------------------------------------------ */
void XFoil::xlfind(double &sle, double x[], double xp[], double y[], double yp[], double s[], int n)
{
    (void)y;
    (void)yp;

    double dslen(0), dseps(0), dx(0), dxds(0), dxdd(0), dsle(0), res(0), ress(0);
    int i(0), iter(0);
    dslen = s[n] - s[1];

    //---- convergence tolerance
    dseps = (s[n]-s[1]) * 0.00001;

    //---- get first guess for sle
    for (i=3; i<=n-2; i++)
    {
        dx = x[i+1] - x[i];
        if(dx > 0.0) break;
    }

    sle = s[i];

    //---- check for sharp le case
    if(fabs(s[i] - s[i-1])<1.e-06) {// changed techwinder
        //ccc        write(*,*) 'sharp le found at ',i,sle
        return;
    }

    //---- newton iteration to get exact sle value
    for (iter=1 ;iter<= 50; iter++)
    {
        dxds = deval(sle,x,xp,s,n);
        dxdd = d2val(sle,x,xp,s,n);

        //------ drive dxds to zero
        res  = dxds;
        ress = dxdd;

        //------ newton delta for sle
        dsle = -res/ress;

        dsle = std::max( dsle , -0.01*fabs(dslen) );
        dsle = std::min( dsle ,  0.01*fabs(dslen) );
        sle += dsle;
        if(fabs(dsle) < dseps) return;
    }
    //      write(*,*) 'xlfind:  left point not found.  continuing...'
    sle = s[i];
}


void XFoil::sortol(double tol,int &kk,double s[],double w[])
{
    //    dimension s(kk), w(kk)
    bool done(false);
    int np(0), kks(0), ipass(0), k(0), kt(0);
    double temp(0), dsq(0);

    //---- sort arrays
    for (ipass=1; ipass<= 1234; ipass++)
    {
        done = true;
        for (int n=1; n<= kk-1; n++)
        {
            np = n+1;

            if(s[np]<s[n])
            {
                temp  = s[np];
                s[np] = s[n];
                s[n]  = temp;
                temp  = w[np];
                w[np] = w[n];
                w[n]  = temp;
                done = false;
            }
        }
        if(done)
            break;
    }
    //    if(!done) AfxMessageBox("sort failed");

    //---- search for near-duplicate pairs and eliminate extra points
    //---- modified 4/24/01 hhy to check list until all duplicates removed
    //    this cures a bug for sharp le foils where there were 3 le points in
    //    camber, thickness lists from getcam.
    done = false;
    while(!done)
    {
        kks = kk;
        done = true;
        for (k=1; k<= kks;k++)
        {
            if(k<kk)
            {//go to 20
                dsq = (s[k]-s[k+1])*(s[k]-s[k+1]) + (w[k]-w[k+1])*(w[k]-w[k+1]);
                if(dsq<tol*tol)
                { //go to 20
                    //------- eliminate extra point pairs
                    //cc         write(*,*) 'extra on point ',k,kks
                    kk = kk-1;
                    for (kt=k+1; kt<=kk; kt++)
                    {
                        s[kt] = s[kt+1];
                        w[kt] = w[kt+1];
                    }
                    done = false;
                }
            }
        }
    }// 20 continue
    //     if(!done) go to 10
}


/** --------------------------------------------------
 *      calculates arc length sopp of point
 *      which is opposite of point si, on the
 *      other side of the airfoil baseline
 * -------------------------------------------------- */
void XFoil::sopps(double &sopp, double si, double x[], double xp[], double y[], double yp[], double s[],
                  int n, double sle){
    //      dimension x(*),xp(*),y(*),yp(*),s(*)
    double chord(0), slen(0), dxc(0), dyc(0), sfrac(0);
    double xi(0), yi(0), xbar(0), xopp(0), yopp(0), xoppd(0), yoppd(0);
    double res(0), resd(0), dsopp(0);
    int in(0), inopp(0);

    //---- reference length for testing convergence
    slen = s[n] - s[1];

    //---this fails miserably with sharp le foils, tsk,tsk,tsk hhy 4/24/01
    //---- set baseline vector normal to surface at le point
    //      dxc = -deval(sle,y,yp,s,n)
    //      dyc =  deval(sle,x,xp,s,n)
    //      dsc = sqrt(dxc**2 + dyc**2)
    //      dxc = dxc/dsc
    //      dyc = dyc/dsc

    //---rational alternative 4/24/01 hhy
    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);
    chord = sqrt((xte-xle)*(xte-xle) + (yte-yle)*(yte-yle));
    //---- set unit chord-line vector
    dxc = (xte-xle) / chord;
    dyc = (yte-yle) / chord;

    if(si<sle) {
        in = 1;
        inopp = n;
    }
    else{
        in = n;
        inopp = 1;
    }
    sfrac = (si-sle)/(s[in]-sle);
    sopp = sle + sfrac*(s[inopp]-sle);

    if(fabs(sfrac) <= 1.0e-5) {
        sopp = sle;
        return;
    }

    //---- xbar = x coordinate in chord-line axes
    xi  = seval(si , x,xp,s,n);
    yi  = seval(si , y,yp,s,n);
    xle = seval(sle, x,xp,s,n);
    yle = seval(sle, y,yp,s,n);
    xbar = (xi-xle)*dxc + (yi-yle)*dyc;

    //---- converge on exact opposite point with same xbar value
    bool bFound = false;
    for (int iter=1; iter<= 12;iter++){
        xopp  = seval(sopp,x,xp,s,n);
        yopp  = seval(sopp,y,yp,s,n);
        xoppd = deval(sopp,x,xp,s,n);
        yoppd = deval(sopp,y,yp,s,n);

        res  = (xopp -xle)*dxc + (yopp -yle)*dyc - xbar;
        resd =  xoppd     *dxc +  yoppd     *dyc;

        if(fabs(res)/slen < 1.0e-5) {
            bFound = true;
            break;//go to 305
        }
        if(resd == 0.0) {
            bFound = false;
            break;//go to 303
        }
        dsopp = -res/resd;
        sopp += dsopp;

        if(fabs(dsopp)/slen < 1.0e-5) {
            bFound = true;
            break;//go to 305
        }
    }
    if(!bFound){
        //303        write(*,*) 'sopps: opposite-point location failed. continuing...'
        sopp = sle + sfrac*(s[inopp]-sle);
    }
    // 305  continue

}


bool XFoil::getxyf(double x[], double xp[], double y[], double yp[], double s[],
                   int n, double &tops, double &bots, double xf, double &yf)
{
    double topy(0), boty(0), yrel(0);
    //    if(xf <= -999.0)  {   //  askr("enter flap hinge x location",xf)
    //        xf = 0.5;//arcs added
    //    }
    //---- find top and bottom y at hinge x location

    tops = s[1] + (x[1] - xf);
    bots = s[n] - (x[n] - xf);
    sinvrt(tops,xf,x,xp,s,n);
    sinvrt(bots,xf,x,xp,s,n);
    topy = seval(tops,y,yp,s,n);
    boty = seval(bots,y,yp,s,n);

    //        write(*,1000) topy, boty
    // 1000 format(/'  top      surface:    y =', f8.4,'     y/t = 1.0'
    //       &       /'  bottom surface:    y =', f8.4,'     y/t = 0.0')

    //    if(*pyf <= -999.0)
    //        askr( 'enter flap hinge y location (or 999 to specify y/t)^',yf);
    //        *pyf=0.0;//arcs added

    //    if(*pyf >= 999.0) {
    //          askr("enter flap hinge relative y/t location",yrel);
    //        yrel = 0.5;//arcs added
    //    }
    //    flap y location in pyf is relative;
    yrel = yf;
    //    so convert to absolute value

    yf = topy*yrel + boty*(1.0-yrel);
    return true;
}



/** --------------------------------------------------------------
 *     Calculates two surface vorticity (gamma) distributions
 *     for alpha = 0, 90  degrees.  These are superimposed
 *     in specal or speccl for specified alpha or cl.
 *-------------------------------------------------------------- */
bool XFoil::ggcalc()
{
    double psi(0), psi_n(0), res(0), res1(0), res2(0), ag1(0), ag2(0);
    double abis(0), cbis(0), sbis(0), ds1(0), ds2(0), dsmin(0);
    double xbis(0), ybis(0), qbis(0), bwt(0);
    double bbb[IQX];
    //    double psiinf;

    cosa = cos(alfa);
    sina = sin(alfa);

    //---- distance of internal control point ahead of sharp TE
    //-    (fraction of smaller panel length adjacent to TE)
    bwt = 0.1;

    std::string str("   Calculating unit vorticity distributions ...\n");
    writeString(str);


    for(int i=1; i<=n; i++)
    {
        gam[i] = 0.0;
        gamu[i][1] = 0.0;
        gamu[i][2] = 0.0;
    }
    psio = 0.0;

    //---- set up matrix system for  psi = psio  on airfoil surface.
    //-    the unknowns are (dgamma)i and dpsio.
    for (int i=1; i<=n; i++)
    {
        //------ calculate psi and dpsi/dgamma array for current node
        psilin(i, x[i], y[i], nx[i], ny[i], psi, psi_n, false, true);

//        psiinf = qinf*(cosa*y[i] - sina*x[i]);

        //------ res1 = psi( 0) - psio
        //------ res2 = psi(90) - psio
        res1 =  qinf*y[i];
        res2 = -qinf*x[i];

        //------ dres/dgamma
        for (int j=1; j<=n; j++)
        {
            aij[i][j] = dzdg[j];
        }

        for (int j=1; j<=n; j++)
        {
            bij[i][j] = -dzdm[j];
        }

        //------ dres/dpsio
        aij[i][n+1] = -1.0;

        gamu[i][1] = -res1;
        gamu[i][2] = -res2;
    }

    //---- set Kutta condition
    //-    res = gam(1) + gam[n]
    res = 0.0;

    for (int j=1; j<=n+1; j++) aij[n+1][j] = 0.0;

    aij[n+1][1] = 1.0;
    aij[n+1][n] = 1.0;

    gamu[n+1][1] = -res;
    gamu[n+1][2] = -res;

    //---- set up Kutta condition (no direct source influence)
    for (int j=1; j<=n; j++) bij[n+1][j] = 0.0;


    if(sharp)
    {
        //----- set zero internal velocity in TE corner

        //----- set TE bisector angle
        ag1 = atan2(-yp[1],-xp[1]    );
        ag2 = atanc( yp[n], xp[n],ag1);
        abis = 0.5*(ag1+ag2);

        cbis = cos(abis);
        sbis = sin(abis);

        //----- minimum panel length adjacent to TE
        ds1 = sqrt((x[1]-x[2]  )*(x[1]-x[2]  ) + (y[1]-y[2]  )*(y[1]-y[2]  ));
        ds2 = sqrt((x[n]-x[n-1])*(x[n]-x[n-1]) + (y[n]-y[n-1])*(y[n]-y[n-1]));
        dsmin = std::min(ds1 , ds2);

        //----- control point on bisector just ahead of TE point
        xbis = xte - bwt*dsmin*cbis;
        ybis = yte - bwt*dsmin*sbis;

        //----- set velocity component along bisector line
        psilin(0, xbis, ybis, -sbis, cbis, psi, qbis, false, true);
        res = 1000.0;

        //----- dres/dgamma
        for (int j=1; j<=n; j++) aij[n][j] = dqdg[j];

        //----- -dres/dmass
        for (int j=1; j<=n; j++) bij[n][j] = -dqdm[j];

        //----- dres/dpsio
        aij[n][n+1] = 0.0;

        //----- -dres/duinf
        gamu[n][1] = -cbis;

        //----- -dres/dvinf
        gamu[n][2] = -sbis;
    }

    //---- lu-factor coefficient matrix aij
    ludcmp(n+1,aij,aijpiv);
    lqaij = true;

    //---- solve system for the two vorticity distributions
    for (int iu=0; iu<IQX; iu++) bbb[iu] = gamu[iu][1];//techwinder : create a dummy array
    baksub(n+1, aij, aijpiv, bbb);
    for (int iu=0; iu<IQX; iu++) gamu[iu][1] = bbb[iu];

    for (int iu=0; iu<IQX; iu++) bbb[iu] = gamu[iu][2];//techwinder : create a dummy array
    baksub(n+1, aij, aijpiv, bbb);
    for (int iu=0; iu<IQX; iu++) gamu[iu][2] = bbb[iu] ;


    //---- set inviscid alpha=0,90 surface speeds for this geometry
    for (int i=1; i<=n+1; i++)
    {
        qinvu[i][1] = gamu[i][1];
        qinvu[i][2] = gamu[i][2];
    }

    lgamu = true;

    return true;
}



bool XFoil::baksub(int n, double a[IQX][IQX], int indx[], double b[])
{
    double sum(0);
    int i(0), ii(0), ll(0), j(0);

    ii = 0;
    for (i=1; i<=n; i++){
        ll = indx[i];
        sum = b[ll];
        b[ll] = b[i];
        if(ii!=0)
            for (j=ii;j<=i-1; j++) sum = sum - a[i][j]*b[j];
        else
            if(sum!=0.0) ii = i;

        b[i] = sum;
    }
    //
    for (i=n; i>=1; i--){
        sum = b[i];
        if(i<n)
            for (j=i+1; j<=n; j++) sum = sum - a[i][j]*b[j];

        b[i] = sum/a[i][i];
    }
    //
    return true;
}


bool XFoil::hct(double hk, double msq, double &hc, double &hc_hk, double &hc_msq)
{
    //---- density shape parameter    (from whitfield)
    hc     = msq * (0.064/(hk-0.8) + 0.251);
    hc_hk  = msq * (-.064/(hk-0.8)/(hk-0.8));
    hc_msq =        0.064/(hk-0.8) + 0.251;

    return true;
}



/**
 *      Changes buffer airfoil
 *      thickness and/or camber highpoint
 */
void XFoil::hipnt(double chpnt, double thpnt)
{
    double ybl(0), cxmax(0), cymax(0), txmax(0), tymax(0);
    double ycc(0),ytt(0);
    double arot(0), sbl(0);
    double xfn[5], yfn[5], yfnp[5] ;//sfn[5]
    double xcm[IQX], ycm[IQX], xtk[IQX], ytk[IQX], ycmp[IQX], ytkp[IQX];
    int ncm(0), ntk(0);

    //--- check chordline direction (should be unrotated for camber routines)
    //    to function correctly
    xle = seval(sble,xb,xbp,sb,nb);
    yle = seval(sble,yb,ybp,sb,nb);
    xte = 0.5*(xb[1]+xb[nb]);
    yte = 0.5*(yb[1]+yb[nb]);
    arot = atan2(yle-yte,xte-xle) / dtor;
    if(fabs(arot)>1.0)
    {
        std::stringstream ss;
        ss << "Warning: High does not work well on rotated foils\n";
        ss << "Current chordline angle: "<<arot<<"\n";
        ss << "Proceeding anyway...\n";
        writeString(ss.str(), true);
    }

    //---- find leftmost point location
    xlfind(sbl,xb,xbp,yb,ybp,sb,nb);
    //    xbl = seval(sbl,xb,xbp,sb,nb);
    ybl = seval(sbl,yb,ybp,sb,nb);

    //---- find the current buffer airfoil camber and thickness
    getcam(xcm,ycm,ncm,xtk,ytk,ntk, xb,xbp,yb,ybp,sb,nb );

    //---- find the max thickness and camber
    getmax(xcm,ycm,ycmp,ncm,cxmax,cymax);
    getmax(xtk,ytk,ytkp,ntk,txmax,tymax);

    //---- make a picture and get some input specs for mods
    //      write(*,1010) 2.0*tymax,txmax, cymax,cxmax
    // 1010 format(/' max thickness = ',f8.4,'  at x = ',f7.3,
    //     &       /' max camber    = ',f8.4,'  at x = ',f7.3/)

    /*      if  (ninput .ge. 2) then
        thpnt = rinput(1)
        chpnt = rinput(2)
      elseif(ninput .ge. 1) then
        thpnt = rinput(1)
        if(lgsym) then
         write(*,*) 'symmetry enforced:  maintaining zero camber.'
        else
         chpnt = 0.0
         call askr('enter new camber    highpoint x: ^',chpnt)
        endif
      else
        thpnt = 0.0
        call askr('enter new thickness highpoint x: ^',thpnt)
        if(lgsym) then
         write(*,*) 'symmetry enforced:  maintaining zero camber.'
        else
         chpnt = 0.0
         call askr('enter new camber    highpoint x: ^',chpnt)
        endif
      endif

      if (thpnt<=0.0) thpnt = txmax;
      if (chpnt<=0.0) chpnt = cxmax;*/
    //
    //--- a simple cubic mapping function is used to map x/c to move highpoints
    //
    //    the assumption is that a smooth function (cubic, given by the old and
    //    new highpoint locations) maps the range 0-1 for x/c
    //    into the range 0-1 for altered x/c distribution for the same y/c
    //    thickness or camber (ie. slide the points smoothly along the x axis)
    //
    //--- shift thickness highpoint
    if (thpnt > 0.0) {
        xfn[1] = xtk[1];
        xfn[2] = txmax;
        xfn[3] = xtk[ntk];
        yfn[1] = xtk[1];
        yfn[2] = thpnt;
        yfn[3] = xtk[ntk];
        splina(yfn,yfnp,xfn,3);
        for (int i = 1; i<=ntk; i++)
            xtk[i] = seval(xtk[i],yfn,yfnp,xfn,3);
    }

    //--- shift camber highpoint
    if (chpnt > 0.0) {
        xfn[1] = xcm[1];
        xfn[2] = cxmax;
        xfn[3] = xcm[ncm];
        yfn[1] = xcm[1];
        yfn[2] = chpnt;
        yfn[3] = xcm[ncm];
        splina(yfn,yfnp,xfn,3);
        for (int i = 1;i <= ncm; i++)
            xcm[i] = seval(xcm[i],yfn,yfnp,xfn,3);
    }

    //---- make new airfoil from thickness and camber
    //     new airfoil points are spaced to match the original
    //--- hhy 4/24/01 got rid of splining vs x,y vs s (buggy), now spline y(x)
    segspl(ytk,ytkp,xtk,ntk);
    segspl(ycm,ycmp,xcm,ncm);


    //---- for each orig. airfoil point setup new yb from camber and thickness
    for (int i=1; i<=nb;i++){

        //------ spline camber and thickness at original xb points
        ycc = seval(xb[i],ycm,ycmp,xcm,ncm);
        ytt = seval(xb[i],ytk,ytkp,xtk,ntk);

        //------ set new y coordinate from new camber & thickness
        if (sb[i] <= sbl) yb[i] = ycc + ytt;
        else  yb[i] = ycc - ytt;

        //---- add y-offset for original leftmost (le) point to camber
        yb[i] = yb[i] + ybl;
    }

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,
           radble,angbte,ei11ba,ei22ba,apx1ba,apx2ba,
           ei11bt,ei22bt,apx1bt,apx2bt);
}

bool XFoil::hkin(double h, double msq, double &hk, double &hk_h, double &hk_msq){
    //---- calculate kinematic shape parameter (assuming air)
    //     (from Whitfield )
    hk     =    (h - 0.29*msq)   /(1.0 + 0.113*msq);
    hk_h   =     1.0              /(1.0 + 0.113*msq);
    hk_msq = (-.29 - 0.113*(hk))/(1.0 + 0.113*msq);

    return true;
}


bool XFoil::hsl(double hk, double &hs, double &hs_hk, double &hs_rt, double &hs_msq)
{
    //---- laminar hs correlation
    double tmp(0);
    if(hk<4.35)
    {
        tmp = hk - 4.35;
        hs    = 0.0111*tmp*tmp/(hk+1.0)
                - 0.0278*tmp*tmp*tmp/(hk+1.0)  + 1.528
                - 0.0002*(tmp*hk)*(tmp*hk);
        hs_hk = 0.0111*(2.0*tmp    - tmp*tmp/(hk+1.0))/(hk+1.0)
                - 0.0278*(3.0*tmp*tmp - tmp*tmp*tmp/(hk+1.0))/(hk+1.0)
                - 0.0002*2.0*tmp*hk * (tmp + hk);
    }
    else
    {
        hs    = 0.015*    (hk-4.35)*(hk-4.35)/hk + 1.528;
        hs_hk = 0.015*2.0*(hk-4.35)   /hk
                - 0.015*    (hk-4.35)* (hk-4.35)/hk/hk;
    }

    hs_rt  = 0.0;
    hs_msq = 0.0;

    return true;
}



bool XFoil::hst(double hk, double rt, double msq, double &hs, double &hs_hk, double &hs_rt, double &hs_msq)
{
    double hsmin(0), dhsinf(0),  rtz(0), rtz_rt(0), ho(0), ho_rt(0), hr(0), hr_hk(0), hr_rt(0), fm(0);
    double grt(0), hdif(0), rtmp(0), htmp(0), htmp_hk(0), htmp_rt(0);

    //---- turbulent hs correlation

    hsmin = 1.5;
    dhsinf = 0.015;

    //---- ###  12/4/94
    //---- limited rtheta dependence for rtheta < 200

    if(rt>400.0)
    {
        ho    = 3.0 + 400.0/rt;
        ho_rt =      - 400.0/rt/rt;
    }
    else
    {
        ho    = 4.0;
        ho_rt = 0.0;
    }

    if(rt>200.0)
    {
        rtz    = rt;
        rtz_rt = 1.0;
    }
    else
    {
        rtz    = 200.0;
        rtz_rt = 0.0;
    }

    if(hk<ho)
    {
        //----- attached branch
        //----- new correlation  29 nov 91
        //-     (from  arctan(y+) + schlichting  profiles)
        hr    =   (ho - hk)/(ho-1.0);
        hr_hk =      - 1.0/(ho-1.0);
        hr_rt = (1.0 - hr)/(ho-1.0) * ho_rt;
        hs    = (2.0-hsmin-4.0/rtz)*hr*hr  * 1.5/(hk+0.5) + hsmin
                + 4.0/rtz;
        hs_hk =-(2.0-hsmin-4.0/rtz)*hr*hr  * 1.5/(hk+0.5)/(hk+0.5)
                + (2.0-hsmin-4.0/rtz)*hr*2.0 * 1.5/(hk+0.5) * hr_hk;
        hs_rt = (2.0-hsmin-4.0/rtz)*hr*2.0 * 1.5/(hk+0.5) * hr_rt
                + (hr*hr * 1.5/(hk+0.5) - 1.0)*4.0/rtz/rtz * rtz_rt;
    }
    else
    {

        //----- separated branch
        grt = log(rtz);
        hdif = hk - ho ;
        rtmp = hk - ho + 4.0/grt;
        htmp    = 0.007*grt/rtmp/rtmp + dhsinf/hk;
        htmp_hk = -.014*grt/rtmp/rtmp/rtmp - dhsinf/hk/hk;
        htmp_rt = -.014*grt/rtmp/rtmp/rtmp * (-ho_rt - 4.0/grt/grt/rtz * rtz_rt) + 0.007  /rtmp/rtmp / rtz * rtz_rt;
        hs    = hdif*hdif * htmp + hsmin + 4.0/rtz;
        hs_hk = hdif*2.0* htmp + hdif*hdif * htmp_hk;
        hs_rt = hdif*hdif * htmp_rt      - 4.0/rtz/rtz * rtz_rt + hdif*2.0* htmp * (-ho_rt);

    }

    //---- whitfield's minor additional compressibility correction
    fm = 1.0 + 0.014*msq;
    hs     = ( hs + 0.028*msq) / fm;
    hs_hk  = ( hs_hk          ) / fm;
    hs_rt  = ( hs_rt          ) / fm;
    hs_msq = 0.028/fm - 0.014*(hs)/fm;

    return true;
}


/** -----------------------------------------------------------
 *     sets  bl location -> panel location  pointer array ipan
 * -----------------------------------------------------------*/
bool XFoil::iblpan()
{
    int iblmax(0), is(0), ibl(0), i(0), iw(0);

    //-- top surface first
    is = 1;

    ibl = 1;
    for(i=ist; i>= 1;i--)
    {
        ibl = ibl+1;
        ipan[ibl][is] = i;
        vti[ibl][is] = 1.0;
    }

    iblte[is] = ibl;
    nbl[is] = ibl;

    //-- bottom surface next
    is = 2;
    ibl = 1;
    for(i=ist+1; i<=n; i++)
    {
        ibl = ibl+1;
        ipan[ibl][is] = i;
        vti[ibl][is] = -1.0;
    }

    //-- wake
    iblte[is] = ibl;

    for(iw=1; iw<=nw; iw++)
    {

        i = n+iw;
        ibl = iblte[is]+iw;
        ipan[ibl][is] = i;
        vti[ibl][is] = -1.0;
    }

    nbl[is] = iblte[is] + nw;

    //-- upper wake pointers (for plotting only)
    for(iw=1; iw<=nw; iw++){
        ipan[iblte[1]+iw][1] = ipan[iblte[2]+iw][2];
        vti[iblte[1]+iw][1] = 1.0;
    }
    iblmax = std::max(iblte[1],iblte[2]) + nw;
    if(iblmax>IVX)
    {
        std::string str("iblpan :  ***  bl array overflow");
        writeString(str, true);

        std::stringstream ss;
        ss << "Increase IVX to at least " << iblmax << "\n";
        writeString(ss.str(), true);
        return false;
    }

    lipan = true;
    return true;
}


/** ---------------------------------------------
 *     sets the bl newton system line number
 *     corresponding to each bl station.
 * --------------------------------------------- */
bool XFoil::iblsys()
{
    int iv(0), is(0), ibl(0);

    iv = 0;
    for (is=1; is<=2;is++)
    {
        for (ibl=2; ibl<= nbl[is]; ibl++)
        {
            iv = iv+1;
            isys[ibl][is] = iv;
        }
    }

    nsys = iv;
    if(nsys>2*IVX)
    {
        std::string str("*** iblsys: bl system array overflow. ***");
        writeString(str, true);
        return false;
    }

    return true;
}


/** Loads the Foil's geometry in XFoil,
 *  calculates the normal vectors,
 *  and sets the results in current foil */
bool XFoil::initXFoilGeometry(int fn, double const *fx, double const *fy, double*fnx, double*fny, bool bFLap, double xhinge, double yhinge)
{
    for (int i=0; i<fn; i++)
    {
        xb[i+1] = fx[i];
        yb[i+1] = fy[i];
    }

    nb = fn;
    lflap  = bFLap;
    xof = xhinge;
    yof = yhinge;

    ddef = 0.0;
    xbf  = 1.0;
    ybf  = 0.0;

    lscini = false;
    lqspec = false;
    lvisc  = false;

    if(PreProcess())
    {
        CheckAngles();
        for (int k=0; k<n;k++)
        {
            fnx[k] = nx[k+1];
            fny[k] = ny[k+1];
        }
//        fn = n;
        return true;
    }
    else
    {
        std::string str = "Unrecognized foil format";
        writeString(str);
        return false;
    }
}


bool XFoil::initXFoilAnalysis(double Re, double alpha, double Mach, double NCrit, double XtrTop, double XtrBot,
                              int reType, int maType, bool bViscous)
{
    //Sets Analysis parameters in XFoil

    lblini = false;
    lipan = false;

    reinf1 = Re;
    alfa =alpha*PI/180.0;

    minf1  = Mach;
    retyp  = reType;
    matyp  = maType;
    lalfa = true;
    qinf  = 1.0;

    lvisc = bViscous;

    acrit      = NCrit;
    xstrip[1]  = XtrTop;
    xstrip[2]  = XtrBot;

    if (Mach > 0.000001)
    {
        if(!setMach())
        {
            std::string str = "... Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ";
            writeString(str);
            return false;
        }
    }

    return true;
}


/**     logical function inside(x,y,n, xf,yf)
 *      dimension x(n),y(n)
 *-------------------------------------
 *     returns true if point xf,yf
 *     is inside contour x(i),y(i).
 *------------------------------------- */
bool XFoil::inside(double x[], double y[], int n, double xf, double yf)
{
    int i(0), ip(0);
    double xb1(0), xb2(0), yb1(0), yb2(0), angle(0);

    //---- integ, ybrate subtended angle around airfoil perimeter, yb
    angle = 0.0;
    for(i=1; i<=n; i++)
    {
        ip = i+1;
        if(i==n) ip = 1;
        xb1 = x[i]  - xf;
        yb1 = y[i]  - yf;
        xb2 = x[ip] - xf;
        yb2 = y[ip] - yf;
        angle = angle + (xb1*yb2 - yb1*xb2) / sqrt((xb1*xb1 + yb1*yb1)*(xb2*xb2 + yb2*yb2));
    }

    //---- angle = 0 if xf,yf is outside, angle = +/- 2 pi  if xf,yf is inside
    return  (fabs(angle) > 1.0);
}



/** ------------------------------------------------------
 *     locates leading edge spline-parameter value sle
 *
 *     the defining condition is
 *
 *      (x-xte,y-yte) . (x',y') = 0     at  s = sle
 *
 *     i.e. the surface tangent is normal to the chord
 *     line connecting x(sle),y(sle) and the TE point.
//------------------------------------------------------ */
bool XFoil::lefind(double &sle, double x[], double xp[], double y[], double yp[], double s[], int n)
{
    int i(0), iter(0);
    double dseps(0), dxte(0), dyte(0), dx(0), dy(0), dotp(0), dxds(0), dyds(0), dxdd(0), dydd(0);
    double res(0), ress(0), dsle(0);
    double xchord(0), ychord(0);

    //---- convergence tolerance
    dseps = (s[n]-s[1]) * 0.00001;

    //---- set trailing edge point coordinates
    xte = 0.5*(x[1] + x[n]);
    yte = 0.5*(y[1] + y[n]);

    //---- get first guess for sle
    for (i=3; i<=n-2; i++)
    {
        dxte = x[i] - xte;
        dyte = y[i] - yte;
        dx = x[i+1] - x[i];
        dy = y[i+1] - y[i];
        dotp = dxte*dx + dyte*dy;
        if(dotp < 0.0) break;
    }

    sle = s[i];

    //---- check for sharp le case
    if(fabs(s[i]-s[i-1])<EPSILON) return false;

    //---- newton iteration to get exact sle value
    for (iter=1; iter<= 50; iter++)
    {
        xle  = seval(sle,x,xp,s,n);
        yle  = seval(sle,y,yp,s,n);
        dxds = deval(sle,x,xp,s,n);
        dyds = deval(sle,y,yp,s,n);
        dxdd = d2val(sle,x,xp,s,n);
        dydd = d2val(sle,y,yp,s,n);

        xchord = xle - xte;
        ychord = yle - yte;

        //------ drive dot product between chord line and le tangent to zero
        res  = xchord*dxds + ychord*dyds;
        ress = dxds  *dxds + dyds  *dyds + xchord*dxdd + ychord*dydd;

        //------ newton delta for sle
        dsle = -res/ress;

        dsle = std::max( dsle , -0.02*fabs(xchord+ychord) );
        dsle = std::min( dsle ,  0.02*fabs(xchord+ychord) );
        sle = sle + dsle;
        if(fabs(dsle) < dseps) return true;
    }

    sle = s[i];
    return true;
}


/**    *******************************************************
 *    *                                                     *
 *    *   factors a full nxn matrix into an lu form.        *
 *    *   subr. baksub can back-substitute it with some rhs.*
 *    *   assumes matrix is non-singular...                 *
 *    *    ...if it isn"t, a divide by zero will result.    *
 *    *                                                     *
 *    *   a is the matrix...                                *
 *    *     ...replaced with its lu factors.                *
 *    *                                                     *
 *    *                              mark drela  1988       *
 *    *******************************************************
*/

bool XFoil::ludcmp(int n, double a[IQX][IQX], int indx[IQX])
{
    //    bool bimaxok = false;
    int imax(0);//added techwinder
    int nvx=IQX;
    int i(0), j(0), k(0);
    double vv[IQX];
    double dum(0), sum(0), aamax(0);

    if(n>nvx)
    {
        std::string str("Stop ludcmp: array overflow. Increase nvx");
        writeString(str, true);
        return false;
    }

    for (i=1; i<=n; i++)
    {
        aamax = 0.0;
        for (j=1; j<=n; j++) aamax = std::max(fabs(a[i][j]), aamax);
        vv[i] = 1.0/aamax;
    }

    for(j=1; j<=n;j++)
    {
        for(i=1; i<=j-1; i++)
        {
            sum = a[i][j];
            for (k=1;k<= i-1;k++) sum = sum - a[i][k]*a[k][j];
            a[i][j] = sum;
        }

        aamax = 0.0;

        for (i=j; i<=n; i++){
            sum = a[i][j];
            for (k=1;k<= j-1;k++) sum = sum - a[i][k]*a[k][j];
            a[i][j] = sum ;
            dum = (vv[i]*fabs(sum));
            if(dum>=aamax){
                imax = i;
                aamax = dum;
                //                bimaxok = true;
            }
        }
        //        ASSERT(bimaxok);// to check imax has been initialized
        if(j!=imax) {
            for (k=1; k<= n; k++){
                dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k] = dum;
            }
            vv[imax] = vv[j];
        }

        indx[j] = imax;
        if(j!=n) {
            dum = 1.0/a[j][j];
            for(i=j+1; i<=n; i++) a[i][j] = a[i][j]*dum;
        }
    }
    return true;
}


/** ----------------------------------------------------
 *        calculates the hinge moment of the flap about
 *        (xof,yof) by integrating surface pressures.
 * ---------------------------------------------------- */
bool XFoil::mhinge()
{
    int i(0);
    double tops(0),bots(0),botp(0),botx(0),boty(0),frac(0),topp(0),topx(0),topy(0);
    double xmid(0),ymid(0),pmid(0);
    double dx(0),dy(0);

    if(!lflap)
    {
        getxyf(x, xp, y, yp, s, n, tops, bots, xof, yof);
        lflap = true;
    }
    else
    {
        //------ find top and bottom y at hinge x location
        tops = xof;
        bots = s[n] - xof;
        sinvrt(tops,xof,x,xp,s,n);
        sinvrt(bots,xof,x,xp,s,n);
    }

    topx = seval(tops,x,xp,s,n);
    topy = seval(tops,y,yp,s,n);
    botx = seval(bots,x,xp,s,n);
    boty = seval(bots,y,yp,s,n);

    hmom = 0.0;
    hfx  = 0.0;
    hfy  = 0.0;

    //---- integrate pressures on top and bottom sides of flap
    for (i=2; i<=n; i++)
    {
        if(s[i-1]>=tops && s[i]<=bots) continue;
        {
            dx = x[i] - x[i-1];
            dy = y[i] - y[i-1];
            xmid = 0.5*(x[i]+x[i-1]) - xof;
            ymid = 0.5*(y[i]+y[i-1]) - yof;

            if(lvisc) pmid = 0.5*(cpv[i] + cpv[i-1]);
            else      pmid = 0.5*(cpi[i] + cpi[i-1]);

            hmom = hmom + pmid*(xmid*dx + ymid*dy);
            hfx  = hfx  - pmid* dy;
            hfy  = hfy  + pmid* dx;
        }
    }

    //---- find s[i]..s[i-1] interval containing s=tops
    for (i=2; i<=n; i++)  {
        if(s[i]>tops) break;
    }

    //stop31
    //---- add on top surface chunk tops..s[i-1],  missed in the do 20 loop.
    dx = topx - x[i-1];
    dy = topy - y[i-1];
    xmid = 0.5*(topx+x[i-1]) - xof;
    ymid = 0.5*(topy+y[i-1]) - yof;
    if(fabs(s[i]-s[i-1])>EPSILON) frac = (tops-s[i-1])/(s[i]-s[i-1]);
    else                          frac = 0.0;

    if(lvisc) {
        topp = cpv[i]*frac + cpv[i-1]*(1.0-frac);
        pmid = 0.5*(topp+cpv[i-1]);
    }
    else{
        topp = cpi[i]*frac + cpi[i-1]*(1.0-frac);
        pmid = 0.5*(topp+cpi[i-1]);
    }
    hmom = hmom + pmid*(xmid*dx + ymid*dy);
    hfx  = hfx  - pmid* dy;
    hfy  = hfy  + pmid* dx;

    //---- add on inside flap surface contribution from hinge to top surface
    dx = xof - topx;
    dy = yof - topy;
    xmid = 0.5*(topx+xof) - xof;
    ymid = 0.5*(topy+yof) - yof;
    hmom = hmom + pmid*(xmid*dx + ymid*dy);
    hfx  = hfx  - pmid* dy;
    hfy  = hfy  + pmid* dx;

    //---- find s[i]..s[i-1] interval containing s=bots
    for (i=n; i>=2; i--){
        if(s[i-1]<bots) break;
    }

    //---- add on bottom surface chunk bots..s[i],    missed in the do 20 loop.
    dx = x[i] - botx;
    dy = y[i] - boty;
    xmid = 0.5*(botx+x[i]) - xof;
    ymid = 0.5*(boty+y[i]) - yof;
    if(fabs(s[i]-s[i-1])>EPSILON) frac = (bots-s[i-1])/(s[i]-s[i-1]);
    else                          frac = 0.0;

    if(lvisc) {
        botp = cpv[i]*frac + cpv[i-1]*(1.0-frac);
        pmid = 0.5*(botp+cpv[i]);
    }
    else{
        botp = cpi[i]*frac + cpi[i-1]*(1.0-frac);
        pmid = 0.5*(botp+cpi[i]);
    }
    hmom = hmom + pmid*(xmid*dx + ymid*dy);
    hfx  = hfx  - pmid* dy;
    hfy  = hfy  + pmid* dx;

    //---- add on inside flap surface contribution from hinge to bottom surface
    dx = botx - xof;
    dy = boty - yof;
    xmid = 0.5*(botx+xof) - xof;
    ymid = 0.5*(boty+yof) - yof;
    hmom = hmom + pmid*(xmid*dx + ymid*dy);
    hfx  = hfx  - pmid* dy;
    hfy  = hfy  + pmid* dx;

    //---- add on T.E. base thickness contribution
    dx = x[1] - x[n];
    dy = y[1] - y[n];
    xmid = 0.5*(x[1]+x[n]) - xof;
    ymid = 0.5*(y[1]+y[n]) - yof;
    if(lvisc) pmid = 0.5*(cpv[1]+cpv[n]);
    else      pmid = 0.5*(cpi[1]+cpi[n]);

    hmom = hmom + pmid*(xmid*dx + ymid*dy);
    hfx  = hfx  - pmid* dy;
    hfy  = hfy  + pmid* dx;

//    qDebug("Result xof=%9.3g  yof=%9.3g", xof, yof);
//    qDebug("hmom=%9.3g   hfx= %9.3g  hfy=%9.3g\n", hmom, hfx, hfy);

    return true;
}


/** ----------------------------------------------------
 *      marches the bls and wake in mixed mode using
 *      the current ue and hk.  the calculated ue
 *      and hk lie along a line quasi-normal to the
 *      natural ue-hk characteristic line of the
 *      current bl so that the goldstein or levy-lees
 *      singularity is never encountered.  continuous
 *      checking of transition onset is performed.
 * ----------------------------------------------------- */
bool XFoil::mrchdu()
{

    double vtmp[5][6], vztmp[5];
    memset(vtmp, 0, 30*sizeof(double));
    memset(vztmp, 0, 5*sizeof(double));
    double deps = 0.000005;
    int is(0), ibl(0), ibm(0), itrold(0), iw(0), itbl(0);//icom

    double senswt(0), thi, uei, dsi(0), cti(0), dswaki(0), ratlen(0);
    double sens(0), sennew(0), dummy(0), msq(0), thm(0), dsm(0), uem(0);
    double xsi(0), hklim(0), dsw(0);
    double ami(0), dte(0), cte(0), tte(0), ueref(0), hkref(0), dmax(0);

    //---- constant controlling how far hk is allowed to deviate
    //    from the specified value.
    senswt = 1000.0;
    sens = 0.0;
    sennew = 0.0;

    for (is=1; is<= 2;is++)
    {//2000

        //---- set forced transition arc length position
        xifset(is);

        //---- set leading edge pressure gradient parameter  x/u du/dx
        ibl = 2;
        xsi = xssi[ibl][is];
        uei = uedg[ibl][is];
        bule = 1.0;

        //---- old transition station
        itrold = itran[is];

        tran = false;
        turb = false;
        itran[is] = iblte[is];
        //---- march downstream
        for(ibl=2;ibl<= nbl[is];ibl++)
        {//1000
            ibm = ibl-1;

            simi = ibl==2;
            wake = ibl>iblte[is];

            //------ initialize current station to existing variables
            xsi = xssi[ibl][is];
            uei = uedg[ibl][is];
            thi = thet[ibl][is];
            dsi = dstr[ibl][is];

            //------ fixed bug   md 7 june 99
            if(ibl<itrold) {
                ami = ctau[ibl][is];// ami must be initialized
                cti = 0.03;
            }
            else{
                cti = ctau[ibl][is];
                if(cti<=0.0) cti = 0.03;
            }

            if(wake) {
                iw = ibl - iblte[is];
                dswaki = wgap[iw];
            }
            else dswaki = 0.0;


            if(ibl<=iblte[is]) dsi = std::max(dsi-dswaki,1.02000*thi) + dswaki;
            if(ibl>iblte[is]) dsi = std::max(dsi-dswaki,1.00005*thi) + dswaki;

            //------ newton iteration loop for current station

            for (itbl=1;itbl<= 25;itbl++){//100

                //-------- assemble 10x3 linearized system for dctau, dth, dds, due, dxi
                //         at the previous "1" station and the current "2" station
                //         (the "1" station coefficients will be ignored)

                blprv(xsi,ami,cti,thi,dsi,dswaki,uei);
                blkin();

                //-------- check for transition and set appropriate flags and things
                if((!simi) && (!turb)) {
                    trchek();
                    ami = ampl2;
                    if( tran) itran[is] = ibl;
                    if(!tran) itran[is] = ibl+2;
                }
                if(ibl==iblte[is]+1) {
                    tte = thet[iblte[1]][1] + thet[iblte[2]][2];
                    dte = dstr[iblte[1]][1] + dstr[iblte[2]][2] + ante;
                    cte = ( ctau[iblte[1]][1]*thet[iblte[1]][1]
                            + ctau[iblte[2]][2]*thet[iblte[2]][2] ) / tte;
                    tesys(cte,tte,dte);
                }
                else{
                    blsys();
                }

                //-------- set stuff at first iteration...
                if(itbl==1) {

                    //--------- set "baseline" ue and hk for forming  ue(hk)  relation
                    ueref = u2;
                    hkref = hk2;

                    //--------- if current point ibl was turbulent and is now laminar, then...
                    if(ibl<itran[is] && ibl>=itrold ) {
                        //---------- extrapolate baseline hk
                        uem = uedg[ibl-1][is];
                        dsm = dstr[ibl-1][is];
                        thm = thet[ibl-1][is];
                        msq = uem*uem*hstinv / (gm1bl*(1.0 - 0.5*uem*uem*hstinv));
                        hkin( dsm/thm, msq, hkref, dummy, dummy );
                    }

                    //--------- if current point ibl was laminar, then...
                    if(ibl<itrold) {
                        //---------- reinitialize or extrapolate ctau if it's now turbulent
                        if(tran) ctau[ibl][is] = 0.03;
                        if(turb) ctau[ibl][is] = ctau[ibl-1][is];
                        if(tran || turb) {
                            cti = ctau[ibl][is];
                            s2 = cti;
                        }
                    }
                }

                if(simi || ibl==iblte[is]+1) {
                    //--------- for similarity station or first wake point, prescribe ue
                    vs2[4][1] = 0.0;
                    vs2[4][2] = 0.0;
                    vs2[4][3] = 0.0;
                    vs2[4][4] = u2_uei;
                    vsrez[4] = ueref - u2;
                }
                else{
                    //******** calculate ue-hk characteristic slope
                    for (int k=1; k<= 4;k++){
                        vztmp[k] = vsrez[k];
                        for (int l=1;l<= 5;l++) vtmp[k][l] = vs2[k][l];
                    }

                    //--------- set unit dhk
                    vtmp[4][1] = 0.0;
                    vtmp[4][2] = hk2_t2;
                    vtmp[4][3] = hk2_d2;
                    vtmp[4][4] = hk2_u2*u2_uei;
                    vztmp[4]   = 1.0;

                    //--------- calculate due response
                    Gauss(4,vtmp,vztmp);

                    //--------- set  senswt * (normalized due/dhk)
                    sennew = senswt * vztmp[4] * hkref/ueref;
                    if(itbl<=5) sens = sennew;
                    else if(itbl<=15) sens = 0.5*(sens + sennew);


                    //--------- set prescribed ue-hk combination
                    vs2[4][1] = 0.0;
                    vs2[4][2] =  hk2_t2 * hkref;
                    vs2[4][3] =  hk2_d2 * hkref;
                    vs2[4][4] =( hk2_u2 * hkref  +  sens/ueref )*u2_uei;
                    vsrez[4]  = -(hkref*hkref)*(hk2 / hkref - 1.0)
                            - sens*(u2  / ueref - 1.0);

                }

                //-------- solve newton system for current "2" station
                Gauss(4,vs2,vsrez);

                //-------- determine max changes and underrelax if necessary
                dmax = std::max(fabs(vsrez[2]/thi), fabs(vsrez[3]/dsi)  );
                if(ibl>=itran[is]) dmax = std::max(dmax,fabs(vsrez[1]/(10.0*cti)));

                rlx = 1.0;
                if(dmax>0.3) rlx = 0.3/dmax;

                //-------- update as usual
                if(ibl<itran[is]) ami = ami + rlx*vsrez[1];
                if(ibl>=itran[is]) cti = cti + rlx*vsrez[1];
                thi = thi + rlx*vsrez[2];
                dsi = dsi + rlx*vsrez[3];
                uei = uei + rlx*vsrez[4];

                //-------- eliminate absurd transients
                if(ibl>=itran[is]) {
                    cti = std::min(cti , 0.30);
                    cti = std::max(cti , 0.0000001);
                }

                if(ibl<=iblte[is]) hklim = 1.02;
                else hklim = 1.00005;

                msq = uei*uei*hstinv / (gm1bl*(1.0 - 0.5*uei*uei*hstinv));
                dsw = dsi - dswaki;
                dslim(dsw,thi,msq,hklim);
                dsi = dsw + dswaki;

                if(dmax<=deps) goto stop110;
            }

            {
                std::stringstream ss;
                ss<< "     mrchdu: convergence failed at "<< ibl <<"  side "<< is <<" res="<< dmax<<"\n";
                writeString(ss.str(), true);
            }

            if (dmax<= 0.1) goto stop109;
            //------ the current unconverged solution might still be reasonable...

            if(dmax > 0.1)
            {
                //------- the current solution is garbage --> extrapolate values instead
                if(ibl>3)
                {
                    if(ibl<=iblte[is])
                    {
                        thi = thet[ibm][is] * sqrt(xssi[ibl][is]/xssi[ibm][is]);
                        dsi = dstr[ibm][is] * sqrt(xssi[ibl][is]/xssi[ibm][is]);
                        uei = uedg[ibm][is];
                    }
                    else{
                        if(ibl==iblte[is]+1)
                        {
                            cti = cte;
                            thi = tte;
                            dsi = dte;
                            uei = uedg[ibm][is];
                        }
                        else{
                            thi = thet[ibm][is];
                            ratlen = (xssi[ibl][is]-xssi[ibm][is]) / (10.0*dstr[ibm][is]);
                            dsi = (dstr[ibm][is] + thi*ratlen) / (1.0 + ratlen);
                            uei = uedg[ibm][is];
                        }
                    }
                    if(ibl==itran[is]) cti = 0.05;
                    if(ibl>itran[is]) cti = ctau[ibm][is];
                }

            }

stop109:
            blprv(xsi,ami,cti,thi,dsi,dswaki,uei);
            blkin();

            //------- check for transition and set appropriate flags and things
            if((!simi) && (!turb)) {
                trchek();
                ami = ampl2;
                if( tran) itran[is] = ibl;
                if(!tran) itran[is] = ibl+2;
            }

            //------- set all other extrapolated values for current station
            if(ibl<itran[is])   blvar(1);
            if(ibl>=itran[is])  blvar(2);
            if(wake)            blvar(3);

            if(ibl<itran[is])   blmid(1);
            if(ibl>=itran[is])  blmid(2);
            if(wake)            blmid(3);

            //------ pick up here after the newton iterations
stop110:
            sens = sennew;

            //------ store primary variables
            if(ibl<itran[is]) ctau[ibl][is] = ami;
            else ctau[ibl][is] = cti;
            thet[ibl][is] = thi;
            dstr[ibl][is] = dsi;
            uedg[ibl][is] = uei;
            mass[ibl][is] = dsi*uei;
            tau[ibl][is]  = 0.5*r2*u2*u2*cf2;
            dis[ibl][is]  =     r2*u2*u2*u2*di2*hs2*0.5;
            ctq[ibl][is]  = cq2;

            //------ set "1" variables to "2" variables for next streamwise station
            blprv(xsi,ami,cti,thi,dsi,dswaki,uei);
            blkin();

            stepbl();

            //------ turbulent intervals will follow transition interval or te
            if(tran || ibl==iblte[is]) {
                turb = true;

                //------- save transition location
                tforce[is] = trforc;
                xssitr[is] = xt;
            }

            tran = false;
            if(s_bCancel) return false;
        }//1000 continue
    }// 2000 continue
    return true;
}


/** ----------------------------------------------------
 *      marches the bls and wake in direct mode using
 *      the uedg array. if separation is encountered,
 *      a plausible value of hk extrapolated from
 *      upstream is prescribed instead.  continuous
 *      checking of transition onset is performed.
 * ----------------------------------------------------*/
bool XFoil::mrchue()
{
    bool direct(false);
    int is(0), ibl(0), ibm(0), iw(0), itbl(0);
    double msq(0), ratlen(0),dsw(0),hklim;
    double hlmax(0), htmax(0), xsi(0), uei(0), ucon(0), tsq(0), thi(0), ami(0), cti(0), dsi(0);
    double dswaki(0);
    double htest(0), hktest(0), dummy(0);
    double cst(0);
    double cte(0), dte(0), tte(0), dmax(0), hmax(0), htarg(0);
    cte = dte = tte = dmax = hmax = htarg = 0.0;


    //---- shape parameters for separation criteria
    hlmax = 3.8;
    htmax = 2.5;

    for (is=1;is<= 2;is++)
    {//2000

        std::stringstream ss;
        ss << "    Side "<<is<<" ...\n";
        writeString(ss.str());

        //---- set forced transition arc length position
        xifset(is);

        //---- initialize similarity station with thwaites' formula
        //    ibl = 2;
        xsi = xssi[2][is];
        uei = uedg[2][is];


        //      bule = log(uedg(ibl+1,is)/uei) / log(xssi(ibl+1,is)/xsi)
        //      bule = std::max( -.08 , bule )
        bule = 1.0;
        ucon = uei/pow(xsi,bule);
        tsq = 0.45/(ucon*(5.0*bule+1.0)*reybl) * pow(xsi,(1.0-bule));
        thi = sqrt(tsq);
        dsi = 2.2*thi;
        ami = 0.0;

        //---- initialize ctau for first turbulent station
        cti = 0.03;

        tran = false;
        turb = false;
        itran[is] = iblte[is];

        //---- march downstream
        for (ibl=2; ibl<=nbl[is];ibl++)
        {// 1000
            ibm = ibl-1;
            iw = ibl - iblte[is];
            simi = (ibl==2);
            wake = ibl>iblte[is];

            //------ prescribed quantities
            xsi = xssi[ibl][is];
            uei = uedg[ibl][is];


            if(wake)
            {
                iw = ibl - iblte[is];
                dswaki = wgap[iw];
            }
            else dswaki = 0.0;


            direct = true;

            //------ newton iteration loop for current station
            for (itbl=1; itbl<= 25;itbl++){//100

                //-------- assemble 10x3 linearized system for dctau, dth, dds, due, dxi
                //         at the previous "1" station and the current "2" station
                //         (the "1" station coefficients will be ignored)

                blprv(xsi,ami,cti,thi,dsi,dswaki,uei);
                blkin();

                //-------- check for transition and set appropriate flags and things
                if((!simi) && (!turb)) {
                    trchek();
                    ami = ampl2;

                    //--------- fixed bug   md 7 jun 99
                    if(tran) {
                        itran[is] = ibl;
                        if(cti<=0.0) {
                            cti = 0.03;
                            s2 = cti;
                        }
                    }
                    else  itran[is] = ibl+2;
                }

                if(ibl==iblte[is]+1) {
                    tte = thet[iblte[1]][1] + thet[iblte[2]][2];
                    dte = dstr[iblte[1]][1] + dstr[iblte[2]][2] + ante;
                    cte = ( ctau[iblte[1]][1]*thet[iblte[1]][1]
                            + ctau[iblte[2]][2]*thet[iblte[2]][2] ) / tte;
                    tesys(cte,tte,dte);
                }
                else
                    blsys();

                if(direct)
                {
                    //--------- try direct mode (set due = 0 in currently empty 4th line)
                    vs2[4][1] = 0.0;
                    vs2[4][2] = 0.0;
                    vs2[4][3] = 0.0;
                    vs2[4][4] = 1.0;
                    vsrez[4] = 0.0;
                    //--------- solve newton system for current "2" station
                    Gauss(4,vs2,vsrez);
                    //--------- determine max changes and underrelax if necessary
                    dmax = std::max( fabs(vsrez[2]/thi), fabs(vsrez[3]/dsi) );
                    if(ibl<itran[is]) dmax = std::max(dmax,fabs(vsrez[1]/10.0));
                    if(ibl>=itran[is]) dmax = std::max(dmax,fabs(vsrez[1]/cti ));

                    rlx = 1.0;
                    if(dmax>0.3) rlx = 0.3/dmax;
                    //--------- see if direct mode is not applicable
                    if(ibl != iblte[is]+1) {
                        //---------- calculate resulting kinematic shape parameter hk
                        msq = uei*uei*hstinv / (gm1bl*(1.0 - 0.5*uei*uei*hstinv));
                        htest = (dsi + rlx*vsrez[3]) / (thi + rlx*vsrez[2]);
                        hkin(htest, msq, hktest, dummy, dummy);

                        //---------- decide whether to do direct or inverse problem based on hk
                        if(ibl<itran[is]) hmax = hlmax;
                        if(ibl>=itran[is]) hmax = htmax;
                        direct = (hktest<hmax);
                    }
                    if(direct)
                    {
                        //---------- update as usual
                        if(ibl>=itran[is])     cti = cti + rlx*vsrez[1];
                        thi = thi + rlx*vsrez[2];
                        dsi = dsi + rlx*vsrez[3];
                    }
                    else
                    {
                        //---------- set prescribed hk for inverse calculation at the current station
                        if(ibl<itran[is])
                            //----------- laminar case: relatively slow increase in hk downstream
                            htarg = hk1 + 0.03*(x2-x1)/theta1;
                        else if(ibl==itran[is]) {
                            //----------- transition interval: weighted laminar and turbulent case
                            htarg = hk1 + (0.03*(xt-x1) - 0.15*(x2-xt))/theta1;
                        }
                        else if(wake)
                        {
                            //----------- turbulent wake case:
                            //--          asymptotic wake behavior with approximate backward euler
                            cst = 0.03*(x2-x1)/theta1;
                            hk2 = hk1;
                            hk2 = hk2 - (hk2 +     cst*(hk2-1.0)*(hk2-1.0)*(hk2-1.0) - hk1)
                                    /(1.0 + 3.0*cst*(hk2-1.0)*(hk2-1.0));
                            hk2 = hk2 - (hk2 +     cst*(hk2-1.0)*(hk2-1.0)*(hk2-1.0) - hk1)
                                    /(1.0 + 3.0*cst*(hk2-1.0)*(hk2-1.0));
                            hk2 = hk2 - (hk2 +     cst*(hk2-1.0)*(hk2-1.0)*(hk2-1.0) - hk1)
                                    /(1.0 + 3.0*cst*(hk2-1.0)*(hk2-1.0));
                            htarg = hk2;
                        }
                        else htarg = hk1 - 0.15*(x2-x1)/theta1;//----------- turbulent case: relatively fast decrease in hk downstream

                        //---------- limit specified hk to something reasonable
                        if(wake) htarg = std::max(htarg , 1.01);
                        else htarg = std::max(htarg , hmax);

                        {
                            std::stringstream ss;
                            ss<< "     mrchue: inverse mode at "<<ibl<<"    hk = "<<htarg <<"\n";
                            writeString(ss.str());
                        }


                        //---------- try again with prescribed hk

                        goto stop100;
                    }
                }
                else
                {
                    //-------- inverse mode (force hk to prescribed value htarg)
                    vs2[4][1] = 0.0;
                    vs2[4][2] = hk2_t2;
                    vs2[4][3] = hk2_d2;
                    vs2[4][4] = hk2_u2;
                    vsrez[4] = htarg - hk2;
                    Gauss(4,vs2,vsrez);

                    dmax = std::max( fabs(vsrez[2]/thi),fabs(vsrez[3]/dsi)  );
                    if(ibl>=itran[is]) dmax = std::max( dmax , fabs(vsrez[1]/cti));
                    rlx = 1.0;
                    if(dmax>0.3) rlx = 0.3/dmax;
                    //--------- update variables
                    if(ibl>=itran[is]) cti = cti + rlx*vsrez[1];
                    thi = thi + rlx*vsrez[2];
                    dsi = dsi + rlx*vsrez[3];
                    uei = uei + rlx*vsrez[4];

                }
                //-------- eliminate absurd transients

                if(ibl>=itran[is]) {
                    cti = std::min(cti, 0.30);
                    cti = std::max(cti, 0.0000001);
                }
                if(ibl<=iblte[is])  hklim = 1.02;
                else hklim = 1.00005;
                msq = uei*uei*hstinv / (gm1bl*(1.0 - 0.5*uei*uei*hstinv));
                dsw = dsi - dswaki;
                dslim(dsw,thi,msq,hklim);
                dsi = dsw + dswaki;
                if(dmax<=0.00001) goto stop110;
stop100:
                int nothing=1;      (void)nothing;    //c++ doesn(t like gotos
            }//end itbl loop

            {
                std::stringstream ss;
                ss<< "     mrchue: convergence failed at "<<ibl<< " side "<<is<<", res = " << dmax <<"\n";
                writeString(ss.str());
            }

            //------ the current unconverged solution might still be reasonable...
            if(dmax > 0.1)
            {

                //------- the current solution is garbage --> extrapolate values instead
                if(ibl>3) {
                    if(ibl<=iblte[is]) {
                        thi = thet[ibm][is] * sqrt(xssi[ibl][is]/xssi[ibm][is]);
                        dsi = dstr[ibm][is] * sqrt(xssi[ibl][is]/xssi[ibm][is]);
                    }
                    else{
                        if(ibl==iblte[is]+1) {
                            cti = cte;
                            thi = tte;
                            dsi = dte;
                        }
                        else{
                            thi = thet[ibm][is];
                            ratlen = (xssi[ibl][is]-xssi[ibm][is]) / (10.0*dstr[ibm][is]);
                            dsi = (dstr[ibm][is] + thi*ratlen) / (1.0 + ratlen);
                        }
                    }
                    if(ibl==itran[is]) cti = 0.05;
                    if(ibl>itran[is]) cti = ctau[ibm][is];

                    uei = uedg[ibl][is];

                    if(ibl>2 && ibl<nbl[is]) uei = 0.5*(uedg[ibl-1][is] + uedg[ibl+1][is]);
                }
            }
            //109
            blprv(xsi,ami,cti,thi,dsi,dswaki,uei);
            blkin();
            //------- check for transition and set appropriate flags and things
            if((!simi) && (!turb)) {
                trchek();
                ami = ampl2;
                if(      tran) itran[is] = ibl;
                if(!tran) itran[is] = ibl+2;
            }
            //------- set all other extrapolated values for current station
            if(ibl<itran[is])  blvar(1);
            if(ibl>=itran[is])  blvar(2);
            if(wake)  blvar(3);
            if(ibl<itran[is])  blmid(1);
            if(ibl>=itran[is])  blmid(2);
            if(wake)  blmid(3);
            //------ pick up here after the newton iterations
stop110:
            //------ store primary variables
            if(ibl<itran[is]) ctau[ibl][is] = ami;
            if(ibl>=itran[is]) ctau[ibl][is] = cti;
            thet[ibl][is] = thi;
            dstr[ibl][is] = dsi;
            uedg[ibl][is] = uei;
            mass[ibl][is] = dsi*uei;
            tau[ibl][is]  = 0.5*r2*u2*u2*cf2;
            dis[ibl][is]  =     r2*u2*u2*u2*di2*hs2*0.5;
            ctq[ibl][is]  = cq2;
            delt[ibl][is] = de2;

            //------ set "1" variables to "2" variables for next streamwise station
            blprv(xsi,ami,cti,thi,dsi,dswaki,uei);
            blkin();

            stepbl();


            //------ turbulent intervals will follow transition interval or te
            if(tran || ibl==iblte[is]) {
                turb = true;

                //------- save transition location
                tforce[is] = trforc;
                xssitr[is] = xt;
            }

            tran = false;

            if(ibl==iblte[is]) {
                thi = thet[iblte[1]][1] + thet[iblte[2]][2];
                dsi = dstr[iblte[1]][1] + dstr[iblte[2]][2] + ante;
            }
        }// 1000 continue : end ibl loop
    }// 2000 continue : end is loop
    return true;
}



/** -------------------------------------------
 *      sets actual mach, reynolds numbers
 *      from unit-cl values and specified cls
 *      depending on matyp,retyp flags.
 * -------------------------------------------- */
bool XFoil::mrcl(double cls, double &m_cls, double &r_cls)
{
    double rrat(0), cla(0);

    cla = std::max(cls, 0.000001);
    if(retyp<1 || retyp>3)
    {
        std::string str("    mrcl:  illegal Re(cls) dependence trigger, Setting fixed Re ");
        writeString(str, true);
        retyp = 1;
    }
    if(matyp<1 || matyp>3)
    {
        std::string str("    mrcl:  illegal Mach(cls) dependence trigger\n Setting fixed Mach");
        writeString(str, true);
        matyp = 1;
    }

    switch(matyp)
    {
        case 1:
        {
            minf  = minf1;
            m_cls = 0.0;
            break;
        }
        case 2:
        {
            minf  =  minf1/sqrt(cla);
            m_cls = -0.5*minf/cla;
            break;
        }
        case 3:
        {
            minf  = minf1;
            m_cls = 0.0;
            break;
        }
    }

    switch(retyp)
    {
        case 1:
        {
            reinf = reinf1;
            r_cls = 0.0;
            break;
        }
        case 2:
        {
            reinf =  reinf1/sqrt(cla);
            r_cls = -0.5*reinf/cla;
            break;
        }
        case 3:
        {
            reinf =  reinf1/cla;
            r_cls = -reinf /cla;
            break;
        }
    }

    if(minf >= 0.99)
    {
        //TRACE("      artificially limiting mach to  0.99\n");
        std::string str("mrcl: Cl too low for chosen Mach(Cl) dependence\n");
        writeString(str, true);
        str = "      artificially limiting mach to  0.99";
        writeString(str, true);
        minf = 0.99;
        m_cls = 0.0;
    }

    rrat = 1.0;
    if(reinf1 > 0.0) rrat = reinf/reinf1;

    if(rrat > 100.0)
    {
        //TRACE("     artificially limiting re to %f\n",reinf1*100.0);
        std::string str("mrcl: cl too low for chosen Re(Cl) dependence\n");
        writeString(str, true);

        std::stringstream ss;
        ss<< "      artificially limiting Re to "<< reinf1*100.0<<"\n";
        writeString(ss.str(), true);
        reinf = reinf1*100.0;
        r_cls = 0.0;
    }
    return true;
}


bool XFoil::ncalc(double x[], double y[], double s[], int n, double xn[], double yn[])
{
    double sx(0), sy(0), smod(0);
    int i(0);

    if(n<=1) return false;
    segspl(x,xn,s,n);
    segspl(y,yn,s,n);
    for (i=1; i<=n; i++)
    {
        sx =  yn[i];
        sy = -xn[i];
        smod = sqrt(sx*sx + sy*sy);
        xn[i] = sx/smod;
        yn[i] = sy/smod;
    }

    //---- average normal vectors at corner points
    for (i=1; i<=n-1; i++)
    {
        if(fabs(s[i]-s[i+1])<EPSILON)
        {
            sx = 0.5*(xn[i] + xn[i+1]);
            sy = 0.5*(yn[i] + yn[i+1]);
            smod = sqrt(sx*sx + sy*sy);
            xn[i]   = sx/smod;
            yn[i]   = sy/smod;
            xn[i+1] = sx/smod;
            yn[i+1] = sy/smod;
        }
    }

    return true;
}


/** ---------------------------------------------------
 *      Set paneling distribution from buffer airfoil
 *      geometry, thus creating current airfoil.
 *
 *      If refine=true, bunch points at x=xsref on
 *      top side and at x=xpref on bottom side
 *      by setting a fictitious local curvature of
 *      ctrrat*(LE curvature) there.
 * --------------------------------------------------- */
void XFoil::pangen()
{
    std::string str;
    int ipfac(0), ible(0), nk(0), nn(0), nfrac1(0), nn2(0), ind(0), ncorn(0),j(0);
    double sbref(0), cvle(0), xble(0), xbte(0), yble(0), ybte(0), chbsq(0), cvsum(0), cvte(0);
    double frac(0), sbk(0), cvk(0), cvavg(0), cc(0), smool(0), smoosq(0), dsm(0), dsp(0), dso(0);
    double xoc(0), cvmax(0), rdste(0), rtf(0), dsavg(0), dsavg1(0), dsavg2(0);
    double cv1(0), cv2(0), cv3(0), cvs1(0), cvs2(0), cvs3(0), cavm(0), cavm_s1(0), cavm_s2(0);
    double cavp(0), cavp_s2(0), cavp_s3(0), fm(0), fp(0), rez(0);
    double dmax(0), ds(0), dds(0), dsrat(0), xbcorn(0), ybcorn(0), sbcorn(0);
    double dsmin(0), dsmax(0);
    //    double gap;
    int i(0),k(0), nn1(0);
    int nothing(0);
    nn1 = 0;

    if(nb<2)
    {
        writeString("PanGen: buffer airfoil not available.");
        n = 0;
        return;
    }

    //---- number of temporary nodes for panel distribution calculation
    //       exceeds the specified panel number by factor of ipfac.
    ipfac = 3;

    //---- number of airfoil panel points
    n = npan;

    //---- number of wake points
    //      nw = npan/8 + 2
    //      if(nw>iwx) then
    //       write(*,*)
    //     &  'array size (iwx) too small.  last wake point index reduced.'
    //       nw = iwx
    //      endif
    //
    //---- set arc length spline parameter
    scalc(xb,yb,sb,nb);

    //---- spline raw airfoil coordinates
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    //---- normalizing length (~ chord)
    sbref = 0.5*(sb[nb]-sb[1]);

    //---- set up curvature array
    for(i = 1; i<=nb; i++)
        w5[i] = fabs(curv(sb[i],xb,xbp,yb,ybp,sb,nb)) * sbref;


    //---- locate LE point arc length value and the normalized curvature there
    lefind(sble,xb,xbp,yb,ybp,sb,nb);
    cvle = fabs(curv(sble,xb,xbp,yb,ybp,sb,nb)) * sbref;

    //---- check for doubled point (sharp corner) at LE
    ible = 0;
    for (i = 1; i<=nb-1; i++)
    {
        if(fabs(sble-sb[i])<EPSILON && fabs(sble-sb[i+1])<EPSILON)
        {
            ible = i;
            //TRACE("Sharp leading edge\n");
            //            std::string str;
            //            str.Format("Sharp leading edge\n");
            //            pXFile->WriteString(str);

            break;
        }
    }
    //stop21:

    //---- set LE, TE points
    xble = seval(sble,xb,xbp,sb,nb);
    yble = seval(sble,yb,ybp,sb,nb);
    xbte = 0.5*(xb[1]+xb[nb]);
    ybte = 0.5*(yb[1]+yb[nb]);
    chbsq = (xbte-xble)*(xbte-xble) + (ybte-yble)*(ybte-yble);

    //---- set average curvature over 2*nk+1 points within rcurv of LE point
    nk = 3;
    cvsum = 0.0;
    for (k = -nk; k<=nk; k++){
        frac = double(k)/double(nk);
        sbk = sble + frac*sbref/std::max(cvle,20.0);
        cvk = fabs(curv(sbk,xb,xbp,yb,ybp,sb,nb)) * sbref;
        cvsum = cvsum + cvk;
    }
    cvavg = cvsum/double(2*nk+1);

    //---- dummy curvature for sharp LE
    if(ible!=0) cvavg = 10.0;

    //---- set curvature attraction coefficient actually used
    cc = 6.0 * cvpar;

    //---- set artificial curvature at TE to bunch panels there
    cvte = cvavg * cterat;
    w5[1]  = cvte;
    w5[nb] = cvte;

    //**** smooth curvature array for smoother panel size distribution  ****


    //---- set smoothing length = 1 / averaged LE curvature, but
    //    no more than 5% of chord and no less than 1/4 average panel spacing
    smool = std::max(1.0/std::max(cvavg,20.0), 0.25/double(npan/2));

    smoosq = (smool*sbref) *(smool*sbref);

    //---- set up tri-diagonal system for smoothed curvatures
    w2[1] = 1.0;
    w3[1] = 0.0;
    for(i=2; i<=nb-1; i++)
    {
        dsm = sb[i] - sb[i-1];
        dsp = sb[i+1] - sb[i];
        dso = 0.5*(sb[i+1] - sb[i-1]);

        if(dsm==0.0 || dsp==0.0)
        {
            //------- leave curvature at corner point unchanged
            w1[i] = 0.0;
            w2[i] = 1.0;
            w3[i] = 0.0;
        }
        else{
            w1[i] =  smoosq * (          - 1.0/dsm) /dso;
            w2[i] =  smoosq * ( 1.0/dsp + 1.0/dsm) /dso  +  1.0;
            w3[i] =  smoosq * (-1.0/dsp          ) /dso;
        }
    }

    w1[nb] = 0.0;
    w2[nb] = 1.0;

    //---- fix curvature at LE point by modifying equations adjacent to LE
    for (i=2; i<=nb-1; i++)
    {
        if(fabs(sb[i]-sble)<EPSILON || i==ible || i==ible+1)
        {
            //------- if node falls right on LE point, fix curvature there
            w1[i] = 0.0;
            w2[i] = 1.0;
            w3[i] = 0.0;
            w5[i] = cvle;
        }
        else if(sb[i-1]<sble && sb[i]>sble)
        {
            //------- modify equation at node just before LE point
            dsm = sb[i-1] - sb[i-2];
            dsp = sble    - sb[i-1];
            dso = 0.5*(sble - sb[i-2]);

            w1[i-1] =  smoosq * (         - 1.0/dsm) /dso;
            w2[i-1] =  smoosq * (1.0/dsp + 1.0/dsm) /dso  +  1.0;
            w3[i-1] =  0.0;
            w5[i-1] = w5[i-1] + smoosq*cvle/(dsp*dso);

            //------- modify equation at node just after LE point
            dsm = sb[i] - sble;
            dsp = sb[i+1] - sb[i];
            dso = 0.5*(sb[i+1] - sble);
            w1[i] =  0.0;
            w2[i] =  smoosq * ( 1.0/dsp + 1.0/dsm) /dso  +  1.0;
            w3[i] =  smoosq * (-1.0/dsp           ) /dso;
            w5[i] = w5[i] + smoosq*cvle/(dsm*dso);

            goto stop51;
        }
    }
stop51:

    //---- set artificial curvature at bunching points and fix it there
    for (i=2; i<=nb-1; i++)
    {
        //------ chord-based x/c coordinate
        xoc = ((xb[i]-xble)*(xbte-xble) +  (yb[i]-yble)*(ybte-yble) ) / chbsq;

        if(sb[i]<sble)
        {
            //------- check if top side point is in refinement area
            if(xoc>xsref1 && xoc<xsref2)
            {
                w1[i] = 0.;
                w2[i] = 1.0;
                w3[i] = 0.;
                w5[i] = cvle*ctrrat;
            }
        }
        else
        {
            //------- check if bottom side point is in refinement area
            if(xoc>xpref1 && xoc<xpref2)
            {
                w1[i] = 0.;
                w2[i] = 1.0;
                w3[i] = 0.;
                w5[i] = cvle*ctrrat;
            }
        }
    }

    //---- solve for smoothed curvature array w5
    if(ible==0) trisol(w2,w1,w3,w5,nb);
    else
    {
        //        i = 1;
        //        trisol(w2[i],w1[i],w3[i],w5[i],ible);
        //        i = ible+1;
        //        trisol(w2[i],w1[i],w3[i],w5[i],nb-ible);
        i = 1;
        trisol(w2,w1,w3,w5,ible);
        i = ible+1;
        trisol(w2+i-1,w1+i-1,w3+i-1,w5+i-1,nb-ible);
    }

    //---- find max curvature
    cvmax = 0.;
    for( i=1; i<=nb; i++)
    {
        cvmax = std::max(cvmax, fabs(w5[i]));
    }

    //---- normalize curvature array
    for( i=1; i<=nb; i++)
    {
        w5[i] = w5[i] / cvmax;
    }

    //---- spline curvature array
    segspl(w5,w6,sb,nb);

    //---- set initial guess for node positions uniform in s.
    //     more nodes than specified (by factor of ipfac) are
    //     temporarily used  for more reliable convergence.
    nn = ipfac*(n-1)+1;

    //---- ratio of lengths of panel at te to one away from the te
    rdste = 0.667;
    rtf = (rdste-1.0)*double(ipfac) + 1.0;

    if(ible==0)
    {

        dsavg = (sb[nb]-sb[1])/(double(nn-3) + 2.0*rtf);
        snew[1] = sb[1];
        for (i=2;i<=nn-1; i++)
        {
            snew[i] = sb[1] + dsavg * (double(i-2) + rtf);
        }
        snew[nn] = sb[nb];
    }
    else
    {
        nfrac1 = (n * ible) / nb;

        nn1 = ipfac*(nfrac1-1)+1;
        dsavg1 = (sble-sb[1])/(double(nn1-2) + rtf);
        snew[1] = sb[1];
        for (i=2; i<=nn1; i++)
        {
            snew[i] = sb[1] + dsavg1 * (double(i-2) + rtf);
        }

        nn2 = nn - nn1 + 1;
        dsavg2 = (sb[nb]-sble)/(double(nn2-2) + rtf);
        for (i=2; i<=nn2-1; i++)
        {
            snew[i-1+nn1] = sble + dsavg2 * (double(i-2) + rtf);
        }
        snew[nn] = sb[nb];

    }

    //---- newton iteration loop for new node positions
    for (int iter=1; iter<= 20; iter++)
    {//iter 10

        //------ set up tri-diagonal system for node position deltas
        cv1  = seval(snew[1],w5,w6,sb,nb);
        cv2  = seval(snew[2],w5,w6,sb,nb);
        cvs1 = deval(snew[1],w5,w6,sb,nb);
        cvs2 = deval(snew[2],w5,w6,sb,nb);

        cavm = sqrt(cv1*cv1 + cv2*cv2);
        if(cavm == 0.0)
        {
            cavm_s1 = 0.0;
            cavm_s2 = 0.0;
        }
        else
        {
            cavm_s1 = cvs1 * cv1/cavm;
            cavm_s2 = cvs2 * cv2/cavm;
        }

        for(i=2; i<=nn-1; i++)
        {//110
            dsm = snew[i] - snew[i-1];
            dsp = snew[i] - snew[i+1];
            cv3  = seval(snew[i+1],w5,w6,sb,nb);
            cvs3 = deval(snew[i+1],w5,w6,sb,nb);

            cavp = sqrt(cv3*cv3 + cv2*cv2);
            if(cavp == 0.0) {
                cavp_s2 = 0.;
                cavp_s3 = 0.;
            }
            else{
                cavp_s2 = cvs2 * cv2/cavp;
                cavp_s3 = cvs3 * cv3/cavp;
            }

            fm = cc*cavm + 1.0;
            fp = cc*cavp + 1.0;

            rez = dsp*fp + dsm*fm;

            //-------- lower, main, and upper diagonals
            w1[i] =      -fm  +  cc*               dsm*cavm_s1;
            w2[i] =  fp + fm  +  cc*(dsp*cavp_s2 + dsm*cavm_s2);
            w3[i] = -fp       +  cc* dsp*cavp_s3;

            //-------- residual, requiring that
            //         (1 + c*curv)*deltas is equal on both sides of node i
            w4[i] = -rez;

            cv1 = cv2;
            cv2 = cv3;
            cvs1 = cvs2;
            cvs2 = cvs3;
            cavm    = cavp;
            cavm_s1 = cavp_s2;
            cavm_s2 = cavp_s3;
        }

        //------ fix endpoints (at te)
        w2[1] = 1.0;
        w3[1] = 0.0;
        w4[1] = 0.0;
        w1[nn] = 0.0;
        w2[nn] = 1.0;
        w4[nn] = 0.0;

        if(rtf != 1.0)
        {
            //------- fudge equations adjacent to te to get TE panel length ratio rtf

            i = 2;
            w4[i] = -((snew[i] - snew[i-1]) + rtf*(snew[i] - snew[i+1]));
            w1[i] = -1.0;
            w2[i] =  1.0 + rtf;
            w3[i] =       - rtf;

            i = nn-1;
            w4[i] = -((snew[i] - snew[i+1]) + rtf*(snew[i] - snew[i-1]));
            w3[i] = -1.0;
            w2[i] =  1.0 + rtf;
            w1[i] =       - rtf;
        }

        //------ fix sharp le point
        if(ible!=0)
        {
            i = nn1;
            w1[i] = 0.0;
            w2[i] = 1.0;
            w3[i] = 0.0;
            w4[i] = sble - snew[i];
        }

        //------ solve for changes w4 in node position arc length values
        trisol(w2,w1,w3,w4,nn);

        //------ find under-relaxation factor to keep nodes from changing order
        rlx = 1.0;
        dmax = 0.0;
        for( i=1; i<=nn-1; i++)
        {
            ds  = snew[i+1] - snew[i];
            dds = w4[i+1] - w4[i];
            dsrat = 1.0 + rlx*dds/ds;
            if(dsrat>4.0) rlx = (4.0-1.0)*ds/dds;
            if(dsrat<0.2) rlx = (0.2-1.0)*ds/dds;
            dmax = std::max(fabs(w4[i]),dmax);
        }

        //------ update node position
        for(i=2; i<=nn-1; i++){
            snew[i] = snew[i] + rlx*w4[i];
        }

        if(fabs(dmax)<0.001) goto stop11;

    }

    //TRACE("Paneling convergence failed.  Continuing anyway...\n");
    str = "Paneling convergence failed.  Continuing anyway...\n";
    writeString(str, true);

stop11:

    //---- set new panel node coordinates
    for(i=1; i<=n; i++)
    {
        ind = ipfac*(i-1) + 1;
        s[i] = snew[ind];
        x[i] = seval(snew[ind],xb,xbp,sb,nb);
        y[i] = seval(snew[ind],yb,ybp,sb,nb);
    }

    //---- go over buffer airfoil again, checking for corners (double points)
    ncorn = 0;
    for(int ib=1; ib<= nb-1; ib++)
    {//25
        if(fabs(sb[ib]-sb[ib+1])<EPSILON)
        {
            //------- found one !

            ncorn = ncorn+1;
            xbcorn = xb[ib];
            ybcorn = yb[ib];
            sbcorn = sb[ib];

            //------- find current-airfoil panel which contains corner
            for(i=1; i<=n ; i++)
            {//252

                //--------- keep stepping until first node past corner
                if(s[i] <= sbcorn) goto stop252;

                //---------- move remainder of panel nodes to make room for additional node
                for(j=n; j>=i; j--)
                {
                    x[j+1] = x[j];
                    y[j+1] = y[j];
                    s[j+1] = s[j];
                }
                n = n+1;

                if(n > IQX-1)
                {
                    //TRACE("panel: too many panels. increase iqx in xfoil.inc");
                    std::string str = "Panel: Too many panels. Increase IQX";
                    writeString(str, true);
                    return;
                }
                x[i] = xbcorn;
                y[i] = ybcorn;
                s[i] = sbcorn;

                //---------- shift nodes adjacent to corner to keep panel sizes comparable
                if(i-2 >= 1)
                {
                    s[i-1] = 0.5*(s[i] + s[i-2]);
                    x[i-1] = seval(s[i-1],xb,xbp,sb,nb);
                    y[i-1] = seval(s[i-1],yb,ybp,sb,nb);
                }

                if(i+2 <= n)
                {
                    s[i+1] = 0.5*(s[i] + s[i+2]);
                    x[i+1] = seval(s[i+1],xb,xbp,sb,nb);
                    y[i+1] = seval(s[i+1],yb,ybp,sb,nb);
                }

                //---------- go on to next input geometry point to check for corner
                goto stop25;
stop252:
                nothing = 0; (void)nothing;// C++ doesn't like gotos
            }
        }
stop25:
        nothing = 0; (void)nothing;// C++ doesn't like gotos
    }

    scalc(x,y,s,n);
    segspl(x,xp,s,n);
    segspl(y,yp,s,n);
    lefind(sle,x,xp,y,yp,s,n);

    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);
    chord  = sqrt((xte-xle)*(xte-xle) + (yte-yle)*(yte-yle));

    //---- calculate panel size ratios (user info)
    dsmin =  1000.0;
    dsmax = -1000.0;
    for(i=1; i<=n-1; i++)
    {
        ds = s[i+1]-s[i];
        if(ds != 0.0)
        {
            dsmin = std::min(dsmin,ds);
            dsmax = std::max(dsmax,ds);
        }
    }

    dsmin = dsmin*double(n-1)/s[n];
    dsmax = dsmax*double(n-1)/s[n];

    //---- set various flags for new airfoil
    lgamu = false;
    lwake = false;
    lqaij = false;
    ladij = false;
    lwdij = false;
    lipan = false;
    lblini = false;
    lvconv = false;

    if(lbflap)
    {
        xof = xbf;
        yof = ybf;
        lflap = true;
    }

    //---- determine if TE is blunt or sharp, calculate TE geometry parameters
    tecalc();

    //---- calculate normal vectors
    ncalc(x,y,s,n,nx,ny);

    //---- calculate panel angles for panel routines
    apcalc();

    if(sharp)
    {
        //        std::string str;
        //        str.Format("Sharp trailing edge\n");
        //        pXFile->WriteString(str);
        //TRACE("sharp trailing edge\n");
    }
    else
    {
        //        gap = sqrt((x[1]-x[n])*(x[1]-x[n]) + (y[1]-y[n])*(y[1]-y[n]));
        //TRACE("Blunt trailing edge.  Gap =%f", gap);
        //        std::string str;
        //        str.Format("Blunt trailing edge.  Gap =%f", gap);
        //        pXFile->WriteString(str);

    }

    //TRACE("paneling parameters used...\n");
    //TRACE("   Number of panel nodes %d\n", npan);
    //TRACE("   Panel bunching parameter  %.3\n", cvpar);
    //TRACE("   TE/LE panel density ratio %.3\n", cterat);
    //TRACE("   refined-area/le panel density ratio %3\n",ctrrat);
    //TRACE("   top side refined area x/c limits %.3 %3\n",xsref1, xsref2);
    //TRACE("   bottom side refined area x/c limits %.3 %3\n",xpref1, xpref2);


    return;
}




bool XFoil::PreProcess()
{
    //    double xble, yble, xbte, ybte;
    //    double xinl, xout, ybot, ytop;

    //---- calculate airfoil area assuming counterclockwise ordering
    if(nb<=2) return false;//added techwinder

    double area = 0.0;
    for (int i=1; i<=nb; i++)
    {
        int ip = i+1;
        if(i==nb) ip = 1;
        area = area + 0.5*(yb[i]+yb[ip])*(xb[i]-xb[ip]);
    }

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    //    segspl(yb,ybp,sb,nb);
    geopar(xb,xbp,yb,ybp,sb,nb, w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);

    //    xble = seval(sble,xb,xbp,sb,nb);
    //    yble = seval(sble,yb,ybp,sb,nb);
    //    xbte = 0.5*(xb[1] + xb[nb]);
    //    ybte = 0.5*(yb[1] + yb[nb]);
    //TRACE(" le_x = %f, le_y = %f, chord=%f\n te_x = %f,  te_y = %f\n", xble,yble, chordb,xbte, ybte);

    //---- set reasonable mses domain parameters for non-mses coordinate file

    //    xble = seval(sble,xb,xbp,sb,nb);
    //    yble = seval(sble,yb,ybp,sb,nb);

    //    What's the use?
    /*        xinl = xble - 2.0*chordb;
        xout = xble + 3.0*chordb;
        ybot = yble - 2.5*chordb;
        ytop = yble + 3.5*chordb;
        xinl = aint(20.0*fabs(xinl/chordb)+0.5)/20.0 * sign(chordb,xinl);
        xout = aint(20.0*fabs(xout/chordb)+0.5)/20.0 * sign(chordb,xout);
        ybot = aint(20.0*fabs(ybot/chordb)+0.5)/20.0 * sign(chordb,ybot);
        ytop = aint(20.0*fabs(ytop/chordb)+0.5)/20.0 * sign(chordb,ytop);*/


    //---- wipe out old flap hinge location
    xbf = 0.0;
    ybf = 0.0;
    lbflap = false;

    // end "load"
    return abcopy();
}



/** -----------------------------------------------------------------------
 *       Calculates current streamfunction psi at panel node or wake node
 *       i due to freestream and all bound vorticity gam on the airfoil.
 *       Sensitivities of psi with respect to alpha (z_alfa) and inverse
 *       qspec dofs (z_qdof0,z_qdof1) which influence gam in inverse cases.
 *       Also calculates the sensitivity vector dpsi/dgam (dzdg).
 *
 *       If siglin=true, then psi includes the effects of the viscous
 *       source distribution sig and the sensitivity vector dpsi/dsig
 *       (dzdm) is calculated.
 *
 *       If geolin=true, then the geometric sensitivity vector dpsi/dn
 *       is calculated, where n is the normal motion of the jth node.
 *
 *            airfoil:  1   <= i <= n         // techwinder: < to <=
 *            wake:      n+1 <= i <= n+nw      // techwinder: < to <=
 *
 * @todo mass defect array sig[] doesn't seem to be ever set,
 *       except perhaps in mixed inverse routines - check former fortran /common/ field?
 *
 * ----------------------------------------------------------------------- */
bool XFoil::psilin(int iNode, double xi, double yi, double nxi, double nyi, double &psi, double &psi_ni,
                   bool geolin, bool siglin)
{
    int io(0),jo(0),jm(0),jq(0),jp(0);

    double dxinv(0), psum(0), qtanm(0), scs(0), sds(0);
    double rx1(0), rx2(0), sx(0), sy(0), dsio(0), dso(0), dsm(0), dsim(0);
    double sgn(0), x0(0), logr0(0), theta0(0), rsq0(0), rsq1(0), rsq2(0);
    double nxo(0), nyo(0), nxp(0), nyp(0), ry1(0), ry2(0);
    double ssum(0), sdif(0), psni(0),pdni(0);
    double psx0(0), psx1(0), psx2(0), pdx0(0), pdx1(0), pdx2(0), psyy(0), pdyy(0), psis(0), psig(0), psid(0);
    double psigx1(0), psigx2(0), psigyy(0), pgamx1(0), pgamx2(0), pgamyy(0), psigni(0), pgamni(0);
    double gsum(0), gdif(0), gsum1(0), gsum2(0), gdif1(0), gdif2(0), pdif(0), dsp(0), dsip(0);
    double sigte1(0), sigte2(0), gamte1(0), gamte2(0), pgam(0);
    double apan(0), yy(0), logr12(0), logr22(0);
    double x1i(0), x2i(0), yyi(0), x1o(0), x1p(0), x2o(0), x2p(0), yyo(0), yyp(0);
    double seps(0);

    //---- distance tolerance for determining if two points are the same
    seps = (s[n]-s[1]) * 0.00001;

    io = iNode;

    cosa = cos(alfa);
    sina = sin(alfa);

    jp = 0;

    for (jo=1; jo<= n;jo++)
    {
        dzdg[jo] = 0.0;
        dzdn[jo] = 0.0;
        dqdg[jo] = 0.0;
    }

    for (jo=1; jo<= n;jo++)
    {
        dzdm[jo] = 0.0;
        dqdm[jo] = 0.0;
    }

    z_qinf = 0.0;
    z_alfa = 0.0;
    z_qdof0 = 0.0;
    z_qdof1 = 0.0;
    z_qdof2 = 0.0;
    z_qdof3 = 0.0;

    psi  = 0.0;
    psi_ni = 0.0;

    qtan1 = 0.0;
    qtan2 = 0.0;
    qtanm = 0.0;

    if(sharp)
    {
        scs = 1.0;
        sds = 0.0;
    }
    else
    {
        scs = ante/dste;
        sds = aste/dste;
    }

    for(jo=1; jo<=n; jo++)
    {
        //stop10
        jp = jo+1;
        jm = jo-1;
        jq = jp+1;

        if(jo==1) jm = jo;
        else
        {
            if(jo==n-1) jq = jp;
            else
            {
                if(jo==n)
                {
                    jp = 1;
                    if((x[jo]-x[jp])*(x[jo]-x[jp]) + (y[jo]-y[jp])*(y[jo]-y[jp]) < seps*seps)
                        goto stop12;
                }
            }
        }
//        Q_ASSERT(jq<=n);

        dso = sqrt((x[jo]-x[jp])*(x[jo]-x[jp]) + (y[jo]-y[jp])*(y[jo]-y[jp]));

        //------ skip null panel
        if(fabs(dso)<1.0e-7) goto stop10;

        dsio = 1.0 /dso;

        apan = apanel[jo];

        rx1 = xi - x[jo];
        ry1 = yi - y[jo];
        rx2 = xi - x[jp];
        ry2 = yi - y[jp];

        sx = (x[jp] - x[jo]) /dso;
        sy = (y[jp] - y[jo]) /dso;

        x1 = sx*rx1 + sy*ry1;
        x2 = sx*rx2 + sy*ry2;
        yy = sx*ry1 - sy*rx1;

        rsq1 = rx1*rx1 + ry1*ry1;
        rsq2 = rx2*rx2 + ry2*ry2;

        //------ set reflection flag sgn to avoid branch problems with arctan
        if(io>=1 && io<=n)
        {
            //------- no problem on airfoil surface
            sgn = 1.0;
        }
        else
        {
            //------- make sure arctan falls between  -/+  pi/2
            sgn = sign(1.0,yy);
        }

        //------ set log(r^2) and arctan(x/y), correcting for reflection if any
        if(io!=jo && rsq1>0.0)
        {
            logr12 = log(rsq1);
            theta1 = atan2(sgn*x1,sgn*yy) + (0.5- 0.5*sgn)*PI;
        }
        else
        {
            logr12 = 0.0;
            theta1 = 0.0;
        }

        if(io!=jp && rsq2>0.0)
        {
            logr22 = log(rsq2);
            theta2 = atan2(sgn*x2,sgn*yy) + (0.5- 0.5*sgn)*PI;
        }
        else
        {
            logr22 = 0.0;
            theta2 = 0.0;
        }

        x1i = sx*nxi + sy*nyi;
        x2i = sx*nxi + sy*nyi;
        yyi = sx*nyi - sy*nxi;

        if(geolin)
        {
            nxo = nx[jo];
            nyo = ny[jo];
            nxp = nx[jp];
            nyp = ny[jp];

            x1o =-((rx1-x1*sx)*nxo + (ry1-x1*sy)*nyo)/dso-(sx*nxo+sy*nyo);
            x1p = ((rx1-x1*sx)*nxp + (ry1-x1*sy)*nyp)/dso;
            x2o =-((rx2-x2*sx)*nxo + (ry2-x2*sy)*nyo)/dso;
            x2p = ((rx2-x2*sx)*nxp + (ry2-x2*sy)*nyp)/dso-(sx*nxp+sy*nyp);
            yyo = ((rx1+x1*sy)*nyo - (ry1-x1*sx)*nxo)/dso-(sx*nyo-sy*nxo);
            yyp =-((rx1-x1*sy)*nyp - (ry1+x1*sx)*nxp)/dso;
        }

        if (jo==n) goto stop11;

        if(siglin)
        {
            //------- set up midpoint quantities
            x0 = 0.5*(x1+x2);
            rsq0 = x0*x0 + yy*yy;
            logr0 = log(rsq0);
            theta0 = atan2(sgn*x0,sgn*yy) + (0.5- 0.5*sgn)*PI;

            //------- calculate source contribution to psi    for  1-0  half-panel
            dxinv = 1.0/(x1-x0);
            psum = x0*(theta0-apan) - x1*(theta1-apan) + 0.5*yy*(logr12-logr0);
            pdif = ((x1+x0)*psum + rsq1*(theta1-apan) - rsq0*(theta0-apan)+ (x0-x1)*yy) * dxinv;

            psx1 =    -(theta1-apan);
            psx0 =      theta0-apan;
            psyy =    0.5*(logr12-logr0);

            pdx1 = ((x1+x0)*psx1 + psum + 2.0*x1*(theta1-apan) - pdif) * dxinv;
            pdx0 = ((x1+x0)*psx0 + psum - 2.0*x0*(theta0-apan) + pdif) * dxinv;
            pdyy = ((x1+x0)*psyy + 2.0*(x0-x1 + yy*(theta1-theta0))  ) * dxinv;

            dsm = sqrt((x[jp]-x[jm])*(x[jp]-x[jm]) + (y[jp]-y[jm])*(y[jp]-y[jm]));
            dsim = 1.0/dsm;

            ssum = (sig[jp] - sig[jo])/dso + (sig[jp] - sig[jm])*dsim;
            sdif = (sig[jp] - sig[jo])/dso - (sig[jp] - sig[jm])*dsim;

            psi += qopi*(psum*ssum + pdif*sdif);

            //------- dpsi/dm
            dzdm[jm] += qopi*(-psum*dsim + pdif*dsim);
            dzdm[jo] += qopi*(-psum/dso - pdif/dso);
            dzdm[jp] += qopi*( psum*(dsio+dsim) + pdif*(dsio-dsim));

            //------- dpsi/dni
            psni = psx1*x1i + psx0*(x1i+x2i)*0.5 + psyy*yyi;
            pdni = pdx1*x1i + pdx0*(x1i+x2i)*0.5 + pdyy*yyi;
            psi_ni = psi_ni + qopi*(psni*ssum + pdni*sdif);

            qtanm = qtanm + qopi*(psni*ssum + pdni*sdif);

            dqdm[jm] += qopi*(-psni*dsim + pdni*dsim);
            dqdm[jo] += qopi*(-psni/dso - pdni/dso);
            dqdm[jp] += qopi*( psni*(dsio+dsim)+ pdni*(dsio-dsim));


            //------- calculate source contribution to psi    for  0-2  half-panel
            dxinv = 1.0/(x0-x2);
            psum = x2*(theta2-apan) - x0*(theta0-apan) + 0.5*yy*(logr0-logr22);
            pdif = ((x0+x2)*psum + rsq0*(theta0-apan) - rsq2*(theta2-apan)+ (x2-x0)*yy) * dxinv;

            psx0 =  -(theta0-apan);
            psx2 =      theta2-apan;
            psyy =    0.5*(logr0-logr22);

            pdx0 = ((x0+x2)*psx0 + psum + 2.0*x0*(theta0-apan) - pdif) * dxinv;
            pdx2 = ((x0+x2)*psx2 + psum - 2.0*x2*(theta2-apan) + pdif) * dxinv;
            pdyy = ((x0+x2)*psyy + 2.0*(x2-x0 + yy*(theta0-theta2))     ) * dxinv;

            dsp = sqrt((x[jq]-x[jo])*(x[jq]-x[jo]) + (y[jq]-y[jo])*(y[jq]-y[jo]));
            dsip = 1.0/dsp;

            ssum = (sig[jq] - sig[jo])*dsip + (sig[jp] - sig[jo])/dso;
            sdif = (sig[jq] - sig[jo])*dsip - (sig[jp] - sig[jo])/dso;

            psi = psi + qopi*(psum*ssum + pdif*sdif);

            //------- dpsi/dm
            dzdm[jo] += qopi*(-psum*(dsip+dsio)- pdif*(dsip-dsio));
            dzdm[jp] += qopi*( psum/dso - pdif/dso);
            dzdm[jq] += qopi*( psum*dsip + pdif*dsip);

            //------- dpsi/dni
            psni = psx0*(x1i+x2i)*0.5 + psx2*x2i + psyy*yyi;
            pdni = pdx0*(x1i+x2i)*0.5 + pdx2*x2i + pdyy*yyi;
            psi_ni = psi_ni + qopi*(psni*ssum + pdni*sdif);

            qtanm = qtanm + qopi*(psni*ssum + pdni*sdif);

            dqdm[jo] += qopi*(-psni*(dsip+dsio)- pdni*(dsip-dsio));
            dqdm[jp] += qopi*( psni/dso - pdni/dso);
            dqdm[jq] += qopi*( psni*dsip + pdni*dsip);
        }

        //------ calculate vortex panel contribution to psi
        dxinv = 1.0/(x1-x2);
        psis = 0.5*x1*logr12 - 0.5*x2*logr22 + x2 - x1 + yy*(theta1-theta2);
        psid = ((x1+x2)*psis + 0.5*(rsq2*logr22-rsq1*logr12 + x1*x1-x2*x2))*dxinv;

        psx1 = 0.5*logr12;
        psx2 = -.5*logr22;
        psyy = theta1-theta2;

        pdx1 = ((x1+x2)*psx1 + psis - x1*logr12 - psid)*dxinv;
        pdx2 = ((x1+x2)*psx2 + psis + x2*logr22 + psid)*dxinv;
        pdyy = ((x1+x2)*psyy - yy*(logr12-logr22)     )*dxinv;

        gsum1 = gamu[jp][1] + gamu[jo][1];
        gsum2 = gamu[jp][2] + gamu[jo][2];
        gdif1 = gamu[jp][1] - gamu[jo][1];
        gdif2 = gamu[jp][2] - gamu[jo][2];

        gsum = gam[jp] + gam[jo];
        gdif = gam[jp] - gam[jo];

        psi += qopi*(psis*gsum + psid*gdif);

        //------ dpsi/dgam
        dzdg[jo] += qopi*(psis-psid);
        dzdg[jp] += qopi*(psis+psid);

        //------ dpsi/dni
        psni = psx1*x1i + psx2*x2i + psyy*yyi;
        pdni = pdx1*x1i + pdx2*x2i + pdyy*yyi;
        psi_ni += qopi*(gsum*psni + gdif*pdni);

        qtan1 += qopi*(gsum1*psni + gdif1*pdni);
        qtan2 += qopi*(gsum2*psni + gdif2*pdni);

        dqdg[jo] += qopi*(psni - pdni);
        dqdg[jp] += qopi*(psni + pdni);

        if(geolin)
        {
            //------- dpsi/dn
            dzdn[jo] +=   qopi*gsum*(psx1*x1o + psx2*x2o + psyy*yyo)
                        + qopi*gdif*(pdx1*x1o + pdx2*x2o + pdyy*yyo);
            dzdn[jp] +=   qopi*gsum*(psx1*x1p + psx2*x2p + psyy*yyp)
                        + qopi*gdif*(pdx1*x1p + pdx2*x2p + pdyy*yyp);

            //------- dpsi/dp
            z_qdof0 += qopi*((psis-psid)*qf0[jo] + (psis+psid)*qf0[jp]);
            z_qdof1 += qopi*((psis-psid)*qf1[jo] + (psis+psid)*qf1[jp]);
            z_qdof2 += qopi*((psis-psid)*qf2[jo] + (psis+psid)*qf2[jp]);
            z_qdof3 += qopi*((psis-psid)*qf3[jo] + (psis+psid)*qf3[jp]);
        }
stop10:
        int nothing=1;      (void)nothing;    //c++ doesn't like gotos
    }

stop11:
    psig = 0.5*yy*(logr12-logr22) + x2*(theta2-apan) - x1*(theta1-apan);
    pgam = 0.5*x1*logr12 - 0.5*x2*logr22 + x2-x1 + yy*(theta1-theta2);

    psigx1 = -(theta1-apan);
    psigx2 =   theta2-apan;
    psigyy =  0.5*(logr12-logr22);
    pgamx1 =  0.5*logr12;
    pgamx2 = -0.5*logr22;
    pgamyy = theta1-theta2;

    psigni = psigx1*x1i + psigx2*x2i + psigyy*yyi;
    pgamni = pgamx1*x1i + pgamx2*x2i + pgamyy*yyi;

    //---- TE panel source and vortex strengths
    sigte1 =  0.5*scs*(gamu[jp][1] - gamu[jo][1]);
    sigte2 =  0.5*scs*(gamu[jp][2] - gamu[jo][2]);
    gamte1 = -0.5*sds*(gamu[jp][1] - gamu[jo][1]);
    gamte2 = -0.5*sds*(gamu[jp][2] - gamu[jo][2]);

    sigte =  0.5*scs*(gam[jp] - gam[jo]);
    gamte = -0.5*sds*(gam[jp] - gam[jo]);

    //---- TE panel contribution to psi
    psi += hopi*(psig*sigte + pgam*gamte);

    //---- dpsi/dgam
    dzdg[jo] += - hopi*psig*scs*0.5;
    dzdg[jp] += + hopi*psig*scs*0.5;

    dzdg[jo] += + hopi*pgam*sds*0.5;
    dzdg[jp] += - hopi*pgam*sds*0.5;

    //---- dpsi/dni
    psi_ni += hopi*(psigni*sigte + pgamni*gamte);

    qtan1 += hopi*(psigni*sigte1 + pgamni*gamte1);
    qtan2 += hopi*(psigni*sigte2 + pgamni*gamte2);

    dqdg[jo] += - hopi*(psigni*0.5*scs - pgamni*0.5*sds);
    dqdg[jp] += + hopi*(psigni*0.5*scs - pgamni*0.5*sds);

    if(geolin)
    {
        //----- dpsi/dn
        dzdn[jo] +=   hopi*(psigx1*x1o + psigx2*x2o + psigyy*yyo)*sigte
                + hopi*(pgamx1*x1o + pgamx2*x2o + pgamyy*yyo)*gamte;
        dzdn[jp] +=   hopi*(psigx1*x1p + psigx2*x2p + psigyy*yyp)*sigte
                + hopi*(pgamx1*x1p + pgamx2*x2p + pgamyy*yyp)*gamte;

        //----- dpsi/dp
        z_qdof0 +=   hopi*psig*0.5*(qf0[jp]-qf0[jo])*scs
                - hopi*pgam*0.5*(qf0[jp]-qf0[jo])*sds;
        z_qdof1 +=   hopi*psig*0.5*(qf1[jp]-qf1[jo])*scs
                - hopi*pgam*0.5*(qf1[jp]-qf1[jo])*sds;
        z_qdof2 +=   hopi*psig*0.5*(qf2[jp]-qf2[jo])*scs
                - hopi*pgam*0.5*(qf2[jp]-qf2[jo])*sds;
        z_qdof3 +=   hopi*psig*0.5*(qf3[jp]-qf3[jo])*scs
                - hopi*pgam*0.5*(qf3[jp]-qf3[jo])*sds;
    }
stop12:

    //**** freestream terms
    psi += qinf*(cosa*yi - sina*xi);

    //---- dpsi/dn
    psi_ni = psi_ni + qinf*(cosa*nyi - sina*nxi);

    qtan1 +=   qinf*nyi;
    qtan2 += - qinf*nxi;

    //---- dpsi/dqinf
    z_qinf += (cosa*yi - sina*xi);

    //---- dpsi/dalfa
    z_alfa += - qinf*(sina*yi + cosa*xi);

    //techwinder: removed image calculattion
    return false;
}



/** --------------------------------------------------------------------
 *       Calculates current streamfunction psi and tangential velocity
 *       qtan at panel node or wake node i due to freestream and wake
 *       sources sig. Also calculates sensitivity vectors dpsi/dsig
 *       (dzdm) and dqtan/dsig (dqdm).
 *
 *            airfoil:  1   <= i <= n         // techwinder: < to <=
 *            wake:      n+1 <= i <= n+nw      // techwinder: < to <=
 *
 *-------------------------------------------------------------------- */
bool XFoil::pswlin(int i, double xi, double yi, double nxi, double nyi, double &psi, double &psi_ni)
{
    double g1(0), g2(0), t1(0), t2(0);
    double x1i(0), x2i(0), yyi(0), x0(0),rs0(0),g0(0), t0(0);
    double dso(0), dsio(0), apan(0), rx1(0), rx2(0), ry1(0), ry2(0);
    double sx(0), sy(0), x1(0), x2(0), yy(0), rs1(0), rs2(0), sgn(0);
    double dxinv(0), psum(0), pdif(0), psx1(0), psx0(0), psyy(0), pdx1(0), pdx0(0), pdyy(0);
    double dsm, dsim(0), ssum(0), sdif(0), psni(0), pdni(0), psx2(0), pdx2(0), dsp(0), dsip(0);
    //double nxi, nyi;
    int io(0), jo(0);

    io = i;

    cosa = cos(alfa);
    sina = sin(alfa);


    for(jo=n+1; jo<=n+nw;jo++)
    {
        dzdm[jo] = 0.0;
        dqdm[jo] = 0.0;
    }

    psi     = 0.0;
    psi_ni = 0.0;

    for(jo=n+1; jo<=n+nw-1; jo++)
    {
        int jp = jo+1;
        int jm = jo-1;
        int jq = jp+1;
        if(jo==n+1)
        {
            jm = jo;
        }
        else
        {
            if(jo==n+nw-1) jq = jp;
        }
        dso = sqrt((x[jo]-x[jp])*(x[jo]-x[jp]) + (y[jo]-y[jp])* (y[jo]-y[jp]));
        dsio = 1.0 /dso;

        apan = apanel[jo];

        rx1 = xi - x[jo];
        ry1 = yi - y[jo];
        rx2 = xi - x[jp];
        ry2 = yi - y[jp];

        sx = (x[jp] - x[jo]) * dsio;
        sy = (y[jp] - y[jo]) * dsio;

        x1 = sx*rx1 + sy*ry1;
        x2 = sx*rx2 + sy*ry2;
        yy = sx*ry1 - sy*rx1;
        rs1 = rx1*rx1 + ry1*ry1;
        rs2 = rx2*rx2 + ry2*ry2;

        sgn =1.0;

        if(io>=n+1 && io<=n+nw)
        {
            sgn = 1.0;
        }
        else
        {
            sgn = sign(1.0,yy);
        }

        if(io!=jo && rs1>0.0)
        {
            g1 = log(rs1);
            t1 = atan2(sgn*x1,sgn*yy) - (0.5- 0.5*sgn)*PI;
        }
        else
        {
            g1 = 0.0;
            t1 = 0.0;
        }

        if(io!=jp && rs2>0.0)
        {
            g2 = log(rs2);
            t2 = atan2(sgn*x2,sgn*yy) - (0.5- 0.5*sgn)*PI;
        }
        else
        {
            g2 = 0.0;
            t2 = 0.0;
        }
        x1i = sx*nxi + sy*nyi;
        x2i = sx*nxi + sy*nyi;
        yyi = sx*nyi - sy*nxi;
        //------- set up midpoint quantities
        x0 = 0.5*(x1+x2);
        rs0 = x0*x0 + yy*yy;
        g0 = log(rs0);
        t0 = atan2(sgn*x0,sgn*yy) - (0.5- 0.5*sgn)*PI;

        //------- calculate source contribution to psi    for  1-0  half-panel
        dxinv = 1.0/(x1-x0);
        psum = x0*(t0-apan) - x1*(t1-apan) + 0.5*yy*(g1-g0);
        pdif = ((x1+x0)*psum + rs1*(t1-apan) - rs0*(t0-apan)
                + (x0-x1)*yy) * dxinv;

        psx1 =  -(t1-apan);
        psx0 =    t0-apan;
        psyy =  0.5*(g1-g0);

        pdx1 = ((x1+x0)*psx1 + psum + 2.0*x1*(t1-apan) - pdif) * dxinv;
        pdx0 = ((x1+x0)*psx0 + psum - 2.0*x0*(t0-apan) + pdif) * dxinv;
        pdyy = ((x1+x0)*psyy + 2.0*(x0-x1 + yy*(t1-t0))      ) * dxinv;

        dsm = sqrt((x[jp]-x[jm])*(x[jp]-x[jm]) + (y[jp]-y[jm])*(y[jp]-y[jm]));
        dsim = 1.0/dsm;

        // ccc          sig0 = (sig[jp] - sig[jo])/dso
        // ccc          sig1 = (sig[jp] - sig[jm])*dsim
        // ccc          ssum = sig0 + sig1
        // ccc          sdif = sig0 - sig1


        ssum = (sig[jp] - sig[jo])/dso + (sig[jp] - sig[jm])*dsim;
        sdif = (sig[jp] - sig[jo])/dso - (sig[jp] - sig[jm])*dsim;

        psi = psi + qopi*(psum*ssum + pdif*sdif);

        //------- dpsi/dm
        dzdm[jm] = dzdm[jm] + qopi*(-psum*dsim + pdif*dsim);
        dzdm[jo] = dzdm[jo] + qopi*(-psum/dso - pdif/dso);
        dzdm[jp] = dzdm[jp] + qopi*( psum*(dsio+dsim) + pdif*(dsio-dsim));

        //------- dpsi/dni
        psni = psx1*x1i + psx0*(x1i+x2i)*0.5+ psyy*yyi;
        pdni = pdx1*x1i + pdx0*(x1i+x2i)*0.5+ pdyy*yyi;
        psi_ni = psi_ni + qopi*(psni*ssum + pdni*sdif);

        dqdm[jm] += qopi*(-psni*dsim + pdni*dsim);
        dqdm[jo] += qopi*(-psni/dso - pdni/dso);
        dqdm[jp] += qopi*( psni*(dsio+dsim)+ pdni*(dsio-dsim));


        //------- calculate source contribution to psi    for  0-2  half-panel
        dxinv = 1.0/(x0-x2);
        psum = x2*(t2-apan) - x0*(t0-apan) + 0.5*yy*(g0-g2);
        pdif = ((x0+x2)*psum + rs0*(t0-apan) - rs2*(t2-apan)+ (x2-x0)*yy) * dxinv;

        psx0 =  -(t0-apan);
        psx2 =    t2-apan;
        psyy =  0.5*(g0-g2);

        pdx0 = ((x0+x2)*psx0 + psum + 2.0*x0*(t0-apan) - pdif) * dxinv;
        pdx2 = ((x0+x2)*psx2 + psum - 2.0*x2*(t2-apan) + pdif) * dxinv;
        pdyy = ((x0+x2)*psyy + 2.0*(x2-x0 + yy*(t0-t2))      ) * dxinv;

        dsp = sqrt((x[jq]-x[jo])*(x[jq]-x[jo]) + (y[jq]-y[jo])*(y[jq]-y[jo]));
        dsip = 1.0/dsp;

        ////ccc          sig2 = (sig[jq] - sig[jo])*dsip
        ////ccc          sig0 = (sig[jp] - sig[jo])/dso
        ////ccc          ssum = sig2 + sig0
        ////ccc          sdif = sig2 - sig0

        ssum = (sig[jq] - sig[jo])*dsip + (sig[jp] - sig[jo])/dso;
        sdif = (sig[jq] - sig[jo])*dsip - (sig[jp] - sig[jo])/dso;

        psi = psi + qopi*(psum*ssum + pdif*sdif);

        //------- dpsi/dm
        dzdm[jo] += qopi*(-psum*(dsip+dsio)- pdif*(dsip-dsio));
        dzdm[jp] += qopi*( psum/dso - pdif/dso);
        dzdm[jq] += qopi*( psum*dsip + pdif*dsip);

        //------- dpsi/dni
        psni = psx0*(x1i+x2i)*0.5+ psx2*x2i + psyy*yyi;
        pdni = pdx0*(x1i+x2i)*0.5+ pdx2*x2i + pdyy*yyi;
        psi_ni = psi_ni + qopi*(psni*ssum + pdni*sdif);

        dqdm[jo] = dqdm[jo] + qopi*(-psni*(dsip+dsio)- pdni*(dsip-dsio));
        dqdm[jp] = dqdm[jp] + qopi*( psni/dso - pdni/dso);
        dqdm[jq] = dqdm[jq] + qopi*( psni*dsip + pdni*dsip);
    }

    return true;
}



/** -----------------------------------------------------
 *        calculates source panel influence coefficient
 *        matrix for current airfoil and wake geometry.
 * ------------------------------------------------------ */
bool XFoil::qdcalc()
{
    int i(0), j(0), k(0), iu(0), iw(0);
    double psi(0), psi_n(0);
    double bbb[IQX];
    memset(bbb, 0, IQX*sizeof(double));

    //TRACE("calculating source influence matrix ...\n");
    std::string str = "   Calculating source influence matrix ...\n";
    writeString(str);

    if(!ladij)
    {
        //----- calculate source influence matrix for airfoil surface if it doesn't exist
        for (j=1; j<=n; j++)
        {
            //------- multiply each dpsi/sig vector by inverse of factored dpsi/dgam matrix
            for (iu=0; iu<IQX; iu++) bbb[iu] = bij[iu][j];//techwinder : create a dummy array
            baksub(n+1,aij,aijpiv,bbb);
            for (iu=0; iu<IQX; iu++) bij[iu][j] = bbb[iu];

            //------- store resulting dgam/dsig = dqtan/dsig vector
            for (i=1; i<=n; i++)
            {
                dij[i][j] = bij[i][j];
            }
        }
        ladij = true;
    }

    //---- set up coefficient matrix of dpsi/dm on wake
    for (i=1; i<=n; i++)
    {
        pswlin(i,x[i],y[i],nx[i],ny[i],psi,psi_n);
        for (j=n+1; j<=n+nw;j++)
        {
            bij[i][j] = -dzdm[j];
        }
    }

    //---- set up Kutta condition (no direct source influence)
    for(j=n+1; j<=n+nw; j++) bij[n+1][j] = 0.0;

    //---- sharp TE gamma extrapolation also has no source influence
    if(sharp)
    {
        for(j=n+1; j<=n+nw; j++) bij[n][j] = 0.0;
    }

/*
for(int i=1; i<=n; i++)
{
    std::string strong;
    for(int j=1; j<=n+nw; j++)
    {
        std::string str;
        str = std::string::asprintf(" %11g", bij[i][j]);
        strong+=str;
    }
    qDebug(strong.toStdString().c_str());
}*/

    //---- multiply by inverse of factored dpsi/dgam matrix
    for(j=n+1; j<=n+nw;j++)
    {
        //        baksub(iqx,n+1,aijpiv,j);
        for (iu=0; iu<IQX; iu++) bbb[iu] = bij[iu][j];//techwinder: create a dummy array

        baksub(n+1,aij,aijpiv,bbb);
        for (iu=0; iu<IQX; iu++) bij[iu][j] = bbb[iu];
    }

    //---- set the source influence matrix for the wake sources
    for(i=1; i<=n; i++)
    {
        for(j=n+1; j<=n+nw;j++)
        {
            dij[i][j] = bij[i][j];
        }
    }

/*
for(int i=1; i<=n; i++)
{
    std::string strong;
    for(int j=1; j<=n+nw; j++)
    {
        std::string str;
        str = std::string::asprintf(" %11g", dij[i][j]);
        strong+=str;
    }
    qDebug(strong.toStdString().c_str());
}*/


    //**** now we need to calculate the influence of sources on the wake velocities

    //---- calculate dqtan/dgam and dqtan/dsig at the wake points

    for (i=n+1; i<=n+nw;i++)
    {
        iw = i-n;
        //------ airfoil contribution at wake panel node
        psilin(i,x[i],y[i],nx[i],ny[i],psi,psi_n,false,true);
        for(j=1; j<=n;j++)
        {
            cij[iw][j] = dqdg[j];
        }
        for(j=1; j<=n; j++)
        {
            dij[i][j] = dqdm[j];
        }
        //------ wake contribution
        pswlin(i,x[i],y[i],nx[i],ny[i],psi,psi_n);
        for(j=n+1; j<=n+nw;j++)
        {
            dij[i][j] = dqdm[j];
        }
    }

    //---- add on effect of all sources on airfoil vorticity which effects wake qtan
    for(i=n+1; i<=n+nw; i++)
    {
        int iw = i-n;

        //------ airfoil surface source contribution first
        for(j=1; j<=n;j++)
        {
            for (k=1; k<=n; k++) dij[i][j] += cij[iw][k]*dij[k][j];
        }

        //------ wake source contribution next
        for(j=n+1; j<=n+nw;j++)
        {
            for(k=1; k<=n; k++) dij[i][j] += cij[iw][k]*bij[k][j];
        }
    }

    //---- make sure first wake point has same velocity as trailing edge
    for(j=1; j<=n+nw;j++)
    {
        dij[n+1][j] = dij[n][j];
    }


    lwdij = true;
    return true;
}


/** -------------------------------------------------------
 *     sets inviscid panel tangential velocity for
 *      current alpha.
 * -------------------------------------------------------- */
bool XFoil::qiset()
{
    cosa = cos(alfa);
    sina = sin(alfa);

    for (int i=1; i<=n+nw; i++)
    {
        qinv  [i] =  cosa*qinvu[i][1] + sina*qinvu[i][2];
        qinv_a[i] = -sina*qinvu[i][1] + cosa*qinvu[i][2];
    }

    return true;

}

/** -------------------------------------------------------------
 *     sets panel viscous tangential velocity from viscous ue
 * -------------------------------------------------------------- */
bool XFoil::qvfue()
{
    int is(0), ibl(0);
    for (is=1;is<= 2; is++)
    {
        for (ibl=2; ibl<=nbl[is];ibl++)
        {
            int i = ipan[ibl][is];
            qvis[i] = vti[ibl][is]*uedg[ibl][is];
        }
    }

    return true;
}


/** ---------------------------------------------------------------
 *      sets inviscid tangential velocity for alpha = 0, 90
 *      on wake due to freestream and airfoil surface vorticity.
 * --------------------------------------------------------------- */
bool XFoil::qwcalc()
{
    double psi, psi_ni;
    int i;

    //---- first wake point (same as te)
    qinvu[n+1][1] = qinvu[n][1];
    qinvu[n+1][2] = qinvu[n][2];

    //---- rest of wake
    for (i=n+2; i<=n+nw; i++)
    {
        psilin(i,x[i],y[i],nx[i],ny[i],psi,psi_ni,false,false);
        qinvu[i][1] = qtan1;
        qinvu[i][2] = qtan2;
    }

    return true;
}


bool XFoil::restoreblData(int icom)
{
    if (icom==1){
        x1     = blsav[icom].xz;
        u1     = blsav[icom].uz;
        theta1     = blsav[icom].tz;
        d1     = blsav[icom].dz;
        s1     = blsav[icom].sz;
        ampl1  = blsav[icom].amplz;
        u1_uei = blsav[icom].uz_uei;
        u1_ms  = blsav[icom].uz_ms;
        dw1    = blsav[icom].dwz;
        h1     = blsav[icom].hz;
        h1_t1  = blsav[icom].hz_tz;
        h1_d1  = blsav[icom].hz_dz;
        m1     = blsav[icom].mz;
        m1_u1  = blsav[icom].mz_uz;
        m1_ms  = blsav[icom].mz_ms;
        r1     = blsav[icom].rz;
        r1_u1  = blsav[icom].rz_uz;
        r1_ms  = blsav[icom].rz_ms;
        v1     = blsav[icom].vz;
        v1_u1  = blsav[icom].vz_uz;
        v1_ms  = blsav[icom].vz_ms;
        v1_re  = blsav[icom].vz_re;
        hk1    = blsav[icom].hkz;
        hk1_u1 = blsav[icom].hkz_uz;
        hk1_t1 = blsav[icom].hkz_tz;
        hk1_d1 = blsav[icom].hkz_dz;
        hk1_ms = blsav[icom].hkz_ms;
        hs1    = blsav[icom].hsz;
        hs1_u1 = blsav[icom].hsz_uz;
        hs1_t1 = blsav[icom].hsz_tz;
        hs1_d1 = blsav[icom].hsz_dz;
        hs1_ms = blsav[icom].hsz_ms;
        hs1_re = blsav[icom].hsz_re;
        hc1    = blsav[icom].hcz;
        hc1_u1 = blsav[icom].hcz_uz;
        hc1_t1 = blsav[icom].hcz_tz;
        hc1_d1 = blsav[icom].hcz_dz;
        hc1_ms = blsav[icom].hcz_ms;
        rt1    = blsav[icom].rtz;
        rt1_u1 = blsav[icom].rtz_uz;
        rt1_t1 = blsav[icom].rtz_tz;
        rt1_ms = blsav[icom].rtz_ms;
        rt1_re = blsav[icom].rtz_re;
        cf1    = blsav[icom].cfz;
        cf1_u1 = blsav[icom].cfz_uz;
        cf1_t1 = blsav[icom].cfz_tz;
        cf1_d1 = blsav[icom].cfz_dz;
        cf1_ms = blsav[icom].cfz_ms;
        cf1_re = blsav[icom].cfz_re;
        di1    = blsav[icom].diz;
        di1_u1 = blsav[icom].diz_uz;
        di1_t1 = blsav[icom].diz_tz;
        di1_d1 = blsav[icom].diz_dz;
        di1_s1 = blsav[icom].diz_sz;
        di1_ms = blsav[icom].diz_ms;
        di1_re = blsav[icom].diz_re;
        us1    = blsav[icom].usz;
        us1_u1 = blsav[icom].usz_uz;
        us1_t1 = blsav[icom].usz_tz;
        us1_d1 = blsav[icom].usz_dz;
        us1_ms = blsav[icom].usz_ms;
        us1_re = blsav[icom].usz_re;
        cq1    = blsav[icom].cqz;
        cq1_u1 = blsav[icom].cqz_uz;
        cq1_t1 = blsav[icom].cqz_tz;
        cq1_d1 = blsav[icom].cqz_dz;
        cq1_ms = blsav[icom].cqz_ms;
        cq1_re = blsav[icom].cqz_re;
        de1    = blsav[icom].dez;
        de1_u1 = blsav[icom].dez_uz;
        de1_t1 = blsav[icom].dez_tz;
        de1_d1 = blsav[icom].dez_dz;
        de1_ms = blsav[icom].dez_ms;
    }
    if (icom==2){
        x2     = blsav[icom].xz;
        u2     = blsav[icom].uz;
        theta2     = blsav[icom].tz;
        d2     = blsav[icom].dz;
        s2     = blsav[icom].sz;
        ampl2  = blsav[icom].amplz;
        u2_uei = blsav[icom].uz_uei;
        u2_ms  = blsav[icom].uz_ms;
        dw2    = blsav[icom].dwz;
        h2     = blsav[icom].hz;
        h2_t2  = blsav[icom].hz_tz;
        h2_d2  = blsav[icom].hz_dz;
        m2     = blsav[icom].mz;
        m2_u2  = blsav[icom].mz_uz;
        m2_ms  = blsav[icom].mz_ms;
        r2     = blsav[icom].rz;
        r2_u2  = blsav[icom].rz_uz;
        r2_ms  = blsav[icom].rz_ms;
        v2     = blsav[icom].vz;
        v2_u2  = blsav[icom].vz_uz;
        v2_ms  = blsav[icom].vz_ms;
        v2_re  = blsav[icom].vz_re;
        hk2    = blsav[icom].hkz;
        hk2_u2 = blsav[icom].hkz_uz;
        hk2_t2 = blsav[icom].hkz_tz;
        hk2_d2 = blsav[icom].hkz_dz;
        hk2_ms = blsav[icom].hkz_ms;
        hs2    = blsav[icom].hsz;
        hs2_u2 = blsav[icom].hsz_uz;
        hs2_t2 = blsav[icom].hsz_tz;
        hs2_d2 = blsav[icom].hsz_dz;
        hs2_ms = blsav[icom].hsz_ms;
        hs2_re = blsav[icom].hsz_re;
        hc2    = blsav[icom].hcz;
        hc2_u2 = blsav[icom].hcz_uz;
        hc2_t2 = blsav[icom].hcz_tz;
        hc2_d2 = blsav[icom].hcz_dz;
        hc2_ms = blsav[icom].hcz_ms;
        rt2    = blsav[icom].rtz;
        rt2_u2 = blsav[icom].rtz_uz;
        rt2_t2 = blsav[icom].rtz_tz;
        rt2_ms = blsav[icom].rtz_ms;
        rt2_re = blsav[icom].rtz_re;
        cf2    = blsav[icom].cfz;
        cf2_u2 = blsav[icom].cfz_uz;
        cf2_t2 = blsav[icom].cfz_tz;
        cf2_d2 = blsav[icom].cfz_dz;
        cf2_ms = blsav[icom].cfz_ms;
        cf2_re = blsav[icom].cfz_re;
        di2    = blsav[icom].diz;
        di2_u2 = blsav[icom].diz_uz;
        di2_t2 = blsav[icom].diz_tz;
        di2_d2 = blsav[icom].diz_dz;
        di2_s2 = blsav[icom].diz_sz;
        di2_ms = blsav[icom].diz_ms;
        di2_re = blsav[icom].diz_re;
        us2    = blsav[icom].usz;
        us2_u2 = blsav[icom].usz_uz;
        us2_t2 = blsav[icom].usz_tz;
        us2_d2 = blsav[icom].usz_dz;
        us2_ms = blsav[icom].usz_ms;
        us2_re = blsav[icom].usz_re;
        cq2    = blsav[icom].cqz;
        cq2_u2 = blsav[icom].cqz_uz;
        cq2_t2 = blsav[icom].cqz_tz;
        cq2_d2 = blsav[icom].cqz_dz;
        cq2_ms = blsav[icom].cqz_ms;
        cq2_re = blsav[icom].cqz_re;
        de2    = blsav[icom].dez;
        de2_u2 = blsav[icom].dez_uz;
        de2_t2 = blsav[icom].dez_tz;
        de2_d2 = blsav[icom].dez_dz;
        de2_ms = blsav[icom].dez_ms;
    }
    return true;
}


bool XFoil::saveblData(int icom){
    if(icom==1) {
        blsav[icom].xz     = x1;
        blsav[icom].uz     = u1;
        blsav[icom].tz     = theta1;
        blsav[icom].dz     = d1;
        blsav[icom].sz     = s1;
        blsav[icom].amplz  = ampl1;
        blsav[icom].uz_uei = u1_uei;
        blsav[icom].uz_ms  = u1_ms;
        blsav[icom].dwz    = dw1;
        blsav[icom].hz     = h1;
        blsav[icom].hz_tz  = h1_t1;
        blsav[icom].hz_dz  = h1_d1;
        blsav[icom].mz     = m1;
        blsav[icom].mz_uz  = m1_u1;
        blsav[icom].mz_ms  = m1_ms;
        blsav[icom].rz     = r1;
        blsav[icom].rz_uz  = r1_u1;
        blsav[icom].rz_ms  = r1_ms;
        blsav[icom].vz     = v1;
        blsav[icom].vz_uz  = v1_u1;
        blsav[icom].vz_ms  = v1_ms;
        blsav[icom].vz_re  = v1_re;
        blsav[icom].hkz    = hk1;
        blsav[icom].hkz_uz = hk1_u1;
        blsav[icom].hkz_tz = hk1_t1;
        blsav[icom].hkz_dz = hk1_d1;
        blsav[icom].hkz_ms = hk1_ms;
        blsav[icom].hsz    = hs1;
        blsav[icom].hsz_uz = hs1_u1;
        blsav[icom].hsz_tz = hs1_t1;
        blsav[icom].hsz_dz = hs1_d1;
        blsav[icom].hsz_ms = hs1_ms;
        blsav[icom].hsz_re = hs1_re;
        blsav[icom].hcz    = hc1;
        blsav[icom].hcz_uz = hc1_u1;
        blsav[icom].hcz_tz = hc1_t1;
        blsav[icom].hcz_dz = hc1_d1;
        blsav[icom].hcz_ms = hc1_ms;
        blsav[icom].rtz    = rt1;
        blsav[icom].rtz_uz = rt1_u1;
        blsav[icom].rtz_tz = rt1_t1;
        blsav[icom].rtz_ms = rt1_ms;
        blsav[icom].rtz_re = rt1_re;
        blsav[icom].cfz    = cf1;
        blsav[icom].cfz_uz = cf1_u1;
        blsav[icom].cfz_tz = cf1_t1;
        blsav[icom].cfz_dz = cf1_d1;
        blsav[icom].cfz_ms = cf1_ms;
        blsav[icom].cfz_re = cf1_re;
        blsav[icom].diz    = di1;
        blsav[icom].diz_uz = di1_u1;
        blsav[icom].diz_tz = di1_t1;
        blsav[icom].diz_dz = di1_d1;
        blsav[icom].diz_sz = di1_s1;
        blsav[icom].diz_ms = di1_ms;
        blsav[icom].diz_re = di1_re;
        blsav[icom].usz    = us1;
        blsav[icom].usz_uz = us1_u1;
        blsav[icom].usz_tz = us1_t1;
        blsav[icom].usz_dz = us1_d1;
        blsav[icom].usz_ms = us1_ms;
        blsav[icom].usz_re = us1_re;
        blsav[icom].cqz    = cq1;
        blsav[icom].cqz_uz = cq1_u1;
        blsav[icom].cqz_tz = cq1_t1;
        blsav[icom].cqz_dz = cq1_d1;
        blsav[icom].cqz_ms = cq1_ms;
        blsav[icom].cqz_re = cq1_re;
        blsav[icom].dez    = de1;
        blsav[icom].dez_uz = de1_u1;
        blsav[icom].dez_tz = de1_t1;
        blsav[icom].dez_dz = de1_d1;
        blsav[icom].dez_ms = de1_ms;
    }
    else{
        blsav[icom].xz     = x2;
        blsav[icom].uz     = u2;
        blsav[icom].tz     = theta2;
        blsav[icom].dz     = d2;
        blsav[icom].sz     = s2;
        blsav[icom].amplz  = ampl2;
        blsav[icom].uz_uei = u2_uei;
        blsav[icom].uz_ms  = u2_ms;
        blsav[icom].dwz    = dw2;
        blsav[icom].hz     = h2;
        blsav[icom].hz_tz  = h2_t2;
        blsav[icom].hz_dz  = h2_d2;
        blsav[icom].mz     = m2;
        blsav[icom].mz_uz  = m2_u2;
        blsav[icom].mz_ms  = m2_ms;
        blsav[icom].rz     = r2;
        blsav[icom].rz_uz  = r2_u2;
        blsav[icom].rz_ms  = r2_ms;
        blsav[icom].vz     = v2;
        blsav[icom].vz_uz  = v2_u2;
        blsav[icom].vz_ms  = v2_ms;
        blsav[icom].vz_re  = v2_re;
        blsav[icom].hkz    = hk2;
        blsav[icom].hkz_uz = hk2_u2;
        blsav[icom].hkz_tz = hk2_t2;
        blsav[icom].hkz_dz = hk2_d2;
        blsav[icom].hkz_ms = hk2_ms;
        blsav[icom].hsz    = hs2;
        blsav[icom].hsz_uz = hs2_u2;
        blsav[icom].hsz_tz = hs2_t2;
        blsav[icom].hsz_dz = hs2_d2;
        blsav[icom].hsz_ms = hs2_ms;
        blsav[icom].hsz_re = hs2_re;
        blsav[icom].hcz    = hc2;
        blsav[icom].hcz_uz = hc2_u2;
        blsav[icom].hcz_tz = hc2_t2;
        blsav[icom].hcz_dz = hc2_d2;
        blsav[icom].hcz_ms = hc2_ms;
        blsav[icom].rtz    = rt2;
        blsav[icom].rtz_uz = rt2_u2;
        blsav[icom].rtz_tz = rt2_t2;
        blsav[icom].rtz_ms = rt2_ms;
        blsav[icom].rtz_re = rt2_re;
        blsav[icom].cfz    = cf2;
        blsav[icom].cfz_uz = cf2_u2;
        blsav[icom].cfz_tz = cf2_t2;
        blsav[icom].cfz_dz = cf2_d2;
        blsav[icom].cfz_ms = cf2_ms;
        blsav[icom].cfz_re = cf2_re;
        blsav[icom].diz    = di2;
        blsav[icom].diz_uz = di2_u2;
        blsav[icom].diz_tz = di2_t2;
        blsav[icom].diz_dz = di2_d2;
        blsav[icom].diz_sz = di2_s2;
        blsav[icom].diz_ms = di2_ms;
        blsav[icom].diz_re = di2_re;
        blsav[icom].usz    = us2;
        blsav[icom].usz_uz = us2_u2;
        blsav[icom].usz_tz = us2_t2;
        blsav[icom].usz_dz = us2_d2;
        blsav[icom].usz_ms = us2_ms;
        blsav[icom].usz_re = us2_re;
        blsav[icom].cqz    = cq2;
        blsav[icom].cqz_uz = cq2_u2;
        blsav[icom].cqz_tz = cq2_t2;
        blsav[icom].cqz_dz = cq2_d2;
        blsav[icom].cqz_ms = cq2_ms;
        blsav[icom].cqz_re = cq2_re;
        blsav[icom].dez    = de2;
        blsav[icom].dez_uz = de2_u2;
        blsav[icom].dez_tz = de2_t2;
        blsav[icom].dez_dz = de2_d2;
        blsav[icom].dez_ms = de2_ms;
    }
    return true;
}


/** ----------------------------------------
 *      calculates the arc length array s  |
 *      for a 2-d array of points (x,y).   |
 * ----------------------------------------- */
bool XFoil::scalc(double x[], double y[], double s[], int n)
{
    s[1] = 0.0;
    for (int i=2;i<=n; i++)
    {
        s[i] = s[i-1] + sqrt((x[i]-x[i-1])*(x[i]-x[i-1])+(y[i]-y[i-1])*(y[i]-y[i-1]));
    }
    return true;
}


/** -----------------------------------------------
 *      Splines x(s) array just like spline,      |
 *      but allows derivative discontinuities     |
 *      at segment joints.  Segment joints are    |
 *      defined by identical successive s values. |
 * ----------------------------------------------- */
bool XFoil::segspl(double x[], double xs[], double s[], int n)
{
    int nseg(0), iseg(0), iseg0(0);

    if(fabs(s[1]-s[2])<EPSILON) return false; //stop 'segspl:  first input point duplicated'
    if(fabs(s[n]-s[n-1])<EPSILON) return false; //stop 'segspl:  last  input point duplicated'

    iseg0 = 1;
    for (iseg=2; iseg<=n-2; iseg++)
    {
        if(fabs(s[iseg]-s[iseg+1])<EPSILON)
        {
            nseg = iseg - iseg0 + 1;
            //            splind(x[iseg0],xs[iseg0],s[iseg0],nseg,-999.0,-999.0);
            splind(x+iseg0-1,xs+iseg0-1,s+iseg0-1,nseg,-999.0,-999.0);
            iseg0 = iseg+1;
        }
    }
    nseg = n - iseg0 + 1;

    //    splind(x[iseg0],xs[iseg0],s[iseg0],nseg,-999.0,-999.0);
    splind(x+iseg0-1,xs+iseg0-1,s+iseg0-1,nseg,-999.0,-999.0);

    return true;
}


/** -----------------------------------------------
 *     splines x(s) array just like splind,      |
 *     but allows derivative discontinuities     |
 *     at segment joints.  segment joints are    |
 *     defined by identical successive s values. |
 * ----------------------------------------------- */
bool XFoil::segspld(double x[],double xs[],double s[],int n, double xs1, double xs2)
{
    int nseg(0), iseg(0), iseg0(0);

    if(fabs(s[1]-s[2])<EPSILON) return false; //stop 'segspl:  first input point duplicated';
    if(fabs(s[n]-s[n-1])<EPSILON) return false; //stop 'segspl:  last  input point duplicated';

    iseg0 = 1;
    for (iseg=2; iseg<=n-2; iseg++)
    {
        if(fabs(s[iseg]-s[iseg+1])<EPSILON)
        {
            nseg = iseg - iseg0 + 1;
            splind(x+iseg0-1,xs+iseg0-1,s+iseg0-1,nseg, xs1, xs2);
            iseg0 = iseg+1;
        }
    }
    nseg = n - iseg0 + 1;
    splind(x+iseg0-1,xs+iseg0-1,s+iseg0-1,nseg,xs1, xs2);
    return true;
}

/** -------------------------------------------------
 *       Sets up the BL Newton system coefficients for the current BL variables
 *     and the edge velocities received from setup.
 *     The local BL system  coefficients are then incorporated into
 *     the global Newton system.
 * ------------------------------------------------- */
bool XFoil::setbl()
{
    int i(0), ibl(0), iv(0),iw(0), j(0), js(0), jv(0), jbl(0), is(0);
    int ile1(0),ile2(0),ite1(0),ite2(0),jvte1(0),jvte2(0);

    double usav[IVX+1][ISX];
    double u1_m[2*IVX+1], u2_m[2*IVX+1];
    double d1_m[2*IVX+1], d2_m[2*IVX+1];
    double ule1_m[2*IVX+1], ule2_m[2*IVX+1];
    double ute1_m[2*IVX+1], ute2_m[2*IVX+1];

    for(int i=0; i<IVX+1; i++) memset(usav[i], 0, ISX*sizeof(double));
    memset(u1_m, 0, (2*IVX+1)*sizeof(double));
    memset(u2_m, 0, (2*IVX+1)*sizeof(double));
    memset(d1_m, 0, (2*IVX+1)*sizeof(double));
    memset(d2_m, 0, (2*IVX+1)*sizeof(double));
    memset(ule1_m, 0, (2*IVX+1)*sizeof(double));
    memset(ule2_m, 0, (2*IVX+1)*sizeof(double));
    memset(ute1_m, 0, (2*IVX+1)*sizeof(double));
    memset(ute2_m, 0, (2*IVX+1)*sizeof(double));

    double msq_clmr(0), mdi(0);
    double herat(0),herat_ms(0);

    double clmr(0),ma_clmr(0),re_clmr(0);
    double ule1_a(0), ule2_a(0), u1_a(0), u2_a(0), d1_a(0), due1(0), due2(0), dds1(0), dds2(0);
    double xsi(0), cti(0), uei(0), thi(0), dsi(0), dswaki(0);
    double d2_a(0), d2_m2(0), d2_u2(0), dte_mte1(0), dte_ute1(0), dte_mte2(0), dte_ute2(0);
    double tte(0), cte(0), dte(0), dule1(0),dule2(0);
    double str(0), chx(0), chy(0), xtr(0), ytr(0), chsq(0);
    double xi_ule1(0), xi_ule2(0);
    double ami(0), tte_tte1(0), tte_tte2(0), cte_tte1(0), cte_tte2(0), cte_cte1(0), cte_cte2(0);


    //---- set the cl used to define mach, reynolds numbers
    if(lalfa) clmr = cl;
    else  clmr = clspec;

    cti = 0.0; //techwinder added, otherwise variable is not initialized

    //---- set current minf(cl)
    mrcl(clmr, ma_clmr, re_clmr);
    msq_clmr = 2.0*minf*ma_clmr;

    //---- set compressibility parameter tklam and derivative tk_msq
    comset();

    //---- set gas constant (= cp/cv)
    gambl = gamma;
    gm1bl = gamm1;

    //---- set parameters for compressibility correction
    qinfbl  = qinf;
    tkbl    = tklam;
    tkbl_ms = tkl_msq;

    //---- stagnation density and 1/enthalpy
    rstbl     = pow((1.0 + 0.5*gm1bl*minf*minf) ,(1.0/gm1bl));
    rstbl_ms = 0.5*rstbl/(1.0 + 0.5*gm1bl*minf*minf);
    hstinv    = gm1bl*(minf/qinfbl)*(minf/qinfbl) / (1.0 + 0.5*gm1bl*minf*minf);
    hstinv_ms = gm1bl*( 1.0/qinfbl)*( 1.0/qinfbl) / (1.0 + 0.5*gm1bl*minf*minf)
            - 0.5*gm1bl*hstinv / (1.0 + 0.5*gm1bl*minf*minf);

    //---- sutherland's const./to    (assumes stagnation conditions are at stp)
    hvrat = 0.35;

    //---- set reynolds number based on freestream density, velocity, viscosity
    herat     = 1.0 - 0.5*qinfbl*qinfbl*hstinv;
    herat_ms =     - 0.5*qinfbl*qinfbl*hstinv_ms;

    reybl     = reinf * sqrt(herat*herat*herat) * (1.0+hvrat)/(herat+hvrat);
    reybl_re =           sqrt(herat*herat*herat) * (1.0+hvrat)/(herat+hvrat);
    reybl_ms = reybl * (1.5/herat - 1.0/(herat+hvrat))*herat_ms;

    amcrit = acrit;

    //---- save te thickness
    dwte = wgap[1];

    if(!lblini)
    {
        //----- initialize bl by marching with ue (fudge at separation)
        //TRACE(" initializing bl ...\n");
        std::string str = "   Initializing bl ...\n";
        writeString(str);

        mrchue();
        lblini = true;
    }

    //---- march bl with current ue and ds to establish transition
    mrchdu();

    for (is=1;is<= 2;is++){
        for(ibl=2; ibl<=nbl[is];ibl++)
            usav[ibl][is] = uedg[ibl][is];
    }

    ueset();

    for (is=1;is<= 2;is++)
    {
        for(ibl=2; ibl<=nbl[is];ibl++)
        {
            double temp = usav[ibl][is];
            usav[ibl][is] = uedg[ibl][is];
            uedg[ibl][is] = temp;
        }
    }
    ile1 = ipan[2][1];
    ile2 = ipan[2][2];
    ite1 = ipan[iblte[1]][1];
    ite2 = ipan[iblte[2]][2];

    jvte1 = isys[iblte[1]][1];
    jvte2 = isys[iblte[2]][2];

    dule1 = uedg[2][1] - usav[2][1];
    dule2 = uedg[2][2] - usav[2][2];

    //---- set le and te ue sensitivities wrt all m values
    for(js=1; js<= 2;js++)
    {
        for(jbl=2;jbl<= nbl[js];jbl++)
        {
            j  = ipan[jbl][js];
            jv = isys[jbl][js];
            ule1_m[jv] = -vti[         2][1]*vti[jbl][js]*dij[ile1][j];
            ule2_m[jv] = -vti[         2][2]*vti[jbl][js]*dij[ile2][j];
            ute1_m[jv] = -vti[iblte[1]][1]*vti[jbl][js]*dij[ite1][j];
            ute2_m[jv] = -vti[iblte[2]][2]*vti[jbl][js]*dij[ite2][j];
        }
    }

    ule1_a = uinv_a[2][1];
    ule2_a = uinv_a[2][2];

    std::string str1 = " \n";
    writeString(str1);

    //*** go over each boundary layer/wake
    for(is=1;is<=2;is++)
    {
        //---- there is no station "1" at similarity, so zero everything out
        for(js=1; js<= 2;js++)
        {
            for(jbl=2;jbl<= nbl[js];jbl++)
            {
                jv = isys[jbl][js];
                u1_m[jv] = 0.0;
                d1_m[jv] = 0.0;
            }
        }
        u1_a = 0.0;
        d1_a = 0.0;

        due1 = 0.0;
        dds1 = 0.0;

        //---- similarity station pressure gradient parameter  x/u du/dx
        ibl = 2;
        bule = 1.0;

        //---- set forced transition arc length position
        xifset(is);

        tran = false;
        turb = false;

        //**** sweep downstream setting up bl equation linearizations
        for(ibl=2;ibl<= nbl[is];ibl++)
        {
            iv    = isys[ibl][is];

            simi = (ibl==2);
            wake = (ibl>iblte[is]);
            tran = (ibl==itran[is]);
            turb = (ibl>itran[is]);

            i = ipan[ibl][is];

            //---- set primary variables for current station
            xsi = xssi[ibl][is];
            if(ibl<itran[is]) ami = ctau[ibl][is];
            else cti = ctau[ibl][is];
            uei = uedg[ibl][is];
            thi = thet[ibl][is];
            mdi = mass[ibl][is];

            dsi = mdi/uei;

            if(wake)
            {
                iw = ibl - iblte[is];
                dswaki = wgap[iw];
            }
            else        dswaki = 0.0;


            //---- set derivatives of dsi (= d2)
            d2_m2 =  1.0/uei;
            d2_u2 = -dsi /uei;

            for(js=1; js<= 2;js++)
            {
                for(jbl=2;jbl<= nbl[js];jbl++)
                {
                    j  = ipan[jbl][js];
                    jv = isys[jbl][js];
                    u2_m[jv] = -vti[ibl][is]*vti[jbl][js]*dij[i][j];
                    d2_m[jv] = d2_u2*u2_m[jv];
                }
            }
            d2_m[iv] = d2_m[iv] + d2_m2;

            u2_a = uinv_a[ibl][is];
            d2_a = d2_u2*u2_a;

            //---- "forced" changes due to mismatch between uedg and usav=uinv+dij*mass
            due2 = uedg[ibl][is] - usav[ibl][is];
            dds2 = d2_u2*due2;

            blprv(xsi,ami,cti,thi,dsi,dswaki,uei);//cti
            blkin();

            //---- check for transition and set tran, xt, etc. if found
            if(tran) {
                trchek();
                ami = ampl2;
            }

            if(ibl==itran[is] && !tran)
            {
                //TRACE("setbl: xtr???  n1=%d n2=%d: \n", ampl1, ampl2);
            }

            //---- assemble 10x4 linearized system for dctau, dth, dds, due, dxi
            //       at the previous "1" station and the current "2" station

            if(ibl==iblte[is]+1)
            {
                //----- define quantities at start of wake, adding te base thickness to dstar
                tte = thet[iblte[1]][1] + thet[iblte[2]][2];
                dte = dstr[iblte[1]][1] + dstr[iblte[2]][2] + ante;
                cte = ( ctau[iblte[1]][1]*thet[iblte[1]][1] + ctau[iblte[2]][2]*thet[iblte[2]][2] ) / tte;
                tesys(cte,tte,dte);

                tte_tte1 = 1.0;
                tte_tte2 = 1.0;
                dte_mte1 =                 1.0  / uedg[iblte[1]][1];
                dte_ute1 = -dstr[iblte[1]][1] / uedg[iblte[1]][1];
                dte_mte2 =                 1.0  / uedg[iblte[2]][2];
                dte_ute2 = -dstr[iblte[2]][2] / uedg[iblte[2]][2];
                cte_cte1 = thet[iblte[1]][1]/tte;
                cte_cte2 = thet[iblte[2]][2]/tte;
                cte_tte1 = (ctau[iblte[1]][1] - cte)/tte;
                cte_tte2 = (ctau[iblte[2]][2] - cte)/tte;

                //----- re-define d1 sensitivities wrt m since d1 depends on both te ds values
                for (js=1; js<= 2;js++)
                {
                    for (jbl=2; jbl<= nbl[js];jbl++)
                    {
                        j  = ipan[jbl][js];
                        jv = isys[jbl][js];
                        d1_m[jv] = dte_ute1*ute1_m[jv] + dte_ute2*ute2_m[jv];
                    }
                }
                d1_m[jvte1] = d1_m[jvte1] + dte_mte1;
                d1_m[jvte2] = d1_m[jvte2] + dte_mte2;

                //----- "forced" changes from  uedg --- usav=uinv+dij*mass    mismatch
                due1 = 0.0;
                dds1 = dte_ute1*(uedg[iblte[1]][1] - usav[iblte[1]][1])
                     + dte_ute2*(uedg[iblte[2]][2] - usav[iblte[2]][2]);
            }
            else{
                blsys();
            }


            //---- save wall shear and equil. max shear coefficient for plotting output
            tau[ibl][is] = 0.5*r2*u2*u2*cf2;
            dis[ibl][is] =     r2*u2*u2*u2*di2*hs2*0.5;
            ctq[ibl][is] = cq2;
            delt[ibl][is] = de2;
            uslp[ibl][is] = 1.60/(1.0+us2);

            //---- set xi sensitivities wrt le ue changes
            if(is==1) {
                xi_ule1 =  sst_go;
                xi_ule2 = -sst_gp;
            }
            else{
                xi_ule1 = -sst_go;
                xi_ule2 =  sst_gp;
            }

            //---- stuff bl system coefficients into main jacobian matrix

            for( jv=1; jv<= nsys;jv++){
                vm[1][jv][iv] = vs1[1][3]*d1_m[jv] + vs1[1][4]*u1_m[jv]
                        + vs2[1][3]*d2_m[jv] + vs2[1][4]*u2_m[jv]
                        + (vs1[1][5] + vs2[1][5] + vsx[1])
                        *(xi_ule1*ule1_m[jv] + xi_ule2*ule2_m[jv]);
            }

            vb[1][1][iv] = vs1[1][1];
            vb[1][2][iv] = vs1[1][2];

            va[1][1][iv] = vs2[1][1];
            va[1][2][iv] = vs2[1][2];

            if(lalfa)  vdel[1][2][iv] = vsr[1]*re_clmr + vsm[1]*msq_clmr;
            else         vdel[1][2][iv] =
                    (vs1[1][4]*u1_a + vs1[1][3]*d1_a)
                    + (vs2[1][4]*u2_a + vs2[1][3]*d2_a)
                    + (vs1[1][5] + vs2[1][5] + vsx[1])
                    *(xi_ule1*ule1_a + xi_ule2*ule2_a);


            vdel[1][1][iv] = vsrez[1]
                    + (vs1[1][4]*due1 + vs1[1][3]*dds1)
                    + (vs2[1][4]*due2 + vs2[1][3]*dds2)
                    + (vs1[1][5] + vs2[1][5] + vsx[1])
                    *(xi_ule1*dule1 + xi_ule2*dule2);

            for(jv=1; jv<= nsys;jv++){
                vm[2][jv][iv] = vs1[2][3]*d1_m[jv] + vs1[2][4]*u1_m[jv]
                        + vs2[2][3]*d2_m[jv] + vs2[2][4]*u2_m[jv]
                        + (vs1[2][5] + vs2[2][5] + vsx[2])
                        *(xi_ule1*ule1_m[jv] + xi_ule2*ule2_m[jv]);
            }
            vb[2][1][iv]    = vs1[2][1];
            vb[2][2][iv]    = vs1[2][2];

            va[2][1][iv] = vs2[2][1];
            va[2][2][iv] = vs2[2][2];

            if(lalfa) vdel[2][2][iv] = vsr[2]*re_clmr + vsm[2]*msq_clmr;
            else         vdel[2][2][iv] =
                    (vs1[2][4]*u1_a + vs1[2][3]*d1_a)
                    + (vs2[2][4]*u2_a + vs2[2][3]*d2_a)
                    + (vs1[2][5] + vs2[2][5] + vsx[2])
                    *(xi_ule1*ule1_a + xi_ule2*ule2_a);


            vdel[2][1][iv] = vsrez[2]
                    + (vs1[2][4]*due1 + vs1[2][3]*dds1)
                    + (vs2[2][4]*due2 + vs2[2][3]*dds2)
                    + (vs1[2][5] + vs2[2][5] + vsx[2])
                    *(xi_ule1*dule1 + xi_ule2*dule2);


            //memory overlap problem
            for(jv=1; jv<= nsys;jv++){
                vm[3][jv][iv] = vs1[3][3]*d1_m[jv] + vs1[3][4]*u1_m[jv]
                        + vs2[3][3]*d2_m[jv] + vs2[3][4]*u2_m[jv]
                        + (vs1[3][5] + vs2[3][5] + vsx[3])
                        *(xi_ule1*ule1_m[jv] + xi_ule2*ule2_m[jv]);
            }

            vb[3][1][iv] = vs1[3][1];
            vb[3][2][iv] = vs1[3][2];

            va[3][1][iv] = vs2[3][1];
            va[3][2][iv] = vs2[3][2];

            if(lalfa) vdel[3][2][iv] = vsr[3]*re_clmr + vsm[3]*msq_clmr;
            else         vdel[3][2][iv] =
                    (vs1[3][4]*u1_a + vs1[3][3]*d1_a)
                    + (vs2[3][4]*u2_a + vs2[3][3]*d2_a)
                    + (vs1[3][5] + vs2[3][5] + vsx[3])
                    *(xi_ule1*ule1_a + xi_ule2*ule2_a);


            vdel[3][1][iv] = vsrez[3]
                    + (vs1[3][4]*due1 + vs1[3][3]*dds1)
                    + (vs2[3][4]*due2 + vs2[3][3]*dds2)
                    + (vs1[3][5] + vs2[3][5] + vsx[3])
                    *(xi_ule1*dule1 + xi_ule2*dule2);

            if(ibl==iblte[is]+1) {

                //----- redefine coefficients for tte, dte, etc
                vz[1][1]    = vs1[1][1]*cte_cte1;
                vz[1][2]    = vs1[1][1]*cte_tte1 + vs1[1][2]*tte_tte1;
                vb[1][1][iv] = vs1[1][1]*cte_cte2;
                vb[1][2][iv] = vs1[1][1]*cte_tte2 + vs1[1][2]*tte_tte2;

                vz[2][1]    = vs1[2][1]*cte_cte1;
                vz[2][2]    = vs1[2][1]*cte_tte1 + vs1[2][2]*tte_tte1;
                vb[2][1][iv] = vs1[2][1]*cte_cte2;
                vb[2][2][iv] = vs1[2][1]*cte_tte2 + vs1[2][2]*tte_tte2;

                vz[3][1]    = vs1[3][1]*cte_cte1;
                vz[3][2]    = vs1[3][1]*cte_tte1 + vs1[3][2]*tte_tte1;
                vb[3][1][iv] = vs1[3][1]*cte_cte2;
                vb[3][2][iv] = vs1[3][1]*cte_tte2 + vs1[3][2]*tte_tte2;

            }

            //---- turbulent intervals will follow if currently at transition interval
            if(tran) {
                turb = true;

                //------ save transition location
                itran[is] = ibl;
                tforce[is] = trforc;
                xssitr[is] = xt;

                //------ interpolate airfoil geometry to find transition x/c
                //        (for user output)
                if(is==1) str = sst - xt;
                else str = sst + xt;

                chx = xte - xle;
                chy = yte - yle;
                chsq = chx*chx + chy*chy;
                xtr = seval(str,x,xp,s,n);
                ytr = seval(str,y,yp,s,n);
                xoctr[is] = ((xtr-xle)*chx + (ytr-yle)*chy)/chsq;
                yoctr[is] = ((ytr-yle)*chx - (xtr-xle)*chy)/chsq;
            }

            tran = false;

            if(ibl==iblte[is]) {
                //----- set "2" variables at te to wake correlations for next station

                turb = true;
                wake = true;
                blvar(3);
                blmid(3);
            }

            for(js=1; js<= 2;js++){
                for(jbl=2; jbl<= nbl[js];jbl++){
                    jv = isys[jbl][js];
                    u1_m[jv] = u2_m[jv];
                    d1_m[jv] = d2_m[jv];
                }
            }

            u1_a = u2_a;
            d1_a = d2_a;

            due1 = due2;
            dds1 = dds2;

            //---- set bl variables for next station
            //            for (icom=1; icom<= ncom;icom++)    com1[icom] = com2[icom];
            stepbl();

            //---- next streamwise station
        }

        std::string strOut;
        if(tforce[is])
        {
//            strOut = QString::asprintf("     Side %d, forced transition at x/c = {:.4f} {:d}\n",  is, xoctr[is], itran[is]);
            //TRACE(strOut);
//            writeString(strOut);

        }
        else
        {
//            strOut = QString::asprintf("     Side %d, ffree transition at x/c = {:.4f} {:d}\n",  is, xoctr[is], itran[is]);
            //TRACE(strOut);
//            writeString(strOut);
        }

        //---- next airfoil side
    }

    return true;
}


void XFoil::scheck(double x[], double y[], int *n, double stol, bool *lchange){

    //-------------------------------------------------------------
    //     removes points from an x,y spline contour wherever
    //     the size of a segment between nodes falls below a
    //     a specified threshold of the adjacent segments.
    //     the two node points defining the short segment are
    //     replaced with a single node at their midpoint.
    //     note that the number of nodes may be altered by
    //     this routine.
    //
    //     intended for eliminating odd "micro" panels
    //     that occur when blending a flap to a foil.
    //     if lchange is set on return the airfoil definition
    //     has been changed and resplining should be done.
    //
    //     the recommended value for stol is 0.05 (meaning
    //     segments less than 5% of the length of either adjoining
    //     segment are removed).  4/24/01 hhy
    //------------------------------------------------------

    //    int im1, ip1, ip2;
    int i(0), l(0);
    double dxm1(0), dym1(0), dsm1(0), dxp1(0), dxp2(0),dyp1(0),dyp2(0),dsp1(0),dsp2(0);

    *lchange = false;
    //--- check stol for sanity
    if(stol>0.3)
    {
        std::string str("scheck:  bad value for small panels (stol > 0.3)\n");
        writeString(str, true);
        return;
    }
    //      10
    for (i = 2; i<= *n-2; i++)
    {
        //        im1 = i-1;
        //        ip1 = i+1;
        //        ip2 = i+2;

        dxm1 = x[i] - x[i-1];
        dym1 = y[i] - y[i-1];
        dsm1 = sqrt(dxm1*dxm1 + dym1*dym1);

        dxp1 = x[i+1] - x[i];
        dyp1 = y[i+1] - y[i];
        dsp1 = sqrt(dxp1*dxp1 + dyp1*dyp1);

        dxp2 = x[i+2] - x[i+1];
        dyp2 = y[i+2] - y[i+1];
        dsp2 = sqrt(dxp2*dxp2 + dyp2*dyp2);

        //------- don't mess with doubled points (slope breaks)
        if(dsp1>=0.00001)
        {//go to 20

            if(dsp1<stol*dsm1 || dsp1<stol*dsp2)
            {
                //------- replace node i with average of i and i+1
                x[i] = 0.5*(x[i]+x[i+1]);
                y[i] = 0.5*(y[i]+y[i+1]);
                //------- remove node i+1
                for (l = i+1 ;l<= *n; l++)
                {
                    x[l] = x[l+1];
                    y[l] = y[l+1];
                }
                *n = *n - 1;
                *lchange = true;

                //       go to 10
            }
        }
    }//20
}


/** ........................................................
 *     sets geometriy stretched array s:
 *
 *       s(i+1) - s(i)  =  r * [s(i) - s(i-1)]
 *
 *       s     (output)  array to be set
 *       ds1   (input)   first s increment:  s[2] - s[1]
 *       smax  (input)   final s value:      s(nn)
 *       nn    (input)   number of points
 * ........................................................*/
bool XFoil::setexp(double s[], double ds1, double smax, int nn)
{
    std::string str;
    int nex(0), iter(0), n(0);
    double sigma(0), rnex(0), rni(0), aaa(0), bbb(0), ccc(0);
    double disc(0), ratio(0), sigman(0), res(0);
    double dresdr(0), dratio(0), ds(0);

    sigma = smax/ds1;
    nex = nn-1;
    rnex = double(nex);
    rni = 1.0/rnex;

    //-- solve quadratic for initial geometric ratio guess
    aaa = rnex*(rnex-1.0)*(rnex-2.0) / 6.0;
    bbb = rnex*(rnex-1.0) / 2.0;
    ccc = rnex - sigma;

    disc = bbb*bbb - 4.0*aaa*ccc;
    disc = std::max(0.0, disc);

    if(nex<=1)
    {
        std::string str("setexp: cannot fill array.  n too small\n");
        writeString(str, true);
        return false;
    }
    else {
        if(nex==2)    ratio = -ccc/bbb  +  1.0;
        else  ratio = (-bbb + sqrt(disc))/(2.0*aaa)  +  1.0;
    }
    if(ratio==1.0) goto stop11;

    //-- newton iteration for actual geometric ratio
    for (iter=1; iter<=100; iter++)
    {
        sigman = (pow(ratio,double(nex)) - 1.0) / (ratio - 1.0);
        res = pow(sigman,rni) - pow(sigma,rni);
        dresdr = rni*pow(sigman,rni)
                * (rnex*pow(ratio,double(nex-1)) - sigman)
                / (pow(ratio,double(nex)) - 1.0);

        dratio = -res/dresdr;
        ratio = ratio + dratio;

        if(fabs(dratio) < 1.0e-5)     goto stop11;

    }


    str = "Setexp: Convergence failed.  Continuing anyway ...\n";
    writeString(str, true);


    //-- set up stretched array using converged geometric ratio
stop11:
    s[1] = 0.0;
    ds = ds1;
    for (n=2; n<= nn; n++)
    {
        s[n] = s[n-1] + ds;
        ds = ds*ratio;
    }
    return true;
}


bool XFoil::setMach()
{
    mrcl(1.0, minf_cl, reinf_cl);
    comset();
    cpcalc(n,qinv,qinf,minf,cpi);
    if(lvisc) {
        cpcalc(n+nw,qvis,qinf,minf,cpv);
    }
    clcalc(xcmref,ycmref);
    cdcalc();
    lvconv = false;
    return true;
}


/**      Calculates x(ss)
*       xs array must have been calculated by spline */
double XFoil::seval(double ss, double x[], double xs[], double s[], int n)
{
    int ilow(0), i(0), imid(0);
    double ds(0), t(0), cx1(0), cx2(0);

    ilow = 1;
    i = n;

    while(i-ilow>1)
    {
        imid = int((i+ilow)/2);
        if(ss < s[imid]) i = imid;
        else ilow = imid;
    }

    ds = s[i] - s[i-1];
    t = (ss - s[i-1]) / ds;
    cx1 = ds*xs[i-1] - x[i] + x[i-1];
    cx2 = ds*xs[i]   - x[i] + x[i-1];
    return  t*x[i] + (1.0-t)*x[i-1] + (t-t*t)*((1.0-t)*cx1 - t*cx2);
}


/** returns the absolute value of "a" x sign(b) */
double XFoil::sign(double a, double b)
{
    if(b>=0.0) return  fabs(a);
    else       return -fabs(a);
}


/**
 *        Calculates the "inverse" spline function s(x).
 *        Since s(x) can be multi-valued or not defined,
 *        this is not a "black-box" routine.  The calling
 *        program must pass via si a sufficiently good
 *        initial guess for s(xi).
 *
 *        xi       specified x value       (input)
 *        si       calculated s(xi) value  (input,output)
 *        x,xs,s  usual spline arrays       (input)
 */
bool XFoil::sinvrt(double &si, double xi, double x[], double xs[], double s[], int n)
{
    int iter(0);
    double sisav(0), res(0), resp(0), ds(0);

    sisav = si;

    for(iter=1;iter<= 10;iter++)
    {
        res  = seval(si,x,xs,s,n) - xi;
        resp = deval(si,x,xs,s,n);
        ds = -res/resp;
        si = si + ds;
        if(fabs(ds/(s[n]-s[1]))< 1.0e-5) return true;
    }

    std::string str = "Sinvrt: spline inversion failed, input value returned\n";
    writeString(str, true);
    si = sisav;

    return false;
}


/**
 *      Converges to specified alpha.
 */
bool XFoil::specal()
{
    double minf_clm(0), msq_clm(0), reinf_clm(0);
    double clm(0), dclm(0), clm1(0);
    int i(0), irlx(0), itcl(0);

    //---- calculate surface vorticity distributions for alpha = 0, 90 degrees
    if(!lgamu || !lqaij) ggcalc();

    cosa = cos(alfa);
    sina = sin(alfa);

    //---- superimpose suitably weighted  alpha = 0, 90  distributions
    for (i=1; i<=n; i++){
        gam[i]   =  cosa*gamu[i][1] + sina*gamu[i][2];
        gam_a[i] = -sina*gamu[i][1] + cosa*gamu[i][2];
    }

    psio = cosa*gamu[n+1][1] + sina*gamu[n+1][2];

    tecalc();
    qiset();

    //---- set initial guess for the newton variable clm
    clm = 1.0;

    //---- set corresponding  m(clm), re(clm)
    mrcl(clm,minf_clm,reinf_clm);
    comset();

    //---- set corresponding cl(m)
    clcalc(xcmref,ycmref);
    //---- iterate on clm
    bool bConv = false;
    for (itcl=1; itcl<= 20;itcl++)
    {
        msq_clm = 2.0*minf*minf_clm;
        dclm = (cl - clm)/(1.0 - cl_msq*msq_clm);

        clm1 = clm;
        rlx = 1.0;

        //------ under-relaxation loop to avoid driving m(cl) above 1
        for (irlx=1; irlx<=12; irlx++){

            clm = clm1 + rlx*dclm;

            //-------- set new freestream mach m(clm)
            mrcl(clm,minf_clm,reinf_clm);

            //-------- if mach is ok, go do next newton iteration
            if(matyp==1 || minf==0.0 || minf_clm!=0.0) break;// goto 91

            rlx = 0.5*rlx;
        }


        //------ set new cl(m)
        comset();
        clcalc(xcmref,ycmref);

        if(fabs(dclm)<=1.0e-6) {
            bConv = true;
            break;
        }
    }
    if(!bConv)
    {
        std::string str("Specal:  MInf convergence failed\n");
        writeString(str, true);
        return false;
    }

    //---- set final mach, cl, cp distributions, and hinge moment
    mrcl(cl,minf_cl,reinf_cl);
    comset();
    clcalc(xcmref,ycmref);
    /*
    if (!cpcalc(n,qinv,qinf,minf,cpi)) return false;// no need to carry on
    if(lvisc) {
        if(!cpcalc(n+nw,qvis,qinf,minf,cpv)) return false;// no need to carry on
        if(!cpcalc(n+nw,qinv,qinf,minf,cpi)) return false;// no need to carry on
    }
    else   if (!cpcalc(n,qinv,qinf,minf,cpi)) return false;// no need to carry on
*/
    cpcalc(n,qinv,qinf,minf,cpi);
    if(lvisc)
    {
        cpcalc(n+nw,qvis,qinf,minf,cpv);
        cpcalc(n+nw,qinv,qinf,minf,cpi);
    }
    else
        cpcalc(n,qinv,qinf,minf,cpi);

    if(lflap) mhinge();

    //Added techwinder to get inviscid q after viscous calculation
    for (i=1; i<=n; i++){
        qgamm[i] = gam[i];
    }
    // end techwinder addition

    return true;
}


bool XFoil::speccl()
{
    //-----------------------------------------
    //     converges to specified inviscid cl.
    //-----------------------------------------
    double dalfa(0);
    int i(0), ital(0);

    //---- calculate surface vorticity distributions for alpha = 0, 90 degrees
    if(!lgamu || !lqaij) ggcalc();

    //---- set freestream mach from specified cl -- mach will be held fixed
    mrcl(clspec,minf_cl,reinf_cl);
    comset();

    //---- current alpha is the initial guess for newton variable alfa
    cosa = cos(alfa);
    sina = sin(alfa);

    for (i=1; i<=n; i++){
        gam[i]   =  cosa*gamu[i][1] + sina*gamu[i][2];
        gam_a[i] = -sina*gamu[i][1] + cosa*gamu[i][2];
    }
    psio = cosa*gamu[n+1][1] + sina*gamu[n+1][2];

    //---- get corresponding cl, cl_alpha, cl_mach
    clcalc(xcmref,ycmref);

    //---- newton loop for alpha to get specified inviscid cl
    bool bConv = false;
    for(ital=1; ital<= 20; ital++)
    {
        dalfa = (clspec - cl) / cl_alf;
        rlx = 1.0;

        alfa = alfa + rlx*dalfa;

        //------ set new surface speed distribution
        cosa = cos(alfa);
        sina = sin(alfa);
        for (i=1; i<=n; i++){
            gam[i]   =  cosa*gamu[i][1] + sina*gamu[i][2];
            gam_a[i] = -sina*gamu[i][1] + cosa*gamu[i][2];
        }
        psio = cosa*gamu[n+1][1] + sina*gamu[n+1][2];

        //------ set new cl(alpha)
        clcalc(xcmref,ycmref);

        if(fabs(dalfa)<=1.0e-6){
            bConv = true;
            break;
        }
    }
    if(!bConv)
    {
        std::string str = "Speccl:  cl convergence failed";
        writeString(str, true);
        return false;
    }

    //---- set final surface speed and cp distributions
    tecalc();
    qiset();
    /*
    if(lvisc) {
        if(!cpcalc(n+nw,qvis,qinf,minf,cpv)){
            return false;
        }
        if(!cpcalc(n+nw,qinv,qinf,minf,cpi)){
            return false;
        }
    }
    else{
        if(!cpcalc(n,qinv,qinf,minf,cpi)){
            return false;
        }
    }
*/
    if(lvisc) {
        cpcalc(n+nw,qvis,qinf,minf,cpv);
        cpcalc(n+nw,qinv,qinf,minf,cpi);

    }
    else{
        cpcalc(n,qinv,qinf,minf,cpi);
    }

    if(lflap) mhinge();

    return true;
}


/** -------------------------------------------------------
 *      Calculates spline coefficients for x(s).          |
 *       A simple averaging of adjacent segment slopes    |
 *      is used to achieve non-oscillatory curve.         |
 *      End conditions are set by end segment slope.      |
 *      To evaluate the spline at some value of s,        |
 *      use seval and/or deval.                           |
 *                                                        |
 *      s        independent variable array (input)       |
 *      x        dependent variable array   (input)       |
 *      xs       dx/ds array                (calculated)  |
 *      n        number of points           (input)       |
 *                                                        |
 * -------------------------------------------------------*/
void XFoil::splina(double x[], double xs[], double s[], int n)
{
    bool lend=false;
    double ds(0), dx(0), xs1(0), xs2(0);
    xs1 = xs2 = 0.0;

    lend = true;
    for (int i=1; i<=n-1; i++)
    {
        ds = s[i+1]-s[i];
        if (fabs(ds)<1.e-10)
        {//=0.0
            xs[i] = xs1;
            lend = true;
        }
        else
        {
            dx = x[i+1]-x[i];
            xs2 = dx / ds;
            if (lend)
            {
                xs[i] = xs2;
                lend = false;
            }
            else xs[i] = 0.5*(xs1 + xs2);
        }
        xs1 = xs2;
    }
    xs[n] = xs1;
}


/** -------------------------------------------------------
 *      Calculates spline coefficients for x(s).          |
 *      Specified 1st derivative and/or usual zero 2nd    |
 *      derivative end conditions are used.               |
 *                                                        |
 *      To evaluate the spline at some value of s,        |
 *      use seval and/or deval.                           |
 *                                                        |
 *      s        independent variable array (input)       |
 *      x        dependent variable array   (input)       |
 *      xs       dx/ds array                (calculated)  |
 *      n        number of points           (input)       |
 *      xs1,xs2  endpoint derivatives       (input)       |
 *               if = 999.0, then usual zero second       |
 *               derivative end condition(s) are used     |
 *               if = -999.0, then zero third             |
 *               derivative end condition(s) are used     |
 *                                                        |
 * ------------------------------------------------------- */
bool XFoil::splind(double *x, double *xs, double *s, int n, double xs1, double xs2)
{
    int nmax=600;
    double a[601], b[601], c[601];
    double dsm(0), dsp(0);
    memset(a, 0, 601*sizeof(double));
    memset(b, 0, 601*sizeof(double));
    memset(c, 0, 601*sizeof(double));

    if(n>nmax)
    {
        std::string str = "splind: array overflow, increase nmax";
        writeString(str, true);
        return false;
    }
    for(int i=2; i<=n-1;i++)
    {
        dsm = s[i] - s[i-1];
        dsp = s[i+1] - s[i];
        b[i] = dsp;
        a[i] = 2.0*(dsm+dsp);
        c[i] = dsm;
        xs[i] = 3.0*((x[i+1]-x[i])*dsm/dsp + (x[i]-x[i-1])*dsp/dsm);
    }

    if(xs1>=998.0)
    {
        //----- set zero second derivative end condition
        a[1] = 2.0;
        c[1] = 1.0;
        xs[1] = 3.0*(x[2]-x[1]) / (s[2]-s[1]);
    }
    else {
        if(xs1<=-998.0) {
            //----- set zero third derivative end condition
            a[1] = 1.0;
            c[1] = 1.0;
            xs[1] = 2.0*(x[2]-x[1]) / (s[2]-s[1]);
        }
        else{
            //----- set specified first derivative end condition
            a[1] = 1.0;
            c[1] = 0.0;
            xs[1] = xs1;
        }
    }

    if(xs2>=998.0)
    {
        b[n] = 1.0;
        a[n] = 2.0;
        xs[n] = 3.0*(x[n]-x[n-1]) / (s[n]-s[n-1]);
    }
    else{
        if(xs2<=-998.0) {
            b[n] = 1.0;
            a[n] = 1.0;
            xs[n] = 2.0*(x[n]-x[n-1]) / (s[n]-s[n-1]);
        }
        else{
            a[n] = 1.0;
            b[n] = 0.0;
            xs[n] = xs2;
        }
    }

    if(n==2 && xs1<=-998.0 && xs2<=-998.0)
    {
        b[n] = 1.0;
        a[n] = 2.0;
        xs[n] = 3.0*(x[n]-x[n-1]) / (s[n]-s[n-1]);
    }

    //---- solve for derivative array xs
    trisol(a,b,c,xs,n);
    return true;
}


void XFoil::sss(double ss, double *s1, double *s2, double del, double xbf, double ybf,
                double x[], double xp[], double y[], double yp[], double s[], int n, int iside)
{
//      dimension x(*),xp(*),y(*),yp(*),s(*)
//----------------------------------------------------------------
//     returns arc length points s1,s2 at flap surface break
//     locations.  s1 is on fixed airfoil part, s2 is on flap.
//     the points are defined according to two cases:
//
//
//     if del > 0:  surface will be eliminated in s1 < s < s2
//
//     returns the arc length values s1,s2 of the endpoints
//     of the airfoil surface segment which "disappears" as a
//     result of the flap deflection.  the line segments between
//     these enpoints and the flap hinge point (xbf,ybf) have
//     an included angle of del.  del is therefore the flap
//     deflection which will join up the points at s1,s2.
//     ss is an approximate arc length value near s1 and s2.
//     it is used as an initial guess for the newton loop
//     for s1 and s2.
//
//
//     if del = 0:  surface will be created at s = s1 = s2
//
//     if del=0, then s1,s2 will cooincide, and will be located
//     on the airfoil surface where the segment joining the
//     point at s1,s2 and the hinge point is perpendicular to
//     the airfoil surface.  this will be the point where the
//     airfoil surface must be broken to permit a gap to open
//     as a result of the flap deflection.
//
//----------------------------------------------------------------
//
//     Translations errors from fortran to C
//     pointed out by Nicolas C. 2018/11/17
//
//----------------------------------------------------------------
    double rsq=0, x1p=0,y1p=0, x2p=0, y2p=0;
    double r1sq=0, r2sq=0, rrsq,rr=0,r1_s1=0,r2_s2=0,rr_s1=0,rr_s2=0,rs1=0, rs2=0;
    double a11=0, a12=0, a21=0, a22=0;
    double x1=0, x2=0, y1=0, y2=0;// also common variables...hmmm
    double x1pp=0, y1pp=0, x2pp=0, y2pp=0, xtot=0, ytot=0;
    double det=0, ds1=0, ds2=0, eps=0, stot=0, sind=0, ssgn=0, r1=0, r2=0;
//---- convergence epsilon
    eps = 1.0e-5;

    stot = fabs( s[n] - s[1] );

    sind = sin(0.5*fabs(del));

    ssgn = 1.0;
    if(iside==1) ssgn = -1.0;

    //---- initial guesses for s1, s2

    r1 = (seval(ss,x,xp,s,n)-xbf);
    r2 = (seval(ss,y,yp,s,n)-ybf);
    rsq = r1*r1 + r2*r2;
    *s1 = ss - (sind*sqrt(rsq) + eps*stot)*ssgn;
    *s2 = ss + (sind*sqrt(rsq) + eps*stot)*ssgn;

    //---- newton iteration loop
//    for (int iter = 1; iter <= 10; iter++)
    int  iter;
    for( iter = 1; iter <= 10; iter++)
    {
        x1  = seval(*s1,x,xp,s,n);
        x1p = deval(*s1,x,xp,s,n);
        y1  = seval(*s1,y,yp,s,n);
        y1p = deval(*s1,y,yp,s,n);

        x2  = seval(*s2,x,xp,s,n);
        x2p = deval(*s2,x,xp,s,n);
        y2  = seval(*s2,y,yp,s,n);
        y2p = deval(*s2,y,yp,s,n);

        r1sq = (x1-xbf)*(x1-xbf) + (y1-ybf)*(y1-ybf);
        r2sq = (x2-xbf)*(x2-xbf) + (y2-ybf)*(y2-ybf);
        r1 = sqrt(r1sq);
        r2 = sqrt(r2sq);

        rrsq = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
        rr = sqrt(rrsq);

        if(r1<=eps*stot || r2<=eps*stot) {
            *s1 = ss;
            *s2 = ss;
            return;
        }

        r1_s1 = (x1p*(x1-xbf) + y1p*(y1-ybf))/r1;
        r2_s2 = (x2p*(x2-xbf) + y2p*(y2-ybf))/r2;

        if(sind>0.01)
        {

            if(rr==0.0) return;

            rr_s1 =  (x1p*(x1-x2) + y1p*(y1-y2))/rr;
            rr_s2 = -(x2p*(x1-x2) + y2p*(y1-y2))/rr;

                //------- residual 1: set included angle via dot product
            rs1 = ((xbf-x1)*(x2-x1) + (ybf-y1)*(y2-y1))/rr - sind*r1;
            a11 = ((xbf-x1)*( -x1p) + (ybf-y1)*( -y1p))/rr
                + ((  -x1p)*(x2-x1) + (  -y1p)*(y2-y1))/rr
                - ((xbf-x1)*(x2-x1) + (ybf-y1)*(y2-y1))*rr_s1/rrsq
                - sind*r1_s1;
            a12 = ((xbf-x1)*(x2p  ) + (ybf-y1)*(y2p  ))/rr
                - ((xbf-x1)*(x2-x1) + (ybf-y1)*(y2-y1))*rr_s2/rrsq;

            //------- residual 2: set equal length segments
            rs2 = r1 - r2;
            a21 = r1_s1;
            a22 =    - r2_s2;
        }
        else
        {

            //------- residual 1: set included angle via small angle approximation
//            rs1 = (r1 + r2)*sind + (s1 - s2)*ssgn;
            rs1 = (r1 + r2)*sind + (*s1 - *s2)*ssgn; // corrected by Nicolas C. 2018/11/17
            a11 =  r1_s1 *sind + ssgn;
            a12 =  r2_s2 *sind - ssgn;

            //------- residual 2: set vector sum of line segments beteen the
            //-       endpoints and flap hinge to be perpendicular to airfoil surface.
            x1pp = d2val(*s1,x,xp,s,n);
            y1pp = d2val(*s1,y,yp,s,n);
            x2pp = d2val(*s2,x,xp,s,n);
            y2pp = d2val(*s2,y,yp,s,n);

            xtot = x1+x2 - 2.0*xbf;
            ytot = y1+y2 - 2.0*ybf;

            rs2 = xtot*(x1p+x2p) + ytot*(y1p+y2p);
            a21 =  x1p*(x1p+x2p) +  y1p*(y1p+y2p) + xtot*x1pp + ytot*y1pp;
            a22 =  x2p*(x1p+x2p) +  y2p*(y1p+y2p) + xtot*x2pp + ytot*y2pp;

        }

        det =   a11*a22 - a12*a21;
        ds1 = -(rs1*a22 - a12*rs2) / det;
        ds2 = -(a11*rs2 - rs1*a21) / det;

        ds1 = std::min( ds1 , 0.01*stot );
        ds1 = std::max( ds1 , -.01*stot );
        ds2 = std::min( ds2 , 0.01*stot );
        ds2 = std::max( ds2 , -.01*stot );

        *s1 = *s1 + ds1;
        *s2 = *s2 + ds2;
        if(fabs(ds1)+fabs(ds2) < eps*stot ) break; // newton loop //go to 11
    }//10 continue

     // corrected by Nicolas C. 2018/11/17
    if (fabs(ds1) + fabs(ds2) >= eps*stot)
    {
        //      write(*,*) 'sss: failed to converge subtending angle points'
        *s1 = ss;
        *s2 = ss;
    }

//    11 continue

    //---- make sure points are identical if included angle is zero.
    if(del<=0.00001)
    {
        *s1 = 0.5*(*s1+*s2);
        *s2 = *s1;
    }
}


bool XFoil::stepbl()
{
    //techwinder : can't think of a more elegant way to do this, and too lazy to search
    x1     = x2;
    u1     = u2;
    theta1 = theta2;
    d1     = d2;
    s1     = s2;
    ampl1  = ampl2;
    u1_uei = u2_uei;
    u1_ms  = u2_ms;
    dw1    = dw2;
    h1     = h2;
    h1_t1  = h2_t2;
    h1_d1  = h2_d2;
    m1     = m2;
    m1_u1  = m2_u2;
    m1_ms  = m2_ms;
    r1     = r2;
    r1_u1  = r2_u2;
    r1_ms  = r2_ms;
    v1     = v2;
    v1_u1  = v2_u2;
    v1_ms  = v2_ms;
    v1_re  = v2_re;
    hk1    = hk2;
    hk1_u1 = hk2_u2;
    hk1_t1 = hk2_t2;
    hk1_d1 = hk2_d2;
    hk1_ms = hk2_ms;
    hs1    = hs2;
    hs1_u1 = hs2_u2;
    hs1_t1 = hs2_t2;
    hs1_d1 = hs2_d2;
    hs1_ms = hs2_ms;
    hs1_re = hs2_re;
    hc1    = hc2;
    hc1_u1 = hc2_u2;
    hc1_t1 = hc2_t2;
    hc1_d1 = hc2_d2;
    hc1_ms = hc2_ms;
    rt1    = rt2;
    rt1_u1 = rt2_u2;
    rt1_t1 = rt2_t2;
    rt1_ms = rt2_ms;
    rt1_re = rt2_re;
    cf1    = cf2;
    cf1_u1 = cf2_u2;
    cf1_t1 = cf2_t2;
    cf1_d1 = cf2_d2;
    cf1_ms = cf2_ms;
    cf1_re = cf2_re;
    di1    = di2;
    di1_u1 = di2_u2;
    di1_t1 = di2_t2;
    di1_d1 = di2_d2;
    di1_s1 = di2_s2;
    di1_ms = di2_ms;
    di1_re = di2_re;
    us1    = us2;
    us1_u1 = us2_u2;
    us1_t1 = us2_t2;
    us1_d1 = us2_d2;
    us1_ms = us2_ms;
    us1_re = us2_re;
    cq1    = cq2;
    cq1_u1 = cq2_u2;
    cq1_t1 = cq2_t2;
    cq1_d1 = cq2_d2;
    cq1_ms = cq2_ms;
    cq1_re = cq2_re;
    de1    = de2;
    de1_u1 = de2_u2;
    de1_t1 = de2_t2;
    de1_d1 = de2_d2;
    de1_ms = de2_ms;
    return true;
}



bool XFoil::stfind()
{
    //-----------------------------------------
    //     locates stagnation point arc length
    //     location sst and panel index ist.
    //-----------------------------------------
    double dgam=0, ds=0;
    int i=0;
    bool bFound = false;

    for(i=1; i<=n-1;i++)
    {
        if(gam[i]>=0.0 && gam[i+1]<0.0)
        {
            bFound = true;
            break;
        }
    }

    if(!bFound)
    {
        std::string str = "stfind: Stagnation point not found. Continuing ...\n";
        writeString(str, true);
        i = n/2;
    }

    //stop11:
    ist = i;
    dgam = gam[i+1] - gam[i];
    ds = s[i+1] - s[i];

    //---- evaluate so as to minimize roundoff for very small gam[i] or gam[i+1]
    if(gam[i] < -gam[i+1])
        sst = s[i]   - ds*(gam[i]  /dgam);
    else
        sst = s[i+1] - ds*(gam[i+1]/dgam);


    //---- tweak stagnation point if it falls right on a node (very unlikely)
    if(sst <= s[i]  )
        sst = s[i]   + 0.0000001;
    if(sst >= s[i+1])
        sst = s[i+1] - 0.0000001;

    sst_go = (sst  - s[i+1])/dgam;
    sst_gp = (s[i] - sst   )/dgam;

    return true;

}


bool XFoil::stmove()
{

    //--------------------------------------------------
    //    moves stagnation point location to new panel.
    //---------------------------------------------------
    int ibl=0, idif=0, istold=0, is=0;
    double dudx=0;

    //-- locate new stagnation point arc length sst from gam distribution
    istold = ist;
    stfind();

    if(istold==ist)
    {

        //--- recalculate new arc length array
        xicalc();
    }

    else
    {

        //       write(*,*) 'stmove: resetting stagnation point'

        //--- set new bl position -> panel position  pointers
        iblpan();

        //--- set new inviscid bl edge velocity uinv from qinv
        uicalc();

        //--- recalculate new arc length array
        xicalc();

        //--- set  bl position -> system line  pointers
        iblsys();

        if(ist>istold) {
            //---- increase in number of points on top side (is=1)
            idif = ist-istold;


            itran[1] = itran[1] + idif;
            itran[2] = itran[2] - idif;

            //---- move top side bl variables downstream
            for (ibl=nbl[1];ibl>= idif+2;ibl--){

                ctau[ibl][1] = ctau[ibl-idif][1];
                thet[ibl][1] = thet[ibl-idif][1];
                dstr[ibl][1] = dstr[ibl-idif][1];
                uedg[ibl][1] = uedg[ibl-idif][1];
            }

            //---- set bl variables between old and new stagnation point
            dudx = uedg[idif+2][1]/xssi[idif+2][1];
            for (ibl=idif+1;ibl>= 2;ibl--)
            {
                ctau[ibl][1] = ctau[idif+2][1];
                thet[ibl][1] = thet[idif+2][1];
                dstr[ibl][1] = dstr[idif+2][1];
                uedg[ibl][1] = dudx * xssi[ibl][1];
            }

            //---- move bottom side bl variables upstream
            for (ibl=2; ibl<= nbl[2];ibl++){
                ctau[ibl][2] = ctau[ibl+idif][2];
                thet[ibl][2] = thet[ibl+idif][2];
                dstr[ibl][2] = dstr[ibl+idif][2];
                uedg[ibl][2] = uedg[ibl+idif][2];
            }
        }
        else{
            //---- increase in number of points on bottom side (is=2)
            idif = istold-ist;

            itran[1] = itran[1] - idif;
            itran[2] = itran[2] + idif;

            //---- move bottom side bl variables downstream
            for (ibl=nbl[2];ibl>= idif+2;ibl--){

                ctau[ibl][2] = ctau[ibl-idif][2];
                thet[ibl][2] = thet[ibl-idif][2];
                dstr[ibl][2] = dstr[ibl-idif][2];
                uedg[ibl][2] = uedg[ibl-idif][2];
            }

            //---- set bl variables between old and new stagnation point
            dudx = uedg[idif+2][2]/xssi[idif+2][2];
            for (ibl=idif+1;ibl>= 2;ibl--){

                ctau[ibl][2] = ctau[idif+2][2];
                thet[ibl][2] = thet[idif+2][2];
                dstr[ibl][2] = dstr[idif+2][2];
                uedg[ibl][2] = dudx * xssi[ibl][2];
            }

            //---- move top side bl variables upstream
            for(ibl=2;ibl<= nbl[1];ibl++){
                ctau[ibl][1] = ctau[ibl+idif][1];
                thet[ibl][1] = thet[ibl+idif][1];
                dstr[ibl][1] = dstr[ibl+idif][1];
                uedg[ibl][1] = uedg[ibl+idif][1];
            }
        }

    }

    //-- set new mass array since ue has been tweaked
    for (is=1;is<= 2;is++)
    {
        for(ibl=2; ibl<= nbl[is];ibl++)
            mass[ibl][is] = dstr[ibl][is]*uedg[ibl][is];

    }

    return true;
}


bool XFoil::tecalc()
{
    //-------------------------------------------
    //     calculates total and projected TE
    //     areas and TE panel strengths.
    //-------------------------------------------

    double scs=0, sds=0;
    //---- set te base vector and te bisector components
    double dxte = x[1] - x[n];
    double dyte = y[1] - y[n];
    double dxs = 0.5*(-xp[1] + xp[n]);
    double dys = 0.5*(-yp[1] + yp[n]);

    //---- normal and streamwise projected TE gap areas
    ante = dxs*dyte - dys*dxte;
    aste = dxs*dxte + dys*dyte;

    //---- total TE gap area
    dste = sqrt(dxte*dxte + dyte*dyte);

    sharp = dste < 0.0001*chord;

    if(sharp) {
        scs = 1.0;
        sds = 0.0;
    }
    else{
        scs = ante/dste;
        sds = aste/dste;
    }

    //---- TE panel source and vorticity strengths
    sigte = 0.5*(gam[1] - gam[n])*scs;
    gamte = -.5*(gam[1] - gam[n])*sds;

    //    sigte_a = 0.5*(gam_a[1] - gam_a[n])*scs;
    //    gamte_a = -.5*(gam_a[1] - gam_a[n])*sds;

    return true;

}


bool XFoil::tesys(double cte, double tte, double dte){
    //--------------------------------------------------------
    //       sets up "dummy" bl system between airfoil te point
    //       and first wake point infinitesimally behind te.
    //--------------------------------------------------------

    for(int k=1;k<= 4;k++)
    {
        vsrez[k] = 0.0;
        vsm[k]     = 0.0;
        vsr[k]     = 0.0;
        vsx[k]     = 0.0;
        for (int l=1; l<=5;l++){
            vs1[k][l] = 0.0;
            vs2[k][l] = 0.0;
        }
    }

    blvar(3);

    vs1[1][1] = -1.0;
    vs2[1][1] = 1.0;
    vsrez[1] = cte - s2;

    vs1[2][2] = -1.0;
    vs2[2][2] = 1.0;
    vsrez[2] = tte - theta2;

    vs1[3][3] = -1.0;
    vs2[3][3] = 1.0;
    vsrez[3] = dte - d2 - dw2;

    return true;
}



bool XFoil::trchek()
{
    //----------------------------------------------------------------
    //     new second-order version:  december 1994.
    //
    //     checks if transition occurs in the current interval x1..x2.
    //     if transition occurs, then set transition location xt, and
    //     its sensitivities to "1" and "2" variables.  if no transition,
    //     set amplification ampl2.
    //
    //     solves the implicit amplification equation for n2:
    //
    //       n2 - n1     n'(xt,nt) + n'(x1,n1)
    //       -------  =  ---------------------
    //       x2 - x1               2
    //
    //     in effect, a 2-point central difference is used between
    //     x1..x2 (no transition), or x1..xt (transition).  the switch
    //     is done by defining xt,nt in the equation above depending
    //     on whether n2 exceeds ncrit.
    //
    //  if n2<ncrit:  nt=n2    , xt=x2                  (no transition)
    //
    //  if n2>ncrit:  nt=ncrit , xt=(ncrit-n1)/(n2-n1)  (transition)
    //
    //----------------------------------------------------------------
    std::string str;

    int itam=0;
    double ax_hk1=0.0, ax_t1=0.0, ax_a1=0.0, ax_hk2=0.0, ax_t2=0.0, ax_rt2=0.0, ax_a2=0.0;
    double amplt=0.0, sfa=0.0, sfa_a1=0.0, sfa_a2=0.0, sfx=0.0;
    double sfx_x1=0.0, sfx_x2=0.0, sfx_xf=0.0;
    double tt=0.0, dt=0.0, ut=0.0,  amsave=0.0;
    double ax=0.0, ax_rt1=0.0,res=0.0, res_a2=0.0;
    double da2=0.0, dxt=0.0, tt_t1=0.0, dt_d1=0.0, ut_u1=0.0;
    double tt_t2=0.0, dt_d2=0.0, ut_u2=0.0, tt_a1=0.0, dt_a1=0.0;
    double ut_a1=0.0, tt_x1=0.0, dt_x1=0.0, ut_x1=0.0, tt_x2=0.0, dt_x2=0.0, ut_x2=0.0;
    double ax_d1=0.0, ax_u1=0.0, ax_x1=0.0, ax_d2=0.0, ax_u2=0.0, ax_x2=0.0, ax_ms=0.0, ax_re=0.0;
    double z_ax=0.0, z_a1=0.0, z_t1=0.0, z_d1=0.0, z_u1=0.0, z_x1=0.0, z_a2=0.0, z_t2=0.0, z_d2=0.0, z_u2=0.0, z_x2=0.0, z_ms=0.0, z_re=0.0;
    //    double ax_xf, tt_xf, dt_xf, ut_xf, z_xf;
    double ax_at=0, ax_rtt=0, ax_tt=0, ax_hkt=0, amplt_a2=0, wf1=0, wf1_a1=0, wf1_a2=0, wf1_xf=0, wf1_x1=0, wf1_x2=0;
    double wf2=0, wf2_a1=0, wf2_a2=0, wf2_xf=0, wf2_x1=0, wf2_x2=0, xt_a2=0, dt_a2=0, tt_a2=0;
    double ut_a2=0, hkt=0, hkt_tt=0, hkt_dt=0, hkt_ut=0, hkt_ms=0, rtt_tt=0, rtt_ut=0, rtt_ms=0, rtt=0, rtt_re=0;
    double daeps=0.00005;

    //---- save variables and sensitivities at ibl ("2") for future restoration
    saveblData(2);

    //---- calculate average amplification rate ax over x1..x2 interval
    axset(hk1, theta1, rt1, ampl1, hk2, theta2, rt2, ampl2, amcrit,
          ax, ax_hk1, ax_t1, ax_rt1, ax_a1, ax_hk2, ax_t2, ax_rt2, ax_a2 );


    //---- set initial guess for iterate n2 (ampl2) at x2
    ampl2 = ampl1 + ax*(x2-x1);

    //---- solve implicit system for amplification ampl2
    for(itam=1; itam<=30;itam++)
    {

        //---- define weighting factors wf1,wf2 for defining "t" quantities from 1,2
        if(ampl2 <= amcrit)
        {
            //------ there is no transition yet,  "t" is the same as "2"
            amplt    = ampl2;
            amplt_a2 = 1.0;
            sfa    = 1.0;
            sfa_a1 = 0.0;
            sfa_a2 = 0.0;
        }
        else
        {
            //------ there is transition in x1..x2, "t" is set from n1, n2
            amplt    = amcrit;
            amplt_a2 = 0.0;
            sfa    = (amplt - ampl1)/(ampl2-ampl1);
            sfa_a1 = ( sfa  - 1.0  )/(ampl2-ampl1);
            sfa_a2 = (      - sfa  )/(ampl2-ampl1);
        }

        if(xiforc<x2)
        {
            sfx    = (xiforc - x1 )/(x2-x1);
            sfx_x1 = (sfx    - 1.0)/(x2-x1);
            sfx_x2 = (       - sfx)/(x2-x1);
            sfx_xf =  1.0          /(x2-x1);
        }
        else
        {
            sfx    = 1.0;
            sfx_x1 = 0.0;
            sfx_x2 = 0.0;
            sfx_xf = 0.0;
        }

        //---- set weighting factor from free or forced transition
        if(sfa<sfx)
        {
            wf2    = sfa;
            wf2_a1 = sfa_a1;
            wf2_a2 = sfa_a2;
            wf2_x1 = 0.0;
            wf2_x2 = 0.0;
            wf2_xf = 0.0;
        }
        else
        {
            wf2    = sfx;
            wf2_a1 = 0.0;
            wf2_a2 = 0.0;
            wf2_x1 = sfx_x1;
            wf2_x2 = sfx_x2;
            wf2_xf = sfx_xf;
        }

        wf1    = 1.0 - wf2;
        wf1_a1 =     - wf2_a1;
        wf1_a2 =     - wf2_a2;
        wf1_x1 =     - wf2_x1;
        wf1_x2 =     - wf2_x2;
        wf1_xf =     - wf2_xf;

        //---- interpolate bl variables to xt
        xt    = x1*wf1    + x2*wf2;
        tt    = theta1*wf1    + theta2*wf2;
        dt    = d1*wf1    + d2*wf2;
        ut    = u1*wf1    + u2*wf2;

        xt_a2 = x1*wf1_a2 + x2*wf2_a2;
        tt_a2 = theta1*wf1_a2 + theta2*wf2_a2;
        dt_a2 = d1*wf1_a2 + d2*wf2_a2;
        ut_a2 = u1*wf1_a2 + u2*wf2_a2;

        //---- temporarily set "2" variables from "t" for blkin
        x2 = xt;
        theta2 = tt;
        d2 = dt;
        u2 = ut;

        //---- calculate laminar secondary "t" variables hkt, rtt
        blkin();

        hkt    = hk2;
        hkt_tt = hk2_t2;
        hkt_dt = hk2_d2;
        hkt_ut = hk2_u2;
        hkt_ms = hk2_ms;

        rtt    = rt2;
        rtt_tt = rt2_t2;
        rtt_ut = rt2_u2;
        rtt_ms = rt2_ms;
        rtt_re = rt2_re;

        //---- restore clobbered "2" variables, except for ampl2
        amsave = ampl2;

        restoreblData(2);

        ampl2 = amsave;

        //---- calculate amplification rate ax over current x1-xt interval
        axset(hk1, theta1, rt1, ampl1, hkt, tt, rtt, amplt,
              amcrit,ax, ax_hk1, ax_t1, ax_rt1, ax_a1,
              ax_hkt, ax_tt, ax_rtt, ax_at);


        //---- punch out early if there is no amplification here
        if(ax <= 0.0) goto stop101;

        //---- set sensitivity of ax(a2)
        ax_a2 = (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_a2
                + (ax_hkt*hkt_dt                        )*dt_a2
                + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_a2
                +  ax_at                                 *amplt_a2;

        //---- residual for implicit ampl2 definition (amplification equation)
        res    = ampl2 - ampl1 - ax   *(x2-x1);
        res_a2 = 1.0          - ax_a2*(x2-x1);

        da2 = -res/res_a2;

        rlx = 1.0;
        dxt = xt_a2*da2;

        if (rlx*fabs(dxt/(x2-x1)) > 0.05) rlx = 0.05*fabs((x2-x1)/dxt);

        if(rlx*fabs(da2)         > 1.0)  rlx = 1.0 *fabs(   1.0 /da2);



        //---- check if converged
        if(fabs(da2) < daeps) goto stop101;

        if((ampl2>amcrit && ampl2+rlx*da2<amcrit)||
                (ampl2<amcrit && ampl2+rlx*da2>amcrit)    )
            //------ limited newton step so ampl2 doesn't step across amcrit either way
            ampl2 = amcrit;
        else
            //------ regular newton step
            ampl2 = ampl2 + rlx*da2;
    }

    //TRACE("trchek2 - n2 convergence failed\n");
    str = "trchek2 - n2 convergence failed\n";
    writeString(str, true);
    if(s_bCancel) return false;
stop101:

    //---- test for free or forced transition
    trfree = (ampl2 >= amcrit);
    trforc = (xiforc>x1) && (xiforc<=x2);

    //---- set transition interval flag
    tran = (trforc || trfree);

    if(!tran) return false;

    //---- resolve if both forced and free transition
    if(trfree && trforc)
    {
        trforc = xiforc < xt;
        trfree = xiforc >= xt;
    }

    if(trforc)
    {
        //----- if forced transition, then xt is prescribed,
        //-     no sense calculating the sensitivities, since we know them...
        xt = xiforc;
        xt_a1 = 0.0;
        xt_x1 = 0.0;
        xt_t1 = 0.0;
        xt_d1 = 0.0;
        xt_u1 = 0.0;
        xt_x2 = 0.0;
        xt_t2 = 0.0;
        xt_d2 = 0.0;
        xt_u2 = 0.0;
        xt_ms = 0.0;
        xt_re = 0.0;
        xt_xf = 1.0;
        return true;
    }

    //---- free transition ... set sensitivities of xt

    xt_x1 =    wf1;
    tt_t1 =    wf1;
    dt_d1 =    wf1;
    ut_u1 =    wf1;

    xt_x2 =                wf2;
    tt_t2 =                wf2;
    dt_d2 =                wf2;
    ut_u2 =                wf2;

    xt_a1 = x1*wf1_a1 + x2*wf2_a1;
    tt_a1 = theta1*wf1_a1 + theta2*wf2_a1;
    dt_a1 = d1*wf1_a1 + d2*wf2_a1;
    ut_a1 = u1*wf1_a1 + u2*wf2_a1;

    xt_x1 = x1*wf1_x1 + x2*wf2_x1 + xt_x1;
    tt_x1 = theta1*wf1_x1 + theta2*wf2_x1;
    dt_x1 = d1*wf1_x1 + d2*wf2_x1;
    ut_x1 = u1*wf1_x1 + u2*wf2_x1;

    xt_x2 = x1*wf1_x2 + x2*wf2_x2 + xt_x2;
    tt_x2 = theta1*wf1_x2 + theta2*wf2_x2;
    dt_x2 = d1*wf1_x2 + d2*wf2_x2;
    ut_x2 = u1*wf1_x2 + u2*wf2_x2;

    xt_xf = x1*wf1_xf + x2*wf2_xf;
    //    tt_xf = t1*wf1_xf + t2*wf2_xf;
    //    dt_xf = d1*wf1_xf + d2*wf2_xf;
    //    ut_xf = u1*wf1_xf + u2*wf2_xf;

    //---- at this point, ax = ax( hk1, t1, rt1, a1, hkt, tt, rtt, at )

    //---- set sensitivities of ax( t1 d1 u1 a1 t2 d2 u2 a2 ms re )
    ax_t1 =  ax_hk1*hk1_t1 + ax_t1 + ax_rt1*rt1_t1
            + (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_t1;
    ax_d1 =  ax_hk1*hk1_d1
            + (ax_hkt*hkt_dt                        )*dt_d1;
    ax_u1 =  ax_hk1*hk1_u1         + ax_rt1*rt1_u1
            + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_u1;
    ax_a1 =  ax_a1
            + (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_a1//is tt_a1 initialized?
            + (ax_hkt*hkt_dt                        )*dt_a1
            + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_a1;
    ax_x1 = (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_x1
            + (ax_hkt*hkt_dt                        )*dt_x1
            + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_x1;

    ax_t2 = (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_t2;
    ax_d2 = (ax_hkt*hkt_dt                        )*dt_d2;
    ax_u2 = (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_u2;
    ax_a2 =  ax_at                                 *amplt_a2
            + (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_a2//is tt_a2 initialized?
            + (ax_hkt*hkt_dt                        )*dt_a2
            + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_a2;
    ax_x2 = (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_x2
            + (ax_hkt*hkt_dt                        )*dt_x2
            + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_x2;

    //    ax_xf = (ax_hkt*hkt_tt + ax_tt + ax_rtt*rtt_tt)*tt_xf
    //    + (ax_hkt*hkt_dt                        )*dt_xf
    //    + (ax_hkt*hkt_ut         + ax_rtt*rtt_ut)*ut_xf;

    ax_ms =  ax_hkt*hkt_ms         + ax_rtt*rtt_ms
            +  ax_hk1*hk1_ms         + ax_rt1*rt1_ms;
    ax_re =                          ax_rtt*rtt_re
            + ax_rt1*rt1_re;

    //---- set sensitivities of residual res
    //c   res  = ampl2 - ampl1 - ax*(x2-x1)
    z_ax =               -    (x2-x1);

    z_a1 = z_ax*ax_a1 - 1.0;
    z_t1 = z_ax*ax_t1;
    z_d1 = z_ax*ax_d1;
    z_u1 = z_ax*ax_u1;
    z_x1 = z_ax*ax_x1 + ax;

    z_a2 = z_ax*ax_a2 + 1.0;
    z_t2 = z_ax*ax_t2;
    z_d2 = z_ax*ax_d2;
    z_u2 = z_ax*ax_u2;
    z_x2 = z_ax*ax_x2 - ax;

    //    z_xf = z_ax*ax_xf;
    z_ms = z_ax*ax_ms;
    z_re = z_ax*ax_re;

    //---- set sensitivities of xt, with res being stationary for a2 constraint
    xt_a1 = xt_a1 - (xt_a2/z_a2)*z_a1;
    xt_t1 =       - (xt_a2/z_a2)*z_t1;
    xt_d1 =       - (xt_a2/z_a2)*z_d1;
    xt_u1 =       - (xt_a2/z_a2)*z_u1;
    xt_x1 = xt_x1 - (xt_a2/z_a2)*z_x1;
    xt_t2 =       - (xt_a2/z_a2)*z_t2;
    xt_d2 =       - (xt_a2/z_a2)*z_d2;
    xt_u2 =       - (xt_a2/z_a2)*z_u2;
    xt_x2 = xt_x2 - (xt_a2/z_a2)*z_x2;
    xt_ms =       - (xt_a2/z_a2)*z_ms;
    xt_re =       - (xt_a2/z_a2)*z_re;
    xt_xf = 0.0;

    return true ;
}


bool XFoil::trdif()
{
    //-----------------------------------------------
    //     sets up the newton system governing the
    //     transition interval.  equations governing
    //     the  laminar  part  x1 < xi < xt  and
    //     the turbulent part  xt < xi < x2
    //     are simply summed.
    //-----------------------------------------------
    double bl1[5][6], bl2[5][6], blrez[5], blm[5], blr[5], blx[5], bt1[5][6], bt2[5][6], btrez[5], btm[5], btr[5], btx[5];
    double wf2=0, wf2_xt=0, wf2_a1=0,wf2_x1=0, wf2_x2=0, wf2_t1=0, wf2_t2;
    double wf2_d1=0, wf2_d2=0, wf2_u1=0, wf2_u2=0, wf2_ms=0, wf2_re=0, wf2_xf=0;
    double wf1=0, wf1_a1=0, wf1_x1=0, wf1_x2=0, wf1_t1=0, wf1_t2=0, wf1_d1=0, wf1_d2=0;
    double wf1_u1=0, wf1_u2=0, wf1_ms=0, wf1_re=0, wf1_xf;
    double tt=0, tt_a1=0, tt_x1=0, tt_x2=0, tt_t1=0, tt_t2=0, tt_d1=0, tt_d2=0, tt_u1=0, tt_u2=0;
    double tt_ms=0, tt_re=0, tt_xf=0, dt=0, dt_a1=0, dt_x1=0, dt_x2=0, dt_t1=0, dt_t2=0;
    double dt_d1=0, dt_d2=0, dt_u1=0, dt_u2=0, dt_ms=0, dt_re=0, dt_xf;
    double ut=0, ut_a1=0, ut_x1=0, ut_x2=0, ut_t1=0, ut_t2=0, ut_d1=0, ut_d2=0, ut_u1=0, ut_u2=0;
    double ut_ms=0, ut_re=0, ut_xf=0;
    double st=0, st_tt=0, st_dt=0, st_ut=0, st_ms=0, st_re=0, st_a1=0, st_x1=0, st_x2=0, st_t1=0, st_t2=0;
    double st_d1=0, st_d2=0, st_u1=0, st_u2=0, st_xf=0;
    double ctr=0, ctr_hk2=0;
    int k;
    //    double c1sav[74], c2sav[74];

    //---- save variables and sensitivities for future restoration
    //    for (int icom=1; icom<= ncom;icom++){
    //        c1sav[icom] = com1[icom];
    //        c2sav[icom] = com2[icom];
    //    }
    saveblData(1);
    saveblData(2);

    //---- weighting factors for linear interpolation to transition point
    wf2    = (xt-x1)/(x2-x1);
    wf2_xt = 1.0/(x2-x1);

    wf2_a1 = wf2_xt*xt_a1;
    wf2_x1 = wf2_xt*xt_x1 + (wf2-1.0)/(x2-x1);
    wf2_x2 = wf2_xt*xt_x2 -  wf2      /(x2-x1);
    wf2_t1 = wf2_xt*xt_t1;
    wf2_t2 = wf2_xt*xt_t2;
    wf2_d1 = wf2_xt*xt_d1;
    wf2_d2 = wf2_xt*xt_d2;
    wf2_u1 = wf2_xt*xt_u1;
    wf2_u2 = wf2_xt*xt_u2;
    wf2_ms = wf2_xt*xt_ms;
    wf2_re = wf2_xt*xt_re;
    wf2_xf = wf2_xt*xt_xf;

    wf1    = 1.0 - wf2;
    wf1_a1 = -wf2_a1;
    wf1_x1 = -wf2_x1;
    wf1_x2 = -wf2_x2;
    wf1_t1 = -wf2_t1;
    wf1_t2 = -wf2_t2;
    wf1_d1 = -wf2_d1;
    wf1_d2 = -wf2_d2;
    wf1_u1 = -wf2_u1;
    wf1_u2 = -wf2_u2;
    wf1_ms = -wf2_ms;
    wf1_re = -wf2_re;
    wf1_xf = -wf2_xf;

    //**** first,  do laminar part between x1 and xt

    //-----interpolate primary variables to transition point
    tt    = theta1*wf1    + theta2*wf2;
    tt_a1 = theta1*wf1_a1 + theta2*wf2_a1;
    tt_x1 = theta1*wf1_x1 + theta2*wf2_x1;
    tt_x2 = theta1*wf1_x2 + theta2*wf2_x2;
    tt_t1 = theta1*wf1_t1 + theta2*wf2_t1 + wf1;
    tt_t2 = theta1*wf1_t2 + theta2*wf2_t2 + wf2;
    tt_d1 = theta1*wf1_d1 + theta2*wf2_d1;
    tt_d2 = theta1*wf1_d2 + theta2*wf2_d2;
    tt_u1 = theta1*wf1_u1 + theta2*wf2_u1;
    tt_u2 = theta1*wf1_u2 + theta2*wf2_u2;
    tt_ms = theta1*wf1_ms + theta2*wf2_ms;
    tt_re = theta1*wf1_re + theta2*wf2_re;
    tt_xf = theta1*wf1_xf + theta2*wf2_xf;

    dt    = d1*wf1    + d2*wf2;
    dt_a1 = d1*wf1_a1 + d2*wf2_a1;
    dt_x1 = d1*wf1_x1 + d2*wf2_x1;
    dt_x2 = d1*wf1_x2 + d2*wf2_x2;
    dt_t1 = d1*wf1_t1 + d2*wf2_t1;
    dt_t2 = d1*wf1_t2 + d2*wf2_t2;
    dt_d1 = d1*wf1_d1 + d2*wf2_d1 + wf1;
    dt_d2 = d1*wf1_d2 + d2*wf2_d2 + wf2;
    dt_u1 = d1*wf1_u1 + d2*wf2_u1;
    dt_u2 = d1*wf1_u2 + d2*wf2_u2;
    dt_ms = d1*wf1_ms + d2*wf2_ms;
    dt_re = d1*wf1_re + d2*wf2_re;
    dt_xf = d1*wf1_xf + d2*wf2_xf;

    ut    = u1*wf1    + u2*wf2;
    ut_a1 = u1*wf1_a1 + u2*wf2_a1;
    ut_x1 = u1*wf1_x1 + u2*wf2_x1;
    ut_x2 = u1*wf1_x2 + u2*wf2_x2;
    ut_t1 = u1*wf1_t1 + u2*wf2_t1;
    ut_t2 = u1*wf1_t2 + u2*wf2_t2;
    ut_d1 = u1*wf1_d1 + u2*wf2_d1;
    ut_d2 = u1*wf1_d2 + u2*wf2_d2;
    ut_u1 = u1*wf1_u1 + u2*wf2_u1 + wf1;
    ut_u2 = u1*wf1_u2 + u2*wf2_u2 + wf2;
    ut_ms = u1*wf1_ms + u2*wf2_ms;
    ut_re = u1*wf1_re + u2*wf2_re;
    ut_xf = u1*wf1_xf + u2*wf2_xf;

    //---- set primary "t" variables at xt  (really placed into "2" variables)
    x2 = xt;
    theta2 = tt;
    d2 = dt;
    u2 = ut;

    ampl2 = amcrit;
    s2 = 0.0;

    //---- calculate laminar secondary "t" variables
    blkin();
    blvar(1);

    //---- calculate x1-xt midpoint cfm value
    blmid(1);

    //=    at this point, all "2" variables are really "t" variables at xt


    //---- set up newton system for dam, dth, dds, due, dxi  at  x1 and xt
    bldif(1);

    //---- the current newton system is in terms of "1" and "t" variables,
    //-    so calculate its equivalent in terms of "1" and "2" variables.
    //-    in other words, convert residual sensitivities wrt "t" variables
    //-    into sensitivities wrt "1" and "2" variables.  the amplification
    //-    equation is unnecessary here, so the k=1 row is left empty.
    for (k=2; k<= 3; k++)
    {
        blrez[k] = vsrez[k];
        blm[k]   = vsm[k]+ vs2[k][2]*tt_ms+ vs2[k][3]*dt_ms+ vs2[k][4]*ut_ms+ vs2[k][5]*xt_ms;
        blr[k]   = vsr[k]+ vs2[k][2]*tt_re+ vs2[k][3]*dt_re+ vs2[k][4]*ut_re+ vs2[k][5]*xt_re;
        blx[k]   = vsx[k]+ vs2[k][2]*tt_xf+ vs2[k][3]*dt_xf+ vs2[k][4]*ut_xf+ vs2[k][5]*xt_xf;

        bl1[k][1] = vs1[k][1]+ vs2[k][2]*tt_a1+ vs2[k][3]*dt_a1+ vs2[k][4]*ut_a1+ vs2[k][5]*xt_a1;
        bl1[k][2] = vs1[k][2]+ vs2[k][2]*tt_t1+ vs2[k][3]*dt_t1+ vs2[k][4]*ut_t1+ vs2[k][5]*xt_t1;
        bl1[k][3] = vs1[k][3]+ vs2[k][2]*tt_d1+ vs2[k][3]*dt_d1+ vs2[k][4]*ut_d1+ vs2[k][5]*xt_d1;
        bl1[k][4] = vs1[k][4]+ vs2[k][2]*tt_u1+ vs2[k][3]*dt_u1+ vs2[k][4]*ut_u1+ vs2[k][5]*xt_u1;
        bl1[k][5] = vs1[k][5]+ vs2[k][2]*tt_x1+ vs2[k][3]*dt_x1+ vs2[k][4]*ut_x1+ vs2[k][5]*xt_x1;

        bl2[k][1] = 0.0;
        bl2[k][2] = vs2[k][2]*tt_t2+ vs2[k][3]*dt_t2+ vs2[k][4]*ut_t2+ vs2[k][5]*xt_t2;
        bl2[k][3] = vs2[k][2]*tt_d2+ vs2[k][3]*dt_d2+ vs2[k][4]*ut_d2+ vs2[k][5]*xt_d2;
        bl2[k][4] = vs2[k][2]*tt_u2+ vs2[k][3]*dt_u2+ vs2[k][4]*ut_u2+ vs2[k][5]*xt_u2;
        bl2[k][5] = vs2[k][2]*tt_x2+ vs2[k][3]*dt_x2+ vs2[k][4]*ut_x2+ vs2[k][5]*xt_x2;

    }

    //**** second, set up turbulent part between xt and x2  ****

    //---- calculate equilibrium shear coefficient cqt at transition point
    blvar(2);

    //---- set initial shear coefficient value st at transition point
    //-    ( note that cq2, cq2_t2, etc. are really "cqt", "cqt_tt", etc.)

    ctr     = 1.8*exp(-3.3/(hk2-1.0));
    ctr_hk2 = ctr * 3.3/(hk2-1.0)/(hk2-1.0);

    st    = ctr*cq2;
    st_tt = ctr*cq2_t2 + cq2*ctr_hk2*hk2_t2;
    st_dt = ctr*cq2_d2 + cq2*ctr_hk2*hk2_d2;
    st_ut = ctr*cq2_u2 + cq2*ctr_hk2*hk2_u2;
    st_ms = ctr*cq2_ms + cq2*ctr_hk2*hk2_ms;
    st_re = ctr*cq2_re;

    //---- calculate st sensitivities wrt the actual "1" and "2" variables
    st_a1 = st_tt*tt_a1 + st_dt*dt_a1 + st_ut*ut_a1;
    st_x1 = st_tt*tt_x1 + st_dt*dt_x1 + st_ut*ut_x1;
    st_x2 = st_tt*tt_x2 + st_dt*dt_x2 + st_ut*ut_x2;
    st_t1 = st_tt*tt_t1 + st_dt*dt_t1 + st_ut*ut_t1;
    st_t2 = st_tt*tt_t2 + st_dt*dt_t2 + st_ut*ut_t2;
    st_d1 = st_tt*tt_d1 + st_dt*dt_d1 + st_ut*ut_d1;
    st_d2 = st_tt*tt_d2 + st_dt*dt_d2 + st_ut*ut_d2;
    st_u1 = st_tt*tt_u1 + st_dt*dt_u1 + st_ut*ut_u1;
    st_u2 = st_tt*tt_u2 + st_dt*dt_u2 + st_ut*ut_u2;
    st_ms = st_tt*tt_ms + st_dt*dt_ms + st_ut*ut_ms + st_ms;
    st_re = st_tt*tt_re + st_dt*dt_re + st_ut*ut_re + st_re;
    st_xf = st_tt*tt_xf + st_dt*dt_xf + st_ut*ut_xf;

    ampl2 = 0.0;
    s2 = st;

    //---- recalculate turbulent secondary "t" variables using proper cti
    blvar(2);

    //---- set "1" variables to "t" variables and reset "2" variables
    //-    to their saved turbulent values
    //    for (icom=1; icom<= ncom; icom++){
    //        com1[icom] = com2[icom];
    //        com2[icom] = c2sav[icom];
    //    }
    stepbl();
    restoreblData(2);


    //---- calculate xt-x2 midpoint cfm value
    blmid(2);

    //---- set up newton system for dct, dth, dds, due, dxi  at  xt and x2
    bldif(2);

    //---- convert sensitivities wrt "t" variables into sensitivities
    //-    wrt "1" and "2" variables as done before for the laminar part
    for (k=1; k<= 3;k++)
    {
        btrez[k] = vsrez[k];
        btm[k]   = vsm[k] + vs1[k][1]*st_ms+ vs1[k][2]*tt_ms+ vs1[k][3]*dt_ms+ vs1[k][4]*ut_ms+ vs1[k][5]*xt_ms;
        btr[k]   = vsr[k] + vs1[k][1]*st_re+ vs1[k][2]*tt_re+ vs1[k][3]*dt_re+ vs1[k][4]*ut_re+ vs1[k][5]*xt_re;
        btx[k]   = vsx[k] + vs1[k][1]*st_xf+ vs1[k][2]*tt_xf+ vs1[k][3]*dt_xf+ vs1[k][4]*ut_xf+ vs1[k][5]*xt_xf;

        bt1[k][1] = vs1[k][1]*st_a1+ vs1[k][2]*tt_a1+ vs1[k][3]*dt_a1+ vs1[k][4]*ut_a1+ vs1[k][5]*xt_a1;
        bt1[k][2] = vs1[k][1]*st_t1+ vs1[k][2]*tt_t1+ vs1[k][3]*dt_t1+ vs1[k][4]*ut_t1+ vs1[k][5]*xt_t1;
        bt1[k][3] = vs1[k][1]*st_d1+ vs1[k][2]*tt_d1+ vs1[k][3]*dt_d1+ vs1[k][4]*ut_d1+ vs1[k][5]*xt_d1;
        bt1[k][4] = vs1[k][1]*st_u1+ vs1[k][2]*tt_u1+ vs1[k][3]*dt_u1+ vs1[k][4]*ut_u1+ vs1[k][5]*xt_u1;
        bt1[k][5] = vs1[k][1]*st_x1+ vs1[k][2]*tt_x1+ vs1[k][3]*dt_x1+ vs1[k][4]*ut_x1+ vs1[k][5]*xt_x1;

        bt2[k][1] = vs2[k][1];
        bt2[k][2] = vs2[k][2]+ vs1[k][1]*st_t2+ vs1[k][2]*tt_t2+ vs1[k][3]*dt_t2+ vs1[k][4]*ut_t2+ vs1[k][5]*xt_t2;
        bt2[k][3] = vs2[k][3]+ vs1[k][1]*st_d2+ vs1[k][2]*tt_d2+ vs1[k][3]*dt_d2+ vs1[k][4]*ut_d2+ vs1[k][5]*xt_d2;
        bt2[k][4] = vs2[k][4]+ vs1[k][1]*st_u2+ vs1[k][2]*tt_u2+ vs1[k][3]*dt_u2+ vs1[k][4]*ut_u2+ vs1[k][5]*xt_u2;
        bt2[k][5] = vs2[k][5]+ vs1[k][1]*st_x2+ vs1[k][2]*tt_x2+ vs1[k][3]*dt_x2+ vs1[k][4]*ut_x2+ vs1[k][5]*xt_x2;

    }

    //---- add up laminar and turbulent parts to get final system
    //-    in terms of honest-to-god "1" and "2" variables.
    vsrez[1] =            btrez[1];
    vsrez[2] = blrez[2] + btrez[2];
    vsrez[3] = blrez[3] + btrez[3];
    vsm[1]   =            btm[1];
    vsm[2]   = blm[2]   + btm[2];
    vsm[3]   = blm[3]   + btm[3];
    vsr[1]   =            btr[1];
    vsr[2]   = blr[2]   + btr[2];
    vsr[3]   = blr[3]   + btr[3];
    vsx[1]   =            btx[1];
    vsx[2]   = blx[2]   + btx[2];
    vsx[3]   = blx[3]   + btx[3];
    for (int l=1; l<=5;l++)
    {
        vs1[1][l] =            bt1[1][l];
        vs2[1][l] =            bt2[1][l];
        vs1[2][l] = bl1[2][l] + bt1[2][l];
        vs2[2][l] = bl2[2][l] + bt2[2][l];
        vs1[3][l] = bl1[3][l] + bt1[3][l];
        vs2[3][l] = bl2[3][l] + bt2[3][l];
    }

    //---- to be sanitary, restore "1" quantities which got clobbered
    //-    in all of the numerical gymnastics above.  the "2" variables
    //-    were already restored for the xt-x2 differencing part.
    //    for (icom=1; icom<=ncom;icom++){
    //        com1[icom] = c1sav[icom];
    //    }
    restoreblData(1);

    return true;
}


bool XFoil::trisol(double a[], double b[], double c[], double d[], int kk){
    //-----------------------------------------
    //     solves kk long, tri-diagonal system |
    //                                         |
    //             a c          d              |
    //             b a c        d              |
    //               b a .      .              |
    //                 . . c    .              |
    //                   b a    d              |
    //                                         |
    //     the righthand side d is replaced by |
    //     the solution.  a, c are destroyed.  |
    //-----------------------------------------

    for (int k=2; k<= kk;k++)
    {
        int km = k-1;
        c[km] = c[km] / a[km];
        d[km] = d[km] / a[km];
        a[k] = a[k] - b[k]*c[km];
        d[k] = d[k] - b[k]*d[km];
    }

    d[kk] = d[kk]/a[kk];

    for(int k=kk-1;k>= 1;k--)
    {
        d[k] = d[k] - c[k]*d[k+1];
    }
    return true;
}


/** ---------------------------------------------------------
 *     sets ue from inviscid ue plus all source influence
 * --------------------------------------------------------- */
bool XFoil::ueset()
{
    double dui=0, ue_m=0;
    for (int is=1; is<= 2;is++)
    {
        for(int ibl=2; ibl<= nbl[is]; ibl++)
        {
            int i = ipan[ibl][is];

            dui = 0.0;
            for (int js=1; js<=2; js++)
            {
                for(int jbl=2; jbl<=nbl[js]; jbl++)
                {
                    int j  = ipan[jbl][js];
                    ue_m = -vti[ibl][is]*vti[jbl][js]*dij[i][j];
                    dui += ue_m*mass[jbl][js];
                }
            }
            uedg[ibl][is] = uinv[ibl][is] + dui;
        }
    }
    return true;
}


/** --------------------------------------------------------------
 *     sets inviscid ue from panel inviscid tangential velocity
 * -------------------------------------------------------------- */
bool XFoil::uicalc()
{
    for (int is=1; is<=2;is++)
    {
        uinv  [1][is] = 0.0;
        uinv_a[1][is] = 0.0;
        for (int ibl=2; ibl<=nbl[is]; ibl++)
        {
            int i = ipan[ibl][is];
            uinv[ibl][is] = vti[ibl][is]*qinv  [i];
            uinv_a[ibl][is] = vti[ibl][is]*qinv_a[i];
        }
    }
    return true;
}


/** ------------------------------------------------------------------
 *     Adds on Newton deltas to boundary layer variables.
 *     Checks for excessive changes and underrelaxes if necessary.
 *     Calculates max and rms changes.
 *     Also calculates the change in the global variable "ac".
 *       if lalfa=true , "ac" is cl
 *       if lalfa=false, "ac" is alpha
 * ------------------------------------------------------------------ */
bool XFoil::update()
{

    int i=0, ip=0, is=0, iv=0, iw=0, j=0, js=0, jv=0, ibl=0, jbl=0, kbl=0;
    double unew[IVX][3], u_ac[IVX][3];
    memset(unew, 0, IVX*3*sizeof(double));
    memset(u_ac, 0, IVX*3*sizeof(double));
    double qnew[IQX],q_ac[IQX];
    memset(qnew, 0, IQX*sizeof(double));
    memset(q_ac, 0, IQX*sizeof(double));
    double dalmax=0.0, dalmin=0.0, dclmax=0.0, dclmin=0.0,dx=0.0, dx_a=0.0, ag=0.0, ag_ms=0.0, ag_ac=0.0;
    double dac=0.0, dhi=0.0, dlo=0.0, dctau=0.0, dthet=0.0, dmass=0.0, duedg=0.0, ddstr=0.0;
    double dn1=0.0, dn2=0.0, dn3=0.0, dn4=0.0, rdn1=0.0,rdn2=0.0,rdn3=0.0,rdn4=0.0;
    double dswaki=0.0, hklim=0.0, msq=0.0, dsw=0.0;
    double dui=0.0, dui_ac=0.0, ue_m=0.0, uinv_ac=0.0,sa=0.0,ca=0.0, beta=0.0, beta_msq=0.0, bfac=0.0, bfac_msq=0.0;
    double clnew=0.0, cl_a=0.0, cl_ms=0.0, cl_ac=0.0, cginc=0.0;
    double cpg1=0.0,cpg1_ms=0.0, cpi_q=0.0, cpc_cpi=0.0, cpg1_ac=0.0,cpg2=0.0, cpg2_ms=0.0, cpg2_ac=0.0;
    std::string vmxbl;

    //---- max allowable alpha changes per iteration
    dalmax =  0.5*dtor;
    dalmin = -0.5*dtor;
    //---- max allowable cl change per iteration
    dclmax =  0.5;
    dclmin = -0.5;
    if(matyp!=1) dclmin = std::max(-0.5, -0.9*cl) ;
    hstinv = gamm1*(minf/qinf)*(minf/qinf) / (1.0 + 0.5*gamm1*minf*minf);

    //--- calculate new ue distribution assuming no under-relaxation
    //--- also set the sensitivity of ue wrt to alpha or re
    for (is=1; is<= 2; is++)
    {
        for(ibl=2;ibl<= nbl[is];ibl++)
        {
            i = ipan[ibl][is];
            dui    = 0.0;
            dui_ac = 0.0;
            for (js=1; js<=2; js++)
            {
                for (jbl=2; jbl<=nbl[js];jbl++)
                {
                    j  = ipan[jbl][js];
                    jv = isys[jbl][js];
                    ue_m = -vti[ibl][is]*vti[jbl][js]*dij[i][j];
                    dui    = dui    + ue_m*(mass[jbl][js]+vdel[3][1][jv]);
                    dui_ac = dui_ac + ue_m*(             -vdel[3][2][jv]);
                }
            }

            //------- uinv depends on "ac" only if "ac" is alpha
            if(lalfa) uinv_ac = 0.0;
            else uinv_ac = uinv_a[ibl][is];

            unew[ibl][is] = uinv[ibl][is] + dui;
            u_ac[ibl][is] = uinv_ac      + dui_ac;

        }
    }

    //--- set new qtan from new ue with appropriate sign change

    for (is=1;  is<= 2;  is++)
    {
        for(ibl=2; ibl<= iblte[is]; ibl++)
        {
            i = ipan[ibl][is];
            qnew[i] = vti[ibl][is]*unew[ibl][is];
            q_ac[i] = vti[ibl][is]*u_ac[ibl][is];
        }
    }

    //--- calculate new cl from this new qtan
    sa = sin(alfa);
    ca = cos(alfa);


    beta = sqrt(1.0 - minf*minf);
    beta_msq = -0.5/beta;

    bfac     = 0.5*minf*minf / (1.0 + beta);
    bfac_msq = 0.5         / (1.0 + beta)
            - bfac        / (1.0 + beta) * beta_msq;

    clnew = 0.0;
    cl_a  = 0.0;
    cl_ms = 0.0;
    cl_ac = 0.0;

    i = 1;
    cginc = 1.0 - (qnew[i]/qinf)*(qnew[i]/qinf);
    cpg1  = cginc / (beta + bfac*cginc);
    cpg1_ms = -cpg1/(beta + bfac*cginc)*(beta_msq + bfac_msq*cginc);

    cpi_q = -2.0*qnew[i]/qinf/qinf;
    cpc_cpi = (1.0 - bfac*cpg1)/ (beta + bfac*cginc);
    cpg1_ac = cpc_cpi*cpi_q*q_ac[i];


    for (i=1; i<=n; i++)
    {
        ip = i+1;
        if(i==n) ip = 1;

        cginc = 1.0 - (qnew[ip]/qinf)*(qnew[ip]/qinf);
        cpg2  = cginc / (beta + bfac*cginc);
        cpg2_ms = -cpg2/(beta + bfac*cginc)*(beta_msq + bfac_msq*cginc);

        cpi_q = -2.0*qnew[ip]/qinf/qinf;
        cpc_cpi = (1.0 - bfac*cpg2)/ (beta + bfac*cginc);
        cpg2_ac = cpc_cpi*cpi_q*q_ac[ip];

        dx   =  (x[ip] - x[i])*ca + (y[ip] - y[i])*sa;
        dx_a = -(x[ip] - x[i])*sa + (y[ip] - y[i])*ca;

        ag    = 0.5*(cpg2    + cpg1   );
        ag_ms = 0.5*(cpg2_ms + cpg1_ms);
        ag_ac = 0.5*(cpg2_ac + cpg1_ac);

        clnew = clnew + dx  *ag;
        cl_a  = cl_a  + dx_a*ag;
        cl_ms = cl_ms + dx  *ag_ms;
        cl_ac = cl_ac + dx  *ag_ac;

        cpg1    = cpg2;
        cpg1_ms = cpg2_ms;
        cpg1_ac = cpg2_ac;
    }

    //--- initialize under-relaxation factor
    rlx = 1.0;

    if(lalfa)
    {
        //===== alpha is prescribed: ac is cl

        //---- set change in re to account for cl changing, since re = re(cl)
        dac = (clnew - cl) / (1.0 - cl_ac - cl_ms*2.0*minf*minf_cl);

        //---- set under-relaxation factor if re change is too large
        if(rlx*dac > dclmax) rlx = dclmax/dac;
        if(rlx*dac < dclmin) rlx = dclmin/dac;
    }
    else
    {
        //===== cl is prescribed: ac is alpha

        //---- set change in alpha to drive cl to prescribed value
        dac = (clnew - clspec) / (0.0 - cl_ac - cl_a);

        //---- set under-relaxation factor if alpha change is too large
        if(rlx*dac > dalmax) rlx = dalmax/dac;
        if(rlx*dac < dalmin) rlx = dalmin/dac;
    }
    rmsbl = 0.0;
    rmxbl = 0.0;
    dhi = 1.5;
    dlo = -.5;
    //--- calculate changes in bl variables and under-relaxation if needed

    for(is=1;is<= 2;is++)
    {
        for(ibl=2; ibl<= nbl[is];ibl++)
        {
            iv = isys[ibl][is];
            //------- set changes without underrelaxation
            dctau = vdel[1][1][iv] - dac*vdel[1][2][iv];
            dthet = vdel[2][1][iv] - dac*vdel[2][2][iv];
            dmass = vdel[3][1][iv] - dac*vdel[3][2][iv];
            duedg = unew[ibl][is] + dac*u_ac[ibl][is]  -  uedg[ibl][is];
            ddstr = (dmass - dstr[ibl][is]*duedg)/uedg[ibl][is];
            //------- normalize changes
            if(ibl<itran[is]) dn1 = dctau / 10.0;
            else dn1 = dctau / ctau[ibl][is];
            dn2 = dthet / thet[ibl][is];
            dn3 = ddstr / dstr[ibl][is];
            dn4 = fabs(duedg)/0.25;
            //------- accumulate for rms change
            rmsbl = rmsbl + dn1*dn1 + dn2*dn2 + dn3*dn3 + dn4*dn4;
            //------- see if ctau needs underrelaxation
            rdn1 = rlx*dn1;
            if(fabs(dn1) > fabs(rmxbl))
            {
                rmxbl = dn1;
                if(ibl<itran[is]) vmxbl = "n";
                if(ibl>=itran[is]) vmxbl = "c";
                imxbl = ibl;
                ismxbl = is;
            }
            if(rdn1 > dhi) rlx = dhi/dn1;
            if(rdn1 < dlo) rlx = dlo/dn1;
            //------- see if theta needs underrelaxation
            rdn2 = rlx*dn2;
            if(fabs(dn2) > fabs(rmxbl))
            {
                rmxbl = dn2;
                vmxbl = "t";
                imxbl = ibl;
                ismxbl = is;
            }
            if(rdn2 > dhi) rlx = dhi/dn2;
            if(rdn2 < dlo) rlx = dlo/dn2;
            //------- see if dstar needs underrelaxation
            rdn3 = rlx*dn3;
            if(fabs(dn3) > fabs(rmxbl))
            {
                rmxbl = dn3;
                vmxbl = "d";
                imxbl = ibl;
                ismxbl = is;
            }
            if(rdn3 > dhi) rlx = dhi/dn3;
            if(rdn3 < dlo) rlx = dlo/dn3;

            //------- see if ue needs underrelaxation
            rdn4 = rlx*dn4;
            if(fabs(dn4) > fabs(rmxbl))
            {
                rmxbl = duedg;
                vmxbl = "u";
                imxbl = ibl;
                ismxbl = is;
            }
            if(rdn4 > dhi) rlx = dhi/dn4;
            if(rdn4 < dlo) rlx = dlo/dn4;
        }
    }


    //--- set true rms change
    rmsbl = sqrt( rmsbl / (4.0*double( nbl[1]+nbl[2] )) );

    if(lalfa)
    {
        //---- set underrelaxed change in reynolds number from change in lift
        cl = cl + rlx*dac;
    }
    else
    {
        //---- set underrelaxed change in alpha
        alfa = alfa + rlx*dac;
        adeg = alfa/dtor;
    }

    //--- update bl variables with underrelaxed changes
    for(is=1;is<= 2;is++)
    {
        for(ibl=2;ibl<= nbl[is];ibl++)
        {
            iv = isys[ibl][is];

            dctau = vdel[1][1][iv] - dac*vdel[1][2][iv];
            dthet = vdel[2][1][iv] - dac*vdel[2][2][iv];
            dmass = vdel[3][1][iv] - dac*vdel[3][2][iv];
            duedg = unew[ibl][is] + dac*u_ac[ibl][is]  -  uedg[ibl][is];
            ddstr = (dmass - dstr[ibl][is]*duedg)/uedg[ibl][is];

            ctau[ibl][is] = ctau[ibl][is] + rlx*dctau;
            thet[ibl][is] = thet[ibl][is] + rlx*dthet;
            dstr[ibl][is] = dstr[ibl][is] + rlx*ddstr;
            uedg[ibl][is] = uedg[ibl][is] + rlx*duedg;

            if(ibl>iblte[is]) {
                iw = ibl - iblte[is];
                dswaki = wgap[iw];
            }
            else dswaki = 0.0;
            //------- eliminate absurd transients
            if(ibl>=itran[is]) ctau[ibl][is] = std::min(ctau[ibl][is], 0.25);

            if(ibl<=iblte[is]) hklim = 1.02;
            else            hklim = 1.00005;

            msq = uedg[ibl][is]*uedg[ibl][is]*hstinv
                    / (gamm1*(1.0 - 0.5*uedg[ibl][is]*uedg[ibl][is]*hstinv));
            dsw = dstr[ibl][is] - dswaki;
            dslim(dsw,thet[ibl][is],msq,hklim);
            dstr[ibl][is] = dsw + dswaki;

            //------- set new mass defect (nonlinear update)
            mass[ibl][is] = dstr[ibl][is] * uedg[ibl][is];
        }
    }


    //--- equate upper wake arrays to lower wake arrays
    for(kbl=1;kbl<= nbl[2]-iblte[2];kbl++)
    {
        ctau[iblte[1]+kbl][1] = ctau[iblte[2]+kbl][2];
        thet[iblte[1]+kbl][1] = thet[iblte[2]+kbl][2];
        dstr[iblte[1]+kbl][1] = dstr[iblte[2]+kbl][2];
        uedg[iblte[1]+kbl][1] = uedg[iblte[2]+kbl][2];
        tau[iblte[1]+kbl][1] =  tau[iblte[2]+kbl][2];
        dis[iblte[1]+kbl][1] =  dis[iblte[2]+kbl][2];
        ctq[iblte[1]+kbl][1] =  ctq[iblte[2]+kbl][2];
    }

    //      equivalence (va(1,1,1), unew(1,1)) , (vb(1,1,1), qnew(1)  )
    //      equivalence (va(1,1,IVX), u_ac(1,1)) , (vb(1,1,ivx), q_ac(1)  )
    /*    for (int kk = 1; kk<250; kk++) {
        vb[kk][1][1]   = qnew[kk];
        vb[kk][1][IVX] = q_ac[kk];
    }

    for (is=1; is<= 2; is++){
        for(ibl=2;ibl<= nbl[is];ibl++){
            va[ibl][is][1]   = unew[ibl][is];
            va[ibl][is][IVX] = u_ac[ibl][is];
        }
    }*/
    return true;
}


/** --------------------------------------
 *      converges viscous operating point
 * -------------------------------------- */
bool XFoil::viscal()
{

    int ibl=0;

    //---- calculate wake trajectory from current inviscid solution if necessary
    if(!lwake)     xyWake();

    //    ---- set velocities on wake from airfoil vorticity for alpha=0, 90
    qwcalc();

    //    ---- set velocities on airfoil and wake for initial alpha
    qiset();

    if(!lipan) {

        if(lblini) gamqv();

        //    ----- locate stagnation point arc length position and panel index
        stfind();

        //    ----- set  bl position -> panel position  pointers
        iblpan();

        //    ----- calculate surface arc length array for current stagnation point location
        xicalc();

        //    ----- set  bl position -> system line  pointers
        iblsys();

    }

    //    ---- set inviscid bl edge velocity uinv from qinv
    uicalc();

    if(!lblini) {
        //    ----- set initial ue from inviscid ue
        for (ibl=1; ibl<= nbl[1];ibl++){
            uedg[ibl][1] = uinv[ibl][1];
        }
        for (ibl=1; ibl<= nbl[2];ibl++){
            uedg[ibl][2] = uinv[ibl][2];
        }
    }

    if(lvconv) {
        //    ----- set correct cl if converged point exists
        qvfue();
        /*
        if(lvisc){
            if(!cpcalc(n+nw,qvis,qinf,minf,cpv)){
                return false;
            }
            if(!cpcalc(n+nw,qinv,qinf,minf,cpi)){
                return false;
            }
        }
        else if(!cpcalc(n,qinv,qinf,minf,cpi)){
            return false;
        }
*/
        if(lvisc){
            cpcalc(n+nw,qvis,qinf,minf,cpv);
            cpcalc(n+nw,qinv,qinf,minf,cpi);
        }
        else cpcalc(n,qinv,qinf,minf,cpi);

        gamqv();
        clcalc(xcmref,ycmref);
        cdcalc();
    }

    //    ---- set up source influence matrix if it doesn't exist
    if(!lwdij || !ladij) qdcalc();

    return true;
}


bool XFoil::ViscalEnd()
{
    /*
    if(!){
        return false;
    }
    if(!cpcalc(n+nw,qvis,qinf,minf,cpv)){
        return false;
    }
*/
    cpcalc(n+nw,qinv,qinf,minf,cpi);
    cpcalc(n+nw,qvis,qinf,minf,cpv);
    if(lflap) mhinge();

    return true;

}


bool XFoil::ViscousIter()
{
    //    Performs one iteration
    double eps1 =0.0001;
    std::string str;


    setbl();//    ------ fill newton system for bl variables


    blsolve();//    ------ solve newton system with custom solver

    update();//    ------ update bl variables


    if(lalfa) {//    ------- set new freestream mach, re from new cl
        mrcl(cl, minf_cl, reinf_cl);
        comset();
    }
    else{//    ------- set new inviscid speeds qinv and uinv for new alpha
        qiset();
        uicalc();
    }

    qvfue();//    ------ calculate edge velocities qvis(.) from uedg(..)
    gamqv();//    ------ set gam distribution from qvis
    stmove();//    ------ relocate stagnation point

    //    ------ set updated cl,cd
    clcalc(xcmref,ycmref);
    cdcalc();


    //    ------ display changes and test for convergence
    if(rlx<1.0)
    {
//        str =QString::asprintf("     rms:%7g   max:%7g at {:d} {3:d}   rlx:{4:.3f}\n", rmsbl, rmxbl, imxbl, ismxbl, rlx);
    }
    else if(fabs(rlx-1.0)<0.001)
    {
//        str =QString::asprintf("     rms:%7g   max:%7g at {:d} {3:d}\n", rmsbl,rmxbl,imxbl, ismxbl);
    }

//    writeString(str);

    cdp = cd - cdf;

//    str = QString::asprintf("     a=%.3f    cl={:.5f}\n     cm={:.5f}  cd={3:.5f} => cdf={4:.5f} cdp={5:.5f}\n\n", alfa/dtor, cl, cm, cd, cdf, cdp);
//    writeString(str);


/*    std::size_t pos = str.find("QN");

    if (pos!=std::string::npos)
    {
        // no error
    }
    else
    {
        lvconv = false;
        str = "--------UNCONVERGED----------\n\n";
        writeString(str, true);
        return false;
    }*/

    if(rmsbl < eps1)
    {
        lvconv = true;
        avisc = alfa;
        mvisc = minf;
        str = "----------CONVERGED----------\n\n";
        writeString(str, true);
    }

    return true;
}


/** -------------------------------------------------------------
 *     sets bl arc length array on each airfoil side and wake
 * ------------------------------------------------------------- */
bool XFoil::xicalc()
{
    double telrat=0, crosp=0, dwdxte=0, aa=0, bb=0, zn=0;
    int i=0, ibl=0, is=0, iw=0;
    is = 1;

    xssi[1][is] = 0.0;

    for (ibl=2;ibl<= iblte[is];ibl++)
    {
        i = ipan[ibl][is];
        xssi[ibl][is] = sst - s[i];
    }

    is = 2;

    xssi[1][is] = 0.0;

    for (ibl=2;ibl<= iblte[is];ibl++)
    {
        i = ipan[ibl][is];
        xssi[ibl][is] = s[i] - sst;
    }

    ibl = iblte[is] + 1;
    xssi[ibl][is] = xssi[ibl-1][is];

    for (ibl=iblte[is]+2;ibl<= nbl[is];ibl++)
    {

        int i = ipan[ibl][is];
        xssi[ibl][is] = xssi[ibl-1][is]    + sqrt((x[i]-x[i-1])* (x[i]-x[i-1]) + (y[i]-y[i-1])*(y[i]-y[i-1]));
    }

    //---- trailing edge flap length to te gap ratio
    telrat = 2.50;

    //---- set up parameters for te flap cubics

    //   dwdxte = yp[1]/xp[1] + yp[n]/xp[n]    !!! bug  2/2/95

    crosp = (xp[1]*yp[n] - yp[1]*xp[n])
            / sqrt(  (xp[1]*xp[1] + yp[1]*yp[1])
            *(xp[n]*xp[n] + yp[n]*yp[n]) );
    dwdxte = crosp / sqrt(1.0 - crosp*crosp);

    //---- limit cubic to avoid absurd te gap widths
    dwdxte = std::max(dwdxte,-3.0/telrat);
    dwdxte = std::min(dwdxte, 3.0/telrat);

    aa =  3.0 + telrat*dwdxte;
    bb = -2.0 - telrat*dwdxte;

    if(sharp)
    {
        for (iw=1; iw<=nw;iw++)
            wgap[iw] = 0.0;
    }

    else
    {
        //----- set te flap (wake gap) array
        is = 2;
        for (iw=1; iw<=nw;iw++)
        {
            ibl = iblte[is] + iw;
            zn = 1.0 - (xssi[ibl][is]-xssi[iblte[is]][is]) / (telrat*ante);
            wgap[iw] = 0.0;
            if(zn>=0.0) wgap[iw] = ante * (aa + bb*zn)*zn*zn;
        }
    }
    return true;
}


/** -----------------------------------------------------
 *        sets forced-transition bl coordinate locations.
 * ----------------------------------------------------- */
bool XFoil::xifset(int is)
{
    double chx=0, chy=0, chsq=0, str=0;

    if(xstrip[is]>=1.0)
    {
        xiforc = xssi[iblte[is]][is];
        return false;
    }

    chx = xte - xle;
    chy = yte - yle;
    chsq = chx*chx + chy*chy;

    //---- calculate chord-based x/c, y/c
    for(int i=1; i<=n; i++){
        w1[i] = ((x[i]-xle)*chx + (y[i]-yle)*chy) / chsq;
        w2[i] = ((y[i]-yle)*chx - (x[i]-xle)*chy) / chsq;
    }

    splind(w1,w3,s,n,-999.0,-999.0);
    splind(w2,w4,s,n,-999.0,-999.0);

    if(is==1) {

        //----- set approximate arc length of forced transition point for sinvrt
        str = sle + (s[1]-sle)*xstrip[is];

        //----- calculate actual arc length
        sinvrt(str,xstrip[is],w1,w3,s,n);

        //----- set bl coordinate value
        xiforc = std::min((sst-str), xssi[iblte[is]][is]);
    }
    else{
        //----- same for bottom side

        str = sle + (s[n]-sle)*xstrip[is];
        sinvrt(str,xstrip[is],w1,w3,s,n);
        xiforc = std::min((str - sst) , xssi[iblte[is]][is]);

    }

    if(xiforc < 0.0) {
        //TRACE(" ***  stagnation point is past trip on side %d\n", is);
//        std::string str = QString::asprintf(" ***  stagnation point is past trip on side %d\n", is);
//        writeString(str);

        xiforc = xssi[iblte[is]][is];
    }

    return true;
}



/** -----------------------------------------------------
 *     sets wake coordinate array for current surface
 *     vorticity and/or mass source distributions.
 *----------------------------------------------------- */
bool XFoil::xyWake()
{
    double ds=0, ds1=0, sx=0, sy=0, smod=0;
    double psi=0, psi_x=0, psi_y=0;
    //
    std::string str("   Calculating wake trajectory ...\n");
    writeString(str, true);
    //
    //--- number of wake points
    nw = n/8 + 2;
    if(nw>IWX)
    {
        std::string str(" XYWake: array size (IWX) too small.\n  Last wake point index reduced.");
        writeString(str, true);
        nw = IWX;
    }

    ds1 = 0.5*(s[2] - s[1] + s[n] - s[n-1]);
    setexp(snew+n,ds1,waklen*chord,nw);

    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);

    //-- set first wake point a tiny distance behind te
    int i = n+1;
    sx = 0.5*(yp[n] - yp[1]);
    sy = 0.5*(xp[1] - xp[n]);
    smod = sqrt(sx*sx + sy*sy);
    nx[i] = sx / smod;
    ny[i] = sy / smod;
    x[i] = xte - 0.0001*ny[i];
    y[i] = yte + 0.0001*nx[i];
    s[i] = s[n];

    //---- calculate streamfunction gradient components at first point
    psilin(i,x[i],y[i],1.0,0.0,psi,psi_x,false,false);
    psilin(i,x[i],y[i],0.0,1.0,psi,psi_y,false,false);

    //---- set unit vector normal to wake at first point
    nx[i+1] = -psi_x / sqrt(psi_x*psi_x + psi_y*psi_y);
    ny[i+1] = -psi_y / sqrt(psi_x*psi_x + psi_y*psi_y);

    //---- set angle of wake panel normal
    apanel[i] = atan2( psi_y , psi_x );

    //---- set rest of wake points
    for(i=n+2; i<=n+nw; i++)
    {
        ds = snew[i] - snew[i-1];

        //------ set new point ds downstream of last point
        x[i] = x[i-1] - ds*ny[i];
        y[i] = y[i-1] + ds*nx[i];
        s[i] = s[i-1] + ds;

        if(i!=n+nw)
        {
            //------- calculate normal vector for next point
            psilin(i,x[i],y[i],1.0,0.0,psi,psi_x,false,false);
            psilin(i,x[i],y[i],0.0,1.0,psi,psi_y,false,false);

            nx[i+1] = -psi_x / sqrt(psi_x*psi_x + psi_y*psi_y);
            ny[i+1] = -psi_y / sqrt(psi_x*psi_x + psi_y*psi_y);

            //------- set angle of wake panel normal
            apanel[i] = atan2( psi_y , psi_x );
        }
    }

    //---- set wake presence flag and corresponding alpha
    lwake = true;
    awake =  alfa;

    //---- old source influence matrix is invalid for the new wake geometry
    lwdij = false;

    return true;
}



int XFoil::arefine(double x[],double y[], double s[], double xs[], double ys[],
                   int n, double atol, int ndim,
                   double xnew[], double ynew[], double x1, double x2)
{
    //-------------------------------------------------------------
    //     adds points to a x,y spline contour wherever
    //     the angle between adjacent segments at a node
    //     exceeds a specified threshold.  the points are
    //     added 1/3 of a segment before and after the
    //     offending node.
    //
    //     the point adding is done only within x1..x2.
    //
    //     intended for doubling the number of points
    //     of eppler and selig airfoils so that they are
    //     suitable for clean interpolation using xfoil's
    //     arc-length spline routines.
    //------------------------------------------------------
    //      real x(*), y(*), s(*), xs(*), ys(*)
    //      real xnew(ndim), ynew(ndim)
    bool lref=false;
    double atolr=0, dxm=0, dym=0,  dxp=0, dyp=0, crsp=0, dotp=0, aseg=0, smid=0, xk=0, yk=0;
    int k=0;
    //    int im, ip;

    atolr = atol * PI/180.0;

    k = 1;
    xnew[k] = x[1];
    ynew[k] = y[1];

    for( int i = 2; i<=n-1; i++)
    {
        //        im = i-1;
        //        ip = i+1;

        dxm = x[i] - x[i-1];
        dym = y[i] - y[i-1];
        dxp = x[i+1] - x[i];
        dyp = y[i+1] - y[i];

        crsp = dxm*dyp - dym*dxp;
        dotp = dxm*dxp + dym*dyp;
        if(crsp==0.0 && dotp==0.0)
            aseg = 0.0;
        else
            aseg = atan2(crsp, dotp );


        lref = fabs(aseg) > atolr;

        if(lref) {
            //------- add extra point just before this node
            smid = s[i] - 0.3333*(s[i]-s[i-1]);
            xk = seval(smid,x,xs,s,n);
            yk = seval(smid,y,ys,s,n);
            if(xk>=x1 && xk<=x2) {
                k = k + 1;
                if(k > ndim)  goto stop90;
                xnew[k] = xk;
                ynew[k] = yk;
            }
        }

        //------ add the node itself
        k = k + 1;
        if(k > ndim)  goto stop90;
        xnew[k] = x[i];
        ynew[k] = y[i];

        if(lref) {
            //------- add extra point just after this node
            smid = s[i] + 0.3333*(s[i+1]-s[i]);
            xk = seval(smid,x,xs,s,n);
            yk = seval(smid,y,ys,s,n);
            if(xk>=x1 && xk<=x2) {
                k = k + 1;
                if(k > ndim)  goto stop90;
                xnew[k] = xk;
                ynew[k] = yk;
            }
        }
    }

    k = k + 1;
    if(k>ndim) goto stop90;
    xnew[k] = x[n];
    ynew[k] = y[n];

    //    nnew = k;
    return k;

stop90: 
    std::string str = "sdouble:  Arrays will overflow.  No action taken.\n";
    writeString(str, true);

    //    nnew = 0;
    return 0;
}



int XFoil::cadd(int ispl, double atol, double xrf1, double xrf2)
{
    int nnew=0, nbadd=0;
    //    cang(xb,yb,nb,&imax,&amax); // Already done???

    if(ispl == 1)
    {
        sb[1] = 0.0;
        for(int i=2; i<=nb; i++){
            if (fabs(xb[i]-xb[i-1])<EPSILON && fabs(yb[i]-yb[i-1])<EPSILON)
                sb[i] = sb[i-1];
            else
                sb[i] = sb[i-1] + 1.0;
        }
        segspl(xb,xbp,sb,nb);
        segspl(yb,ybp,sb,nb);
    }

    nnew = arefine(xb,yb,sb,xbp,ybp,nb,atol,IBX,w1,w2,xrf1,xrf2);

    nbadd = nnew - nb;
    //TRACE("Number of points added: %d\n", nbadd);

    nb = nnew;
    for(int i=1; i<=nb; i++){
        xb[i] = w1[i];
        yb[i] = w2[i];
    }

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,
           radble,angbte,ei11ba,ei22ba,apx1ba,apx2ba,
           ei11bt,ei22bt,apx1bt,apx2bt);


    cang(x,y,n,imax,amax);

    return nbadd;
}


void XFoil::flap()
{
    //----------------------------------------------------
    //     modifies buffer airfoil for a deflected flap.
    //     points may be added/subtracted in the flap
    //     break vicinity to clean things up.
    //----------------------------------------------------

    bool lchange(false);
    bool insid(false);
    int i(0), it2q(0), ib2q(0), idif(0);
    int npadd(0), ip(0);
    double atop(0), abot(0), chx(0), chy(0), fvx(0), fvy(0), crsp(0);
    double st1(0), st2(0), sb1(0), sb2(0), xt1(0), yt1(0), xb1(0);
    double yb1(0), sb1p(0), sb1q(0), sb2p(0), sb2q(0);
    //    double xb2, yb2, xt2, yt2;
    double dsavg(0), sfrac(0), st1p(0), st1q(0), st2p(0), st2q(0);
    double dsnew(0);
    double tops(0), bots(0);
    double sind(0), cosd(0), dang(0), ang(0), ca(0), sa(0);
    double xbar(0), ybar(0);
    double stol(0);
    bool lt1new(false);
    bool lt2new(false);
    bool lb1new(false);
    bool lb2new(false);
    int it1(0);
    int it2(0);
    int ib1(0);
    int ib2(0);

    double xt1new = 0.0;
    double yt1new= 0.0;
    double xt2new= 0.0;
    double yt2new= 0.0;
    double xb1new= 0.0;
    double yb1new= 0.0;
    double xb2new= 0.0;
    double yb2new= 0.0;


    /*  if(ninput>=2) {
        xbf = rinput[1];
        ybf = rinput[2];
    }
    else{
        xbf = -999.0;
        ybf = -999.0;
}*/

    getxyf(xb,xbp,yb,ybp,sb,nb,tops,bots,xbf, ybf);
    insid = inside(xb,yb,nb,xbf,ybf);

    //      write(*,1050) xbf, ybf
    //1050 format(/' flap hinge: x,y =', 29.5 )

    double rdef = ddef*PI/180.0;//ddef : flap deflection in degrees
    if(fabs(rdef) <= 0.001) return;

    if(insid) {
        atop = std::max( 0.0 , -rdef );
        abot = std::max( 0.0 ,  rdef );
    }
    else{
        chx = deval(bots,xb,xbp,sb,nb) - deval(tops,xb,xbp,sb,nb);
        chy = deval(bots,yb,ybp,sb,nb) - deval(tops,yb,ybp,sb,nb);
        fvx = seval(bots,xb,xbp,sb,nb) + seval(tops,xb,xbp,sb,nb);
        fvy = seval(bots,yb,ybp,sb,nb) + seval(tops,yb,ybp,sb,nb);
        crsp = chx*(ybf-0.5*fvy) - chy*(xbf-0.5*fvx);
        if(crsp >0.0) {
            //------ flap hinge is above airfoil
            atop = std::max(0.0, rdef);
            abot = std::max(0.0, rdef);
        }
        else{
            //------ flap hinge is below airfoil
            atop = std::max(0.0, -rdef);
            abot = std::max(0.0, -rdef);
        }
    }

    //-- find upper and lower surface break arc length values...


    sss(tops,&st1,&st2,atop,xbf,ybf,xb,xbp,yb,ybp,sb,nb,1);
    sss(bots,&sb1,&sb2,abot,xbf,ybf,xb,xbp,yb,ybp,sb,nb,2);

    //-- ... and x,y coordinates
    xt1 = seval(st1,xb,xbp,sb,nb);
    yt1 = seval(st1,yb,ybp,sb,nb);
    //    xt2 = seval(st2,xb,xbp,sb,nb);
    //    yt2 = seval(st2,yb,ybp,sb,nb);
    xb1 = seval(sb1,xb,xbp,sb,nb);
    yb1 = seval(sb1,yb,ybp,sb,nb);
    //    xb2 = seval(sb2,xb,xbp,sb,nb);
    //    yb2 = seval(sb2,yb,ybp,sb,nb);

    //      write(*,1100) xt1, yt1, xt2, yt2, xb1, yb1, xb2, yb2
    // 1100 format(/' top breaks: x,y =  ', 29.5, 4x, 29.5
    //     &       /' bot breaks: x,y =  ', 29.5, 4x, 29.5)

    //-- find points adjacent to breaks
    for(i=1; i<=nb-1; i++){
        if(sb[i]<=st1 && sb[i+1]> st1) it1 = i+1;
        if(sb[i]< st2 && sb[i+1]>=st2) it2 = i;
        if(sb[i]<=sb1 && sb[i+1]> sb1) ib1 = i;
        if(sb[i]< sb2 && sb[i+1]>=sb2) ib2 = i+1;
    }

    dsavg = (sb[nb]-sb[1])/double(nb-1);

    //-- smallest fraction of s increments i+1 and i+2 away from break point
    sfrac = 0.33333;

    if(atop != 0.0) {
        st1p = st1 + sfrac*(sb[it1  ]-st1);
        st1q = st1 + sfrac*(sb[it1+1]-st1);
        if(sb[it1] < st1q) {
            //------ simply move adjacent point to ideal sfrac location
            xt1new = seval(st1q,xb,xbp,sb,nb);
            yt1new = seval(st1q,yb,ybp,sb,nb);
            lt1new = false;
        }
        else{
            //------ make new point at sfrac location
            xt1new = seval(st1p,xb,xbp,sb,nb);
            yt1new = seval(st1p,yb,ybp,sb,nb);
            lt1new = true;
        }

        st2p = st2 + sfrac*(sb[it2 ]-st2);
        it2q = std::max(it2-1,1);
        st2q = st2 + sfrac*(sb[it2q]-st2);
        if(sb[it2] >st2q) {
            //------ simply move adjacent point
            xt2new = seval(st2q,xb,xbp,sb,nb);
            yt2new = seval(st2q,yb,ybp,sb,nb);
            lt2new = false;
        }
        else{
            //------ make new point
            xt2new = seval(st2p,xb,xbp,sb,nb);
            yt2new = seval(st2p,yb,ybp,sb,nb);
            lt2new = true;
        }
    }

    if(abot != 0.0) {
        sb1p = sb1 + sfrac*(sb[ib1  ]-sb1);
        sb1q = sb1 + sfrac*(sb[ib1-1]-sb1);
        if(sb[ib1] >sb1q) {
            //------ simply move adjacent point
            xb1new = seval(sb1q,xb,xbp,sb,nb);
            yb1new = seval(sb1q,yb,ybp,sb,nb);
            lb1new = false;
        }
        else{
            //------ make new point
            xb1new = seval(sb1p,xb,xbp,sb,nb);
            yb1new = seval(sb1p,yb,ybp,sb,nb);
            lb1new = true;
        }

        sb2p = sb2 + sfrac*(sb[ib2 ]-sb2);
        ib2q = std::min(ib2+1,nb);
        sb2q = sb2 + sfrac*(sb[ib2q]-sb2);
        if(sb[ib2] < sb2q) {
            //------ simply move adjacent point
            xb2new = seval(sb2q,xb,xbp,sb,nb);
            yb2new = seval(sb2q,yb,ybp,sb,nb);
            lb2new = false;
        }
        else{
            //------ make new point
            xb2new = seval(sb2p,xb,xbp,sb,nb);
            yb2new = seval(sb2p,yb,ybp,sb,nb);
            lb2new = true;
        }
    }

    //      dstop = fabs(sb(it2)-sb(it1));
    //      dsbot = fabs(sb(ib2)-sb(ib1));

    sind =  sin(rdef);
    cosd =  cos(rdef);

    //-- rotate flap points about the hinge point (xbf,ybf)
    for (i=1; i<=nb; i++){
        //        if(i>=it1 && i<=ib1) go to 10
        if(i<it1 || i>ib1) {
            xbar = xb[i] - xbf;
            ybar = yb[i] - ybf;
            xb[i] = xbf  +  xbar*cosd  +  ybar*sind;
            yb[i] = ybf  -  xbar*sind  +  ybar*cosd;
        }
    }

    idif = it1-it2-1;
    if(idif>0) {
        //--- delete points on upper airfoil surface which "disappeared".
        nb  = nb -idif;
        it1 = it1-idif;
        ib1 = ib1-idif;
        ib2 = ib2-idif;
        for( i=it2+1; i<=nb; i++){
            sb[i]  = sb[i+idif];
            xb[i] = xb[i+idif];
            yb[i] = yb[i+idif];
        }
    }

    idif = ib2-ib1-1;
    if(idif>0) {
        //--- delete points on lower airfoil surface which "disappeared".
        nb  = nb -idif;
        ib2 = ib2-idif;
        for(i=ib1+1; i<=nb; i++){
            sb[i] = sb[i+idif];
            xb[i] = xb[i+idif];
            yb[i] = yb[i+idif];
        }
    }

    if(fabs(atop) < 0.000001) {

        //---- arc length of newly created surface on top of airfoil
        dsnew = fabs(rdef)*sqrt((xt1-xbf)* (xt1-xbf) + (yt1-ybf)* (yt1-ybf));

        //---- number of points to be added to define newly created surface
        npadd = int(1.5*dsnew/dsavg + 1.0);
        //     npadd = int(1.5*dsnew/dstop + 1.0)

        //---- skip everything if no points are to be added
        if(npadd!=0) {//go to 35

            //---- increase coordinate array length to make room for the new point(s)
            nb  = nb +npadd;
            it1 = it1+npadd;
            ib1 = ib1+npadd;
            ib2 = ib2+npadd;
            for(i=nb; i>= it1; i--){
                xb[i] = xb[i-npadd];
                yb[i] = yb[i-npadd];
            }

            //---- add new points along the new surface circular arc segment
            dang = rdef / double(npadd);
            xbar = xt1 - xbf;
            ybar = yt1 - ybf;
            for(ip=1; ip<= npadd; ip++){
                ang = dang*(double(ip) - 0.5);
                ca = cos(ang);
                sa = sin(ang);

                xb[it1-ip] = xbf  +  xbar*ca + ybar*sa;
                yb[it1-ip] = ybf  -  xbar*sa + ybar*ca;
            }
        }
    }
    else{

        //---- set point in the corner and possibly two adjacent points
        npadd = 1;
        if(lt2new) npadd = npadd+1;
        if(lt1new) npadd = npadd+1;

        nb  = nb +npadd;
        it1 = it1+npadd;
        ib1 = ib1+npadd;
        ib2 = ib2+npadd;
        for (i=nb; i>= it1; i--){
            xb[i] = xb[i-npadd];
            yb[i] = yb[i-npadd];
        }

        if(lt1new) {
            xb[it1-1] = xt1new;
            yb[it1-1] = yt1new;
            xb[it1-2] = xt1;
            yb[it1-2] = yt1;
        }
        else {
            xb[it1  ] = xt1new;
            yb[it1  ] = yt1new;
            xb[it1-1] = xt1;
            yb[it1-1] = yt1;
        }

        xbar = xt2new - xbf;
        ybar = yt2new - ybf;
        if(lt2new) {
            xb[it2+1] = xbf  +  xbar*cosd + ybar*sind;
            yb[it2+1] = ybf  -  xbar*sind + ybar*cosd;
        }
        else{
            xb[it2  ] = xbf  +  xbar*cosd + ybar*sind;
            yb[it2  ] = ybf  -  xbar*sind + ybar*cosd;
        }

    }
    //            35 continue


    if(fabs(abot) <= 0.000001) {

        //---- arc length of newly created surface on top of airfoil
        dsnew = fabs(rdef)*sqrt((xb1-xbf)* (xb1-xbf) + (yb1-ybf)* (yb1-ybf));

        //---- number of points to be added to define newly created surface
        npadd = int(1.5*dsnew/dsavg + 1.0);

        //---- skip everything if no points are to be added
        if(npadd!=0) {//go to 45

            //---- increase coordinate array length to make room for the new point(s)
            nb  = nb +npadd;
            ib2 = ib2+npadd;
            for(i=nb;i>= ib2; i--){
                xb[i] = xb[i-npadd];
                yb[i] = yb[i-npadd];
            }

            //---- add new points along the new surface circular arc segment
            dang = rdef / double(npadd);
            xbar = xb1 - xbf;
            ybar = yb1 - ybf;
            for(ip=1; ip<= npadd; ip++){
                ang = dang*(double(ip) - 0.5);
                ca = cos(ang);
                sa = sin(ang);

                xb[ib1+ip] = xbf  +  xbar*ca + ybar*sa;
                yb[ib1+ip] = ybf  -  xbar*sa + ybar*ca;
            }
        }
    }
    else{

        //---- set point in the corner and possibly two adjacent points
        npadd = 1;
        if(lb2new) npadd = npadd+1;
        if(lb1new) npadd = npadd+1;

        nb  = nb +npadd;
        ib2 = ib2+npadd;
        for (i=nb; i>=ib2; i--){
            xb[i] = xb[i-npadd];
            yb[i] = yb[i-npadd];
        }

        if(lb1new) {
            xb[ib1+1] = xb1new;
            yb[ib1+1] = yb1new;
            xb[ib1+2] = xb1;
            yb[ib1+2] = yb1;        }
        else{
            xb[ib1  ] = xb1new;
            yb[ib1  ] = yb1new;
            xb[ib1+1] = xb1;
            yb[ib1+1] = yb1;
        }

        xbar = xb2new - xbf;
        ybar = yb2new - ybf;
        if(lb2new) {
            xb[ib2-1] = xbf  +  xbar*cosd + ybar*sind;
            yb[ib2-1] = ybf  -  xbar*sind + ybar*cosd;
        }
        else{
            xb[ib2  ] = xbf  +  xbar*cosd + ybar*sind;
            yb[ib2  ] = ybf  -  xbar*sind + ybar*cosd;
        }

    }
    //   45 continue


    //-- check new geometry for splinter segments
    stol = 0.2;
    scheck(xb,yb,&nb, stol, &lchange);

    //-- spline new geometry
    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    //    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,
    //        radble,angbte,ei11ba,ei22ba,apx1ba,apx2ba,
    //        ei11bt,ei22bt,apx1bt,apx2bt,thickb,cambrb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,
           radble,angbte,ei11ba,ei22ba,apx1ba,apx2ba,
           ei11bt,ei22bt,apx1bt,apx2bt);

    lbflap = true;

    //        if(lgsym) {
    //       write(*,*)
    //       write(*,*) 'disabling symmetry enforcement'
    //            lgsym = false;
    //        }

    //        lgeopl = false; // plot?
    lgsame = false;// plot?
}


bool XFoil::CheckAngles()
{
    cang(x,y,n, imax,amax);
    if(fabs(amax)>angtol)
    {
        return true;// we have a coarse paneling
    }
    return false;// we have a fine paneling
}


bool XFoil::eiwset(int nc1)
{
    //----------------------------------------------------
    //     calculates the uniformly-spaced circle-plane
    //     coordinate array wc (omega), and the
    //     corresponding complex unit numbers exp(inw)
    //     for slow fourier transform operations.
    //----------------------------------------------------
    //      include 'circle.inc'

    //      PI = 4.0*atan(1.0)
    int ic=0;
    //---- set requested number of points in circle plane
    nc  = nc1;
    mc  = int(nc1/4);
    mct = int(nc1/16);

    if(nc>ICX)
    {
        writeString("eiwset: Array overflow. Increase ICX.");
        return false;
    }

    dwc = 2.0*PI / double(nc-1);

    for (ic=1; ic<=nc; ic++)  wc[ic] = dwc*double(ic-1);


    //---- set  m = 0  numbers
    for (ic=1; ic<=nc; ic++)
        eiw[ic][0] = std::complex<double>(1.0, 0.0);


    //---- set  m = 1  numbers
    for (ic=1; ic<=nc; ic++)
        eiw[ic][1] = exp(std::complex<double>( 0.0 , wc[ic] ) );


    //---- set  m > 1  numbers by indexing appropriately from  m = 1  numbers
    for(int m=2; m<= mc; m++){
        for (int ic=1; ic<= nc;ic++){
            int ic1 = m*(ic-1);
            ic1 = ic1%(nc-1) +1;
            eiw[ic][m] = eiw[ic1][1];
        }
    }

    return true;

}


void XFoil::scinit(int n, double x[], double xp[], double y[], double yp[], double s[], double sle)
{
    //----------------------------------------------------------
    //     calculates the circle-plane coordinate s(w) = sc
    //     at each point of the current geometry.
    //     a by-product is the complex-mapping coefficients cn.
    //     (see cncalc header for more info).
    //----------------------------------------------------------

    //     include 'circle.inc'
    std::complex<double> dcn=0, zle=0, zte=0;
    int ipass=0, ic=0;
    double sic=0, dxds=0, dyds=0, qim=0, dzwt=0;

    double ceps = 1.e-7;
    double seps = 5.e-7;

    //---- set te angle parameter
    agte = ( atan2(xp[n], -yp[n])
             -atan2(xp[1], -yp[1]))/PI - 1.0;

    //---- set surface angle at first point
    ag0 = atan2( xp[1] , -yp[1] );

    //---- temporary offset qo to make  q(w)-qo = 0  at  w = 0 , 2 PI
    //     --- avoids gibbs problems with q(w)'s fourier sine transform
    qim0 = ag0 + 0.5*PI*(1.0+agte);

    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);

    //---- save te gap and airfoil chord
    double dxte = x[1] - x[n];
    double dyte = y[1] - y[n];
    dzte = std::complex<double>(dxte,dyte);

    double chordx = 0.5*(x[1]+x[n]) - xle;
    double chordy = 0.5*(y[1]+y[n]) - yle;
    chordz = std::complex<double>( chordx , chordy );
    zleold = std::complex<double>(xle, yle);

    //      write(*,1100) real(dzte), imag(dzte), agte*180.0
    //1100 format(/' current te gap  dx dy =', 27.4,
    //     &        '    te angle =', f7.3,' deg.' / )
    //      write(*,*) 'initializing mapping coordinate ...'

    //---- set approximate slope ds/dw at airfoil nose
    double cvle = curv(sle,x,xp,y,yp,s,n) * s[n];
    double cvabs = fabs(cvle);
    double dsdwle = std::max(0.001, 0.5/cvabs );

    double tops = sle/s[n];
    double bots = (s[n]-sle)/s[n];

    //---- set initial top surface s(w)
    double wwt = 1.0 - 2.0*dsdwle/tops;
    for (ic=1;ic<= (nc-1)/2+1;ic++)
        sc[ic] = tops*(1.0 - cos(wwt*wc[ic]) ) /(1.0 - cos(wwt*PI    ) );


    //---- set initial bottom surface s(w)
    wwt = 1.0 - 2.0*dsdwle/bots;
    for(ic=(nc-1)/2+2; ic<=nc ; ic++)
        sc[ic] =  1.0 - bots *(1.0 - cos(wwt*(wc[nc]-wc[ic])) )
                /(1.0 - cos(wwt* PI            ) );


    //---- iteration loop for s(w) array
    for (ipass=1; ipass<= 30; ipass++){

        //---- calculate imaginary part of harmonic function  p(w) + iq(w)
        for (ic=1; ic<= nc;ic++){

            sic = s[1] + (s[n]-s[1])*sc[ic];
            dxds = deval(sic,x,xp,s,n);
            dyds = deval(sic,y,yp,s,n);

            //------ set q(w) - qo   (qo defined so that q(w)-qo = 0  at  w = 0 , 2 PI)
            qim = atan2(dxds, -dyds ) - 0.5*(wc[ic]-PI)*(1.0+agte)- qim0;
            piq[ic] = std::complex<double>(0.0, qim );

        }

        //---- fourier-decompose q(w)
        ftp();

        //---- zero out average real part and add on qo we took out above
        cn[0] = std::complex<double>( 0.0 , imag(cn[0])+qim0 );

        //---- transform back to get entire  piq = p(w) + iq(w)
        piqsum();

        //---- save s(w) for monitoring of changes in s(w) by zccalc
        for (ic=1; ic<= nc;ic++) scold[ic] = sc[ic];


        //---- correct n=1 complex coefficient cn for proper te gap
        for (int itgap=1; itgap<= 5;itgap++)
        {
            zccalc(1);

            //------ set current le,te locations
            zlefind(&zle,zc,wc,nc,piq,agte);

            zte = 0.5*(zc[1]+zc[nc]);

            dzwt = abs(zte-zle)/abs(chordz);
            dcn = -(zc[1] - zc[nc] - dzwt*dzte ) / (zc_cn[1][1] - zc_cn[nc][1]);
            cn[1] = cn[1] + dcn;

            piqsum();

            if(std::abs(dcn) < ceps) break;
            //            if(real(dcn)*real(dcn)+imag(dcn)*imag(dcn) < ceps*ceps) break;
        }


        double dscmax = 0.0;
        for(ic=1; ic<=nc;ic++)
            dscmax = std::max( dscmax , fabs(sc[ic]-scold[ic]) );

        if(dscmax < seps) break;

    }
    //  505 continue

    //---- normalize final geometry
    zcnorm(1);

    //---- set final  s(w), x(w), y(w)  arrays for old airfoil
    for (ic=1; ic<= nc; ic++){
        scold[ic] = sc[ic];
        xcold[ic] = real(zc[ic]);
        ycold[ic] = imag(zc[ic]);
    }

    for(ic=1; ic<= nc;ic++){
        double sinw = 2.0*sin(0.5*wc[ic]);
        double sinwe = 0.0;
        if(sinw>0.0) sinwe = pow(sinw,(1.0-agte));

        double hwc = 0.5*(wc[ic]-PI)*(1.0+agte) - 0.5*PI;
        zcoldw[ic] = sinwe * exp( piq[ic] + std::complex<double>(0.0,hwc) );
    }

    qimold = imag(cn[0]);
    return;
}


void XFoil::ftp()
{
    //----------------------------------------------------------------
    //     slow-fourier-transform p(w) using trapezoidal integration.
    //----------------------------------------------------------------

    std::complex<double> zsum=0;

    for (int m=0; m<= mc;m++){
        zsum = std::complex<double>(0.0,0.0);
        for(int ic=2; ic<= nc-1; ic++)
            zsum = zsum + piq[ic]*eiw[ic][m];

        cn[m] = (0.5*(piq[1]*eiw[1][m] + piq[nc]*eiw[nc][m])+ zsum)*dwc / PI;
    }
    cn[0] = 0.5*cn[0];

    return;
}


void XFoil::piqsum()
{
    //---------------------------------------------
    //     inverse-transform to get back modified
    //     speed function and its conjugate.
    //---------------------------------------------
    std::complex<double> zsum=0;

    for(int ic=1; ic <= nc; ic++){
        zsum = std::complex<double>(0.0,0.0);
        for(int m=0; m<= mc; m++){
            zsum = zsum + cn[m]*conjg(eiw[ic][m]);
        }
        piq[ic] = zsum;
    }

    return;
}


std::complex<double> XFoil::conjg(std::complex<double> cplx)
{
    double a = real(cplx);
    double b = imag(cplx);
    return std::complex<double>(a, -b);
}


void XFoil::zcnorm(int mtest)
{
    //-----------------------------------------------
    //    normalizes the complex airfoil z(w) to
    //    the old chord and angle, and resets the
    //    influence coefficients  dz/dcn .
    //-----------------------------------------------
    //      include 'circle.inc'
//    std::complex<double> dzdw1, dzdw2;
    std::complex<double> zcnew=0, zle=0, zte=0, zc_zte=0, zte_cn[IMX4+1];
    int m=0, ic=0;

    //---- find current le location
    zlefind(&zle,zc,wc,nc,piq,agte);

    //---- place leading edge at origin
    for (ic=1; ic <= nc; ic++)   {
        zc[ic] = zc[ic] - zle;
    }

    //---- set normalizing quantities and sensitivities
    zte = 0.5*(zc[1] + zc[nc]);
    for (m=1; m<= mtest; m++)   zte_cn[m] = 0.5*(zc_cn[1][m] + zc_cn[nc][m]);

    //---- normalize airfoil to proper chord, put le at old position,
    //-    and set sensitivities dz/dcn for the rescaled coordinates
    for (ic=1; ic<= nc; ic++){
        zcnew  = chordz*zc[ic]/zte;
        zc_zte = -zcnew/zte;
        zc[ic] = zcnew;
        for (m=1; m<= mtest; m++)  zc_cn[ic][m] = chordz*zc_cn[ic][m]/zte + zc_zte*zte_cn[m];
    }

    //---- add on rotation to mapping coefficient so qccalc gets the right alpha
    double qimoff = -imag( log(chordz/zte) );
    cn[0] = cn[0] - std::complex<double>(0.0, qimoff);

    //---- shift airfoil to put le at old location
    for (ic=1; ic<= nc; ic++)
        zc[ic] = zc[ic] + zleold;

    return;
}


void XFoil::zccalc(int mtest)
{
    //--------------------------------------------------------
    //    calculates the airfoil geometry z(w) from the
    //    harmonic function p(w) + iq(w).  also normalizes
    //    the coordinates to the old chord and calculates
    //    the geometry sensitivities dz/dcn  (1 < n < mtest)
    //    for each point.
    //--------------------------------------------------------
    //      include 'circle.inc'
    std::complex<double> dzdw1=0, dzdw2=0, dz_piq1=0, dz_piq2=0;

    //---- integrate upper airfoil surface coordinates from x,y = 4,0
    int ic = 1;
    zc[ic] = std::complex<double>(4.0,0.0);
    for (int m=1; m<= mtest; m++) zc_cn[ic][m] = std::complex<double>(0.0,0.0);

    double sinw = 2.0*sin(0.5*wc[ic]);
    double sinwe = 0.0;
    if(sinw>0.0) sinwe = pow(sinw,(1.0-agte));

    double hwc = 0.5*(wc[ic]-PI)*(1.0+agte) - 0.5*PI;
    dzdw1 = sinwe * exp( piq[ic] + std::complex<double>(0.0,hwc) );

    for (ic=2; ic<= nc; ic++){

        sinw = 2.0*sin(0.5*wc[ic]);
        sinwe = 0.0;
        if(sinw>0.0) sinwe = pow(sinw,(1.0-agte));

        hwc = 0.5*(wc[ic]-PI)*(1.0+agte) - 0.5*PI;
        dzdw2 = sinwe * exp( piq[ic] + std::complex<double>(0.0,hwc));

        zc[ic]  = 0.5*(dzdw1+dzdw2)*dwc + zc[ic-1];
        dz_piq1 = 0.5*(dzdw1      )*dwc;
        dz_piq2 = 0.5*(      dzdw2)*dwc;

        for (int m=1; m<= mtest; m++){
            zc_cn[ic][m] = dz_piq1*conjg(eiw[ic-1][m])
                    + dz_piq2*conjg(eiw[ic][m])
                    + zc_cn[ic-1][m];
        }

        dzdw1 = dzdw2;
    }

    //---- set arc length array s(w)
    sc[1] = 0.0;
    for (ic=2; ic<= nc; ic++)   sc[ic] = sc[ic-1] + std::abs(zc[ic]-zc[ic-1]);


    //---- normalize arc length
    for (ic=1; ic<= nc; ic++)  sc[ic] = sc[ic]/sc[nc];

    return;
}


void XFoil::zlefind(std::complex<double>*zle,std::complex<double>zc[],double wc[],
                    int nc,std::complex<double>piq[], double agte)
{

    std::complex<double> dzdw1=0, dzdw2=0, zte=0;
    int ic=0, ic1=0, ic2=0;
    //---- temporary work arrays for splining near leading edge
    int ntx=33;
    double xc[33+1],yc[33+1], xcw[33+1],ycw[33+1];

    int i, icle, nic;
    icle = 0;// added techwinder

    zte = 0.5*(zc[1]+zc[nc]);

    //---- find point farthest from te
    double dmax = 0.0;
    for (ic = 1; ic<= nc;ic++){
        double dist = std::abs( zc[ic] - zte);

        if(dist>dmax) {
            dmax = dist;
            icle = ic;
        }
    }

    //---- set restricted spline limits around leading edge
    ic1 = std::max( icle - (ntx-1)/2 ,  1 );
    ic2 = std::min( icle + (ntx-1)/2 , nc );

    //---- set up derivatives at spline endpoints
    double sinw = 2.0*sin(0.5*wc[ic1]);
    double sinwe = pow(sinw,(1.0-agte));
    double hwc = 0.5*(wc[ic1]-PI)*(1.0+agte) - 0.5*PI;
    dzdw1 = sinwe * exp( piq[ic1] + std::complex<double>(0.0,hwc));

    sinw = 2.0*sin(0.5*wc[ic2]);
    sinwe = pow(sinw,(1.0-agte));
    hwc = 0.5*(wc[ic2]-PI)*(1.0+agte) - 0.5*PI;
    dzdw2 = sinwe * exp(piq[ic2] + std::complex<double>(0.0,hwc));

    //---- fill temporary x,y coordinate arrays
    for(ic=ic1; ic<=ic2;ic++){
        i = ic-ic1+1;
        xc[i] = real(zc[ic]);
        yc[i] = imag(zc[ic]);
    }

    //---- calculate spline near leading edge with derivative end conditions
    nic = ic2 - ic1 + 1;

    splind(xc,xcw,wc+ic1-1,nic,real(dzdw1),real(dzdw2));
    splind(yc,ycw,wc+ic1-1,nic,imag(dzdw1),imag(dzdw2));

    double xcte = 0.5*real(zc[1] + zc[nc]);
    double ycte = 0.5*imag(zc[1] + zc[nc]);

    //---- initial guess for leading edge coordinate
    double wcle = wc[icle];

    //---- newton loop for improved leading edge coordinate
    double xcle, ycle, dxdw, dydw, dxdd, dydd, xchord, ychord, res, resw, dwcle;
    bool found = false;

    for (int itcle=1; itcle <=10; itcle++){
        xcle = seval(wcle,xc,xcw,wc+ic1-1,nic);
        ycle = seval(wcle,yc,ycw,wc+ic1-1,nic);
        dxdw = deval(wcle,xc,xcw,wc+ic1-1,nic);
        dydw = deval(wcle,yc,ycw,wc+ic1-1,nic);
        dxdd = d2val(wcle,xc,xcw,wc+ic1-1,nic);
        dydd = d2val(wcle,yc,ycw,wc+ic1-1,nic);

        xchord = xcle - xcte;
        ychord = ycle - ycte;

        //------ drive dot product between chord line and le tangent to zero
        res  = xchord*dxdw + ychord*dydw;
        resw = dxdw  *dxdw + dydw  *dydw + xchord*dxdd + ychord*dydd;

        dwcle = -res/resw;
        wcle = wcle + dwcle;

        if(fabs(dwcle)<0.00001) {
            found = true;
            break;//go to 51
        }
    }
    if (!found)
    {
        writeString("zlefind: le location failed.");
        wcle = wc[icle];
    }
    //   51 continue

    //---- set final leading edge point complex coordinate
    xcle = seval(wcle,xc,xcw,wc+ic1-1, nic);
    ycle = seval(wcle,yc,ycw,wc+ic1-1, nic);
    *zle = std::complex<double>(xcle,ycle);

    return;
}


void XFoil::mapgam(int iac, double &alg, double &clg, double &cmg)
{
    //--------------------------------------------
    //     sets mapped q for current airfoil
    //     for angle of attack or cl.
    //
    //       iac=1: specified algam
    //       iac=2: specified clgam
    //--------------------------------------------
    //
    //---- calculate q(w), set number of circle points nsp
    qccalc(iac,&alg,&clg,&cmg,minf,qinf,&nsp,w1,w2,w5,w6);

    //---- store q(w), s(w), x(w), y(w)
    double chx = xte - xle;
    double chy = yte - yle;
    double chsq = chx*chx + chy*chy;
    for(int i=1; i<=nsp;i++)
    {
        qgamm[i] = w6[i];
        sspec[i] = w5[i];
        double xic = seval(s[n]*sspec[i],x,xp,s,n);
        double yic = seval(s[n]*sspec[i],y,yp,s,n);
        xspoc[i] = ((xic-xle)*chx + (yic-yle)*chy)/chsq;
        yspoc[i] = ((yic-yle)*chx - (xic-xle)*chy)/chsq;
    }
    ssple = sle/s[n];

    return;
}


void XFoil::qccalc(int ispec,double *alfa, double *cl, double *cm,
                   double minf, double qinf, int *ncir,
                   double xcir[], double ycir[], double scir[], double qcir[])
{
    //---------------------------------------------------
    //    Calculates the surface speed from the complex
    //    speed function so that either a prescribed
    //    alfa or cl is achieved, depending on whether
    //    ispec=1 or 2.  The cl calculation uses the
    //    transformed karman-tsien cp.
    //---------------------------------------------------
    std::complex<double> dz, za, eia, cmt,cft,cft_a;
    //    double rr1, rr2, rr3;
    int icp, ic, ipass;
    //    int ll;
    double dalfa = 0.0;
    double alfcir, ppp, eppp;
    double sinw, sinwe;
    double cpinc1, cpi_q1, cpcom1,cpc_q1, cpc_a1;
    double cpinc2, cpi_q2, cpcom2,cpc_q2, cpc_a2;
    double qc_a[ICX+1];
    //    double minf;
    //    double aeps = 5.0 *pow(10,-7);
    double aeps = 5.0e-007;

    //---- karman-tsien quantities
    double beta = sqrt(1.0 - minf*minf);
    double bfac = 0.5*minf*minf / (1.0 + beta);

    *ncir = nc;

    /*    for (ll=1; ll<ICX+1; ll++){
        rr1  = xcir[ll];
        rr2  = ycir[ll];
        rr3  = scir[ll];
    }*/

    //---- newton iteration loop (executed only once if alpha specified)
    for (ipass=1; ipass<= 10; ipass++){

        //------ set alpha in the circle plane
        alfcir = *alfa - imag(cn[0]);

        cmt   = std::complex<double>(0.0,0.0);
        cft   = std::complex<double>(0.0,0.0);
        cft_a = std::complex<double>(0.0,0.0);

        //------ set surface speed for current circle plane alpha
        for (ic=1; ic<= nc; ic++){
            ppp = real(piq[ic]);
            eppp = exp(-ppp);
            sinw = 2.0*sin(0.5*wc[ic]);

            if(fabs(agte)<=0.0001)  sinwe = 1.0;
            else if(sinw>0.0) sinwe = pow(sinw,agte);
            else sinwe = 0.0;

            qcir[ic] = 2.0*cos(0.5*wc[ic] - alfcir)*sinwe * eppp;
            qc_a[ic] = 2.0*sin(0.5*wc[ic] - alfcir)*sinwe * eppp;

            xcir[ic] = real(zc[ic]);
            ycir[ic] = imag(zc[ic]);
            scir[ic] = sc[ic];
        }

        //------ integrate compressible  cp dz  to get complex force  cl + icd
        ic = 1;
        cpinc1 = 1.0 - (qcir[ic]/qinf)*(qcir[ic]/qinf);
        cpi_q1 =   -2.0*qcir[ic]/qinf/qinf;
        cpcom1 = cpinc1 / (beta + bfac*cpinc1);
        cpc_q1 = (1.0 - bfac*cpcom1)/(beta + bfac*cpinc1) * cpi_q1;
        cpc_a1 = cpc_q1*qc_a[ic];
        for(ic=1; ic<=nc; ic++)
        {
            icp = ic+1;
            if(ic==nc) icp = 1;

            cpinc2 = 1.0 - (qcir[icp]/qinf)*(qcir[icp]/qinf);
            cpi_q2 =   -2.0*qcir[icp]/qinf/qinf;
            cpcom2 = cpinc2 / (beta + bfac*cpinc2);
            cpc_q2 = (1.0 - bfac*cpcom2)/(beta + bfac*cpinc2) * cpi_q2;
            cpc_a2 = cpc_q2*qc_a[icp];

            za = (zc[icp] + zc[ic])*0.5 - std::complex<double>(0.25,0.0);
            dz =  zc[icp] - zc[ic];

            cmt   = cmt   - 0.5*(cpcom1 + cpcom2)*dz*conjg(za)
                    +      (cpcom1 - cpcom2)*dz*conjg(dz)/12.0;
            cft   = cft   + 0.5*(cpcom1 + cpcom2)*dz;
            cft_a = cft_a + 0.5*(cpc_a1 + cpc_a2)*dz;

            cpcom1 = cpcom2;
            cpc_a1 = cpc_a2;
        }

        //------ rotate force vector into freestream coordinates

        eia = exp(std::complex<double>(0.0,- *alfa));
        cft   = cft  *eia;
        cft_a = cft_a*eia + cft*std::complex<double>(0.0,-1.0);

        //------ lift is real part of complex force vector
        double clt   = real(cft);
        double clt_a = real(cft_a);

        //------ moment is real part of complex moment
        *cm = real(cmt);

        if(ispec==1) {
            //------- if alpha is prescribed, we're done
            *cl = clt;
            return;
        }
        else{
            //------- adjust alpha with newton-raphson to get specified cl
            dalfa = (*cl - clt)/clt_a;
            *alfa = *alfa + dalfa;
            if(fabs(dalfa) < aeps) return;
        }

    }
//    std::string str = QString::asprintf("qccalc: cl convergence failed.  dalpha = {:.3g}", dalfa);
//    writeString(str);
}


void XFoil::mapgen(int n, double x[], double y[])
{
    //-------------------------------------------------------
    //     calculates the geometry from the speed function
    //     fourier coefficients cn, modifying them as needed
    //     to achieve specified constraints.
    //-------------------------------------------------------
    //      include 'circle.inc'
    //      dimension x(nc), y(nc)
    //
    (void)n;
    std::complex<double> qq[IMX4+1][IMX4+1];
    std::complex<double> dcn[IMX4+1];
    double dx=0, dy=0, qimoff=0, dcnmax=0;
    int ncn=0, m=0;

    //--- preset rotation offset of airfoil so that initial angle is close
    //-    to the old airfoil's angle
    dx = xcold[2] - xcold[1];
    dy = ycold[2] - ycold[1];
    qim0 = atan2( dx , -dy )  +  0.5*PI*(1.0+agte);

    qimoff = qim0 - imag(cn[0]);
    cn[0] = cn[0] + std::complex<double>( 0.0 , qimoff );

    //--- inverse-transform and calculate geometry zc = z(w)
    //ccc   call cnfilt(ffilt)
    piqsum();
    zccalc(mct);

    //--- scale,rotate z(w) to get previous chord and orientation
    zcnorm(mct);

    //cc//-- put back rotation offset so speed routine qccalc gets the right alpha
    //cc      cn[0] = cn[0] - cmplx( 0.0 , qimoff )

    //--- enforce lighthill's first constraint
    cn[0] = std::complex<double>( 0.0, imag(cn[0]) );

    //--- number of free coefficients
    ncn = 1;

    //--- newton iteration loop for modified cn's
    for( int itercn=1; itercn<= 10;itercn++){

        //----- fix te gap
        m = 1;
        dcn[m] = zc[1] - zc[nc]  -  dzte;
        for (int l=1; l<= ncn;l++){
            qq[m][l] = zc_cn[1][l] - zc_cn[nc][l];
        }

        cgauss(ncn,qq,dcn);

        dcnmax = 0.0;
        for(int m=1; m<= ncn;m++){
            cn[m] = cn[m] - dcn[m];
            dcnmax = std::max(std::abs(dcn[m]) , dcnmax );
        }

        //ccc     call cnfilt(ffilt)
        piqsum();

        zccalc(mct);
        zcnorm(mct);

        //        write(*,*) itercn, dcnmax
        if(dcnmax <= 5.0e-5) break;//go to 101
        //  100 continue
    }
    //      write(*,*)
    //      write(*,*) 'mapgen: geometric constraints not fully converged'

    // 101 continue

    //--- return new airfoil coordinates
    n = nc;
    for(int i=1; i<=nc;i++){
        x[i] = real(zc[i]);
        y[i] = imag(zc[i]);
    }

    //      end ! mapgen
}


void XFoil::cgauss(int nn, std::complex <double> z[IMX4+1][IMX4+1], std::complex <double> r[IMX4+1]){
    //*******************************************
    //     solves general complex linear systems.
    //*******************************************
    //      complex z(nsiz,nsiz), r(nsiz,nrhs)
    // nrhs = 1 // techwinder : one right hand side is enough !
    std::complex<double> pivot=0, temp=0, ztmp=0;
    int np1=0;
    int nx=0;
    int l=0,k=0, n=0, np=0;

    for(np=1;np<= nn-1;np++)
    {
        np1 = np+1;

        //----- find max pivot index nx
        nx = np;
        for(n=np1; n<= nn;n++){
            if(std::abs(z[n][np])-std::abs(z[nx][np])>0)  nx = n;
        }

        pivot = std::complex<double>(1.0,0.0)/z[nx][np];

        //----- switch pivots
        z[nx][np] = z[np][np];

        //----- switch rows & normalize pivot row
        for (l=np1; l<= nn;l++){
            temp = z[nx][l]*pivot;
            z[nx][l] = z[np][l];
            z[np][l] = temp;
        }

        //        for (l=1; l<= nrhs; l++){
        //            temp = r[nx][l]*pivot;
        //            r[nx][l] = r[np][l];
        //            r[np][l] = temp;
        //        }
        temp = r[nx]*pivot;
        r[nx] = r[np];
        r[np] = temp;

        //----- forward eliminate everything
        for(k=np1; k<= nn;k++){
            ztmp = z[k][np];

            //         if(ztmp.eq.0.0) go to 15

            for(l=np1; l<= nn;l++){
                z[k][l] = z[k][l] - ztmp*z[np][l];
            }
            //            for(l=1; l<= nrhs; l++){
            //                r[k][l] = r[k][l] - ztmp*r[np][l];
            //            }
            r[k] = r[k] - ztmp*r[np];

        }

    }

    //--- solve for last row
    //    for(l=1; l<= nrhs;l++){
    //        r[nn][l] = r[nn][l]/z[nn][nn];
    //    }
    r[nn] = r[nn]/z[nn][nn];

    //--- back substitute everything
    for(np=nn-1;np>= 1;np--){
        np1 = np+1;
        //        for(l=1; l<= nrhs; l++){
        //            for(k=np1; k<= nn;k++){
        //                r[np][l] = r[np][l] - z[np][k]*r[k][l];
        //            }
        //        }

        for(k=np1; k<= nn;k++){
            r[np] = r[np] - z[np][k]*r[k];
        }

    }
}

double XFoil::qcomp(double g){
    return g*(1.0-tklam) / (1.0 - tklam*(g/qinf)*(g/qinf));
}


double XFoil::qincom(double qc, double qinf, double tklam)
{
    //-------------------------------------
    //     sets incompressible speed from
    //     karman-tsien compressible speed
    //-------------------------------------

    if(tklam<1.0e-4 || fabs(qc)<1.0e-4)
        //----- for nearly incompressible case or very small speed, use asymptotic
        //      expansion of singular quadratic formula to avoid numerical problems
        return qc/(1.0 - tklam);
    else{
        //----- use quadratic formula for typical case
        double tmp = 0.5*(1.0 - tklam)*qinf/(qc*tklam);
        return qinf*tmp*(sqrt(1.0 + 1.0/(tklam*tmp*tmp)) - 1.0);
    }
}


void XFoil::cncalc(double qc[], bool lsymm)
{
    //----------------------------------------------------------
    //     calculates the complex fourier coefficients cn of
    //     the real part of the harmonic function p(w) + iq(w)
    //     which is set from either the current surface speed
    //     function
    //                                                  e
    //                   2 cos(w/2 - alpha) [2 sin(w/2)]
    //       p(w) =  ln  -------------------------------
    //                               q(w)
    //
    //
    //     or the geometry function
    //
    //                                         e
    //                       z'(w) [2 sin(w/2)]
    //          p(w) =   ln  ------------------
    //                           2 sin(w/2)
    //
    //     depending on whether the speed q(w) or the
    //     geometry z(w) is specified for that particular
    //     value of w.
    //     (z(w) option is currently implemented separately in scinit)
    //
    //     by fourier-transforming p(w) into a sequence
    //     of fourier coefficients cn, its complex conjugate
    //     function q(w) is automatically determined by an
    //     inverse transformation in piqsum.  the overall
    //     p(w) + iq(w) then uniquely defines the overall
    //     airfoil geometry, which is calculated in zccalc.
    //
    //     if lsymm=t, then the real(cn) change from current
    //     cn values is doubled, and imag(cn) is zeroed out.
    //----------------------------------------------------------
    //      real qc(nc)

    double qcw[ICX+1];
    int ic=0,m=0;
    double wcle=0, alfcir=0;
    double cosw=0, sinw=0, sinwe=0, pfun=0, cnr=0;

    //      common /work/ cnsav(0:imx)

    //c      real wcj(2)

    if(nc > ICX)
    {
        writeString("CNCALC: array overflow.");
        return;
    }

    //c//---- assume q(w) segment is entire airfoil
    //      wcj(1) = wc(1)
    //      wcj(2) = wc(nc)
    //cc
    //      if(liqset) then
    //c//----- set w at q(w) segment endpoints
    //       wcj(1) = wc[iq1]
    //       wcj(2) = wc[iq2]
    //      endif

    //---- spline q(w)
    splind(qc,qcw,wc,nc,-999.0,-999.0);

    //---- get approximate w value at stagnation point
    for (ic=2; ic<=nc; ic++){
        if(qc[ic]<0.0) {
            wcle = wc[ic];
            break;
        }
    }


    //---- set exact numerical w value at stagnation point from splined q(w)
    sinvrt(wcle,0.0,qc,qcw,wc,nc);

    //---- set corresponding circle plane alpha
    alfcir = 0.5*(wcle - PI);

    //---- calculate real part of harmonic function  p(w) + iq(w)
    for(ic=2;ic<= nc-1;ic++){

        cosw = 2.0*cos(0.5*wc[ic] - alfcir);
        sinw = 2.0*sin(0.5*wc[ic]);
        sinwe = pow(sinw,agte);

        //c        if(wc[ic].ge.wcj(1) .and. wc[ic].le.wcj(2)) then

        //------- set p(w) from q(w)
        if(fabs(cosw)<1.0e-4)
            //-------- use asymptotic form near stagnation point
            pfun = fabs( sinwe/qcw[ic] );
        else
            //-------- use actual expression
            pfun = fabs( cosw*sinwe/qc[ic] );


        //c        else
        //cc
        ////------- set p(w) from old geometry derivative z'(w)
        //c         pfun = abs( zcoldw[ic]*sinwe/sinw )
        //cc
        //c        endif

        piq[ic] = std::complex<double>( log(pfun) , 0.0 );

    }

    //---- extrapolate p(w) to te
    piq[1]  = 3.0*piq[2]    - 3.0*piq[3]    + piq[4];
    piq[nc] = 3.0*piq[nc-1] - 3.0*piq[nc-2] + piq[nc-3];

    for(m=0; m<= mc;m++)   cnsav[m] = cn[m];


    //---- fourier-transform p(w) to get new cn coefficients
    ftp();
    cn[0] = std::complex<double>(0.0 , qimold);

    if(lsymm) {
        for( m=1; m<= mc; m++){
            cnr = 2.0*real(cn[m]) - real(cnsav[m]);
            cn[m] = std::complex<double>( cnr , 0.0 );
        }
    }

    piqsum();


}


void XFoil::qspcir()
{
    //----------------------------------------------------
    //     sets qspec arrays for all design alphas or cls
    //----------------------------------------------------

    for(int kqsp=1; kqsp<= nqsp;kqsp++)
    {
        qccalc(iacqsp,&alqsp[kqsp],&clqsp[kqsp],&cmqsp[kqsp],minf,qinf,
               &nsp,w1,w2,w5,qspec[kqsp]);
        splqsp(kqsp);
    }
    lqspec = true;
}


void XFoil::splqsp(int kqsp)
{
    //------------------------------------------------------
    //     splines qspec(s).  The end intervals are treated
    //     specially to avoid Gibbs-type problems from
    //     blindly splining to the stagnation point.
    //------------------------------------------------------
    int i=0;

    //---- usual spline with natural end bcs
    //    splind(qspec[kqsp][2],qspecp[kqsp][2],sspec[2], nsp-2, -999.0,-999.0);
    splind(qspec[kqsp]+2-1,qspecp[kqsp]+2-1,sspec+2-1, nsp-2, -999.0,-999.0);


    //c//---- pseudo-monotonic spline with simple secant slope calculation
    //c      call splina(qspec(2,kqsp),qspecp(2,kqsp),sspec(2),nsp-2)

    //---- end intervals are splined separately with natural bcs at
    //     the trailing edge and matching slopes at the interior points

    i = 1;
    splind(qspec[kqsp]+i-1,qspecp[kqsp]+i-1,sspec+i-1,2,-999.0,qspecp[kqsp][i+1]);

    i = nsp-1;
    splind(qspec[kqsp]+i-1,qspecp[kqsp]+i-1,sspec+i-1,2, qspecp[kqsp][i],-999.0);

}


void XFoil::ExecMDES()
{
    //----- calculate new mapping coefficients
    double clq=0;
    int kqsp = 1;
    int nqsp = 1; // for the present time
    if(!lqspec) {
        cncalc(qspec[kqsp],lqsym);

        //----- set new qspec(s) for all alphas or cls
        qspcir();
    }

    for(kqsp=1; kqsp<= nqsp; kqsp++){
        //cc         call qspplt(iqmod1,iqmod2,kqsp,ntqspl)
        //cc         if(lqsym) call qspplt(nsp-iqmod2+1,nsp-iqmod1+1,kqsp,ntqspl)

        qspint(kqsp, clq);

        //------- set new cl only if alpha is prescribed
        if(iacqsp==1) clqsp[kqsp] = clq;

    }

    lcnpl = false;

    mapgen(nb,xb,yb);

    //----- spline new buffer airfoil
    scalc(xb,yb,sb,nb);
    splind(xb,xbp,sb,nb,-999.0,-999.0);
    splind(yb,ybp,sb,nb,-999.0,-999.0);


    geopar(xb,xbp,yb,ybp,sb,nb, w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);

}


/** ---------------------------------------------
 *      integrates circle-plane array surface
 *      pressures to get cl and cm
 * ---------------------------------------------- */
void XFoil::qspint(int kqsp, double &clq)
{
    //      include 'circle.inc'
    //      dimension qspec(nc)
    //      real minf
    int i=0, ip;
    double sa=0,ca=0, cpq1=0, cpq2=0, beta=0, bfac=0, cqinc=0;
    double dx=0,dy=0,du=0,ax=0,ay=0,aq=0;

    sa = sin(alqsp[kqsp]);
    ca = cos(alqsp[kqsp]);

    beta = sqrt(1.0 - minf*minf);
    bfac = 0.5*minf*minf / (1.0 + beta);

    clq = 0.0;
    cmqsp[kqsp] = 0.0;

    i = 1;
    cqinc = 1.0 - (qspec[kqsp][i]/qinf)*(qspec[kqsp][i]/qinf);
    cpq1 = cqinc / (beta + bfac*cqinc);

    for(i=1; i<=nc; i++){
        ip = i+1;
        if(i==nc) ip = 1;

        cqinc = 1.0 - (qspec[kqsp][ip]/qinf)* (qspec[kqsp][ip]/qinf);
        cpq2 = cqinc / (beta + bfac*cqinc);

        dx = (xcold[ip] - xcold[i])*ca + (ycold[ip] - ycold[i])*sa;
        dy = (ycold[ip] - ycold[i])*ca - (xcold[ip] - xcold[i])*sa;
        du = cpq2 - cpq1;

        ax = 0.5*(xcold[ip]+xcold[i])*ca + 0.5*(ycold[ip]+ycold[i])*sa;
        ay = 0.5*(ycold[ip]+ycold[i])*ca - 0.5*(xcold[ip]+xcold[i])*sa;
        aq = 0.5*(cpq2 + cpq1);

        clq = clq + dx* aq;
        cmqsp[kqsp] = cmqsp[kqsp] - dx*(aq*(ax-0.25) + du*dx/12.0) - dy*(aq* ay + du*dy/12.0);

        cpq1 = cpq2;
    }
}


/** --------------------------------------------
 *     smooths qspec(s) inside target segment
 * --------------------------------------------*/
void XFoil::smooq(int kq1,int kq2,int kqsp)
{
    int i=0;
    double smool=0, smoosq=0, ds=0, dsm=0, dsp=0, dso=0, qspp1=0, qspp2=0;

    //------ mixed inverse: use arc length coordinate
    for (i=1; i<=nsp;i++){
        w8[i] = sspec[i];
    }

    if(kq2-kq1 < 2) {
        //       write(*,*) 'segment is too short.  no smoothing possible.'
        return;
    }

    //---- set smoothing length ( ~ distance over which data is smeared )
    smool = 0.002*(w8[nsp] - w8[1]);

    //---- set up tri-diagonal system for smoothed qspec
    smoosq = smool*smool;
    for (i=kq1+1; i<= kq2-1; i++){
        dsm = w8[i  ] - w8[i-1];
        dsp = w8[i+1] - w8[i  ];
        dso = 0.5*(w8[i+1] - w8[i-1]);

        w1[i] =  smoosq * (          - 1.0/dsm) /dso;
        w2[i] =  smoosq * ( 1.0/dsp + 1.0/dsm) /dso  +  1.0;
        w3[i] =  smoosq * (-1.0/dsp           ) /dso;
    }

    //---- set fixed-qspec end conditions
    w2[kq1] = 1.0;
    w3[kq1] = 0.0;

    w1[kq2] = 0.0;
    w2[kq2] = 1.0;

    if(lqslop) {
        //----- also enforce slope matching at endpoints
        i = kq1 + 1;
        dsm = w8[i  ] - w8[i-1];
        dsp = w8[i+1] - w8[i  ];
        ds  = w8[i+1] - w8[i-1];
        w1[i] = -1.0/dsm - (dsm/ds)/dsm;
        w2[i] =  1.0/dsm + (dsm/ds)/dsm + (dsm/ds)/dsp;
        w3[i] =                          - (dsm/ds)/dsp;
        qspp1 = w1[i]*qspec[kqsp][i-1] + w2[i]*qspec[kqsp][i] + w3[i]*qspec[kqsp][i+1];

        i = kq2 - 1;
        dsm = w8[i  ] - w8[i-1];
        dsp = w8[i+1] - w8[i  ];
        ds  = w8[i+1] - w8[i-1];
        w1[i] =                            (dsp/ds)/dsm;
        w2[i] = -1.0/dsp - (dsp/ds)/dsp - (dsp/ds)/dsm;
        w3[i] =  1.0/dsp + (dsp/ds)/dsp;
        qspp2 = w1[i]*qspec[kqsp][i-1] + w2[i]*qspec[kqsp][i] + w3[i]*qspec[kqsp][i+1];

        qspec[kqsp][kq1+1] = qspp1;
        qspec[kqsp][kq2-1] = qspp2;
    }

    //---- solve for smoothed qspec array
    //      trisol(w2[kq1],w1[kq1],w3[kq1],qspec[kqsp][kq1],(kq2-kq1+1));
    trisol(w2+kq1-1,w1+kq1-1,w3+kq1-1,qspec[kqsp]+kq1-1,(kq2-kq1+1));

}


void XFoil::HanningFilter(double cfilt, std::string &)
{
    //----- apply modified hanning filter to cn coefficients
    double clq=0;

    cnfilt(cfilt);
    piqsum();
    qspcir();

    //      write(*,1200) algam/dtor,clgam,cmgam
//    std::string str0 = std::string("  current:\n     alpha=%1\n     Cl=%2\n     Cm=%3").arg(algam/dtor, 9, 'f',4).arg(clgam, 11, 'f', 6).arg(cmgam, 11, 'f', 6);
//    ts << str0<<"\n";
    for(int kqsp=1; kqsp<= nqsp; kqsp++)
    {
//        qspint(alqsp[kqsp],qspec[kqsp][1],qinf,minf,clq,cmqsp[kqsp]);
        qspint(kqsp, clq);

        //------- set new cl only if alpha is prescribed
        if(iacqsp == 1) clqsp[kqsp] = clq;

        //         write(*,1210) kqsp,alqsp(kqsp)/dtor,clqsp(kqsp),cmqsp(kqsp)
//        std::string str1 = std::string("  QSpec:\n     alpha=%2\n     Cl=%3\n     Cm=%4").arg(alqsp[kqsp]/dtor, 9, 'f',4).arg(clqsp[kqsp], 11, 'f', 6).arg(cmqsp[kqsp], 11, 'f', 6);
//        ts << str1<<"\n";
    }
    lqspec = true;

}


void XFoil::cnfilt(double ffilt)
{
    //-------------------------------------
    //     filters out upper harmonics
    //     with modified hanning filter.
    //-------------------------------------

    double cwt=0, cwtx=0;
    double freq=0;

    if(ffilt<=0.00001) return;

    for(int m=0; m<= mc; m++)
    {
        freq = double(m)/double(mc);
        cwt = 0.5*(1.0 + cos(PI*freq));
        cwtx = cwt;
        if(ffilt>0.0) cwtx = pow(cwt,ffilt);
        cn[m] = std::complex<double>(real(cn[m]) * cwtx, imag(cn[m]) * cwtx);
    }
}


void XFoil::pert_init(int kqsp)
{
    double dx=0, dy=0, qimoff=0;
    //---- calculate mapping coefficients for initial airfoil shape
    //      cncalc(qspec,false);
    cncalc(qspec[kqsp]+1-1,false);

    //---- preset rotation offset of airfoil so that initial angle is close
    //-    to the old airfoil's angle
    dx = xcold[2] - xcold[1];
    dy = ycold[2] - ycold[1];
    qim0 = atan2(dx, -dy)  +  0.5*PI*(1.0+agte);
    qimoff = qim0 - imag(cn[0]);
    cn[0] = cn[0] + std::complex<double>(0.0, qimoff);

    //      write(*,*)
    //      write(*,*) 'current mapping coefficients...'
    //      write(*,*) '      n    re(cn)      im(cn)'
    //cc   do m = 1, nc
    //      do m = 1, min(nc,32)
    //        write(*,1010) m, real(cn(m)), imag(cn(m))
    // 1010   format(4x,i4, 212.6)
    //      enddo

    /* 10   write(*,1050)
 1050 format(/4x,'enter  n, delta(cnr), delta(cni):  ', $)
      read(*,*,err=10) m, dcnr, dcni
      if(m.<=0) {
          go to 10;
      }
      else if(m>nc) {
       write(*,*) 'max number of modes is', nc
       go to 10;
      }*/
    //      cn[m] = cn[m] + std::complex<double>(dcnr, dcni);

}


void XFoil::pert_process(int kqsp)
{
    (void) kqsp;
    int m=0, ncn=0;
    //    double dx,dy,qimoff;
    std::complex<double> qq[IMX/4+1][IMX/4+1],dcn[IMX/4+1];

    //--------------------------------------------------------
    //     calculates the perturbed geometry resulting from
    //     one cn mapping coefficient being perturbed by user.
    //--------------------------------------------------------
    //      include 'circle.inc'
    //      dimension qspec(icx)

    //---- inverse-transform and calculate geometry
    //cc   call cnfilt(ffilt)
    piqsum();
    zccalc(mct);

    //---- normalize chord and set exact previous alpha
    zcnorm(mct);
    //---- put back rotation offset so speed routine qccalc gets the right alpha
    //cc      cn(0) = cn(0) - cmplx(0.0 , qimoff )

    //---- enforce lighthill's first constraint
    cn[0] = std::complex<double>(0.0, imag(cn[0]) );
    //---- number of free coefficients
    ncn = 1;

    //---- newton iteration loop for modified cn's
    bool bConv = false;
    for(int itercn=1; itercn<= 10; itercn++)
    {

        //------ fix te gap
        m=1;
        dcn[m] = zc[1] - zc[nc]  -  dzte;
        for (int l=1; l<= ncn; l++){
            qq[m][l] = zc_cn[1][l] - zc_cn[nc][l];
        }

        //        cgauss(IMX/4,ncn,qq,dcn,1);
        cgauss(ncn,qq,dcn);

        double dcnmax = 0.0;
        for (m=1; m<= ncn; m++){
            cn[m] = cn[m] - dcn[m];
            dcnmax = std::max(std::abs(dcn[m]), dcnmax );
        }

        //cc     call cnfilt(ffilt)
        piqsum();

        zccalc(mct);
        zcnorm(mct);

        //        write(*,*) itercn, dcnmax
        if(dcnmax<=5.0e-5){
            bConv = true;
            break;
        }
    }
    if(!bConv)
    {
        writeString("TE gap,chord did not converge");
        return;
    }

    qspcir();
}


void XFoil::InitMDES()
{
    //    double xsp[IBX], ysp[IBX][IPX], yspd[IBX][IPX];

    //    int ntqspl;
    //      save comold, argold???

    lrecalc = false;

    if(n==0) {
        //       write(*,*) '***  no airfoil available  ***'
        return;
    }

    lcnpl = false;
    //    lsym = true;
    ffilt = 0.0;

    //    ntqspl = 1;
    //    if(lqslop) ntqspl = 4;

    //---- see if current qspec, if any, didn't come from mixed-inverse
    if(nsp!=nc1){
        lqspec = false;
        iq1 = 1;
        iq2 = nc1;
    }

    //---- initialize fourier transform arrays if it hasn't been done
    if(!leiw) eiwset(nc1);
    leiw = true;

    //---- if qspec alpha has never been set, set it to current alpha
    if(nqsp == 0) {
        iacqsp = 1;
        alqsp[1] = alfa;
        nqsp = 1;
    }

    if(!lscini) {
        //------ initialize s(w) for current airfoil, generating its cn coefficients
        scinit(n,x,xp,y,yp,s,sle);
        lscini = true;

        //------ set up to initialize qspec to current conditions
        lqspec = false;
    }

    //---- set initial q for current alpha
    algam = alfa;
    mapgam(1,algam,clgam,cmgam);
    //    TRACE("Current Q Operating conditions : %4  %4\n", algam/dtor, clgam);

    if(!lqspec) {
        //------ set cn coefficients from current q
        cncalc(qgamm,false);

        //------ set qspec from cn coefficients
        qspcir();
        //        write(*,1190)
    }
}


bool XFoil::InitQDES()
{
    //    int ntqspl;
    //    int kqtarg;
    double chx(0), chy(0), chsq(0);

    lrecalc = false;

    if(n==0) {
        //       write(*,*) '***  no airfoil available  ***'
        return false;
    }

    //    lsym = true;

    //---- number of sub-intervals for qspec(s) plotting
    //    ntqspl = 1;
    //    if(lqslop) ntqspl = 8;

    //---- make sure a current solution exists
    specal();

    //---- see if current qspec, if any, didn't come from full-inverse
    if(nsp!=n) {
        lqspec = false;
        liqset = false;
    }

    //---- set alpha, etc corresponding to q
    algam = alfa;
    clgam = cl;
    cmgam = cm;

    //---- set "old" speed distribution q, arc length, and x/c,y/c arrays
    chx = xte - xle;
    chy = yte - yle;
    chsq = chx*chx + chy*chy;
    nsp = n;
    for (int i=1; i<=nsp; i++){
        qgamm[i] = gam[i];
        sspec[i] = s[i]/s[n];
        xspoc[i] = ((x[i]-xle)*chx + (y[i]-yle)*chy)/chsq;
        yspoc[i] = ((y[i]-yle)*chx - (x[i]-xle)*chy)/chsq;
    }
    ssple = sle/s[n];

    //    write(*,1150) algam/dtor, clgam
    //    1150 format(/' current q operating condition:'
    //    &       /' alpha = ', f8.3, ' deg.      cl = ', f8.4 / )

    if(!lqspec) {
        //----- initialize qspec to "old" solution and notify user
        nqsp = 1;
        //        kqtarg = 1;
        gamqsp(1);
        //        write(*,1155);
        lqspec = true;
    }

    return true;
}


void XFoil::gamqsp(int kqsp)
{
    //------------------------------------------------
    //     sets qspec(s,k) from current speed q(s).
    //------------------------------------------------

    alqsp[kqsp] = algam;
    clqsp[kqsp] = clgam;
    cmqsp[kqsp] = cmgam;

    for (int i=1; i<=nsp; i++){
        qspec[kqsp][i] = qgamm[i];
    }

    //---- zero out qspec dofs
    qdof0 = 0.0;
    qdof1 = 0.0;
    qdof2 = 0.0;
    qdof3 = 0.0;

    splqsp(kqsp);

    //---- reset target segment endpoints
    if(!liqset) {
        iq1 = 1;
        iq2 = nsp;
    }
}


bool XFoil::mixed(int kqsp)
{
    //-------------------------------------------------
    //     performs a mixed-inverse calculation using
    //     the specified surface speed array qspec.
    //-------------------------------------------------
    int i=0, j=0, iter;
    //    int inmax=0, igmax=0;
    //    double sina=0, cosa=0;
    double bwt=0, fs=0, psi=0, psi_n=0;
    double ag1=0, ag2=0, abis=0, cbis=0, sbis=0;
    double ds1=0, ds2=0, dsmin=0, xbis=0, ybis=0, qbis=0;
    double res=0;
    double dnmax=0, dgmax=0;

    //---- distance of internal control point ahead of sharp te
    //    (fraction of smaller panel length adjacent to te)
    bwt = 0.1;

    //    cosa = cos(alfa);
    //    sina = sin(alfa);
    scalc(x,y,s,n);

    //---- zero-out and set dof shape functions
    for (i=1; i<=n; i++){
        qf0[i] = 0.0;
        qf1[i] = 0.0;
        qf2[i] = 0.0;
        qf3[i] = 0.0;
    }

    //---- set dof shape functions and specified speed
    for(i=iq1; i<= iq2; i++){
        fs = (s[i]-s[iq1]) / (s[iq2]-s[iq1]);
        //ccc        qf0[i] = (1.0-fs)**2
        //ccc        qf1[i] = fs**2
        qf0[i] = 1.0 - fs;
        qf1[i] = fs;
        if(lcpxx) {
            qf2[i] = exp(-5.0*     fs );
            qf3[i] = exp(-5.0*(1.0-fs));
        }
        else{
            qf2[i] = 0.0;
            qf3[i] = 0.0;
        }
        gam[i] = qspec[kqsp][i] + qdof0*qf0[i] + qdof1*qf1[i]
                + qdof2*qf2[i] + qdof3*qf3[i];
    }


    //---- perform newton iterations on the new geometry
    for(iter=1; iter<= niterq; iter++){

        for (i=1; i<=n+5; i++){
            for (j=1; j<=n+5;j++){
                q[i][j] = 0.0;
            }
        }

        //---- calculate normal direction vectors along which the nodes move
        ncalc(x,y,s,n,nx,ny);

        //---- go over all nodes, setting up  psi = psi0  equations
        for(i=1; i<=n; i++)
        {
            psilin(i,x[i],y[i],nx[i],ny[i],psi,psi_n,true,false);

            dzdn[i] = dzdn[i] + psi_n;

            //------ fill columns for specified geometry location
            for(j=1; j<=iq1-1; j++) q[i][j] += + dzdg[j];

            //------ fill columns for specified surface speed location
            for(j=iq1; j<= iq2; j++) q[i][j] += dzdn[j];

            //------ fill columns for specified geometry location
            for(j=iq2+1; j<=n; j++) q[i][j] += dzdg[j];

            //------ set residual
            dq[i] = psio - psi;

            //------ fill global unknown columns
            q[i][n+1] += - 1.0;
            q[i][n+2] += z_qdof0;
            q[i][n+3] += z_qdof1;
            q[i][n+4] += z_qdof2;
            q[i][n+5] += z_qdof3;
        }

        //---- set up kutta condition
        dq[n+1] = -( gam[1] + gam[n] );
        gamlin(n+1,1,1.0);
        gamlin(n+1,n,1.0);

        if(sharp) {
            //----- set zero internal velocity in te corner

            //----- set te bisector angle
            ag1 = atan2(-yp[1],-xp[1]    );
            ag2 = atanc( yp[n], xp[n],ag1);
            abis = 0.5*(ag1+ag2);
            cbis = cos(abis);
            sbis = sin(abis);

            //----- minimum panel length adjacent to te
            ds1 = sqrt( (x[1]-x[2]  )*(x[1]-x[2]  ) + (y[1]-y[2]  )*(y[1]-y[2]  ));
            ds2 = sqrt( (x[n]-x[n-1])*(x[n]-x[n-1]) + (y[n]-y[n-1])*(y[n]-y[n-1]));
            dsmin = std::min( ds1 , ds2 );

            //----- control point on bisector just ahead of te point
            xbis = xte - bwt*dsmin*cbis;
            ybis = yte - bwt*dsmin*sbis;
            //ccc       write(*,*) xbis, ybis

            //----- set velocity component along bisector line
            psilin(0,xbis,ybis,-sbis,cbis,psi,qbis,false,true);

            //c//--- res = dqdgj*gamj + dqdmj*massj + qinf*(cosa*cbis + sina*sbis)
            res = qbis;

            for(j=1; j<=n+5; j++){
                q[n][j] = 0.0;
            }

            //----- dres/dgamj
            for(j=1; j<=n; j++){
                gamlin(n,j, dqdg[j]);
                q[n][j] = dqdg[j];
            }

            //----- dres/dpsio
            q[n][n+1] = 0.0;

            //----- -dres/duinf
            dq[n] = -res;
        }

        //---- pinned iq1 point condition
        q[n+2][iq1] = 1.0;
        dq[n+2] = 0.0;

        //---- pinned iq2 point condition
        q[n+3][iq2] = 1.0;
        dq[n+3] = 0.0;

        if(iq1>1 && lcpxx) {
            //----- speed regularity iq1 condition
            res = gam[iq1-1]      - 2.0*  gam[iq1]      +   gam[iq1+1]
                    - (qspec[kqsp][iq1-1] - 2.0*qspec[kqsp][iq1] + qspec[kqsp][iq1+1] );
            gamlin(n+4,iq1-1, 1.0);
            gamlin(n+4,iq1  ,-2.0);
            gamlin(n+4,iq1+1, 1.0);
            dq[n+4] = -res;
        }
        else{
            //----- zero dof condition
            q[n+4][n+4] = 1.0;
            dq[n+4] = -qdof2;
        }

        if(iq2<n && lcpxx) {
            //----- speed regularity iq2 condition
            res = gam[iq2-1]      - 2.0*  gam[iq2]      +   gam[iq2+1]
                    - (qspec[kqsp][iq2-1] - 2.0*qspec[kqsp][iq2] + qspec[kqsp][iq2+1] );
            gamlin(n+5,iq2-1, 1.0);
            gamlin(n+5,iq2  ,-2.0);
            gamlin(n+5,iq2+1, 1.0);
            dq[n+5] = -res;
        }
        else{
            //----- zero dof condition
            q[n+5][n+5] = 1.0;
            dq[n+5] = -qdof3;
        }

        Gauss(n+5,q,dq);

        //        inmax = 0;
        //        igmax = 0;
        dnmax = 0.0;
        dgmax = 0.0;

        //---- update surface speed gam before target segment
        for(i=1; i<= iq1-1; i++){
            gam[i] += dq[i];
            if(fabs(dq[i]) > fabs(dgmax)) {
                dgmax = dq[i];
                //                igmax = i;
            }
        }

        //---- update panel nodes inside target segment
        for(i=iq1; i<= iq2; i++){
            x[i] += nx[i]*dq[i];
            y[i] += ny[i]*dq[i];
            if(fabs(dq[i]) > fabs(dnmax)) {
                dnmax = dq[i];
                //                inmax = i;
            }
        }

        //---- update surface speed gam after target segment
        for(i=iq2+1; i<=n; i++){
            gam[i] += dq[i];
            if(fabs(dq[i]) > fabs(dgmax)) {
                dgmax = dq[i];
                //                igmax = i;
            }
        }

        //---- update gloabal variables
        psio  = psio  + dq[n+1];
        qdof0 = qdof0 + dq[n+2];
        qdof1 = qdof1 + dq[n+3];
        qdof2 = qdof2 + dq[n+4];
        qdof3 = qdof3 + dq[n+5];

        //        cosa = cos(alfa);
        //        sina = sin(alfa);
        scalc(x,y,s,n);

        //---- set correct surface speed over target segment including dof contributions
        for(i=iq1; i<= iq2; i++){
            gam[i] = qspec[kqsp][i] + qdof0*qf0[i] + qdof1*qf1[i]
                    + qdof2*qf2[i] + qdof3*qf3[i];
        }

        //---- update everything else
        tecalc();
        clcalc(xcmref,ycmref);
        /*      write(*,2000) dnmax,inmax,dgmax,igmax,cl
        &             ,dq(n+2),dq(n+3)
        &             ,dq(n+4),dq(n+5)
        2000 format(/' dnmax =',e10.3,i4,'   dqmax =',e10.3,i4,'    cl =',f7.4
        &       /' dqf1  =',e10.3,4x,'   dqf2  =',e10.3
        &       /' dqf3  =',e10.3,4x,'   dqf4  =',e10.3)*/
        dnTrace[iter] = fabs(dnmax);
        dgTrace[iter] = fabs(dgmax);
        //        TRACE("%d  dNMax = %.3e  dGMax = %.3e\n",iter, dnmax, dgmax);
        if(fabs(dnmax)<5.0e-5 && fabs(dgmax)<5.0e-4) {
            //       write(*,*)
            //       write(*,*) 'new current airfoil generated'
            //       write(*,*) 'old buffer  airfoil unchanged'
            QMax = iter;
            return true;
        }

    }
    QMax = niterq;
    //    TRACE("Unconverged - shit/Schei�e/merde !\n");
    //    TRACE("not quite converged.  can exec again if necessary.\n");
    return false;
}


void XFoil::gamlin(int i, int j, double coef)
{
    //-------------------------------------------------------------------
    //     adds on jacobian entry for point i due to node speed gam at j.
    //     gam is either a local unknown if outside target segment,
    //     or dependent on global qspec dof's if inside target segment.
    //-------------------------------------------------------------------

    if(j>=iq1 && j<=iq2)
    {
        //----- inside target segment
        q[i][n+2] += coef*qf0[j];
        q[i][n+3] += coef*qf1[j];
        q[i][n+4] += coef*qf2[j];
        q[i][n+6] += coef*qf3[j];
    }
    else{
        //----- outside target segment
        q[i][j] +=  coef;
    }

}


bool XFoil::ExecQDES()
{
    int kqsp=0, i=0;
    //---- check if target segment includes stagnation point
    ist = 0;
    for (i=iq1; i<= iq2-1; i++){
        if(qgamm[i]>=0.0 && qgamm[i+1]<0.0) ist = i;
    }

    if(ist!=0)
    {
        writeString("Target segment cannot include\nstagnation point in mixed-inverse");
        return false;
    }

    kqsp = 1;
    clspec = clqsp[kqsp];
    //ccc      call askr('enter specified cl^',clspec)

    //----- save current coordinates for restoration if requested
    for(i=1; i<=n; i++){
        xb[i]  = x[i];
        yb[i]  = y[i];
        sb[i]  = s[i];
        xbp[i] = xp[i];
        ybp[i] = yp[i];
    }
    nb = n;
    lgsame = true;

    //       write(*,*)
    //       write(*,*) 'current airfoil saved in buffer airfoil'

    //----- execute mixed-inverse calculation
    //       call aski('enter max number of iterations^',niterq)


    bool bRes = mixed(kqsp);
    adeg = alfa/dtor;

    //----- spline new airfoil shape
    scalc(x,y,s,n);
    splind(x,xp,s,n,-999.0,-999.0);
    splind(y,yp,s,n,-999.0,-999.0);
    ncalc(x,y,s,n,nx,ny);
    lefind(sle,x,xp,y,yp,s,n);
    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    chord  = sqrt((0.5*(x[1]+x[n]) - xle)*(0.5*(x[1]+x[n]) - xle)
            +(0.5*(y[1]+y[n]) - yle)*(0.5*(y[1]+y[n]) - yle));
    tecalc();
    apcalc();

    algam = alfa;

    nsp = n;
    for (i=1; i<=n; i++){
        qgamm[i] = gam[i];
        sspec[i] = s[i]/s[n];
    }
    ssple = sle/s[n];

    //----- set inviscid surface speeds and calculate compressible cp
    for(i=1; i<=n; i++){
        qinv[i] = gam[i];
    }
    cpcalc(n,qinv,qinf,minf,cpi);


    //----- influence coefficients & other stuff is no longer valid for new airfoil
    lgamu = false;
    lqinu = false;
    lwake = false;
    lqaij = false;
    ladij = false;
    lwdij = false;
    lipan = false;
    lvconv = false;
    lscini = false;
    //ccc      lblini = false
    lgsame = false;

    return bRes;
}


void XFoil::RestoreQDES()
{
    //    Foil is restored from CXInverse rather than from XFoil
    /*    for (int i=1; i<=n; i++){
        x[i] = xb[i];
        y[i] = yb[i];
    }*/

    scalc(x,y,s,n);
    splind(x,xp,s,n,-999.0,-999.0);
    splind(y,yp,s,n,-999.0,-999.0);
    ncalc(x,y,s,n,nx,ny);
    lefind(sle,x,xp,y,yp,s,n);
    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    chord  = sqrt((0.5*(x[1]+x[n]) - xle)*(0.5*(x[1]+x[n]) - xle)
            +(0.5*(y[1]+y[n]) - yle)*(0.5*(y[1]+y[n]) - yle));
    tecalc();
    apcalc();
    lgamu  = false;
    lqinu  = false;
    lgsame = true;
}


void XFoil::tcset(double cnew, double tnew)
{
    //      dimension rinput(*)
    //------------------------------------------------------
    //     finds buffer airfoil thickness and/or camber,
    //     plots thickness, camber and airfoil,
    //     and scales t and/or c by user input factors
    //------------------------------------------------------

    double cfac=0, tfac=0;
    //--- find the current buffer airfoil camber and thickness
    double xcm[IQX], ycm[IQX], xtk[IQX], ytk[IQX], ycmp[IQX], ytkp[IQX];
    double  txmax=0, tymax=0, cxmax=0, cymax=0;
    int ncm=0, ntk=0;

    getcam(xcm,ycm,ncm,xtk,ytk,ntk, xb,xbp,yb,ybp,sb,nb );
    getmax(xcm,ycm,ycmp,ncm,cxmax,cymax);
    getmax(xtk,ytk,ytkp,ntk,txmax,tymax);

    //      write(*,1000) 2.0*tymax,txmax, cymax,cxmax
    // 1000 format(/' max thickness = ',f8.4,'  at x = ',f7.3,
    //     &       /' max camber    = ',f8.4,'  at x = ',f7.3/)

    /*      if    (ninput .ge. 2) then
        tnew = rinput(1)
        cnew = rinput(2)
      elseif(ninput .ge. 1) then
        tnew = rinput(1)
        if(lgsym) then
         write(*,*) 'symmetry enforced:  maintaining zero camber.'
        else
         cnew = 999
         call askr('enter new max  camber   <ret> to skip^',cnew)
        endif
      else
        tnew = 999
        call  askr('enter new max thickness <ret> to skip^',tnew)
        if(lgsym) then
         write(*,*) 'symmetry enforced:  maintaining zero camber.'
        else
         cnew = 999
         call askr('enter new max  camber   <ret> to skip^',cnew)
        endif
      endif*/

    //      cfac = 1.0;
    //      tfac = 1.0;
    //      if(cymax!=0.0 && cnew!=999.0) cfac = cnew / (    cymax)
    //      if(tymax!=0.0 && tnew!=999.0) tfac = tnew / (2.0*tymax)
    cfac = cnew / (     cymax);
    tfac = tnew / (2.0*tymax);
    //---- sanity checks on scaling factors
    if(fabs(tfac) > 100.0 || fabs(cfac) > 100.0) {
        //        write(*,1100) tfac, cfac
        // 1100   format(/' questionable input...'
        //     &         /' implied scaling factors are:', f13.2,' x thickness'
        //     &         /'                             ', f13.2,' x camber   ')
        //        call askl('apply scaling factors?^',ok)
        //        if(.not.ok) then
        //          write(*,*) 'no action taken'
        //          return
        //        endif
    }

    //ccc      if (tfac.lt.0.0) tfac = 0.0
    thkcam(tfac,cfac);
    getcam(xcm,ycm,ncm,xtk,ytk,ntk,xb,xbp,yb,ybp,sb,nb);
}


void XFoil::thkcam(double tfac, double cfac){
    //---------------------------------------------------
    //     changes buffer airfoil thickness and camber
    //---------------------------------------------------
    int i=0;
    double dxc=0, dyc=0,sbopp=0, xbopp=0, ybopp=0, xcavg=0, ycavg=0, xcdel=0, ycdel=0;
    lefind(sble,xb,xbp,yb,ybp,sb,nb);

    //---this fails miserably with sharp le foils, tsk,tsk,tsk hhy 4/24/01
    //---- set baseline vector normal to surface at le point
    //      dxc = -deval(sble,yb,ybp,sb,nb)
    //      dyc =  deval(sble,xb,xbp,sb,nb)
    //      dsc = sqrt(dxc**2 + dyc**2)
    //      dxc = dxc/dsc
    //      dyc = dyc/dsc

    //---rational alternative 4/24/01 hhy
    xle = seval(sble,xb,xbp,sb,nb);
    yle = seval(sble,yb,ybp,sb,nb);
    xte = 0.5*(xb[1]+xb[nb]);
    yte = 0.5*(yb[1]+yb[nb]);
    chord = sqrt((xte-xle)*(xte-xle) + (yte-yle)*(yte-yle));
    //---- set unit chord-line vector
    dxc = (xte-xle) / chord;
    dyc = (yte-yle) / chord;

    //---- go over each point, changing the y-thickness appropriately
    for(i=1; i<=nb; i++){
        //------ coordinates of point on the opposite side with the same x value
        sopps(sbopp, sb[i],xb,xbp,yb,ybp,sb,nb,sble);
        xbopp = seval(sbopp,xb,xbp,sb,nb);
        ybopp = seval(sbopp,yb,ybp,sb,nb);

        //------ set new y coordinate by changing camber & thickness appropriately
        xcavg =        ( 0.5*(xb[i]+xbopp)*dxc + 0.5*(yb[i]+ybopp)*dyc );
        ycavg = cfac * ( 0.5*(yb[i]+ybopp)*dxc - 0.5*(xb[i]+xbopp)*dyc );

        xcdel =        ( 0.5*(xb[i]-xbopp)*dxc + 0.5*(yb[i]-ybopp)*dyc );
        ycdel = tfac * ( 0.5*(yb[i]-ybopp)*dxc - 0.5*(xb[i]-xbopp)*dyc );

        w1[i] = (xcavg+xcdel)*dxc - (ycavg+ycdel)*dyc;
        w2[i] = (ycavg+ycdel)*dxc + (xcavg+xcdel)*dyc;
    }

    for (i=1; i<=nb; i++){
        xb[i] = w1[i];
        yb[i] = w2[i];
    }

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb, w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);

}



void XFoil::inter(double x0[], double xp0[], double y0[], double yp0[], double s0[],int n0,double sle0,
                  double x1[], double xp1[], double y1[], double yp1[], double s1[],int n1,double sle1,
                  double x[], double y[], int n, double frac){
    //     .....................................................................
    //
    //     interpolates two source airfoil shapes into an "intermediate" shape.
    //
    //     procedure:
    //        The interpolated x coordinate at a given normalized spline
    //        parameter value is a weighted average of the two source
    //        x coordinates at the same normalized spline parameter value.
    //        ditto for the y coordinates. The normalized spline parameter
    //        runs from 0 at the leading edge to 1 at the trailing edge on
    //        each surface.
    //     .....................................................................

    /*    real x0(n0),y0(n0),xp0(n0),yp0(n0),s0(n0)
    real x1(n1),y1(n1),xp1(n1),yp1(n1),s1(n1)
    real x(n),y(n)*/
    double f0=0, f1=0, tops0=0, tops1=0, bots0=0, bots1=0;
    double sn=0, st0=0, st1=0;
    //---- number of points in interpolated airfoil is the same as in airfoil 0
    n = n0;

    //---- interpolation weighting fractions
    f0 = 1.0 - frac;
    f1 = frac;

    //---- top side spline parameter increments
    tops0 = s0[1] - sle0;
    tops1 = s1[1] - sle1;

    //---- bottom side spline parameter increments
    bots0 = s0[n0] - sle0;
    bots1 = s1[n1] - sle1;

    for (int i=1; i<=n; i++){

        //------ normalized spline parameter is taken from airfoil 0 value
        if(s0[i]< sle0) sn = (s0[i] - sle0) / tops0   ;// top side
        else sn = (s0[i] - sle0) / bots0   ;// bottom side

        //------ set actual spline parameters
        st0 = s0[i];
        if(st0< sle0) st1 = sle1 + tops1 * sn;
        //        if(st0>=sle0) st1 = sle1 + bots1 * sn;
        else st1 = sle1 + bots1 * sn;

        //------ set interpolated x,y coordinates
        x[i] = f0*seval(st0,x0,xp0,s0,n0) + f1*seval(st1,x1,xp1,s1,n1);
        y[i] = f0*seval(st0,y0,yp0,s0,n0) + f1*seval(st1,y1,yp1,s1,n1);

    }
}



void XFoil::interpolate(double xf1[], double yf1[], int n1,
                        double xf2[], double yf2[], int n2,
                        double mixt)
{
    int i=0;
    double x1[IBX], y1[IBX], x2[IBX], y2[IBX];
    double xp1[IBX], yp1[IBX], xp2[IBX], yp2[IBX];
    double s1[IBX], s2[IBX];
    double sleint1=0, sleint2=0;

    for (i=0; i<n1; i++)
    {
        x1[i+1] = xf1[i];
        y1[i+1] = yf1[i];
    }
    for (i=0; i<n2; i++)
    {
        x2[i+1] = xf2[i];
        y2[i+1] = yf2[i];
    }

    scalc(x1,y1,s1,n1);
    segspld(x1,xp1,s1,n1, -999.0, -999.0);
    segspld(y1,yp1,s1,n1, -999.0, -999.0);
    lefind(sleint1, x1, xp1, y1, yp1, s1, n1);

    scalc(x2,y2,s2,n2);
    segspld(x2,xp2,s2,n2, -999.0, -999.0);
    segspld(y2,yp2,s2,n2, -999.0, -999.0);
    lefind(sleint2, x2, xp2, y2, yp2, s2, n2);

    inter(x1, xp1, y1, yp1, s1, n1, sleint1,
          x2, xp2, y2, yp2, s2, n2, sleint2,
          xb,yb,nb,mixt);

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);

}


double XFoil::DeRotate()
{
    lefind(sble,xb,xbp,yb,ybp,sb,nb);
    xle = seval(sble,xb,xbp,sb,nb);
    yle = seval(sble,yb,ybp,sb,nb);
    xte = 0.5*(xb[1] + xb[nb]);
    yte = 0.5*(yb[1] + yb[nb]);

    arad = atan2(yte-yle,xte-xle);
    //    call rotate(xb,yb,nb,arad);

    double sa = sin(arad);
    double ca = cos(arad);
    double xoff = 0.0;
    double yoff = 0.0;
    double xt, yt;
    for (int i=1; i<=n; i++)
    {
        xt = xb[i];
        yt = yb[i];
        xb[i] = ca*xt + sa*yt + xoff;
        yb[i] = ca*yt - sa*xt + yoff;
    }


    //    write(*,1080) arad / dtor
    // 1080  format(/'rotating buffer airfoil by ',f8.3,' deg.')

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);

    return arad*180.0/PI;

}


/** -------------------------------------------------
 *      used to set buffer airfoil trailing edge gap
 * -------------------------------------------------- */
void XFoil::tgap(double gapnew, double blend)
{
    double xble=0, yble=0, xbte=0, ybte=0, chbsq=0, xoc=0, tfac=0;
    double dxn=0, dyn=0, dxu=0, dyu=0;
    double gap=0, dgap=0, doc=0;
    double arg=0;

    lefind(sble,xb,xbp,yb,ybp,sb,nb);
    xble = seval(sble,xb,xbp,sb,nb);
    yble = seval(sble,yb,ybp,sb,nb);
    xbte = 0.5*(xb[1]+xb[nb]);
    ybte = 0.5*(yb[1]+yb[nb]);
    chbsq = (xbte-xble)*(xbte-xble) + (ybte-yble)*(ybte-yble);

    dxn = xb[1] - xb[nb];
    dyn = yb[1] - yb[nb];
    gap = sqrt(dxn*dxn + dyn*dyn);

    //---- components of unit vector parallel to te gap
    if(gap>0.0) {
        dxu = dxn / gap;
        dyu = dyn / gap;
    }
    else{
        dxu = -.5*(ybp[nb] - ybp[1]);
        dyu = 0.5*(xbp[nb] - xbp[1]);
    }

    doc = std::min( std::max( blend , 0.0 ) , 1.0 );
    dgap = gapnew - gap;

    //---- go over each point, changing the y-thickness appropriately
    for (int i=1; i<=nb; i++){

        //------ chord-based x/c
        xoc = (  (xb[i]-xble)*(xbte-xble)  + (yb[i]-yble)*(ybte-yble) ) / chbsq;

        //------ thickness factor tails off exponentially away from trailing edge
        if(doc == 0.0) {
            tfac = 0.0;
            if(i==1 || i==nb) tfac = 1.0;
        }
        else{
            arg = std::min((1.0-xoc)*(1.0/doc-1.0), 15.0 );
            tfac = exp(-arg);
        }

        if(sb[i]<=sble) {
            xb[i] = xb[i] + 0.5*dgap*xoc*tfac*dxu;
            yb[i] = yb[i] + 0.5*dgap*xoc*tfac*dyu;
        }
        else{
            xb[i] = xb[i] - 0.5*dgap*xoc*tfac*dxu;
            yb[i] = yb[i] - 0.5*dgap*xoc*tfac*dyu;
        }
    }

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);


    //    lgeopl = false;
    lgsame = false;

}


/** ------------------------------------------------
 *      Changes buffer airfoil leading edge radius.
 * ------------------------------------------------- */
void XFoil::lerad(double rfac, double blend)
{
    int i=0;
    double doc=0, cvmax=0, cv=0;
    //    double radius;

    doc = std::max( blend , 0.001 );

    lerscl(xb,xbp,yb,ybp,sb,nb, doc,rfac, w1,w2);

    for (i=1; i<=nb; i++){
        xb[i] = w1[i];
        yb[i] = w2[i];
    }

    //---- spline new coordinates
    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,radble,angbte,
           ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt);

    //---- find max curvature
    cvmax = 0.0;
    for(i=int(nb/4); i<=(3*nb)/4; i++)
    {
        cv = curv(sb[i],xb,xbp,yb,ybp,sb,nb);
        cvmax = std::max(fabs(cv) , cvmax );
    }

    //    radius = 1.0/cvmax;

    /*      write(*,1000) radius
    1000 format(/' new le radius = ',f7.5)*/

    //      lgeopl = .0alse.
    lgsame = false;

}


/** ---------------------------------------------------------
 *      Adjusts airfoil to scale LE radius by factor rfac.
 *      Blending of new shape is done with decay length doc.
 * --------------------------------------------------------- */
void XFoil::lerscl(double *x, double *xp, double* y, double *yp,
                   double *s, int n, double doc, double rfac, double *xnew,double *ynew)
{
    double dxc=0, dyc=0, srfac=0, xbar=0, ybar=0, sopp=0, xopp=0, yopp=0, ybarop=0, xoc=0, tfac=0, arg=0, ybarct=0;

    lefind(sle,x,xp,y,yp,s,n);
    xle = seval(sle,x,xp,s,n);
    yle = seval(sle,y,yp,s,n);
    xte = 0.5*(x[1]+x[n]);
    yte = 0.5*(y[1]+y[n]);
    chord = sqrt((xte-xle)*(xte-xle) + (yte-yle)*(yte-yle));

    //---- set unit chord-line vector
    dxc = (xte-xle) / chord;
    dyc = (yte-yle) / chord;

    srfac = sqrt(fabs(rfac));

    //---- go over each point, changing the y-thickness appropriately
    for (int i=1; i<=n; i++){
        xbar = (x[i]-xle)*dxc + (y[i]-yle)*dyc;
        ybar = (y[i]-yle)*dxc - (x[i]-xle)*dyc;

        //------ set point on the opposite side with the same chord x value
        sopps(sopp, s[i], x,xp,y,yp,s,n, sle);
        xopp = seval(sopp,x,xp,s,n);
        yopp = seval(sopp,y,yp,s,n);

        ybarop = (yopp-yle)*dxc - (xopp-xle)*dyc;

        //------ thickness factor tails off exponentially towards trailing edge
        xoc = xbar/chord;
        arg = std::min( xoc/doc , 15.0);
        tfac = 1.0 - (1.0-srfac)*exp(-arg);

        //------ set new chord x,y coordinates by changing thickness locally
        ybarct = 0.5*(ybar+ybarop) + tfac*0.5*(ybar-ybarop);

        xnew[i] = xle + xbar  *dxc - ybarct*dyc;
        ynew[i] = yle + ybarct*dxc + xbar  *dyc;
    }
}


/**
 * @brief Creates a 4-digit naca foil
 * @param ides the identification digits of the naca foil
 * @param nside the number of points of the generated foil
 */
void XFoil::naca4(int ides, int nside)
{
    int n1=0, n2=0, n3=0, n4=0, ib=0, i=0;
    //    double xx[nside], yt[nside], yc[nside], xb[2*nside], yb[2*nside]

    double *xx    = w1;
    double *yt    = w2;//thickness function
    double *yc    = w3;//mean camber line function
    memset(w1,0,sizeof(w1));
    memset(w2,0,sizeof(w2));
    memset(w3,0,sizeof(w3));

    double m=0, p=0, t=0, frac=0;

    if(nside>int(IQX/3)) nside = int(IQX/3);


    //---- te point bunching parameter
    //      data an / 1.5 /
    double an = 1.5;
    double anp = 0.0;

    n4 =  ides                             / 1000;
    n3 = (ides - n4*1000                 ) / 100;
    n2 = (ides - n4*1000 - n3*100        ) / 10;
    n1 = (ides - n4*1000 - n3*100 - n2*10);

    m = double(n4) / 100.0;         //maximum value of the mean line in hundredths of chord,
    p = double(n3) / 10.0;          //chordwise position of the maximum camber in tenths of the chord.
    t = double(n2*10 + n1) / 100.0; //maximum thickness, t/c, in percent chord.

    anp = an + 1.0;
    for (i=1; i<=nside; i++)
    {
        frac = double(i-1)/double(nside-1);
        xx[i] = 1.0 - anp*frac*pow((1.0-frac),an) - pow((1.0-frac),anp);
        yt[i] = (
                    1.4845*sqrt(xx[i])
                    - 0.6300*xx[i]
                    - 1.7580*xx[i]*xx[i]
                    + 1.4215*xx[i]*xx[i]*xx[i]
                    - 0.5075*xx[i]*xx[i]*xx[i]*xx[i]
                    ) * t ;
        if(xx[i]<p)
            yc[i] = m/p/p* (2.0*p*xx[i] - xx[i]*xx[i]);
        else
            yc[i] = m/(1.0-p)/(1.0-p) * (1.0-2.0*p + 2.0*p*xx[i]-xx[i]*xx[i]);
    }


    ib = 0;
    for(i=nside; i>= 1; i--){
        ib = ib + 1;
        xb[ib] = xx[i];
        yb[ib] = yc[i] + yt[i];
    }
    for(i=2; i<=nside; i++){
        ib = ib + 1;
        xb[ib] = xx[i];
        yb[ib] = yc[i] - yt[i];
    }
    nb = ib;

    scalc(xb,yb,sb,nb);
    segspl(xb,xbp,sb,nb);
    segspl(yb,ybp,sb,nb);

    geopar(xb,xbp,yb,ybp,sb,nb,w1,sble,chordb,areab,
           radble,angbte,ei11ba,ei22ba,apx1ba,apx2ba,
           ei11bt,ei22bt,apx1bt,apx2bt);
}


/**
 * @brief Creates a 5-digit naca foil
 * @param ides the identification digits of the naca 5 foil
 * @param nside the number of points of the generated foil
 * @return true if successful, false if not a valid 5-digit number.
 */
bool XFoil::naca5(int ides, int nside)
{
    //      real xx(nside), yt(nside), yc(nside)
    //     real xb(2*nside), yb(2*nside)
    double m=0, c=0, t=0, anp=0, frac=0;
    int n1=0, n2=0, n3=0, n4=0, n5=0, n543=0, i=0, ib=0;

    if(nside>int(IQX/3)) nside = int(IQX/3);
    double *xx = w1;
    double *yt = w2;
    double *yc = w3;

    //---- te point bunching parameter
    double an = 1.5;

    n5 =  ides                                        / 10000;
    n4 = (ides - n5*10000                           ) / 1000;
    n3 = (ides - n5*10000 - n4*1000                 ) / 100;
    n2 = (ides - n5*10000 - n4*1000 - n3*100        ) / 10;
    n1 = (ides - n5*10000 - n4*1000 - n3*100 - n2*10);

    n543 = 100*n5 + 10*n4 + n3;

    if (n543 == 210){
        //     p = 0.05
        m = 0.0580;
        c = 361.4;
    }
    else if (n543 == 220) {
        //     p = 0.10
        m = 0.1260;
        c = 51.64;
    }
    else if (n543 == 230) {
        //     p = 0.15
        m = 0.2025;
        c = 15.957;
    }
    else if (n543 == 240) {
        //     p = 0.20
        m = 0.2900;
        c = 6.643;
    }
    else if (n543 == 250) {
        //     p = 0.25
        m = 0.3910;
        c = 3.230;
    }
    else{
        std::string str("Illegal 5-digit designation\n");
        str += "first three digits must be 210, 220, ... 250";
        ides = 0;
        writeString(str);

        return false;
    }

    t = double(n2*10 + n1) / 100.0;

    anp = an + 1.0;
    for(i=1; i<=nside; i++){
        frac = double(i-1)/double(nside-1);
        xx[i] = 1.0 - anp*frac*pow((1.0-frac),an) - pow((1.0-frac),anp);
        yt[i] = ( 0.29690*sqrt(xx[i])
                  - 0.12600*xx[i]
                  - 0.35160*xx[i]*xx[i]
                  + 0.28430*xx[i]*xx[i]*xx[i]
                  - 0.10150*xx[i]*xx[i]*xx[i]*xx[i]) * t / 0.20;
        if(xx[i]<m)
            yc[i] = (c/6.0) * (xx[i]*xx[i]*xx[i] - 3.0*m*xx[i]*xx[i]
                               + m*m*(3.0-m)*xx[i]);
        else
            yc[i] = (c/6.0) * m*m*m * (1.0 - xx[i]);

    }

    ib = 0;
    for(i=nside;i>=1;i--){
        ib = ib + 1;
        xb[ib] = xx[i];
        yb[ib] = yc[i] + yt[i];
    }

    for(i=2; i<=nside;i++){
        ib = ib + 1;
        xb[ib] = xx[i];
        yb[ib] = yc[i] - yt[i];
    }
    nb = ib;
    return true;
}



void XFoil::fillHk()
{
    int nside[] = {0,0,0};
    nside[1] = m_nSide1;
    nside[2] = m_nSide2;
    double thi=0, dsi=0, uei=0, uc=0, amsq=0, dummy=0;
    double hstinv = gamm1*(minf/qinf)*(minf/qinf) / (1.0 + 0.5*gamm1*minf*minf);

    //---- fill kinematic shape parameter array
    for (int is=1; is<=2; is++)
    {
        for(int ibl=2; ibl< nside[is]; ibl++)
        {
            thi = thet[ibl][is];
            dsi = dstr[ibl][is];
            uei = uedg[ibl][is];
            uc = uei * (1.0-tklam) / (1.0 - tklam*(uei/qinf)*(uei/qinf));
            amsq = uc*uc*hstinv / (gamm1*(1.0 - 0.5*uc*uc*hstinv));
            hkin(dsi/thi, amsq, Hk[ibl][is], dummy, dummy);
        }
    }
}


void XFoil::fillRTheta()
{
    int nside[] = {0,0,0};
    nside[1] = m_nSide1;
    nside[2] = m_nSide2;
    double ue=0, herat=0, rhoe=0, amue=0, uei=0;
    //---- 1 / (total enthalpy)
    double hstinv = gamm1*(minf/qinf)*(minf/qinf) / (1.0 + 0.5*gamm1*minf*minf);

    //---- Sutherland's const./to   (assumes stagnation conditions are at stp)
    double hvrat = 0.35;

    //---- fill rtheta arrays
    for (int is=1; is<=2; is++)
    {
        for(int ibl=2; ibl< nside[is]; ibl++)
        {
            uei = uedg[ibl][is];
            ue  = uei * (1.0-tklam) / (1.0 - tklam*(uei/qinf)*(uei/qinf));
            herat =   (1.0 - 0.5*hstinv*ue  *ue)
                    / (1.0 - 0.5*hstinv*qinf*qinf);
            rhoe = pow(herat, 1.0/gamm1);
            amue = sqrt(herat*herat*herat) * (1.0+hvrat)/(herat+hvrat);
            RTheta[ibl][is] = reinf * rhoe*ue*thet[ibl][is]/amue;
        }
    }
}


/**
 * @brief Creates the x coordinates of the boundary layer points.
 * @param xs the pre-allocated array of coordinates
 * @param nside1 the number of nodes on side 1 (top?)
 * @param nside2 the number of nodes on side 2 (bottom?)
 */
void XFoil::createXBL()
{
    int i=0;
    //---- set up cartesian bl x-arrays for plotting
    for(int is=1; is<=2; is++)
    {
        for (int ibl=2; ibl<=nbl[is]; ibl++)
        {
            i = ipan[ibl][is];
            xbl[ibl][is] = x[i];
//            xxtr[is] = xle + (xte-xle)*xoctr[is] - (yte-yle)*yoctr[is];
        }
    }

    m_nSide1 = nbl[2] + iblte[1] - iblte[2];
    m_nSide2 = nbl[2];

    for(int iblw=1; iblw <= nbl[2]-iblte[2]; iblw++)
        xbl[iblte[1]+iblw][1] = xbl[iblte[2]+iblw][2];
}

