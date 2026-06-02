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

#include <QList>
#include <QFile>

#include <api/linestyle.h>
#include <api/enums_objects.h>
#include <api/sail.h>

class Boat;
class BoatOpp;
class Foil;
class MainFrame;
class Node;
class Part;
class Plane;
class PlaneOpp;
class PlaneSTL;
class PlaneXfl;
class Polar;
class Sail;
class Triangle3d;
class TriMesh;
class PlanePolar;
class WingXfl;


namespace Objects3d
{
    extern MainFrame *g_pMainFrame;
    inline void setMainFrame(MainFrame *pMainFrame) {g_pMainFrame=pMainFrame;} // to position popup windows

    Plane * setModifiedPlane(Plane *pModPlane);
    PlanePolar* insertNewPolar(PlanePolar *pNewWPolar, Plane const*pCurPlane);
    void renamePlanePolar(PlanePolar *pWPolar, Plane const *pPlane);
    void renamePlane(QString const &PlaneName);


    QStringList planeNames();
    QStringList polarNames(Plane const*pPlane);


    void fillSectionCp3Uniform(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts);
    void fillSectionCp3Linear( PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts);
    void fillSectionCp4(       PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts);

    void fillSectionCp3Uniform(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts);
    void fillSectionCp3Linear( Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts);

    void makePlaneTriangulation(Plane *pPlane);
    void makeBoatTriangulation(Boat *pBoat);
    void makeSailTriangulation(Sail *pSail, int nx=Sail::iXRes(), int nz=Sail::iZRes());
}


