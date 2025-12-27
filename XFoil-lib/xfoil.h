/****************************************************************************

    XFoil Class
    Copyright (C) 2000 Mark Drela 
    André Deperrois - translation to C - 2003

    
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

 
/**
 *@file This class defines the Xfoil object.
 */


#pragma once


/**
*@class XFoil
*@brief  The class which defines the XFoil object.

This is a translation to C++ of the original Fortran code by Mark Drela and Harold Youngren.
See http://raphael.mit.edu/xfoil for more information.
*/


#include <string>
#include <complex>


#include <xfoil-lib_global.h>
#include <xfoil_params.h>


struct blData
{
    public:
      double xz,  uz,  tz,  dz,  sz, amplz, uz_uei, uz_ms, dwz,
            hz, hz_tz, hz_dz,
            mz, mz_uz,                            mz_ms,
            rz, rz_uz,                            rz_ms,
            vz, vz_uz,                            vz_ms,  vz_re,
            hkz, hkz_uz, hkz_tz, hkz_dz,         hkz_ms,
            hsz, hsz_uz, hsz_tz, hsz_dz,         hsz_ms, hsz_re,
            hcz, hcz_uz, hcz_tz, hcz_dz,         hcz_ms,
            rtz, rtz_uz, rtz_tz,                 rtz_ms, rtz_re,
            cfz, cfz_uz, cfz_tz, cfz_dz,         cfz_ms, cfz_re,
            diz, diz_uz, diz_tz, diz_dz, diz_sz, diz_ms, diz_re,
            usz, usz_uz, usz_tz, usz_dz,         usz_ms, usz_re,
            cqz, cqz_uz, cqz_tz, cqz_dz,         cqz_ms, cqz_re,
            dez, dez_uz, dez_tz, dez_dz,         dez_ms;
};



class XFOILLIBSHARED_EXPORT XFoil
{
    public:
        XFoil();
        virtual ~XFoil();

    public:
        void interpolate(double xf1[], double yf1[], int n1,
                         double xf2[], double yf2[], int n2, double mixt);

        bool CheckAngles();
        bool PreProcess();
        std::string const &report() const {return m_Report;}


        void pangen();
        void pert_process(int kqsp);
        void pert_init(int kqsp);
        void HanningFilter(double cfilt, std::string &strout);
        void smooq(int kq1,int kq2,int kqsp);
        void ExecMDES();
        bool ExecQDES();
        bool initialize();
        bool initXFoilGeometry(int fn, const double *fx, const double *fy, double *fnx, double *fny,
                               bool bFLap=false, double xhinge=0.0, double yhinge=0.0);
        bool initXFoilAnalysis(double Re, double alpha, double Mach, double NCrit, double XtrTop, double XtrBot,
                               int reType, int maType, bool bViscous);

        void splqsp(int kqsp);
        void qspcir();
        void InitMDES();
        bool InitQDES();
        void cncalc(double qc[], bool lsymm);
        double qcomp(double g);
        bool clcalc(double xref, double yref);

        void createXBL();
        void fillHk();
        void fillRTheta();
        void writeString(std::string const &str, bool bFullReport = false);
        double DeRotate();
        bool specal();
        bool speccl();
        bool viscal();
        bool ViscalEnd();
        bool ViscousIter();
        bool fcpmin();
        int cadd(int ispl, double atol, double xrf1, double xrf2);
        bool abcopy();
        void tcset(double cnew, double tnew);
        void hipnt(double xhc, double xht);
        void lerad(double rfac, double blend);
        void naca4(int ides, int nside);
        bool naca5(int ides, int nside);
        void tgap(double gapnew, double blend);

        bool isBLInitialized() const {return lblini;}
        void setBLInitialized(bool bInitialized) {lblini = bInitialized;}

        double QInf() const {return qinf;}
        void setQInf(double v) {qinf=v;}

        double alpha() const {return alfa;}
        void setAlpha(double aoa) {alfa = aoa;}

        double ClSpec() const {return clspec;}
        void setClSpec(double cl) {clspec=cl;}


        static bool isCancelled() {return s_bCancel;}
        static void setCancel(bool bCancel) {s_bCancel=bCancel;}
        static void setFullReport(bool bFull) {s_bFullReport=bFull;}
        static bool bFullReport() {return s_bFullReport;}
        static double VAccel() {return vaccel;}
        static void setVAccel(double accel) {vaccel=accel;}

    private:

        void inter(double x0[], double xp0[], double y0[], double yp0[], double s0[],int n0,double sle0,
                   double x1[], double xp1[], double y1[], double yp1[], double s1[],int n1,double sle1,
                   double x[], double y[], int n, double frac);
        void thkcam(double tfac, double cfac);
        void qspint(int kqsp, double &clq);

        double qincom(double qc, double qinf, double tklam);
        void qccalc(int ispec,double *alfa, double *cl, double *cm,double minf, double qinf, int *ncir, double xcir[], double ycir[], double scir[], double qcir[]);
        void mapgam(int iac, double &alg, double &clg, double &cmg);
        bool eiwset(int nc1);
        void cgauss(int nn,std::complex <double> z[IMX4+1][IMX4+1],std::complex <double> r[IMX4+1]);
        void zccalc(int mtest);
        void zcnorm(int mtest);
        void zlefind(std::complex<double>*zle, std::complex<double>zc[],double wc[],int nc, std::complex<double>piq[], double agte);
        void piqsum();
        void ftp();
        void scinit(int n, double x[], double xp[], double y[], double yp[], double s[], double sle);
        void mapgen(int n, double x[],double y[]);

        //    int kqtarg,nname,nprefix;
        void gamlin(int i, int j, double coef);
        bool mixed(int kqsp);
        void gamqsp(int kqsp);
        void cnfilt(double ffilt);
        void RestoreQDES();

        bool setMach();
        void scheck(double x[], double y[], int *n, double stol, bool *lchange);
        void sss(double ss, double *s1, double *s2, double del, double xbf, double ybf,
                 double x[], double xp[], double y[], double yp[], double s[],int n, int iside);
        bool inside(double xb[], double yb[], int nb, double xbf, double ybf);
        void flap();
        int arefine(double x[],double y[], double s[], double xs[], double ys[],
                     int n, double atol, int ndim,
                     double xnew[], double ynew[], double x1, double x2);

        bool comset();
        bool mrcl(double cls, double &m_cls, double &r_cls);
        bool restoreblData(int icom);
        bool saveblData(int icom);



        bool aecalc(int n, double x[], double y[], double t[], int itype, double &area,
                    double &xcen, double &ycen, double &ei11, double &ei22, double &apx1, double &apx2);
        bool apcalc();
        bool axset( double hk1, double thet1, double rt1, double a1,
                    double hk2, double thet2, double rt2, double a2,
                    double acrit, double &ax,
                    double &ax_hk1, double &ax_t1, double &ax_rt1, double &ax_a1,
                    double &ax_hk2, double &ax_t2, double &ax_rt2, double &ax_a2);
        bool baksub(int n, double a[IQX][IQX],int indx[], double b[]);
        bool bldif(int ityp);
        bool blkin();
        bool blmid(int ityp);
        bool blprv(double xsi,double ami,double cti,double thi,double dsi,double dswaki,double uei);
        bool blsolve();
        bool blsys();
        bool blvar(int ityp);
        bool cang(double x[], double y[], int n, int &imax, double &amax);
        bool cdcalc();
        bool cfl(double hk, double rt, double &cf, double &cf_hk, double &cf_rt, double &cf_msq);
        bool cft(double hk, double rt, double msq, double &cf, double &cf_hk, double &cf_rt, double &cf_msq);
        bool cpcalc(int n, double q[], double qinf, double minf, double cp[]);
        bool dampl(double hk, double th, double rt,
                   double &ax, double &ax_hk, double &ax_th, double &ax_rt);
        bool dil(double hk, double rt, double &di, double &di_hk, double &di_rt);
        bool dilw(double hk, double rt, double &di, double &di_hk, double &di_rt);
        bool dslim(double &dstr, double thet, double msq, double hklim);

        bool gamqv();
        bool Gauss(int nn, double z[][6], double r[5]);
        bool Gauss(int nn, double z[IQX][IQX], double r[IQX]);
        bool geopar(double x[], double xp[], double y[], double yp[], double s[],
                   int n, double t[], double &sle, double &chord,
                   double &area, double &radle, double &angte,
                   double &ei11a, double &ei22a, double &apx1a, double &apx2a,
                   double &ei11t, double &ei22t, double &apx1t, double &apx2t);
        void tccalc(double x[], double xp[], double y[], double yp[], double s[],
                    int n, double &thick, double &xthick, double &cambr, double &xcambr);

        void sopps(double &sopp, double si, double x[], double xp[], double y[], double yp[], double s[],
                      int n, double sle);
        void getcam(double xcm[],double ycm[], int &ncm,double xtk[],double ytk[],int &ntk,
                        double x[],double xp[],double y[],double yp[],double s[],int n );
        void getmax(double x[],double y[], double yp[], int n,double &xmax, double &ymax);
        void xlfind(double &sle, double x[], double xp[], double y[], double yp[], double s[], int n);
        void sortol(double tol,int &kk,double s[],double w[]);
        bool getxyf(double x[],double xp[],double y[],double yp[],double s[], int n, double &tops, double &bots,double xf,double &yf);
        bool ggcalc();
        bool hct(double hk, double msq, double &hc, double &hc_hk, double &hc_msq);
        bool hkin(double h, double msq, double &hk, double &hk_h, double &hk_msq);
        bool hsl(double hk, double &hs, double &hs_hk, double &hs_rt, double &hs_msq);
        bool hst(double hk, double rt, double msq, double &hs, double &hs_hk, double &hs_rt, double &hs_msq );
        bool iblpan();
        bool iblsys();
        bool lefind(double &sle, double x[], double xp[], double y[], double yp[], double s[], int n);
        void lerscl(double *x, double *xp, double* y, double *yp, double *s, int n, double doc, double rfac, double *xnew,double *ynew);
        bool ludcmp(int n, double a[IQX][IQX],int indx[IQX]);
        bool mhinge();
        bool mrchdu();
        bool mrchue();
        bool ncalc(double x[], double y[], double s[], int n, double xn[], double yn[]);
        bool psilin(int i, double xi,double yi,double nxi, double nyi, double &psi, double &psi_ni, bool geolin, bool siglin);
        bool pswlin(int i,double xi, double yi, double nxi, double nyi, double &psi, double &psi_ni);
        bool qdcalc();
        bool qiset();
        bool qvfue();
        bool qwcalc();
        bool scalc(double x[], double y[], double s[], int n);
        bool segspl( double x[], double xs[], double s[], int n);
        bool segspld(double x[], double xs[], double s[], int n, double xs1, double xs2);
        bool setbl();
        bool setexp(double s[],double ds1,double smax,int nn);
        bool sinvrt(double &si,double xi,double x[],double xs[],double s[],int n);

        void splina(double x[], double xs[], double s[], int n);
        bool splind(double *x, double *xs, double *s, int n, double xs1, double xs2);
        bool stepbl();
        bool stfind();
        bool stmove();
        bool tecalc();
        bool tesys(double cte, double tte, double dte);
        bool trchek();
        bool trdif();
        bool trisol(double a[],double b[], double c[], double d[], int kk);
        bool ueset();
        bool uicalc();
        bool update();
        bool xicalc();
        bool xifset(int is);
        bool xyWake();
        double aint(double number);
        double atanc(double y, double x, double thold);
        double curv(double ss, double x[], double xs[], double y[], double ys[], double s[], int n);
        double d2val(double ss, double x[], double xs[], double s[], int n);
        double deval(double ss, double x[], double xs[], double s[], int n);
        double seval(double ss, double x[], double xs[], double s[], int n);
        double sign(double a, double b);


    public:
        static double vaccel;
        static bool s_bCancel;
        static bool s_bFullReport;

        std::string m_Report;

        double agte,ag0,qim0,qimold;
        double ssple, dwc,algam,clgam,cmgam;
        double clspec;
        double sspec[IBX+1],xspoc[IBX+1],yspoc[IBX+1];
        double qspec[IPX+1][IBX+1],qspecp[IPX+1][IBX+1];
        double alqsp[IPX+1],clqsp[IPX+1],cmqsp[IPX+1];
        std::complex<double> dzte, chordz, zleold, zcoldw[ICX+1];
        std::complex<double> piq[ICX+1], cn[IMX+1], eiw[ICX+1][IMX+1];
        double dnTrace[100];//... added techwinder
        double dgTrace[100];//... added techwinder
        int QMax;

        bool lqspec, lsym,leiw,lqslop,lscini, lrecalc, lcnpl;
        int nsp,nqsp,iacqsp;
        int nc,nc1,mc,mct;
        int iq1, iq2;
        int imax;// needed for preprocessing

        double thickb,cambrb;
        double xb[IBX],yb[IBX],nx[IZX],ny[IZX];
        double xpref1,xpref2;
        double cvpar,cterat,ctrrat,xsref1,xsref2;

        double cl,cm,cd,cdp,cdf,cpi[IZX],cpv[IZX],acrit;
        double xcp;
        double alfa, avisc, awake, reinf1, qinf, mvisc, rmsbl, ante;
        double cpmn;
        double minf, reinf;
        bool lalfa, lvisc, lvconv, lwake;
        double qgamm[IBX+1];
        double hmom;
        double hfx,hfy;
        bool lcpxx;
        int niterq;
        double xbf,ybf;
        double ddef; // flap angle
        double rmxbl;

        double amax;// needed for preprocessing
        double minf1;
        bool lblini, lipan,lqsym;
        bool lbflap,lflap;
        int n, nb,iblte[ISX],ipan[IVX][ISX],nbl[ISX];
        int npan;
        double x[IZX],y[IZX],xstrip[ISX],xoctr[ISX],yoctr[ISX];
        double qvis[IZX];
        bool liqset;
        double adeg, xcmref, ycmref;
        double tklam;
        double xp[IZX],yp[IZX],s[IZX];
        double dtor;

        double thet[IVX][ISX], tau[IVX][ISX], ctau[IVX][ISX], ctq[IVX][ISX];
        double dis[IVX][ISX], uedg[IVX][ISX];
        double xbl[IVX][ISX], Hk[IVX][ISX], RTheta[IVX][ISX];
        double dstr[IVX][ISX];
        double delt[IVX][ISX];
        int m_nSide1, m_nSide2;
        int itran[ISX];

        double m_ctrl; /** information storage for xflr5 gui */

    private:

        double wc[ICX+1],sc[ICX+1];
        double scold[ICX+1],xcold[ICX+1],ycold[ICX+1];
        double qf0[IQX+1],qf1[IQX+1],qf2[IQX+1],qf3[IQX+1];

        double qdof0,qdof1,qdof2,qdof3,ffilt;

        std::complex<double> zc[ICX+1], zc_cn[ICX+1][IMX4+1];
        std::complex<double> cnsav[IMX+1];

        int retyp, matyp;
        double rlx;

        double minf_cl, reinf_cl;
        double angtol;

        double xcam[IQX], ycam[IQX], xthk[IQX], ythk[IQX], ycamp[IQX], ythkp[IQX];
        double  thick, xthick, cambr, xcambr;
        int ncam, nthk;

        blData blsav[3];
        std::complex<double> conjg(std::complex<double> cplx);

        bool m_bTrace;

    //    int nax, npx, nfx;

    //    double com2[74], com1[74];
    //    int ncom;
    //    std::string labref;
    //    std::string fname, pfname, pfnamx;
    //    std::string oname, prefix;
    //    std::string name, namepol, codepol, nameref;
    //    std::string ispars;


    //    bool lpacc,lqvdes,,lqrefl,lcpref,lforef,lpfile,lpfilx;
    //    bool lppsho,lplot,lclip,lvlab,lcurs,lcminp, lhmomp,lland;
    //    bool lplcam,lgparm,lnorm,,lgsym,lgslop, lcslop,lclock,
        bool limage,lgamu,sharp,lqaij,ladij,lwdij;
        bool lqinu,lgsame;//???
    //    bool lgtick, lplegn,,lplist, lpgrid,lblgrd,lblsym,
    //         lcpgrd,lggrid,lgeopl, lpcdw,lqsppl, liqset, lqgrid;

        double sccon, gacon, gbcon, gbc0, gbc1, gccon, dlcon, ctcon;

    //---- dimension temporary work and storage arrays [equivalenced below]
        double w1[6*IQX], w2[6*IQX], w3[6*IQX], w4[6*IQX];
        double w5[6*IQX], w6[6*IQX], w7[6*IQX], w8[6*IQX];
        int nsys;
        int isys[IVX][ISX];
        double xbp[IBX],ybp[IBX],sb[IBX],snew[4*IBX];
        double xof,yof,sble,chordb;
    //    double xbmin,xbmax,ybmin,ybmax;
        double areab,radble,angbte;
        double ei11ba,ei22ba,apx1ba,apx2ba,ei11bt,ei22bt,apx1bt,apx2bt;
    //    double xcm[1200],ycm[1200],scm[1200],xcmp[1200],ycmp[1200];
    //    double xtk[1200],ytk[1200],stk[1200];
    //    double xtkp[1200],ytkp[1200];

        double sle,xle,yle,xte,yte;
        double chord,yimage,wgap[IWX],waklen;
    //    double size,scrnfr,plotar, pfac,qfac,vfac,xwind,ywind;
    //    double xpage,ypage,xmarg,ymarg, chg, chq,xofair,yofair,facair, xofa,yofa,faca,uprwt;
    //    double cpmin,cpmax,cpdel;
    //    double cpolplf[3][4];
    //    double xcdwid,xalwid,xocwid;
        double ch;
        int nw,ist;

    //    int kimage,nseqex;
        int aijpiv[IQX];
    //    int idev,idevrp,ipslu,ncolor,icols[5],nover, ncm,ntk;

        double cl_alf, cl_msq;
        double psio,cosa,sina,gamma,gamm1;
    //    double circ;
        double tkl_msq,cpstar,qstar;
        double cpmni,cpmnv,xcpmni,xcpmnv;
        double arad;
        double xssi[IVX][ISX],uinv[IVX][ISX],mass[IVX][ISX];
        double uslp[IVX][ISX],guxq[IVX][ISX],guxd[IVX][ISX];
        double vti[IVX][ISX];
        double xssitr[ISX],uinv_a[IVX][ISX];
        double gam[IQX],gam_a[IQX],gamu[IQX][ISX],sig[IZX];
        double apanel[IZX],sst,sst_go,sst_gp,gamte,sigte;
    //    double sigte_a,gamte_a;
        double dste,aste;
        double qinv[IZX],qinvu[IZX][3], qinv_a[IZX];
        double q[IQX][IQX],dq[IQX],dzdg[IQX],dzdn[IQX],dzdm[IZX],dqdg[IQX];
        double dqdm[IZX],qtan1,qtan2,z_qinf,z_alfa,z_qdof0,z_qdof1,z_qdof2,z_qdof3;
        double aij[IQX][IQX];
        double bij[IQX][IZX],dij[IZX][IZX];
        double cij[IWX][IQX];
        double hopi,qopi;


        double vs1[5][6],vs2[5][6],vsrez[5],vsr[5],vsm[5],vsx[5];
        bool tforce[ISX];

        bool trforc, simi,tran,turb,wake, trfree;

        double dwte, qinfbl, tkbl, tkbl_ms, rstbl, rstbl_ms, hstinv, hstinv_ms;
        double reybl, reybl_ms, reybl_re, gambl, gm1bl, hvrat, bule, xiforc, amcrit;

        double x2, u2, theta2, d2, s2, ampl2, u2_uei, u2_ms, dw2,
            h2, h2_t2, h2_d2, m2, m2_u2,m2_ms, r2, r2_u2,r2_ms,
            v2, v2_u2,v2_ms,v2_re, hk2, hk2_u2, hk2_t2, hk2_d2,hk2_ms,
            hs2, hs2_u2, hs2_t2, hs2_d2,hs2_ms, hs2_re, hc2, hc2_u2,
            hc2_t2, hc2_d2,hc2_ms, rt2, rt2_u2, rt2_t2,rt2_ms, rt2_re,
            cf2, cf2_u2, cf2_t2, cf2_d2,cf2_ms, cf2_re, di2, di2_u2,
            di2_t2, di2_d2, di2_s2, di2_ms, di2_re, us2, us2_u2, us2_t2,
            us2_d2,us2_ms, us2_re, cq2, cq2_u2, cq2_t2, cq2_d2,cq2_ms,
            cq2_re, de2, de2_u2, de2_t2, de2_d2,de2_ms;

        double x1, u1, theta1, d1, s1, ampl1, u1_uei, u1_ms, dw1, h1, h1_t1, h1_d1,
            m1, m1_u1,m1_ms,r1, r1_u1,r1_ms,v1, v1_u1,v1_ms, v1_re,hk1, hk1_u1,
            hk1_t1, hk1_d1,hk1_ms,hs1, hs1_u1, hs1_t1, hs1_d1,hs1_ms, hs1_re,
            hc1, hc1_u1, hc1_t1, hc1_d1,hc1_ms, rt1, rt1_u1, rt1_t1,rt1_ms,
            rt1_re, cf1, cf1_u1, cf1_t1, cf1_d1,cf1_ms, cf1_re, di1, di1_u1,
            di1_t1, di1_d1, di1_s1, di1_ms, di1_re, us1, us1_u1, us1_t1, us1_d1,
            us1_ms, us1_re, cq1, cq1_u1, cq1_t1, cq1_d1,cq1_ms, cq1_re, de1, de1_u1,
            de1_t1, de1_d1,de1_ms;
    //    double  xoff,yoff,xgmin,xgmax,ygmin,ygmax,dxyg,xcmin,xcmax,ycmin,ycmax,dxyc,dyoffc,xpmin,xpmax,ypmin,ypmax,dxyp,dyoffp,ysfp,gtick;
        int imxbl,ismxbl;
        double  xsf,ysf;
    //    std::string vmxbl;

    //    double cpol[800][iptot][9],cpolsd[800][3][jptot][9];//what's iptot???
    //    double xpref[300],cpref[300], verspol[9],cpolxy[300][2][9]
    //    double machp1[9], reynp1[9],acritp[9],xstripp[3][9],cpolref[128][2][4][9];
        double cfm, cfm_ms, cfm_re, cfm_u1, cfm_t1, cfm_d1, cfm_u2, cfm_t2, cfm_d2;
        double xt, xt_a1, xt_ms, xt_re, xt_xf, xt_x1, xt_t1, xt_d1, xt_u1,
              xt_x2, xt_t2, xt_d2, xt_u2;
        double va[4][3][IZX],vb[4][3][IZX],vdel[4][3][IZX],vm[4][IZX][IZX],vz[4][3];

    //    int ncpref, napol[9], npol, ipact, nlref, icolp[9],icolr[9],imatyp[9],iretyp[9], nxypol[9],npolref, ndref[4][9];
    //    double c1sav[74], c2sav[74];

    /*
    c
    c-    sccon  =  shear coefficient lag constant
    c-    gacon  =  g-beta locus constants...
    c-    gbcon  =  g = gacon * sqrt(1.0 + gbcon*beta)
    c-    gccon  =         + gccon / [h*rtheta*sqrt(cf/2)]   <-- wall term
    c-    dlcon  =  wall/wake dissipation length ratio  lo/l
    c-    ctcon  =  ctau weighting coefficient (implied by g-beta constants)

    c   version     version number of this xfoil implementation
    c
    c   fname       airfoil data filename
    c   pfname[.]   polar append filename
    c   pfnamx[.]   polar append x/c dump filename
    c   oname       default overlay airfoil filename
    c   prefix      default filename prefix
    c   name        airfoil name
    c
    c   ispars      ises domain parameters  [not used in xfoil]
    c
    c   q[..]       generic coefficient matrix
    c   dq[.]       generic matrix righthand side
    c
    c   dzdg[.]     dpsi/dgam
    c   dzdn[.]     dpsi/dn
    c   dzdm[.]     dpsi/dsig
    c
    c   dqdg[.]     dqtan/dgam
    c   dqdm[.]     dqtan/dsig
    c   qtan1       qtan at alpha =  0 deg.
    c   qtan2       qtan at alpha = 90 deg.
    c
    c   z_qinf      dpsi/dqinf
    c   z_alfa      dpsi/dalfa
    c   z_qdof0     dpsi/dqdof0
    c   z_qdof1     dpsi/dqdof1
    c   z_qdof2     dpsi/dqdof2
    c   z_qdof3     dpsi/dqdof3
    c
    c   aij[..]     dpsi/dgam  influence coefficient matrix [factored if lqaij=t]
    c   bij[..]     dgam/dsig  influence coefficient matrix
    c   cij[..]     dqtan/dgam influence coefficient matrix
    c   dij[..]     dqtan/dsig influence coefficient matrix
    c   qinv[.]     tangential velocity due to surface vorticity
    c   qvis[.]     tangential velocity due to surface vorticity & mass sources
    c   qinvu[..]   qinv for alpha = 0, 90 deg.
    c   qinv_a[.]   dqinv/dalpha
    c
    c   x[.],y[.]   airfoil [1<i<n] and wake [n+1<i<n+nw] coordinate arrays
    c   xp[.],yp[.] dx/ds, dy/ds arrays for spline evaluation
    c   s[.]        arc length along airfoil [spline parameter]
    c   sle         value of s at leading edge
    c   xle,yle     leading  edge coordinates
    c   xte,yte     trailing edge coordinates
    c   wgap[.]     thickness of "dead air" region inside wake just behind te
    c   waklen      wake length to chord ratio
    c
    c   gam[.]      surface vortex panel strength array
    c   gamu[.2]    surface vortex panel strength arrays for alpha = 0, 90 deg.
    c   gam_a[.]    dgam/dalfa
    c   sig[.]      surface and wake mass defect array
    c
    c   nx[.],ny[.] normal unit vector components at airfoil and wake coordinates
    c   apanel[.]   surface and wake panel angle array [+ counterclockwise]
    c
    c   sst         s value at stagnation point
    c   sst_go      dsst/dgam[ist]
    c   sst_gp      dsst/dgam[ist+1]
    c
    c   gamte       vortex panel strength across finite-thickness te
    c   sigte       source panel strength across finite-thickness te
    c   gamte_a     dgamte/dalfa
    c   sigte_a     dsigte/dalfa
    c   dste        te panel length
    c   ante,aste   projected te thickness perp.,para. to te bisector
    c   sharp       .true.  if  dste.eq.0.0 ,  .false. otherwise
    c
    c   sspec[.]    normalized arc length around airfoil [qspec coordinate]
    c   xspoc[.]    x/c at sspec points
    c   yspoc[.]    y/c at sspec points
    c   qspec[..]   specified surface velocity for inverse calculations
    c   qspecp[..]  dqspec/dsspec
    c   qgamm[.]    surface velocity for current airfoil geometry
    c   ssple       sspec value at airfoil nose
    c
    c   iq1,iq2     target segment endpoint indices on qspec[s] plot
    c   nsp         number of points in qspec array
    c   nqsp        number qspec arrays
    c   iacqsp      1:  alqsp is prescribed for qspec arrays
    c               2:  clqsp is prescribed for qspec arrays
    c   nc1         number of circle plane points, must be 2**n - 1
    c
    c   nname       number of characters in airfoil name
    c   nprefix     number of characters in default filename prefix
    c
    c   alqsp[.]    alpha,cl,cm corresponding to qspec distributions
    c   clqsp[.]
    c   cmqsp[.]
    c   algam       alpha,cl,cm corresponding to qgamm distribution
    c   clgam
    c   cmgam
    c
    c   qf0[.]      shape function for qspec modification
    c   qf1[.]        "
    c   qf2[.]        "
    c   qf3[.]        "
    c   qdof0       shape function weighting coefficient [inverse dof]
    c   qdof1         "
    c   qdof2         "
    c   qdof3         "
    c   clspec      specified cl
    c   ffilt       circle-plane mapping filter parameter
    c
    c   adeg,alfa   angle of attack in degrees, radians
    c   awake       angle of attack corresponding to wake geometry [radians]
    c   avisc       angle of attack corresponding to bl solution   [radians]
    c   mvisc       mach number corresponding to bl solution
    c   cl,cm       current cl and cm calculated from gam[.] distribution
    c   cd          current cd from bl solution
    c   cdf         current friction cd from bl solution
    c   cl_alf      dcl/dalfa
    c   cl_msq      dcl/d[minf^2]
    c
    c   psio        streamfunction inside airfoil
    c   circ        circulation
    c   cosa,sina   cos[alfa], sin[alfa]
    c   qinf        freestream speed    [defined as 1]
    c   gamma,gamm1 gas constant cp/cv, cp/cv - 1
    c   minf1       freestream mach number at cl=1
    c   minf        freestream mach number at current cl
    c   minf_cl     dminf/dcl
    c   tklam       karman-tsien parameter minf^2 / [1 + sqrt[1-minf^2]]^2
    c   tkl_msq     d[tklam]/d[minf^2]
    c   cpstar      sonic pressure coefficient
    c   qstar       sonic speed
    c
    c   ncpref      number of reference cp vs x/c points
    c   xpref[.]    x/c array corresponding to reference cp data array
    c   cpref[.]    reference cp data array
    c   labref      reference cp data descriptor string
    c
    c   nlref       number of characters in labref string
    c   napol[.]    number of points in each stored polar
    c   npol        number of stored polars
    c   ipact       index of "active" polar being accumulated [0 if none are]
    c   icolp[.]    color for each polar
    c   icolr[.]    color for each reference polar
    c
    c   ndref[..]   number of points in each stored reference polar
    c   npolref     number of stored reference polars
    c
    c   verspol[.]  version number of generating-code for each polar
    c   cpol[...]   cl,cd,and other parameters for each polar
    c   cpolxy[.1.] x,y coordinates of airfoil geometry which generated each polar
    c   cpolxy[.2.]
    c   nxypol[.]   number of x,y points in cpolxy array
    c
    c   pxtr[..]    transition locations for each polar
    c   namepol[.]  airfoil names for each polar
    c   codepol[.]  generating-code names for each polar
    c
    c   nameref[.]  name label of reference polar
    c
    c   pi          3.1415926...
    c   hopi,qopi   1/[2 pi] ,  1/[4 pi]
    c   dtor        pi / 180    [degrees to radians conversion factor]
    c
    c   cvpar       curvature attraction parameter for airfoil paneling
    c               0 = uniform panel node spacing around airfoil
    c              ~1 = panel nodes strongly bunched in areas of large curvature
    c   cterat      te panel density / le panel density ratio
    c   ctrrat      local refinement panel density / le panel density ratio
    c   xsref1-2    suction  side local refinement x/c limits
    c   xpref1-2    pressure side local refinement x/c limits
    c
    c   n           number of points on airfoil
    c   nb          number of points in buffer airfoil array
    c   nw          number of points in wake
    c   npan        default/specified number of points on airfoil
    c
    c   ist         stagnation point lies between s[ist], s[ist+1]
    c   itmax       max number of newton iterations
    c   nseqex      max number of unconverged sequence points for early exit
    c
    c   retyp       index giving type of re variation with cl ...
    c            ... 1  re constant
    c            ... 2  re ~ 1/sqrt[cl]    [fixed lift]
    c            ... 3  re ~ 1/cl          [fixed lift and dynamic pressure]
    c
    c   matyp       index giving type of ma variation with cl ...
    c            ... 1  ma constant
    c            ... 2  ma ~ 1/sqrt[cl]    [fixed lift]
    c
    c   aijpiv[.]   pivot index array for lu factoring routine
    c
    c   idev        "device" number for normal screen plotting
    c   idevrp      "device" number for replotting [typically for hardcopy]
    c   ipslu       postscript file specifier
    c   ncolor      number of defined colors in colormap
    c   icols[1]    color indices of top side
    c   icols[2]    color indices of bottom side
    c
    c   nover       number of airfoils overlaid on gdes geometry plot
    c
    c   scrnfr      screen fraction taken up by initial plot window
    c   size        plot width [inches]
    c   plotar      plot aspect ratio
    c   xwind,ywind window size in inches
    c   xpage,ypage plot-page size in inches [for hardcopy]
    c   xmarg,ymarg margin dimensions in inches
    c   pfac        scaling factor for  cp
    c   qfac        scaling factor for  q  [surface speed]
    c   vfac        scaling factor for  cp vectors
    c   ch          character width / plot size  ratio
    c   chg         character width / plot size  ratio for geometry plot
    c   chq         character width / plot size  ratio for qspec[s] plot
    c
    c   xofair      x offset for airfoil in  cp vs x plots
    c   yofair      y offset for airfoil in  cp vs x plots
    c   facair      scale factor for airfoil in  cp vs x plots
    c   xofa        x offset for airfoil in  cp vs x plots in airfoil units
    c   yofa        y offset for airfoil in  cp vs x plots in airfoil units
    c   faca        scale factor for airfoil in  cp vs x plots  in airfoil units
    c   uprwt       u/qinf scale factor for profile plotting
    c   cpmax       max cp  in  cp vs x plots
    c   cpmin       min cp  in  cp vs x plots
    c   cpdel       delta cp  in  cp vs x plots
    c
    c   cpolplf[1,icd]  min cd in cd-cl polar plot
    c   cpolplf[2,icd]  max cd in cd-cl polar plot
    c   cpolplf[3,icd]  delta cd in cd-cl polar plot
    c
    c   xcdwid      width of cd   -cl polar plot
    c   xalwid      width of alpha-cl polar plot
    c   xocwid      width of xtr/c-cl polar plot
    c
    c   ok          user question response
    c   limage      .true. if image airfoil is present
    c   lgamu       .true. if gamu  arrays exist for current airfoil geometry
    c   lqinu       .true. if qinvu arrays exist for current airfoil geometry
    c   lvisc       .true. if viscous option is invoked
    c   lalfa       .true. if alpha is specifed, .false. if cl is specified
    c   lwake       .true. if wake geometry has been calculated
    c   lpacc       .true. if each point calculated is to be saved
    c   lblini      .true. if bl has been initialized
    c   lipan       .true. if bl->panel pointers ipan have been calculated
    c   lqaij       .true. if dpsi/dgam matrix has been computed and factored
    c   ladij       .true. if dq/dsig matrix for the airfoil has been computed
    c   lwdij       .true. if dq/dsig matrix for the wake has been comphd
    c   lqvdes      .true. if viscous ue is to be plotted in qdes routines
    c   lqspec      .true. if qspec has been initialized
    c   lqrefl      .true. if reflected qspec is to be plotted in qdes routines
    c   lvconv      .true. if converged bl solution exists
    c   lcpref      .true. if reference data is to be plotted on cp vs x/c plots
    c   lclock      .true. if source airfoil coordinates are clockwise
    c   lpfile      .true. if polar file is ready to be appended to
    c   lpfilx      .true. if polar dump file is ready to be appended to
    c   lppsho      .true. if cl-cd polar is plotted during point sequence
    c   lbflap      .true. if buffer  airfoil flap parameters are defined
    c   lflap       .true. if current airfoil flap parameters are defined
    c   leiw        .true. if unit circle complex number array is initialized
    c   lscini      .true. if old-airfoil circle-plane arc length s[w] exists
    c   lforef      .true. if cl,cd... data is to be plotted on cp vs x/c plots
    c   lnorm       .true. if input buffer airfoil is to be normalized
    c   lgsame      .true. if current and buffer airfoils are identical
    c
    c   lplcam      .true. if thickness and camber are to be plotted
    c   lqsym       .true. if symmetric qspec will be enforced
    c   lgsym       .true. if symmetric geometry will be enforced
    c   lqgrid      .true. if grid is to overlaid on qspec[s] plot
    c   lggrid      .true. if grid is to overlaid on buffer airfoil geometry plot
    c   lgtick      .true. if node tick marks are to be plotted on buffer airfoil
    c   lqslop      .true. if modified qspec[s] segment is to match slopes
    c   lgslop      .true. if modified geometry segment is to match slopes
    c   lcslop      .true. if modified camber line segment is to match slopes
    c   lqsppl      .true. if current qspec[s] in in plot
    c   lgeopl      .true. if current geometry in in plot
    c   lcpgrd      .true. if grid is to be plotted on cp plots
    c   lblgrd      .true. if grid is to be plotted on bl variable plots
    c   lblsym      .true. if symbols are to be plotted on bl variable plots
    c   lcminp      .true. if min cp is to be written to polar file for cavitation
    c   lhmomp      .true. if hinge moment is to be written to polar file
    c
    c   lpgrid      .true. if polar grid overlay is enabled
    c   lpcdw       .true. if polar cdwave is plotted
    c   lplist      .true. if polar listing lines [at top of plot] are enabled
    c   lplegn      .true. if polar legend is enabled
    c
    c   lplot       .true. if plot page is open
    c   lsym        .true. if symbols are to be plotted in qdes routines
    c   liqset      .true. if inverse target segment is marked off in qdes
    c   lclip       .true. if line-plot clipping is to be performed
    c   lvlab       .true. if label is to be plotted on viscous-variable plots
    c   lcurs       .true. if cursor input is to be used for blowups, etc.
    c   lland       .true. if landscape orientation for postscript is used
    c
    c
    c   xb[.],yb[.] buffer airfoil coordinate arrays
    c   xbp[.]      dxb/dsb
    c   ybp[.]      dyb/dsb
    c   sb[.]       spline parameter for buffer airfoil
    c   snew[.]     new panel endpoint arc length array
    c
    c   xbf,ybf     buffer  airfoil flap hinge coordinates
    c   xof,yof     current airfoil flap hinge coordinates
    c   hmom        moment of flap about hinge point
    c   hfx         x-force of flap on hinge point
    c   hfy         y-force of flap on hinge point
    c
    c~~~~~~~~~~~~~~ properties of current buffer airfoil
    c
    c   xbmin,xbmax  limits of xb array
    c   ybmin,ybmax  limits of yb array
    c   sble         le tangency-point sb location
    c   chordb       chord
    c   areab        area
    c   radble       le radius
    c   angbte       te angle  [rad]
    c
    c   ei11ba       bending inertia about axis 1    x^2 dx dy
    c   ei22ba       bending inertia about axis 2    y^2 dx dy
    c   apx1ba       principal axis 1 angle
    c   apx2ba       principal axis 2 angle
    c
    c   ei11bt       bending inertia about axis 1    x^2 t ds
    c   ei22bt       bending inertia about axis 2    y^2 t ds
    c   apx1bt       principal axis 1 angle
    c   apx2bt       principal axis 2 angle
    c
    c   thickb       max thickness
    c   cambrb       max camber
    c
    c~~~~~~~~~~~~~~
    c
    c   xssi[..]    bl arc length coordinate array on each surface
    c   uedg[..]    bl edge velocity array
    c   uinv[..]    bl edge velocity array without mass defect influence
    c   mass[..]    bl mass defect array  [ = uedg*dstr ]
    c   thet[..]    bl momentum thickness array
    c   dstr[..]    bl displacement thickness array
    c   ctau[..]    sqrt[max shear coefficient] array
    c               [in laminar regions, log of amplification ratio]
    c
    c   tau[..]     wall shear stress array                 [for plotting only]
    c   dis[..]     dissipation array                       [for plotting only]
    c   ctq[..]     sqrt[equilibrium max shear coefficient] array [  "  ]
    c   vti[..]     +/-1 conversion factor between panel and bl variables
    c   uinv_a[..]  duinv/dalfa array
    c
    c   reinf1      reynolds number  vinf c / ve  for cl=1
    c   reinf       reynolds number for current cl
    c   reinf_cl    dreinf/dcl
    c
    c   acrit       log [critical amplification ratio]
    c   xstrip[.]   transition trip  x/c locations [if xtrip > 0],
    c               transition trip -s/s_side locations [if xtrip < 0],
    c   xoctr[.]    actual transition x/c locations
    c   yoctr[.]    actual transition y/c locations
    c   xssitr[.]   actual transition xi locations
    c
    c   iblte[.]    bl array index at trailing edge
    c   nbl[.]      max bl array index
    c   ipan[..]    panel index corresponding to bl location
    c   isys[..]    bl newton system line number corresponding to bl location
    c   nsys        total number of lines in bl newton system
    c   itran[.]    bl array index of transition interval
    c   tforce[.]   .true. if transition is forced due to transition strip
    c
    c   va,vb[...]  diagonal and off-diagonal blocks in bl newton system
    c   vz[..]      way-off-diagonal block at te station line
    c   vm[...]     mass-influence coefficient vectors in bl newton system
    c   vdel[..]    residual and solution vectors in bl newton system
    c
    c   rmsbl       rms change from bl newton system solution
    c   rmxbl       max change from bl newton system solution
    c   imxbl       location of max change
    c   ismxbl      index of bl side containing max change
    c   vmxbl       character identifying variable with max change
    c   rlx         underrelaxation factor for newton update
    c   vaccel      parameter for accelerating bl newton system solution
    c               [any off-diagonal element < vaccel is not eliminated,
    c                which speeds up each iteration, but may increase
    c                iteration count]
    c                can be set to zero for unadulterated newton method
    c
    c   xoff,yoff   x and y offsets for windowing in qdes,gdes routines
    c   xsf ,ysf    x and y scaling factors for windowing in qdes,gdes routines
    c
    c   xgmin       airfoil grid plot limits
    c   xgmax
    c   ygmin
    c   ygmax
    c   dxyg        airfoil grid-plot annotation increment
    c   gtick       airfoil-plot tick marks size [as fraction of arc length]
    */


        //    complex<double> zcoldw, dzte, chordz, zleold, zc, zc_cn, piq, cn, eiw;

        //----- CIRCLE.INC include file for circle-plane operations
        //   NC         number of circle plane points, must be 2**n + 1
        //   MC         number of Fourier harmonics of P(w) + iQ(w)
        //   MCT        number of Fourier harmonics for which dZC/dCN are calculated
        //
        //   PI         3.1415926
        //   AGTE       trailing edge angle/pi
        //   AG0        angle of airfoil surface at first point
        //   QIM0       Q(w) offset   = Q(0)
        //   QIMOLD     Q(w) offset for old airfoil
        //   DWC        increment of circle-plane coordinate w,  DWC = 2 pi/(NC-1)
        //   WC(.)      circle plane coordinate w for Fourier operations
        //   SC(.)      normalized arc length array s(w)
        //   SCOLD(.)   normalized arc length s(w) of old airfoil
        //   XCOLD(.)   x coordinate x(w) of old airfoil
        //   YCOLD(.)   y coordinate y(w) of old airfoil
        //
        //   DZTE       trailing edge gap specified in the complex plane
        //   CHORDZ     airfoil chord specified in the complex plane
        //   ZLEOLD     leading edge of old airfoil
        //   ZCOLDW(.)  d(x+iy)/dw of old airfoil
        //   ZC(.)      complex airfoil coordinates derived from P(w) + iQ(w)
        //   ZC_CN(..)  sensitivities dZC/dCN for driving geometry constraints
        //   PIQ(.)     complex harmonic function P(w) + iQ(w)
        //   CN(.)      Fourier coefficients of P(w) + iQ(w)
        //   EIW(..)    complex number  exp(inw)  array on the unit circle

        //-----End Specific Inverse MDES-------------------------------
};

