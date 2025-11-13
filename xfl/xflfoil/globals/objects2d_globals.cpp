/****************************************************************************

    flow5
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

#include <QFileInfo>

#include "objects2d_globals.h"


#include <xflcore/xflcore.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>
#include <xflmath/constants.h>


void drawFoil(QPainter &painter, Foil const*pFoil, double alpha, double twist, double scalex, double scaley,
              QPointF const &Offset, bool bFill, QColor fillClr)
{
    if(pFoil->nNodes()<=0) return;

    painter.save();
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cost = cos(twist*PI/180.0);
    double sint = sin(twist*PI/180.0);

    QPolygonF polyline(pFoil->nNodes());

    double xa(0), ya(0);
    double xr(0), yr(0);
    for(int k=0; k<pFoil->nNodes(); k++)
    {
        xa = (pFoil->x(k)-0.5)*cosa - pFoil->y(k)*sina+ 0.5;
        ya = (pFoil->x(k)-0.5)*sina + pFoil->y(k)*cosa;
        xr = xa*cost - ya*sint;
        yr = xa*sint + ya*cost;
        polyline[k].rx() =  xr;
        polyline[k].ry() = -yr;
    }

    painter.translate(Offset);
    painter.scale(scalex, scaley);

    if(bFill)
    {
        painter.setBrush(fillClr);
        painter.drawPolygon(polyline);
    }
    else
        painter.drawPolyline(polyline);
    painter.restore();
}


void drawFoilNormals(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley, QPointF const &Offset)
{
    QPointF From, To;

    QPen NormalPen;

    NormalPen.setColor(pFoil->lineColor().darker());
    NormalPen.setWidth(1);
    NormalPen.setStyle(Qt::SolidLine);
    painter.setPen(NormalPen);

    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);

    for (int k=0; k<pFoil->nNodes(); k++)
    {
        double xa = (pFoil->x(k)-0.5)*cosa - pFoil->y(k)*sina+ 0.5;
        double ya = (pFoil->x(k)-0.5)*sina + pFoil->y(k)*cosa;
        From.rx() =  xa*scalex+Offset.x();
        From.ry() = -ya*scaley+Offset.y();

        double nx = pFoil->normal(k).x*cosa - pFoil->normal(k).y*sina;
        double ny = pFoil->normal(k).x*sina + pFoil->normal(k).y*cosa;

        xa += nx/10.0;
        ya += ny/10.0;

        To.rx() =  xa*scalex+Offset.x();
        To.ry() = -ya*scaley+Offset.y();

        painter.drawLine(From,To);
    }
}


void drawFoilMidLine(QPainter &painter, Foil const*pFoil, double scalex, double scaley, QPointF const &Offset)
{
    painter.save();
    painter.translate(Offset);
    painter.scale(scalex, scaley);

    if(pFoil->CbLine().size()>0)
    {
        QPolygonF camberline;
        for (int k=0; k<pFoil->CbLine().size(); k++)
        {
            camberline.append({  pFoil->CbLine().at(k).x, -pFoil->CbLine().at(k).y});
        }
        painter.drawPolyline(camberline);
    }
    painter.restore();
}


void drawFoilPoints(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley,
                    QPointF const &Offset, QColor const &backColor, QRect const &drawrect)
{
    QPen FoilPen;
    FoilPen.setColor(pFoil->lineColor());
    FoilPen.setWidth(pFoil->lineWidth());
    FoilPen.setStyle(Qt::SolidLine);
    FoilPen.setCosmetic(true);
    painter.setPen(FoilPen);

    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);

    for (int i=0; i<pFoil->nNodes();i++)
    {
        double xa = (pFoil->x(i)-0.5)*cosa - pFoil->y(i)*sina + 0.5;
        double ya = (pFoil->x(i)-0.5)*sina + pFoil->y(i)*cosa;

        QPointF pt( xa*scalex + Offset.x(), -ya*scaley + Offset.y());

        if(drawrect.contains(int(pt.x()), int(pt.y())))
            xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, pFoil->lineColor(), pt);
    }

    int ih = pFoil->iSelectedPt();


    if(ih>=0 && ih<int(pFoil->nNodes()))
    {
        QPen HighPen;
        HighPen.setCosmetic(true);
        HighPen.setColor(QColor(255,0,0));
        HighPen.setWidth(2);
        painter.setPen(HighPen);

        double xa = (pFoil->x(ih)-0.5)*cosa - pFoil->y(ih)*sina + 0.5;
        double ya = (pFoil->x(ih)-0.5)*sina + pFoil->y(ih)*cosa;

        QPointF pt( xa*scalex + Offset.x(), -ya*scaley + Offset.y());

        xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, QColor(255,0,0), pt);
    }
}


bool readFoilFile(QFile &FoilFile, Foil *pFoil)
{
    QString line;
    QString FoilName;
    double x(0), y(0), z(0);

    QVector<Node2d> basenodes;

    QTextStream inStream(&FoilFile);

    QFileInfo fi(FoilFile);
    QString filename = fi.baseName();

    // identify and read the first non-empty line
    while(!inStream.atEnd())
    {
        line = inStream.readLine();
        if(line.isNull()) return false; // premature end of file, file is unreadable
        if (line.isEmpty()) continue;

        line = line.trimmed();

        if(xfl::readValues(line,x,y,z)==2)
        {
            //there isn't a name on the first line, use the file's name
            FoilName = filename;
            // store initial coordinates
            basenodes.push_back({x,y});
        }
        else FoilName = line;

        FoilName = FoilName.trimmed();
        break;
    }

    // read coordinates
    do
    {
        line = inStream.readLine();
        if(line.isNull()) break; // end of file
        if(line.isEmpty()) continue;

        if(xfl::readValues(line, x,y,z)==2)
        {
            basenodes.push_back({x,y});
        }
        else
        {
            // non-empty but unreadable line, abort
            break;
        }
    }while(true);

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


bool readPolarFile(QFile &plrFile, QVector<Foil*> &foilList, QVector<Polar*> &polarList)
{
    Foil* pFoil(nullptr);
    Polar *pPolar(nullptr);
    Polar *pOldPolar(nullptr);
    int n(0), l(0);

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
            if (!serializeFoil(pFoil, ar, false))
            {
                delete pFoil;
                return false;
            }
            foilList.append(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0; i<n; i++)
        {
            pPolar = new Polar();

            if (!serializePolarv6(pPolar, ar, false))
            {
                delete pPolar;
                return false;
            }
            for (l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);
                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.removeAt(l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.append(pPolar);
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
            foilList.append(pFoil);
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
            for (l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);

                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.removeAt(l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.append(pPolar);
        }
    }
    return true;
}


Polar *importXFoilPolar(QFile &txtFile, QString &logmsg)
{
    double Re(0), alpha(0), CL(0), CD(0), CDp(0), CM(0), Xt(0), Xb(0),Cpmn(0), HMom(0);
    QString FoilName, strong, strange, str;
    bool bRead = false;

    if (!txtFile.open(QIODevice::ReadOnly))
    {
        strange = "Could not open the file "+txtFile.fileName();
        logmsg += strange;
        return nullptr;
    }
    Polar *pPolar = new Polar;

    QTextStream in(&txtFile);
    int Line = 0;
    bool bOK=false, bOK2=false;

    xfl::readAVLString(in, Line, strong);    // XFoil or XFLR5 version
    xfl::readAVLString(in, Line, strong);    ;// Foil Name

    FoilName = strong.right(strong.length()-22);
    FoilName = FoilName.trimmed();

    pPolar->setFoilName(FoilName);

    xfl::readAVLString(in, Line, strong);// analysis type

    int retype = strong.mid(0,2).toInt(&bOK);
    if(bOK) pPolar->setReType(retype);
    int matype = strong.mid(2,2).toInt(&bOK2);
    if(bOK) pPolar->setMaType(matype);

    if(!bOK || !bOK2)
    {
        str = QString::asprintf("Error reading line %d: Unrecognized Mach and Reynolds type.\nThe polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";

        return nullptr;
    }
    if     (pPolar->ReType() ==1 && pPolar->MaType() ==1) pPolar->setType(xfl::T1POLAR);
    else if(pPolar->ReType() ==2 && pPolar->MaType() ==2) pPolar->setType(xfl::T2POLAR);
    else if(pPolar->ReType() ==3 && pPolar->MaType() ==1) pPolar->setType(xfl::T3POLAR);
    else                                                  pPolar->setType(xfl::T1POLAR);

    bRead = xfl::readAVLString(in, Line, strong);
    if(!bRead || strong.length() < 34)
    {
        str = QString::asprintf("Error reading line %d. The polar(s) will not be stored.",Line);
        delete pPolar;

        logmsg += str+"\n";
        return nullptr;
    }

    double xtr = strong.mid(9,6).toDouble(&bOK);
    if(bOK) pPolar->setXTripBot(xtr);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Bottom Transition value at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    xtr = strong.mid(28,6).toDouble(&bOK);
    if(bOK) pPolar->setXTripTop(xtr);

    if(!bOK)
    {
        str = QString::asprintf("Error reading Top Transition value at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;

        logmsg += str+"\n";
        return nullptr;
    }

    // Mach     Re     NCrit
    bRead = xfl::readAVLString(in, Line, strong);// blank line
    if(!bRead || strong.length() < 50)
    {
        str = QString::asprintf("Error reading line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    double Ma = strong.mid(8,6).toDouble(&bOK);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Mach Number at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }
    else
        pPolar->setMach(Ma);

    Re = strong.mid(24,10).toDouble(&bOK);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Reynolds Number at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }
    Re *=1000000.0;
    pPolar->setReynolds(Re);

    double ncrit = strong.mid(52,8).toDouble(&bOK);
    if(bOK) pPolar->setNCrit(ncrit);
    if(!bOK)
    {
        str = QString::asprintf("Error reading NCrit at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    xfl::readAVLString(in, Line, strong);// column titles
    bRead = xfl::readAVLString(in, Line, strong);// underscores


    while(bRead && !in.atEnd())
    {
        bRead = xfl::readAVLString(in, Line, strong);// polar data
        if(strong.length())
        {
            if(strong.length())
            {
                //                textline = strong.toLatin1();
                //                text = textline.constData();
                //                res = sscanf(text, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &alpha, &CL, &CD, &CDp, &CM, &Xt, &Xb, &Cpmn, &HMom);

                //Do this the C++ way
                QStringList values;
#if QT_VERSION >= 0x050F00
                values = strong.split(" ", Qt::SkipEmptyParts);
#else
                values = strong.split(" ", QString::SkipEmptyParts);
#endif

                if(values.length()>=7)
                {
                    alpha  = values.at(0).toDouble();
                    CL     = values.at(1).toDouble();
                    CD     = values.at(2).toDouble();
                    CDp    = values.at(3).toDouble();
                    CM     = values.at(4).toDouble();
                    Xt     = values.at(5).toDouble();
                    Xb     = values.at(6).toDouble();

                    if(values.length() >= 9)
                    {
                        Cpmn    = values.at(7).toDouble();
                        HMom    = values.at(8).toDouble();
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, Cpmn, HMom, Re, 0, 0, Xt, Xb, 0, 0, 0, 0);
                    }
                    else
                    {
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, 0.0, 0.0,Re,0.0,0.0, Xt, Xb, 0, 0, 0, 0);

                    }
                }
            }
        }
    }
    txtFile.close();

    Re = pPolar->Reynolds()/1000000.0;
    QString name = QString("T%1_Re%2_M%3")
            .arg(pPolar->type()+1)
            .arg(Re,0,'f',2)
            .arg(pPolar->Mach(),0,'f',2);
    str = QString("_N%1").arg(pPolar->NCrit(),0,'f',1);
    name += str;
    pPolar->setName(name);


    return pPolar;
}


bool serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    int n(0), l(0), k(0);
    int ArchiveFormat(0);// identifies the format of the file
    float f(0);
//    double dble(0);


    if(bIsStoring)
    {
        //write variables
        n = pPolar->m_Alpha.size();

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

        for (int i=0; i< pPolar->m_Alpha.size(); i++){
            ar << float(pPolar->m_Alpha.at(i)) << float(pPolar->m_Cd.at(i)) ;
            ar << float(pPolar->m_Cdp.at(i))   << float(pPolar->m_Cl.at(i)) << float(pPolar->m_Cm.at(i));
            ar << float(pPolar->m_XTrTop.at(i))  << float(pPolar->m_XTrBot.at(i));
            ar << float(pPolar->m_HMom.at(i))  << float(pPolar->m_Cpmn.at(i));
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
        QString strange;
        float Alpha=0, Cd(0), Cdp(0), Cl(0), Cm(0), XTr1(0), XTr2(0), HMom(0), Cpmn(0), Re(0), XCp(0);
        int iRe(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat <1001 || ArchiveFormat>1100)
        {
            return false;
        }

        xfl::readString(ar, pPolar->m_FoilName);
        xfl::readString(ar, strange); pPolar->setName(strange);

        if(pPolar->m_FoilName.isEmpty() || pPolar->name().isEmpty())
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
            int r,g,b;
            xfl::readColor(ar, r,g,b);
            pPolar->setLineColor({r,g,b});
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
                for (int j=0; j<pPolar->m_Alpha.size(); j++)
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
                for (int j=0; j<pPolar->m_Re.size(); j++)
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


bool serializeFoil(Foil *pFoil, QDataStream &ar, bool bIsStoring)
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

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        xfl::writeString(ar, pFoil->name());
        xfl::writeString(ar, pFoil->description());
        ar << pFoil->theStyle().m_Stipple << pFoil->theStyle().m_Width;
        xfl::writeColor(ar, pFoil->lineColor().red(), pFoil->lineColor().green(), pFoil->lineColor().blue());

        if (pFoil->theStyle().m_bIsVisible)  ar << 1; else ar << 0;
        if (pFoil->theStyle().m_Symbol>0)    ar << 1; else ar << 0;//1004
        if (pFoil->isCamberLineVisible())    ar << 1; else ar << 0;//1004
        if (pFoil->hasLEFlap())        ar << 1; else ar << 0;
        ar << float(pFoil->LEFlapAngle()) << float(pFoil->LEXHinge()) << float(pFoil->LEYHinge());
        if (pFoil->hasTEFlap())        ar << 1; else ar << 0;
        ar << float(pFoil->TEFlapAngle()) << float(pFoil->TEXHinge()) << float(pFoil->TEYHinge());

        ar << 1.f << 1.f << 9.f;//formerly transition parameters
        ar << pFoil->nBaseNodes();
        for (int jl=0; jl<pFoil->nBaseNodes(); jl++)
        {
            ar << float(pFoil->xb(jl)) << float(pFoil->yb(jl));
        }
        ar << pFoil->nNodes();
        for (int jl=0; jl<pFoil->nNodes(); jl++)
        {
            ar << float(pFoil->x(jl)) << float(pFoil->y(jl));
        }
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000||ArchiveFormat>1010)
            return false;

        QString strange;
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
            pFoil->setLineColor(QColor(r,g,b));
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

        QVector<Node2d> basenodes(p);
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

