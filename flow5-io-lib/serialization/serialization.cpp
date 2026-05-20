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

#define _MATH_DEFINES_DEFINED

#include <format>



#include <QDebug>
#include <QtCore>


#include <TopoDS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

#include <api/aeroforces.h>
#include <api/bezierspline.h>
#include <api/bldata.h>
#include <api/boat.h>
#include <api/boatopp.h>
#include <api/boatpolar.h>
#include <api/bspline.h>
#include <api/cubicspline.h>
#include <api/edgesplit.h>
#include <api/eigenvalues.h>
#include <api/extradrag.h>
#include <api/foil.h>
#include <api/frame.h>
#include <api/fuse.h>
#include <api/fuseflatfaces.h>
#include <api/fusenurbs.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/linestyle.h>
#include <api/naca4spline.h>
#include <api/occ_globals.h>
#include <api/oppoint.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/planepolarext.h>
#include <api/planestl.h>
#include <api/planexfl.h>
#include <api/pointspline.h>
#include <api/polar.h>
#include <api/polar3d.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailocc.h>
#include <api/sailspline.h>
#include <api/sailstl.h>
#include <api/sailwing.h>
#include <api/serialization.h>
#include <api/spandistribs.h>
#include <api/splinefoil.h>
#include <api/utils.h>
#include <api/utils-io.h>
#include <api/vorton.h>
#include <api/wingopp.h>
#include <api/wingsailsection.h>
#include <api/wingxfl.h>

#include <flow5-io.h>


bool serial::serializePolarFl5(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    int nIntSpares(0);
    int nDbleSpares(0);

    int nVariables(0);
    int m(0), n(0);

    double dble(0);

    QString strange;

    int ArchiveFormat(0);// identifies the format of the file
    // 500001 : initialization of the new fl5 format
    // 500750 : v7.50

    ArchiveFormat = 500750;

    if(bIsStoring)
    {
        ar << ArchiveFormat; // first format for XFL file

        ar << QString::fromStdString(pPolar->foilName());
        ar << QString::fromStdString(pPolar->name());

        serial::serializeLineStyleFl5(pPolar->theStyle(), ar, bIsStoring);

        switch(pPolar->type())
        {
            default:
            case xfl::T1POLAR:  ar<<1;  break;
            case xfl::T2POLAR:  ar<<2;  break;
            case xfl::T3POLAR:  ar<<3;  break;
            case xfl::T4POLAR:  ar<<4;  break;
            case xfl::T6POLAR:  ar<<6;  break;
        }

            switch(pPolar->BLMethod())
        {
            default:
            case BL::XFOIL:         n=0;  break;
            case BL::NOBLMETHOD:    n=4;  break;
        }
        ar << n;

        ar << pPolar->MaType() << pPolar->ReType();

        /*        ar << m_Density << m_nu;
        ar << m_QInf;
        ar << m_Chord;*/
        ar << pPolar->Reynolds();
        ar << pPolar->Mach();
        ar << pPolar->aoaSpec();
        ar << pPolar->XTripTop() << pPolar->XTripBot();
        ar << pPolar->NCrit();

        //        ar << nCtrls();
        //        for (int ic=0; ic<Polar::nCtrls(); ic++) ar << dble<<dble;
        ar << nVariables; // formerly nCtrls

        nVariables = 12; // change to add new variables
        ar << nVariables;
        ar << int(pPolar->dataSize());
        for (int l=0; l<pPolar->dataSize(); l++)
        {
            ar << float(pPolar->m_Alpha.at(l)) << float(pPolar->m_Cd.at(l));
            ar << float(pPolar->m_Cdp.at(l))   << float(pPolar->m_Cl.at(l)) << float(pPolar->m_Cm.at(l));
            ar << float(pPolar->m_HMom.at(l))  << float(pPolar->m_Cpmn.at(l));
            ar << float(pPolar->m_Re.at(l));
            ar << float(pPolar->m_XCp.at(l));
            ar << float(pPolar->m_Control.at(l));
            ar << float(pPolar->m_XTrTop.at(l)) << float(pPolar->m_XTrBot.at(l));
            ar << float(pPolar->m_XLamSepTop.at(l)) << float(pPolar->m_XLamSepBot.at(l));
            ar << float(pPolar->m_XTurbSepTop.at(l)) << float(pPolar->m_XTurbSepBot.at(l));
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=1;
        ar << nDbleSpares;
        ar << pPolar->TEFlapAngle();

        return true;
    }
    else
    {
        //read variables
        float Alpha, Cd(0), Cdp(0), Cl(0), Cm(0), HMom(0), Cpmn(0), Re(0), XCp(0), Ctrl(0);
        float XTr1(0), XTr2(0), XLSTop(0), XLSBot(0), XTSTop(0), XTSBot(0);

        ar >> ArchiveFormat;
        if ((ArchiveFormat<500000) || (ArchiveFormat>501000)) return false;

        ar >> strange;    pPolar->setFoilName(strange.toStdString());
        ar >> strange;    pPolar->setName(strange.toStdString());

        LineStyle ls;
        serial::serializeLineStyleFl5(ls, ar, bIsStoring);
        pPolar->setTheStyle(ls);

        xfl::enumPolarType type;
        ar >> n;
        switch (n)
        {
            default:
            case 1: type=xfl::T1POLAR;    break;
            case 2: type=xfl::T2POLAR;    break;
            case 3: type=xfl::T3POLAR;    break;
            case 4: type=xfl::T4POLAR;    break;
            case 5:
            case 6: type=xfl::T6POLAR;    break;
        }
            pPolar->setType(type);


        BL::enumBLMethod method;
        ar >>n;
        switch(n)
        {
            case 0: method=BL::XFOIL;         break;
            default:
            case 4: method=BL::NOBLMETHOD;    break;
        }

            pPolar->setBLMethod(method);

        ar >> m >> n;
            pPolar->setMaType(m);
        pPolar->setReType(n);

        if(ArchiveFormat<500750)
        {
            double qinf(0), rho(0), nu(0), chord(0);
            ar >> rho >> nu;
            ar >> qinf; // m_QInf;
            ar >> chord; // m_Chord;
            pPolar->setReynolds(qinf*chord/nu);
        }
        else
        {
            ar >> dble; pPolar->setReynolds(dble);
        }
        ar >> dble; pPolar->setMach(dble);
        ar >> dble; pPolar->setAoaSpec(dble);
        ar >> dble; pPolar->setXTripTop(dble);
        ar >> dble; pPolar->setXTripBot(dble);

        ar >> dble; pPolar->setNCrit(dble);

        ar >> n; // formerly nCtrls;
        double d1(0), d2(0);
        for (int ic=0; ic<n; ic++)
        {
            ar>>d1>>d2;
        }

        ar >> nVariables;
        ar >> n;
        for (int i=0; i< n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm >> HMom >> Cpmn >> Re >> XCp >> Ctrl;
            ar >> XTr1 >> XTr2 >> XLSTop >> XLSBot >> XTSTop >>XTSBot;
            pPolar->addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(HMom), double(Cpmn),
                           double(Re), double(XCp), double(Ctrl),
                           double(XTr1), double(XTr2), double(XLSTop), double(XLSBot), double(XTSTop), double(XTSBot));
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        if(nDbleSpares>0)
            ar >> pPolar->m_TEFlapAngle;

        // correct past errors
        if(pPolar->isType6()) pPolar->m_TEFlapAngle = 0.0;
    }
    return true;
}



bool serial::serializePolarXFL(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    double dble(0.0);
    bool boolean(false);
    int i(0), k(0), n(0);
    QString strange;

    int ArchiveFormat(0);// identifies the format of the file
    // 100005 : added the array of control values
    ArchiveFormat = 100005;

    if(bIsStoring)
    {
        return true;
    }
    else
    {
        //read variables
        float Alpha(0), Cd(0), Cdp(0), Cl(0), Cm(0), XTr1(0), XTr2(0), HMom(0), Cpmn(0), Re(0), XCp(0), Ctrl(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat <100000 || ArchiveFormat>110000) return false;

        ar >> strange;   pPolar->setFoilName(strange.toStdString());
        ar >> strange;   pPolar->setName(strange.toStdString());
        if(ArchiveFormat<100005)
        {
            int s(0);
            int w(0);
            ar >>s>>w;
            pPolar->setLineStipple(LineStyle::convertLineStyle(s));
            pPolar->setLineWidth(w);
            int r(0),g(0),b(0),a(0);
            io::readColor(ar, r,g,b,a);
            pPolar->setLineColor(fl5Color(r,g,b,a));
            ar >> boolean; pPolar->setVisible(boolean);
            ar >>  boolean;
        }
        else
        {
            LineStyle ls;
            serial::serializeLineStyleXfl(ls, ar, bIsStoring);
            pPolar->setTheStyle(ls);
        }


        ar >> n;
        if     (n==2) pPolar->setType(xfl::T2POLAR);
        else if(n==3) pPolar->setType(xfl::T3POLAR);
        else if(n==4) pPolar->setType(xfl::T4POLAR);
        else if(n==5) pPolar->setType(xfl::T6POLAR);
        else pPolar->setType(xfl::T1POLAR);

        ar >> n; pPolar->setMaType(n);
        ar >> n; pPolar->setReType(n);

        ar >> dble; pPolar->setReynolds(dble);
        ar >> dble; pPolar->setMach(dble);

        ar >> dble; pPolar->setAoaSpec(dble);
        ar >> dble; pPolar->setXTripTop(dble);
        ar >> dble; pPolar->setXTripBot(dble);
        ar >> dble; pPolar->setNCrit(dble);

        ar >> n;

        for (i=0; i<n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm >> XTr1 >> XTr2 >> HMom >> Cpmn >> Re >> XCp;

            pPolar->addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(HMom), double(Cpmn),
                     double(Re), double(XCp), double(Ctrl), double(XTr1), double(XTr2),
                     0,0,0,0);
        }
        if(ArchiveFormat<100005)
        {
            ar >> n;
            pPolar->theStyle().m_Symbol=LineStyle::convertSymbol(n);
        }

        // space allocation
        for (int i=0; i<19; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }


    return true;
}


bool serial::serializeFrameFl5(Frame *pFrame, QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat=500001; //500001 : first xf7 format
    int nIntSpares(0);
    int nDbleSpares(0);
    int n(0);
    double dble(0);

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << pFrame->position().x << pFrame->position().y << pFrame->position().z;
        ar << pFrame->angle();
        ar << pFrame->nCtrlPoints();
        for(int k=0; k<pFrame->nCtrlPoints(); k++)
        {
            ar << pFrame->ctrlPoint(k).x << pFrame->ctrlPoint(k).y << pFrame->ctrlPoint(k).z;
        }
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

        Vector3d pos;
        ar >> pos.x >> pos.y >> pos.z;
        pFrame->setPosition(pos);
        ar >> dble;
        pFrame->setAngle(dble);
        int nPts=0;
        ar >> nPts;
        pFrame->clearCtrlPoints();
        double dx(0), dy(0), dz(0);
        for(int k=0; k<nPts; k++)
        {
            ar >> dx >> dy >> dz;
            pFrame->appendCtrlPoint({dx, dy, dz});
        }
        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool serial::serializeFrameXfl(Frame *pFrame, QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat(0);
    int k(0),n(0);
    float fx(0), fy(0), fz(0);

    if(bIsStoring)
    {
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000 || ArchiveFormat>1100) return false;
        ar >> n;
        pFrame->clearCtrlPoints();
        for(k=0; k<n; k++)
        {
            ar >> fx;
            ar >> fy;
            ar >> fz;
            pFrame->appendCtrlPoint({fx, fy, fz});
        }
    }
    return true;
}


bool serial::serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    int n(0), l(0), k(0);
    int ArchiveFormat(0);// identifies the format of the file
    float f(0);

    if(bIsStoring)
    {
        //write variables
        n = pPolar->dataSize();

        ar << 1005; // identifies the format of the file
        // 1005: added Trim Polar parameters
        // 1004: added XCp
        // 1003: re-instated NCrit, XTopTr and XBotTr with polar
        io::writeString(ar, pPolar->m_FoilName);
        io::writeString(ar, pPolar->name());

        if     (pPolar->isFixedSpeedPolar())  ar<<1;
        else if(pPolar->isFixedLiftPolar())   ar<<2;
        else if(pPolar->isRubberChordPolar()) ar<<3;
        else if(pPolar->isFixedaoaPolar())    ar<<4;
        else if(pPolar->isControlPolar())     ar<<5;
        else                                  ar<<1;

        ar << pPolar->m_MaType << pPolar->m_ReType;
        ar << int(pPolar->Reynolds()) << float(pPolar->m_Mach);
        ar << float(pPolar->m_aoaSpec);
        ar << n << float(pPolar->m_ACrit);
        ar << float(pPolar->m_XTripTop) << float(pPolar->m_XTripBot);
        io::writeColor(ar, pPolar->lineColor().red(), pPolar->lineColor().green(), pPolar->lineColor().blue());

        ar << pPolar->theStyle().m_Stipple << pPolar->theStyle().m_Width;
        if (pPolar->isVisible())  ar<<1; else ar<<0;
        ar<<pPolar->pointStyle();

        for (int i=0; i<pPolar->dataSize(); i++)
        {
            ar << float(pPolar->m_Alpha.at(i))  << float(pPolar->m_Cd.at(i)) ;
            ar << float(pPolar->m_Cdp.at(i))    << float(pPolar->m_Cl.at(i)) << float(pPolar->m_Cm.at(i));
            ar << float(pPolar->m_XTrTop.at(i)) << float(pPolar->m_XTrBot.at(i));
            ar << float(pPolar->m_HMom.at(i))   << float(pPolar->m_Cpmn.at(i));
            ar << float(pPolar->m_Re.at(i));
            ar << float(pPolar->m_XCp.at(i));
            ar << float(pPolar->m_Control.at(i));
        }

        ar << pPolar->m_ACrit << pPolar->m_XTripTop << pPolar->m_XTripBot;

        /*        for(int i=0; i<pPolar->nCtrls(); i++)
        {
            ar<<dble<<dble;
        }*/

        return true;
    }
    else
    {
        //read variables
        std::string strange;
        float Alpha=0, Cd(0), Cdp(0), Cl(0), Cm(0), XTr1(0), XTr2(0), HMom(0), Cpmn(0), Re(0), XCp(0);
        int iRe(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat <1001 || ArchiveFormat>1100)
        {
            return false;
        }

        io::readString(ar, strange); pPolar->setFoilName(strange);
        io::readString(ar, strange); pPolar->setName(strange);

        if(pPolar->m_FoilName.length()==0 || pPolar->name().length()==0)
        {
            return false;
        }

        ar >>k;
        if     (k==1) pPolar->m_Type = xfl::T1POLAR;
        else if(k==2) pPolar->m_Type = xfl::T2POLAR;
        else if(k==3) pPolar->m_Type = xfl::T3POLAR;
        else if(k==4) pPolar->m_Type = xfl::T4POLAR;
        else          pPolar->m_Type = xfl::T1POLAR;


        ar >> pPolar->m_MaType >> pPolar->m_ReType;

        if(pPolar->m_MaType!=1 && pPolar->m_MaType!=2 && pPolar->m_MaType!=3)
        {
            return false;
        }
        if(pPolar->m_ReType!=1 && pPolar->m_ReType!=2 && pPolar->m_ReType!=3)
        {
            return false;
        }

        ar >> iRe;
        pPolar->setReynolds(double(iRe));
        ar >> f; pPolar->m_Mach = double(f);

        ar >> f; pPolar->m_aoaSpec= double(f);

        ar >> n;
        ar >> f; pPolar->m_ACrit    = double(f);
        ar >> f; pPolar->m_XTripTop = double(f);
        ar >> f; pPolar->m_XTripBot = double(f);

        if(ArchiveFormat<1005)
        {
            int r(0),g(0),b(0);
            io::readColor(ar, r, g, b);
            pPolar->setLineColor(fl5Color(r, g, b));
            ar >>n;
            pPolar->setLineStipple(LineStyle::convertLineStyle(n));
            ar >> n; pPolar->setLineWidth(n);
            if(ArchiveFormat>=1002)
            {
                ar >> l;
                if(l!=0 && l!=1 )
                {
                    return false;
                }
                if (l) pPolar->setVisible(true); else pPolar->setVisible(false);
            }
            ar >> l;  pPolar->setPointStyle(LineStyle::convertSymbol(l));
        }
        else serial::serializeLineStyleXfl(pPolar->theStyle(), ar, bIsStoring);

        bool bExists=false;
        for (int i=0; i< n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm;
            ar >> XTr1 >> XTr2;
            ar >> HMom >> Cpmn;

            if(ArchiveFormat >=4) ar >> Re;
            else                  Re = float(pPolar->Reynolds());

            if(ArchiveFormat>=1004) ar>> XCp;
            else                    XCp = 0.0;

            bExists = false;
            if(pPolar->m_Type!=xfl::T4POLAR)
            {
                for (int j=0; j<pPolar->dataSize(); j++)
                {
                    if(fabs(double(Alpha)-pPolar->m_Alpha.at(j))<0.001)
                    {
                        bExists = true;
                        break;
                    }
                }
            }
            else
            {
                for (uint j=0; j<pPolar->m_Re.size(); j++)
                {
                    if(fabs(double(Re)-pPolar->m_Re.at(j))<0.1)
                    {
                        bExists = true;
                        break;
                    }
                }
            }
            if(!bExists)
            {
                pPolar->addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(HMom),
                                 double(Cpmn), double(Re), double(XCp), 0.0, double(XTr1), double(XTr2), 0,0,0,0);
            }
        }
        if(ArchiveFormat>=1003)
            ar >>pPolar->m_ACrit >> pPolar->m_XTripTop >> pPolar->m_XTripBot;
    }
    return true;
}


bool serial::serializeFoil(Foil *pFoil, QDataStream &ar)
{
    // saves or loads the foil to the archive ar

    int ArchiveFormat = 1007;
    // 1007 : saved hinge positions is absolute values rather than %
    // 1006 : QFLR5 v0.02 : added Foil description
    // 1005 : added LE Flap data
    // 1004 : added Points and Centerline property
    // 1003 : added Visible property
    // 1002 : added color and style save
    // 1001 : initial format
    int p(0), j(0);
    float f(0), ff(0);

    float xh(0), yh(0), angle(0);

    bool bIsStoring = false;

    if(bIsStoring)
    {
        // deprecated
        assert(false);
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000||ArchiveFormat>1010)
            return false;

        std::string strange;
        io::readString(ar, strange);
        pFoil->setName(strange);
        if(ArchiveFormat>=1006)
        {
            io::readString(ar, strange);
            pFoil->setDescription(strange);
        }
        if(ArchiveFormat>=1002)
        {
            ar >> p;
            pFoil->setLineStipple(LineStyle::convertLineStyle(p));
            ar >> p; pFoil->setLineWidth(p);
            int r=0,g=0,b=0;
            io::readColor(ar, r, g, b);
            pFoil->setLineColor(fl5Color(r,g,b));
        }
        if(ArchiveFormat>=1003)
        {
            ar >> p;
            if(p) pFoil->setVisible(true); else pFoil->setVisible(false);
        }
        if(ArchiveFormat>=1004)
        {
            ar >> p;
            pFoil->setPointStyle(LineStyle::convertSymbol(p));
            ar >> p;
            pFoil->showCamberLine(p);
            //            if(p) pFoil->m_bCamberLine = true; else pFoil->m_bCamberLine = false;
        }

        if(ArchiveFormat>=1005)
        {
            ar >> p;
            ar >> angle;
            ar >> xh;
            ar >> yh;
            pFoil->setLEFlapData(p, xh, yh, angle);
        }
        ar >> p;
        ar >> angle;
        ar >> xh;
        ar >> yh;
        pFoil->setTEFlapData(p, xh, yh, angle);

        if(ArchiveFormat<1007)
        {
            pFoil->scaleHingeLocations();
        }

        ar >> f >> f >> f; //formerly transition parameters
        ar >> p;
        //        if(pFoil->nb()>IBX) return false;

        std::vector<Node2d> basenodes(p);
        //        pFoil->resizePointArrays(p);
        for (j=0; j<p; j++)
        {
            ar >> f >> ff;
            basenodes[j].x = double(f);
            basenodes[j].y = double(ff);
        }

        pFoil->setBaseNodes(basenodes);

        /** @todo remove. We don't need to save/load the current foil geom
         *  since we recreate it using base geometry and flap data */
        if(ArchiveFormat>=1001)
        {
            ar >> p; //pFoil->n;
            //            if(pFoil->n>IBX) return false;

            if(p>pFoil->nNodes())
            {
                //                pFoil->m_Node.resize(p);
                //                pFoil->resizeArrays(p);
            }
            for (j=0; j<p; j++)
            {
                ar >> f >> ff;
                //                pFoil->x[j]=f; pFoil->y[j]=ff;
            }
            if(pFoil->nBaseNodes()==0 && pFoil->nNodes()!=0)
            {
                //                pFoil->nb = pFoil->n();
                //                pFoil->xb= pFoil->x;/** @todo is this an array copy?*/
                //                pFoil->yb= pFoil->y;
            }
        }
        else
        {
            //            pFoil->x= pFoil->xb; /** @todo is this an array copy?*/
            //            pFoil->y= pFoil->yb;
            //            pFoil->n=pFoil->nb;
        }


        pFoil->initGeometry();

        return true;
    }
}


bool serial::serializeOppXFL(OpPoint* pOpp, QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    bool boolean(false);
    int k(0);
    float f0(0), f1(0);
    double dble(0);

    QString strange;

    if(bIsStoring)
    {
    }
    else
    {
        ar >> ArchiveFormat;
        //write variables
        ar >> strange;  pOpp->setFoilName(strange.toStdString());
        ar >> strange;  pOpp->setPolarName(strange.toStdString());

        if(ArchiveFormat<200005)
        {
            ar >> k;
            pOpp->theStyle().setStipple(k);
            ar >> k;
            pOpp->theStyle().setWidth(k);
            pOpp->theStyle().setColor(io::readQColor(ar));
            ar >> boolean; pOpp->theStyle().setVisible(boolean);
            ar >> boolean;
        }
        else
            serial::serializeLineStyleXfl(pOpp->theStyle(), ar, bIsStoring);

        int m_n(0);
        ar >> pOpp->m_Reynolds >> pOpp->m_Mach >> pOpp->m_Alpha;
        ar >> m_n >> pOpp->m_BLXFoil.nd1 >> pOpp->m_BLXFoil.nd2 >> pOpp->m_BLXFoil.nd3;

        ar >> pOpp->m_bViscResults;
        ar >> pOpp->m_bBL;

        ar >> pOpp->m_Cl >> pOpp->m_Cm >> pOpp->m_Cd >> pOpp->m_Cdp;
        ar >> pOpp->m_XTrTop >> pOpp->m_XTrBot >> pOpp->m_XCP;
        ar >> pOpp->m_NCrit >> pOpp->m_TEHMom >> pOpp->m_Cpmn;

        pOpp->m_Cpv.resize(m_n);
        pOpp->m_Cpi.resize(m_n);
        pOpp->m_Qv.resize(m_n);
        pOpp->m_Qi.resize(m_n);
        for (k=0; k<m_n; k++)
        {
            ar >> f0 >> f1;
            pOpp->m_Cpv[k] = f0;
            pOpp->m_Cpi[k] = f1;
        }
        for (k=0; k<m_n; k++)
        {
            ar >> f0 >> f1;
            pOpp->m_Qv[k] = double(f0);
            pOpp->m_Qi[k] = double(f1);
        }
        for (k=0; k<=pOpp->m_BLXFoil.nd1; k++)
        {
            ar >> f0 >> f1;
            pOpp->m_BLXFoil.xd1[k] = double(f0);
            pOpp->m_BLXFoil.yd1[k] = double(f1);
        }
        for (k=0; k<pOpp->m_BLXFoil.nd2; k++)
        {
            ar >> f0 >> f1;
            pOpp->m_BLXFoil.xd2[k] = double(f0);
            pOpp->m_BLXFoil.yd2[k] = double(f1);
        }
        for (k=0; k<pOpp->m_BLXFoil.nd3; k++)
        {
            ar >> f0 >> f1;
            pOpp->m_BLXFoil.xd3[k] = double(f0);
            pOpp->m_BLXFoil.yd3[k] = double(f1);
        }

        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}


bool serial::serializeOppFl5(OpPoint* pOpp, QDataStream &ar, bool bIsStoring)
{
    double dble=0.0;
    int nIntSpares=0;
    int nDbleSpares=0;

    int n(0), k(0);
    float f0(0), f1(0);
    bool boolean(false);

    QString strange;

    // 500001: first fl5 format
    // 500002: restored Cpv and Cpi
    // 500003: added surface nodes
    // 500004: restored BLXFoil save
    // 500750: v750, added theta
    int ArchiveFormat = 500750;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        //write variables
        ar << QString::fromStdString(pOpp->m_FoilName);
        ar << QString::fromStdString(pOpp->m_PlrName);

        serial::serializeLineStyleFl5(pOpp->theStyle(), ar, bIsStoring);

        switch(pOpp->m_BLMethod)
        {
            default:
            case BL::XFOIL:         n=0;  break;
        }
        ar << n;

        ar << pOpp->m_Reynolds << pOpp->m_Mach << pOpp->m_Alpha;

        ar << pOpp->m_Theta;

        ar << pOpp->m_bViscResults;
        ar << pOpp->m_bBL;

        ar << pOpp->m_Cl << pOpp->m_Cm << pOpp->m_Cd << pOpp->m_Cdp;
        ar << pOpp->m_XTrTop << pOpp->m_XTrBot << pOpp->m_XCP;
        ar << pOpp->m_NCrit << pOpp->m_TEHMom << pOpp->m_Cpmn;

        n = 0;
        ar << n; //int(m_Node.size());
        /*        for(int l=0; l<m_Node.size(); l++)
        {
            Node2d const &n2d = m_Node.at(l);
            ar << n2d.index() << n2d.isWakeNode() <<n2d.xf() << n2d.yf() << n2d.normal().xf() << n2d.normal().yf();
        }*/

        ar << int(pOpp->m_Qi.size());
        for (uint l=0; l<pOpp->m_Qi.size(); l++)     ar << float(pOpp->m_Cpv[l]) << float(pOpp->m_Cpi[l]);
        for (uint l=0; l<pOpp->m_Qi.size(); l++)     ar << float(pOpp->m_Qv[l])  << float(pOpp->m_Qi[l]);

        if(pOpp->m_BLMethod==BL::XFOIL)
        {
            serial::serializeBLXFoil(&pOpp->m_BLXFoil, ar, bIsStoring);
        }
        else
        {
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>550000) return false;

        ar >> strange; pOpp->m_FoilName = strange.toStdString();
        ar >> strange; pOpp->m_PlrName = strange.toStdString();;

        serial::serializeLineStyleFl5(pOpp->theStyle(), ar, bIsStoring);


        ar >>n;
        switch(n)
        {
            default:
            case 0: pOpp->m_BLMethod=BL::XFOIL;         break;
        }

        ar >> pOpp->m_Reynolds >> pOpp->m_Mach >> pOpp->m_Alpha;

        if(ArchiveFormat>=500750)
            ar >> pOpp->m_Theta;

        ar >> pOpp->m_bViscResults;
        ar >> pOpp->m_bBL;

        ar >> pOpp->m_Cl >> pOpp->m_Cm >> pOpp->m_Cd >> pOpp->m_Cdp;
        ar >> pOpp->m_XTrTop >> pOpp->m_XTrBot >> pOpp->m_XCP;
        ar >> pOpp->m_NCrit >> pOpp->m_TEHMom >> pOpp->m_Cpmn;

        if(ArchiveFormat>=500003)
        {
            ar >> n;
            //            m_Node.resize(n);
            for(int l=0; l<n; l++)
            {
                //                Node2d &n2d = m_Node[l];
                ar >> k;        //  n2d.setIndex(k);
                ar >> boolean;  //  n2d.setWakeNode(boolean);
                ar >> f0 >> f1; //  n2d.set(f0, f1);
                ar >> f0 >> f1; //  n2d.setNormal(f0, f1);
            }
        }

        ar >> n;
        pOpp->resizeSurfacePoints(n);
        if(ArchiveFormat>=500002)
        {
            for (int l=0; l<n; l++)
            {
                ar >> f0 >> f1;
                pOpp->m_Cpv[l] = double(f0);
                pOpp->m_Cpi[l] = double(f1);
            }
        }

        for (int l=0; l<n; l++)
        {
            ar >> f0 >> f1;
            pOpp->m_Qv[l] = double(f0);
            pOpp->m_Qi[l] = double(f1);
        }

        if(ArchiveFormat>=500004)
        {
            if(pOpp->m_BLMethod==BL::XFOIL)
            {
                serial::serializeBLXFoil(&pOpp->m_BLXFoil, ar, bIsStoring);
            }
            else
            {
            }
        }
        else
        {
            // unused but need to read nonetheless
            BLData bl;
            serial::serializeBLDataFl5(bl, ar, bIsStoring); // top
            serial::serializeBLDataFl5(bl, ar, bIsStoring); // bot
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool serial::serializeFoilXfl(Foil *pFoil, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int k(0);
    double f(0), g(0), h(0);

    int ArchiveFormat = 100007;
    // 100006 : first version of new xfl format
    // 100007: foil new style
    QString strange;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(pFoil->name());
        ar << QString::fromStdString(pFoil->description());

        serial::serializeLineStyleXfl(pFoil->theStyle(), ar, bIsStoring);

        ar << pFoil->isCamberLineVisible() << pFoil->hasLEFlap() << pFoil->hasTEFlap();
        ar << pFoil->LEFlapAngle() << pFoil->LEXHinge()*100.0 << pFoil->LEYHinge()*100.0;
        ar << pFoil->TEFlapAngle() << pFoil->TEXHinge()*100.0 << pFoil->TEYHinge()*100.0;
        ar << pFoil->nBaseNodes();
        for (int j=0; j<pFoil->nBaseNodes(); j++)
        {
            ar << pFoil->basePoint(j).x << pFoil->basePoint(j).y;
        }
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        ar >> strange;   pFoil->setName(strange.toStdString());
        ar >> strange;   pFoil->setDescription(strange.toStdString());

        if(ArchiveFormat<100007)
        {
            ar >> k; pFoil->theStyle().setStipple(k);
            ar >> k; pFoil->theStyle().setWidth(k);

            serial::serializefl5Color(pFoil->theStyle().m_Color, ar, false);
            ar >> pFoil->theStyle().m_bIsVisible;

            qint8 b = 0x00;
            ar >> b; pFoil->theStyle().setPointStyle(LineStyle::convertSymbol(int(b)));
        }
        else
            serial::serializeLineStyleXfl(pFoil->theStyle(), ar, bIsStoring);

        ar >> boolean; pFoil->showCamberLine(boolean);
        ar >> boolean; pFoil->setLEFlap(boolean);
        ar >> boolean; pFoil->setTEFlap(boolean);

        ar >>f >>g >>h;
        pFoil->setLEFlapAngle(f);
        pFoil->setLEXHinge(g/100.0);
        pFoil->setLEYHinge(h/100.0);

        ar >>f >>g >>h;
        pFoil->setTEFlapAngle(f);
        pFoil->setTEXHinge(g/100.0);
        pFoil->setTEYHinge(h/100.0);

        int n;
        ar >> n;
        if(n>604) return false;

        for (int jl=0; jl<n; jl++)
        {
            double x, y;
            ar>>x>>y;
            pFoil->appendBasePoint(x,y);
        }
        pFoil->initGeometry();
        return true;
    }
}


bool serial::serializeFoilFl5(Foil *pFoil, QDataStream &ar, bool bIsStoring)
{
    int kj(0), n(0);

    bool boolean(false);
//    int k(0);
    double f(0), g(0), h(0);

    QString strange;

    int ArchiveFormat = 500753;
    // 500001: first version of new fl5 format
    // 500750: v7.50 making legacy TE flaps permanent
    // 500753: v7.53 added spline bunch parameters

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(pFoil->name());
        ar << QString::fromStdString(pFoil->description());
        ar << LineStyle::convertLineStyle(pFoil->theStyle().m_Stipple);
        ar << pFoil->theStyle().m_Width;
        ar << LineStyle::convertSymbol(pFoil->theStyle().m_Symbol);
        serial::serializefl5Color(pFoil->theStyle().m_Color, ar, true);

        ar << pFoil->theStyle().m_bIsVisible;
        ar << pFoil->isCamberLineVisible() << pFoil->hasLEFlap() << pFoil->hasTEFlap();
        ar << pFoil->LEFlapAngle() << pFoil->LEXHinge() << pFoil->LEYHinge();
        ar << pFoil->TEFlapAngle() << pFoil->TEXHinge() << pFoil->TEYHinge();

        ar << pFoil->bunchAmp();
        n=0;
        switch(pFoil->bunchType())
        {
            case Spline::NOBUNCH:    n=0;    break;
            case Spline::UNIFORM:    n=1;    break;
            case Spline::SIGMOID:    n=2;    break;
            case Spline::DOUBLESIG:  n=3;    break;
        }
        ar << n;

        ar << pFoil->nBaseNodes(); // store as int!
        for (int l=0; l<pFoil->nBaseNodes(); l++)
        {
            ar <<pFoil->basePoint(l).x << pFoil->basePoint(l).y;
        }

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>550000) return false;
        ar >> strange;   pFoil->setName(strange.toStdString());
        ar >> strange;   pFoil->setDescription(strange.toStdString());
        ar >> kj;   pFoil->theStyle().m_Stipple=LineStyle::convertLineStyle(kj);
        ar >> kj;   pFoil->theStyle().setWidth(kj);
        ar >> kj;   pFoil->theStyle().m_Symbol=LineStyle::convertSymbol(kj);
        serial::serializefl5Color(pFoil->theStyle().m_Color, ar, false);
        ar >> pFoil->theStyle().m_bIsVisible;

        ar >> boolean; pFoil->showCamberLine(boolean);
        ar >> boolean; pFoil->setLEFlap(boolean);
        ar >> boolean; pFoil->setTEFlap(boolean);

        ar >>f >>g >> h;
        pFoil->setLEFlapAngle(f);
        pFoil->setLEXHinge(g);
        pFoil->setLEYHinge(h);

        ar >>f >>g >> h;
        pFoil->setTEFlapAngle(f);
        pFoil->setTEXHinge(g);
        pFoil->setTEYHinge(h);

        f = 0.0;
        Spline::enumBunch m_BunchType(Spline::NOBUNCH);
        if(ArchiveFormat>=500753)
        {
            ar >> f;

            ar >> n;
            switch(n)
            {
                case 0: m_BunchType = Spline::NOBUNCH;    break;
                case 1: m_BunchType = Spline::UNIFORM;    break;
                case 2: m_BunchType = Spline::SIGMOID;    break;
                default:
                case 3: m_BunchType = Spline::DOUBLESIG;  break;
            }
        }
        m_BunchType = Spline::DOUBLESIG; // force it
        pFoil->setBunchParameters(m_BunchType, f);

        int n(0);
        double x(0), y(0);
        ar >> n;

        pFoil->resizePointArrays(n);
        for (int j=0; j<n; j++)
        {
            ar>>x>>y;
            pFoil->setBasePoint(j, {x,y});
        }

        if(ArchiveFormat<500750)
        {
            if(pFoil->hasTEFlap() && fabs(pFoil->TEFlapAngle())>FLAPANGLEPRECISION)
            {
                pFoil->initGeometry();
                pFoil->setFlaps();
                pFoil->makeModPermanent();
            }
        }

        pFoil->setTEFlapAngle(0.0);


        pFoil->initGeometry();
        return true;
    }
}


void serial::serializeBLXFoil(BLXFoil*pBL, QDataStream &ar, bool bIsStoring)
{
    double dble=0.0;
    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    //500001 : first  format
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pBL->nside1 << pBL->nside2;

        ar << pBL->nd1 << pBL->nd2 << pBL->nd3;
        for (int k=0; k<=pBL->nd1; k++)  ar << float(pBL->xd1[k]) << float(pBL->yd1[k]);
        for (int k=0; k<pBL->nd2;  k++)  ar << float(pBL->xd2[k]) << float(pBL->yd2[k]);
        for (int k=0; k<pBL->nd3;  k++)  ar << float(pBL->xd3[k]) << float(pBL->yd3[k]);

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
        float f0(0.0f), f1(0.0f);

        ar >> ArchiveFormat;

        ar >> pBL->nside1 >> pBL->nside2;

        ar >> pBL->nd1 >> pBL->nd2 >> pBL->nd3;
        for (int k=0; k<=pBL->nd1; k++)
        {
            ar >> f0 >> f1;
            pBL->xd1[k] = double(f0);
            pBL->yd1[k] = double(f1);
        }
        for (int k=0; k<pBL->nd2; k++)
        {
            ar >> f0 >> f1;
            pBL->xd2[k] = double(f0);
            pBL->yd2[k] = double(f1);
        }
        for (int k=0; k<pBL->nd3; k++)
        {
            ar >> f0 >> f1;
            pBL->xd3[k] = double(f0);
            pBL->yd3[k] = double(f1);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
}


bool serial::serializeSplineFl5(Spline *pSpline, QDataStream &ar, bool bIsStoring)
{
    int n(0), k(0);
    bool boolean(false);
    int nIntSpares(0);
    int nDbleSpares(0);
    double dble(0);
    double x(0), y(0), w(0);
    int ArchiveFormat = 500001;
    // 500001 : first version of the new fl5 format

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        serial::serializeLineStyleFl5(pSpline->theStyle(), ar, bIsStoring);

        ar << pSpline->bShowNormals();
        ar << pSpline->outputSize();

        ar << pSpline->nCtrlPoints();
        for (int j=0; j<pSpline->nCtrlPoints(); j++)
        {
            ar << pSpline->controlPoint(j).x << pSpline->controlPoint(j).y;
        }

        ar << pSpline->nCtrlPoints();
        for (int j=0; j<pSpline->nCtrlPoints(); j++)
        {
            ar << pSpline->weight(j);
        }

        ar << pSpline->isClosed();
        ar << pSpline->isSymmetric();

        ar << pSpline->bunchAmplitude();

        switch(pSpline->bunchType())
        {
            default:
            case Spline::NOBUNCH:    n=0;        break;
            case Spline::UNIFORM:    n=1;        break;
            case Spline::SIGMOID:    n=2;        break;
            case Spline::DOUBLESIG:  n=3;        break;
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

        serial::serializeLineStyleFl5(pSpline->theStyle(), ar, bIsStoring);

        ar >> boolean; pSpline->showNormals(boolean);
        ar >> n;       pSpline->setOutputSize(n);

        ar >> n;
        pSpline->setControlSize(n);
        for (int j=0; j<n; j++)
        {
            ar>>x>>y;
            pSpline->setCtrlPoint(j, Node2d(x,y));
        }

        ar >> n;
//        pSpline->m_Weight.clear();
        for (int j=0; j<n; j++)
        {
            ar>>w;
            pSpline->setWeight(j, w);
        }

        ar >> boolean; pSpline->setClosed(boolean);
        ar >> boolean; pSpline->setForcedsymmetric(boolean);

        ar >> dble; pSpline->setBunchAmplitude(dble);

        ar >> n  >> k;        // ar >> m_BunchDistrib;
        switch(n)
        {
            default:
            case 0: pSpline->setBunchType(Spline::NOBUNCH);    break;
            case 1: pSpline->setBunchType(Spline::UNIFORM);    break;
            case 2: pSpline->setBunchType(Spline::SIGMOID);    break;
            case 3: pSpline->setBunchType(Spline::DOUBLESIG);  break;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool serial::serializeBezierSplineFl5(BezierSpline *pSpline, QDataStream &ar, bool bIsStoring)
{
    serializeSplineFl5(pSpline, ar, bIsStoring);

    int n=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    double dble=0.0;


    if(bIsStoring)
    {
        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
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


        pSpline->updateSpline();
        pSpline->makeCurve();
        return true;
    }
}


bool serial::serializeCubicSplineFl5(CubicSpline *pSpline, QDataStream &ar, bool bIsStoring)
{
    serial::serializeSplineFl5(pSpline, ar, bIsStoring);

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

        pSpline->updateSpline();
        pSpline->makeCurve();
        return true;
    }
}



bool serial::serializePointSplineFl5(PointSpline *pSpline, QDataStream &ar, bool bIsStoring)
{
    if(!serial::serializeSplineFl5(pSpline, ar, bIsStoring)) return false;

    if(!bIsStoring)
    {
        pSpline->updateSpline();
        pSpline->makeCurve();
    }
    return true;
}


bool serial::serializeBSplineFl5(BSpline *pSpline, QDataStream &ar, bool bIsStoring)
{
    int n=0;
    // 500001 : first version of new fl5 format

    if(bIsStoring)
    {
        serial::serializeSplineFl5(pSpline, ar, bIsStoring);
        ar << pSpline->degree();

        // dynamic space allocation for the future storage of more data, without need to change the format
        n=0;
        ar << n << n;

        return true;
    }
    else
    {
        pSpline->resetSpline();
        serial::serializeSplineFl5(pSpline, ar, bIsStoring);
        ar >> n; pSpline->setDegree(n);

        // space allocation
        ar >> n >> n;

        pSpline->updateSpline();
        pSpline->makeCurve();
        return true;
    }
}


bool serial::serializeSplineFoilXfl(SplineFoil *pSF, QDataStream &ar, bool bIsStoring)
{
    int k(0), m(0), n(0);
    bool boolean(false);
    double dble(0), x(0), y(0);
    int ArchiveFormat=200002; // 200002: new LineStyle format
    QString strangename("SplineFoil");

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << strangename;

        serial::serializeLineStyleXfl(pSF->theStyle(), ar, bIsStoring);

        ar<<pSF->bCenterLine() << pSF->bOutPoints();

        ar << pSF->extrados().degree() << pSF->intrados().degree();
        ar << pSF->extrados().outputSize() << pSF->intrados().outputSize();

        ar << pSF->extrados().ctrlPointCount();
        for (k=0; k<pSF->extrados().ctrlPointCount(); k++)
        {
            ar << pSF->extrados().controlPoint(k).x << pSF->extrados().controlPoint(k).y;
        }

        ar << pSF->intrados().ctrlPointCount();
        for (k=0; k<pSF->intrados().ctrlPointCount(); k++)
        {
            ar << pSF->intrados().controlPoint(k).x << pSF->intrados().controlPoint(k).y;
        }

        if(pSF->bClosedLE()) k=1; else k=0;
        ar << k;
        if(pSF->bClosedTE()) k=1; else k=0;
        ar << k;
        // space allocation for the future storage of more data, without need to change the format
        n=0;
        for (int i=0; i<8; i++) ar << n;
        dble=0;
        for (int i=0; i<10; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat < 200000 || ArchiveFormat > 210000) return false;

        ar >> strangename;
        if(ArchiveFormat<200002)
        {
            serial::serializefl5Color(pSF->theStyle().m_Color, ar, false);

            ar >> n; pSF->theStyle().setStipple(n);
            ar >> n; pSF->theStyle().setWidth(n);
            ar >> boolean; pSF->theStyle().setVisible(boolean);
        }
        else
            serial::serializeLineStyleXfl(pSF->theStyle(), ar, bIsStoring);

        pSF->intrados().setTheStyle(pSF->theStyle());
        pSF->extrados().setTheStyle(pSF->theStyle());

        ar >> boolean; pSF->showCenterLine(boolean);
        ar >> boolean; pSF->showOutPoints(boolean);

        ar >> m >> n;
        pSF->extrados().setDegree(m);
        pSF->intrados().setDegree(n);
        ar >> m >> n;
        pSF->extrados().setOutputSize(m);
        pSF->intrados().setOutputSize(n);

        pSF->extrados().clearControlPoints();
        ar >> n;
        for (k=0; k<n;k++)
        {
            ar >> x >> y;
            pSF->extrados().appendControlPoint({x, y});
        }

        pSF->intrados().clearControlPoints();
        ar >> n;
        for (k=0; k<n;k++)
        {
            ar >> x >> y;
            pSF->intrados().appendControlPoint({x, y});
        }

        ar >> k;
        if(k>0) pSF->setClosedLE(true); else pSF->setClosedLE(false);
        ar >> k;
        if(k>0) pSF->setClosedTE(true); else pSF->setClosedTE(false);
        // space allocation
        for (int i=0; i<8; i++) ar >> k;
        for (int i=0; i<10; i++) ar >> dble;

        pSF->extrados().splineKnots();
        pSF->intrados().splineKnots();

        pSF->makeSplineFoil();

    }
    pSF->setModified(false);
    return true;
}


bool serial::serializeSplineFoilFl5(SplineFoil*pSF,QDataStream &ar, bool bIsStoring)
{
    int n=0;
    bool boolean(false);
    int ArchiveFormat=500001;
    double dble=0;

    int nIntSpares=0;
    int nDbleSpares=0;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        serial::serializeLineStyleFl5(pSF->theStyle(), ar, bIsStoring);
        serial::serializeBSplineFl5(&pSF->extrados(), ar, bIsStoring);
        serial::serializeBSplineFl5(&pSF->intrados(), ar, bIsStoring);

        ar << pSF->bCenterLine() << pSF->bOutPoints();
        ar << pSF->isSymmetric();
        ar << pSF->bClosedLE() << pSF->bClosedTE();

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat < 500000 || ArchiveFormat > 510000) return false;

        serial::serializeLineStyleFl5(pSF->theStyle(), ar, bIsStoring);
        serial::serializeBSplineFl5(&pSF->extrados(), ar, bIsStoring);
        serial::serializeBSplineFl5(&pSF->intrados(), ar, bIsStoring);

        ar >> boolean; pSF->showCenterLine(boolean);
        ar >> boolean; pSF->showOutPoints(boolean);
        ar >> boolean; pSF->setSymmetric(boolean);
        ar >> boolean; pSF->setClosedLE(boolean);
        ar >> boolean; pSF->setClosedTE(boolean);

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        pSF->makeSplineFoil();
    }
    pSF->setModified(false);
    return true;
}


bool serial::serializeBoatPolarFl5v726(BoatPolar *pBtPolar, QDataStream &ar, bool bIsStoring)
{
    serial::serializePolar3dFl5v726(pBtPolar, ar, bIsStoring);

    bool boolean(false);
    int n(0);
    double dble(0),d0(0),d1(0),d2(0), d3(0);
    int i(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    QString strange;

    if(bIsStoring)
    {
        assert(false);
    }
    else
    {
        //read variables
        int format(0);
        ar >> format;
        pBtPolar->setPolarFormat(format);
        if(format<500001 || format>500100) return false;

        if(format<500028) pBtPolar->setIgnoreBodyPanels(true);

        ar >> strange; pBtPolar->setBoatName(strange.toStdString());

        if(format>=500023)
        {
            ar >> boolean;
            pBtPolar->setReferenceDim(boolean ? xfl::AUTODIMS : xfl::CUSTOM);
            ar >> dble; pBtPolar->setReferenceArea(dble);
            ar >> dble; pBtPolar->setReferenceChordLength(dble);
        }

        if(format<500024)
        {
            ar >> dble >> dble >> dble >> dble; // formerly wind gradient
        }
        else
        {
            serial::serializeBSplineFl5(&pBtPolar->windSpline(), ar, bIsStoring);
        }

        if(format>=500026)
        {
            ar >> dble; pBtPolar->setVBtMin(dble);
            ar >> dble; pBtPolar->setVBtMax(dble);
        }
        ar >> d0 >> d1 >> d2 >> d3;
        pBtPolar->setQInfMin(d0);
        pBtPolar->setQInfMax(d1);
        pBtPolar->setTwaMin(d2);
        pBtPolar->setTwaMax(d3);

        ar >> d0 >> d1;
        pBtPolar->setPhiMin(d0);
        pBtPolar->setPhiMax(d1);
        if(format>=500025)
        {
            ar >> d0 >> d1;
            pBtPolar->setRyMin(d0);
            pBtPolar->setRyMax(d1);
        }

        ar >> n;
        pBtPolar->resizeSailAngles(n);
        for (int is=0; is<n; is++)
        {
            ar >> d0 >> d1;
            pBtPolar->setSailAngleRange(is, d0, d1);
        }

        int datasize=0;
        ar >> datasize;
        ar >> nDbleSpares;
        pBtPolar->resizePoints(datasize);
        for (i=0; i<datasize; i++)
        {
            ar >>d0;
            ar >>d1>>d2>>d3;
            pBtPolar->setData(i, d0,d1,d2,d3);

            if(format<500022)  serial::serializeAeroForcesFl5_b17(pBtPolar->AF(i), ar, bIsStoring);
            else
            {
                if(!serial::serializeAeroForcesFl5(pBtPolar->AF(i), ar, bIsStoring)) return false;
            }

            for(int l=0; l<nDbleSpares; l++) ar >> dble;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++)  ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;


        // clean-up legacy formats
        pBtPolar->setType(xfl::BOATPOLAR);
        pBtPolar->setGroundEffect(true);
        pBtPolar->setGroundHeight(0.0);
    }
    return true;
}


bool serial::serializeBoatPolarFl5v750(BoatPolar *pBtPolar, QDataStream &ar, bool bIsStoring)
{
    serial::serializePolar3dFl5v750(pBtPolar, ar, bIsStoring);

    bool boolean(false);
    int integer(0);
    double dble(0);
    QString strange;

    int n(0);
    double d0(0),d1(0),d2(0), d3(0);
    int i(0);

    int nDbleSpares(0);

    if(bIsStoring)
    {
        //write variables
        ar << pBtPolar->polarFormat(); // identifies the format of the file
        ar << QString::fromStdString(pBtPolar->boatName());

        boolean = pBtPolar->referenceDim()==xfl::AUTODIMS ? true : false;
        ar << boolean;
        ar << pBtPolar->referenceArea() << pBtPolar->referenceChordLength();

        serial::serializeBSplineFl5(&pBtPolar->windSpline(), ar, bIsStoring);

        ar << pBtPolar->VBtMin()  << pBtPolar->VBtMax();
        ar << pBtPolar->qInfMin() << pBtPolar->qInfMax() << pBtPolar->twaMin() << pBtPolar->twaMax();
        ar << pBtPolar->phiMin()  << pBtPolar->phiMax();
        ar << pBtPolar->RyMin()   << pBtPolar->RyMax();

        ar << pBtPolar->sailAngleSize();
        for(int is=0; is<pBtPolar->sailAngleSize(); is++)
        {
            ar << pBtPolar->sailAngleMin(is) << pBtPolar->sailAngleMax(is);
        }

        ar << pBtPolar->dataSize();

        nDbleSpares = 0;
        ar << nDbleSpares;
        for(int i=0; i<pBtPolar->dataSize(); i++)
        {
            ar << pBtPolar->ctrl(i) << pBtPolar->vInf(i) <<pBtPolar->beta(i) << pBtPolar->phiPt(i);
            serial::serializeAeroForcesFl5(pBtPolar->AF(i), ar, bIsStoring);

            dble=0.0;
            for(int i=0; i<nDbleSpares; i++) ar << dble;
        }

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar <<boolean;
        for(int i=0; i<20; i++) ar <<integer;
        for(int i=0; i<20; i++) ar <<dble;

        return true;
    }
    else
    {
        //read variables
        int format(0);
        ar >> format;
        pBtPolar->setPolarFormat(format);
        if(format<500750 || format>501000) return false;

        ar >> strange;  pBtPolar->setBoatName(strange.toStdString());

        ar >> boolean;
        pBtPolar->setReferenceDim(boolean ? xfl::AUTODIMS : xfl::CUSTOM);
        ar >> dble; pBtPolar->setReferenceArea(dble);
        ar >> dble; pBtPolar->setReferenceChordLength(dble);

        serial::serializeBSplineFl5(&pBtPolar->windSpline(), ar, bIsStoring);


        ar >> d0 >> d1; pBtPolar->setVBtMin(d0); pBtPolar->setVBtMax(d1);
        ar >> d0 >> d1 >> d2 >> d3;
        pBtPolar->setQInfMin(d0);
        pBtPolar->setQInfMax(d1);
        pBtPolar->setTwaMin(d2);
        pBtPolar->setTwaMax(d3); // AWS up to format 500026

        ar >> d0 >> d1;
        pBtPolar->setPhiMin(d0); pBtPolar->setPhiMax(d1);
        ar >> d0 >> d1;
        pBtPolar->setRyMin(d0);  pBtPolar->setRyMax(d1);

        ar >> n;
        pBtPolar->resizeSailAngles(n);
        for (int is=0; is<n; is++)
        {
            ar >> d0 >> d1;
            pBtPolar->setSailAngleRange(is, d0, d1);
        }

        int datasize=0;
        ar >> datasize;
        ar >> nDbleSpares;
        pBtPolar->resizePoints(datasize);
        for (i=0; i<datasize; i++)
        {
            ar >>d0>>d1>>d2>>d3;
            pBtPolar->setData(i, d0, d1, d2, d3);
            if(!serial::serializeAeroForcesFl5(pBtPolar->AF(i), ar, bIsStoring)) return false;

            for(int l=0; l<nDbleSpares; l++) ar >> dble;
        }

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar >>boolean;
        for(int i=0; i<20; i++) ar >>integer;
        for(int i=0; i<20; i++) ar >>dble;

    }
    return true;
}


bool serial::serializePlanePolarXfl(PlanePolar *pPolar, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int i(0), k(0), n(0);
    double dble(0);
    double d0(0), d1(0), d2(0);
    double r0(0), r1(0), r2(0), r3(0), r4(0), r5(0), r6(0), r7(0);
    double i0(0), i1(0), i2(0), i3(0), i4(0), i5(0), i6(0), i7(0);
    QString strange;

    pPolar->setPolarFormat(200013);
    // 200013: v0.00
    // 200014:  added array of control values for stability polars
    // 200015:  added array of control values for controls polars

    if(bIsStoring)
    {

        return false;
    }
    else
    {
        //input the variables from the stream
        int format = 200013;
        ar >> k; format=k;
        if((format<200000) || (format>205000)) return false;
        pPolar->setPolarFormat(format);

        ar >> strange;   pPolar->setPlaneName(strange.toStdString());
        ar >> strange;   pPolar->setName(strange.toStdString());

        ar >> d0 >> d1 >> d2;
        pPolar->setReferenceArea(d0);
        pPolar->setReferenceChordLength(d1);
        pPolar->setReferenceSpanLength(d2);

        if(format<200014)
        {
            ar >> k;
            pPolar->theStyle().setStipple(k);
            ar >> pPolar->theStyle().m_Width;

            serial::serializefl5Color(pPolar->theStyle().m_Color, ar, false);

            ar >> pPolar->theStyle().m_bIsVisible >> boolean;
        }
        else
            serial::serializeLineStyleXfl(pPolar->theStyle(), ar, bIsStoring);

        ar >> n;
        if     (n==1) pPolar->setAnalysisMethod(xfl::LLT);
        else if(n==2) pPolar->setAnalysisMethod(xfl::VLM1);
        else if(n==3) pPolar->setAnalysisMethod(xfl::QUADS);
        else if(n==4) pPolar->setAnalysisMethod(xfl::TRILINEAR);
        else if(n==5) pPolar->setAnalysisMethod(xfl::TRIUNIFORM);

        ar >> n;
        if     (n==1) pPolar->setType(xfl::T1POLAR);
        else if(n==2) pPolar->setType(xfl::T2POLAR);
        else if(n==4) pPolar->setType(xfl::T4POLAR);
        else if(n==5) pPolar->setType(xfl::T5POLAR);
        else if(n==6) pPolar->setType(xfl::T6POLAR);
        else if(n==7) pPolar->setType(xfl::T7POLAR);

        bool bVLM1(false);
        ar >> bVLM1;
        if(pPolar->isVLM1())
        {
            if(!bVLM1) pPolar->setAnalysisMethod(xfl::VLM2);
        }

        ar >> boolean; pPolar->setThinSurfaces(boolean);
        if(pPolar->bThinSurfaces() && !pPolar->isLLTMethod())
        {
            if(bVLM1) pPolar->setAnalysisMethod(xfl::VLM1);
            else      pPolar->setAnalysisMethod(xfl::VLM2);
        }

        if(pPolar->isTriangleMethod()) pPolar->setThinSurfaces(false);  // cleaning up incorrectly constructed polars

        ar >> boolean; // m_bTiltedGeom;
        ar >> boolean;
        pPolar->setBoundaryCondition(boolean? xfl::DIRICHLET : xfl::NEUMANN);

        ar >> boolean;
        pPolar->setViscous(boolean);        pPolar->setViscFromCl(true);

        ar >> boolean; pPolar->setIgnoreBodyPanels(boolean);

        if(pPolar->isVLM()) pPolar->setIgnoreBodyPanels(true); //clean up override

        ar >> boolean; pPolar->setGroundEffect(boolean);
        ar >> dble;    pPolar->setGroundHeight(dble);

        ar >> d0 >> d1;
        pPolar->setDensity(d0);
        pPolar->setViscosity(d1);

        ar >> k;
        if     (k==1) pPolar->setReferenceDim(xfl::PLANFORM);
        else if(k==2) pPolar->setReferenceDim(xfl::PROJECTED);
        else if(k==3) pPolar->setReferenceDim( xfl::CUSTOM);
        else          pPolar->setReferenceDim(xfl::PLANFORM);

        ar >> boolean; pPolar->setAutoInertia(boolean);
        ar >> dble;    pPolar->setMass(dble);

        ar >> dble >> dble >> dble; // CoGxyz moved to Polar3d
        ar >> dble >> dble >> dble >> dble; //Inertia tensor moved to polar3d

/*        ar >> m_CoG.x >> m_CoG.y >> m_CoG.z;
        ar >> m_Inertia[0] >> m_Inertia[1] >> m_Inertia[2] >> m_Inertia[3];*/

        ar >> k;

        for(int icg=0; icg<k; icg++)
        {
            ar >> dble;
//            m_ControlGain[icg] = dble;
        }

        ar >> n;
        pPolar->setNXWakePanel4(n);
        ar >> d0 >> d1;
        pPolar->setTotalWakeLengthFactor(d0); pPolar->setWakePanelFactor(d1);

        ar >> d0 >> d1 >> d2;
        pPolar->setVelocity(d0);
        pPolar->setAlphaSpec(d1);
        pPolar->setBeta(d2);

        // Last store the array data
        // assumes the arrays have been cleared previously
        double d[20];
        pPolar->clearPolarData();

        ar >> n;
        if(abs(n)>10000) return false;

        for (i=0; i<n; i++)
        {
            for(int j=0; j<20; j++)
            {
                ar >> d[j];
            }
            //            insertDataAt(i, d[0],  d[1],  d[2],  d[3],  d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13],
            //                            d[14], d[15], d[16], d[17], d[18], d[19]);
            /*            void WPolar::insertDataAt(int pos, double Alpha, double Beta, double QInf, double Ctrl, double CL, double CY,
 *                                    double ICd, double VCd,
                                      double GCm, double ICm, double VCm, double GRm, double GYm, double IYm, double VYm,
                                      double XCP, double YCP, double ZCP,
                                      double Cb, double XNP)*/
            //            double alpha = d[0];
            //            double beta = d[1];
            //            double QInf = d[2];
            //            double ctrl = d[3];
            double CL   = d[4];
            double CY   = d[5];
            double ICd  = d[6];
            double VCd  = d[7];
            //            double GCm  = d[8];
            double ICm  = d[9];
            double VCm  = d[10];
            double GRm = d[11];
            //            double GYm = d[12];
            double IYm = d[13];
            double VYm = d[14];
/*            double XCP = d[15];
            double YCP = d[16];
            double ZCP = d[17];*/

            //rebuild aeroforces
            double cosa = cos(d[0]*PI/180.0);
            double sina = sin(d[0]*PI/180.0);

            AeroForces AF;
            double area  = pPolar->referenceArea();
            double chord = pPolar->referenceChordLength();
            double span  = pPolar->referenceSpanLength();
            AF.setReferenceArea(area);
            AF.setReferenceChord(chord);
            AF.setReferenceSpan(span);
            AF.setFff({(ICd*cosa-CL*sina)*area, CY*area, (ICd*sina+CL*cosa)*area}); // N/q
            AF.setProfileDrag(VCd*area);
            AF.setMi({GRm*span*area, ICm*chord*area, IYm*span*area});
            AF.setMv({0.0,           VCm*chord*area, VYm*span*area});
//            AF.setCP({XCP, YCP, ZCP});
            pPolar->m_AF.push_back(AF);

            pPolar->insertDataAt(i, d[0],d[1],0.0,d[2],d[3],d[4],d[5]);

            ar >> r0 >> r1 >>r2 >> r3;
            ar >> i0 >> i1 >>i2 >> i3;
            ar >> r4 >> r5 >>r6 >> r7;
            ar >> i4 >> i5 >>i6 >> i7;

            pPolar->m_EV.push_back(EigenValues());
            pPolar->m_EV[i].m_EV[0] = std::complex<double>(r0, i0);
            pPolar->m_EV[i].m_EV[1] = std::complex<double>(r1, i1);
            pPolar->m_EV[i].m_EV[2] = std::complex<double>(r2, i2);
            pPolar->m_EV[i].m_EV[3] = std::complex<double>(r3, i3);
            pPolar->m_EV[i].m_EV[4] = std::complex<double>(r4, i4);
            pPolar->m_EV[i].m_EV[5] = std::complex<double>(r5, i5);
            pPolar->m_EV[i].m_EV[6] = std::complex<double>(r6, i6);
            pPolar->m_EV[i].m_EV[7] = std::complex<double>(r7, i7);

        }

        // space allocation
        // integers
        for (int i=0; i<15; i++) ar >> k;

        pPolar->setVortonWake(false);
        ar >> k; //m_nWakeIterations; m_nWakeIterations=std::max(m_nWakeIterations, 1);
        ar >> k;
        ar >> k;
        ar >> k;    pPolar->setAdjustedVelocity(k ? true : false);
        ar >> k;    pPolar->theStyle().m_Symbol=LineStyle::convertSymbol(k);

        // double
        for (int i=0; i<29; i++) ar >> dble;
        ar >> dble >> dble >> dble;
        ar >> d0 >> d1 >> d2;
        pPolar->setNCrit(d0);
        pPolar->setXTrTop(d1);
        pPolar->setXTrBot(d2);

        if(fabs(pPolar->NCrit())<PRECISION)
        {
            pPolar->setNCrit(9.0);
            pPolar->setXTrTop(1.0);
            pPolar->setXTrBot(1.0);
        }

        pPolar->clearExtraDrag();
        pPolar->resizeExtraDrag(4);
        for (int ix=0; ix<4; ix++)
        {
            std::string strong;
            strong = std::format("Extra drag {:d}", ix);
            pPolar->extraDrag(ix).setName(strong);
        }
        for (int ix=0; ix<4; ix++) {ar>>dble; pPolar->extraDrag(ix).setArea(dble);}
        for (int ix=0; ix<4; ix++) {ar>>dble; pPolar->extraDrag(ix).setCoef(dble);}
        //clear the null extradrag
        for (int ix=3; ix>=0; ix--)
        {
            ExtraDrag &xd = pPolar->extraDrag(ix);
            if(fabs(xd.area())<PRECISION && fabs(xd.coef())<PRECISION) pPolar->extraDrags().erase(pPolar->extraDrags().begin()+ix);
        }

        if(format<200013)
        {
/*            for (int ix=0; ix<MAXEXTRADRAG; ix++)
            {
                    m_ExtraDragArea[ix] = 0.0;
                    m_ExtraDragCoef[ix] = 0.0;
            }*/
        }

        for (int i=0; i<7; i++)
        {
            ar >> dble; // m_InertiaGain
        }

        for(int iPt=0; iPt<pPolar->dataSize(); iPt++)    pPolar->calculatePoint(iPt);

        return true;
    }
}


bool serial::serializePlanePolarFl5v726(PlanePolar *pPolar, QDataStream &ar, bool bIsStoring)
{
    if(!serial::serializePolar3dFl5v726(pPolar, ar, bIsStoring)) return false;

    int k(0), n(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    double dble(0), dmin(0), dmax(0);
    double d0(0), d1(0), d2(0);
    bool boolean(false);

    QString strange;

    if(bIsStoring)
    {
        assert(false);
    }
    else
    {
        //input the variables from the stream

        // METADATA
        if ((pPolar->polarFormat()<500001) || (pPolar->polarFormat()>500100))
            return false;

        ar >> strange;  pPolar->setPlaneName(strange.toStdString());

        ar >> dble; pPolar->setVelocity(dble);
        ar >> dble; pPolar->setAlphaSpec(dble);

        ar >> boolean; pPolar->setThinSurfaces(boolean);

        // REFERENCE DIMENSIONS
        ar >> k;
        if     (k==1) pPolar->setReferenceDim(xfl::PLANFORM);
        else if(k==2) pPolar->setReferenceDim(xfl::PROJECTED);
        else if(k==3) pPolar->setReferenceDim(xfl::CUSTOM);
        else          pPolar->setReferenceDim(xfl::PLANFORM);
        ar >> d0 >> d1 >> d2;
        pPolar->setReferenceArea(d0);
        pPolar->setReferenceChordLength(d1);
        pPolar->setReferenceSpanLength(d2);

        if(pPolar->polarFormat()<500020) pPolar->setVortonCoreSize(pPolar->vortonCoreSize() / pPolar->referenceChordLength());

        // AVL type control ranges
        if(pPolar->polarFormat()>=500021)
        {
            ar >> n;
            pPolar->resizeAVLCtrls(n);
            for(int ic=0; ic<pPolar->nAVLCtrls(); ic++) serial::serializeAngleControlFl5(pPolar->AVLCtrl(ic), ar, bIsStoring);

            //            if(m_Type!=xfl::T6POLAR) m_AVLControls.clear(); // cleaning up
        }

        //STABILITY POLAR ANGLE AND INERTIA GAINS
        int nCtrls;
        ar>>nCtrls; // formerly m_AngleCoef.size() - deprecated in v713
        if(nCtrls<0 || n>10000) return false;
        for(int iw=0; iw<nCtrls; iw++)
        {
            ar>>k;
            for(int ic=0; ic<k; ic++)
            {
                if(pPolar->polarFormat()>=500007)
                {
                    ar>>dble;
                }
                ar>>dble;  // m_AngleGain[iw].push_back(dble);
            }
        }

        for (int i=0; i<7; i++) ar >> dble; // m_InertiaGain

        ar >> k;    pPolar->setAdjustedVelocity(k ? true : false);

        //CONTROL POLAR RANGES
        pPolar->clearAngleRangeList();
        ar>>n;
        pPolar->resizeAngleRange(n);
        for(int iw=0; iw<pPolar->nAngleRange(); iw++)
        {
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
                pPolar->m_AngleRange[iw].push_back({strange.toStdString(), dmin, dmax});
            }
        }

        if(pPolar->polarFormat()>=500002)
        {
            // formerly LE range
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
            }
        }

        if(pPolar->polarFormat()>=500003)
        {
            // formerly Wing Shape ranges
            ar>>n;
            for(int iw=0; iw<n; iw++)
            {
                ar>>k;
                for(int c=0; c<k; c++)
                {
                    ar >> strange;
                    ar >> dmin>> dmax;
                }
            }
        }

        pPolar->m_OperatingRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            pPolar->m_OperatingRange.push_back({strange.toStdString(), dmin, dmax});
        }
        if(pPolar->m_OperatingRange.size()<4)
        {
            pPolar->m_OperatingRange.resize(4);
            pPolar->m_OperatingRange[3].set("phi", 0.0, 0.0);
        }

        // update old formats
        pPolar->m_OperatingRange[0].setName("Vinf");
        pPolar->m_OperatingRange[1].setName("alpha");
        pPolar->m_OperatingRange[2].setName("beta");
        pPolar->m_OperatingRange[3].setName("phi");

        pPolar->m_InertiaRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            pPolar->m_InertiaRange.push_back({strange.toStdString(), dmin, dmax});
        }

        // FUSE DATA
        if(pPolar->polarFormat()>=500004)
        {
            ar >> k;    pPolar->setIncludeFuseMi(k ? true : false);
        }
        ar >> k;        pPolar->setIncludeFuseDrag(k ? true : false);
        ar >> k;
        if     (k==1) pPolar->setFuseDragMethod(PlanePolar::KARMANSCHOENHERR);
        else if(k==2) pPolar->setFuseDragMethod(PlanePolar::PRANDTLSCHLICHTING);
        else          pPolar->setFuseDragMethod(PlanePolar::MANUALFUSECF);

        // Last store the array data
        // assumes the arrays have been cleared previously
        double d[20];
        pPolar->clearPolarData();

        int nSpares(0);
        ar >> nSpares;
        ar >> n;
        if(abs(n)>10000) return false;

        for (int i=0; i<n; i++)
        {
            AeroForces AC;
            if(pPolar->polarFormat()<500022) serial::serializeAeroForcesFl5_b17(AC, ar, bIsStoring);
            else
            {
                if(!serial::serializeAeroForcesFl5(AC, ar, bIsStoring))
                    return false;
            }
            pPolar->m_AF.push_back(AC);
            for(int j=0; j<6; j++) ar >> d[j];

            pPolar->insertDataAt(i, d[0], d[1], 0.0, d[2], d[3], d[4], d[5]);

            EigenValues EV;
            serial::serializeEigenFl5(EV, ar, bIsStoring);
            pPolar->m_EV.push_back(EV);

            if(nSpares>=1)
            {
                ar >> dble; pPolar->m_Phi[i] = dble;
            }

            for(int i=1; i<nSpares; i++) ar >> dble;

            pPolar->m_AF.back().setOpp(pPolar->m_Alpha.at(i), pPolar->m_Beta.at(i), pPolar->m_Phi.at(i), pPolar->m_QInfinite.at(i));
        }

        if(pPolar->polarFormat()>=500017)
        {
            ar >> boolean;
            BSpline AVLBSpline;
            serial::serializeBSplineFl5(&AVLBSpline, ar, bIsStoring);
            pPolar->setAVLDrag(boolean, AVLBSpline);
        }

        // space allocation
        ar >> nIntSpares; // 500005: nIntSpares=1
        if(nIntSpares>=1)
        {
            if(nIntSpares>=1)
            {
                ar >> n;
                pPolar->setIncludeOtherWingAreas(n==1 ? true : false);
            }

            pPolar->setIncludeWingTipMi(pPolar->bThinSurfaces() ? false : true); // default for polars saved prior to v7.24
            if(nIntSpares>=2)
            {
                ar >> n;
                pPolar->setIncludeWingTipMi(n==1 ? true : false);
            }
        }

        ar >> nDbleSpares;
        if(nDbleSpares>=1)
        {
            ar >> dble; pPolar->setCustomFuseCf(dble);
        }

        for(int iPt=0; iPt<pPolar->dataSize(); iPt++) pPolar->calculatePoint(iPt);
    }
    return true;
}


bool serial::serializePlanePolarFl5v750(PlanePolar *pPolar, QDataStream &ar, bool bIsStoring)
{
    if(!serial::serializePolar3dFl5v750(pPolar, ar, bIsStoring)) return false;

    int k(0), n(0);
    double d0(0), d1(0), d2(0);
    bool boolean(false);
    int integer(0);
    double dble(0);
    double dmin(0), dmax(0);

    QString strange;

    if(bIsStoring)
    {
        //METADATA
        ar << QString::fromStdString(pPolar->planeName());

        ar << pPolar->velocity();
        ar << pPolar->alphaSpec();

        ar << pPolar->bThinSurfaces();

        ar << pPolar->bViscousLoop();

        // REFERENCE DIMENSIONS
        if     (pPolar->referenceDim() == xfl::PLANFORM)  ar << 1;
        else if(pPolar->referenceDim() == xfl::PROJECTED) ar << 2;
        else if(pPolar->referenceDim() == xfl::CUSTOM)    ar << 3;
        ar << pPolar->referenceArea() << pPolar->referenceChordLength() << pPolar->referenceSpanLength();

        // TE flap angles
        ar << pPolar->nFlapCtrls();
        for(int ic=0; ic<pPolar->nFlapCtrls(); ic++)
            serial::serializeAngleControlFl5(pPolar->m_FlapControls[ic], ar, bIsStoring);

        // AVL type control ranges
        ar << pPolar->nAVLCtrls();
        for(int ic=0; ic<pPolar->nAVLCtrls(); ic++)
            serial::serializeAngleControlFl5(pPolar->m_AVLControls[ic], ar, bIsStoring);

        ar << pPolar->isAdjustedVelocity();

        ar << pPolar->nAngleRange();
        for(int icg=0; icg<pPolar->nAngleRange(); icg++)
        {
            ar << int(pPolar->m_AngleRange.at(icg).size());
            for(uint jcg=0; jcg<pPolar->m_AngleRange.at(icg).size(); jcg++)
            {
                ar << QString::fromStdString(pPolar->m_AngleRange.at(icg).at(jcg).name());
                ar << pPolar->m_AngleRange.at(icg).at(jcg).ctrlMin() << pPolar->m_AngleRange.at(icg).at(jcg).ctrlMax();
            }
        }

        //Operating point range
        ar << int(pPolar->m_OperatingRange.size());
        for(uint jcg=0; jcg<pPolar->m_OperatingRange.size(); jcg++)
        {
            ar << QString::fromStdString(pPolar->m_OperatingRange.at(jcg).name());
            ar << pPolar->m_OperatingRange.at(jcg).ctrlMin() << pPolar->m_OperatingRange.at(jcg).ctrlMax();
        }

        //Inertia range
        ar << int(pPolar->m_InertiaRange.size());
        for(uint jcg=0; jcg<pPolar->m_InertiaRange.size(); jcg++)
        {
            ar << QString::fromStdString(pPolar->m_InertiaRange.at(jcg).name());
            ar << pPolar->m_InertiaRange.at(jcg).ctrlMin() << pPolar->m_InertiaRange.at(jcg).ctrlMax();
        }

        // FUSE DATA
        ar << pPolar->bFuseMi();
        ar << pPolar->hasFuseDrag();

        switch(pPolar->fuseDragMethod())
        {
            case PlanePolar::MANUALFUSECF:         ar<<0;  break;
            case PlanePolar::KARMANSCHOENHERR:     ar<<1;  break;
            case PlanePolar::PRANDTLSCHLICHTING:   ar<<2;  break;
        }

        // Last store the array data
        int nSpares=0;
        ar << nSpares;
        ar << pPolar->dataSize();

        for (int i=0; i<pPolar->dataSize(); i++)
        {
            serial::serializeAeroForcesFl5(pPolar->m_AF[i], ar, bIsStoring);

            ar << pPolar->m_Alpha.at(i) << pPolar->m_Beta.at(i) << pPolar->m_Phi.at(i) << pPolar->m_QInfinite.at(i) << pPolar->m_Ctrl.at(i);

            ar << pPolar->m_MaxBending.at(i);
            ar << pPolar->m_XNP.at(i);

            serial::serializeEigenFl5(pPolar->m_EV[i], ar, bIsStoring);

            for(int js=0; js<nSpares; js++) ar<<dble;  // nSpares=0 anyway in v750
        }


        ar << pPolar->bAVLDrag();
        serial::serializeBSplineFl5(&pPolar->AVLSpline(), ar, bIsStoring);

        ar << pPolar->bIncludeOtherWingAreas();
        ar << pPolar->bWingTipMi();
        ar << pPolar->customFuseCf();

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar <<boolean;
        for(int i=0; i<20; i++) ar <<integer;
        for(int i=0; i<20; i++) ar <<dble;

        return true;
    }
    else
    {
        // METADATA
        if ((pPolar->polarFormat()<500750) || (pPolar->polarFormat()>501000)) // failsafe
            return false;

        ar >> strange;    pPolar->setPlaneName(strange.toStdString());

        ar >> dble; pPolar->setVelocity(dble);
        ar >> dble; pPolar->setAlphaSpec(dble);

        ar >> boolean; pPolar->setThinSurfaces(boolean);
        ar >> boolean; pPolar->setViscousLoop(boolean);


        // REFERENCE DIMENSIONS
        ar >> k;
        if     (k==1) pPolar->setReferenceDim(xfl::PLANFORM);
        else if(k==2) pPolar->setReferenceDim(xfl::PROJECTED);
        else if(k==3) pPolar->setReferenceDim(xfl::CUSTOM);
        else          pPolar->setReferenceDim(xfl::PLANFORM);
        ar >> d0 >> d1 >> d2;
        pPolar->setReferenceArea(d0);
        pPolar->setReferenceChordLength(d1);
        pPolar->setReferenceSpanLength(d2);

        // TE flap angles
        ar >> n;
        pPolar->m_FlapControls.resize(n);
        for(int ic=0; ic<pPolar->nFlapCtrls(); ic++) serial::serializeAngleControlFl5(pPolar->m_FlapControls[ic], ar, bIsStoring);

        // AVL type control ranges
        ar >> n;
        pPolar->m_AVLControls.resize(n);
        for(int ic=0; ic<pPolar->nAVLCtrls(); ic++) serial::serializeAngleControlFl5(pPolar->m_AVLControls[ic], ar, bIsStoring);


        ar >> boolean; pPolar->setAdjustedVelocity(boolean);

        //CONTROL POLAR RANGES
        pPolar->clearAngleRangeList();
        ar>>n;
        pPolar->m_AngleRange.resize(n);
        for(int iw=0; iw<pPolar->nAngleRange(); iw++)
        {
            ar>>n;
            for(int c=0; c<n; c++)
            {
                ar >> strange;
                ar >> dmin>> dmax;
                pPolar->m_AngleRange[iw].push_back({strange.toStdString(), dmin, dmax});
            }
        }

        pPolar->m_OperatingRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;

            pPolar->m_OperatingRange.push_back({strange.toStdString(), dmin, dmax});
        }


        pPolar->m_InertiaRange.clear();
        ar>>n;
        for(int c=0; c<n; c++)
        {
            ar >> strange;
            ar >> dmin>> dmax;
            pPolar->m_InertiaRange.push_back({strange.toStdString(), dmin, dmax});
        }

        // FUSE DATA

        ar >> boolean; pPolar->setIncludeFuseMi(boolean);
        ar >> boolean; pPolar->setIncludeFuseDrag(boolean);

        ar >> k;
        if     (k==1) pPolar->setFuseDragMethod(PlanePolar::KARMANSCHOENHERR);
        else if(k==2) pPolar->setFuseDragMethod(PlanePolar::PRANDTLSCHLICHTING);
        else          pPolar->setFuseDragMethod(PlanePolar::MANUALFUSECF);

        // Last store the array data
        // assumes the arrays have been cleared previously
        double d[20];
        pPolar->clearPolarData();

        int nSpares(0);
        ar >> nSpares;
        ar >> n;
        if(abs(n)>10000) return false;

        for (int i=0; i<n; i++)
        {
            AeroForces AC;

            if(!serial::serializeAeroForcesFl5(AC, ar, bIsStoring))
                return false;

            pPolar->m_AF.push_back(AC);
            for(int j=0; j<7; j++) ar >> d[j];

            pPolar->insertDataAt(i, d[0], d[1], d[2], d[3], d[4], d[5], d[6]);

            EigenValues EV;
            serial::serializeEigenFl5(EV, ar, bIsStoring);
            pPolar->m_EV.push_back(EV);


            for(int i=0; i<nSpares; i++) ar >> dble;

            pPolar->m_AF.back().setOpp(pPolar->m_Alpha.at(i), pPolar->m_Beta.at(i), pPolar->m_Phi.at(i), pPolar->m_QInfinite.at(i));
        }

        ar >> boolean;
        BSpline bspline;
        serial::serializeBSplineFl5(&bspline, ar, bIsStoring);
        pPolar->setAVLDrag(boolean, bspline);


        ar >> boolean; pPolar->setIncludeOtherWingAreas(boolean);
        ar >> boolean; pPolar->setIncludeWingTipMi(boolean);
        ar >> dble;    pPolar->setCustomFuseCf(dble);

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar >> boolean;
        for(int i=0; i<20; i++) ar >> integer;
        for(int i=0; i<20; i++) ar >> dble;


        for(int iPt=0; iPt<pPolar->dataSize(); iPt++) pPolar->calculatePoint(iPt);
        return true;
    }
}


bool serial::serializePlanePolarExtFl5v726(PlanePolarExt *pPolar, QDataStream &ar, bool bIsStoring)
{
    if(!serial::serializePolar3dFl5v726(pPolar, ar, bIsStoring)) return false;

    int n=0;
    int nDataPoints=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    QString strange;
    double dble=0.0;

    if(bIsStoring)
    {
        assert(false);
    }
    else
    {
        // METADATA
        if((pPolar->polarFormat() < 500001) || (pPolar->polarFormat())>500100) return false;
        ar >> strange; pPolar->setPlaneName(strange.toStdString());

        // load the array data
        pPolar->clearData();
        int nSpares=0;
        ar >> nSpares;
        ar >> nDataPoints;
        if(abs(n)>10000) return false;
        pPolar->resizeData(nDataPoints);
//        for(uint ivar=0; ivar<pPolar->nVariables(); ivar++)            pPolar->m_data[ivar].resize(nDataPoints);

        int nStoredVariables = pPolar->variableCount();
        if     (pPolar->polarFormat()<500013) nStoredVariables = 54;
        else if(pPolar->polarFormat()<500030) nStoredVariables = 56;
        else                                  nStoredVariables = 57;

        int ivar = 0;
        for (int i=0; i<nStoredVariables; i++)
        {
            if(pPolar->polarFormat()<500030)
            {
                if(i==3) ivar++; // skipping phi non existant in < 500030
            }

            for(int ipt=0; ipt<nDataPoints; ipt++)
            {
                ar >> dble;
                if((ivar<pPolar->nVariables()) && (ipt < pPolar->dataSize()))
                    pPolar->setData(ivar, ipt, dble);
            }
            ivar++;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        for(int iPt=0; iPt<pPolar->dataSize(); iPt++)	pPolar->calculatePoint(iPt);
    }
    return true;
}



bool serial::serializePlanePolarExtFl5v750(PlanePolarExt *pPolar, QDataStream &ar, bool bIsStoring)
{
    if(!serial::serializePolar3dFl5v750(pPolar, ar, bIsStoring)) return false;

    int n=0;
    int nDataPoints(0);
    QString strange;

    bool boolean(false);
    int integer(0);
    double dble(0.0);

    if(bIsStoring)
    {
        //METADATA
        ar << QString::fromStdString(pPolar->planeName());

        // store the array data
        int nSpares=0;
        dble=0.0;
        ar << nSpares;
        ar << pPolar->dataSize();
        for(int ivar=0; ivar<pPolar->nVariables(); ivar++)
        {
            for(int i=0; i<pPolar->dataSize(); i++)
            {
                ar<<pPolar->data(ivar, i);
                for(int js=0; js<nSpares; js++) ar<<dble;
            }
        }

        // provisions for future variable saves
        for(int i=0; i<5; i++)  ar << boolean;
        for(int i=0; i<10; i++) ar << integer;
        for(int i=0; i<10; i++) ar << dble;

        return true;
    }
    else
    {
        // METADATA
        if(pPolar->polarFormat() < 500750 || pPolar->polarFormat()>501000) return false;
        ar >> strange; pPolar->setPlaneName(strange.toStdString());

        // load the array data
        pPolar->clearData();
        int nSpares=0;
        ar >> nSpares;
        ar >> nDataPoints;
        if(abs(n)>10000) return false;
        pPolar->resizeData(nDataPoints);
//        for(uint ivar=0; ivar<pPolar->dataSize(); ivar++)            pPolar->m_data[ivar].resize(nDataPoints);

        int nStoredVariables = pPolar->variableCount();

        int ivar = 0;
        for (int i=0; i<nStoredVariables; i++)
        {
            for(int ipt=0; ipt<nDataPoints; ipt++)
            {
                ar >> dble;
                if(ivar<int(pPolar->dataSize()) && (ipt<pPolar->dataSize()))
                    pPolar->setData(ivar, ipt, dble);
            }
            ivar++;
        }

        // provisions for future variable saves
        for(int i=0; i<5; i++)  ar >> boolean;
        for(int i=0; i<10; i++) ar >> integer;
        for(int i=0; i<10; i++) ar >> dble;


        for(int iPt=0; iPt<pPolar->dataSize(); iPt++) pPolar->calculatePoint(iPt);
        return true;
    }
}


bool serial::serializePolar3dFl5v726(Polar3d *pPolar, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int k(0), n(0);

    double dble(0.0), d0(0), d1(0), d2(0), d3(0);
    int nIntExtra(0);
    int nDbleExtra(0);

    QString strange;

    // 500001:  new fl5 format
    // 500002: changes in WPolar serialization
    // 500003: changes in WPolar serialization
    // 500004: changes in WPolar serialization
    // 500005: changes in WPolar serialization
    // 500006: added vorton wake data
    // 500007: changes in WPolar serialization
    // 500011: marked the change to beta 08
    // 500012: added buffer wake length and vorton total length
    // 500013: beta 13 - added Lift and Drag (N) to variables; not stored, but changes the variable count in WPolarExt
    // 500015: beta 15 - added the vorton core size
    // 500016: beta 15 - added the VPW length
    // 500017: beta 15 - added AVL type parabolic drag
    // 500018: beta 15 - skipped?
    // 500019: beta 15 - added VPW max iterations
    // 500020: beta 16 - stored the Vorton core size as a fraction of the reference chord
    // 500021: beta 17 - stored the AVL type controls for stability analyses
    // 500022: beta 18 - modified the format of AeroForces serialization
    // 500023: beta 18 - added ref. dimensions to BoatPolar
    // 500024: beta 19 - added windgradient spline to BoatPolar
    // 500025: beta 20 - added Ry rotations to BoatPolar
    // 500026: v7.03 - added boat speed to BoatPolar
    // 500027: v7.04 - replaced AW variables by TW variables in boat polars
    // 500028: v7.08 - forced IgnoreHullPanels=true for legacy boat polars
    // 500029: v7.21 - added free surface effect
    // 500030: v7.26 - added phi - changes the variable count in WPolarExt

    int format = 500030;
    pPolar->setPolarFormat(format);


    if(bIsStoring)
    {
        assert(false);
    }
    else
    {
        //input the variables from the stream
        ar >> format;
        if((format < 500000) || (format>500100)) return false;

        pPolar->setPolarFormat(format);

        // METADATA
        ar >> strange; pPolar->setName(strange.toStdString());

        serial::serializeLineStyleFl5(pPolar->theStyle(), ar, bIsStoring);


        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) pPolar->setAnalysisMethod(xfl::LLT);
        else if(n==2) pPolar->setAnalysisMethod(xfl::VLM1);
        else if(n==3) pPolar->setAnalysisMethod(xfl::VLM2);
        else if(n==4) pPolar->setAnalysisMethod(xfl::QUADS);
        else if(n==5) pPolar->setAnalysisMethod(xfl::TRILINEAR);
        else if(n==6) pPolar->setAnalysisMethod(xfl::TRIUNIFORM);

        ar >> boolean; // m_bTiltedGeom;
        ar >> boolean; pPolar->setTrefftz(boolean);
        ar >> boolean;
        pPolar->setBoundaryCondition(boolean? xfl::DIRICHLET : xfl::NEUMANN);
        ar >> boolean; pPolar->setIgnoreBodyPanels(boolean);

        // POLAR TYPE
        ar >> n;
        if     (n==1)   pPolar->setType(xfl::T1POLAR);
        else if(n==2)   pPolar->setType(xfl::T2POLAR);
        else if(n==3)   pPolar->setType(xfl::T3POLAR);
        else if(n==4)   pPolar->setType(xfl::T4POLAR);
        else if(n==5)   pPolar->setType(xfl::T5POLAR);
        else if(n==6)   pPolar->setType(xfl::T6POLAR);
        else if(n==7)   pPolar->setType(xfl::T7POLAR);
        else if(n==8 || n==100) pPolar->setType(xfl::T8POLAR);

        // EXTRADRAGDATA
        QString strong;
        double area=0.0, coef=0.0;
        int ExtraDragCount = 0;
        ar >> ExtraDragCount;
        pPolar->clearExtraDrag();
        for (int ix=0; ix<ExtraDragCount; ix++)
        {
            ar >> strong >> area >> coef;
            pPolar->appendExtraDrag({strong.toStdString(), area, coef});
        }

        //AERO DATA
        ar >> boolean; pPolar->setGroundEffect(boolean);
        if(format>=500029)
        {
            ar >> boolean; pPolar->setFreeSurfaceEffect(boolean);
        }
        ar >> dble; pPolar->setGroundHeight(dble);
        ar >> dble;
        ar >> d0 >> d1;
        pPolar->setDensity(d0);
        pPolar->setViscosity(d1);

        //ATTITUDE
        ar >> dble; pPolar->setBeta(dble);
        ar >> dble; pPolar->setPhi(dble);

        // DEFAULT INERTIA DATA
        ar >> boolean; pPolar->setAutoInertia(boolean);
        ar >> dble; pPolar->setMass(dble);
        ar >> d0 >> d1 >> d2;
        pPolar->setCoG({d0, d1, d2});

        ar >> d0 >> d1 >> d2 >> dble >> d3 >> dble;
        pPolar->setInertiaTensor(d0, d1, d2, d3);

        // VISCOSITY DATA
        ar >> boolean; pPolar->setViscous(boolean);
        ar >> boolean; // formerly bViscousOnTheFly
        ar >> d0 >> d1 >> d2;
        pPolar->setNCrit(d0);
        pPolar->setXTrTop(d1);
        pPolar->setXTrBot(d2);

        // fix test values from legacy file ????
        // m_NCrit=9.0;   m_XTrTop=1.0;   m_XTrBot=1.0;
        pPolar->setNCrit(9);
        pPolar->setXTrTop(1.0);
        pPolar->setXTrBot(1.0);


        // WAKE DATA
        ar >> k;   pPolar->setVortonWake(k ? true : false);
        ar >> k;    pPolar->setNXWakePanel4(k);
        ar >> dble; pPolar->setTotalWakeLengthFactor(dble);
        ar >> dble; pPolar->setWakePanelFactor(dble);;
        ar >> k; // m_nWakeIterations; m_nWakeIterations=std::max(m_nWakeIterations, 1);


        // space allocation
        ar >> nIntExtra;
        if(nIntExtra>=1)
        {
            ar >> n;
            pPolar->setViscFromCl(n==1 ? true : false);
            if(pPolar->analysisMethod()==xfl::LLT) pPolar->setViscFromCl(true);
        }
        if(nIntExtra>=2)
        {
            // 500006 format
            ar >> n;
        }
        for(int n=2; n<nIntExtra; n++) ar >> n;

        ar >> nDbleExtra;
        if(nDbleExtra>=2)
        {
            // 500006 format
            ar >> dble; pPolar->setVortonL0(dble);
            ar >> dble; // m_VortonXFactor;
        }
        if(nDbleExtra>=4)
        {
            // 500012 format
            ar >> dble; pPolar->setBufferWakeFactor(dble);
            ar >> dble; //m_VortonLengthFactor;
        }

        if(format>=500015)
        {
            // 500015 format
            ar >> dble; pPolar->setVortonCoreSize(dble);
        }

        if(format>=500016)
        {
            // 500016 format
            ar >> dble; pPolar->setVPWMaxLength(dble);
        }
        if(format>=500019)
        {
            // 500019 format
            ar >> k; pPolar->setVPWIterations(k);
        }
    }
    return true;
}


/** v7.50 - clean slate serialization*/
bool serial::serializePolar3dFl5v750(Polar3d *pPolar, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int integer(0);
    double dble(0.0), d0(0), d1(0), d2(0), d3(0);
    int n(0);
    QString strange;

    // 500750: v7.50 - clean slate serialization

    int format = 500750;
    pPolar->setPolarFormat(format);

    if(bIsStoring)
    {
        //output the variables to the stream
        ar << format;

        //METADATA
        ar << QString::fromStdString(pPolar->name());

        serial::serializeLineStyleFl5(pPolar->theStyle(), ar, bIsStoring);


        //ANALYSIS METHOD
        if     (pPolar->analysisMethod()==xfl::LLT)        ar<<1;
        else if(pPolar->analysisMethod()==xfl::VLM1)       ar<<2;
        else if(pPolar->analysisMethod()==xfl::VLM2)       ar<<3;
        else if(pPolar->analysisMethod()==xfl::QUADS)      ar<<4;
        else if(pPolar->analysisMethod()==xfl::TRILINEAR)  ar<<5;
        else if(pPolar->analysisMethod()==xfl::TRIUNIFORM) ar<<6;
        else                                               ar<<0;

        ar << pPolar->bTrefftz();
        ar << (pPolar->boundaryCondition()==xfl::DIRICHLET);
        ar << pPolar->bIgnoreBodyPanels();

        //POLAR TYPE
        if     (pPolar->type()==xfl::T1POLAR) ar<<1;
        else if(pPolar->type()==xfl::T2POLAR) ar<<2;
        else if(pPolar->type()==xfl::T3POLAR) ar<<3;
        else if(pPolar->type()==xfl::T4POLAR) ar<<4;
        else if(pPolar->type()==xfl::T5POLAR) ar<<5;
        else if(pPolar->type()==xfl::T6POLAR) ar<<6;
        else if(pPolar->type()==xfl::T7POLAR) ar<<7;
        else if(pPolar->type()==xfl::T8POLAR) ar<<8;
        else                                  ar<<0;

        // EXTRADRAGDATA
        int nDrag = pPolar->extraDragCount();
        ar << nDrag;
        for (int ix=0; ix<nDrag; ix++)
        {
            ExtraDrag const &drag = pPolar->extraDrag(ix);
            ar << QString::fromStdString(drag.name()) << drag.area() << drag.coef();
        }


        // AERO DATA
        ar << pPolar->bGroundEffect() << pPolar->bFreeSurfaceEffect();
        ar << pPolar->groundHeight() << dble;
        ar << pPolar->density() << pPolar->viscosity();

        //ATTITUDE
        ar << pPolar->betaSpec();
        ar << pPolar->phi();


        // DEFAULT INERTIA
        ar << pPolar->bAutoInertia();
        ar << pPolar->mass();
        ar << pPolar->CoG().x<< pPolar->CoG().y << pPolar->CoG().z;
        ar << pPolar->Ixx() << pPolar->Iyy()  << pPolar->Izz()  << dble << pPolar->Ixz() << dble;

        // VISCOSITY DATA
        ar << pPolar->isViscous();
        ar << pPolar->isViscOnTheFly();
        ar << pPolar->NCrit() << pPolar->XTrTop() << pPolar->XTrBot();

        // WAKE DATA
        ar << pPolar->bVortonWake();
        ar << pPolar->NXWakePanel4() << pPolar->totalWakeLengthFactor() << pPolar->wakePanelFactor();

        ar << pPolar->isViscFromCl();
        ar << pPolar->vortonL0();
        ar << pPolar->bufferWakeFactor();
        ar << pPolar->vortonCoreSize();
        ar << pPolar->VPWMaxLength();
        ar << pPolar->VPWIterations();

        // converted v7.54
        ar << pPolar->bTransAtHinge();

        // provisions for future variable saves
        for(int i=1; i<10; i++) ar <<boolean;
        for(int i=0; i<20; i++) ar <<integer;
        for(int i=0; i<20; i++) ar <<dble;

        return true;
    }
    else
    {
        //input the variables from the stream
        ar >> format;
        if((format < 500750) || (format>501000)) return false; // failsafe

        pPolar->setPolarFormat(format);

        // METADATA
        ar >> strange;   pPolar->setName(strange.toStdString());

        serial::serializeLineStyleFl5(pPolar->theStyle(), ar, bIsStoring);

        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) pPolar->setAnalysisMethod(xfl::LLT);
        else if(n==2) pPolar->setAnalysisMethod(xfl::VLM1);
        else if(n==3) pPolar->setAnalysisMethod(xfl::VLM2);
        else if(n==4) pPolar->setAnalysisMethod(xfl::QUADS);
        else if(n==5) pPolar->setAnalysisMethod(xfl::TRILINEAR);
        else if(n==6) pPolar->setAnalysisMethod(xfl::TRIUNIFORM);


        ar >> boolean; pPolar->setTrefftz(boolean);

        ar >> boolean;
        pPolar->setBoundaryCondition(boolean ? xfl::DIRICHLET : xfl::NEUMANN);

        ar >> boolean; pPolar->setIgnoreBodyPanels(boolean);

        // POLAR TYPE
        ar >> n;
        if     (n==1)   pPolar->setType(xfl::T1POLAR);
        else if(n==2)   pPolar->setType(xfl::T2POLAR);
        else if(n==3)   pPolar->setType(xfl::T3POLAR);
        else if(n==4)   pPolar->setType(xfl::T4POLAR);
        else if(n==5)   pPolar->setType(xfl::T5POLAR);
        else if(n==6)   pPolar->setType(xfl::T6POLAR);
        else if(n==7)   pPolar->setType(xfl::T7POLAR);
        else if(n==8)   pPolar->setType(xfl::T8POLAR);

        // EXTRADRAGDATA
        int nDrag = pPolar->extraDragCount();
        ar >> nDrag;
        pPolar->resizeExtraDrag(nDrag);
        for (int ix=0; ix<nDrag; ix++)
        {
            ExtraDrag &drag = pPolar->extraDrag(ix);
            ar >> strange;        drag.m_Tag = strange.toStdString();
            ar >> dble;  drag.m_Area = dble;
            ar >> dble;  drag.m_Coef = dble;
        }

        // AERO DATA
        ar >> boolean; pPolar->setGroundEffect(boolean);
        ar >> boolean; pPolar->setFreeSurfaceEffect(boolean);
        ar >> dble; pPolar->setGroundHeight(dble);
        ar >> dble;
        ar >> d0 >> d1;
        pPolar->setDensity(d0);
        pPolar->setViscosity(d1);

        //ATTITUDE
        ar >> dble; pPolar->setBeta(dble);
        ar >> dble; pPolar->setPhi(dble);


        // DEFAULT INERTIA
        ar >> boolean; pPolar->setAutoInertia(boolean);
        ar >> dble; pPolar->setMass(dble);
        ar >> d0 >> d1 >> d2;
        pPolar->setCoG({d0, d1, d2});
        ar >> d0 >> d1 >> d2 >> dble >> d3 >> dble;
        pPolar->setInertiaTensor(d0, d1, d2, d3);

        // VISCOSITY DATA
        ar >> boolean; pPolar->setViscous(boolean);
        ar >> boolean; pPolar->setViscOnTheFly(boolean);
        ar >> d0 >> d1 >> d2;
        pPolar->setNCrit(d0);
        pPolar->setXTrTop(d1);
        pPolar->setXTrBot(d2);

        // WAKE DATA
        ar >> boolean; pPolar->setVortonWake(boolean);
        ar >> n; pPolar->setNXWakePanel4(n);
        ar >> dble; pPolar->setTotalWakeLengthFactor(dble);
        ar >> dble; pPolar->setWakePanelFactor(dble);

        ar >> boolean; pPolar->setViscFromCl(boolean);
        ar >> dble; pPolar->setVortonL0(dble);
        ar >> dble; pPolar->setBufferWakeFactor(dble);
        ar >> dble; pPolar->setVortonCoreSize(dble);
        ar >> dble; pPolar->setVPWMaxLength(dble);
        ar >> n; pPolar->setVPWIterations(n);


        // converted v7.54
        ar >> boolean; pPolar->setTransAtHinge(boolean);

        // provisions for future variable saves
        for(int i=1; i<10; i++) ar >> boolean;
        for(int i=0; i<20; i++) ar >> integer;
        for(int i=0; i<20; i++) ar >> dble;

        return true;
    }
}


bool serial::serializePartFl5(Part *pPart, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble(0), d0(0), d1(0);
    QString strange;

    //500001: new fl5 format;
    //500002: added m_bReversed
    //500754: added GmshParams;
    //500755: modified GmshParams to avoid excessively small tessellation elements;

    int ArchiveFormat = 500755;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(pPart->name());
        ar << QString::fromStdString(pPart->description());

        serial::serializeLineStyleFl5(pPart->theStyle(), ar, bIsStoring);

        serial::serializeInertiaFl5(pPart->inertia(), ar, bIsStoring);

        ar << pPart->bAutoInertia();
        bool bReverse=false; // deprecated
        ar << bReverse; // added format 500002

        ar << pPart->gmshTessParams().m_MinSize;
        ar << pPart->gmshTessParams().m_MaxSize;
        ar << pPart->gmshTessParams().m_nCurvature;

        ar << pPart->gmshParams().m_MinSize;
        ar << pPart->gmshParams().m_MaxSize;
        ar << pPart->gmshParams().m_nCurvature;

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
        if(ArchiveFormat<500000 || ArchiveFormat>501000) return false;

        ar >> strange;    pPart->setName(strange.toStdString());
        ar >> strange;    pPart->setDescription(strange.toStdString());

        serial::serializeLineStyleFl5(pPart->theStyle(), ar, bIsStoring);

        serial::serializeInertiaFl5(pPart->inertia(), ar, bIsStoring);

        ar >> boolean; pPart->setAutoInertia(boolean);

        if(ArchiveFormat>=500002)
        {
            bool bReverse = false; // deprecated
            ar >> bReverse;
        }

        if(ArchiveFormat>=500754)
        {
            ar >> d0 >> d1 >> n;

            if(ArchiveFormat<500755)
            {
                d0 = std::max(d0, 0.02); // could be 0.5 mm by default in v7.54
            }

            if(d0<0.0005) d0 = 0.0005; // avoid excessively long tessellations
            if(d1<0.001)  d1 = 10.0; // cleaning past errors
            if(n<=0) n=23; // cleaning past errors

            pPart->setGmshTessParams({d0,d1,n});


            ar >> d0 >> d1 >> n;
            //            if(m_GmshParams.m_MinSize<0.0001) m_GmshParams.m_MinSize = 0.0001; // avoid excessively long meshing times
            if(d1<0.001)  d1 = 10.0; // cleaning past errors
            if(n<=0)      n= 23; // cleaning past errors
            pPart->setGmshParams({d0,d1,n});
        }

        // space allocation
        ar >> nIntSpares;
        if(nIntSpares<0 || nIntSpares>100) return false;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        if(nDbleSpares<0 || nDbleSpares>100) return false;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}

bool serial::serializeSailFl5(Sail *pSail, QDataStream &ar, bool bIsStoring)
{
    serial::serializePartFl5(pSail, ar, bIsStoring);
    bool boolean(false);
    int n=0;
    double dble(0), d0(0), d1(0), d2(0);

    // 500001: new fl5 format;
    // 500002: beta 17: added Clew-Peak-Head-Tack
    // 500003: beta 18: added reference the area
    // 500004: beta 18: added reference the reference chord
    // 500005: beta 19: added free mesh parameters
    // 500006: v7.03:   added edge split parameters

    int ArchiveFormat=500006;// identifies the format of the file
    int nIntSpares=0;
    int nDbleSpares=0;

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        ar << pSail->refArea() << pSail->refChord();

        ar << pSail->isThinSurface();
        ar << pSail->bRuledMesh();
        ar << pSail->maxElementSize();

        ar << pSail->nXPanels();
        ar << 11; // formerly NZPanels
        switch(pSail->xDistType())
        {
            default:
            case xfl::UNIFORM:       n=0;  break;
            case xfl::COSINE:        n=1;  break;
            case xfl::SINE:          n=2;  break;
            case xfl::INV_SINE:      n=3;  break;
            case xfl::INV_SINH:      n=4;  break;
            case xfl::TANH:          n=5;  break;
            case xfl::EXP:           n=6;  break;
            case xfl::INV_EXP:       n=7;  break;
        }
        ar << n;

        n=0;
        ar << n; // formerly m_ZDist

        ar << pSail->position().x << pSail->position().y << pSail->position().z;

        ar << pSail->clew().x << pSail->clew().y << pSail->clew().z;
        ar << pSail->peak().x << pSail->peak().y << pSail->peak().z;
        ar << pSail->tack().x << pSail->tack().y << pSail->tack().z;
        ar << pSail->head().x << pSail->head().y << pSail->head().z;

        ar << int(pSail->edgeSplit().size());
        for(uint i=0; i<pSail->edgeSplit().size(); i++)
        {
            std::vector<EdgeSplit> &split = pSail->edgeSplit()[i];
            ar << int(split.size());
            for(uint j=0; j<split.size(); j++)
                serial::serializeEdgeSplit(split[j], ar, bIsStoring);
        }

        ar << 0; // nIntSpares
        ar << 0; // nDoubleSpares
        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat<500000 || ArchiveFormat>510000)  return false;

        if(ArchiveFormat>=500003)
        {
            ar >> dble;
            pSail->setRefArea(dble);
        }
        if(ArchiveFormat>=500004)
        {
            ar >> dble;
            pSail->setRefChord(dble);
        }
        if(ArchiveFormat>=500005)
        {
            ar >> boolean; pSail->setThinSurface(boolean);
            ar >> boolean; pSail->setRuledMesh(boolean);
            ar >> dble;    pSail->setMaxElementSize(dble);
        }

        ar >> n; pSail->setNXPanels(n);
        ar >> n; // m_NZPanels;
        ar >> n;
        switch(n)
        {
            default:
            case 0: pSail->setXDistType(xfl::UNIFORM);      break;
            case 1: pSail->setXDistType(xfl::COSINE);       break;
            case 2: pSail->setXDistType(xfl::SINE);         break;
            case 3: pSail->setXDistType(xfl::INV_SINE);     break;
            case 4: pSail->setXDistType(xfl::INV_SINH);     break;
            case 5: pSail->setXDistType(xfl::TANH);         break;
            case 6: pSail->setXDistType(xfl::EXP);          break;
            case 7: pSail->setXDistType(xfl::INV_EXP);      break;
        }

        if(ArchiveFormat<=500006)
        {
            ar >> n;
            /*            switch(n)
            {
                default:
                case 0: m_ZDist=Xfl::UNIFORM;      break;
                case 1: m_ZDist=Xfl::COSINE;       break;
                case 2: m_ZDist=Xfl::SINE;         break;
                case 3: m_ZDist=Xfl::INV_SINE;     break;
                case 4: m_ZDist=Xfl::INV_SINH;     break;
                case 5: m_ZDist=Xfl::TANH;         break;
                case 6: m_ZDist=Xfl::EXP;          break;
                case 7: m_ZDist=Xfl::INV_EXP;      break;
            }*/
        }

        ar >> d0 >> d1>> d2;
        pSail->setPosition(d0, d1, d2);

        if(ArchiveFormat>=500002)
        {
            ar >> d0 >> d1>> d2;
            pSail->setClew({d0, d1, d2});
            ar >> d0 >> d1>> d2;
            pSail->setPeak({d0, d1, d2});
            ar >> d0 >> d1>> d2;
            pSail->setTack({d0, d1, d2});
            ar >> d0 >> d1>> d2;
            pSail->setHead({d0, d1, d2});
        }

        if(ArchiveFormat>=500006)
        {
            ar >> n;
            if(n==0)
            {
                // update legacy projects
                n=4;
                pSail->edgeSplit().resize(n);
                pSail->edgeSplit().front().resize(4);
            }
            else
            {
                pSail->edgeSplit().resize(n);
                for(uint i=0; i<pSail->edgeSplit().size(); i++)
                {
                    ar >> n;
                    pSail->edgeSplit()[i].resize(n);
                    for(uint j=0; j<pSail->edgeSplit()[i].size(); j++)
                        serial::serializeEdgeSplit(pSail->edgeSplit()[i][j], ar, bIsStoring);
                }
            }
        }

        // space allocation
        ar >> nIntSpares;
        ar >> nDbleSpares;

        return true;
    }
}


bool serial::serializeWingSailSectionFl5(WingSailSection *pSec, QDataStream &ar, bool bIsStoring)
{
    int n(0), k(0);
    int ArchiveFormat=500001;// identifies the format of the file
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    QString strange;

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;
        ar << QString::fromStdString(pSec->foilName());
        ar << pSec->m_NXPanels << pSec->m_NZPanels;
        ar << pSec->m_Chord << pSec->m_Twist;

        switch(pSec->m_XPanelDist)
        {
            case xfl::COSINE:      ar <<  1;  break;
            case xfl::SINE:        ar <<  2;  break;
            case xfl::INV_SINE:    ar << -2;  break;
            case xfl::INV_SINH:    ar <<  3;  break;
            case xfl::TANH:        ar <<  4;  break;
            case xfl::EXP:         ar <<  5;  break;
            case xfl::INV_EXP:     ar <<  6;  break;
            case xfl::UNIFORM:
            default:               ar <<  0;  break;
        }

        switch(pSec->m_ZPanelDist)
        {
            case xfl::COSINE:      ar <<  1;  break;
            case xfl::SINE:        ar <<  2;  break;
            case xfl::INV_SINE:    ar << -2;  break;
            case xfl::INV_SINH:    ar <<  3;  break;
            case xfl::TANH:        ar <<  4;  break;
            case xfl::EXP:         ar <<  5;  break;
            case xfl::INV_EXP:     ar <<  6;  break;
            case xfl::UNIFORM:
            default:               ar <<  0;  break;
        }

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
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat!=500001)  return false;

        ar >> strange;  pSec->m_FoilName = strange.toStdString();
        ar >> pSec->m_NXPanels >> pSec->m_NZPanels;
        ar >> pSec->m_Chord >> pSec->m_Twist;

        ar >> k;
        if     (k==1)  pSec->m_XPanelDist = xfl::COSINE;
        else if(k==2)  pSec->m_XPanelDist = xfl::SINE;
        else if(k==-2) pSec->m_XPanelDist = xfl::INV_SINE;
        else if(k==3)  pSec->m_XPanelDist = xfl::INV_SINH;
        else if(k==4)  pSec->m_XPanelDist = xfl::TANH;
        else if(k==5)  pSec->m_XPanelDist = xfl::EXP;
        else if(k==6)  pSec->m_XPanelDist = xfl::INV_EXP;
        else           pSec->m_XPanelDist = xfl::UNIFORM;

        ar >> k;
        if     (k==1)  pSec->m_ZPanelDist = xfl::COSINE;
        else if(k==2)  pSec->m_ZPanelDist = xfl::SINE;
        else if(k==-2) pSec->m_ZPanelDist = xfl::INV_SINE;
        else if(k==3)  pSec->m_ZPanelDist = xfl::INV_SINH;
        else if(k==4)  pSec->m_ZPanelDist = xfl::TANH;
        else if(k==5)  pSec->m_ZPanelDist = xfl::EXP;
        else if(k==6)  pSec->m_ZPanelDist = xfl::INV_EXP;
        else           pSec->m_ZPanelDist = xfl::UNIFORM;

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        return true;
    }
}


bool serial::serializeSailWingFl5(SailWing *pSail, QDataStream &ar, bool bIsStoring)
{
    serial::serializeSailFl5(pSail, ar, bIsStoring);

    int n(0);
    int ArchiveFormat=500001;// identifies the format of the file
    double dble(0);
    double d0(0), d1(0), d2(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        ar<<pSail->sectionCount();
        for(int i=0; i<pSail->sectionCount(); i++)
        {
            serial::serializeWingSailSectionFl5(&pSail->section(i), ar, bIsStoring);
            ar << pSail->sectionPosition(i).x<<pSail->sectionPosition(i).y<<pSail->sectionPosition(i).z;
            ar << pSail->sectionAngle(i);
        }

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
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat!=500001)  return false;

        ar>>n;
        pSail->resizeSections(n);

        for(int i=0; i<n; i++)
        {
            serial::serializeWingSailSectionFl5(&pSail->section(i), ar, bIsStoring);
            ar >> d0 >> d1 >> d2;
            pSail->setSectionPosition(i, {d0,d1,d2});
            ar >> dble; pSail->setSectionAngle(i, dble);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        pSail->makeSurface();

        return true;
    }
}


bool serial::serializeSailSTLFl5(SailStl*pSail, QDataStream &ar, bool bIsStoring)
{
    serial::serializeSailFl5(pSail, ar, bIsStoring);

    bool boolean(false);
    int k=0, n=0;
    float xf=0,yf=0,zf=0;
    Vector3d V0, V1, V2;

    // 500001: new fl5 format
    // 500002: added top TE indexes in beta 18
    // 500003: brand new format
    int ArchiveFormat = 500003;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << pSail->isThinSurface();
        ar << int(pSail->triangles().size());
        for(uint i=0; i<pSail->triangles().size(); i++)
        {
            Triangle3d const &t3d = pSail->triangleAt(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        ar << int(pSail->botMidTEIndexes().size());
        for(int idx : pSail->botMidTEIndexes())   ar << idx;

        ar << int(pSail->topTEIndexes().size());
        for(int idx : pSail->topTEIndexes())      ar << idx;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>510000) return false;
        if(ArchiveFormat<=500002)
        {
            // deprecated
            serial::serializeTriangulationFl5(pSail->triangulation(), ar, bIsStoring);
            pSail->triangulation().makeNodes();
            pSail->triangulation().makeNodeNormals();
            pSail->setTriangles(pSail->triangulation().triangles());

            if(ArchiveFormat>=500002)
            {
                ar >> boolean; pSail->setThinSurface(boolean);
            }

            pSail->clearTEIndexes();

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->botMidTEIndexes().push_back(k);
            }

            if(ArchiveFormat>=500002)
            {
                ar >> n;
                for(int i=0; i<n; i++)
                {
                    ar >> k;
                    pSail->topTEIndexes().push_back(k);
                }
            }
        }
        else
        {
            ar >> boolean; pSail->setThinSurface(boolean);
            ar >> n;
            pSail->triangles().resize(n);
            for(int i3=0; i3<n; i3++)
            {
                ar >> xf >> yf >> zf;
                V0.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V1.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V2.set(double(xf), double(yf), double(zf));

                pSail->triangles()[i3].setTriangle(V0, V1, V2);
            }

            pSail->triangulation().setTriangles(pSail->triangles());
            pSail->triangulation().makeNodes();
            pSail->triangulation().makeNodeNormals();

            pSail->clearTEIndexes();

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->botMidTEIndexes().push_back(k);
            }

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->topTEIndexes().push_back(k);
            }
        }

        pSail->computeProperties();
        pSail->updateStations();
    }
    return true;
}


bool serial::serializeSailSplineFl5(SailSpline *pSail, QDataStream &ar, bool bIsStoring)
{
    if(!serial::serializeSailFl5(pSail, ar, bIsStoring)) return false;

    int k(0), n(0);
    float xf(0), yf(0), zf(0);
    double d0(0), d1(0), d2(0);
    Vector3d V0, V1, V2;

    //500001: first .fl5 format
    //500002: added refpanels in beta19
    //500003: v7.03 added NZPanels and ZDist arrays
    int ArchiveFormat=500003;// identifies the format of the file

    int nIntSpares=0;
    int nDbleSpares=0;

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        switch(pSail->splineType())
        {
                default:
                case Spline::BSPLINE:   ar<<0;    break;
                case Spline::CUBIC:     ar<<1;    break;
                case Spline::BEZIER:    ar<<2;    break;
                case Spline::POINT:     ar<<3;    break;
                case Spline::ARC:       ar<<4;    break;
                case Spline::NACA4:     ar<<5;    break;
        }

        ar<<pSail->nSections();
        for(int i=0; i<pSail->sectionCount(); i++)
        {
            if      (pSail->spline(i)->isBSpline())
            {
                ar<<1;
                BSpline *pBS = dynamic_cast<BSpline*>(pSail->spline(i));
                serial::serializeBSplineFl5(pBS, ar, bIsStoring);
            }
            else if (pSail->spline(i)->isBezierSpline())
            {
                ar<<2;
                BezierSpline *pBzS = dynamic_cast<BezierSpline*>(pSail->spline(i));
                serial::serializeBezierSplineFl5(pBzS, ar, bIsStoring);
            }
            else if (pSail->spline(i)->isCubicSpline())
            {
                ar<<3;
                CubicSpline *pC3S = dynamic_cast<CubicSpline*>(pSail->spline(i));
                serial::serializeCubicSplineFl5(pC3S, ar, bIsStoring);
            }
            else if (pSail->spline(i)->isPointSpline())
            {
                ar<<4;
                PointSpline *pPtS = dynamic_cast<PointSpline*>(pSail->spline(i));
                serial::serializePointSplineFl5(pPtS, ar, bIsStoring);
            }

            ar << pSail->sectionPosition(i).x<<pSail->sectionPosition(i).y<<pSail->sectionPosition(i).z;
            ar << pSail->sectionAngle(i);
            n=1; ar <<n;  //          ar << m_nZPanels.at(i);
            n=0; ar <<n;  //          ar <<  xfl::UNIFORM;  break;
        }

        ar << int(pSail->triangles().size());
        for(uint i=0; i<pSail->triangles().size(); i++)
        {
            Triangle3d const &t3d = pSail->triangles().at(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        ar << int(pSail->botMidTEIndexes().size());
        for(int idx : pSail->botMidTEIndexes())   ar << idx;

        ar << int(pSail->topTEIndexes().size());
        for(int idx : pSail->topTEIndexes())      ar << idx;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;

        nDbleSpares=0;
        ar << nDbleSpares;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if ((ArchiveFormat<500001) || (ArchiveFormat>500100))  return false;

        ar >> n;
        switch(n)
        {
            default:
            case 0: pSail->setSplineType(Spline::BSPLINE);       break;
            case 1: pSail->setSplineType(Spline::CUBIC);         break;
            case 2: pSail->setSplineType(Spline::BEZIER);        break;
            case 3: pSail->setSplineType(Spline::POINT);         break;
            case 4: pSail->setSplineType(Spline::ARC);           break;
            case 5: pSail->setSplineType(Spline::NACA4);         break;
        }

        ar>>n;
        pSail->resizeSections(n);

        for(int i=0; i<n; i++)
        {
            int type=0;
            ar >> type;
            switch (type)
            {
                case 1:
                {
                    BSpline *pBS = new BSpline;
                    serial::serializeBSplineFl5(pBS, ar, bIsStoring);
                    pSail->setSpline(i, pBS);
                    break;
                }
                case 2:
                {
                    BezierSpline *pBzS = new BezierSpline;
                    serial::serializeBezierSplineFl5(pBzS, ar, bIsStoring);
                    pSail->setSpline(i, pBzS);
                    break;
                }
                case 3:
                {
                    CubicSpline *pC3S = new CubicSpline;
                    serial::serializeCubicSplineFl5(pC3S, ar, bIsStoring);
                    pSail->setSpline(i, pC3S);
                    break;
                }
                case 4:
                {
                    PointSpline *pPtS = new PointSpline;
                    serial::serializePointSplineFl5(pPtS, ar, bIsStoring);
                    pSail->setSpline(i, pPtS);
                    break;
                }
                default:
                    return false;
            }

            ar>> d0 >> d1 >> d2;
            pSail->setSectionPosition(i, {d0, d1, d2});
            ar >> d0;
            pSail->setSectionAngle(i, d0);

            if(ArchiveFormat>=500003)
            {
                ar >> k;
                ar >> k;
            }
            else
            {
            }
        }

        if( ArchiveFormat>=500002)
        {
            ar >> n;
            pSail->triangles().resize(n);
            for(int i3=0; i3<n; i3++)
            {
                ar >> xf >> yf >> zf;
                V0.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V1.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V2.set(double(xf), double(yf), double(zf));

                pSail->triangles()[i3].setTriangle(V0, V1, V2);
            }

            pSail->clearTEIndexes();
            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->botMidTEIndexes().push_back(k);
            }

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->topTEIndexes().push_back(k);
            }
//            if(m_RefTriangles.size()==0) clearTEIndexes(); // clean-up past mistakes
        }
        else
        {
            pSail->makeRuledMesh(Vector3d());
        }

        // space allocation
        ar >> nIntSpares;
        ar >> nDbleSpares;

        pSail->makeSurface();
        pSail->updateStations();
        return true;
    }
}


bool serial::serializeSailOCCFl5(SailOcc *pSail, QDataStream &ar, bool bIsStoring)
{
    serial::serializeSailFl5(pSail, ar, bIsStoring);

    bool boolean(false);
    int k(0), n(0);
    double dble(0);
    float xf(0), yf(0), zf(0);

    Vector3d V0,V1,V2;

    // 500001: first fl5 format in beta18
    // 500002: added OccMeshParams in beta 18
    int ArchiveFormat = 500002;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pSail->isThinSurface();

        ar << int(pSail->botMidTEIndexes().size());
        for(int idx : pSail->botMidTEIndexes())
            ar << idx;

        ar << int(pSail->topTEIndexes().size());
        for(int idx : pSail->topTEIndexes())
            ar << idx;

        ar << pSail->maxElementSize();

        ar << int(pSail->shapes().Size());

        std::string brepstr;
        for(NCollection_List<TopoDS_Shape>::Iterator shapeit(pSail->shapes()); shapeit.More(); shapeit.Next())
        {
            occ::shapeToBrep(shapeit.Value(), brepstr);
            ar << QString::fromStdString(brepstr);
        }

        ar << int(pSail->triangles().size());
        for(uint i=0; i<pSail->triangles().size(); i++)
        {
            Triangle3d const &t3d = pSail->triangles().at(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        serial::serializeOccParams(pSail->occTessParams(), ar, bIsStoring);
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>500100) return false;

        ar >> boolean; pSail->setThinSurface(boolean);

        pSail->clearTEIndexes();
        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> k;
            pSail->botMidTEIndexes().push_back(k);
        }

        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> k;
            pSail->topTEIndexes().push_back(k);
        }

        ar >> dble; pSail->setMaxElementSize(dble);

        pSail->clearShapes();
        int nShapes;
        ar >> nShapes;


        pSail->bReps().resize(nShapes);
        QString strange;
        for(int iShape=0; iShape<nShapes; iShape++)
        {
            ar >> strange;   pSail->bReps()[iShape] = strange.toStdString();
            try
            {
                std::stringstream sstream;
                sstream << pSail->bReps().at(iShape).c_str();

                TopoDS_Shape shape;
                BRep_Builder aBuilder;
                BRepTools::Read(shape, sstream, aBuilder);
                if(shape.IsNull())
                {
                    std::cout <<"Error serializing CAD sail " << pSail->name() << std::endl;
                    // continue serializing
                }
                pSail->shapes().Append(shape);

            }
            catch(...)
            {
                std::cout<< "Error converting Brep for CAD sail " + pSail->name() << std::endl;
                return false;
            }

        }
        ar >> n;
        pSail->triangles().resize(n);
        for(int i3=0; i3<n; i3++)
        {
            ar >> xf >> yf >> zf;
            V0.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V1.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V2.set(double(xf), double(yf), double(zf));

            pSail->triangles()[i3].setTriangle(V0,V1, V2);
        }

        if(ArchiveFormat>=500002)
            serial::serializeOccParams(pSail->occTessParams(), ar, bIsStoring);

        pSail->setRuledMesh(false); // forced in v7.55

        pSail->makeSurface();
        pSail->computeProperties();
        pSail->updateStations();
    }
    return true;
}


bool serial::serializeSailNURBSFl5(SailNurbs *pSail, QDataStream &ar, bool bIsStoring)
{
    serial::serializeSailFl5(pSail, ar, bIsStoring);

//    bool boolean(false);
    int k(0), n(0);
    float xf(0),yf(0),zf(0);
    Vector3d V0, V1, V2;

    //500001: first .fl5 format
    //500002: added refpanels in beta19
    //500003: added EdgeSplits in v7.03
    int ArchiveFormat=500004;// identifies the format of the file

    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        serial::serializeNURBSFl5(&pSail->nurbs(), ar, bIsStoring);

        ar << int(pSail->triangles().size());
        for(uint i=0; i<pSail->triangles().size(); i++)
        {
            Triangle3d const &t3d = pSail->triangles().at(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        ar << int(pSail->botMidTEIndexes().size());
        for(int idx : pSail->botMidTEIndexes())   ar << idx;

        ar << int(pSail->topTEIndexes().size());
        for (int idx : pSail->topTEIndexes())      ar << idx;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;

        nDbleSpares=0;
        ar << nDbleSpares;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat<500001 || ArchiveFormat>500100)  return false;


        if(!serial::serializeNURBSFl5(&pSail->nurbs(), ar, bIsStoring)) return false;

        pSail->nurbs().setColor(pSail->color());


        if( ArchiveFormat>=500002)
        {
            ar >> n;
            pSail->triangles().resize(n);
            for(int i3=0; i3<n; i3++)
            {
                ar >> xf >> yf >> zf;
                V0.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V1.set(double(xf), double(yf), double(zf));

                ar >> xf >> yf >> zf;
                V2.set(double(xf), double(yf), double(zf));

                pSail->triangles()[i3].setTriangle(V0, V1, V2);
            }

            pSail->triangulation().setTriangles(pSail->triangles());
            pSail->triangulation().makeNodes();
            pSail->triangulation().makeNodeNormals();

            pSail->clearTEIndexes();
            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->botMidTEIndexes().push_back(k);
            }

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                pSail->topTEIndexes().push_back(k);
            }
        }


        if(ArchiveFormat<500004)
        {
            pSail->edgeSplit().resize(1);
            std::vector<EdgeSplit> & es = pSail->edgeSplit().front();
            es.resize(4);
            for(int iEdge=0; iEdge<4; iEdge++)
                serial::serializeEdgeSplit(es[iEdge], ar, bIsStoring);
        }

        // clean up past serialization errors
        if(pSail->edgeSplit().size()!=1) pSail->edgeSplit().resize(1); // 1 face in the case of a NURBS sail
        std::vector<EdgeSplit> & es = pSail->edgeSplit().front();
        if(es.size()!=4)
        {
            es.resize(4); // A nurbs has 4 edges
            es[0].setSplit(pSail->nZPanels(), xfl::UNIFORM);
            es[1].setSplit(pSail->nXPanels(), xfl::UNIFORM);
            es[2].setSplit(pSail->nZPanels(), xfl::UNIFORM);
            es[3].setSplit(pSail->nXPanels(), xfl::UNIFORM);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;


        pSail->makeSurface();
        pSail->updateStations();

        // compute luff angle
        Vector3d LE = pSail->nurbs().leadingEdgeAxis();
        pSail->setLuffAngle(atan2(LE.x, LE.z) * 180./PI);

        return true;
    }
}


bool serial::serializeBoatFl5(Boat *pBoat, QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;// identifies the format of the file
    int k(0), n(0);
    double dble(0);
    QString strange;

    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {    // storing code
        ar << 500009;
        //500001: initial format
        //500002: added theStyle in beta 17

        ar << QString::fromStdString(pBoat->name());
        ar << QString::fromStdString(pBoat->description());

        serial::serializeLineStyleFl5(pBoat->theStyle(), ar, bIsStoring);

        ar << pBoat->nSails();
        for(int is=0; is<pBoat->nSails(); is++)
        {
            Sail *pSail = pBoat->sail(is);
            if(!pSail) return false;

            if     (pSail->isSplineSail())
            {
                ar<<1;
                SailSpline *pSailSpline = dynamic_cast<SailSpline*>(pSail);
                if(!serial::serializeSailSplineFl5(pSailSpline, ar, bIsStoring)) return false;
            }
            else if     (pSail->isNURBSSail())
            {
                ar<<2;
                SailNurbs *pSailNurbs = dynamic_cast<SailNurbs*>(pSail);
                if(!serial::serializeSailNURBSFl5(pSailNurbs, ar, bIsStoring)) return false;
            }
            else if     (pSail->isWingSail())
            {
                ar<<3;
                SailWing *pSailWing = dynamic_cast<SailWing*>(pSail);
                if(!serial::serializeSailWingFl5(pSailWing, ar, bIsStoring)) return false;
            }
            else if     (pSail->isStlSail())
            {
                ar<<4;
                SailStl *pSailSTL = dynamic_cast<SailStl*>(pSail);
                if(!serial::serializeSailSTLFl5(pSailSTL, ar, bIsStoring)) return false;
            }
            else if     (pSail->isOccSail())
            {
                ar<<5;
                SailOcc *pSailOCC = dynamic_cast<SailOcc*>(pSail);
                if(!serial::serializeSailOCCFl5(pSailOCC, ar, bIsStoring)) return false;
            }
            else ar<<0;
        }

        ar << pBoat->nHulls();

        for(int ifuse=0; ifuse<pBoat->nHulls(); ifuse++)
        {
            Fuse *pHull = pBoat->hull(ifuse);
            if(pHull->isXflType())
            {
                if(pHull->fuseType()==Fuse::NURBS) ar << 100001;
                else                               ar << 100004; // flat faces
                FuseXfl *pBodyXfl = dynamic_cast<FuseXfl*>(pHull);
                serial::serializeFuseXflFl5(pBodyXfl, ar, true);
            }
            else if(pHull->isOccType())
            {
                ar << 100002;
                FuseOcc *pBodyOcc = dynamic_cast<FuseOcc*>(pHull);
                serial::serializeFuseOccFl5(pBodyOcc, ar, true);
            }
            else if(pHull->isStlType())
            {
                ar << 100003;
                FuseStl *pBodyStl = dynamic_cast<FuseStl*>(pHull);
                serial::serializeFuseStlFl5(pBodyStl, ar, true);
            }
            serial::serializePanelsFl5(pHull->triMesh(), ar, bIsStoring);
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;
        if (ArchiveFormat<500000 || ArchiveFormat>500100)  return false;

        ar >> strange;  pBoat->setName(strange.toStdString());
        ar >> strange;  pBoat->setDescription(strange.toStdString());
        if(ArchiveFormat>=500002)
            serial::serializeLineStyleFl5(pBoat->theStyle(), ar, bIsStoring);

        ar>>n;
        for(int is=0; is<n; is++)
        {
            ar >> k;
            if(k==1)
            {
                SailSpline *pSSail = new SailSpline;
                if(!serial::serializeSailSplineFl5(pSSail, ar, false)) return false;
                pBoat->appendSail(pSSail);
            }
            else if(k==2)
            {
                SailNurbs *pNSail = new SailNurbs;
                if(!serial::serializeSailNURBSFl5(pNSail, ar, false)) return false;
                pBoat->appendSail(pNSail);
            }
            else if(k==3)
            {
                SailWing *pWSail = new SailWing;
                if(!serial::serializeSailWingFl5(pWSail, ar, false)) return false;
                pBoat->appendSail(pWSail);
            }
            else if(k==4)
            {
                SailStl *pStlSail = new SailStl;
                if(!serial::serializeSailSTLFl5(pStlSail, ar, false)) return false;
                pBoat->appendSail(pStlSail);
            }
            else if(k==5)
            {
                SailOcc *pOccSail = new SailOcc;
                if(!serial::serializeSailOCCFl5(pOccSail, ar, false)) return false;
                pBoat->appendSail(pOccSail);
            }
            else return false;
        }

        int nFuse(0);
        ar >> nFuse;
        pBoat->clearHulls();

        for(int ifuse=0; ifuse<nFuse; ifuse++)
        {
            int format=0;
            ar >> format;
            if(format==100001)
            {
                FuseNurbs *pBodyNurbs = new FuseNurbs;
                serial::serializeFuseXflFl5(pBodyNurbs, ar, bIsStoring);
                if(pBodyNurbs) pBoat->appendHull(pBodyNurbs);
            }
            else if(format==100004)
            {
                FuseFlatFaces *pBodyFlat = new FuseFlatFaces;
                serial::serializeFuseXflFl5(pBodyFlat, ar, bIsStoring);
                if(pBodyFlat) pBoat->appendHull(pBodyFlat);
            }
            else if(format==100002)
            {
                FuseOcc *pBodyOcc = new FuseOcc;
                serial::serializeFuseOccFl5(pBodyOcc, ar, bIsStoring);
                if(pBodyOcc) pBoat->appendHull(pBodyOcc);
            }
            else if(format==100003)
            {
                FuseStl *pBodyStl = new FuseStl;
                serial::serializeFuseStlFl5(pBodyStl, ar, bIsStoring);
                if(pBodyStl) pBoat->appendHull(pBodyStl);
            }

            Fuse *pBackFuse = pBoat->hulls().back();

            serial::serializePanelsFl5(pBackFuse->triMesh(), ar, bIsStoring);

            // compatibility
            if(pBackFuse->nPanel3()==0)
            {
                std::string strange;
                pBackFuse->makeDefaultTriMesh(strange, "");
            }
            pBackFuse->setUniqueIndex();
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        return true;
    }
}


bool serial::serializeFuseFl5(Fuse*pFuse, QDataStream &ar, bool bIsStoring)
{
    serial::serializePartFl5(pFuse, ar, bIsStoring);

    double d0(0), d1(0), d2(0);
    int n=0;

    // 500001: new fl5 format;
    // 500003; added max element size in beta 12
    int ArchiveFormat = 500003;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pFuse->position().x << pFuse->position().y << pFuse->position().z;
        ar << pFuse->rx() << pFuse->ry() << pFuse->rz();

        serial::serializeOccParams(pFuse->occTessParams(), ar, bIsStoring);

        ar << pFuse->maxElementSize();

        n=0;
        ar << n;
        ar << n;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500003) return false;

        ar >> d0 >> d1 >> d2;
        pFuse->setPosition(d0, d1, d2);
        ar >> d0 >> d1 >> d2;
        pFuse->setRx(d0);
        pFuse->setRy(d1);
        pFuse->setRz(d2);


        if(ArchiveFormat>=500002)
            serial::serializeOccParams(pFuse->occTessParams(), ar, bIsStoring);

        if(ArchiveFormat>=500003)
        {
            ar >> d0; pFuse->setMaxElementSize(d0);
        }
        else
            pFuse->setMaxElementSize(pFuse->occTessParams().maxElementSize());

        ar >> n;
        ar >> n;
    }
    return true;
}


bool serial::serializeFuseStlFl5(FuseStl*pFuse, QDataStream &ar, bool bIsStoring)
{
    serial::serializeFuseFl5(pFuse, ar, bIsStoring);
    int k(0);
    float xf(0),yf(0),zf(0);
    double x(0),y(0),z(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    int n(0);
    double dble(0);

    Vector3d V0,V1,V2;

    // 500001: new fl5 format
    // 500002: storing variables as floats rather than doubles
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pFuse->nTriangles();
        for(int i=0; i<pFuse->nTriangles(); i++)
        {
            Triangle3d const &t3d = pFuse->triangleAt(i);
            xf = t3d.vertexAt(0).xf(); yf = t3d.vertexAt(0).yf(); zf = t3d.vertexAt(0).zf();
            ar << xf <<yf << zf;
            //            ar << t3d.vertex(0).xf() << t3d.vertex(0).yf() << t3d.vertex(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500010) return false;

        pFuse->clearTriangles();
        ar >> k;
        for(int i=0; i<k; i++)
        {
            if(ArchiveFormat<500002) { ar >> x  >> y  >> z;    V0.set(x,y,z);}
            else
            {
                ar >> xf >> yf >> zf;
                V0.set(double(xf), double(yf), double(zf));
            }
            if(ArchiveFormat<500002) { ar >> x  >> y  >> z;    V1.set(x,y,z);}
            else
            {
                ar >> xf >> yf >> zf;
                V1.set(double(xf), double(yf), double(zf));
            }
            if(ArchiveFormat<500002) { ar >> x  >> y  >> z;    V2.set(x,y,z);}
            else
            {
                ar >> xf >> yf >> zf;
                V2.set(double(xf), double(yf), double(zf));
            }
            Triangle3d t3(V0,V1,V2);
            pFuse->triangulation().appendTriangle(t3);
        }


        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        pFuse->makeFuseGeometry();
        pFuse->makeTriangleNodes();
        pFuse->makeNodeNormals();

        std::string logmsg;
        pFuse->makeDefaultTriMesh(logmsg, "");
    }
    return true;
}


bool serial::serializeFuseOccFl5(FuseOcc*pFuseOcc, QDataStream &ar, bool bIsStoring)
{
    serial::serializeFuseFl5(pFuseOcc, ar, bIsStoring);

    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble=0;

    // 500001: new fl5 format;
//    // 500002: serializing shells instead of shapes
    int ArchiveFormat = 500001;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        // v7.57: serializing shells instead of shapes
        ar<<pFuseOcc->nShells();

        std::stringstream sstream;
        for(NCollection_List<TopoDS_Shape>::Iterator shapeit(pFuseOcc->shells()); shapeit.More(); shapeit.Next())
        {
            sstream.str(std::string()); // clear the stream
            BRepTools::Write(shapeit.Value(), sstream); // stream the brep to the stringstream
            std::string string = sstream.str();
            QString brepstr = QString::fromStdString(string);
            ar << brepstr; // write the QString to the archive file
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

    }
    else
    {
        ar >> ArchiveFormat;
        if((ArchiveFormat<=500000) || (ArchiveFormat>500100)) return false; // failsafe

//        pFuse->clearShapes();
        // v7.57: serializing shells instead of shapes
        int nShapes;
        ar >> nShapes;
        TopoDS_ListOfShape shapes;
        for(int iShape=0; iShape<nShapes; iShape++)
        {
            QString brepstr;
            ar >> brepstr;
            try
            {
                std::stringstream sstream;
                sstream << brepstr.toStdString().c_str();

                TopoDS_Shape shape;
                BRep_Builder aBuilder;
                BRepTools::Read(shape, sstream, aBuilder);
                if(shape.IsNull())
                {
                    //                    qDebug()<<"Error serializing CAD fuse " + m_Name;
                    return false;
                }
                shapes.Append(shape);
            }
            catch(...)
            {
                //                qDebug()<<"Error converting Rrep for CAD fuse " + m_Name;
                return false;
            }
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        // v7.57: updating legacy models
        pFuseOcc->setShapes(shapes);
        pFuseOcc->extractShellsFromShapes();

        pFuseOcc->makeFuseGeometry();
    }
    return true;
}


bool serial::serializeFuseXFLXfl(FuseXfl *pFuse, QDataStream &ar, bool bIsStoring, int format)
{
    int i(0),k(0),n(0),p(0);

    double dble(0),m(0),px(0),py(0),pz(0);
    QString str;

    if(bIsStoring)
    {
    }
    else
    {
        if(format<100000 || format>200000) return false;

        NURBSSurface &nurbs = pFuse->nurbs();

        ar >> str;  pFuse->setName(str.toStdString());
        ar >> str;  pFuse->setDescription(str.toStdString());
        serial::serializefl5Color(pFuse->theStyle().m_Color, ar, false);

        ar >> k;
        if(k==1) pFuse->setFuseType(Fuse::FlatFace);
        else     pFuse->setFuseType(Fuse::NURBS);

        ar >> k; //m_iRes
        ar >> n >>p;
        pFuse->setNxNurbsPanels(n);
        pFuse->setNhNurbsPanels(p);
        ar >> dble;   nurbs.setBunchAmplitude(dble);

        pFuse->clearHPanels();
        ar >> n;
        for(k=0; k<n; k++)
        {
            ar >> p;
            pFuse->appendHPanel(p);
        }

        nurbs.clearFrames();
        pFuse->clearXPanels();
        ar >> n;
        for(k=0; k<n; k++)
        {
            nurbs.appendNewFrame();

            ar >> p;
            pFuse->appendXPanel(p);

            ar >> dble;
            nurbs.frame(k).setuPosition(pFuse->nurbs().uAxis(), dble);
            for(int ic=0; ic<nurbs.frame(k).nCtrlPoints(); ic++)
            {
                nurbs.frame(k).ctrlPoint(ic).x = dble;
            }

            serial::serializeFrameXfl(&nurbs.frame(k), ar, bIsStoring);
        }

        ar >> dble;
        pFuse->inertia().setStructuralMass(dble);

        pFuse->clearPointMasses();
        ar >> k;
        for(i=0; i<k; i++)
        {
            ar >> m >> px >> py >> pz;
            ar >> str;
            pFuse->inertia().appendPointMass(m, Vector3d(px, py, pz), str.toStdString());
        }

        // space allocation
        ar >> k; // m_bTextures = k ? true : false;
        ar >> k; // m_bReversed = k ? true : false;

        for (int i=2; i<18; i++) ar >> k;
        ar >> k;
        k = std::min(k, pFuse->frameCount()-1);
        k = std::max(k,2); k = std::min(k,5);
        nurbs.setuDegree(k);

        ar >> k;
        k = std::min(k, pFuse->framePointCount()-1);
        k = std::max(k,2); k = std::min(k,5);
        nurbs.setvDegree(k);

        for (int i=0; i<50; i++) ar >> dble;

        // make the shapes, shells, and triangulation

        pFuse->makeFuseGeometry();
        pFuse->computeStructuralInertia(Vector3d());

        // make the triangular mesh
        std::string strange;
        pFuse->makeDefaultTriMesh(strange, "");
    }
    return true;
}


bool serial::serializeFuseXflFl5(FuseXfl*pFuse, QDataStream &ar, bool bIsStoring)
{
    serial::serializeFuseFl5(pFuse, ar, bIsStoring);

    int k(0), l(0);
    int n(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    double dble(0);
    // 500001 : new fl5 format
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        if     (pFuse->fuseType()==Fuse::FlatFace) ar << 1;
        else if(pFuse->fuseType()==Fuse::NURBS)    ar << 2;
        else if(pFuse->fuseType()==Fuse::Sections) ar << 3;

        // point and spline data
        serial::serializeNURBSFl5(&pFuse->nurbs(), ar, bIsStoring);

        //mesh data
        ar << pFuse->nxNurbsPanels() << pFuse->nhNurbsPanels();
        for(k=0; k<pFuse->sideLineCount(); k++) ar << pFuse->hPanels(k);
        for(k=0; k<pFuse->frameCount();    k++) ar << pFuse->xPanels(k);

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

        ar >> k;
        switch(k)
        {
            case 1: pFuse->setFuseType(Fuse::FlatFace);     break;
            default:
            case 2: pFuse->setFuseType(Fuse::NURBS);        break;
            case 3: pFuse->setFuseType(Fuse::Sections);     break;
        }

        // point and spline data
        serial::serializeNURBSFl5(&pFuse->nurbs(), ar, bIsStoring);

        //mesh data
        ar >> k >> l;
        pFuse->setNxNurbsPanels(k);
        pFuse->setNhNurbsPanels(l);

        pFuse->resizeHPanels(pFuse->sideLineCount());
        for(k=0; k<pFuse->sideLineCount(); k++)
        {
            ar >> l; pFuse->setHPanels(k, l);
        }

        pFuse->resizeXPanels(pFuse->frameCount());
        for(k=0; k<pFuse->frameCount(); k++)
        {
            ar >> l; pFuse->setXPanels(k,l);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        // make the shapes, shells, and triangulation
        pFuse->makeFuseGeometry();
        pFuse->computeStructuralInertia(Vector3d());

        // make the default triangular mesh
        std::string strange;
        pFuse->makeDefaultTriMesh(strange, "");
    }
    return true;
}

bool serial::serializeFuseSectionFl5(FuseSections *pFuse, QDataStream &ar, bool bIsStoring)
{
    serial::serializeFuseXflFl5(pFuse, ar, bIsStoring);
    int k=0;
    int n=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    double dble=0;

    // 500001: new fl5 format
    // 500002: added fit precision
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pFuse->nSections();
        if(pFuse->nSections()>0)
            ar << int(pFuse->sections().front().size());

        for(int is=0; is<pFuse->nSections(); is++)
        {
            std::vector<Vector3d> const & section = pFuse->section(is);
            for(int ic=0; ic<int(section.size()); ic++)
            {
                Vector3d const & pt = section.at(ic);
                ar << pt.x << pt.y << pt.z;
            }
        }

        ar << pFuse->fitPrecision();

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
        if((ArchiveFormat<500001) || (ArchiveFormat>500002)) return false;

        ar >> n;
        pFuse->sections().resize(n);

        if(n>0)
        {
            ar >> k;
            for(int i=0; i<n; i++) pFuse->section(i).resize(k);
        }

        for(int is=0; is<pFuse->nSections(); is++)
        {
            std::vector<Vector3d> &section = pFuse->section(is);
            for(int ic=0; ic<int(section.size()); ic++)
            {
                Vector3d & pt = section[ic];
                ar >> pt.x >> pt.y >> pt.z;
            }
        }

        if(ArchiveFormat>=500002)
        {
            ar >> dble; pFuse->setFitPrecision(dble);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        // make the shapes, shells, and triangulation
        pFuse->makeFuseGeometry();
        pFuse->computeStructuralInertia(Vector3d());

        // make the default triangular mesh
        std::string strange;
        pFuse->makeDefaultTriMesh(strange, "");
    }

    return true;
}



bool serial::serializeWingXflXfl(WingXfl *pWing, QDataStream &ar, bool bIsStoring)
{
    QString tag;
    QString rightfoil, leftfoil;
    bool boolean(false);
    int nx(0), ny(0);
    int k(0), n(0);
    int ArchiveFormat(0);// identifies the format of the file
    double dble(0), dm(0), px(0), py(0), pz(0);
    double cord(0), tw(0), pos(0), dih(0), off(0);
    QString strange;
    xfl::enumDistribution xDist=xfl::COSINE, yDist=xfl::UNIFORM;

    if(bIsStoring)
    {
        // using flow5 format instead
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<100000 || ArchiveFormat>100003) return false;

        ar >> strange;  pWing->setName(strange.toStdString());
        ar >> strange;  pWing->setDescription(strange.toStdString());

        serial::serializefl5Color(pWing->theStyle().m_Color, ar, false);

        ar >> boolean; pWing->setSymmetric(boolean);

        pWing->clearSections();
        ar >> n;
        for (int i=0; i<n; i++)
        {
            ar >> rightfoil;
            ar >> leftfoil;
            ar >> cord;
            ar >> pos;
            ar >> off;
            ar >> dih;
            ar >> tw;
            ar >> nx;
            ar >> ny;

            ar >> k;
            if(k==1)       xDist = xfl::COSINE;
            else if(k== 2) xDist = xfl::SINE;
            else if(k==-2) xDist = xfl::INV_SINE;
            else           xDist = xfl::UNIFORM;

            ar >> k;
            if(k==1)       yDist = xfl::COSINE;
            else if(k== 2) yDist = xfl::SINE;
            else if(k==-2) yDist = xfl::INV_SINE;
            else           yDist = xfl::UNIFORM;

            pWing->appendSection(cord, tw, pos, dih, off, nx, ny, xDist, yDist, rightfoil.toStdString(), leftfoil.toStdString());
        }

        // improve uniformity of nx panel numbers
        if(n>=2)
        {
            int nx = pWing->section(n-2).nXPanels();
            pWing->sections().back().setNX(nx);
        }

        ar >> dble;
        pWing->setStructuralMass(dble);

        pWing->clearPointMasses();
        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> dm >> px >> py >> pz;
            ar >> tag;
            pWing->appendPointMass(PointMass(dm, Vector3d(px, py, pz), tag.toStdString()));
        }

        ar>>k; // if(k) m_bTextures=true; else m_bTextures=false;


        // space allocation
        for (int i=1; i<15; i++) ar >> k;
        ar>>k;  // m_bIsFin     = k? true : false; deprecated
        ar>>k;  pWing->setTwoSided(k? true : false);
        ar>>k; // m_bSymFin    = k? true : false;
        ar>>k;
        switch (k)
        {
            case 0:
                pWing->setWingType(xfl::Main);
                if(ArchiveFormat<100002) pWing->setTwoSided(true);
                break;
            case 1:
                pWing->setWingType(xfl::OtherWing);
                if(ArchiveFormat<100002) pWing->setTwoSided(true);
                break;
            case 2:
                pWing->setWingType(xfl::Elevator);
                if(ArchiveFormat<100002) pWing->setTwoSided(true);
                break;
            case 3:
                pWing->setWingType(xfl::Fin);
                pWing->setClosedInnerSide(true);
                if(ArchiveFormat<100002) pWing->setTwoSided(false);
                break;
            case 4:
                pWing->setWingType(xfl::OtherWing);
                if(ArchiveFormat<100002) pWing->setTwoSided(true);
                break;
            default:
                break;
        }
        ar >>k; /*m_bReversed = k? true : false;*/

        for (int i=0; i<50; i++) ar >> dble;

        pWing->setNTipStrips(1);

        pWing->computeGeometry();

        return true;
    }
}


bool serial::serializeWingXflFl5(WingXfl*pWing, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int i(0), k(0), n(0), is(0);
    double dble(0), d0(0), d1(0), d2(0);
    int nIntSpares = 0;
    int nDbleSpares = 0;

    // 500001 : new fl5 format;
    int ArchiveFormat = 500001;

    if(!serial::serializePartFl5(pWing, ar, bIsStoring))
        return false;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pWing->isSymmetric();

        ar << pWing->nSections();

        for (is=0; is<pWing->nSections(); is++)
        {
            WingSection &ws = pWing->section(is);
            serial::serializeWingSectionFl5(ws, ar, bIsStoring);
        }

        ar << pWing->isTwoSided();
        ar << pWing->isClosedInnerSide();
        bool bReverse = false; // deprecated
        ar << bReverse;

        switch (pWing->wingType())
        {
            case xfl::Main:      ar<<0; break;
//            case WingXfl::Second:    ar<<1; break;
            case xfl::Elevator:  ar<<2; break;
            case xfl::Fin:       ar<<3; break;
            case xfl::OtherWing: ar<<4; break;
        }

        ar << pWing->position().x << pWing->position().y<< pWing->position().z;
        ar << pWing->rx() << pWing->ry() << pWing->rz();


        // space allocation for the future storage of more data, without need to change the format

        nIntSpares = 2;
        ar << nIntSpares;
        ar << pWing->nTipStrips();
        ar << pWing->nXFlapPanels();
        //        for (int i=0; i<nIntSpares-1; i++) ar << 0;

        nDbleSpares = 0;
        ar << nDbleSpares;
        //       for (int i=0; i<nDbleSpares; i++) ar << 0.0;

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<=500000 || ArchiveFormat>500002) return false;

        ar >> boolean; pWing->setSymmetric(boolean);

        pWing->clearSections();
        ar >> n;
        for (i=0; i<n; i++)
        {
            pWing->sections().push_back(WingSection());
            serial::serializeWingSectionFl5(pWing->sections().back(), ar, bIsStoring);
        }

        ar >> boolean;  pWing->setTwoSided(boolean);
        ar >> boolean;  pWing->setClosedInnerSide(boolean);
        bool bReverse = false; // deprecated
        ar >> bReverse;

        ar >> n;
        switch(n)
        {
            case 0: pWing->setWingType(xfl::Main);       break;
            case 1: pWing->setWingType(xfl::OtherWing);  break;
            case 2: pWing->setWingType(xfl::Elevator);   break;
            case 3: pWing->setWingType(xfl::Fin);        break;
            default:
            case 4: pWing->setWingType(xfl::OtherWing);  break;
        }

        ar >> d0 >> d1 >> d2;
        pWing->setPosition(d0, d1, d2);
        ar >> d0 >> d1 >> d2;
        pWing->setRx(d0);
        pWing->setRy(d1);
        pWing->setRz(d2);

        // space allocation

        ar >> nIntSpares;

        if(nIntSpares>0)
        {
            ar >> k;
            pWing->setNTipStrips(std::max(k, 1));
        }
        if(nIntSpares>1)
        {
            ar >> k;
            pWing->setnXFlapPanels(k);
        }

        for (int i=0; i<nIntSpares-2; i++) ar >> k;

        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
        pWing->computeGeometry();

        return true;
    }
}


bool serial::serializePlaneFl5(Plane *pPlane, QDataStream &ar, bool bIsStoring)
{
    PlaneXfl* pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
    PlaneSTL* pPlaneStl = dynamic_cast<PlaneSTL*>(pPlane);
    if     (pPlaneXfl) return serial::serializePlaneXflFl5(pPlaneXfl, ar, bIsStoring);
    else if(pPlaneStl) return serial::serializePlaneStlFl5(pPlaneStl, ar, bIsStoring);
    else return false;
}


bool serial::serializePlaneXflXfl(PlaneXfl *pPlane, QDataStream &ar, bool bIsStoring)
{
    int i(0), k(0);
    double dble(0), mass(0), px(0), py(0), pz(0);
    bool bDouble(false), bSym(false), bl(false), bBiplane(false), bStab(false), bFin(false), bFuse(false);
    QString str, strange;

    int ArchiveFormat(0);// identifies the format of the file
    if (bIsStoring)
    {
        // using xf7 format instead
        return true;
    }
    else
    {    // loading code

        ar >> ArchiveFormat;
        if (ArchiveFormat <100001 || ArchiveFormat>110000)
        {
            return false;
        }

        int nw=4; //MAXWINGS
        pPlane->clearWings();
        for(int iw=0; iw<nw; iw++)
        {
            pPlane->addWing();
        }

        ar >> strange;    pPlane->setName(strange.trimmed().toStdString());
        ar >> strange;    pPlane->setDescription(strange.toStdString());

        if(ArchiveFormat>=100002)  serial::serializeLineStyleFl5(pPlane->theStyle(), ar, bIsStoring);

        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            serial::serializeWingXflXfl(pPlane->wing(iw), ar, bIsStoring);
        }

        if(ArchiveFormat<100003)
        {
            pPlane->wing(0)->setWingType(xfl::Main);
            pPlane->wing(1)->setWingType(xfl::OtherWing);
            pPlane->wing(2)->setWingType(xfl::Elevator) ;
            pPlane->wing(3)->setWingType(xfl::Fin);
            pPlane->wing(3)->setClosedInnerSide(true);
        }

        ar >> bBiplane>> bStab >>bFin >> bDouble>> bSym>> bl; // m_bDoubleSymFin;
        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            ar >> px >> py >> pz >> dble;
            // correcting past errors
            if(std::isnan(px))   px = 0.0;
            if(std::isnan(py))   py = 0.0;
            if(std::isnan(pz))   pz = 0.0;
            if(std::isnan(dble)) dble = 0.0;
            if(fabs(px)  <LENGTHPRECISION) px = 0.0;
            if(fabs(py)  <LENGTHPRECISION) py = 0.0;
            if(fabs(pz)  <LENGTHPRECISION) pz = 0.0;
            if(fabs(dble)<LENGTHPRECISION) dble = 0.0;
            if(fabs(px)  >1000.0) px = 0.0;
            if(fabs(py)  >1000.0) py = 0.0;
            if(fabs(pz)  >1000.0) pz = 0.0;
            if(fabs(dble)>1000.0) dble = 0.0;

            pPlane->wing(iw)->setPosition(px, py, pz);
            pPlane->wing(iw)->setRy(dble);
            if(pPlane->wing(iw)->isFin())
            {
                pPlane->wing(iw)->setTwoSided(bDouble);
            }
        }

        if(ArchiveFormat<100003 && pPlane->nWings()>=4 && pPlane->wing(3)->isFin())
        {
            //            m_Wing[3].isDoubleFin() = bDouble;
            //            m_Wing[3].setSymFin(bSym);
            pPlane->wing(3)->setRx(-90.0);
        }

        ar >> bFuse;
        ar >> px >> pz;
        if(bFuse)
        {
            QString BodyName;
            ar >> BodyName; //unused
            int format=0;
            ar >> format;
            if(100000<=format && format<200000)
            {
                pPlane->clearFuses();
                FuseXfl *pBody = new FuseNurbs;
                serial::serializeFuseXFLXfl(pBody, ar, bIsStoring, format);
                pPlane->addFuse(pBody);
                pPlane->fuse(0)->setPosition(px,0.0,pz);
            }
            else if(500000<=format && format<600000)
            {
/*                pPlane->clearFuses();
                FuseOcc *pBodyOcc = new FuseOcc;
                pBodyOcc->serializePartFl5(ar, bIsStoring);
                addFuse(pBodyOcc);
                m_Fuse[0]->setPosition(px,0.0,pz);*/
            }
            else if(600000<=format && format<700000)
            {
/*                pPlane->clearFuses();
                FuseStl *pBodyStl = new FuseStl;
                pBodyStl->serializePartFl5(ar, bIsStoring);
                addFuse(pBodyStl);
                m_Fuse[0]->setPosition(px,0.0,pz);*/
            }


            std::vector<int> &indexes = pPlane->partIndexes();
            if(std::find(indexes.begin(), indexes.end(), pPlane->fuse(0)->uniqueIndex()) == indexes.end())
                indexes.insert(indexes.begin(), pPlane->fuse(0)->uniqueIndex());
        }

        pPlane->clearPointMasses();

        ar >> k;
        for(i=0; i<k; i++)
        {
            ar >> mass >> px >>py >> pz;
            ar >> str;
            pPlane->inertia().appendPointMass(mass, Vector3d(px, py, pz), str.toStdString());
        }

        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;

        if(ArchiveFormat<100003)
        {
            std::vector<int> &indexes = pPlane->partIndexes();
            if(!bFin)
            {
                int index = pPlane->wings().back()->uniqueIndex();

                std::vector<int>::iterator it = std::find(indexes.begin(), indexes.end(), index);
                if(it!=indexes.end()) indexes.erase(it);
                pPlane->wings().pop_back();
            }
            if(!bStab)
            {
                int index = pPlane->wing(2)->uniqueIndex();
                std::vector<int>::iterator it = std::find(indexes.begin(), indexes.end(), index);
                if(it!=indexes.end()) indexes.erase(it);
                pPlane->wings().erase(pPlane->wings().begin()+2);
            }

            if(!bBiplane)
            {
                int index = pPlane->wing(1)->uniqueIndex();
                std::vector<int>::iterator it = std::find(indexes.begin(), indexes.end(), index);
                if(it!=indexes.end()) indexes.erase(it);
                pPlane->wings().erase(pPlane->wings().begin()+1);
            }
        }

        return true;
    }
}


bool serial::serializePlaneXflFl5(PlaneXfl *pPlane, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    double dble=0.0, m=0.0, px=0.0, py=0.0, pz=0.0;

    QString str, strange;

    int ArchiveFormat;// identifies the format of the file
    // 500001: new fl5 format
    // 500002: added m_bInertiaFromParts flag
    // 500003: beta 11; added new sub classes for FuseXfl
    ArchiveFormat = 500003;

    if (bIsStoring)
    {
        ar << ArchiveFormat;

        ar << pPlane->nWings();

        ar << QString::fromStdString(pPlane->name());
        ar << QString::fromStdString(pPlane->description());
        LineStyle &ls = pPlane->theStyle();
        ar << ls.m_Stipple << ls.m_Width << ls.m_Symbol;
        serial::serializefl5Color(ls.m_Color, ar, true);

        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            serial::serializeWingXflFl5(pPlane->wing(iw), ar, bIsStoring);
        }

        ar << pPlane->nFuse();

        for(int ifuse=0; ifuse<pPlane->nFuse(); ifuse++)
        {
            Fuse *pFuse = pPlane->fuse(ifuse);
            if(pFuse->isFlatFaceType())
            {
                ar << 100004;
                FuseFlatFaces *pBody = dynamic_cast<FuseFlatFaces*>(pFuse);
                serial::serializeFuseXflFl5(pBody, ar, true);
            }
            else if(pFuse->isSplineType())
            {
                ar << 100005;
                FuseNurbs *pBody = dynamic_cast<FuseNurbs*>(pFuse);
                serial::serializeFuseXflFl5(pBody, ar, true);
            }
            else if(pFuse->isSectionType())
            {
                ar << 100006;
                FuseSections *pBody = dynamic_cast<FuseSections*>(pFuse);
                serial::serializeFuseXflFl5(pBody, ar, true);
            }
            else if(pFuse->isOccType())
            {
                ar << 100002;
                FuseOcc *pBodyOcc = dynamic_cast<FuseOcc*>(pFuse);
                serial::serializeFuseOccFl5(pBodyOcc, ar, true);
            }
            else if(pFuse->isStlType())
            {
                ar << 100003;
                FuseStl *pBodyStl = dynamic_cast<FuseStl*>(pFuse);
                serial::serializeFuseStlFl5(pBodyStl, ar, true);
            }
            serial::serializePanelsFl5(pFuse->triMesh(), ar, bIsStoring);
        }

        ar << pPlane->bAutoInertia();
        serial::serializeInertiaFl5(pPlane->inertia(), ar, bIsStoring);

        //        serializeTriMesh(ar, bIsStoring);

        // space allocation for the future storage of more data, without need to change the format
        int nSpares=1;
        ar << nSpares;
        ar << int(pPlane->isThickBuild());
        //        for (int i=0; i<nSpares; i++) ar << 0;
        int nDbleSpares = 0;
        ar << nDbleSpares;
        //        for (int i=0; i<nSpares; i++) ar << 0.0;

        return true;
    }
    else
    {    // loading code
        int k=0;
        int nw=0;
        ar >> ArchiveFormat;
        if (ArchiveFormat <500000 || ArchiveFormat>500010) return false;

        ar >> nw;
        pPlane->clearWings();
        for(int iw=0; iw<nw; iw++)
        {
            pPlane->addWing();
        }

        ar >> strange;    pPlane->setName(strange.trimmed().toStdString());
        ar >> strange;    pPlane->setDescription(strange.toStdString());

        LineStyle ls;
        ar >> k; ls.m_Stipple = LineStyle::convertLineStyle(k);
        ar >> ls.m_Width;
        ar >> k; ls.m_Symbol=LineStyle::convertSymbol(k);
        serial::serializefl5Color(ls.m_Color, ar, false);
        pPlane->setTheStyle(ls);

        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            if(!serial::serializeWingXflFl5(pPlane->wing(iw), ar, bIsStoring))
                return false;
        }

        int nFuse;
        ar >> nFuse;
        pPlane->clearFuses();

        if(nFuse<0 || nFuse>10000)
            return false;

        for(int ifuse=0; ifuse<nFuse; ifuse++)
        {
            int format=0;
            ar >> format;
            if(format==100001)
            {
                FuseNurbs *pFuseNurbs = new FuseNurbs;
                serial::serializeFuseXflFl5(pFuseNurbs, ar, bIsStoring);
                if(pFuseNurbs)
                {
                    if(pFuseNurbs->fuseType()==Fuse::FlatFace)
                    {
                        // clean old mess
                        FuseFlatFaces *pFuseFF = new FuseFlatFaces();
                        pFuseFF->duplicateFuseXfl(*pFuseNurbs);
                        pFuseFF->setFuseType(Fuse::FlatFace);
                        pFuseFF->makeFuseGeometry();
                        if(pFuseFF->bAutoInertia()) pFuseFF->computeStructuralInertia(Vector3d());
                        std::string logmsg;
                        pFuseFF->makeDefaultTriMesh(logmsg, "");

                        delete pFuseNurbs;
                        pPlane->addFuse(pFuseFF);
                    }
                    else
                    {
                        pPlane->addFuse(pFuseNurbs);
                    }
                }
            }
            else if(format==100002)
            {
                FuseOcc *pBodyOcc = new FuseOcc;
                serial::serializeFuseOccFl5(pBodyOcc, ar, bIsStoring);
                if(pBodyOcc) pPlane->addFuse(pBodyOcc);
            }
            else if(format==100003)
            {
                FuseStl *pBodyStl = new FuseStl;
                serial::serializeFuseStlFl5(pBodyStl, ar, bIsStoring);
                if(pBodyStl) pPlane->addFuse(pBodyStl);
            }
            else if(format==100004)
            {
                FuseFlatFaces *pBodyFF = new FuseFlatFaces;
                serial::serializeFuseXflFl5(pBodyFF, ar, bIsStoring);
                if(pBodyFF) pPlane->addFuse(pBodyFF);
            }
            else if(format==100005)
            {
                FuseNurbs *pBodyNurbs = new FuseNurbs;
                serial::serializeFuseXflFl5(pBodyNurbs, ar, bIsStoring);
                if(pBodyNurbs) pPlane->addFuse(pBodyNurbs);
            }
            else if(format==100006)
            {
                FuseSections *pBodyFromPts = new FuseSections;
                serial::serializeFuseXflFl5(pBodyFromPts, ar, bIsStoring);
                if(pBodyFromPts) pPlane->addFuse(pBodyFromPts);
            }
            Fuse *pFuse = pPlane->fuses().back();
            serial::serializePanelsFl5(pFuse->triMesh(), ar, bIsStoring);

            // compatibility with legacy project formats
            if(pFuse->nPanel3()==0)
            {
                std::string strange;
                pFuse->makeDefaultTriMesh(strange, "");
            }

            for(uint in=0; in<pFuse->nodes().size(); in++)
            {
                pFuse->nodes()[in].setSurfacePosition(xfl::FUSESURFACE);
            }
            pFuse->setUniqueIndex();
        }

        pPlane->makeUniqueIndexList();

        if(ArchiveFormat>=500002)
        {
            ar >> boolean; pPlane->setAutoInertia(boolean);
            serial::serializeInertiaFl5(pPlane->inertia(), ar, bIsStoring);
        }
        else
        {
            pPlane->clearPointMasses();
            ar >> k;
            for(int i=0; i<k; i++)
            {
                ar >> m >> px >>py >> pz;
                ar >> str;
                pPlane->inertia().appendPointMass(m, Vector3d(px, py, pz), str.toStdString());
            }
        }

        // space allocation
        int nSpares=0;
        ar >> nSpares;
        if(nSpares>0)
        {
            ar >> k;  pPlane->setThickBuild(bool(k)); // v7.54
        }

        for (int i=1; i<nSpares; i++) ar >> k;
        int nDbleSpares = 0;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        return true;
    }
}


bool serial::serializePlaneStlFl5(PlaneSTL *pPlane, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int k(0);
    float x(0),y(0),z(0);
    double d0(0), d1(0), d2(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    QString strange;

    int n(0);
    double dble(0);

    // 500001: new fl5 format
    // 500002: added geom and inertia data; serialized the style properly - beta 14
    // 500003: serialized mesh info instead of triangulation info
    // 500004: surface color
    // 500005: base triangulation in beta20

    int ArchiveFormat = 500005;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(pPlane->name());
        ar << QString::fromStdString(pPlane->description());
        serial::serializefl5Color(pPlane->surfaceColor(), ar, bIsStoring);
        serializeLineStyleFl5(pPlane->theStyle(), ar, bIsStoring);

        ar << pPlane->isReversed();
        ar << pPlane->refArea() << pPlane->refChord()<< pPlane->refSpan();
        ar << pPlane->wettedArea() << pPlane->span() << pPlane->length() << pPlane->height();

        serial::serializeInertiaFl5(pPlane->inertia(), ar, bIsStoring);

        serial::serializeMeshFl5(pPlane->refTriMesh(), ar, bIsStoring);

        serial::serializeTriangulationFl5(pPlane->triangulation(), ar, bIsStoring);

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        nDbleSpares=0;
        ar << nDbleSpares;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>500100) return false;

        ar >> strange;   pPlane->setName(strange.trimmed().toStdString());
        ar >> strange;   pPlane->setDescription(strange.trimmed().toStdString());
        if(ArchiveFormat>=500004) serial::serializefl5Color(pPlane->surfaceColor(), ar, false);
        if(ArchiveFormat<500002)
        {
            LineStyle ls;
            ar >> k; ls.m_Stipple = LineStyle::convertLineStyle(k);
            ar >> ls.m_Width;
            ar >> k; ls.m_Symbol=LineStyle::convertSymbol(k);
            serial::serializefl5Color(ls.m_Color, ar, false);
            pPlane->setTheStyle(ls);
        }

        if(ArchiveFormat>=500002)
        {
            serial::serializeLineStyleFl5(pPlane->theStyle(), ar, bIsStoring);
            ar >> boolean;  pPlane->setReversed(boolean);
            ar >> d0 >> d1 >> d2;
            pPlane->setRefArea(d0);
            pPlane->setRefChord(d1);
            pPlane->setRefSpan(d2);
            ar >> dble; pPlane->setWettedArea(dble);
            ar >> d0 >> d1 >> d2;
            pPlane->setRefSpan(d0);
            pPlane->setLength(d1);
            pPlane->setHeight(d2);
            serial::serializeInertiaFl5(pPlane->inertia(), ar, bIsStoring);
        }

        if(ArchiveFormat<=500002)
        {
            pPlane->clearTriangles();
            ar >> k;
            Vector3d V0, V1, V2;
            for(int i=0; i<k; i++)
            {
                ar >> x >> y >> z;
                V0.set(double(x), double(y), double(z));
                ar >> x >> y >> z;
                V1.set(double(x), double(y), double(z));
                ar >> x >> y >> z;
                V2.set(double(x), double(y), double(z));
                Triangle3d t3(V0,V1,V2);
                pPlane->triangulation().appendTriangle(t3);
            }

            // space allocation
            ar >> nIntSpares;
            for (int i=0; i<nIntSpares; i++) ar >> n;
            ar >> nDbleSpares;
            for (int i=0; i<nDbleSpares; i++) ar >> dble;

            pPlane->makeTriangleNodes();
            pPlane->makeNodeNormals();
            pPlane->makeTriMesh(false);
            pPlane->setRefTriMesh(pPlane->triMesh());
        }
        else
        {
            serial::serializeMeshFl5(pPlane->refTriMesh(), ar, bIsStoring);
            for(int i3=0; i3<pPlane->refTriMesh().nPanels(); i3++)
            {
                pPlane->refTriMesh().panel(i3).setFromSTL(true);
            }
            pPlane->restoreMesh();

            if(ArchiveFormat<500005)
            {
                // make triangulation from mesh
                pPlane->triangulation().setNodes(pPlane->refTriMesh().nodes());
                pPlane->triangulation().setTriangleCount(pPlane->refTriMesh().nPanels());
                for(int i3=0; i3<pPlane->refTriMesh().nPanels(); i3++)
                {
                    Panel3 const &p3 = pPlane->refTriMesh().panelAt(i3);
                    Triangle3d &t3d = pPlane->triangulation().triangle(i3);
                    t3d.setVertices(p3.vertices());
                    t3d.setTriangle();
                }
            }
            else
            {
                serial::serializeTriangulationFl5(pPlane->triangulation(), ar, bIsStoring);
                pPlane->triangulation().makeNodes();
            }
            pPlane->triangulation().makeNodeNormals();

            // space allocation
            ar >> nIntSpares;
            ar >> nDbleSpares;

            if(pPlane->bAutoInertia())
                pPlane->computeStructuralInertia();
            pPlane->setInitialized(true);
        }
    }
    return true;
}

void serial::serializeEdgeSplit(EdgeSplit &ES, QDataStream &ar, bool bIsStoring)
{
    int k(0);

    if(bIsStoring)
    {
        ar << ES.nSegs();
        switch(ES.distrib())
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
        ar >> k;
        if(k<0) k=3; // cleaning up pas errors
        ES.setNSegs(k);

        ar>>k;
        switch (k)
        {
            case 0:   ES.setDistrib(xfl::UNIFORM);    break;
            case 1:   ES.setDistrib(xfl::COSINE);     break;
            case 2:   ES.setDistrib(xfl::SINE);       break;
            case 3:   ES.setDistrib(xfl::INV_SINE);   break;
            case 4:   ES.setDistrib(xfl::INV_SINH);   break;
            case 5:   ES.setDistrib(xfl::TANH);       break;
            case 6:   ES.setDistrib(xfl::EXP);        break;
            case 7:   ES.setDistrib(xfl::INV_EXP);    break;
        }
    }
}



bool serial::serializeNURBSFl5(NURBSSurface *pNurbs, QDataStream &ar, bool bIsStoring)
{
    int nIntSpares=0;
    int nDbleSpares=0;
    int k(0), l(0), n(0);
    double dble(0), d0(0), d1(0);

    // 500001 : new fl5 format
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        serial::serializefl5Color(pNurbs->color(), ar, true);
        ar << pNurbs->uDegree()<<pNurbs->vDegree();
        ar << pNurbs->bunchAmplitude() << pNurbs->bunchDist();
        ar << pNurbs->uEdgeWeight() << pNurbs->vEdgeWeight();
        ar << pNurbs->uAxis() << pNurbs->VAxis();
        ar << n; // pNurbs->m_iOutput;


        ar << pNurbs->frameCount();
        for(int k=0; k<pNurbs->frameCount(); k++)
        {
            serial::serializeFrameFl5(&pNurbs->frame(k), ar, bIsStoring);
        }

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

        serial::serializefl5Color(pNurbs->color(), ar, false);

        ar >> k >> l;
        pNurbs->setuDegree(k); pNurbs->setvDegree(l);

        ar >> d0 >> d1;
        pNurbs->setBunchAmplitude(d0); pNurbs->setBunchDistribution(d1);

        ar >> d0 >> d1;
        pNurbs->setuEdgeWeight(d0); pNurbs->setvEdgeWeight(d1);

        ar >> k >> l;
        pNurbs->setUAxis(k); pNurbs->setVAxis(l);

        ar >> n; // m_iOutput;


        pNurbs->clearFrames();
        int nFrames=0;
        ar >> nFrames;
        for(int k=0; k<nFrames; k++)
        {
            pNurbs->appendNewFrame();
            Frame &pFrame = pNurbs->lastFrame();
            serial::serializeFrameFl5(&pFrame, ar, bIsStoring);
            pFrame.setuPosition(pNurbs->uAxis());
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


void serial::serializeAeroForcesFl5_b17(AeroForces &AF, QDataStream &ar, bool bIsStoring)
{
    double dble(0);
    double d0(0), d1(0), d2(0);
    if(bIsStoring)
    {
        //deprecated
    }
    else
    {
        ar >> d0 >> d1 >> d2;
        AF.setReferenceDims(d0, d1, d2);

        ar >> dble >> dble >> dble;

        ar >> d0 >> d1 >> d2;
        AF.setFff({d0, d1, d2});

        ar >> d0 >> d1 >> d2;
        AF.setFsum({d0, d1, d2});

        ar >> d0 >> d1 >> d2;
        AF.setProfileDrag(d0);
        AF.setFuseDrag(d1);
        AF.setExtraDrag(d2);

        ar >> d0 >> d1 >> d2;
        AF.setMi({d0, d1, d2});

        ar >> d0 >> d1 >> d2;
        AF.setMv({d0, d1, d2});
    }
}


bool serial::serializeAeroForcesFl5(AeroForces &AF, QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat = 500750;
    // beta18: added Archive format, alpha, beta ,QInf;
    // 5000750:  v750 removed Cp and added M0

    double dble(0), d0(0), d1(0), d2(0);

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << AF.alpha() << AF.beta() << AF.qInf();
        ar << AF.refArea() << AF.refChord() << AF.refSpan();
        ar << dble << dble << dble;  // formerly CP
        ar << AF.Fff().x<<AF.Fff().y<<AF.Fff().z;
        ar << AF.Fsum().x<<AF.Fsum().y<<AF.Fsum().z;
        ar << AF.profileDrag() << AF.fuseDrag() << AF.extraDrag();
        ar << AF.Mi().x<<AF.Mi().y<<AF.Mi().z;
        ar << AF.Mv().x<<AF.Mv().y<<AF.Mv().z;
        ar << AF.M0().x<<AF.M0().y<<AF.M0().z;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>510000) return false;
        ar >> d0 >> d1 >> d2;
        AF.setOpp(d0, d1, 0.0, d2); /** @todo missing phi */

        ar >> d0 >> d1 >> d2;
        AF.setReferenceDims(d0, d1, d2);

        ar >> dble >> dble >> dble; // formerly CP

        ar >> d0 >> d1 >> d2;
        AF.setFff({d0, d1, d2});

        ar >> d0 >> d1 >> d2;
        AF.setFsum({d0, d1, d2});

        ar >> d0 >> d1 >> d2;
        AF.setProfileDrag(d0);
        AF.setFuseDrag(d1);
        AF.setExtraDrag(d2);

        ar >> d0 >> d1 >> d2;
        AF.setMi({d0, d1, d2});

        ar >> d0 >> d1 >> d2;
        AF.setMv({d0, d1, d2});

        if(ArchiveFormat>=500750)
        {
            ar >> d0 >> d1 >> d2;
            AF.setM0({d0, d1, d2});
        }

        if(ArchiveFormat<500002)
        {
            AF.setMi({-AF.Mi().x, AF.Mi().y, -AF.Mi().z});
            AF.setMv({-AF.Mv().x, AF.Mv().y, -AF.Mv().z});
        }
    }
    return true;
}


bool serial::serializeWingOppXFL(WingOpp *pWOpp, QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat=0;
    int k=0, n=0;
    double dble(0);
    Vector3d V;
    QString strange;

    if(bIsStoring)
    {
    }
    else
    {
        ar >> ArchiveFormat;

        ar >> strange;  pWOpp->setWingName(strange.toStdString());

        QString plrname;
        ar >> plrname;

        ar >> n; // analysis method


        ar >> pWOpp->m_bOut;

        ar >> pWOpp->m_NStation;
        ar >> pWOpp->m_nPanel4;

        int m_nWakeNodes, m_NXWakePanels;
        double m_FirstWakePanel, m_WakeFactor;
        ar >> m_nWakeNodes >> m_NXWakePanels;
        ar >> m_FirstWakePanel >> m_WakeFactor;

        ar >> dble >> dble >> dble; // alpha, beta, Qinf no longer of use
        ar >> dble >> pWOpp->m_Span >> pWOpp->m_MAChord; // mass of no use anymore
        // m_CL etc
        ar >> dble >> dble >> dble;
        ar >> dble >> dble;
        ar >> dble >> dble >> dble;
        ar >> dble >> dble;


        ar >> V.x >> V.y >> V.z;// formerly CP

        SpanDistribs &distrib = pWOpp->m_SpanDistrib;
        distrib.resizeGeometry(pWOpp->m_NStation);
        distrib.resizeResults(pWOpp->m_NStation);
        for (k=0; k<pWOpp->m_NStation; k++)
        {
            ar >> distrib.m_Re[k] >> distrib.m_Chord[k] >> distrib.m_Twist[k];
            ar >> distrib.m_Ai[k] >> distrib.m_Cl[k] >> distrib.m_PCd[k] >> distrib.m_ICd[k];
            ar >> distrib.m_CmViscous[k] >> distrib.m_CmC4[k];
            ar >> distrib.m_XCPSpanRel[k]>> distrib.m_XCPSpanAbs[k];
            ar >> distrib.m_XTrTop[k] >> distrib.m_XTrBot[k];
            ar >> distrib.m_BendingMoment[k];
            ar >> distrib.m_Vd[k].x >> distrib.m_Vd[k].y >> distrib.m_Vd[k].z;
            ar >> distrib.m_F[k].x >> distrib.m_F[k].y >> distrib.m_F[k].z;
            ar >> distrib.m_StripPos[k] >> distrib.m_StripArea[k];
        }

        ar >> pWOpp->m_nFlaps;
        pWOpp->m_FlapMoment.clear();
        for(k=0; k<pWOpp->m_nFlaps; k++)
        {
            ar >> dble;
            pWOpp->m_FlapMoment.push_back(dble);
        }


        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}


bool serial::serializeWingOppFl5(WingOpp *pWOpp, QDataStream &ar, bool bIsStoring)
{
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    int k(0), n(0);
    QString strange;

    // 500001: new fl5 format
    // 500002: changed spandistrib format - beta 08
    // 500003: Modified the format of AeroForces serialization

    int ArchiveFormat = 500003;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(pWOpp->wingName());

        switch(pWOpp->m_WingType)
        {
            case xfl::Main:
            {
                k=0;
                ar<<k;
                break;
            }
            case xfl::Elevator:
            {
                k=1;
                ar<<k;
                break;
            }
            case xfl::Fin:
            {
                k=2;
                ar<<k;
                break;
            }
            default:
            case xfl::OtherWing:
            {
                k=3;
                ar<<k;
                break;
            }
        }

        ar << pWOpp->m_bOut;

        ar << pWOpp->m_NStation;
        ar << pWOpp->m_nPanel4;

        ar << pWOpp->m_Span << pWOpp->m_MAChord;

        serial::serializeAeroForcesFl5(pWOpp->m_AF, ar, bIsStoring);
        serial::serializeSpanResultsFl5(pWOpp->m_SpanDistrib, ar, bIsStoring);

        ar << int(pWOpp->m_FlapMoment.size());
        for(uint l=0; l<pWOpp->m_FlapMoment.size(); l++)
        {
            ar << pWOpp->m_FlapMoment.at(l);
        }

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
        if(ArchiveFormat<500001 || ArchiveFormat>500100) return false;

        ar >> strange;  pWOpp->setWingName(strange.toStdString());

        ar >> k;
        switch(k)
        {
            case 0: pWOpp->m_WingType = xfl::Main;       break;
            case 1: pWOpp->m_WingType = xfl::Elevator;   break;
            case 2: pWOpp->m_WingType = xfl::Fin;        break;
            default:
            case 3: pWOpp->m_WingType = xfl::OtherWing;  break;
        }

        ar >> pWOpp->m_bOut;

        ar >> pWOpp->m_NStation;
        ar >> pWOpp->m_nPanel4;

        ar >> pWOpp->m_Span >> pWOpp->m_MAChord;


        if(ArchiveFormat<500003) serial::serializeAeroForcesFl5_b17(pWOpp->m_AF, ar, bIsStoring);
        else
        {
            if(!serial::serializeAeroForcesFl5(pWOpp->m_AF, ar, bIsStoring))
                return false;
        }

        if(!serial::serializeSpanResultsFl5(pWOpp->m_SpanDistrib, ar, bIsStoring)) return false;

        ar >> pWOpp->m_nFlaps;
        pWOpp->m_FlapMoment.clear();
        for(k=0; k<pWOpp->m_nFlaps; k++)
        {
            ar >> dble;
            pWOpp->m_FlapMoment.push_back(dble);
        }


        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool serial::serializeSpanResultsFl5(SpanDistribs &distrib, QDataStream &ar, bool bIsStoring)
{
    double dble = 0.0;;
    float xf=0, yf=0, zf=0;
    // 500001: new fl5 format
    // 500002: added offset and PtC4 properties
    int ArchiveFormat = 500002;
    int NStation = int(distrib.m_Re.size());
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << NStation;
        for (int k=0; k<NStation; k++)
        {
            ar << distrib.m_Chord.at(k) << distrib.m_Twist.at(k) << distrib.m_StripPos.at(k) << distrib.m_StripArea.at(k);
            ar << distrib.m_Re.at(k);
            ar << distrib.m_Ai.at(k) << distrib.m_Cl.at(k) << distrib.m_PCd.at(k) << distrib.m_ICd.at(k);
            ar << distrib.m_CmPressure.at(k) << distrib.m_CmViscous.at(k) << distrib.m_CmC4.at(k);
            ar << distrib.m_XCPSpanRel.at(k)<< distrib.m_XCPSpanAbs.at(k);
            ar << distrib.m_XTrTop.at(k) << distrib.m_XTrBot.at(k);
            ar << distrib.m_VTwist.at(k);
            ar << distrib.m_BendingMoment.at(k);
            ar << distrib.m_Vd.at(k).x << distrib.m_Vd.at(k).y << distrib.m_Vd.at(k).z;
            ar << distrib.m_F.at(k).x << distrib.m_F.at(k).y << distrib.m_F.at(k).z;
            ar << distrib.m_Alpha_0.at(k) << distrib.m_Gamma.at(k);

            ar << distrib.m_Offset.at(k);
            ar << distrib.m_PtC4.at(k).xf()<< distrib.m_PtC4.at(k).yf()<< distrib.m_PtC4.at(k).zf();
            dble=0.0;
            for(int i=0; i<8; i++) ar<<dble; // space allocation
        }

        for(int i=0; i<10; i++) ar<<dble; // space allocation
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500002) return false;

        ar >> NStation;
        distrib.resizeGeometry(NStation);
        distrib.resizeResults(NStation);
        for (int k=0; k<NStation; k++)
        {
            ar >> distrib.m_Chord[k] >> distrib.m_Twist[k] >> distrib.m_StripPos[k] >> distrib.m_StripArea[k];
            ar >> distrib.m_Re[k];
            ar >> distrib.m_Ai[k] >> distrib.m_Cl[k] >> distrib.m_PCd[k] >> distrib.m_ICd[k];
            ar >> distrib.m_CmPressure[k] >> distrib.m_CmViscous[k] >> distrib.m_CmC4[k];
            ar >> distrib.m_XCPSpanRel[k]>> distrib.m_XCPSpanAbs[k];
            ar >> distrib.m_XTrTop[k] >> distrib.m_XTrBot[k];
            ar >> distrib.m_VTwist[k];
            ar >> distrib.m_BendingMoment[k];
            ar >> distrib.m_Vd[k].x >> distrib.m_Vd[k].y >> distrib.m_Vd[k].z;
            ar >> distrib.m_F[k].x >> distrib.m_F[k].y >> distrib.m_F[k].z;
            ar >> distrib.m_Alpha_0[k] >> distrib.m_Gamma[k];
            if(ArchiveFormat>=500002)
            {
                ar >> distrib.m_Offset[k];
                ar >> xf >> yf >> zf;
                distrib.m_PtC4[k].set(xf,yf,zf);
            }
            for(int i=0; i<8; i++) ar>>dble; // space allocation
        }
        for(int i=0; i<10; i++) ar >> dble; // space allocation
    }
    return true;
}


bool serial::serializeBoatOppFl5(BoatOpp *pBtOpp, QDataStream &ar, bool bIsStoring)
{
    // 100001: first file format
    // 100002: added lift and drag
    // 100003: added vortons and negating vortices
    // 100004: Modified the format of AeroForces serialization
    // 100005: beta20 - Added the roration about Ry

    int ArchiveFormat=100005;
    bool boolean(false);
    float f(0),g(0),h(0);
    double d0(0), d1(0), d2(0);
    int n(0);
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    QString strange;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(pBtOpp->boatName());
        ar << QString::fromStdString(pBtOpp->polarName());

        serial::serializeLineStyleFl5(pBtOpp->theStyle(), ar, bIsStoring);

        //ANALYSIS METHOD
        if     (pBtOpp->analysisMethod()==xfl::LLT)        ar<<1;
        else if(pBtOpp->analysisMethod()==xfl::VLM1)       ar<<2;
        else if(pBtOpp->analysisMethod()==xfl::VLM1)       ar<<3;
        else if(pBtOpp->analysisMethod()==xfl::QUADS)      ar<<4;
        else if(pBtOpp->analysisMethod()==xfl::TRILINEAR)  ar<<5;
        else if(pBtOpp->analysisMethod()==xfl::TRIUNIFORM) ar<<6;
        else                                               ar<<0;

        ar << pBtOpp->bThinSurfaces();
        ar << pBtOpp->bTrefftz();

        ar << pBtOpp->bIgnoreBodyPanels();

        ar << pBtOpp->groundHeight() << pBtOpp->QInf() << pBtOpp->beta() << pBtOpp->phi() << pBtOpp->Ry() << pBtOpp->ctrl();

        ar<<int(pBtOpp->sailAngles().size());
        for(uint is=0; is<pBtOpp->sailAngles().size(); is++)
        {
            ar<< pBtOpp->sailAngle(is);
        }

        serial::serializeAeroForcesFl5(pBtOpp->aeroForces(), ar, bIsStoring);

        ar << int(pBtOpp->sailForceFF().size());
        for(uint i=0; i<pBtOpp->sailForceFF().size(); i++)
        {
            ar << pBtOpp->sailForceFF(i).xf()  << pBtOpp->sailForceFF(i).yf()  << pBtOpp->sailForceFF(i).zf();
            ar << pBtOpp->sailForceSum(i).xf() << pBtOpp->sailForceSum(i).yf() << pBtOpp->sailForceSum(i).zf();
        }

        int N=0;
        if      (pBtOpp->isQuadMethod())     N = pBtOpp->nPanel4();
        else if(pBtOpp->isTriangleMethod())  N = 3*pBtOpp->nPanel3();
        ar << N;
        for (int p=0; p<N; p++) ar << float(pBtOpp->Cp(p)) << float(pBtOpp->gamma(p)) << float(pBtOpp->sigma(p));


        ar << int(pBtOpp->vortonRows());
        for(int ir=0; ir<pBtOpp->vortonRows(); ir++)
        {
            ar <<int(pBtOpp->vortonRow(ir).size());
            for(uint ic=0; ic<pBtOpp->vortonRow(ir).size(); ic++)
            {
                serial::serializeVortonFl5(pBtOpp->vortonRow(ir)[ic], ar, bIsStoring);
            }
        }

        ar << int(pBtOpp->vortexNeg().size());
        for(uint iv=0; iv<pBtOpp->vortexNeg().size(); iv++)
        {
            serial::serializeVortexFl5(pBtOpp->vortexNeg()[iv], ar, bIsStoring);
        }


        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<100000 || ArchiveFormat>120000) return false;
        //read variables
        ar >> strange;   pBtOpp->setBoatName(strange.toStdString());
        ar >> strange;   pBtOpp->setPolarName(strange.toStdString());

        serial::serializeLineStyleFl5(pBtOpp->theStyle(), ar, bIsStoring);

        // ANALYSIS METHOD
        ar >> n;
        if     (n==1) pBtOpp->setAnalysisMethod(xfl::LLT);
        else if(n==2) pBtOpp->setAnalysisMethod(xfl::VLM1);
        else if(n==3) pBtOpp->setAnalysisMethod(xfl::VLM2);
        else if(n==4) pBtOpp->setAnalysisMethod(xfl::QUADS);
        else if(n==5) pBtOpp->setAnalysisMethod(xfl::TRILINEAR);
        else if(n==6) pBtOpp->setAnalysisMethod(xfl::TRIUNIFORM);

        ar >> boolean; pBtOpp->setThinSurfaces(boolean);
        ar >> boolean; pBtOpp->setTrefftz(boolean);

        ar >> boolean; pBtOpp->setIgnoreBodyPanels(boolean);

        ar >> dble; pBtOpp->setGroundHeight(dble);
        ar >> d0 >> d1 >> d2;
        pBtOpp->setQInf(d0);
        pBtOpp->setBeta(d1);
        pBtOpp->setPhi(d2);

        if(ArchiveFormat>=100005)
        {
            ar >> dble;
            pBtOpp->setRy(dble);
        }
        ar >> dble; pBtOpp->setCtrl(dble);

        ar>>n;
        pBtOpp->sailAngles().resize(n);
        for(int is=0; is<n; is++)
        {
            ar>> dble; pBtOpp->setSailAngle(is, dble);
        }

        if(ArchiveFormat<100004) serial::serializeAeroForcesFl5_b17(pBtOpp->aeroForces(), ar, bIsStoring);
        else
        {
            if(!serial::serializeAeroForcesFl5(pBtOpp->aeroForces(), ar, bIsStoring)) return false;
        }
        pBtOpp->aeroForces().setOpp(0.0, pBtOpp->beta(), pBtOpp->phi(), pBtOpp->QInf());

        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> f >> g >> h;
            pBtOpp->sailForceFF().push_back({double(f), double(g), double (h)});
            ar >> f >> g >> h;
            pBtOpp->sailForceSum().push_back({double(f), double(g), double (h)});
        }

        int N=0;
        ar >> N;

        pBtOpp->Cp().resize(N);
        pBtOpp->gamma().resize(N);
        pBtOpp->sigma().resize(N);

        for (int p=0; p<N; p++)
        {
            ar >> f >> g >> h;
            pBtOpp->Cp()[p]    = double(f);
            pBtOpp->gamma()[p] = double(g);
            pBtOpp->sigma()[p] = double(h);
        }

        pBtOpp->setNPanel4(0);
        pBtOpp->setNPanel3(0);

        if     (pBtOpp->isQuadMethod())      pBtOpp->setNPanel4(N);
        else if(pBtOpp->isTriangleMethod())  pBtOpp->setNPanel3(N/3);

        if(ArchiveFormat>=100003)
        {
            ar >> n;
            pBtOpp->vortons().resize(n);
            for(int ir=0; ir<pBtOpp->vortonRows(); ir++)
            {
                ar >> n;
                pBtOpp->vortonRow(ir).resize(n);
                for(uint ic=0; ic<pBtOpp->vortonRow(ir).size(); ic++)
                {
                    serial::serializeVortonFl5(pBtOpp->vortonRow(ir)[ic], ar, bIsStoring);
                }
            }

            ar >> n;
            pBtOpp->vortexNeg().resize(n);
            for(int iv=0; iv<n; iv++)
            {
                serial::serializeVortexFl5(pBtOpp->vortexNeg()[iv], ar, bIsStoring);
            }
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool serial::serializePlaneOppXFL(PlaneOpp *pPOpp, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    int k(0), n(0);
    float f0(0), f1(0), f2(0);
    double dble(0), dbl1(0), dbl2(0);
    QString strange;

    int ArchiveFormat=200002;

    if(bIsStoring)
    {
        //using fl5 format instead
    }
    else
    {
        ar >> ArchiveFormat;
        if (ArchiveFormat<200000 || ArchiveFormat>200003 ) return false;

        n=4;
        pPOpp->m_WingOpp.clear();
        pPOpp->m_WingOpp.resize(n);

        ar >> strange;   pPOpp->setPlaneName(strange.toStdString());
        ar >> strange;   pPOpp->setPolarName(strange.toStdString());
        if(ArchiveFormat<200002)
        {
            LineStyle ls;
            ar >> k; ls.m_Stipple=LineStyle::convertLineStyle(k);
            ar >> ls.m_Width;
            serial::serializefl5Color(ls.m_Color, ar, false);
            ar >> ls.m_bIsVisible >> boolean;
            pPOpp->setTheStyle(ls);
        }
        else serial::serializeLineStyleXfl(pPOpp->theStyle(), ar, bIsStoring);

        ar >> boolean; pPOpp->setOut(boolean);
        ar >> boolean;

        ar >> boolean; pPOpp->setThinSurfaces(boolean);
        ar >> boolean; //m_bTiltedGeom;

        ar >> n;
        if(n==1)      pPOpp->m_PolarType=xfl::T1POLAR;
        else if(n==2) pPOpp->m_PolarType=xfl::T2POLAR;
        else if(n==4) pPOpp->m_PolarType=xfl::T4POLAR;
        else if(n==5) pPOpp->m_PolarType=xfl::T5POLAR;
        else if(n==6) pPOpp->m_PolarType=xfl::T6POLAR;
        else if(n==7) pPOpp->m_PolarType=xfl::T7POLAR;

        ar >> n;
        if     (n==1) pPOpp->setAnalysisMethod(xfl::LLT);
        else if(n==2)
        {
            if(boolean)  pPOpp->setAnalysisMethod(xfl::VLM1);
            else         pPOpp->setAnalysisMethod(xfl::VLM2);
        }
        else if(n==2) pPOpp->setAnalysisMethod(xfl::QUADS);

        ar >> k;
        if(pPOpp->isTriangleMethod())   pPOpp->setNPanel3(k);
        else if (pPOpp->isQuadMethod()) pPOpp->setNPanel4(k);
        ar >> n;
        ar >> dble; pPOpp->setAlpha(dble);
        ar >> dble; pPOpp->setQInf(dble);
        ar >> dble; pPOpp->setBeta(dble);
        ar >> dble; pPOpp->setCtrl(dble);

        ar >> dble; pPOpp->m_Mass = dble;

        /*        if(m_AnalysisMethod!=xfl::LLTMETHOD)
                {
                        for (k=0; k<nPanels; k++)
                        {
                                ar >> f0 >> f1 >> f2;
                                m_dCp[k]    = (double)f0;
                                m_dSigma[k] = (double)f1;
                                m_dG[k]     = (double)f2;
                        }
                }*/
        if(pPOpp->isQuadMethod())
        {
            pPOpp->Cp().resize(pPOpp->nPanel4());
            pPOpp->sigma().resize(pPOpp->nPanel4());
            pPOpp->gamma().resize(pPOpp->nPanel4());
            for (k=0; k<pPOpp->nPanel4(); k++)
            {
                ar >> f0 >> f1 >> f2;
                pPOpp->Cp()[k]    = double(f0);
                pPOpp->sigma()[k] = double(f1);
                pPOpp->gamma()[k] = double(f2);
            }
        }
        else if (pPOpp->isTriangleMethod())
        {
            int N =  3*pPOpp->nPanel3();
            pPOpp->Cp().resize(N);
            pPOpp->gamma().resize(N);
            pPOpp->sigma().resize(pPOpp->nPanel3());
            for (k=0; k<N; k++)
            {
                ar >> f0;
                pPOpp->Cp()[k] = double(f0);
            }
            for (k=0; k<N; k++)
            {
                ar >> f0;
                pPOpp->gamma()[k] = double(f0);
            }
            for (k=0; k<pPOpp->nPanel3(); k++)
            {
                ar >> f0;
                pPOpp->sigma()[k] = double(f0);
            }
        }


        int pos = 0;
        for(uint iw=0; iw<pPOpp->m_WingOpp.size(); iw++)
        {
            ar >> n;

            if(n)
            {
                serial::serializeWingOppXFL(&pPOpp->m_WingOpp[iw], ar, bIsStoring);

                pPOpp->m_WingOpp[iw].m_dCp    = pPOpp->Cp().data()    + pos;
                pPOpp->m_WingOpp[iw].m_dSigma = pPOpp->sigma().data() + pos;
                pPOpp->m_WingOpp[iw].m_dG     = pPOpp->gamma().data() + pos;
                pos +=pPOpp->m_WingOpp[iw].m_nPanel4;
            }

        }


        ar >> dble >> dbl1 >> dbl2;
        ar >> dble >> dble >> dble >> dble;
        ar >> dble;
        ar >> dble >> dbl1 >> dbl2;
        ar >> dble >> dbl1 >> dbl2;

        StabDerivatives &SD = pPOpp->m_SD;
        ar >> SD.CXa >> SD.CXq >> SD.CXu >> SD.CZu >> SD.Cmu;
        ar >> SD.CZa >> SD.CZq >> SD.Cma >> SD.Cmq;
        ar >> SD.CYb >> SD.CYp >> SD.CYr >> SD.Clb >> SD.Clp >> SD.Clr >> SD.Cnb >> SD.Cnp >> SD.Cnr;

        ar >> n;
        SD.resizeControlDerivatives(1);
        ar >> SD.CXe.front() >> SD.CYe.front() >> SD.CZe.front();
        ar >> SD.CLe.front() >> SD.CMe.front() >> SD.CNe.front();

        std::vector<std::vector<double>> &BLong = pPOpp->m_BLong;
        std::vector<std::vector<double>> &BLat  = pPOpp->m_BLat;

        pPOpp->m_BLat.resize(1);
        BLong.resize(1);
        BLat.front().resize(4);
        BLong.front().resize(4);
        ar >> BLat[0][0] >> BLat[0][1] >> BLat[0][2] >> BLat[0][3];
        ar >> BLong[0][0]>> BLong[0][1]>> BLong[0][2]>> BLong[0][3];

        for(k=0; k<4; k++)
        {
            ar >> pPOpp->m_ALong[k][0]>> pPOpp->m_ALong[k][1]>> pPOpp->m_ALong[k][2]>> pPOpp->m_ALong[k][3];
            ar >> pPOpp->m_ALat[k][0] >> pPOpp->m_ALat[k][1] >> pPOpp->m_ALat[k][2] >> pPOpp->m_ALat[k][3];
        }

        ar >> dble; // formerly m_XNP
        //        if(m_WPolarType!=Xfl::STABILITYPOLAR) m_XNP = 0.0;

        for(int kv=0; kv<8;kv++)
        {
            ar >> dbl1 >> dbl2;
            pPOpp->m_EigenValue[kv] = std::complex<double>(dbl1, dbl2);

            for(int lv=0; lv<4; lv++)
            {
                ar >> dbl1 >> dbl2;
                pPOpp->m_EigenVector[kv][lv] = std::complex<double>(dbl1, dbl2);
            }
        }

        // space allocation
        for (int i=0; i<17; i++) ar >> k;
        int n3,n4;
        ar >> n3 >> n4;
        if (ArchiveFormat==200002)
        {
            pPOpp->setNPanel3(n3);
            pPOpp->setNPanel4(n4);
        }

        ar >> k; pPOpp->theStyle().m_Symbol=LineStyle::convertSymbol(k);

        ar >> dble; pPOpp->m_MAChord = dble;
        ar >> dble; pPOpp->m_Span = dble;

        double real=0.0, imag=0.0;
        ar >> real >> imag;
        pPOpp->m_phiPH = std::complex<double>(real, imag);
        ar >> real >> imag;
        pPOpp->m_phiDR = std::complex<double>(real, imag);

        for (int i=6; i<50; i++) ar >> dble;
    }
    return true;
}


bool serial::serializePlaneOppFl5(PlaneOpp *pPOpp, QDataStream &ar, bool bIsStoring)
{
    int nIntSpares(0);
    int nDbleSpares(0);
    bool boolean(false);
    int k(0), n(0);
    float f0(0), f1(0), f2(0);
    QString strange;

    double dble(0), dbl1(0), dbl2(0);

    // 500001: new fl5 format
    // 500002: moved StabilityDerivative serialization to separate class
    // 500011: changed WingOpp/spandistrib format
    // 500012: added vorton serialization in beta 12
    // 500013: added ground props in beta 13
    // 500014: beta 18: added multiple control matrices
    // 500015: beta 18: Modified the format of AeroForces serialization
    // 500016: v7.21: Addded free surface effect
    int ArchiveFormat = 500016;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << int(pPOpp->m_WingOpp.size());

        ar << QString::fromStdString(pPOpp->planeName());
        ar << QString::fromStdString(pPOpp->polarName());

        ar << LineStyle::convertLineStyle(pPOpp->theStyle().m_Stipple);
        ar << pPOpp->theStyle().m_Width;
        ar << LineStyle::convertSymbol(pPOpp->theStyle().m_Symbol);
        serial::serializefl5Color(pPOpp->theStyle().m_Color, ar, true);
        ar << pPOpp->theStyle().m_bIsVisible << false;

        ar <<pPOpp->nPanel3() << pPOpp->nPanel4();
        ar << pPOpp->isOut();
        ar << boolean;

        ar << pPOpp->bThinSurfaces() << boolean; //m_bTiltedGeom;

        if     (pPOpp->m_PolarType==xfl::T1POLAR) ar<<1;
        else if(pPOpp->m_PolarType==xfl::T2POLAR) ar<<2;
        else if(pPOpp->m_PolarType==xfl::T4POLAR) ar<<4;
        else if(pPOpp->m_PolarType==xfl::T5POLAR) ar<<5;
        else if(pPOpp->m_PolarType==xfl::T6POLAR) ar<<6;
        else if(pPOpp->m_PolarType==xfl::T7POLAR) ar<<7;
        else if(pPOpp->m_PolarType==xfl::T8POLAR) ar<<100;
        else                                      ar<<1;

        if     (pPOpp->analysisMethod()==xfl::LLT)        ar<<1;
        else if(pPOpp->analysisMethod()==xfl::VLM1)       ar<<2;
        else if(pPOpp->analysisMethod()==xfl::VLM2)       ar<<3;
        else if(pPOpp->analysisMethod()==xfl::QUADS)      ar<<4;
        else if(pPOpp->analysisMethod()==xfl::TRILINEAR)  ar<<5;
        else if(pPOpp->analysisMethod()==xfl::TRIUNIFORM) ar<<6;
        else                                              ar<<0;

        if(pPOpp->isQuadMethod())          ar<<pPOpp->nPanel4();
        else if(pPOpp->isTriangleMethod()) ar<<pPOpp->nPanel3();
        else                        ar<<0;

        ar << n; // m_NStations
        ar << pPOpp->alpha() << pPOpp->QInf();
        ar << pPOpp->beta();
        ar << pPOpp->ctrl();

        ar << pPOpp->m_MAChord<<pPOpp->m_Span;
        ar << pPOpp->m_Mass;
        ar << pPOpp->cog().x << pPOpp->cog().z;
        ar << pPOpp->m_Inertia[0] << pPOpp->m_Inertia[1] << pPOpp->m_Inertia[2] << pPOpp->m_Inertia[3];

        ar << pPOpp->bGround() << pPOpp->bFreeSurfaceEffect() << pPOpp->groundHeight();

        if(pPOpp->isQuadMethod())
        {
            for (k=0; k<pPOpp->nPanel4(); k++)
                ar<<float(pPOpp->Cp().at(k))<<float(pPOpp->sigma().at(k))<<float(pPOpp->gamma().at(k));
        }
        else if (pPOpp->isTriangleMethod())
        {
            int N3 = 3*pPOpp->nPanel3();
            for (k=0; k<N3; k++) ar<<float(pPOpp->Cp().at(k));
            for (k=0; k<N3; k++) ar<<float(pPOpp->gamma().at(k));
            for (k=0; k<pPOpp->nPanel3(); k++) ar<<float(pPOpp->sigma().at(k));
        }


        for(uint iw=0; iw<pPOpp->m_WingOpp.size(); iw++)
        {
            serial::serializeWingOppFl5(&pPOpp->m_WingOpp[iw], ar, bIsStoring);
        }

        serial::serializeAeroForcesFl5(pPOpp->aeroForces(), ar, bIsStoring);

        ar << int(pPOpp->m_FuseAF.size());
        for(uint ifuse=0; ifuse<pPOpp->m_FuseAF.size(); ifuse++)
            serial::serializeAeroForcesFl5(pPOpp->m_FuseAF[ifuse], ar, bIsStoring);
        /*
        ar << m_SD.CXa << m_SD.CXq << m_SD.CXu << m_SD.CZu <<m_SD.Cmu;
        ar << m_SD.CLa << m_SD.CLq << m_SD.Cma << m_SD.Cmq;
        ar << m_SD.CYb << m_SD.CYp << m_SD.CYr << m_SD.Clb << m_SD.Clp << m_SD.Clr << m_SD.Cnb << m_SD.Cnp << m_SD.Cnr;
        ar << m_SD.CXe << m_SD.CYe << m_SD.CZe;
        ar << m_SD.CLe << m_SD.CMe << m_SD.CNe;*/

        serial::serializeStabDerivativesFl5(pPOpp->m_SD, ar, bIsStoring);

        ar <<int(pPOpp->m_BLat.size());
        for(uint ie=0; ie<pPOpp->m_BLat.size(); ie++)
        {
            for(int j=0; j<4; j++)
            {
                ar << pPOpp->m_BLat.at(ie).at(j);
                ar << pPOpp->m_BLong.at(ie).at(j);
            }
        }

        for(k=0; k<4; k++)
        {
            ar << pPOpp->m_ALong[k][0]<< pPOpp->m_ALong[k][1]<< pPOpp->m_ALong[k][2]<< pPOpp->m_ALong[k][3];
            ar << pPOpp->m_ALat[k][0] << pPOpp->m_ALat[k][1] << pPOpp->m_ALat[k][2] << pPOpp->m_ALat[k][3];
        }


        ar << pPOpp->phi(); // repurposing

        for(int kv=0; kv<8;kv++)
        {
            ar << pPOpp->m_EigenValue[kv].real() << pPOpp->m_EigenValue[kv].imag();
            for(int lv=0; lv<4; lv++)
            {
                ar << pPOpp->m_EigenVector[kv][lv].real() << pPOpp->m_EigenVector[kv][lv].imag();
            }
        }

        ar << pPOpp->m_phiPH.real() << pPOpp->m_phiPH.imag();
        ar << pPOpp->m_phiDR.real() << pPOpp->m_phiDR.imag();

        ar << pPOpp->vortonRows();
        for(int ir=0; ir<pPOpp->vortonRows(); ir++)
        {
            ar <<int(pPOpp->vortonRow(ir).size());
            for(uint ic=0; ic<pPOpp->vortonRow(ir).size(); ic++)
            {
                serial::serializeVortonFl5(pPOpp->vortonRow(ir)[ic], ar, bIsStoring);
            }
        }

        ar << int(pPOpp->vortexNeg().size());
        for(uint iv=0; iv<pPOpp->vortexNeg().size(); iv++)
        {
            serial::serializeVortexFl5(pPOpp->vortexNeg()[iv], ar, bIsStoring);
        }

        ar << 0;
        ar << 0;
    }
    else
    {
        ar >> ArchiveFormat;
        if (ArchiveFormat<500001 || ArchiveFormat>500030) return false;

        ar >> n;
        pPOpp->m_WingOpp.clear();
        pPOpp->m_WingOpp.resize(n);

        ar >> strange;   pPOpp->setPlaneName(strange.toStdString());
        ar >> strange;   pPOpp->setPolarName(strange.toStdString());

        LineStyle &ls = pPOpp->theStyle();
        ar >> k; ls.m_Stipple=LineStyle::convertLineStyle(k);
        ar >> ls.m_Width;
        ar >> k; ls.m_Symbol=LineStyle::convertSymbol(k);
        serial::serializefl5Color(ls.m_Color, ar, false);
        ar >> ls.m_bIsVisible >> boolean;

        ar >> n; pPOpp->setNPanel3(n);
        ar >> n; pPOpp->setNPanel4(n);
        ar >> boolean; pPOpp->setOut(boolean);
        ar >> boolean;

        ar >> boolean; pPOpp->setThinSurfaces(boolean);
        ar >> boolean; //m_bTiltedGeom;

        ar >> n;
        if     (n==1)   pPOpp->setPolarType(xfl::T1POLAR);
        else if(n==2)   pPOpp->setPolarType(xfl::T2POLAR);
        else if(n==4)   pPOpp->setPolarType(xfl::T4POLAR);
        else if(n==5)   pPOpp->setPolarType(xfl::T5POLAR);
        else if(n==6)   pPOpp->setPolarType(xfl::T6POLAR);
        else if(n==7)   pPOpp->setPolarType(xfl::T7POLAR);
        else if(n==100) pPOpp->setPolarType(xfl::T8POLAR);

        ar >> n;
        if     (n==1) pPOpp->setAnalysisMethod(xfl::LLT);
        else if(n==2) pPOpp->setAnalysisMethod(xfl::VLM1);
        else if(n==3) pPOpp->setAnalysisMethod(xfl::VLM2);
        else if(n==4) pPOpp->setAnalysisMethod(xfl::QUADS);
        else if(n==5) pPOpp->setAnalysisMethod(xfl::TRILINEAR);
        else if(n==6) pPOpp->setAnalysisMethod(xfl::TRIUNIFORM);
        ar >> k;
        if(pPOpp->isTriangleMethod())   pPOpp->setNPanel3(k);
        else if (pPOpp->isQuadMethod()) pPOpp->setNPanel4(k);
        ar >> k; //m_NStations;
        ar >> dble; pPOpp->setAlpha(dble);
        ar >> dble; pPOpp->setQInf(dble);
        ar >> dble; pPOpp->setBeta(dble);
        ar >> dble; pPOpp->setCtrl(dble);

        ar >> pPOpp->m_MAChord >> pPOpp->m_Span;

        ar >> pPOpp->m_Mass;
        ar >> pPOpp->m_CoG.x >> pPOpp->m_CoG.z;
        ar >> pPOpp->m_Inertia[0] >> pPOpp->m_Inertia[1] >> pPOpp->m_Inertia[2] >> pPOpp->m_Inertia[3];

        if(ArchiveFormat>=500013)
        {
            ar >> boolean; pPOpp->setGroundEffect(boolean);
            if(ArchiveFormat>=500016)
            {
                ar >> boolean; pPOpp->setFreeSurfaceEffect(boolean);
            }
            ar >> dble; pPOpp->setGroundHeight(dble);
        }

        if(pPOpp->isQuadMethod())
        {
            pPOpp->Cp().resize(pPOpp->nPanel4());
            pPOpp->sigma().resize(pPOpp->nPanel4());
            pPOpp->gamma().resize(pPOpp->nPanel4());
            for (k=0; k<pPOpp->nPanel4(); k++)
            {
                ar >> f0 >> f1 >> f2;
                pPOpp->Cp()[k]    = double(f0);
                pPOpp->sigma()[k] = double(f1);
                pPOpp->gamma()[k] = double(f2);
            }
        }
        else if (pPOpp->isTriangleMethod())
        {
            int N =  3*pPOpp->nPanel3();
            pPOpp->Cp().resize(N);
            pPOpp->gamma().resize(N);
            pPOpp->sigma().resize(pPOpp->nPanel3());
            for (k=0; k<N; k++)
            {
                ar >> f0;
                pPOpp->Cp()[k] = double(f0);
            }
            for (k=0; k<N; k++)
            {
                ar >> f0;
                pPOpp->gamma()[k] = double(f0);
            }
            for (k=0; k<pPOpp->nPanel3(); k++)
            {
                ar >> f0;
                pPOpp->sigma()[k] = double(f0);
            }
        }

        int pos = 0;
        for(uint iw=0; iw<pPOpp->m_WingOpp.size(); iw++)
        {
            if(!serial::serializeWingOppFl5(&pPOpp->m_WingOpp[iw], ar, bIsStoring))
                return false;

            pPOpp->m_WingOpp[iw].m_dCp    = pPOpp->Cp().data()    + pos;
            pPOpp->m_WingOpp[iw].m_dG     = pPOpp->gamma().data() + pos;
            pPOpp->m_WingOpp[iw].m_dSigma = pPOpp->sigma().data() + pos;
            pos += pPOpp->m_WingOpp[iw].m_nPanel4;
        }

        if(ArchiveFormat<500015) serial::serializeAeroForcesFl5_b17(pPOpp->aeroForces(), ar, bIsStoring);
        else
        {
            if(!serial::serializeAeroForcesFl5(pPOpp->aeroForces(), ar, bIsStoring))
                return false;
        }
        /*        m_AF.setOpp(m_Alpha, m_Beta, m_Phi, m_QInf);
        for(int iw=0; iw<m_WingOpp.size(); iw++)
        {
            m_WingOpp[iw].m_AF.setOpp(m_Alpha, m_Beta, m_Phi, m_QInf);
        }*/

        int nFuse=0;
        ar >> nFuse;
        pPOpp->m_FuseAF.resize(nFuse);
        for(int ifuse=0; ifuse<nFuse; ifuse++)
        {
            if(ArchiveFormat<500015) serial::serializeAeroForcesFl5_b17(pPOpp->m_FuseAF[ifuse], ar, bIsStoring);
            else
            {
                if(!serial::serializeAeroForcesFl5(pPOpp->m_FuseAF[ifuse], ar, bIsStoring))
                    return false;
            }
        }

        if(ArchiveFormat<=500001)
        {
            for(int isd=0; isd<24; isd++) ar>>dble;
        }
        else
            serial::serializeStabDerivativesFl5(pPOpp->m_SD, ar, bIsStoring);

        std::vector<std::vector<double>> &BLong = pPOpp->m_BLong;
        std::vector<std::vector<double>> &BLat  = pPOpp->m_BLat;

        if(ArchiveFormat<=500013)
        {

            pPOpp->m_BLat.resize(1);
            BLong.resize(1);
            BLat.front().resize(4);
            BLong.front().resize(4);
            ar >> BLat[0][0] >> BLat[0][1] >> BLat[0][2] >> BLat[0][3];
            ar >> BLong[0][0]>> BLong[0][1]>> BLong[0][2]>> BLong[0][3];
        }
        else
        {
            ar >>n;
            BLat.resize(n);
            BLong.resize(n);
            for(uint ie=0; ie<BLat.size(); ie++)
            {
                BLat[ie].resize(4);
                BLong[ie].resize(4);
                for(int j=0; j<4; j++)
                {
                    ar >> BLat[ie][j];
                    ar >> BLong[ie][j];
                }
            }

        }

        for(k=0; k<4; k++)
        {
            ar >> pPOpp->m_ALong[k][0]>> pPOpp->m_ALong[k][1]>> pPOpp->m_ALong[k][2]>> pPOpp->m_ALong[k][3];
            ar >> pPOpp->m_ALat[k][0] >> pPOpp->m_ALat[k][1] >> pPOpp->m_ALat[k][2] >> pPOpp->m_ALat[k][3];
        }


        ar >> dble;  pPOpp->setPhi(dble);  // repurposing - formerly m_XNP
        pPOpp->aeroForces().setOpp(pPOpp->alpha(), pPOpp->beta(), pPOpp->phi(), pPOpp->QInf());
        for(uint iw=0; iw<pPOpp->m_WingOpp.size(); iw++)
        {
            pPOpp->m_WingOpp[iw].aeroForces().setOpp(pPOpp->alpha(), pPOpp->beta(), pPOpp->phi(), pPOpp->QInf());
        }

        for(int kv=0; kv<8;kv++)
        {
            ar >> dbl1 >> dbl2;
            pPOpp->m_EigenValue[kv] = std::complex<double>(dbl1, dbl2);

            for(int lv=0; lv<4; lv++)
            {
                ar >> dbl1 >> dbl2;
                pPOpp->m_EigenVector[kv][lv] = std::complex<double>(dbl1, dbl2);
            }
        }

        double real=0.0, imag=0.0;
        ar >> real >> imag;
        pPOpp->m_phiPH = std::complex<double>(real, imag);
        ar >> real >> imag;
        pPOpp->m_phiDR = std::complex<double>(real, imag);

        if(ArchiveFormat<=500001)
        {
            for (int i=0; i<50; i++) ar >> n;
            for (int i=0; i<50; i++) ar >> dble;
        }
        else
        {
            if(ArchiveFormat>=500012)
            {
                ar >> n;
                pPOpp->vortons().resize(n);
                for(int ir=0; ir<pPOpp->vortonRows(); ir++)
                {
                    ar >> n;
                    pPOpp->vortonRow(ir).resize(n);
                    for(int ic=0; ic<int(pPOpp->vortonRow(ir).size()); ic++)
                    {
                        serial::serializeVortonFl5(pPOpp->vortons()[ir][ic], ar, bIsStoring);
                    }
                }

                ar >> n;
                pPOpp->vortexNeg().resize(n);
                for(int iv=0; iv<n; iv++)
                {
                    serial::serializeVortexFl5(pPOpp->vortexNeg()[iv], ar, bIsStoring);
                }
            }

            ar >> nIntSpares;
            ar >> nDbleSpares;
        }
    }
    return true;
}


bool serial::serializeStabDerivativesFl5(StabDerivatives &SD, QDataStream &ar, bool bIsStoring)
{
    int k=0, n=0;
    int nIntSpares=0;
    int nDbleSpares=0;
    QString strange;
    double dble=0.0;

    // 500001: new fl5 format
    // 500002: beta 18 - added multiple control derivatives
    int ArchiveFormat = 500002;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << SD.m_Span << SD.m_Area << SD.m_MAC;
        ar << SD.m_QInf << dble << dble;
        ar << SD.m_Mass;
        ar << SD.m_CoG_x;
        ar << SD.m_rho;


        // DIMENSIONAL derivatives
        // longitudinal
        ar <<  SD.Xu  << SD.Xw << SD.Zu<< SD.Zw << SD.Xq << SD.Zq << SD.Mu << SD.Mw << SD.Mq;
        ar <<  SD.Zwp << SD.Mwp;
        // lateral
        ar <<  SD.Yv << SD.Yp << SD.Yr << SD.Lv << SD.Lp << SD.Lr << SD.Nv << SD.Np << SD.Nr;
        // control
        ar << int(SD.Xde.size());
        for(uint ie=0; ie<SD.Xde.size(); ie++)
        {
            ar << QString::fromStdString(SD.ControlNames.at(ie));
            ar <<  SD.Xde.at(ie) << SD.Yde.at(ie) << SD.Zde.at(ie) << SD.Lde.at(ie) << SD.Mde.at(ie) << SD.Nde.at(ie);
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        k=0;
        for (int i=0; i<nIntSpares; i++) ar << k;

        nDbleSpares=0;
        ar << nDbleSpares;
        dble=0.0;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if (ArchiveFormat<500000 || ArchiveFormat>500100) return false;

        ar >> SD.m_Span >> SD.m_Area >> SD.m_MAC;
        ar >> SD.m_QInf >> dble  >> dble;
        ar >> SD.m_Mass;
        ar >> SD.m_CoG_x;
        ar >> SD.m_rho;

        // DIMENSIONAL derivatives
        // longitudinal
        ar >>  SD.Xu >> SD.Xw >> SD.Zu >> SD.Zw >> SD.Xq >> SD.Zq >> SD.Mu >> SD.Mw >> SD.Mq;
        ar >>  SD.Zwp>> SD.Mwp;
        // lateral
        ar >>  SD.Yv >> SD.Yp >> SD.Yr >> SD.Lv >> SD.Lp >> SD.Lr >> SD.Nv >> SD.Np >> SD.Nr;

        // control

        if(ArchiveFormat<500002)
        {
            SD.resizeControlDerivatives(1);
            int ie = 0;
            ar >>  SD.Xde[ie] >> SD.Yde[ie] >> SD.Zde[ie] >> SD.Lde[ie] >> SD.Mde[ie] >> SD.Nde[ie];
        }
        else
        {
            ar >> n;
            SD.resizeControlDerivatives(n);
            for(int ie=0; ie<n; ie++)
            {
                ar >> strange; SD.ControlNames[ie] = strange.toStdString();
                ar >> SD.Xde[ie] >> SD.Yde[ie] >> SD.Zde[ie] >> SD.Lde[ie] >> SD.Mde[ie] >> SD.Nde[ie];
            }
        }

        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> k;

        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        SD.computeNDStabDerivatives();
    }
    return true;
}




bool serial::serializeTriangulationFl5(Triangulation & tr, QDataStream &ar, bool bIsStoring)
{
    int n(0);
    float xf(0),yf(0),zf(0);

    Vector3d V0,V1,V2;

    // 500001: new fl5 format
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << tr.nTriangles();
        for(int i=0; i<tr.nTriangles(); i++)
        {
            Triangle3d const &t3d = tr.triangleAt(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500010) return false;

        tr.clear();

        ar >> n;
        tr.triangles().resize(n);
        for(int i3=0; i3<n; i3++)
        {
            ar >> xf >> yf >> zf;
            V0.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V1.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V2.set(double(xf), double(yf), double(zf));

            tr.triangles()[i3].setTriangle(V0, V1, V2);
        }
    }
    return true;
}


bool serial::serializeVortexFl5(Vortex &vtx, QDataStream &ar, bool bIsStoring)
{
    double dble(0);
    int ArchiveFormat = 500001;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << vtx.vertex(0).x<< vtx.vertex(0).y<< vtx.vertex(0).z;
        ar << vtx.vertex(1).x<< vtx.vertex(1).y<< vtx.vertex(1).z;
        ar << vtx.circulation();
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;
        ar >> vtx.vertex(0).x>> vtx.vertex(0).y>> vtx.vertex(0).z;
        ar >> vtx.vertex(1).x>> vtx.vertex(1).y>> vtx.vertex(1).z;
        ar >> dble; vtx.setCirculation(dble);

        vtx.setNodes(vtx.vertex(0), vtx.vertex(1));
    }
    return true;
}


bool serial::serializeVortonFl5(Vorton &vtn, QDataStream &ar, bool bIsStoring)
{
    bool boolean(false);
    double d0(0), d1(0), d2(0);
    // 500001: first format

    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << vtn.position().x << vtn.position().y << vtn.position().z;
        ar << vtn.vortex().x   << vtn.vortex().y   << vtn.vortex().z;
        ar << vtn.volume();
        ar << vtn.isActive();
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>=500002) return false;
        ar >> d0 >> d1 >> d2;
        vtn.setPosition({d0, d1, d2});
        ar >> d0 >> d1 >> d2;
        vtn.setVortex({d0, d1, d2});
        ar >> d0; vtn.setVolume(d0);
        ar >> boolean; vtn.setActive(boolean);
    }
    return true;
}


void serial::serializePanelsFl5(TriMesh &mesh, QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring)
    {
        int nIntSpares=0;
        int nDbleSpares=0;
        int n=0;
        double dble=0.0;

        // 500001: new v7 format
        int ArchiveFormat = 500001;

        ar << ArchiveFormat;
        ar << mesh.panelCount();

        for(int i3=0; i3<mesh.panelCount(); i3++)
        {
            Panel3 const &p3 = mesh.panel(i3);
            for(int in=0; in<3; in++)
            {
                ar << p3.node(in).xf() << p3.node(in).yf() << p3.node(in).zf();
            }
            ar << p3.isPositiveOrientation();
        }

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
        int nIntSpares=0;
        int nDbleSpares=0;
        int n=0;
        double dble=0.0;
        int ArchiveFormat=0;// identifies the format of the file
        bool bPositiveOrientation=false;
        int n3=0;
        float f0=0,f1=0,f2=0;
        Vector3d S[3];
        ar >> ArchiveFormat;
        ar >> n3;

        mesh.clearMesh();
        for(int i3=0; i3<n3; i3++)
        {
            for(int in=0; in<3; in++)
            {
                ar >> f0 >> f1 >> f2;
                S[in].set(double(f0), double(f1), double(f2));
            }
            ar >> bPositiveOrientation;
            mesh.addPanel({S[0], S[1], S[2]});
            mesh.lastPanel().setSurfacePosition(xfl::FUSESURFACE);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        std::string logmsg;

        mesh.makeNodeArrayFromPanels(0, logmsg, "   "); //node and panel indexes are set later at plane assembly time

    }
}


void serial::serializeMeshFl5(TriMesh &mesh, QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring)
    {
        // 500001: new fl5 format
        // 500002: addded TE opposite indexes
        int ArchiveFormat = 500002;


        ar << ArchiveFormat;

        ar << mesh.nNodes();
        for(int in=0; in<mesh.nNodes(); in++)
        {
            Node const &nd = mesh.node(in);
            ar << nd.index();

            ar << nd.xf() << nd.yf() << nd.zf();
            ar << nd.normal().xf() << nd.normal().yf() << nd.normal().zf();
            ar << nd.isTrailing();
            ar << int(nd.neighbourNodeCount());
            for(int ine=0; ine<nd.neighbourNodeCount(); ine++)
                ar << nd.nodeNeighbourIndex(ine);
            ar << int(nd.triangleCount());
            for(int ine=0; ine<nd.triangleCount(); ine++)
                ar << nd.triangleIndex(ine);
        }

        ar << mesh.panelCount();
        for(int i3=0; i3<mesh.panelCount(); i3++)
        {
            Panel3 const &p3 = mesh.panel(i3);
            ar << p3.index();
            ar << p3.nodeIndex(0) << p3.nodeIndex(1) << p3.nodeIndex(2);

            ar << p3.neighbour(0) << p3.neighbour(1) << p3.neighbour(2);
            ar << p3.isPositiveOrientation();
            ar << p3.isTrailing();
            ar << p3.oppositeIndex();
            switch(p3.surfacePosition())
            {
                case xfl::BOTSURFACE:   ar<<0;     break;
                case xfl::MIDSURFACE:   ar<<1;     break;
                case xfl::TOPSURFACE:   ar<<2;     break;
                case xfl::SIDESURFACE:  ar<<3;     break;
                case xfl::FUSESURFACE:  ar<<4;     break;
                case xfl::WAKESURFACE:  ar<<5;     break;
                case xfl::NOSURFACE:    ar<<6;     break;
            }
        }
    }
    else
    {
        bool boolean(false);
        int n(0), ne(0), k(0);
        int ArchiveFormat(0);// identifies the format of the file
        int n3(0);
        int i0(0), i1(0), i2(0);
        float f0(0),f1(0),f2(0);

        mesh.clearMesh();

        ar >> ArchiveFormat;

        ar >>n;
        mesh.nodes().resize(n);
        for(int in=0; in<n; in++)
        {
            Node &nd = mesh.node(in);
            ar >> k;                     nd.setIndex(k);
            ar >> f0 >> f1 >> f2;        nd.set(double(f0), double(f1), double(f2));
            ar >> f0 >> f1 >> f2;        nd.setNormal(double(f0), double(f1), double(f2));
            ar >> boolean;               nd.setTrailing(boolean);

            ar >> ne;
            nd.resizeNodeNeighbours(ne);
            for(int j=0; j<ne; j++)
            {
                ar >> k;                 nd.setNodeNeighbourIndex(j,k);
            }

            ar >> ne;
            nd.resizeTriangles(ne);
            for(int j=0; j<ne; j++)
            {
                ar >> k;                 nd.setTriangleIndex(j,k);
            }
        }

        ar >> n3;
        mesh.panels().resize(n3);
        for(int i3=0; i3<n3; i3++)
        {
            Panel3 &p3 = mesh.panel(i3);
            ar >> k;  p3.setIndex(k);
            ar >> i0 >> i1 >> i2;
            p3.setVertex(0, mesh.node(i0));
            p3.setVertex(1, mesh.node(i1));
            p3.setVertex(2, mesh.node(i2));

            ar >> i0 >> i1 >> i2;
            p3.setNeighbour(0, i0);
            p3.setNeighbour(1, i1);
            p3.setNeighbour(2, i2);

            ar >> boolean; // p3.m_bPositiveOrientation;
            ar >> p3.m_bIsTrailing;
            if(ArchiveFormat>=500002)
            {
                ar >> k;
                p3.setOppositeIndex(k);
            }

            ar >> k;
            switch(k)
            {
                case 0:   p3.setSurfacePosition(xfl::BOTSURFACE);   break;
                case 1:   p3.setSurfacePosition(xfl::MIDSURFACE);   break;
                case 2:   p3.setSurfacePosition(xfl::TOPSURFACE);   break;
                case 3:   p3.setSurfacePosition(xfl::SIDESURFACE);  break;
                case 4:   p3.setSurfacePosition(xfl::FUSESURFACE);  break;
                case 5:   p3.setSurfacePosition(xfl::WAKESURFACE);  break;
                default:
                case 6:   p3.setSurfacePosition(xfl::NOSURFACE);    break;
            }

            p3.setFrame();
        }
        if(ArchiveFormat<500002)
        {
            // cleaning up past errors
            std::vector<int> errorlist;
            mesh.connectTrailingEdges(errorlist);
        }
    }
}


void serial::serializeOccParams(OccMeshParams &params, QDataStream &ar, bool bIsStoring)
{
    int k=0;
    double dble=0;
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << params.m_bLinDefAbs;
        ar << params.m_LinDeflectionAbs;
        ar << params.m_LinDeflectionRel;
        ar << params.m_AngularDeviation;
        ar << dble; //m_MinElementSize;
        ar << params.m_MaxElementSize;
        ar << dble; //m_AutoSize;
        ar << k; //m_TreeMin;
        ar << k; //m_TreeMax;

        k=0;
        ar << k;
        ar << k;
    }
    else
    {
        ar >> ArchiveFormat;
        ar >> params.m_bLinDefAbs;
        ar >> params.m_LinDeflectionAbs;
        ar >> params.m_LinDeflectionRel;
        ar >> params.m_AngularDeviation;
        ar >> dble; //m_MinElementSize;
        ar >> params.m_MaxElementSize;
        ar >> dble; //m_AutoSize;
        ar >> k; //m_TreeMin;
        ar >> k; //m_TreeMax;

        ar >> k;
        ar >> k;
    }
}


void serial::serializeAngleControlFl5(AngleControl &ctrl, QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat = 500750;
    int n=0;
    double dble(0);

    QString strange;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(ctrl.name());
        ar << ctrl.nValues();
        for(int ig=0; ig<ctrl.nValues(); ig++) ar<<ctrl.value(ig);
    }
    else
    {
        ar >> ArchiveFormat;
        ar >> strange; ctrl.setName(strange.toStdString());
        ar >> n;
        ctrl.resizeValues(n);
        for(int ig=0; ig<ctrl.nValues(); ig++)
        {
            ar>>dble;
            ctrl.setValue(ig,dble);
        }
    }
}


void serial::serializeEigenFl5(EigenValues &EV, QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring)
    {
        for(int i=0; i<8; i++)
            ar << EV.m_EV[i].real() << EV.m_EV[i].imag();
    }
    else
    {
        double real=0.0, imag=0.0;
        for(int i=0; i<8; i++)
        {
            ar >> real >> imag;
            EV.m_EV[i] = std::complex<double>(real, imag);
        }
    }
}


bool serial::serializeNaca4Spline(Naca4Spline &spline, QDataStream &ar, bool bIsStoring)
{
    if(bIsStoring)
    {
        ar << spline.m << spline.p;
    }
    else
    {
        ar >> spline.m >> spline.p;
        spline.updateSpline();
        spline.makeCurve();
    }
    return true;
}


void serial::serializeBLDataFl5(BLData &data, QDataStream &ar, bool bIsStoring)
{
    //500001 : first fl5 format
    int n=0;
    int nVariables = 23;

    std::vector<float> fl(nVariables, 0);

    double dble=0.0;
    int nIntSpares=0;
    int nDbleSpares=0;

    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        switch(data.BLMethod)
        {
            case BL::XFOIL:        n=3;   break;
            case BL::NOBLMETHOD:   n=5;   break;
        }
        ar << n;

        switch(data.Side)
        {
            default:
            case BL::TOP:    n = 0;  break;
            case BL::BOTTOM: n = 1;  break;
            case BL::WAKE:   n = 2;  break;
        }
        ar << n;

        ar << data.iLE <<data.nTE;
        ar << data.bIsConverged;
        ar << data.QInf << data.CL << data.Cm << data.XCP << data.Cd_SY << data.XTr << data.XLamSep << data.XTurbSep;


        ar << int(data.s.size()); // number of data points
        ar << nVariables; // adjustable number of variables for future growth
        for(uint in=0; in<data.s.size(); in++)
        {
            ar << float(data.s[in]);
            ar << float(data.Qi[in]);
            ar << float(data.Qv[in]);
            ar << float(data.CTau[in]);
            ar << float(data.CTq[in]);
            ar << float(data.Cd[in]);
            ar << float(data.Cf[in]);
            ar << float(data.tauw[in]);
            ar << float(data.H[in]);
            ar << float(data.HStar[in]);
            ar << float(data.delta3[in]);
            ar << float(data.dstar[in]);
            ar << float(data.nTS[in]);
            ar << float(data.theta[in]);
            ar << float(data.delta[in]);
            ar << float(data.gamtr[in]);
            data.bConverged[in] ? ar<<1.0f : ar<<0.0f;
            ar << float(data.foilnode[in].index());
            ar << float(data.foilnode[in].x) << float(data.foilnode[in].y);
            ar << float(data.foilnode[in].normal().x) << float(data.foilnode[in].normal().y);
            ar << (data.foilnode[in].isWakeNode() ? 1.0f : 0.0f);
        }

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

        ar >> n;
        switch(n)
        {
            default:
            case 3: data.BLMethod=BL::XFOIL;          break;
            case 5: data.BLMethod=BL::NOBLMETHOD;     break;
        }

        ar >> n;
        switch(n)
        {
            case 0: data.Side=BL::TOP;      break;
            case 1: data.Side=BL::BOTTOM;   break;
            case 2: data.Side=BL::WAKE;     break;
        }

        ar >> data.iLE >> data.nTE;
        ar >> data.bIsConverged;
        ar >> data.QInf >> data.CL >> data.Cm >> data.XCP >> data.Cd_SY >> data.XTr >> data.XLamSep >> data.XTurbSep;

        ar >> n; // number of data points
        data.resizeData(n, false);
        ar >> nVariables;
        for(int in=0; in<n; in++)
        {
            for(int iv=0; iv<nVariables; iv++) ar >> fl[iv];
            data.s[in]      = double(fl[0]);
            data.Qi[in]     = double(fl[1]);
            data.Qv[in]     = double(fl[2]);
            data.CTau[in]   = double(fl[3]);
            data.CTq[in]    = double(fl[4]);
            data.Cd[in]     = double(fl[5]);
            data.Cf[in]     = double(fl[6]);
            data.tauw[in]   = double(fl[7]);
            data.H[in]      = double(fl[8]);
            data.HStar[in]  = double(fl[9]);
            data.delta3[in] = double(fl[10]);
            data.dstar[in]  = double(fl[11]);
            data.nTS[in]    = double(fl[12]);
            data.theta[in]  = double(fl[13]);
            data.delta[in]  = double(fl[14]);
            data.gamtr[in]  = double(fl[15]);
            data.bConverged[in] = double(fl[16])<0.5 ? false : true;
            data.foilnode[in].setIndex(int(fl[17]));
            data.foilnode[in].set(double(fl[18]), double(fl[19]));
            data.foilnode[in].setNormal(Vector2d(double(fl[20]), double(fl[21])));
            data.foilnode[in].setWakeNode(double(fl[22])>0.5);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
}


bool serial::serializeWingSectionFl5(WingSection &ws, QDataStream &ar, bool bIsStoring)
{
    int k=0;
    double dble=0;
    QString strange;

    // 500001 : new fl5 format;
    int ArchiveFormat = 500001;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(ws.m_RightFoilName);
        ar << QString::fromStdString(ws.m_LeftFoilName);
        ar << ws.chord();
        ar << ws.yPosition();
        ar << ws.offset();
        ar << ws.dihedral();
        ar << ws.twist();
        ar << ws.nXPanels();
        ar << ws.nYPanels();

        switch(ws.xDistType())
        {
            case xfl::COSINE:      ar <<  1;  break;
            case xfl::SINE:        ar <<  2;  break;
            case xfl::INV_SINE:    ar << -2;  break;
            case xfl::INV_SINH:    ar <<  3;  break;
            case xfl::TANH:        ar <<  4;  break;
            case xfl::EXP:         ar <<  5;  break;
            case xfl::INV_EXP:     ar <<  6;  break;
            case xfl::UNIFORM:
            default:               ar <<  0;  break;
        }

        switch(ws.yDistType())
        {
            case xfl::COSINE:      ar <<  1;  break;
            case xfl::SINE:        ar <<  2;  break;
            case xfl::INV_SINE:    ar << -2;  break;
            case xfl::INV_SINH:    ar <<  3;  break;
            case xfl::TANH:        ar <<  4;  break;
            case xfl::EXP:         ar <<  5;  break;
            case xfl::INV_EXP:     ar <<  6;  break;
            case xfl::UNIFORM:
            default:               ar <<  0;  break;
        }

        // space allocation for the future storage of more data, without need to change the format
        int nSpares=10;
        ar << nSpares;
        for (int i=0; i<nSpares; i++) ar << 0;
        for (int i=0; i<nSpares; i++) ar << 0.0;

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        ar >> strange;   ws.m_RightFoilName = strange.toStdString();
        ar >> strange;   ws.m_LeftFoilName  = strange.toStdString();
        ar >> ws.m_Chord;
        ar >> ws.m_YPosition;
        ar >> ws.m_Offset;
        ar >> ws.m_Dihedral;
        ar >> ws.m_Twist;
        ar >> ws.m_NXPanels;
        ar >> ws.m_NYPanels;

        ar >> k;
        if     (k==1)  ws.m_XPanelDist = xfl::COSINE;
        else if(k==2)  ws.m_XPanelDist = xfl::SINE;
        else if(k==-2) ws.m_XPanelDist = xfl::INV_SINE;
        else if(k==3)  ws.m_XPanelDist = xfl::INV_SINH;
        else if(k==4)  ws.m_XPanelDist = xfl::TANH;
        else if(k==5)  ws.m_XPanelDist = xfl::EXP;
        else if(k==6)  ws.m_XPanelDist = xfl::INV_EXP;
        else           ws.m_XPanelDist = xfl::UNIFORM;

        ar >> k;
        if     (k==1)  ws.m_YPanelDist = xfl::COSINE;
        else if(k==2)  ws.m_YPanelDist = xfl::SINE;
        else if(k==-2) ws.m_YPanelDist = xfl::INV_SINE;
        else if(k==3)  ws.m_YPanelDist = xfl::INV_SINH;
        else if(k==4)  ws.m_YPanelDist = xfl::TANH;
        else if(k==5)  ws.m_YPanelDist = xfl::EXP;
        else if(k==6)  ws.m_YPanelDist = xfl::INV_EXP;
        else           ws.m_YPanelDist = xfl::UNIFORM;


        // space allocation
        int nSpares=0;
        ar >> nSpares;
        for (int i=0; i<nSpares; i++) ar >> k;
        for (int i=0; i<nSpares; i++) ar >> dble;

        return true;
    }
}


void serial::serializeLineStyleXfl(LineStyle &ls, QDataStream &ar, bool bIsStoring)
{
    int k=0;
    if(bIsStoring)
    {
        ar << LineStyle::convertLineStyle(ls.m_Stipple);
        ar << ls.m_Width;
        ar << LineStyle::convertSymbol(ls.m_Symbol);
        serial::serializefl5Color(ls.m_Color, ar, true);
        ar << ls.m_bIsVisible;
    }
    else
    {
        ar >> k; ls.m_Stipple=LineStyle::convertLineStyle(k);
        ar >> ls.m_Width;
        ar >> k; ls.m_Symbol=LineStyle::convertSymbol(k);
        serial::serializefl5Color(ls.m_Color, ar, false);
        ar >> ls.m_bIsVisible;
    }
}


void serial::serializeLineStyleFl5(LineStyle &ls, QDataStream &ar, bool bIsStoring)
{
    int k=0;
    QString strange;

    // 500756: serialized fl5Color in place of QColor - no mod.
    int ArchiveFormat = 500756;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << LineStyle::convertLineStyle(ls.m_Stipple);
        ar << ls.m_Width;
        ar << LineStyle::convertSymbol(ls.m_Symbol);
        serial::serializefl5Color(ls.m_Color, ar, true);
        ar << ls.m_bIsVisible;
        ar << QString::fromStdString(ls.m_Tag);
    }
    else
    {
        ar >> k;
        if(k<500001)
        {
            // --> v712 format
            ls.m_Stipple=LineStyle::convertLineStyle(k);
            ar >> ls.m_Width;
            ar >> k; ls.m_Symbol=LineStyle::convertPointStyle_old(k);
            serial::serializefl5Color(ls.m_Color, ar, false);
            ar >> ls.m_bIsVisible;
            ar >> strange;   ls.m_Tag=strange.toStdString();
        }
        else
        {
            // v713+ format
            ar >> k; ls.m_Stipple=LineStyle::convertLineStyle(k);
            ar >> ls.m_Width;
            ar >> k; ls.m_Symbol=LineStyle::convertSymbol(k);
            serial::serializefl5Color(ls.m_Color, ar, false);
            ar >> ls.m_bIsVisible;
            ar >> strange;   ls.m_Tag = strange.toStdString();

        }
    }
}


bool serial::serializeInertiaFl5(Inertia &inertia, QDataStream &ar, bool bIsStoring)
{
    int nIntSpares=5;
    int nDbleSpares=5;
    int k=0;
    double dble(0), d0(0), d1(0), d2(0);
    QString strange;

    //500001 : new fl5 format;
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << inertia.structuralMass();
        ar << inertia.CoG_s().x << inertia.CoG_s().y << inertia.CoG_s().z;
        ar << inertia.Ixx_s() << inertia.Ixy_s() << inertia.Ixz_s();
        ar << inertia.Iyy_s() << inertia.Iyz_s() << inertia.Izz_s();

        ar << inertia.pointMassCount();
        for(int im=0; im<inertia.pointMassCount(); im++)
        {
            PointMass const &pm = inertia.pointMassAt(im);
            ar << pm.mass();
            ar << pm.position().x << pm.position().y <<pm.position().z;
            ar << QString::fromStdString(pm.tag());
        }

        k=0;
        ar << nIntSpares;
        for(int i=0; i<nIntSpares; i++) ar<<k;

        ar << nDbleSpares;
        dble = 0;
        for(int i=0; i<nDbleSpares; i++) ar<<dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        ar >> dble; inertia.setStructuralMass(dble);

        ar >> d0 >> d1 >> d2;
        inertia.setCoG_s({d0,d1,d2});


        ar >> d0 >> d1 >> d2;
        inertia.setIxx_s(d0);
        inertia.setIxy_s(d1);
        inertia.setIxz_s(d2);
        ar >> d0 >> d1 >> d2;
        inertia.setIyy_s(d0);
        inertia.setIyz_s(d1);
        inertia.setIzz_s(d2);

        inertia.clearPointMasses();
        ar >> k;
        double m=0,px=0,py=0,pz=0;
        for(int im=0; im<k; im++)
        {
            ar >> m >> px >> py >> pz;
            ar >> strange;
            inertia.appendPointMass(m, Vector3d(px, py, pz), strange.toStdString());
        }

        ar >> nIntSpares;
        for(int i=0; i<nIntSpares; i++) ar>>k;
        ar >> nDbleSpares;
        for(int i=0; i<nDbleSpares; i++) ar>>dble;
    }
    return true;
}

unsigned int qt_div_257_floor(unsigned int x) { return  (x - (x >> 8)) >> 8; }
unsigned int qt_div_257(unsigned int x) { return qt_div_257_floor(x + 128); }

// in place replacement for QColor serialization
void serial::serializefl5Color(fl5Color &color, QDataStream &stream, bool bIsStoring)
{
    if(bIsStoring)
    {
        signed char s(0);
        unsigned short a = color.m_Alpha * 0x101;
        unsigned short r = color.m_Red   * 0x101;
        unsigned short g = color.m_Green * 0x101;
        unsigned short b = color.m_Blue  * 0x101;
        unsigned short p(0);
        stream << s;
        stream << a;
        stream << r;
        stream << g;
        stream << b;
        stream << p;
    }
    else
    {
        signed char s(0);
        unsigned short a(0), r(0), g(0), b(0), p(0);
        stream >> s;
        stream >> a;
        stream >> r;
        stream >> g;
        stream >> b;
        stream >> p;

        color.m_Alpha = qt_div_257(a);
        color.m_Red   = qt_div_257(r);
        color.m_Green = qt_div_257(g);
        color.m_Blue  = qt_div_257(b);
    }
}












