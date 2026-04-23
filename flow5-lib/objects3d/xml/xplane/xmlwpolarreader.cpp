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

#include <QStringList>
#include <string>

#define _MATH_DEFINES_DEFINED


#include <api/xmlwpolarreader.h>

#include <api/objects_global.h>
#include <api/utils.h>
#include <api/wpolar.h>

XmlWPolarReader::XmlWPolarReader(QFile &file) : XflXmlReader(file)
{
    m_pWPolar = nullptr;
}


bool XmlWPolarReader::readXMLPolarFile()
{
    double lengthunit   = 1.0;
    double areaunit     = 1.0;
    double massunit     = 1.0;
    double velocityunit = 1.0;
    double inertiaunit  = 1.0;

    if (readNextStartElement())
    {
        if (name().toString() == "xflPlanePolar")
        {
            // get version
            QString strange = attributes().value("version").toString();
            if(strange.length())
            {
                int pos = strange.indexOf('.');
                if(pos<0)
                {
                    raiseError("Unrecognized file version: "+strange);
                    return true;
                }
                QString strMajor = strange.left(pos);
                QString strMinor = strange.right(pos);
                m_VMajor = strMajor.toInt();
                m_VMinor = strMinor.toInt();
            }

            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    readUnits(lengthunit, massunit, velocityunit, areaunit, inertiaunit);
                }
                else if (name().toString().compare(QString("Polar"), Qt::CaseInsensitive)==0)
                {
                    m_pWPolar = new WPolar;
                    readWPolar(m_pWPolar, lengthunit, areaunit, massunit, velocityunit, inertiaunit);
                }
                else
                    skipCurrentElement();
            }
        }
        else
        {
            raiseError("      The file is not an xfl plane polar version 1.0 file.");
            return true;
        }
    }

    if(!m_pWPolar) return false;

    if(hasError())
    {
        if(m_pWPolar)
        {
            delete m_pWPolar;
            m_pWPolar = nullptr;
        }
    }

    return !hasError();
}


void XmlWPolarReader::readWPolar(WPolar *pWPolar, double lengthunit, double areaunit, double massunit, double velocityunit, double inertiaunit)
{
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("polar_name"), Qt::CaseInsensitive) ==0)
        {
            pWPolar->setName(readElementText().trimmed());
        }
        else if (name().toString().compare(QString("the_style"), Qt::CaseInsensitive) ==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pWPolar->setTheStyle(ls);
        }
        else if (name().toString().compare(QString("plane_name"), Qt::CaseInsensitive) ==0)
        {
            pWPolar->setPlaneName(readElementText().toStdString());
        }
        else if (name().toString().compare(QString("type"), Qt::CaseInsensitive) ==0)
        {
            pWPolar->setType(objects::polarType(readElementText()));
        }
        else if (name().toString().compare(QString("method"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setAnalysisMethod(Polar3d::analysisMethod(readElementText()));
        }
        else if (name().compare(QString("Include_Fuse_Moments"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setIncludeFuseMi(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Thin_Surfaces"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setThinSurfaces(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Ground_Effect"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setGroundEffect(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Free_Surface"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setFreeSurfaceEffect(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Ground_Height"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setGroundHeight(readElementText().toDouble()*lengthunit); /** @todo min and max */
        }
        else if (name().compare(QString("Fluid"), Qt::CaseInsensitive)==0)
        {
            readFluidData(pWPolar->m_Viscosity, pWPolar->m_Density);
        }
        else if (name().compare(QString("Fixed_Velocity"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setVelocity(readElementText().toDouble()*velocityunit);
        }
        else if (name().compare(QString("Fixed_AOA"), Qt::CaseInsensitive)==0)
        {
            pWPolar->setAlphaSpec(readElementText().toDouble());
        }
        else if (name().compare(QString("Wake"), Qt::CaseInsensitive)==0)
        {
            readWakeData(*pWPolar);
        }
        else if (name().compare(QString("Reference_Dimensions"), Qt::CaseInsensitive)==0)
        {
            readReferenceDimensions(lengthunit, areaunit);
        }
        else if (name().compare(QString("Viscous_Analysis"), Qt::CaseInsensitive)==0)
        {
            readViscosity();
        }
        else if (name().compare(QString("ExtraDrag"), Qt::CaseInsensitive)==0)
        {
            readExtraDrag(m_pWPolar->m_ExtraDrag, areaunit);
        }
        else if (name().compare(QString("Use_plane_inertia"), Qt::CaseInsensitive)==0)
        {
            bool bInertia = xfl::stringToBool(readElementText());
            pWPolar->setAutoInertia(bInertia);
        }
        else if (name().compare(QString("Inertia"), Qt::CaseInsensitive)==0)
        {
            readWPolarInertia(pWPolar, lengthunit, massunit, inertiaunit);
        }
        else if (name().compare(QString("Inertia_gains"), Qt::CaseInsensitive)==0)
        {
//            readInertiaGains(lengthunit, massunit, inertiaunit); // deprecated in beta 18
        }
        else if (name().compare(QString("Surface_angles"), Qt::CaseInsensitive)==0)
        {
//            readAngleCoeffs(); // deprecated in v713
        }
        else if (name().compare(QString("Flap_settings"), Qt::CaseInsensitive)==0)
        {
            readFlapSettings();
        }
        else if (name().compare(QString("AVL_controls"), Qt::CaseInsensitive)==0)
        {
            readAVLControls();
        }
        else if (name().compare(QString("Operating_range"), Qt::CaseInsensitive)==0)
        {
            readOperatingRange(velocityunit);
        }
        else if (name().compare(QString("Inertia_range"), Qt::CaseInsensitive)==0)
        {
            readInertiaRange(lengthunit, massunit);
        }
        else if (name().compare(QString("Angle_range"), Qt::CaseInsensitive)==0)
        {
            readAngleRange();
        }
        else if (name().compare(QString("Fuselage_Drag"), Qt::CaseInsensitive)==0)
        {
            readFuselageDrag();
        }
        else
            skipCurrentElement();
    }
}


void XmlWPolarReader::readReferenceDimensions(double lengthunit, double areaunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Reference_Dimensions"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setReferenceDim(Polar3d::referenceDimension(readElementText()));
        }
        else if (name().compare(QString("Reference_Area"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setReferenceArea(readElementText().toDouble()*areaunit);
        }
        else if (name().compare(QString("Reference_Span_Length"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setReferenceSpanLength(readElementText().toDouble()*lengthunit);
        }
        else if (name().compare(QString("Reference_Chord_Length"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setReferenceChordLength(readElementText().toDouble()*lengthunit);
        }
        else if (name().compare(QString("Include_Other_Wing_Area"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setIncludeOtherWingAreas(xfl::stringToBool(readElementText()));
        }
        else
            skipCurrentElement();
    }
}


void XmlWPolarReader::readFlapSettings()
{
    m_pWPolar->clearFlapCtrls();
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        QString tag = name().toString();
        if(tag.contains("wing", Qt::CaseInsensitive))
        {
            AngleControl &ac = m_pWPolar->addFlapCtrl();
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                QString strange = name().toString();
                if(strange.contains("flap", Qt::CaseInsensitive))
                {
                    QString strange(readElementText().simplified());
                    double theta = strange.toDouble();
                    ac.addValue(theta);
                }
                else
                    skipCurrentElement();
            }
        }
        else
            skipCurrentElement();
    }
}


void XmlWPolarReader::readAVLControls()
{
    m_pWPolar->clearAVLCtrls();
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if(name().toString().compare("control", Qt::CaseInsensitive)==0)
        {
            AngleControl avlc;
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if(name().toString().compare("name", Qt::CaseInsensitive)==0)
                {
                    avlc.setName(readElementText());
                }
                else if(name().toString().compare("gains", Qt::CaseInsensitive)==0)
                {
                    QString strange(readElementText().simplified());
                    QStringList fields = strange.split(" ");
                    avlc.resizeValues(fields.count());
                    for(int ic=0; ic<fields.count(); ic++)
                    {
                        bool bOK = false;
                        double gain = fields.at(ic).toDouble(&bOK);
                        if(bOK) avlc.setValue(ic, gain);
                    }
                }
            }
            m_pWPolar->addAVLControl(avlc);
        }
        else
            skipCurrentElement();
    }
}


void XmlWPolarReader::readInertiaRange(double lengthunit, double massunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        double cmin=0.0, cmax=0.0;
        std::string ctrlname;

        if(name().toString().compare("Mass", Qt::CaseInsensitive)==0)
        {
            ctrlname = "Mass";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble()*massunit;
                cmax = ctrllist.at(1).toDouble()*massunit;
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble()*massunit;
            }
            m_pWPolar->m_InertiaRange[0] = CtrlRange(ctrlname, cmin, cmax);
        }
        else if(name().toString().compare("CoG_x", Qt::CaseInsensitive)==0)
        {
            ctrlname = "CoG_x";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble()*lengthunit;
                cmax = ctrllist.at(1).toDouble()*lengthunit;
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble()*lengthunit;
            }
            m_pWPolar->m_InertiaRange[1] = CtrlRange(ctrlname, cmin, cmax);
        }
        else if(name().toString().compare("CoG_z", Qt::CaseInsensitive)==0)
        {
            ctrlname = "CoG_z";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble()*lengthunit;
                cmax = ctrllist.at(1).toDouble()*lengthunit;
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble()*lengthunit;
            }
            m_pWPolar->m_InertiaRange[2] = CtrlRange(ctrlname, cmin, cmax);
        }
        else
            skipCurrentElement();
    }

}


void XmlWPolarReader::readAngleRange()
{
    m_pWPolar->clearAngleRangeList();
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if(name().left(5).toString().compare("wing_", Qt::CaseInsensitive)==0)
        {
            int index = name().right(1).toInt();
            m_pWPolar->m_AngleRange.push_back({});
            Q_ASSERT(index==m_pWPolar->m_AngleRange.size()-1);
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                double cmin=0.0, cmax=0.0;
                std::string ctrlname;

                if(name().left(6).toString().compare("Range_", Qt::CaseInsensitive)==0)
                {
                    m_pWPolar->m_AngleRange[index].push_back({});
                    ctrlname = name().toString().toStdString();
                    QStringList ctrllist = readElementText().simplified().split(" ");

                    if(ctrllist.length()>=2)
                    {
                        cmin = ctrllist.at(0).toDouble();
                        cmax = ctrllist.at(1).toDouble();
                    }
                    else if(ctrllist.length()==1)
                    {
                        cmin = ctrllist.at(0).toDouble();
                    }
                    m_pWPolar->m_AngleRange[index].back() = CtrlRange(ctrlname, cmin, cmax);
                }
                else
                    skipCurrentElement();
            }
        }
    }

}


void XmlWPolarReader::readOperatingRange(double velocityunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        double cmin=0.0, cmax=0.0;
        std::string ctrlname;

        if(name().toString().compare("Adjusted_velocity", Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setAdjustedVelocity(xfl::stringToBool(readElementText()));
        }
        else if (name().toString().compare("Velocity", Qt::CaseInsensitive)==0)
        {
            ctrlname = "Velocity";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble()*velocityunit;
                cmax = ctrllist.at(1).toDouble()*velocityunit;
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble()*velocityunit;
            }
            m_pWPolar->m_OperatingRange[0] = CtrlRange(ctrlname, cmin, cmax);
        }
        else if(name().toString().compare("alpha", Qt::CaseInsensitive)==0)
        {
            ctrlname = "Alpha";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble();
                cmax = ctrllist.at(1).toDouble();
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble();
            }
            m_pWPolar->m_OperatingRange[1] = CtrlRange(ctrlname, cmin, cmax);
        }
        else if(name().toString().compare("Beta", Qt::CaseInsensitive)==0)
        {
            ctrlname = "Beta";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble();
                cmax = ctrllist.at(1).toDouble();
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble();
            }
            m_pWPolar->m_OperatingRange[2] = CtrlRange(ctrlname, cmin, cmax);
        }
        else if(name().toString().compare("Phi", Qt::CaseInsensitive)==0)
        {
            ctrlname = "Phi";
            QStringList ctrllist = readElementText().simplified().split(" ");
            if(ctrllist.length()>=2)
            {
                cmin = ctrllist.at(0).toDouble();
                cmax = ctrllist.at(1).toDouble();
            }
            else if(ctrllist.length()==1)
            {
                cmin = ctrllist.at(0).toDouble();
            }
        }
        else
            skipCurrentElement();
    }
}


void XmlWPolarReader::readViscosity()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Is_Viscous_Analysis"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setViscous(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("From_CL"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setViscFromCl(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("XFoil_OnTheFly"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setViscOnTheFly(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("NCrit"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setNCrit(readElementText().toDouble());
        }
        else if (name().compare(QString("XTrTop"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setXTrTop(readElementText().toDouble());
        }
        else if (name().compare(QString("XTrBot"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setXTrBot(readElementText().toDouble());
        }
        else
            skipCurrentElement();
    }
}


void XmlWPolarReader::readFuselageDrag()
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Friction_Drag"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setIncludeFuseDrag(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Friction_Drag_Method"), Qt::CaseInsensitive)==0)
        {
            QString strange = readElementText();
            int index = strange.indexOf("Karman", Qt::CaseInsensitive);
            if (index>=0)
            {
                m_pWPolar->setFuseDragMethod(WPolar::KARMANSCHOENHERR);
            }
            else
            {
                int index = strange.indexOf("Prandtl", Qt::CaseInsensitive);
                if (index>=0)
                {
                    m_pWPolar->setFuseDragMethod(WPolar::PRANDTLSCHLICHTING);
                }
            }
        }
        else
            skipCurrentElement();
    }
}


/** @todo CoG or body axis */
bool XmlWPolarReader::readWPolarInertia(WPolar *pWPolar, double lengthunit, double massunit, double inertiaunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Mass"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setMass(readElementText().toDouble()*massunit);
        }
        else if (name().compare(QString("CoG"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                pWPolar->m_CoG.x = coordList.at(0).toDouble()*lengthunit;
                pWPolar->m_CoG.y = coordList.at(1).toDouble()*lengthunit;
                pWPolar->m_CoG.z = coordList.at(2).toDouble()*lengthunit;
            }
        }
        else if (name().compare(QString("CoG_Ixx"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setIxx(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("CoG_Iyy"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setIyy(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("CoG_Izz"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setIzz(readElementText().toDouble()*inertiaunit);
        }
        else if (name().compare(QString("CoG_Ixz"), Qt::CaseInsensitive)==0)
        {
            m_pWPolar->setIxz(readElementText().toDouble()*inertiaunit);
        }
        else
            skipCurrentElement();
    }
    return hasError();
}





