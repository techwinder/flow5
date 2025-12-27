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



#include <api/xmlfusereader.h>

#include <api/frame.h>
#include <api/fusenurbs.h>
#include <api/fusexfl.h>
#include <api/planexfl.h>

XmlFuseReader::XmlFuseReader(QFile &file) : XflXmlReader(file)
{

}


bool XmlFuseReader::readXMLFuseFile()
{
    double lengthunit = 1.0;
    double massunit = 1.0;

    if (readNextStartElement())
    {
        if (name().toString().compare("xflfuse", Qt::CaseInsensitive)==0 && attributes().value("version").toString() == "1.0")
        {
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().toString().compare(QString("units"), Qt::CaseInsensitive)==0)
                {
                    while(!atEnd() && !hasError() && readNextStartElement() )
                    {
                        if (name().compare(QString("length_unit_to_meter"),      Qt::CaseInsensitive)==0)
                        {
                            lengthunit = readElementText().trimmed().toDouble();
                        }
                        else if (name().compare(QString("mass_unit_to_kg"),      Qt::CaseInsensitive)==0)
                        {
                            massunit = readElementText().trimmed().toDouble();
                        }
                        else
                            skipCurrentElement();
                    }
                }
                else if (name().toString().compare(QString("body"), Qt::CaseInsensitive)==0)
                {
                    m_pFuseXfl = new FuseNurbs;
                    readFuseXfl(m_pFuseXfl, lengthunit, massunit);
                }
            }
        }
        else
            raiseError("The file is not an xml fuse v.1.0 definition file.");
    }

    if(!m_pFuseXfl) return false;
    return(!hasError());
}

