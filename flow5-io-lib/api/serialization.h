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

#include <QDataStream>
#include <QFile>

#include <flow5-io-lib_global.h>

class AngleControl;
class BSpline;
class BezierSpline;
class Boat;
class BoatOpp;
class BoatPolar;
class CubicSpline;
class EdgeSplit;
class Foil;
class Frame;
class Fuse;
class FuseOcc;
class FuseSections;
class FuseStl;
class FuseXfl;
class Inertia;
class NURBSSurface;
class Naca4Spline;
class OccMeshParams;
class OpPoint;
class Part;
class Plane;
class PlaneOpp;
class PlanePolar;
class PlanePolarExt;
class PlaneSTL;
class PlaneXfl;
class PointSpline;
class Polar3d;
class Polar;
class Sail;
class SailNurbs;
class SailOcc;
class SailSpline;
class SailStl;
class SailWing;
class Spline;
class SplineFoil;
class StabDerivatives;
class TriMesh;
class Triangulation;
class Vortex;
class Vorton;
class WingOpp;
class WingSailSection;
class WingSection;
class WingXfl;
struct AeroForces;
struct BLData;
struct BLXFoil;
struct EigenValues;
struct LineStyle;
struct SpanDistribs;
struct fl5Color;



namespace serial
{
    FL5IOLIB_EXPORT bool serializePolarFl5(Polar *pPolar, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePolarXFL(Polar *pPolar, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeOppXFL(OpPoint* pOpp, QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
    FL5IOLIB_EXPORT bool serializeOppFl5(OpPoint* pOpp, QDataStream &ar, bool bIsStoring);


    FL5IOLIB_EXPORT bool serializeFrameFl5(Frame *pFrame, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeFrameXfl(Frame *pFrame, QDataStream &ar, bool bIsStoring);


    FL5IOLIB_EXPORT bool readPolarFile(QFile &plrFile, std::vector<Foil *> &foilList, std::vector<Polar *> &polarList);

    FL5IOLIB_EXPORT bool serializeFoil(Foil*pFoil, QDataStream &ar);
    FL5IOLIB_EXPORT bool serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeFoilXfl(Foil *pFoil, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeFoilFl5(Foil *pFoil, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT void serializeBLXFoil(BLXFoil*pBL, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeSplineFl5(Spline *pSpline, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeBezierSplineFl5(BezierSpline *pSpline, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeCubicSplineFl5(CubicSpline *pSpline, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePointSplineFl5(PointSpline *pSpline, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeBSplineFl5(BSpline *pSpline, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeSplineFoilXfl(SplineFoil*pSF, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSplineFoilFl5(SplineFoil*pSF, QDataStream &ar, bool bIsStoring);



    FL5IOLIB_EXPORT bool serializePolar3dFl5v726(Polar3d*pPolar, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePolar3dFl5v750(Polar3d*pPolar, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeBoatPolarFl5v726(BoatPolar *pBtPolar, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeBoatPolarFl5v750(BoatPolar *pBtPolar, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializePlanePolarXfl(PlanePolar *pPolar, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlanePolarFl5v726(PlanePolar *pPolar, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlanePolarFl5v750(PlanePolar *pPolar, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializePlanePolarExtFl5v726(PlanePolarExt *pPolar,QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlanePolarExtFl5v750(PlanePolarExt *pPolar,QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializePartFl5(Part *pPart, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeSailFl5(Sail *pSail, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeWingSailSectionFl5(WingSailSection *pSec, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSailWingFl5(SailWing *pSail, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSailSTLFl5(SailStl*pSail, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSailSplineFl5(SailSpline *pSail, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSailOCCFl5(SailOcc *pSail, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSailNURBSFl5(SailNurbs *pSail, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeBoatFl5(Boat *pBoat, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeFuseFl5(Fuse *pFuse, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeFuseXFLXfl(FuseXfl *pFuse, QDataStream &ar, bool bIsStoring, int format);
    FL5IOLIB_EXPORT bool serializeFuseXflFl5(FuseXfl *pFuse, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeFuseStlFl5(FuseStl *pFuse, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeFuseOccFl5(FuseOcc *pFuse, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeFuseSectionFl5(FuseSections *pFuse, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeWingXflXfl(WingXfl*pWing, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeWingXflFl5(WingXfl*pWing, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializePlaneFl5(Plane *pPlane, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlaneXflXfl(PlaneXfl *pPlane, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlaneXflFl5(PlaneXfl *pPlane, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlaneStlFl5(PlaneSTL *pPlane, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeNURBSFl5(NURBSSurface *pNurbs, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeWingOppXFL(WingOpp *pWOpp, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeWingOppFl5(WingOpp *pWOpp, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeBoatOppFl5(BoatOpp *pBtOpp, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlaneOppXFL(PlaneOpp *pPOpp, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializePlaneOppFl5(PlaneOpp *pPOpp, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT void serializeEdgeSplit(EdgeSplit &ES, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeSpanResultsFl5(SpanDistribs &distrib, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT void serializeAeroForcesFl5_b17(AeroForces &AF, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeAeroForcesFl5(AeroForces &AF, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeStabDerivativesFl5(StabDerivatives &SD, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeTriangulationFl5(Triangulation & tr, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeVortexFl5(Vortex &vtx, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeVortonFl5(Vorton &vtn, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT void serializePanelsFl5(TriMesh &mesh, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT void serializeMeshFl5(TriMesh &mesh, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT void savePanels(TriMesh &mesh, QDataStream &ar);
    FL5IOLIB_EXPORT void loadPanels(TriMesh &mesh, QDataStream &ar);
    FL5IOLIB_EXPORT void saveMesh(TriMesh &mesh, QDataStream &ar);
    FL5IOLIB_EXPORT void loadMesh(TriMesh &mesh, QDataStream &ar);

    FL5IOLIB_EXPORT void serializeOccParams(OccMeshParams &params, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT void serializeAngleControlFl5(AngleControl &ctrl, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT void serializeEigenFl5(EigenValues &EV, QDataStream &ar, bool bIsStoring);


    FL5IOLIB_EXPORT bool serializeNaca4Spline(Naca4Spline &spline, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT void serializeBLDataFl5(BLData &data, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT bool serializeWingSectionFl5(WingSection &ws, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT void serializeLineStyleXfl(LineStyle &ls, QDataStream &ar, bool bIsStoring);
    FL5IOLIB_EXPORT void serializeLineStyleFl5(LineStyle &ls, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT bool serializeInertiaFl5(Inertia &inertia, QDataStream &ar, bool bIsStoring);

    FL5IOLIB_EXPORT void serializefl5Color(fl5Color &color, QDataStream &stream, bool bIsStoring);

}




