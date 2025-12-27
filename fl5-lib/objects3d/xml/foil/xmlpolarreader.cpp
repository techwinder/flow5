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



#include <api/xmlpolarreader.h>

#include <api/polar.h>
#include <api/objects_global.h>


XmlPolarReader::XmlPolarReader(QFile &file, Polar *pPolar) : XflXmlReader(file)
{
    m_pPolar = pPolar;
}


bool XmlPolarReader::readXMLPolarFile()
{
    if (readNextStartElement())
    {
        if (name().toString().compare("Foil_Polar", Qt::CaseInsensitive)==0 && attributes().value("version").toString().compare("1.0", Qt::CaseInsensitive)==0)
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("Polar"), Qt::CaseInsensitive)==0)
                {
                    readPolar(m_pPolar);
                }
                else
                    skipCurrentElement();
            }
        }
        else
        {
            raiseError("      The file is not an xfl polar version 1.0 file.");
            return true;
        }
    }
    return(hasError());
}


bool XmlPolarReader::readPolar(Polar *pPolar)
{
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("polar_name"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setName(readElementText());
        }
        else if (name().toString().compare(QString("foil_name"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setFoilName(readElementText().toStdString());
        }
        else if (name().toString().compare(QString("type"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setType(objects::polarType(readElementText()));
        }
        else if (name().toString().compare(QString("method"),Qt::CaseInsensitive) ==0)
        {
            QString method = readElementText();
            if     (method.compare("XFoil", Qt::CaseInsensitive)==0)        pPolar->setBLMethod(BL::XFOIL);
            else pPolar->setBLMethod(BL::XFOIL);
        }
        else if (name().compare(QString("Fixed_Reynolds"), Qt::CaseInsensitive)==0)
        {
            pPolar->setReynolds(readElementText().toDouble());
        }
        else if (name().compare(QString("Fixed_AOA"), Qt::CaseInsensitive)==0)
        {
            pPolar->setAoaSpec(readElementText().toDouble());
        }
        else if (name().toString().compare(QString("Forced_Top_Transition"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setXTripTop(readElementText().toDouble());
        }
        else if (name().toString().compare(QString("Forced_Bottom_Transition"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setXTripBot(readElementText().toDouble());
        }
        else if (name().toString().compare(QString("Reynolds_Type"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setReType(readElementText().toInt());
        }
        else if (name().toString().compare(QString("Mach_Type"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setMaType(readElementText().toInt());
        }
        else if (name().toString().compare(QString("NCrit"),Qt::CaseInsensitive) ==0)
        {
            pPolar->setNCrit(readElementText().toDouble());
        }
        else
            skipCurrentElement();
    }
    return !hasError();
}

