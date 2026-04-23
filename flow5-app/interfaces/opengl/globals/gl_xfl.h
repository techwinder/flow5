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

#include <QOpenGLBuffer>

#include <api/panel3.h>
#include <api/panel4.h>
#include <api/vorton.h>

class SailSpline;
class SailWing;
class Boat;
class BoatPolar;
class FuseXfl;
class Fuse;
class FuseSections;

// gl related methods using xfl objects

namespace gl
{
    void makeQuadPanels(const std::vector<Panel4> &panel4list, Vector3d const &pos, QOpenGLBuffer &vbo);
    void makeQuadEdges(const std::vector<Panel4> &panel4list, Vector3d const &pos, QOpenGLBuffer &vbo);
    void makeTriPanels(const std::vector<Panel3> &panel3, const Vector3d &position, QOpenGLBuffer &vbo);
    void makeTriEdges(std::vector<Panel3> const &panel3, const Vector3d &position, QOpenGLBuffer &vbo);
    void makePanelNormals(const std::vector<Panel4> &panel4list, float length, QOpenGLBuffer &vbo);
    void makePanelNormals(const std::vector<Panel3> &panel3, float length, QOpenGLBuffer &vbo);
    void makeQuadNodeClrMap(const std::vector<Panel4> &panel4list, const std::vector<double> &data, double &lmin, double &lmax, bool bAuto, QOpenGLBuffer &vbo);
    void makeTriUniColorMap(const std::vector<Panel3> &panel3list, const std::vector<double> &tab, double &lmin, double &lmax, bool bAuto, QOpenGLBuffer &vbo);
    void makeTriangleNodeValues(QVector<Panel3> const &panel3, QVector<Node> const &NodeList, QVector<double> const &tab,
                                QVector<double> &CpInf, QVector<double> &CpSup, QVector<double> &Cp100,
                                double &lmin, double &lmax);

    void makeTriLinColorMap(const std::vector<Panel3> &panel3, const std::vector<Node> &NodeList, const std::vector<double> &Cp,
                            double lmin, double lmax, QOpenGLBuffer &vbo);
    void makeTriangleContoursOnMesh(const std::vector<Panel3> &panel3list, const std::vector<double> &Cp,
                                    double lmin, double lmax, QOpenGLBuffer &vbo);

    void makeVortons_spheres(std::vector<std::vector<Vorton>> const &Vortons, QOpenGLBuffer &vbo);
    void makeVortons(std::vector<std::vector<Vorton>> const &Vortons, QOpenGLBuffer &vbo);

    void makePanelForces(std::vector<Panel4> const &panel4list, const std::vector<double> &Cp, float qDyn, bool bVLM, double &rmin, double &rmax, bool bAuto, double scale, QOpenGLBuffer &vbo);
    void makePanelForces(const std::vector<Panel3> &panel3list, const std::vector<double> &Cp, float qDyn, double &rmin, double &rmax, bool bAuto, double scale, QOpenGLBuffer &vbo);
    void makeSplineSailOutline(const SailSpline *pSSail, Vector3d const &pos, QOpenGLBuffer &vbo);
    void makeWingSailOutline(const SailWing *pWSail, Vector3d const &pos, QOpenGLBuffer &vbo);
    void makeSectionHighlight(const SailSpline *pSSail, Vector3d pos, QOpenGLBuffer &vbo);
    void makeSectionHighlight(const SailWing *pWSail, Vector3d pos, QOpenGLBuffer &vbo);
    void makeWind(Boat const *pBoat, BoatPolar const *pBoatPolar, QOpenGLBuffer &vbo);

    void makeFuseXflQuadPanels(const FuseXfl *pFuseXfl, Vector3d position, QOpenGLBuffer &vbo);
    void makeFuseXflFlatPanels(const FuseXfl *pFuseXfl, Vector3d position, QOpenGLBuffer &vbo);
    void makeFuseXflSplinePanels(const FuseXfl *pFuseXfl, Vector3d position, QOpenGLBuffer &vbo);

    void makeFuseTriMesh(const Fuse *pFuse, const Vector3d &pos, QOpenGLBuffer &vbo, QOpenGLBuffer &vboEdges);
    void makeBodySplines_outline(const FuseXfl *pFuseXfl, const Vector3d &pos, QOpenGLBuffer &vbo);
    void makeFuseXflFrames(const FuseXfl *pFuseXfl, Vector3d const &pos, int NXXXX, int NHOOOP, QOpenGLBuffer &vbo);
    void makeFuseXflSections(const FuseSections *pFuseSecs, Vector3d const &pos, int NXXXX, int NHOOOP, QOpenGLBuffer &vbo);

    void makeBodyFlatFaces_2triangles_outline(const FuseXfl *pBody, const Vector3d &pos, QOpenGLBuffer &vbo);

}

