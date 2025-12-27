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

#pragma once

#include <QFile>

class Foil;
class Polar;

#include <fl5lib_global.h>

namespace objects
{
FL5LIB_EXPORT bool readFoilFile(const std::string &filename, Foil *pFoil, int &iLineError);
    FL5LIB_EXPORT bool readPolarFile(QFile &plrFile, std::vector<Foil *> &foilList, std::vector<Polar *> &polarList);

    FL5LIB_EXPORT bool serializeFoil(Foil*pFoil, QDataStream &ar);
    FL5LIB_EXPORT bool serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring);
}
