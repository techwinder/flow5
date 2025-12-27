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

#include <api/fl5lib_global.h>

class Polar;

class PolarNameMaker
{
    friend class  PolarAutoNameDlg;

    public:
        PolarNameMaker(Polar *pPolar);
        static QString makeName(const Polar *pPolar);

    private:
        Polar *m_pPolar;

        static bool s_bType;
        static bool s_bBLMethod;
        static bool s_bReynolds;
        static bool s_bMach;
        static bool s_bNCrit;
        static bool s_bXTrTop;
        static bool s_bXTrBot;
};
