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


#include <xmlbtpolarreader.h>

#include <aeroforces.h>
#include <boatpolar.h>
#include <objects_global.h>
#include <units.h>
#include <xml_globals.h>


XmlBtPolarReader::XmlBtPolarReader(QFile &file) : XflXmlReader(file)
{
    m_pBtPolar = nullptr;
}


bool XmlBtPolarReader::readXMLPolarFile()
{
    double lengthunit   = 1.0;
    double areaunit     = 1.0;
    double massunit     = 1.0;
    double velocityunit = 1.0;
    double inertiaunit  = 1.0;

    if(readNextStartElement())
    {
        if (name().compare(QString("xflboatpolar"), Qt::CaseInsensitive)==0)
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

            while(!atEnd() && !hasError() && readNextStartElement())
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    readUnits(lengthunit, massunit, velocityunit, inertiaunit, areaunit);
                }
                else if (name().toString().compare(QString("Polar"), Qt::CaseInsensitive)==0)
                {
                    m_pBtPolar = new BoatPolar;
                    readBtPolar(m_pBtPolar, lengthunit, areaunit, velocityunit);
                }
                else
                    skipCurrentElement();
            }
        }
        else
        {
            raiseError("The file is not an xfl boat polar version 1.0 file.");
            return true;
        }
    }

    if(hasError())
    {
        if(m_pBtPolar) delete m_pBtPolar;
        m_pBtPolar= nullptr;
    }
    return !hasError();
}


bool XmlBtPolarReader::readBtPolar(BoatPolar *pBtPolar, double lengthunit, double areaunit, double velocityunit)
{
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("polar_name"), Qt::CaseInsensitive) ==0)
        {
            pBtPolar->setName(readElementText().trimmed().toStdString());
        }
        else if (name().toString().compare(QString("boat_name"), Qt::CaseInsensitive) ==0)
        {
            pBtPolar->setBoatName(readElementText().toStdString());
        }
        else if(name().toString().compare(QString("The_Style"), Qt::CaseInsensitive)==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pBtPolar->setTheStyle(ls);
        }
        else if (name().toString().compare(QString("method"), Qt::CaseInsensitive)==0)
        {
            pBtPolar->setAnalysisMethod(xml::analysisMethod(readElementText()));
        }
        else if (name().compare(QString("Reference_Dimensions"), Qt::CaseInsensitive)==0)
        {
            readReferenceDimensions(lengthunit, areaunit);
        }
        else if (name().compare(QString("Ground_Effect"), Qt::CaseInsensitive)==0)
        {
            pBtPolar->setGroundEffect(xfl::stringToBool(readElementText()));
        }
        else if (name().compare(QString("Wake"), Qt::CaseInsensitive)==0)
        {
            readWakeData(*pBtPolar);
        }
        else if (name().compare(QString("Fluid"), Qt::CaseInsensitive)==0)
        {
            readFluidData(pBtPolar->m_Viscosity, pBtPolar->m_Density);
        }
        else if (name().compare(QString("ExtraDrag"), Qt::CaseInsensitive)==0)
        {
            readExtraDrag(m_pBtPolar->m_ExtraDrag, areaunit);
        }
        else if (name().compare(QString("Wind_gradient"), Qt::CaseInsensitive)==0)
        {
            readWindSpline(m_pBtPolar->windSpline());
        }
        else if (name().compare(QString("CoG"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                pBtPolar->m_CoG.x = coordList.at(0).toDouble()*lengthunit;
                pBtPolar->m_CoG.y = coordList.at(1).toDouble()*lengthunit;
                pBtPolar->m_CoG.z = coordList.at(2).toDouble()*lengthunit;
            }
        }
        else if(name().compare(QString("Boat_Speed"), Qt::CaseInsensitive)==0)
        {
            QStringList datalist = readElementText().simplified().split(",");
            if(datalist.size()==2)
            {
                pBtPolar->setVBtMin(datalist.at(0).toDouble() * velocityunit);
                pBtPolar->setVBtMax(datalist.at(1).toDouble() * velocityunit);
            }
        }
        else if(name().compare(QString("TWS"), Qt::CaseInsensitive)==0)
        {
            QStringList datalist = readElementText().simplified().split(",");
            if(datalist.size()==2)
            {
                pBtPolar->setQInfMin(datalist.at(0).toDouble() * velocityunit);
                pBtPolar->setQInfMax(datalist.at(1).toDouble() * velocityunit);
            }
        }
        else if(name().compare(QString("TWA"), Qt::CaseInsensitive)==0)
        {
            QStringList datalist = readElementText().simplified().split(",");
            if(datalist.size()==2)
            {
                pBtPolar->setTwaMin(datalist.at(0).toDouble());
                pBtPolar->setTwaMax(datalist.at(1).toDouble());
            }
        }
        else if(name().compare(QString("Phi"), Qt::CaseInsensitive)==0)
        {
            QStringList datalist = readElementText().simplified().split(",");
            if(datalist.size()==2)
            {
                pBtPolar->setPhiMin(datalist.at(0).toDouble());
                pBtPolar->setPhiMax(datalist.at(1).toDouble());
            }
        }
        else if(name().compare(QString("Ry"), Qt::CaseInsensitive)==0)
        {
            QStringList datalist = readElementText().simplified().split(",");
            if(datalist.size()==2)
            {
                pBtPolar->setRyMin(datalist.at(0).toDouble());
                pBtPolar->setRyMax(datalist.at(1).toDouble());
            }
        }
        else if(name().compare(QString("SailAngles"), Qt::CaseInsensitive)==0)
        {
            readSailAngles();
        }
        else
            skipCurrentElement();
    }
    return hasError();
}



bool XmlBtPolarReader::readWindSpline(BSpline &spline)
{
    spline.clearControlPoints();
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if      (name().compare(QString("degree"), Qt::CaseInsensitive)==0) spline.setDegree(readElementText().toInt());
        else if (name().compare(QString("point"),  Qt::CaseInsensitive)==0)
        {
            Vector2d ctrlPt;
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=2)
            {
                ctrlPt.x = coordList.at(0).toDouble();
                ctrlPt.y = coordList.at(1).toDouble(); // meters only
                spline.appendControlPoint(ctrlPt);
            }
        }
        else
            skipCurrentElement();
    }
    spline.updateSpline();
    spline.makeCurve();
    return !hasError();

}


bool XmlBtPolarReader::readSailAngles()
{
    bool bOk1, bOk2;
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().left(4).compare(QString("Sail"), Qt::CaseInsensitive)==0)
        {
            QStringList datalist = readElementText().simplified().split(",");
            if(datalist.size()==2)
            {
                double amin = datalist.at(0).toDouble(&bOk1);
                double amax = datalist.at(1).toDouble(&bOk2);
                if(bOk1 && bOk2)
                {
                    m_pBtPolar->m_SailAngleMin.push_back(amin);
                    m_pBtPolar->m_SailAngleMax.push_back(amax);
                }
            }
        }
        else
            skipCurrentElement();
    }
    return hasError();
}



void XmlBtPolarReader::readReferenceDimensions(double lengthunit, double areaunit)
{
    while(!atEnd() && !hasError() && readNextStartElement() )
    {
        if (name().compare(QString("Reference_Dimensions"), Qt::CaseInsensitive)==0)
        {
            m_pBtPolar->setReferenceDim(xml::referenceDimension(readElementText()));
        }
        else if (name().compare(QString("Reference_Area"), Qt::CaseInsensitive)==0)
        {
            m_pBtPolar->setReferenceArea(readElementText().toDouble()*areaunit);
        }
        else if (name().compare(QString("Reference_Chord_Length"), Qt::CaseInsensitive)==0)
        {
            m_pBtPolar->setReferenceChordLength(readElementText().toDouble()*lengthunit);
        }
        else
            skipCurrentElement();
    }
}

