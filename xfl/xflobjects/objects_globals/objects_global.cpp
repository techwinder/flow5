/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include <QDataStream>
#include <QFileInfo>
#include <QFileDialog>

#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom2d/segment2d.h>
#include <xflgeom/geom3d/quaternion.h>
#include <xflmath/constants.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>
#include <xflfoil/objects2d/splinefoil.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects_globals/objects_global.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/objects3d/analysis/boatopp.h>


void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &zeta)
{
    omega1 = fabs(lambda.imag());

    if(omega1 > PRECISION)
    {
        omegaN = sqrt(lambda.real()*lambda.real()+omega1*omega1);
        zeta = -lambda.real()/omegaN;
    }
    else
    {
        omegaN = 0.0;
        zeta = 0.0;
    }
}


Vector3d windDirection(double alpha, double beta)
{
    //change of beta sign introduced in v7.24 to be consistent with AVL
    Vector3d Vinf;
    Vinf.x =   cos(alpha*PI/180.0) * cos(-beta*PI/180.0);
    Vinf.y =   sin(-beta*PI/180.0);
    Vinf.z =   sin(alpha*PI/180.0) * cos(-beta*PI/180.0);
    return Vinf;
}


Vector3d windSide(double alpha, double beta)
{
     //change of beta sign introduced in v7.24 to be consistent with AVL
    Vector3d Vinf;
    Vinf.x =  -cos(alpha*PI/180.0) * sin(-beta*PI/180.0);
    Vinf.y =   cos(-beta*PI/180.0);
    Vinf.z =  -sin(alpha*PI/180.0) * sin(-beta*PI/180.0);
    return Vinf;
}


Vector3d windNormal(double alpha, double beta)
{
    (void)beta;
    // V=(0,0,1)_wind
    Vector3d Vinf;
    Vinf.x = -sin(alpha*PI/180.0);
    Vinf.y = 0.0;
    Vinf.z = cos(alpha*PI/180.0);
    return Vinf;
}


/** Used in the case of a T6 polars to convert the forces to the rotated body axes*/
Vector3d windToGeomAxes(Vector3d const &Vw, double alpha, double beta)
{
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cosb = cos(beta*PI/180.0);
    double sinb = sin(beta*PI/180.0);

    double i[9];
    i[0] = cosb*cosa;    i[1] = -sinb*cosa;     i[2] = -sina;
    i[3] = sinb;         i[4] = cosb;           i[5] = 0;
    i[6] = cosb*sina;    i[7] = -sinb*sina;     i[8] = cosa;

    Vector3d Vg;
    Vg.x = i[0]*Vw.x +i[1]*Vw.y + i[2]*Vw.z;
    Vg.y = i[3]*Vw.x +i[4]*Vw.y + i[5]*Vw.z;
    Vg.z = i[6]*Vw.x +i[7]*Vw.y + i[8]*Vw.z;
    return Vg;
}



xfl::enumPolarType polarType(QString const &strPolarType)
{
    QString strange = strPolarType.trimmed();
    if     (strange.compare("FIXEDSPEEDPOLAR", Qt::CaseInsensitive)==0) return xfl::T1POLAR;
    else if(strange.compare("FIXEDLIFTPOLAR",  Qt::CaseInsensitive)==0) return xfl::T2POLAR;
    else if(strange.compare("GLIDEPOLAR",      Qt::CaseInsensitive)==0) return xfl::T3POLAR;
    else if(strange.compare("FIXEDAOAPOLAR",   Qt::CaseInsensitive)==0) return xfl::T4POLAR;
    else if(strange.compare("BETAPOLAR",       Qt::CaseInsensitive)==0) return xfl::T5POLAR;
    else if(strange.compare("CONTROLPOLAR",    Qt::CaseInsensitive)==0) return xfl::T6POLAR;
    else if(strange.compare("STABILITYPOLAR",  Qt::CaseInsensitive)==0) return xfl::T7POLAR;
    else if(strange.compare("T8POLAR",         Qt::CaseInsensitive)==0) return xfl::T8POLAR;
    else if(strange.compare("BOATPOLAR",       Qt::CaseInsensitive)==0) return xfl::BOATPOLAR;
    else return xfl::T1POLAR;
}


QString polarType(xfl::enumPolarType polarType)
{
    switch(polarType)
    {
        case xfl::T1POLAR:   return "FIXEDSPEEDPOLAR";
        case xfl::T2POLAR:   return "FIXEDLIFTPOLAR";
        case xfl::T3POLAR:   return "GLIDEPOLAR";
        case xfl::T4POLAR:   return "FIXEDAOAPOLAR";
        case xfl::T5POLAR:   return "BETAPOLAR";
        case xfl::T6POLAR:   return "CONTROLPOLAR";
        case xfl::T7POLAR:   return "STABILITYPOLAR";
        case xfl::T8POLAR:   return "T8POLAR";
        case xfl::BOATPOLAR: return "BOATPOLAR";
        default: return QString();
    }
}


/**
 * Exports the part's triangular mesh to an STL file
 */
bool exportMeshToSTLFile(TriMesh const &trimesh, QString const &FileName, QString const &STLDirName, double mtounit)
{
    QString filter ="STL File (*.stl)";

    QString filename(FileName+".stl");

    filename.replace('/', '_');
    filename.replace(' ', '_');
    filename += "_mesh";

    filename = QFileDialog::getSaveFileName(nullptr, "Export to STL File",
                                            STLDirName + "/"+filename,
                                            "STL File (*.stl)",
                                            &filter);

    if(!filename.length()) return false;

    bool bBinary = true;

    int pos = filename.indexOf(".stl", Qt::CaseInsensitive);
    if(pos<0) filename += ".stl";

    QFile XFile(filename);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    if(bBinary)
    {
        QDataStream out(&XFile);
        exportTriMesh(out,mtounit, trimesh);
    }
    else
    {
//        QTextStream out(&XFile);
    }

    XFile.close();
    return true;
}


int exportTriMesh(QDataStream &outStream, double scalefactor, TriMesh const &trimesh)
{
    outStream.setByteOrder(QDataStream::LittleEndian);

    /***
     *  UINT8[80] – Header
     *     UINT32 – Number of triangles
     *
     *     foreach triangle
     *     REAL32[3] – Normal vector
     *     REAL32[3] – Vertex 1
     *     REAL32[3] – Vertex 2
     *     REAL32[3] – Vertex 3
     *     UINT16 – Attribute byte count
     *     end
    */

    //    80 character header, avoid word "solid"
    // leave 1 extra character for end zero
    //                   0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

    QString strong =     "--- STL file ---                                                               ";

    xfl::writeCString(outStream, strong);

    outStream << trimesh.nPanels();

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));

    for (int it=0; it<trimesh.nPanels(); it++)
    {
        Panel3 const &p3 = trimesh.panelAt(it);
        xfl::writeFloat(outStream, p3.normal().xf());
        xfl::writeFloat(outStream, p3.normal().yf());
        xfl::writeFloat(outStream, p3.normal().zf());

        if(p3.isPositiveOrientation())
        {
            xfl::writeFloat(outStream, float(p3.vertexAt(0).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(1).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(2).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).z*scalefactor));
        }
        else
        {
            xfl::writeFloat(outStream, float(p3.vertexAt(0).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(2).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(1).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).z*scalefactor));
        }

        outStream.writeRawData(buffer, 2);
    }
    return trimesh.nPanels();
}


int exportTriangulation(QDataStream &outStream, double scalefactor, QVector<Triangle3d> const &triangle)
{
    /***
     *  UINT8[80] – Header
     *     UINT32 – Number of triangles
     *
     *     foreach triangle
     *     REAL32[3] – Normal vector
     *     REAL32[3] – Vertex 1
     *     REAL32[3] – Vertex 2
     *     REAL32[3] – Vertex 3
     *     UINT16 – Attribute byte count
     *     end
    */

    //    80 character header, avoid word "solid"
    // leave 1 extra character for end zero
    //                   0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

    QString strong =     "--- STL file ---                                                               ";

    xfl::writeCString(outStream, strong);

    outStream << triangle.count();

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));

    for (int it=0; it<triangle.count(); it++)
    {
        Triangle3d const & t3 = triangle.at(it);
        xfl::writeFloat(outStream, t3.normal().xf());
        xfl::writeFloat(outStream, t3.normal().yf());
        xfl::writeFloat(outStream, t3.normal().zf());

        xfl::writeFloat(outStream, float(t3.vertexAt(0).x*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(0).y*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(0).z*scalefactor));

        xfl::writeFloat(outStream, float(t3.vertexAt(1).x*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(1).y*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(1).z*scalefactor));

        xfl::writeFloat(outStream, float(t3.vertexAt(2).x*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(2).y*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(2).z*scalefactor));

        outStream.writeRawData(buffer, 2);
    }
    return triangle.size();
}


void computeSurfaceInertia(Inertia &inertia, QVector<Triangle3d> const &triangles, Vector3d const &PartPosition)
{
    Vector3d cog_s;
    double CoGIxx=0, CoGIyy=0, CoGIzz=0, CoGIxz=0;
    double Ixx=0.0,Iyy=0.0,Izz=0.0,Ixz=0.0;

    cog_s.set(0.0,0.0,0.0);

    // compute the CoG position = average of all triangle's CoG
    // compute the inertia tensor in the body axis
    // using unit density, i.e. triangle mass = area
    double totalarea = 0.0;
    for(int it=0; it<triangles.count(); it++)
    {
        const Triangle3d &t3 = triangles.at(it);
        cog_s += t3.CoG_g() * t3.area();

        Ixx +=  t3.area() * (t3.CoG_g().y*t3.CoG_g().y + t3.CoG_g().z*t3.CoG_g().z);
        Iyy +=  t3.area() * (t3.CoG_g().x*t3.CoG_g().x + t3.CoG_g().z*t3.CoG_g().z);
        Izz +=  t3.area() * (t3.CoG_g().x*t3.CoG_g().x + t3.CoG_g().y*t3.CoG_g().y);
        // sign modification in v7.13 from negative to positive
        Ixz += t3.area() * (t3.CoG_g().x*t3.CoG_g().z);

        totalarea += t3.area();
    }
    cog_s *= 1.0/totalarea;
    //    m_Inertia.setCoG(CoG);

    //Apply Huyghens/Steiner theorem to get inertia tensor in CoG reference frame
    double d2 = cog_s.x*cog_s.x + cog_s.y*cog_s.y + cog_s.z*cog_s.z;
    CoGIxx = Ixx - totalarea * (d2 - cog_s.x*cog_s.x);
    CoGIyy = Iyy - totalarea * (d2 - cog_s.y*cog_s.y);
    CoGIzz = Izz - totalarea * (d2 - cog_s.z*cog_s.z);
    CoGIxz = Ixz - totalarea * (   - cog_s.x*cog_s.z);

    double density = inertia.structuralMass()/totalarea;
    CoGIxx *= density;
    CoGIyy *= density;
    CoGIzz *= density;
    CoGIxz *= density;

    cog_s = cog_s-PartPosition;

    inertia.setCoG_s(cog_s);
    inertia.setIxx_s(CoGIxx);
    inertia.setIyy_s(CoGIyy);
    inertia.setIzz_s(CoGIzz);
    inertia.setIxz_s(CoGIxz);
}



QString surfacePosition(xfl::enumSurfacePosition pos)
{
    switch(pos)
    {
        case xfl::BOTSURFACE:
            return "BOTTOM";
            break;
        case xfl::MIDSURFACE:
            return "MID";
            break;
        case xfl::TOPSURFACE:
            return "TOP";
            break;
        case xfl::SIDESURFACE:
            return "SIDE";
            break;
        case xfl::FUSESURFACE:
            return "FUSE";
            break;
        case xfl::WAKESURFACE:
            return "WAKE";
            break;
        default:
        case xfl::NOSURFACE:
            return "NOSURFACE";
            break;
    }
}


void fillSectionCp3Uniform(PlaneXfl const* pPlaneXfl, PlaneOpp const*pPOpp, int iWing, int iStrip, QVector<double> &Cp, QVector<Node> &pts)
{
//    qDebug()<<"istrip"<<iStrip;
    pts.clear();
    Cp.clear();

    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;
    int i3=0;
    bool bFound = false;

    QVector<Panel3> const &panels = pPlaneXfl->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    for (i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panels.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    if(!bFound) return;

    Q_ASSERT(p3->isTrailing());
    do
    {
        Cp.append(pPOpp->Cp(p3->index()*3));
        pts.append(p3->CoG());
        pts.last().setNormal(p3->normal());
        if(p3->iPU()==-1) p3=nullptr;
        else              p3=panels.data()+p3->index()+1;
    }
    while (p3);
}


void fillSectionCp3Linear(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip,
                          QVector<double> &Cp, QVector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

//    Vector3d WingLE = pPlaneXfl->wingLE(iWing);

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;

    int nxnodes = 0;
    if(pPOpp->bThickSurfaces()) nxnodes = pWing->nXPanel3()+1;
    else                        nxnodes = pWing->nXPanels()+1;

    bool bFound = false;

    QVector<Panel3> const &panel3 = pPlaneXfl->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    int iRightNode = -1; // only used in the case of the right tip strip
    for (int i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panel3.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            iRightNode = p3->rightTrailingNode().index();
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    int iStartNode = 0;

    if(bFound)
        iStartNode = p3->leftTrailingNode().index();
    else
    {
        if(iStrip==strip)
            // case of the last right node strip
            iStartNode = iRightNode;
        else  return; // something went wrong
    }

    for(int in=iStartNode; in<iStartNode+nxnodes; in++)
    {
        Node const &nd = pPlaneXfl->node(in);
        Cp.append(pPOpp->nodeValue(in));
        pts.append(nd);
    }
}


void fillSectionCp4(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, QVector<double> &Cp, QVector<Node> &pts)
{
    pts.clear();
    Cp.clear();
    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

//    Vector3d WingLE = pPlaneXfl->wingLE(iWing);

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);
    WingOpp const& wopp = pPOpp->m_WingOpp.at(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;

    int i4=0;
    bool bFound = false;

    int coef = pPOpp->bThinSurfaces() ? 1 : 2;

    QVector<Panel4> const &panel4 = pPlaneXfl->quadpanels();
    int strip=0;
    for (i4=0; i4<pPOpp->nPanel4(); i4++)
    {
        Panel4 const &p4 = panel4.at(pWing->firstPanel4Index() + i4);
        if(p4.isTrailing() && p4.surfacePosition()<=xfl::MIDSURFACE)
        {
            if(strip == iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    Q_ASSERT(strip<pWing->nStations());

    if(bFound)
    {
        for (int pp=i4; pp<i4+coef*pWing->surfaceAt(0).NXPanels(); pp++)
        {
            Panel4 const &p4 = panel4.at(pWing->firstPanel4Index() + pp);
            Cp.append(wopp.m_dCp[pp]);
//            pts.append(p4.m_CollPt-WingLE);
            pts.append(p4.m_CollPt);
            pts.last().setNormal(p4.normal());
        }
    }
}


void fillSectionCp3Uniform(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, QVector<double> &Cp, QVector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iSail<0 || iSail>=pBoat->nSails()) return;

    Sail const*pWing = pBoat->sailAt(iSail);

    if(iStrip<0 || iStrip>pWing->nStations()) return;
    int i3=0;
    bool bFound = false;

    QVector<Panel3> const &panel3 = pBoat->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    for (i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panel3.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    if(bFound)
    {
        Q_ASSERT(iStrip>=0 && iStrip<pWing->nStations());
        Q_ASSERT(p3->isTrailing());
        do
        {
            Cp.append(pBtOpp->Cp(p3->index()*3));
            pts.append(p3->CoG());
            pts.last().setNormal(p3->normal());
            if(p3->iPU()==-1) p3=nullptr;
            else              p3=panel3.data()+p3->index()+1;
        }
        while (p3);
    }
}


void fillSectionCp3Linear(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, QVector<double> &Cp, QVector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iSail<0 || iSail>=pBoat->nSails()) return;

    Sail const*pWing = pBoat->sailAt(iSail);

    if(iStrip<0 || iStrip>pWing->nStations()) return;

    int nxnodes = 0;
    if(pBtOpp->bThickSurfaces()) nxnodes = pWing->nXPanels()*2+1;
    else                         nxnodes = pWing->nXPanels()+1;

    bool bFound = false;

    QVector<Panel3> const &panel3 = pBoat->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    int iRightNode = -1; // only used in the case of the right tip strip
    for (int i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panel3.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            iRightNode = p3->rightTrailingNode().index();
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    int iStartNode = 0;

    if(bFound)
        iStartNode = p3->leftTrailingNode().index();
    else
    {
        if(iStrip==strip)
            // case of the last right node strip
            iStartNode = iRightNode;
        else  return; // something went wrong
    }

    for(int in=iStartNode; in<iStartNode+nxnodes; in++)
    {
        Node const &nd = pBoat->node(in);
        Cp.append(pBtOpp->nodeValue(in));
        pts.append(nd);
    }
}


int AVLSpacing(xfl::enumDistribution distrib)
{

    /*         AVL spacing
               3.0        equal         |   |   |   |   |   |   |   |   |
               2.0        sine          || |  |   |    |    |     |     |
               1.0        cosine        ||  |    |      |      |    |  ||
               0.0        equal         |   |   |   |   |   |   |   |   |
              -1.0        cosine        ||  |    |      |      |    |  ||
              -2.0       -sine          |     |     |    |    |   |  | ||
              -3.0        equal         |   |   |   |   |   |   |   |   | */
    switch (distrib)
    {
        default:
        case xfl::UNIFORM:
            return 0;
        case xfl::COSINE:
        case xfl::TANH:
            return 1;
        case  xfl::SINE:
        case  xfl::EXP:
            return 2;
        case  xfl::INV_SINE:
        case  xfl::INV_EXP:
            return -2;
    }
    return 0;
}


bool readVSPFoilFile(QString const &FoilFileName, Foil *pFoil)
{
    QString strong;
    QString FoilName;

    int pos(0);
    double x(0), y(0);
    double xp(0), yp(0);
    bool bRead=false;

    QFileInfo fi(FoilFileName);
    if(!fi.exists()) return false;

    QFile xFoilFile(FoilFileName);
    if(!xFoilFile.open(QIODevice::ReadOnly)) return false;

    QTextStream inStream(&xFoilFile);

    QFileInfo fileInfo(xFoilFile);

    QString fileName = fileInfo.fileName();
    int suffixLength = fileInfo.suffix().length()+1;
    fileName = fileName.left(fileName.size()-suffixLength);

    FoilName = inStream.readLine();
    pos = FoilName.length()-FoilName.lastIndexOf("/");
    FoilName = FoilName.right(pos-1);
    pos = FoilName.lastIndexOf(".dat");
    FoilName.truncate(pos);
    pFoil->setName(FoilName);

    bRead = true;
    xp=-9999.0;
    yp=-9999.0;
    do
    {
        strong = inStream.readLine().trimmed();
        QStringList fields = strong.split(",");
        if(fields.size()==2)
        {
            x = fields.at(0).trimmed().toDouble();
            y = fields.at(1).trimmed().toDouble();
            //add values only if the point is not coincident with the previous one
            double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
            if(dist>0.000001)
            {
                pFoil->m_BaseNode.push_back({x,y});

                xp = x;
                yp = y;
            }
        }
        else bRead = false;

    }while (bRead && !strong.isNull());

    xFoilFile.close();

    pFoil->m_Node.resize(pFoil->nBaseNodes());
    for(int i=0; i<pFoil->nBaseNodes(); i++)
    {
        pFoil->m_Node[i].x = pFoil->xb(i);
        pFoil->m_Node[i].y = pFoil->yb(i);
    }
    pFoil->initGeometry();
    return true;
}



/**
 * HORSESHOE VORTEX FORMULATION
 *
 *    LA__________LB               |
 *    |           |                |
 *    |           |                | freestream velocity vector
 *    |           |                |
 *    |           |               \/
 *    |           |
 *   \/          \/
 *
 *
 * Returns the velocity greated by a horseshoe vortex with unit circulation at a distant point
 *
 * Notes :
 *  - The geometry has been rotated by the sideslip angle, hence, there is no need to align the trailing vortices with sideslip
 *  - Vectorial operations are written inline to save computing times -->longer code, but 4x more efficient....
 *
 * @param A the left point of the bound vortex
 * @param B the right point of the bound vortex
 * @param C the point where the velocity is calculated
 * @param V the resulting velocity vector at point C
 * @param bAll true if the influence of the bound vortex should be evaluated; false for a distant point in the far field.
 */
void VLMCmnVelocity(Vector3d const &A, Vector3d const &B, Vector3d const &C,
                            Vector3d &V, bool bAll, double fardist)
{
    Vector3d VelSeg, Far;
    V.x = 0.0;
    V.y = 0.0;
    V.z = 0.0;

    if(bAll)
    {
        vortexInducedVelocity(A, B, C, VelSeg, Vortex::coreRadius());
        V += VelSeg;
    }

    // Create Far points to align the trailing vortices with the reference axis
    // The trailing vortex legs are not aligned with the free-stream, i.a.w. the small angle approximation
    // If this approximation is not valid, then the geometry should be tilted in the polar definition

    // calculate left contribution
    Far.x = A.x +  fardist;
    Far.y = A.y ;
    Far.z = A.z;// + (Far_x-A.x) * tan(m_Alpha*PI/180.0);
    vortexInducedVelocity(Far, A, C, VelSeg, Vortex::coreRadius());
    V += VelSeg;


    // calculate right vortex contribution
    Far.x = B.x +  fardist;
    Far.y = B.y;
    Far.z = B.z;// + (Far_x-B.x) * tan(m_Alpha*PI/180.0);
    vortexInducedVelocity(B, Far, C, VelSeg, Vortex::coreRadius());
    V += VelSeg;
}


/** No explicit formulation of the potential for a vortex filament.
 * Use instead the equivalence of the vortex ring to a uniform doublet strength panel */
void VLMQmnPotential(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB,
                             Vector3d const &C, double &phi)
{
    // make a false quad Panel
    Panel4 p4Ring(LA, LB, TA, TB);

    // get the induced potential
    p4Ring.doubletN4023Potential(C, false, phi, Vortex::coreRadius(), true);
}



void VLMQmnVelocity(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB, Vector3d const &C, Vector3d &V)
{
    Vector3d const m_pR[] = {LB, TB, TA, LA, LB};
    Vector3d velseg;
    V.x = 0.0;
    V.y = 0.0;
    V.z = 0.0;

    for (int i=0; i<4; i++)
    {
        vortexInducedVelocity(m_pR[i], m_pR[i+1], C, velseg, Vortex::coreRadius());
        V+= velseg;
    }
}
