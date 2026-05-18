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



#pragma once

class QString;


#include <flow5-io-lib_global.h>

#include <enums_objects.h>

class Polar;

namespace xml
{
    FL5IOLIB_EXPORT xfl::enumAnalysisMethod analysisMethod(QString const &strAnalysisMethod);
    FL5IOLIB_EXPORT QString analysisMethod(xfl::enumAnalysisMethod analysisMethod);

    FL5IOLIB_EXPORT xfl::enumPolarType polarType(const QString &strPolarType);
    FL5IOLIB_EXPORT QString polarType(xfl::enumPolarType polarType);

    FL5IOLIB_EXPORT xfl::enumBC boundaryCondition(QString const &strBC);
    FL5IOLIB_EXPORT QString boundaryCondition(xfl::enumBC boundaryCondition);

    FL5IOLIB_EXPORT QString referenceDimension(xfl::enumRefDimension refDimension);
    FL5IOLIB_EXPORT xfl::enumRefDimension referenceDimension(const QString &strRefDimension);

    FL5IOLIB_EXPORT QString wingType(xfl::enumType wingType);
    FL5IOLIB_EXPORT xfl::enumType wingType(QString const &strWingType);

    Polar *importAnalysisFromXml(std::string const &pathname);

    FL5IOLIB_EXPORT  bool stringToBool(QString const &str);
    FL5IOLIB_EXPORT  QString boolToString(bool b);

}
