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

#include <filesystem>

#include <RWObj.hxx>
#include <RWStl.hxx>
#include <gp_Trsf.hxx>
#include <Bnd_Box.hxx>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <boat.h>
#include <boatopp.h>
#include <boatpolar.h>
#include <fileio.h>
#include <flow5-io.h>
#include <foil.h>
#include <objects2d.h>
#include <objects3d.h>
#include <occ_globals.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planepolarnamemaker.h>
#include <planestl.h>
#include <planexfl.h>
#include <sailobjects.h>
#include <serialization.h>
#include <triangle3d.h>
#include <trimesh.h>
#include <units.h>
#include <utils-io.h>
#include <utils.h>
#include <vector3d.h>
#include <xmlplanepolarreader.h>
#include <xmlplanereader.h>

fl5Color io::readQColor(QDataStream &ar)
{
    uchar byte=0;

    ar>>byte; // a format identifier?

    ar>>byte>>byte;
    int a = int(byte);
    ar>>byte>>byte;
    int r = int(byte);
    ar>>byte>>byte;
    int g = int(byte);
    ar>>byte>>byte;
    int b = int(byte);

    return fl5Color(r,g,b,a);
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void io::readColor(QDataStream &ar, int &r, int &g, int &b)
{
    qint32 colorref;

    ar >> colorref;
    b = colorref/256/256;
    colorref -= b*256*256;
    g = colorref/256;
    r = colorref - g*256;
}


/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component

*/
void io::writeColor(QDataStream &ar, int r, int g, int b)
{
    qint32 colorref;

    colorref = b*256*256+g*256+r;
    ar << colorref;
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void io::readColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
    uchar byte=0;

    ar>>byte;//probably a format identificator
    ar>>byte>>byte;
    a = int(byte);
    ar>>byte>>byte;
    r = int(byte);
    ar>>byte>>byte;
    g = int(byte);
    ar>>byte>>byte;
    b = int(byte);
    ar>>byte>>byte; //
}

/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void io::writeColor(QDataStream &ar, int r, int g, int b, int a)
{
    uchar byte;

    byte = 1;
    ar<<byte;
    byte = a & 0xFF;
    ar << byte<<byte;
    byte = r & 0xFF;
    ar << byte<<byte;
    byte = g & 0xFF;
    ar << byte<<byte;
    byte = b & 0xFF;
    ar << byte<<byte;
    byte = 0;
    ar << byte<<byte;
}


void io::readString(QDataStream &ar, std::string &strong)
{
    std::string str;
    qint8 qi(0), ch(0);
    char c(0);

    ar >> qi;
    str.clear();
    for(int j=0; j<qi;j++)
    {
        str += " ";
        ar >> ch;
        c = char(ch);
        str[j] = c;
    }

    strong = str;
}



void io::readFloat(QDataStream &inStream, float &f)
{
    char buffer[4];
    inStream.readRawData(buffer, 4);
    memcpy(&f, buffer, sizeof(float));
}


void io::writeFloat(QDataStream &outStream, float f)
{
    char buffer[4];
    memcpy(buffer, &f, sizeof(float));
    outStream.writeRawData(buffer, 4);
}


void io::writeString(QDataStream &ar, QString const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}


void io::writeString(QDataStream &ar, std::string const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text= textline.data();;

    ar << qi;
    ar.writeRawData(text, qi);
}


bool io::saveProject(std::string const& stdPathName, std::string &logmsg)
{
    QString PathName = QString::fromStdString(stdPathName);

    QFile fp(PathName);
    if (!fp.open(QIODevice::WriteOnly))
    {
        logmsg = "Could not open the file: "+stdPathName+" for writing\n\n";
        return false;
    }

    QDataStream ar(&fp);

    FileIO saver;
    if(!saver.serializeProjectFl5(ar, true))
    {
        logmsg = "Unknown error saving the project file " + stdPathName;
        return false;
    }

    fp.close(); // or let the destructor do it

    return true;
}


bool io::loadProject(std::string const& stdPathName, std::string &logmsg)
{
    QString PathName = QString::fromStdString(stdPathName);

    QFile fp(PathName);
    if (!fp.open(QIODevice::ReadOnly))
    {
        logmsg = "Could not open the file: "+stdPathName+" for reading\n\n";
        return false;
    }

    QDataStream ar(&fp);

    FileIO loader;
    if(!loader.serializeProjectFl5(ar, false))
    {
        logmsg = "Unknown error loading the project file " + stdPathName;
        return false;
    }

    fp.close(); // or let the destructor do it

    return true;
}


bool io::readSTLFile(std::string const&FilePath, double FileUnitsToMeter, double theMergeAngle,
                     std::vector<Triangle3d> &triangles, Vector3d &botleft, Vector3d &topright)
{
    Standard_CString STLFile = FilePath.c_str();

    Handle(Poly_Triangulation) polyTriangulation = RWStl::ReadFile(STLFile, theMergeAngle); // not sure how to set the merge angle

    occ::polyTriangulationToTriangles(polyTriangulation, FileUnitsToMeter, triangles, botleft, topright);

    return true;
}


bool io::readOBJFile(std::string const&FilePath, double FileUnitsToMeter,
                     std::vector<Triangle3d> &triangles, Vector3d &botleft, Vector3d &topright)
{
    Standard_CString ObjFile = FilePath.c_str();

    Handle(Poly_Triangulation) polyTriangulation = RWObj::ReadFile(ObjFile); // not sure how to set the merge angle

    occ::polyTriangulationToTriangles(polyTriangulation, FileUnitsToMeter, triangles, botleft, topright);

    return true;
}


PlaneSTL *io::importPlaneFromMesh(std::string const&FilePath, enumMeshFileType type, double FileUnitsToMeter, std::string &logmsg)
{
    if(!std::filesystem::exists(FilePath))
    {
        logmsg += "File " + FilePath + " does not exist\n";
        return nullptr;
    }

    std::vector<Triangle3d> triangles;
    Vector3d botleft, topright;

    switch (type)
    {
        case io::STL:
            readSTLFile(FilePath, FileUnitsToMeter, 0.0, triangles, botleft, topright);
            break;
        case io::OBJ:
            readOBJFile(FilePath, FileUnitsToMeter, triangles, botleft, topright);
            break;
    }

    if(triangles.size()==0)
    {
        logmsg += "No triangles found in file\n";
        return nullptr;
    }

    PlaneSTL *pPlaneSTL = new PlaneSTL;
    pPlaneSTL->setBaseTriangles(triangles);
    pPlaneSTL->setInitialized(false);

    botleft  *= Units::mtoUnit();
    topright *= Units::mtoUnit();

    logmsg += std::format("Imported {:d} triangles\n", triangles.size());
    logmsg +=             "Bounding box limits:                     x           y           z\n";
    logmsg += std::format("                     botleft = {:11g} {:11g} {:11g} ", botleft.x,  botleft.y,  botleft.z)  + Units::lengthUnitLabel() + EOLstr;
    logmsg += std::format("                     topright= {:11g} {:11g} {:11g} ", topright.x, topright.y, topright.z) + Units::lengthUnitLabel() + EOLstr;

    return pPlaneSTL;
}


int io::exportTriMeshToSTL(QString const &pathname, double scalefactor, TriMesh const &trimesh)
{
    QFile XFile(pathname);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        return -1;
    }

    QDataStream outStream(&XFile);
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

    io::writeString(outStream, strong);

    outStream << trimesh.nPanels();

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));

    for (int it=0; it<trimesh.nPanels(); it++)
    {
        Panel3 const &p3 = trimesh.panelAt(it);
        io::writeFloat(outStream, p3.normal().xf());
        io::writeFloat(outStream, p3.normal().yf());
        io::writeFloat(outStream, p3.normal().zf());

        if(p3.isPositiveOrientation())
        {
            io::writeFloat(outStream, float(p3.vertexAt(0).x*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(0).y*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(0).z*scalefactor));

            io::writeFloat(outStream, float(p3.vertexAt(1).x*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(1).y*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(1).z*scalefactor));

            io::writeFloat(outStream, float(p3.vertexAt(2).x*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(2).y*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(2).z*scalefactor));
        }
        else
        {
            io::writeFloat(outStream, float(p3.vertexAt(0).x*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(0).y*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(0).z*scalefactor));

            io::writeFloat(outStream, float(p3.vertexAt(2).x*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(2).y*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(2).z*scalefactor));

            io::writeFloat(outStream, float(p3.vertexAt(1).x*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(1).y*scalefactor));
            io::writeFloat(outStream, float(p3.vertexAt(1).z*scalefactor));
        }

        outStream.writeRawData(buffer, 2);
    }

    XFile.close();

    return trimesh.nPanels();
}


bool io::exportMeshToSTLFile(const QString &filename, TriMesh const &trimesh, double mtounit)
{
    if(!filename.length()) return false;

    bool bBinary = true;

    if(bBinary)
    {
        exportTriMeshToSTL(filename,mtounit, trimesh);
    }
    else
    {
        //        QTextStream out(&XFile);
    }

    return true;
}


int io::exportTriangulationToSTL(QString const &pathname, double scalefactor, std::vector<Triangle3d> const &triangle)
{
    QFile XFile(pathname);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        return -1;
    }

    QDataStream outStream(&XFile);
    // stl format uses Little-Endian byte order
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

    io::writeString(outStream, strong);

    outStream << int(triangle.size()); /// @todo check STL format

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));

    for (uint it=0; it<triangle.size(); it++)
    {
        Triangle3d const & t3 = triangle.at(it);
        io::writeFloat(outStream, t3.normal().xf());
        io::writeFloat(outStream, t3.normal().yf());
        io::writeFloat(outStream, t3.normal().zf());

        io::writeFloat(outStream, float(t3.vertexAt(0).x*scalefactor));
        io::writeFloat(outStream, float(t3.vertexAt(0).y*scalefactor));
        io::writeFloat(outStream, float(t3.vertexAt(0).z*scalefactor));

        io::writeFloat(outStream, float(t3.vertexAt(1).x*scalefactor));
        io::writeFloat(outStream, float(t3.vertexAt(1).y*scalefactor));
        io::writeFloat(outStream, float(t3.vertexAt(1).z*scalefactor));

        io::writeFloat(outStream, float(t3.vertexAt(2).x*scalefactor));
        io::writeFloat(outStream, float(t3.vertexAt(2).y*scalefactor));
        io::writeFloat(outStream, float(t3.vertexAt(2).z*scalefactor));

        outStream.writeRawData(buffer, 2);
    }

    XFile.close();
    return int(triangle.size());
}


bool io::importVSPWing(QString const &filename, QVector<WingXfl*> &winglist, QString &logmsg)
{
    QFileInfo fi(filename);

    QFile VSPFile(filename);
    if(!VSPFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        logmsg += "Could not read the file - aborting\n";
        return false;
    }

    QStringList wingnames;
    QTextStream instream(&VSPFile);

    QString strVSP = instream.readAll(); // need a copy to parse multiple times
    VSPFile.close();

    QTextStream stream(&strVSP);
    QString strange;
    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Geom Name", Qt::CaseSensitive))
        {
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                if(!wingnames.contains(fields.at(1)))
                    wingnames.append(fields.at(1));
            }
        }

    }while (!strange.isNull());


    logmsg += QString::asprintf("Found %d wings:\n", int(wingnames.size()));
    for(int i=0; i<wingnames.size(); i++)
    {
        logmsg += QString("   ")+wingnames.at(i)+"\n";
    }
    /*    QVector<Wing*> wings;
    for(int i=0; i<wingnames.size(); i++)
    {
        wings.append(new WingXfl(xfl::OtherWing));
        wings.back()->setName(wingnames.at(i));
    }
*/

    //read all the foil names
    stream.seek(0);
    QStringList airfoilfilenames;
    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Airfoil File Name", Qt::CaseSensitive))
        {
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                if(!airfoilfilenames.contains(fields.at(1)))
                    airfoilfilenames.append(fields.at(1));
            }
        }

    }while (!strange.isNull());

    logmsg += QString::asprintf("Found %d airfoil files to load:\n", int(airfoilfilenames.size()));
    for(int i=0; i<airfoilfilenames.size(); i++)
    {
        logmsg += QString("   ")+airfoilfilenames.at(i)+"\n";
    }


    QStringList filter = {"*.dat"};
    QStringList files = io::findFiles(fi.absolutePath(), filter, false);
    for(int i=0; i<files.size(); i++)
    {
        Foil *pFoil = new Foil;
        if(io::readVSPFoilFile(files.at(i), pFoil))
            Objects2d::insertThisFoil(pFoil);
        else
            delete  pFoil;
    }

    /*    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("########################################", Qt::CaseSensitive))
        {
            QString wingname;
            int index(0);
            WingSection *ws = new WingSection;
            readVSPSection(stream, wingname, index, ws);
        }

    }while (!strange.isNull());*/
    return true;
}


bool io::readVSPFoilFile(QString const &FoilFileName, Foil *pFoil)
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
    pFoil->setName(FoilName.toStdString());

    std::vector<Node2d> basenodes;

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
                basenodes.push_back({x,y});

                xp = x;
                yp = y;
            }
        }
        else bRead = false;

    }while (bRead && !strong.isNull());

    xFoilFile.close();

    /*    pFoil->m_Node.resize(pFoil->nBaseNodes());
    for(int i=0; i<pFoil->nBaseNodes(); i++)
    {
        pFoil->m_Node[i].x = pFoil->xb(i);
        pFoil->m_Node[i].y = pFoil->yb(i);
    }*/

    pFoil->setBaseNodes(basenodes);

    pFoil->initGeometry();
    return true;
}


void io::readVSPSection(QTextStream &stream, QString &wingname, int &index, WingSection &ws)
{
    QString strange;
    do
    {
        strange = stream.readLine();
        QStringList fields = strange.split(",");
        if(fields.count()>2)
        {
            if     (fields.front().contains("Geom Name"))          wingname = fields.at(1);
            else if(fields.front().contains("Airfoil Index"))      index = fields.at(1).toInt();
            else if(fields.front().contains("Leading Edge Point") && fields.size()==4)
            {
                //                ws.setOffset(fields.at(1).toDouble(), fields.at(1).toDouble(), fields.at(3).toDouble());
            }
            else if(fields.front().contains("Airfoil File Name"))
            {
                ws.setLeftFoilName(fields.at(1).toStdString());
                ws.setRightFoilName(fields.at(1).toStdString());
            }
        }
    }
    while(!strange.contains("#######"));
}


bool io::exportAllStlMesh(QString const &pathname)
{
    for(int p=0; p<Objects3d::nPlanes(); p++)
    {
        Plane const *pPlane = Objects3d::planeAt(p);
        if(pPlane && pPlane->isXflType())
        {
            PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

            QString fileName = QString::fromStdString(pPlaneXfl->name())+".stl";
            fileName.replace("/", "_");
            fileName = pathname + QDir::separator() +fileName;

            if(io::exportTriMeshToSTL(fileName, 1.0, pPlaneXfl->triMesh())<0) return false;
        }
    }
    return true;
}


bool io::exportAllPolars(QString const &pathname, xfl::enumTextFileType fileType)
{
    QFile XFile;
    QTextStream out(&XFile);
    QString fileName;

    bool bCSV = fileType==xfl::CSV;

    for(int l=0; l<Objects2d::nPolars(); l++)
    {
        Polar const *pPolar = Objects2d::polarAt(l);
        Foil  const *pFoil  = Objects2d::foil(pPolar->foilName());
        if(!pFoil) continue;

        QString FoilSubDirPath = pathname + QDir::separator() + QString::fromStdString(pFoil->name());
        QDir ExportFoilDir(FoilSubDirPath);
        if(!ExportFoilDir.exists())
        {
            if(!ExportFoilDir.mkpath(FoilSubDirPath))  continue;
        }

        fileName = QString::fromStdString(pPolar->name());
        if(fileType==xfl::TXT) fileName += ".txt";
        else                   fileName += ".csv";

        XFile.setFileName(ExportFoilDir.absolutePath() + QDir::separator() + fileName);

        if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            std::string str;
            pPolar->exportToString(str, false, bCSV);
            out << QString::fromStdString(str);
            XFile.close();
        }
        else return false;
    }
    return true;
}


bool io::exportAllWPolars(QString const &pathname, bool bCSV)
{
    QFile XFile;
    QTextStream out(&XFile);
    QString fileName;

    xfl::enumTextFileType fileType = bCSV? xfl::CSV : xfl::TXT;

    for(int p=0; p<Objects3d::nPlanes(); p++)
    {
        Plane const *pPlane = Objects3d::planeAt(p);
        if(pPlane && pPlane->isXflType())
        {
            PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            QString PlaneSubDirPath = pathname + QDir::separator() + QString::fromStdString(pPlaneXfl->name());
            QDir ExportPlaneDir(PlaneSubDirPath);
            if(!ExportPlaneDir.exists())
            {
                if(!ExportPlaneDir.mkpath(PlaneSubDirPath))
                {
                    return false;
                }
            }

            for(int l=0; l<Objects3d::nPolars(); l++)
            {
                PlanePolar const *pWPolar = Objects3d::plPolarAt(l);
                if(!pWPolar) continue;
                if(pWPolar->planeName().compare(pPlaneXfl->name())!=0) continue;

                fileName = QString::fromStdString(pWPolar->name());
                fileName.replace("/", "_");
                fileName.replace(".", "_");
                fileName = PlaneSubDirPath + QDir::separator() +fileName;
                if(fileType==xfl::TXT) fileName += ".txt";
                else                   fileName += ".csv";

                XFile.setFileName(fileName);
                if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    out.setDevice(&XFile);
                    std::string props;

                    QString lenlab, arealab, masslab, speedlab;
                    lenlab = Units::lengthUnitQLabel();
                    arealab = Units::areaUnitQLabel();
                    masslab = Units::massUnitQLabel();
                    speedlab = Units::speedUnitQLabel();

                    pWPolar->getProperties(props, pPlane);
                    out << QString::fromStdString(props);

                    std::string sep = "  ";
                    if(bCSV) sep = xfl::textSeparator()+ " ";
                    std::string exportdata = pWPolar->exportToString(sep);
                    out << QString::fromStdString(exportdata);
                    XFile.close();
                }
                else return false;
            }
        }
    }
    return true;
}


bool io::exportAllBtPolars(QString const &pathname, bool bCSV)
{
    QFile XFile;
    QTextStream out(&XFile);
    QString fileName;

    xfl::enumTextFileType fileType = bCSV? xfl::CSV : xfl::TXT;

    for(int p=0; p<SailObjects::nBoats(); p++)
    {
        Boat const *pBoat = SailObjects::boat(p);

        QString BoatSubDirPath = pathname + QDir::separator() + QString::fromStdString(pBoat->name());
        QDir ExportBoatDir(BoatSubDirPath);
        if(!ExportBoatDir.exists())
        {
            if(!ExportBoatDir.mkpath(BoatSubDirPath))
            {
                return false;
            }
        }

        for(int l=0; l<SailObjects::nBtPolars(); l++)
        {
            BoatPolar const *pBtPolar = SailObjects::btPolar(l);
            if(!pBtPolar) continue;
            if(pBtPolar->boatName().compare(pBoat->name())!=0) continue;

            fileName = QString::fromStdString(pBtPolar->name());
            fileName.replace("/", "_");
            fileName.replace(".", "_");
            fileName = BoatSubDirPath + QDir::separator() +fileName;
            if(fileType==xfl::TXT) fileName += ".txt";
            else                   fileName += ".csv";

            XFile.setFileName(fileName);
            if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                out.setDevice(&XFile);
                std::string props;

                pBtPolar->getProperties(props, fileType);
                out << QString::fromStdString(props);

                std::string sep = "  ";
                if(xfl::exportFileType()==xfl::CSV) sep = xfl::textSeparator()+ " ";
                std::string data;
                pBtPolar->getBtPolarData(data, sep);
                out << QString::fromStdString(data);
                XFile.close();
            }
            else return false;
        }
    }
    return true;
}


bool io::exportAllBtOpps(const QString &pathname, bool bCSV, bool bPanelData)
{
    QFile XFile;
    QTextStream out(&XFile);
    QString fileName;

    xfl::enumTextFileType fileType = bCSV? xfl::CSV : xfl::TXT;

    for(int p=0; p<SailObjects::nBoats(); p++)
    {
        Boat const *pBoat = SailObjects::boat(p);
        if(pBoat)
        {
            QString BoatSubDirPath = pathname + QDir::separator() + QString::fromStdString(pBoat->name());
            QDir ExportBoatDir(BoatSubDirPath);
            if(!ExportBoatDir.exists())
            {
                if(!ExportBoatDir.mkpath(BoatSubDirPath))
                {
                    return false;
                }
            }

            for(int l=0; l<SailObjects::nBtPolars(); l++)
            {
                BoatPolar const *pBtPolar = SailObjects::btPolar(l);
                if(!pBtPolar) continue;
                if(pBtPolar->boatName().compare(pBoat->name())!=0) continue;
                QString PolarName = QString::fromStdString(pBtPolar->name());
                PolarName.replace("/", "_");
                PolarName.replace(".", "_");
                QString BtPolarSubDirPath = BoatSubDirPath + QDir::separator() + PolarName;
                QDir ExportWPolarDir(BtPolarSubDirPath);
                if(!ExportWPolarDir.exists())
                {
                    if(!ExportWPolarDir.mkpath(BtPolarSubDirPath))
                    {
                        return false;
                    }
                }
                for(int k=0; k<SailObjects::nBtOpps(); k++)
                {
                    BoatOpp const *pBtOpp = SailObjects::btOpp(k);
                    fileName = QString::fromStdString(pBtOpp->title(false));
                    fileName.replace("/", "_");
                    fileName.replace(".", "_");
                    fileName = BtPolarSubDirPath + QDir::separator() +fileName;
                    if(fileType==xfl::TXT) fileName += ".txt";
                    else                   fileName += ".csv";

                    XFile.setFileName(fileName);
                    if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
                    {
                        out.setDevice(&XFile);
                        std::string props;

                        pBtOpp->exportMainDataToString(pBoat, props, xfl::exportFileType(), xfl::textSeparator());
                        out << QString::fromStdString(props);
                        if(bPanelData)
                        {
                            props.clear();
                            if(pBtOpp->isTriangleMethod())
                                pBtOpp->exportPanel3DataToString(pBoat, xfl::exportFileType(), props);
                            out << QString::fromStdString(props);
                        }
                        XFile.close();
                    }
                }
            }
        }
    }
    return true;
}


bool io::exportAllPOpps(const QString &pathname, bool bCSV, bool bPanelData)
{
    QFile XFile;
    QTextStream out(&XFile);
    QString fileName;

    xfl::enumTextFileType fileType = bCSV? xfl::CSV : xfl::TXT;

    for(int p=0; p<Objects3d::nPlanes(); p++)
    {
        Plane const *pPlane = Objects3d::planeAt(p);
        if(pPlane && pPlane->isXflType())
        {
            PlaneXfl const* pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            QString PlaneSubDirPath = pathname + QDir::separator() + QString::fromStdString(pPlaneXfl->name());
            QDir ExportPlaneDir(PlaneSubDirPath);
            if(!ExportPlaneDir.exists())
            {
                if(!ExportPlaneDir.mkpath(PlaneSubDirPath))
                {
                    return false;
                }
            }

            for(int l=0; l<Objects3d::nPolars(); l++)
            {
                PlanePolar const *pWPolar = Objects3d::plPolarAt(l);
                if(!pWPolar) continue;
                if(pWPolar->planeName().compare(pPlaneXfl->name())!=0) continue;
                QString PolarName = QString::fromStdString(pWPolar->name());
                PolarName.replace("/", "_");
                PolarName.replace(".", "_");
                QString WPolarSubDirPath = PlaneSubDirPath + QDir::separator() + PolarName;
                QDir ExportWPolarDir(WPolarSubDirPath);
                if(!ExportWPolarDir.exists())
                {
                    if(!ExportWPolarDir.mkpath(WPolarSubDirPath))
                    {
                        return false;
                    }
                }
                for(int k=0; k<Objects3d::nPOpps(); k++)
                {
                    PlaneOpp const *pPOpp = Objects3d::POppAt(k);
                    fileName = QString::fromStdString(pPOpp->title(false));
                    fileName.replace("/", "_");
                    fileName.replace(".", "_");
                    fileName = WPolarSubDirPath + QDir::separator() +fileName;
                    if(fileType==xfl::TXT) fileName += ".txt";
                    else                   fileName += ".csv";

                    XFile.setFileName(fileName);
                    if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
                    {
                        out.setDevice(&XFile);
                        std::string props;

                        pPOpp->exportMainDataToString(pPlane, props, xfl::exportFileType(), xfl::textSeparator());
                        out << QString::fromStdString(props);
                        if(bPanelData)
                        {
                            props.clear();
                            if(pPOpp->isQuadMethod())
                                pPOpp->exportPanel4DataToString(pPlaneXfl, pWPolar, xfl::exportFileType(), props);
                            else if(pPOpp->isTriangleMethod())
                                pPOpp->exportPanel3DataToString(pPlaneXfl, pWPolar, xfl::exportFileType(), xfl::textSeparator(), props);
                            out << QString::fromStdString(props);
                        }
                        XFile.close();
                    }
                }
            }
        }
    }
    return true;
}



bool io::writeFoilPolars(QString const &pathname, Foil *pFoil)
{
    QFile XFile(pathname);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QDataStream ar(&XFile);
    ar.setVersion(QDataStream::Qt_4_5);
    ar.setByteOrder(QDataStream::LittleEndian);


    // 100003 : added foil comment
    // 100002 : means we are serializings opps in the new numbered format
    // 100001 : transferred NCrit, XTopTr, XBotTr to polar file
    // 500001 : v7 format
    int ArchiveFormat = 500001;
    ar << ArchiveFormat;

    //first write the foil
    ar << 1; //only one foil to write
    serial::serializeFoilFl5(pFoil, ar,true);

    //count polars associated to the foil
    Polar * pPolar =nullptr;
    int n=0;
    for (int i=0; i<Objects2d::nPolars();i++)
    {
        pPolar = Objects2d::polarAt(i);
        if (pPolar->foilName() == pFoil->name()) n++;
    }

    //then write the polars
    ar << n;
    for (int i=0; i<Objects2d::nPolars();i++)
    {
        pPolar = Objects2d::polarAt(i);
        if (pPolar->foilName() == pFoil->name())
            serial::serializePolarFl5(pPolar, ar, true);
    }

    XFile.close();
    return true;
}


PlaneXfl *io::importPlaneFromXML(std::string const &xmlfilepath, std::string &logmsg)
{
    QFile xmlFile(QString::fromStdString(xmlfilepath));
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        logmsg = "Could not open the file "+xmlfilepath+"\n";
        return nullptr;
    }

    XmlPlaneReader planereader(xmlFile);
    if(!planereader.readFile())
    {
        logmsg = "Failed to read the file "+xmlfilepath+"\n";
        logmsg += std::format("   error at line {:d} column {:d}\n",int(planereader.lineNumber()),int(planereader.columnNumber()));

        xmlFile.close();
        return nullptr;
    }

    PlaneXfl *pPlaneXfl  = planereader.plane();
    if(!pPlaneXfl)
    {
        logmsg = "No plane definition found in the file " + xmlfilepath +"\n";
        xmlFile.close();
        return nullptr;
    }

    logmsg = "The plane "+ pPlaneXfl->name()+" has been imported successfully\n";

    xmlFile.close();
    return pPlaneXfl;
}


PlanePolar* io::importAnalysisFromXML(std::string const &xmlfilepath, std::string &logmsg)
{
    QFile xmlFile(QString::fromStdString(xmlfilepath));
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        logmsg = "Could not open the file " + xmlfilepath +"\n";
        return nullptr;
    }

    QFileInfo fi(xmlFile);

    XmlPlanePolarReader wpolarreader(xmlFile);
    wpolarreader.readXMLPolarFile();

    if(wpolarreader.hasError())
    {
        logmsg = wpolarreader.errorString().toStdString() + std::format("\nline {:d} column {:d}", wpolarreader.lineNumber(), wpolarreader.columnNumber());
        logmsg +="\n";
        xmlFile.close();
        return nullptr;
    }
    PlanePolar *pWPolar = wpolarreader.wpolar();

    Plane *pPlane = Objects3d::plane(pWPolar->planeName());
    if(!pPlane)
    {
        logmsg = "leaving plane name blank...\n";
    }

    if(pWPolar->name().length()==0)
        pWPolar->setName(PlanePolarNameMaker::makeName(pPlane, pWPolar));

    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
    if(pPlaneXfl)
    {
        for(int ie=0; ie<pWPolar->nAVLCtrls(); ie++)
            pWPolar->AVLCtrl(ie).resizeValues(pPlaneXfl->nAVLGains());
    }

    xmlFile.close();
    return pWPolar;
}

