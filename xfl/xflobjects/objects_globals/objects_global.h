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

#include <QFile>
#include <QPainter>
#include <QVector>


#include <xflcore/enums_objects.h>
#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflmath/mathelem.h>

#include <complex>

class BSpline;
class CubicSpline;
class Foil;
class Inertia;
class Node;
class PlaneOpp;
class PlaneXfl;
class Polar;
class Segment2d;
class Spline;
class TriMesh;
class Triangle3d;
class WPolar;
class WingXfl;
class Boat;
class BoatOpp;


bool exportMeshToSTLFile(TriMesh const &trimesh, const QString &FileName, const QString &STLDirName, double mtounit);
int exportTriMesh(QDataStream &outStream, double scalefactor, TriMesh const &trimesh);

int exportTriangulation(QDataStream &outStream, double scalefactor, QVector<Triangle3d> const &triangle);

QString surfacePosition(xfl::enumSurfacePosition pos);

xfl::enumPolarType polarType(const QString &strPolarType);
QString polarType(xfl::enumPolarType polarType);


void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &dsi);

void computeSurfaceInertia(Inertia &inertia, QVector<Triangle3d> const &triangles, const Vector3d &PartPosition);

Vector3d windDirection(double alpha, double beta);
Vector3d windSide(double alpha, double beta);
Vector3d windNormal(double alpha, double beta);
Vector3d windToGeomAxes(Vector3d const &Vw, double alpha, double beta);


bool readVSPFoilFile(const QString &FoilFileName, Foil *pFoil);


void fillSectionCp3Uniform(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, QVector<double> &Cp, QVector<Node> &pts);
void fillSectionCp3Linear( PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, QVector<double> &Cp, QVector<Node> &pts);
void fillSectionCp4(       PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, QVector<double> &Cp, QVector<Node> &pts);

void fillSectionCp3Uniform(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, QVector<double> &Cp, QVector<Node> &pts);
void fillSectionCp3Linear( Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, QVector<double> &Cp, QVector<Node> &pts);

int AVLSpacing(xfl::enumDistribution distrib);


void VLMCmnVelocity(Vector3d const &A, Vector3d const &B, Vector3d const &C, Vector3d &V, bool bAll, double fardist);
void VLMQmnVelocity(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, Vector3d &V);
void VLMQmnPotential(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, double &phi);




