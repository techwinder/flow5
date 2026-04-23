/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    
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


#include <QFileInfo>

#include <QDir>

#include <api/xflxmlreader.h>


#include <api/extradrag.h>
#include <api/foil.h>
#include <api/fusexfl.h>
#include <api/inertia.h>
#include <api/nurbssurface.h>
#include <api/objects2d.h>
#include <api/objects2d_globals.h>
#include <api/pointmass.h>
#include <api/polar3d.h>
#include <api/utils.h>
#include <api/wingxfl.h>



XflXmlReader::XflXmlReader(QFile &file) : QXmlStreamReader()
{
    m_VMajor = 1;
    m_VMinor = 0;

    QFileInfo fi(file);
    m_FileName = fi.fileName();
    m_Path = fi.absolutePath();
    setDevice(&file);
}


XflXmlReader::~XflXmlReader()
{
}

bool XflXmlReader::readTheStyle(LineStyle &theStyle)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if      (name().compare(QString("Width"), Qt::CaseInsensitive)==0)
        {
            theStyle.m_Width = 1;
            bool bOk = false;
            int w = readElementText().trimmed().toInt(&bOk);
            if(bOk) theStyle.m_Width = w;
        }
        else if (name().compare(QString("Stipple"), Qt::CaseInsensitive)==0)
        {
            theStyle.m_Stipple = Line::SOLID; // the default

            QString stipple = readElementText().trimmed();
            if     (stipple.compare("SOLID",      Qt::CaseInsensitive)==0) theStyle.m_Stipple = Line::SOLID;
            else if(stipple.compare("DASH",       Qt::CaseInsensitive)==0) theStyle.m_Stipple = Line::DASH;
            else if(stipple.compare("DOT",        Qt::CaseInsensitive)==0) theStyle.m_Stipple = Line::DOT;
            else if(stipple.compare("DASHDOT",    Qt::CaseInsensitive)==0) theStyle.m_Stipple = Line::DASHDOT;
            else if(stipple.compare("DASHDOTDOT", Qt::CaseInsensitive)==0) theStyle.m_Stipple = Line::DASHDOTDOT;
            else if(stipple.compare("NOLINE",     Qt::CaseInsensitive)==0) theStyle.m_Stipple = Line::NOLINE;
        }
        else if (name().compare(QString("PointStyle"), Qt::CaseInsensitive)==0)
        {
            theStyle.m_Symbol = Line::NOSYMBOL;

            QString ptstyle = readElementText().trimmed();
            if     (ptstyle.compare("NOSYMBOL",           Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::NOSYMBOL;
            else if(ptstyle.compare("LITTLECIRCLE",       Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::LITTLECIRCLE;
            else if(ptstyle.compare("BIGCIRCLE",          Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::BIGCIRCLE;
            else if(ptstyle.compare("LITTLESQUARE",       Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::LITTLESQUARE;
            else if(ptstyle.compare("BIGSQUARE",          Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::BIGSQUARE;
            else if(ptstyle.compare("TRIANGLE",           Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::TRIANGLE;
            else if(ptstyle.compare("TRIANGLE_INV",       Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::TRIANGLE_INV;
            else if(ptstyle.compare("LITTLECROSS",        Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::LITTLECROSS;
            else if(ptstyle.compare("BIGCROSS",           Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::BIGCROSS;
            else if(ptstyle.compare("LITTLECIRCLEFILLED", Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::LITTLECIRCLE_F;
            else if(ptstyle.compare("BIGCIRCLEFILLED",    Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::BIGCIRCLE_F;
            else if(ptstyle.compare("LITTLESQUAREFILLED", Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::LITTLESQUARE_F;
            else if(ptstyle.compare("BIGSQUAREFILLED",    Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::BIGSQUARE_F;
            else if(ptstyle.compare("TRIANGLEFILLED",     Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::TRIANGLE_F;
            else if(ptstyle.compare("TRIANGLEFILLED_INV", Qt::CaseInsensitive)==0) theStyle.m_Symbol = Line::TRIANGLE_INV_F;
        }
        else if (name().compare(QString("Color"), Qt::CaseInsensitive)==0) readColor(theStyle.m_Color);
        else skipCurrentElement();
    }
    return !hasError();
}


bool XflXmlReader::readColor(fl5Color &color)
{
    color.setRgba(0,0,0,255);
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if      (name().compare(QString("red"),   Qt::CaseInsensitive)==0)  color.setRed(readElementText().trimmed().toInt());
        else if (name().compare(QString("green"), Qt::CaseInsensitive)==0)  color.setGreen(readElementText().trimmed().toInt());
        else if (name().compare(QString("blue"),  Qt::CaseInsensitive)==0)  color.setBlue(readElementText().trimmed().toInt());
        else if (name().compare(QString("alpha"), Qt::CaseInsensitive)==0)  color.setAlpha(readElementText().trimmed().toInt());
        else skipCurrentElement();
    }
    return !hasError();
}


bool XflXmlReader::readPointMass(PointMass &pm, double massUnit, double lengthUnit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if      (name().compare(QString("tag"), Qt::CaseInsensitive)==0)  pm.setTag(readElementText().trimmed());
        else if (name().compare(QString("mass"), Qt::CaseInsensitive)==0) pm.setMass(readElementText().trimmed().toDouble()*massUnit);
        else if (name().compare(QString("coordinates"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                pm.setXPosition(coordList.at(0).toDouble()*lengthUnit);
                pm.setYPosition(coordList.at(1).toDouble()*lengthUnit);
                pm.setZPosition(coordList.at(2).toDouble()*lengthUnit);
            }
        }
        else skipCurrentElement();
    }
    return !hasError();
}


bool XflXmlReader::readNurbs(NURBSSurface &nurbs, QVector<int> &xpanels, double lengthUnit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if      (name().compare(QString("u_degree"),           Qt::CaseInsensitive)==0) nurbs.setuDegree(readElementText().toInt());
        else if (name().compare(QString("v_degree"),           Qt::CaseInsensitive)==0) nurbs.setvDegree(readElementText().toInt());
        else if (name().compare(QString("uAxis"),              Qt::CaseInsensitive)==0) nurbs.setUAxis(readElementText().toInt());
        else if (name().compare(QString("vAxis"),              Qt::CaseInsensitive)==0) nurbs.setVAxis(readElementText().toInt());
        else if (name().compare(QString("uEdgeWeight"),        Qt::CaseInsensitive)==0) nurbs.setuEdgeWeight(readElementText().toDouble());
        else if (name().compare(QString("vEdgeWeight"),        Qt::CaseInsensitive)==0) nurbs.setvEdgeWeight(readElementText().toDouble());
        else if (name().compare(QString("Bunch_amplitude"),    Qt::CaseInsensitive)==0) nurbs.setBunchAmplitude(readElementText().toDouble());
        else if (name().compare(QString("Bunch_distribution"), Qt::CaseInsensitive)==0) nurbs.setBunchDistribution(readElementText().toDouble());
        else if (name().compare(QString("Frame"),              Qt::CaseInsensitive)==0)
        {
            Frame &frame = nurbs.appendNewFrame();
            frame.clearCtrlPoints();
            xpanels.append(1);
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Angle"), Qt::CaseInsensitive)==0)
                {
                    frame.setAngle(readElementText().toDouble());
                }
                else if (name().compare(QString("x_panels"), Qt::CaseInsensitive)==0)
                {
                    xpanels.back() = readElementText().toInt();
                }
                else if (name().compare(QString("Position"), Qt::CaseInsensitive)==0)
                {
                    QStringList coordList = readElementText().simplified().split(",");
                    if(coordList.length()>=3)
                    {
                        double x = coordList.at(0).toDouble()*lengthUnit;
                        double y = coordList.at(1).toDouble()*lengthUnit;
                        double z = coordList.at(2).toDouble()*lengthUnit;
                        frame.setPosition(Vector3d(x,y,z));
                    }
                }
                else if (name().compare(QString("point"), Qt::CaseInsensitive)==0)
                {
                    Vector3d ctrlPt;
                    QStringList coordList = readElementText().simplified().split(",");
                    if(coordList.length()>=3)
                    {
                        ctrlPt.x = coordList.at(0).toDouble()*lengthUnit;
                        ctrlPt.y = coordList.at(1).toDouble()*lengthUnit;
                        ctrlPt.z = coordList.at(2).toDouble()*lengthUnit;
                        frame.appendPoint(ctrlPt);
                    }
                }
            }
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflXmlReader::readFluidData(double &viscosity, double &density)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Viscosity"), Qt::CaseInsensitive)==0)
        {
            viscosity = readElementText().toDouble();
        }
        else if (name().compare(QString("Density"), Qt::CaseInsensitive)==0)
        {
            density = readElementText().toDouble();
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


void XflXmlReader::readInertia(Inertia &inertia, double lengthunit, double massunit, double inertiaunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Mass"), Qt::CaseInsensitive)==0)
        {
            inertia.setStructuralMass(readElementText().toDouble()*massunit);
        }
        else if (name().compare(QString("CoG"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                double x = coordList.at(0).toDouble()*lengthunit;
                double z = coordList.at(2).toDouble()*lengthunit;
                inertia.setCoG_s(x,0.0,z);
            }
        }
        else if (name().compare(QString("CoG_Ixx"), Qt::CaseInsensitive)==0)
        {
            inertia.setIxx_s(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("CoG_Iyy"), Qt::CaseInsensitive)==0)
        {
            inertia.setIyy_s(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("CoG_Izz"), Qt::CaseInsensitive)==0)
        {
            inertia.setIzz_s(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("CoG_Izz"), Qt::CaseInsensitive)==0)
        {
            inertia.setIxz_s(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("Point_Mass"), Qt::CaseInsensitive)==0)
        {
            PointMass pm;
            readPointMass(pm, massunit, lengthunit);
            inertia.appendPointMass(pm);
        }
        else
            skipCurrentElement();
    }
}


bool XflXmlReader::readExtraDrag(QVector<ExtraDrag> &extra, double areaunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Drag"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.size()==3)
            {
                ExtraDrag xd;
                xd.setTag(coordList.at(0));
                xd.setArea(coordList.at(1).toDouble() * areaunit);
                xd.setCoef(coordList.at(2).toDouble());
                extra.push_back(xd);
            }
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflXmlReader::readUnits(double &lengthunit, double &massunit, double &velocityunit, double &areaunit, double &inertiaunit)
{
    // changed tags in v7.50, kept legacy ones active
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("length_unit_to_meter"),      Qt::CaseInsensitive)==0)
        {
            lengthunit = readElementText().toDouble();
        }
        else if (name().compare(QString("meter_to_length_unit"), Qt::CaseInsensitive)==0) // changed tag in v7.50
        {
            lengthunit = readElementText().toDouble();
        }
        else if (name().compare(QString("mass_unit_to_kg"),      Qt::CaseInsensitive)==0)
        {
            massunit = readElementText().toDouble();
        }
        else if (name().compare(QString("kg_to_mass_unit"),      Qt::CaseInsensitive)==0) // changed tag in v7.50
        {
            massunit = readElementText().toDouble();
        }
        else if (name().compare(QString("speed_unit_to_ms"),      Qt::CaseInsensitive)==0)
        {
            velocityunit = readElementText().toDouble();
        }
        else if (name().compare(QString("ms_to_speed_unit"),      Qt::CaseInsensitive)==0) // changed tag in v7.50
        {
            velocityunit = readElementText().toDouble();
        }
        else if (name().compare(QString("area_unit_to_m2"),       Qt::CaseInsensitive)==0)
        {
            areaunit = readElementText().toDouble();
        }
        else if (name().compare(QString("m2_to_area_unit"),       Qt::CaseInsensitive)==0) // changed tag in v7.50
        {
            areaunit = readElementText().toDouble();
        }
        else if (name().compare(QString("inertia_unit_to_kgm2"),  Qt::CaseInsensitive)==0)
        {
            inertiaunit = readElementText().toDouble();
        }
        else if (name().compare(QString("kgm2_to_inertia_unit"),  Qt::CaseInsensitive)==0) // changed tag in v7.50
        {
            inertiaunit = readElementText().toDouble();
        }
       else
            skipCurrentElement();
    }
    return !hasError();
}


bool XflXmlReader::readWakeData(Polar3d &polar3d)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("FlatPanelWake"),                    Qt::CaseInsensitive)==0)
        {
            polar3d.setVortonWake(!xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("NX"),                          Qt::CaseInsensitive)==0)
        {
            polar3d.setNXWakePanel4(readElementText().toInt());
        }
        else if (name().compare(QString("ProgressionFactor"),           Qt::CaseInsensitive)==0)
        {
            polar3d.setWakePanelFactor(readElementText().toDouble());
        }
        else if (name().compare(QString("LengthFactor"),                Qt::CaseInsensitive)==0)
        {
            polar3d.setTotalWakeLengthFactor(readElementText().toDouble());
        }
        else if (name().compare(QString("VPW_BufferWakeLength"),        Qt::CaseInsensitive)==0)
        {
            polar3d.setBufferWakeFactor(readElementText().toDouble());
        }
        else if (name().compare(QString("VPW_FirstStep"),               Qt::CaseInsensitive)==0)
        {
            polar3d.setVortonL0(readElementText().toDouble());
        }
/*        else if (name().compare(QString("VPW_GeomFactor"),              Qt::CaseInsensitive)==0)
        {
            polar3d.setVortonXFactor(readElementText().toDouble());
        }*/
        else if (name().compare(QString("VPW_MaxLength"),               Qt::CaseInsensitive)==0)
        {
            polar3d.setVPWMaxLength(readElementText().toDouble());
        }
        else if (name().compare(QString("Vorton_Core_Size"),            Qt::CaseInsensitive)==0)
        {
            polar3d.setVortonCoreSize(readElementText().toDouble());
        }
        else if (name().compare(QString("VPW_Iterations"),              Qt::CaseInsensitive)==0)
        {
            polar3d.setVPWIterations(readElementText().toInt());
        }
        else
             skipCurrentElement();
    }
    return !hasError();

}


bool XflXmlReader::readFuseXfl(FuseXfl *pFuseXfl, double lengthUnit, double massUnit)
{
    pFuseXfl->nurbs().clearFrames();
    pFuseXfl->clearXPanels();
    pFuseXfl->clearHPanels();

    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().toString().compare(QString("name"),Qt::CaseInsensitive) ==0)
        {
            pFuseXfl->setName(readElementText().trimmed().toStdString());
        }
        else if (name().toString().compare(QString("color"), Qt::CaseInsensitive)==0)
        {
            fl5Color clr;
            readColor(clr);
            pFuseXfl->setColor(clr);
        }
        else if (name().toString().compare(QString("description"), Qt::CaseInsensitive)==0)
        {
            pFuseXfl->setDescription(readElementText().trimmed().toStdString());
        }
        else if (name().compare(QString("AutoInertia"), Qt::CaseInsensitive)==0)
        {
            pFuseXfl->setAutoInertia(readElementText().trimmed().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("Inertia"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("volume_mass"), Qt::CaseInsensitive)==0)
                {
                    pFuseXfl->setStructuralMass(readElementText().toDouble());
                }
                else if (name().compare(QString("point_mass"), Qt::CaseInsensitive)==0)
                {
                    pFuseXfl->appendPointMass({});
                    readPointMass(pFuseXfl->inertia().lastPointMass(), massUnit, lengthUnit);
                }
                else
                    skipCurrentElement();
            }
        }
        else if (name().compare(QString("position"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                Vector3d position;
                position.x = coordList.at(0).toDouble()*lengthUnit;
                position.y = coordList.at(1).toDouble()*lengthUnit;
                position.z = coordList.at(2).toDouble()*lengthUnit;
                pFuseXfl->setPosition(position);
            }
        }
        else if (name().compare(QString("type"), Qt::CaseInsensitive)==0)
        {
            if(readElementText().trimmed().compare(QString("NURBS"), Qt::CaseInsensitive)==0) pFuseXfl->setFuseType(Fuse::NURBS);
            else                                                                              pFuseXfl->setFuseType(Fuse::FlatFace);
        }
        else if (name().compare(QString("x_panels"), Qt::CaseInsensitive)==0)
            pFuseXfl->setNxNurbsPanels(readElementText().toInt());
        else if (name().compare(QString("hoop_panels"), Qt::CaseInsensitive)==0)
            pFuseXfl->setNhNurbsPanels(readElementText().toInt());
        else if (name().compare(QString("Panel_Stripes"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().contains("stripe", Qt::CaseInsensitive))
                {
                    pFuseXfl->appendHPanel(readElementText().trimmed().toInt());
                }
            }
        }
        else if (name().compare(QString("NURBS"), Qt::CaseInsensitive)==0)
        {
            QVector<int> xPanels;
            readNurbs(pFuseXfl->nurbs(), xPanels, lengthUnit);
            pFuseXfl->setXPanels(xPanels);
        }
    }

    if(pFuseXfl->isSplineType())
    {
        pFuseXfl->resizeHPanels(pFuseXfl->sideLineCount());
        pFuseXfl->resizeXPanels(pFuseXfl->frameCount());

        pFuseXfl->setPanelPos();
    }
    else if(pFuseXfl->isFlatFaceType())
    {
        // clean up potential errors from redundant and missing fields.
        while(pFuseXfl->nxPanels()<pFuseXfl->frameCount())
        {
            pFuseXfl->appendXPanel(1);
        }
        while(pFuseXfl->nhPanels()<pFuseXfl->sideLineCount())
        {
            pFuseXfl->appendHPanel(1);
        }

        // clean potential excess
        pFuseXfl->resizeXPanels(pFuseXfl->frameCount());
        pFuseXfl->resizeHPanels(pFuseXfl->sideLineCount());
    }

    return !hasError();
}


bool XflXmlReader::readWing(WingXfl *pWing, Vector3d &WingLE, double &Rx, double &Ry, double lengthUnit, double massUnit, double inertiaUnit)
{
    pWing->setWingType(WingXfl::OtherWing);
    pWing->m_Section.clear();

    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("name"),                 Qt::CaseInsensitive)==0)
        {
            pWing->setName(readElementText().trimmed().toStdString());
        }
        else if (name().compare(QString("type"),           Qt::CaseInsensitive)==0)
        {
            pWing->setWingType(WingXfl::type(readElementText()));
            /** @todo improve */
            /**                        if(pWing->wingType()==WingXfl::Elevator)        pPlane->hasElevator() = true;
            else if(pWing->wingType()==WingXfl::SECONDWING) pPlane->hasSecondWing() = true;
            else if(pWing->wingType()==WingXfl::Fin)        pPlane->hasFin() = true;*/
        }
        else if (name().compare(QString("color"),           Qt::CaseInsensitive)==0)
        {
            fl5Color clr;
            readColor(clr);
            pWing->setColor(clr);
        }
        else if (name().compare(QString("description"),     Qt::CaseInsensitive)==0)
        {
            pWing->setDescription(readElementText().trimmed().toStdString());
        }
        else if (name().compare(QString("position"),        Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                WingLE.x = coordList.at(0).toDouble()*lengthUnit;
                WingLE.y = coordList.at(1).toDouble()*lengthUnit;
                WingLE.z = coordList.at(2).toDouble()*lengthUnit;
            }
        }
        else if (name().compare(QString("Tip_Strips"), Qt::CaseInsensitive)==0)
        {
            pWing->setNTipStrips(readElementText().trimmed().toInt());
        }
        else if (name().compare(QString("Rx_angle"), Qt::CaseInsensitive)==0)
        {
            Rx = readElementText().trimmed().toDouble();
        }
        else if (name().compare(QString("Ry_angle"),Qt::CaseInsensitive)==0)
        {
            Ry = readElementText().trimmed().toDouble();
        }
        else if (name().compare(QString("symmetric"), Qt::CaseInsensitive)==0)
        {
            pWing->setsymmetric(readElementText().trimmed().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("Two_Sided"), Qt::CaseInsensitive)==0)
        {
            pWing->setTwoSided(readElementText().trimmed().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("Closed_Inner_Side"), Qt::CaseInsensitive)==0)
        {
            pWing->setClosedInnerSide(readElementText().trimmed().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("AutoInertia"), Qt::CaseInsensitive)==0)
        {
            pWing->setAutoInertia(readElementText().trimmed().compare(QString("true"), Qt::CaseInsensitive)==0);
        }
        else if (name().compare(QString("Inertia"), Qt::CaseInsensitive)==0)
        {
            readInertia(pWing->inertia(), lengthUnit, massUnit, inertiaUnit);
        }
        else if (name().compare(QString("Sections"), Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Section"), Qt::CaseInsensitive)==0)
                {
                    pWing->m_Section.push_back({});
                    WingSection*pWingSec = &pWing->m_Section.back();
                    while(!atEnd() && !hasError() && readNextStartElement() )
                    {
                        if (name().compare(QString("x_number_of_panels"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_NXPanels = readElementText().trimmed().toInt();
                        }
                        else if (name().compare(QString("y_number_of_panels"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_NYPanels = readElementText().trimmed().toInt();
                        }
                        else if (name().compare(QString("x_panel_distribution"), Qt::CaseInsensitive)==0)
                        {
                            QString strPanelDist = readElementText().trimmed();
                            pWingSec->m_XPanelDist = xfl::distributionType(strPanelDist);
                        }
                        else if (name().compare(QString("y_panel_distribution"), Qt::CaseInsensitive)==0)
                        {
                            QString strPanelDist = readElementText().trimmed();
                            pWingSec->m_YPanelDist = xfl::distributionType(strPanelDist);
                        }
                        else if (name().compare(QString("Chord"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_Chord = readElementText().trimmed().toDouble()*lengthUnit;
                        }
                        else if (name().compare(QString("y_position"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_YPosition = readElementText().toDouble()*lengthUnit;
                        }
                        else if (name().compare(QString("xOffset"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_Offset = readElementText().trimmed().toDouble()*lengthUnit;
                        }
                        else if (name().compare(QString("Dihedral"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_Dihedral = readElementText().trimmed().toDouble();
                        }
                        else if (name().compare(QString("Twist"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_Twist = readElementText().trimmed().toDouble();
                        }
                        else if (name().compare(QString("Left_Side_FoilName"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_LeftFoilName = readElementText().trimmed();
                        }
                        else if (name().compare(QString("Left_Side_Foil_File"), Qt::CaseInsensitive)==0)
                        {
                            QString filename = m_Path + QDir::separator() + readElementText().trimmed();

                            QFileInfo fi(filename);
                            if (fi.exists() && fi.isReadable())
                            {
                                Foil *pFoil = new Foil();
                                objects::readFoilFile(filename, pFoil);
                                if(pFoil)
                                {
                                    pWingSec->m_LeftFoilName = QString::fromStdString(pFoil->name());
                                    Objects2d::insertThisFoil(pFoil);
                                }
                                else        pWingSec->m_LeftFoilName =readElementText().trimmed();

                            }
                        }
                        else if (name().compare(QString("Right_Side_FoilName"), Qt::CaseInsensitive)==0)
                        {
                            pWingSec->m_RightFoilName = readElementText().trimmed();
                        }
                        else if (name().compare(QString("Right_Side_Foil_File"), Qt::CaseInsensitive)==0)
                        {
                            QString filename = m_Path + QDir::separator() + readElementText().trimmed();

                            QFileInfo fi(filename);
                            if (fi.exists() && fi.isReadable())
                            {
                                Foil *pFoil = new Foil();
                                objects::readFoilFile(filename, pFoil);
                                if(pFoil)
                                {
                                    pWingSec->m_RightFoilName = QString::fromStdString(pFoil->name());
                                    Objects2d::insertThisFoil(pFoil);
                                }
                                else        pWingSec->m_RightFoilName =readElementText().trimmed();

                            }
                        }
                        else
                            skipCurrentElement();
                    }
                }
                else
                    skipCurrentElement();
            }
        }
        else
        {
            skipCurrentElement();
        }
    }

    return !hasError();
}

