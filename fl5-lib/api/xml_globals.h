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

#include <QString>


#include <fl5lib_global.h>

#include <enums_objects.h>

namespace xml
{
    FL5LIB_EXPORT xfl::enumAnalysisMethod analysisMethod(QString const &strAnalysisMethod);
    FL5LIB_EXPORT QString analysisMethod(xfl::enumAnalysisMethod analysisMethod);


    FL5LIB_EXPORT xfl::enumPolarType polarType(const QString &strPolarType);
    FL5LIB_EXPORT QString polarType(xfl::enumPolarType polarType);


    FL5LIB_EXPORT xfl::enumBC boundaryCondition(QString const &strBC);
    FL5LIB_EXPORT QString boundaryCondition(xfl::enumBC boundaryCondition);

    FL5LIB_EXPORT QString referenceDimension(xfl::enumRefDimension refDimension);
    FL5LIB_EXPORT xfl::enumRefDimension referenceDimension(const QString &strRefDimension);

    FL5LIB_EXPORT QString wingType(xfl::enumType wingType);
    FL5LIB_EXPORT xfl::enumType wingType(QString const &strWingType);

}
