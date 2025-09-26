/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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


