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

#include <fstream>
#include <string>


#include <objects2d_globals.h>

#include <constants.h>
#include <foil.h>
#include <polar.h>
#include <utils.h>



bool objects::readFoilFile(const std::string &filename, Foil *pFoil, int &iLineError)
{
    std::string line;
    std::string FoilName;

    std::vector<Node2d> basenodes;

    std::ifstream file(filename);


    int iLine(0);

    if(!file.is_open())
    {
        iLineError = 0;
        return false;
    }

    FoilName = filename;

    float val[] {0,0,0};

    // identify and read the first non-empty line
    while (std::getline(file, line))
    {
        iLine++;
        xfl::trim(line);

        if (line.length()==0) continue;

        if(xfl::readValues(line, val, 2)==2)
        {
            //there isn't a name on the first line, use the file's name
            FoilName = filename;
            // store initial coordinates
            basenodes.push_back({val[0], val[1]});
        }
        else FoilName = line;

        break;
    }

    // read coordinates
    while (std::getline(file, line))
    {
        iLine++;
        xfl::trim(line);
        if(line.length()==0) continue;

        if(xfl::readValues(line, val, 2)==2)
        {
            basenodes.push_back({val[0], val[1]});
        }
        else
        {
            // non-empty but unreadable line, abort
            iLineError = iLine;
            return false;
        }
    }

    pFoil->setName(FoilName);

    // Check if the foil was written clockwise or counter-clockwise
    int ip = 0;
    double area = 0.0;
    for (int i=0; i<pFoil->nBaseNodes(); i++)
    {
        if(i==pFoil->nBaseNodes()-1) ip = 0;
        else                         ip = i+1;
        area +=  0.5*(pFoil->yb(i)+pFoil->yb(ip))*(pFoil->xb(i)-pFoil->xb(ip));
    }

    if(area < 0.0)
    {
        //reverse the points order
        double xtmp(0), ytmp(0);
        for (int i=0; i<pFoil->nBaseNodes()/2; i++)
        {
            xtmp         = pFoil->xb(i);
            ytmp         = pFoil->yb(i);
            basenodes[i].x = pFoil->xb(pFoil->nBaseNodes()-i-1);
            basenodes[i].y = pFoil->yb(pFoil->nBaseNodes()-i-1);
            basenodes[pFoil->nBaseNodes()-i-1].x = xtmp;
            basenodes[pFoil->nBaseNodes()-i-1].y = ytmp;
        }
    }

    pFoil->setBaseNodes(basenodes);
    pFoil->initGeometry();

    return true;
}


bool objects::readPolarFile(QFile &plrFile, std::vector<Foil*> &foilList, std::vector<Polar*> &polarList)
{
    Foil* pFoil(nullptr);
    Polar *pPolar(nullptr);
    Polar *pOldPolar(nullptr);
    int n(0);

    QDataStream ar(&plrFile);
    ar.setVersion(QDataStream::Qt_4_5);
    ar.setByteOrder(QDataStream::LittleEndian);

    ar >> n;

    if(n<100000)
    {
        // deprecated format
        return false;
    }
    else if (n >=100000 && n<200000)
    {
        //new format XFLR5 v1.99+
        //first read all available foils
        ar>>n;
        for (int i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!objects::serializeFoil(pFoil, ar))
            {
                delete pFoil;
                return false;
            }
            foilList.push_back(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0; i<n; i++)
        {
            pPolar = new Polar();

            if (!objects::serializePolarv6(pPolar, ar, false))
            {
                delete pPolar;
                return false;
            }
            for (uint l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);
                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.erase(polarList.begin()+l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.push_back(pPolar);
        }
    }
    else if (n >=500000 && n<600000)
    {
        // v7 format
        // number of foils to read
        ar>>n;
        for (int i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!pFoil->serializeFl5(ar, false))
            {
                delete pFoil;
                return false;
            }
            foilList.push_back(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0;i<n; i++)
        {
            pPolar = new Polar();

            if (!pPolar->serializePolarFl5(ar, false))
            {
                delete pPolar;
                return false;
            }

            for (uint l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);

                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.erase(polarList.begin()+l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.push_back(pPolar);
        }
    }
    return true;
}

bool objects::serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring)
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
        xfl::writeString(ar, pPolar->m_FoilName);
        xfl::writeString(ar, pPolar->name());

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
        xfl::writeColor(ar, pPolar->lineColor().red(), pPolar->lineColor().green(), pPolar->lineColor().blue());

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

        xfl::readString(ar, strange); pPolar->setFoilName(strange);
        xfl::readString(ar, strange); pPolar->setName(strange);

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
            xfl::readColor(ar, r, g, b);
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
        else pPolar->theStyle().serializeXfl(ar, bIsStoring);

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


bool objects::serializeFoil(Foil *pFoil, QDataStream &ar)
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
        xfl::readString(ar, strange);
        pFoil->setName(strange);
        if(ArchiveFormat>=1006)
        {
            xfl::readString(ar, strange);
            pFoil->setDescription(strange);
        }
        if(ArchiveFormat>=1002)
        {
            ar >> p;
            pFoil->setLineStipple(LineStyle::convertLineStyle(p));
            ar >> p; pFoil->setLineWidth(p);
            int r=0,g=0,b=0;
            xfl::readColor(ar, r, g, b);
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



