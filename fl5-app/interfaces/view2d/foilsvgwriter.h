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

#include <QXmlStreamReader>
#include <QFile>
#include <QSettings>

class Foil;
class FoilSVGWriter : public QXmlStreamWriter
{
    public:
        FoilSVGWriter(QFile &XFile);
        void writeFoil(Foil const *pFoil);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public:

        static void setSVGClosedTE(bool bClosed)     {s_bSVGClosedTE=bClosed;}
        static void setSVGFillFoil(bool bFill)       {s_bSVGFillFoil=bFill;}
        static void setSVGExportStyle(bool bExport)  {s_bSVGExportStyle=bExport;}
        static void setSVGScaleFactor(double factor) {s_SVGScaleFactor=factor;}
        static void setSVGMargin(double margin)      {s_SVGMargin=margin;}

        static bool   bSVGClosedTE()     {return s_bSVGClosedTE;}
        static bool   bSVGFillFoil()     {return s_bSVGFillFoil;}
        static bool   bSVGExportStyle()  {return s_bSVGExportStyle;}
        static double SVGScaleFactor()   {return s_SVGScaleFactor;}
        static double SVGMargin()        {return s_SVGMargin;}
    private:
        static bool s_bSVGClosedTE, s_bSVGFillFoil, s_bSVGExportStyle;
        static double s_SVGScaleFactor;
        static double s_SVGMargin;
};


