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

#include <QString>
#include <QTextStream>
#include <QDataStream>

#include <flow5-io-lib_global.h>
#include <fl5color.h>
#include <utils.h>

class Foil;
class FuseStl;
class PlanePolar;
class PlaneSTL;
class PlaneXfl;
class TriMesh;
class Triangle3d;
class Vector3d;
class WingSection;
class WingXfl;

namespace io
{
    typedef enum {STL, OBJ} enumMeshFileType;


    FL5IOLIB_EXPORT  bool saveProject(const std::string &stdPathName, std::string &logmsg);
    FL5IOLIB_EXPORT  bool loadProject(const std::string &stdPathName, std::string &logmsg);

    FL5IOLIB_EXPORT  bool readSTLFile(const std::string &FilePath, double FileUnitsToMeter, double theMergeAngle, std::vector<Triangle3d> &triangles, Vector3d &botleft, Vector3d &topright);
    FL5IOLIB_EXPORT  bool readOBJFile(const std::string &FilePath, double FileUnitsToMeter, std::vector<Triangle3d> &triangles, Vector3d &botleft, Vector3d &topright);

    FL5IOLIB_EXPORT PlaneSTL* importPlaneFromMesh(std::string const&FilePath, enumMeshFileType type, double FileUnitsToMeter, std::string &logmsg);
    FL5IOLIB_EXPORT FuseStl* importFuseFromMesh(std::string const&FilePath, enumMeshFileType type, double FileUnitsToMeter, std::string &logmsg);


    FL5IOLIB_EXPORT bool exportMeshToSTLFile(const QString &filename, TriMesh const &trimesh, double mtounit);
    FL5IOLIB_EXPORT int exportTriangulationToSTL(const QString &pathname, double scalefactor, std::vector<Triangle3d> const &triangle);

    FL5IOLIB_EXPORT int exportTriMeshToSTL(QString const &pathname, double scalefactor, TriMesh const &trimesh);

    FL5IOLIB_EXPORT bool importVSPWing(QString const &filename, QVector<WingXfl *> &winglist, QString &logmsg);
    FL5IOLIB_EXPORT bool readVSPFoilFile(QString const &FoilFileName, Foil *pFoil);
    FL5IOLIB_EXPORT void readVSPSection(QTextStream &stream, QString &wingname, int &index, WingSection &ws);

    FL5IOLIB_EXPORT bool exportAllStlMesh(QString const &pathname);

    FL5IOLIB_EXPORT bool exportAllPolars(const QString &pathName, xfl::enumTextFileType fileType);
    FL5IOLIB_EXPORT bool exportAllWPolars(const QString &pathName, bool bCSV);
    FL5IOLIB_EXPORT bool exportAllBtPolars(QString const &pathname, bool bCSV);
    FL5IOLIB_EXPORT bool exportAllPOpps(QString const &pathName, bool bCSV, bool bPanelData);
    FL5IOLIB_EXPORT bool exportAllBtOpps(const QString &pathname, bool bCSV, bool bPanelData);

    FL5IOLIB_EXPORT bool writeFoilPolars(QString const &pathname, Foil *pFoil);

    FL5IOLIB_EXPORT PlaneXfl*  importPlaneFromXML(const std::string &xmlfilepath, std::string &logmsg);
    FL5IOLIB_EXPORT PlanePolar* importAnalysisFromXML(std::string const &xmlfilepath, std::string &logmsg);
}
