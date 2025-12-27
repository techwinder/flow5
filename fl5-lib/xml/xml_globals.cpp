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



#include <xml_globals.h>




xfl::enumAnalysisMethod xml::analysisMethod(const QString &strAnalysisMethod)
{
    QString strange = strAnalysisMethod.trimmed();
    if     (strange.compare("LLT",        Qt::CaseInsensitive)==0) return xfl::LLT;
    else if(strange.compare("VLM1",       Qt::CaseInsensitive)==0) return xfl::VLM1;
    else if(strange.compare("VLM2",       Qt::CaseInsensitive)==0) return xfl::VLM2;
    else if(strange.compare("QUADS",      Qt::CaseInsensitive)==0) return xfl::QUADS;
    else if(strange.compare("TRIUNIFORM", Qt::CaseInsensitive)==0) return xfl::TRIUNIFORM;
    else if(strange.compare("TRILINEAR",  Qt::CaseInsensitive)==0) return xfl::TRILINEAR;

    return xfl::VLM2;
}


QString xml::analysisMethod(xfl::enumAnalysisMethod analysisMethod)
{
    switch(analysisMethod)
    {
        case xfl::LLT:        return "LLT";
        case xfl::VLM1:       return "VLM1";
        case xfl::VLM2:       return "VLM2";
        case xfl::QUADS:      return "QUADS";
        case xfl::TRIUNIFORM: return "TRIUNIFORM";
        case xfl::TRILINEAR:  return "TRILINEAR";
        case xfl::NOMETHOD:   return "NOMETHOD";
    }
    return QString();
}


xfl::enumPolarType xml::polarType(QString const &strPolarType)
{
    QString strange = strPolarType.trimmed();
    if     (strange.compare("FIXEDSPEEDPOLAR", Qt::CaseInsensitive)==0) return xfl::T1POLAR;
    else if(strange.compare("FIXEDLIFTPOLAR",  Qt::CaseInsensitive)==0) return xfl::T2POLAR;
    else if(strange.compare("GLIDEPOLAR",      Qt::CaseInsensitive)==0) return xfl::T3POLAR;
    else if(strange.compare("FIXEDAOAPOLAR",   Qt::CaseInsensitive)==0) return xfl::T4POLAR;
    else if(strange.compare("BETAPOLAR",       Qt::CaseInsensitive)==0) return xfl::T5POLAR;
    else if(strange.compare("CONTROLPOLAR",    Qt::CaseInsensitive)==0) return xfl::T6POLAR;
    else if(strange.compare("STABILITYPOLAR",  Qt::CaseInsensitive)==0) return xfl::T7POLAR;
    else if(strange.compare("T8POLAR",         Qt::CaseInsensitive)==0) return xfl::T8POLAR;
    else if(strange.compare("BOATPOLAR",       Qt::CaseInsensitive)==0) return xfl::BOATPOLAR;
    else return xfl::T1POLAR;
}


QString xml::polarType(xfl::enumPolarType polarType)
{
    switch(polarType)
    {
        case xfl::T1POLAR:   return "FIXEDSPEEDPOLAR";
        case xfl::T2POLAR:   return "FIXEDLIFTPOLAR";
        case xfl::T3POLAR:   return "GLIDEPOLAR";
        case xfl::T4POLAR:   return "FIXEDAOAPOLAR";
        case xfl::T5POLAR:   return "BETAPOLAR";
        case xfl::T6POLAR:   return "CONTROLPOLAR";
        case xfl::T7POLAR:   return "STABILITYPOLAR";
        case xfl::T8POLAR:   return "T8POLAR";
        case xfl::BOATPOLAR: return "BOATPOLAR";
        default: return QString();
    }
}


QString xml::referenceDimension(xfl::enumRefDimension refDimension)
{
    switch(refDimension)
    {
        case xfl::PLANFORM:  return "PLANFORM";
        case xfl::PROJECTED: return "PROJECTED";
        case xfl::CUSTOM:    return "CUSTOM";
        case xfl::AUTODIMS:  return "AUTO"; // Sails only
    }
    return QString();
}


xfl::enumRefDimension xml::referenceDimension(QString const &strRefDimension)
{
    QString strRef = strRefDimension.trimmed();
    if     (strRef.compare("PLANFORM",  Qt::CaseInsensitive)==0) return xfl::PLANFORM;
    else if(strRef.compare("PROJECTED", Qt::CaseInsensitive)==0) return xfl::PROJECTED;
    else if(strRef.compare("CUSTOM",    Qt::CaseInsensitive)==0) return xfl::CUSTOM;
    else if(strRef.compare("AUTO",      Qt::CaseInsensitive)==0) return xfl::AUTODIMS; // Sails only
    else return xfl::PLANFORM;
}


xfl::enumBC xml::boundaryCondition(const QString &strBC)
{
    QString strange = strBC.trimmed();
    if   (strange.compare("DIRICHLET", Qt::CaseInsensitive)==0) return xfl::DIRICHLET;
    else                                                        return xfl::NEUMANN;
}


QString xml::boundaryCondition(xfl::enumBC boundaryCondition)
{
    switch(boundaryCondition)
    {
        case xfl::DIRICHLET: return "DIRICHLET";
        case xfl::NEUMANN:   return "NEUMANN";
    }
    return QString();
}


xfl::enumType xml::wingType(QString const &strWingType)
{
    if     (strWingType.compare("MAINWING",   Qt::CaseInsensitive)==0) return xfl::Main;
    else if(strWingType.compare("ELEVATOR",   Qt::CaseInsensitive)==0) return xfl::Elevator;
    else if(strWingType.compare("FIN",        Qt::CaseInsensitive)==0) return xfl::Fin;
    else                                                               return xfl::OtherWing;
}


QString xml::wingType(xfl::enumType wingType)
{
    switch(wingType)
    {
        case xfl::Main:      return "MAINWING";
        case xfl::Elevator:  return "ELEVATOR";
        case xfl::Fin:       return "FIN";
        case xfl::OtherWing: return "OTHERWING";
    }
    return "OTHERWING";
}
