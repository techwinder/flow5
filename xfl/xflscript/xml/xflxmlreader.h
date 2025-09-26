/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QXmlStreamReader>
#include <QFile>

class PlaneXfl;
class PointMass;
class NURBSSurface;
class Inertia;
class Polar3d;
class FuseXfl;
class Vector3d;
class WingXfl;

struct ExtraDrag;
struct LineStyle;

class XflXmlReader : public QXmlStreamReader
{
    public:
        XflXmlReader(QFile &file);
        virtual ~XflXmlReader();

    protected:
        bool readTheStyle(LineStyle &theStyle);
        bool readColor(QColor &color);
        bool readPointMass(PointMass &pm, double massUnit, double lengthUnit);
        bool readNurbs(NURBSSurface &nurbs, QVector<int> &xpanels, double lengthUnit);
        bool readFluidData(double &viscosity, double &density);
        void readInertia(Inertia &inertia, double lengthunit, double massunit, double inertiaunit);
        bool readExtraDrag(QVector<ExtraDrag> &extra, double areaunit);
        bool readUnits(double &lengthunit, double &massunit, double &velocityunit, double &areaunit, double &inertiaunit);
        bool readWakeData(Polar3d &polar3d);
        bool readFuseXfl(FuseXfl *pBody, double lengthUnit, double massUnit);
        bool readWing(WingXfl *pWing, Vector3d &WingLE, double &Rx, double &Ry, double lengthUnit, double massUnit, double inertiaUnit);


    protected:
        int m_VMajor, m_VMinor; // version number
        QString m_FileName, m_Path;
};
