/****************************************************************************

    flow5
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

#include <QXmlStreamWriter>
#include <QFile>

class WingXfl;
class FuseXfl;
class FuseOcc;
class FuseStl;
class PointMass;
class Vector3d;
class NURBSSurface;
class Inertia;
class Polar3d;

struct ExtraDrag;

struct LineStyle;

class XflXmlWriter : public QXmlStreamWriter
{
    public:
        XflXmlWriter(QFile &XFile);
        virtual ~XflXmlWriter();

        void writeXMLBody(FuseXfl const &bodyxfl);
        void writeXMLBody(FuseOcc const &bodyocc);
        void writeXMLBody(FuseStl const &bodystl);

        void writeWing(const WingXfl &wing, Vector3d WingLE, double Rx, double Ry);

    protected:
        virtual void writeHeader()=0;

        void writeXflFuse(FuseXfl const &body, Vector3d const &position);
        void writeNURBS(NURBSSurface const &nurbs, const QVector<int> &xPanels, bool bFramesOnly);
        void writePointMass(PointMass const &pm);
        void writeTheStyle(LineStyle const &theStyle);
        void writeColor(QColor const &color);
        void writeUnits();
        void writeExtraDrag(QVector<ExtraDrag> const &XDrag);
        void writeInertia(Inertia const &inertia);
        void writeFluidProperties(double viscosity, double density);
        void writeWakeData(const Polar3d &polar3d);

    protected:
        QString m_Path, m_FileName;
};

