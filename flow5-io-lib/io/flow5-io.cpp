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
#include <BRepTools.hxx>
#include <STEPControl_Writer.hxx>
#include <BRepBuilderAPI_Transform.hxx>

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
#include <fuseocc.h>
#include <fusestl.h>
#include <gmesh_globals.h>
#include <objects2d.h>
#include <objects2d_globals.h>
#include <objects3d.h>
#include <occ_globals.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planepolarnamemaker.h>
#include <planestl.h>
#include <planexfl.h>
#include <sailobjects.h>
#include <sailwing.h>
#include <serialization.h>
#include <triangle3d.h>
#include <trimesh.h>
#include <units.h>
#include <utils-io.h>
#include <utils.h>
#include <vector3d.h>
#include <xmlplanepolarreader.h>
#include <xmlplanereader.h>


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


FuseOcc* io::importFuseFromSTEP(std::string const&FilePath, std::string &logmsg)
{
    if(!std::filesystem::exists(FilePath))
    {
        logmsg += "File " + FilePath + " does not exist\n";
        return nullptr;
    }

    std::string str;
    FuseOcc *pFuseOcc = new FuseOcc;
    pFuseOcc->setName(FilePath);
    double refdimension(0); // some kind of reference dimension extracted from the STEP file
    TopoDS_ListOfShape shapes;
    bool bImport = occ::importCADShapes(FilePath, shapes, refdimension, str);

    logmsg += str+"\n";

    if(!bImport)
    {
        delete pFuseOcc;
        logmsg += "Error importing CAD file:"+FilePath+ EOLstr;
        return nullptr;
    }

    pFuseOcc->setShapes(shapes);

    // Test whether the imported Shapes contain shells ready to mesh
    logmsg += "Extracting shells from imported shapes\n";
    pFuseOcc->extractShellsFromShapes();

    if(pFuseOcc->shellCount()==0)
    {
        logmsg += "Warning: Imported LIST_OF_SHAPES does not contain any SHELL.\n"
                  "         Use shape healing methods to build one.\n";
    }
    else
    {
        logmsg += "---Making shell triangulation-----\n";
        gmesh::makeFuseTriangulation(pFuseOcc, logmsg, "   ");

        pFuseOcc->computeSurfaceProperties(str, "   ");
    }

    return pFuseOcc;
}


FuseStl* io::importFuseFromMesh(std::string const&FilePath, enumMeshFileType type, double FileUnitsToMeter, std::string &logmsg)
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

    FuseStl *pFuseStl = new FuseStl;
    pFuseStl->setTriangles(triangles);

    botleft  *= Units::mtoUnit();
    topright *= Units::mtoUnit();

    logmsg += std::format("Imported {:d} triangles\n", triangles.size());
    logmsg +=             "Bounding box limits:                     x           y           z\n";
    logmsg += std::format("                     botleft = {:11g} {:11g} {:11g} ", botleft.x,  botleft.y,  botleft.z)  + Units::lengthUnitLabel() + EOLstr;
    logmsg += std::format("                     topright= {:11g} {:11g} {:11g} ", topright.x, topright.y, topright.z) + Units::lengthUnitLabel() + EOLstr;

    return pFuseStl;
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


int io::exportTriangulationToSTLBinary(QString const &pathname, double scalefactor, std::vector<Triangle3d> const &triangle)
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


int io::exportTriangulationToSTLText(QString const &pathname, double scalefactor, std::vector<Triangle3d> const &triangles)
{
    QFile XFile(pathname);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        return -1;
    }

    QTextStream outStream(&XFile);

    /***
        facet normal ni nj nk
            outer loop
                vertex v1x v1y v1z
                vertex v2x v2y v2z
                vertex v3x v3y v3z
            endloop
        endfacet
    */


    QString strong =     "solid flow5 model\n";
    outStream << strong;

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));


    for (unsigned int it=0; it<triangles.size(); it++)
    {
        Triangle3d const & t3 = triangles.at(it);

        QString facet = QString::asprintf("facet   %13g   %13g   %13g\n", t3.normal().xf(), t3.normal().yf(), t3.normal().zf());
        outStream << facet;
        outStream << "    outer loop\n";

        outStream << QString::asprintf("        vertex %13g %13g %13g\n",
                                        float(t3.vertexAt(0).x*scalefactor), float(t3.vertexAt(0).y*scalefactor), float(t3.vertexAt(0).z*scalefactor));

        outStream << QString::asprintf("        vertex %13g %13g %13g\n",
                                        float(t3.vertexAt(1).x*scalefactor), float(t3.vertexAt(1).y*scalefactor), float(t3.vertexAt(1).z*scalefactor));

        outStream << QString::asprintf("        vertex %13g %13g %13g\n",
                                        float(t3.vertexAt(2).x*scalefactor), float(t3.vertexAt(2).y*scalefactor), float(t3.vertexAt(2).z*scalefactor));

        outStream << "    endloop\n";
        outStream << "endfacet\n";

    }

    XFile.close();
    return int(triangles.size());
}


bool io::importVSPWings(QString const &filename, std::vector<WingXfl*> &winglist, QString &logmsg)
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
                QString name = fields.at(1).trimmed();
                if(!wingnames.contains(name))
                    wingnames.append(name);
            }
        }

    }while (!strange.isNull());


    logmsg += QString::asprintf("Found %d wings:\n", int(wingnames.size()));
    for(int i=0; i<wingnames.size(); i++)
    {
        logmsg += QString("   ")+wingnames.at(i)+"\n";
    }

    winglist.clear();
    for(unsigned int i=0; i<wingnames.size(); i++)
    {
        WingXfl*pWing = new WingXfl;
        pWing->setName(wingnames.at(i).toStdString());
        switch (i)
        {
            case 0:  pWing->setWingType(xfl::Main);      break;
            case 1:  pWing->setWingType(xfl::Elevator);  break;
            case 2:  pWing->setWingType(xfl::Fin);       break;
            default: pWing->setWingType(xfl::OtherWing); break;
        }

        winglist.push_back(pWing);
    }


    // read the section data
    // format assumptions:
    //    sections written for right half wing i.e. y>0
    //    sections are read in order from root to tip
    //    LE.y==TE.y at each station

    stream.seek(0);
    QStringList airfoilfilenames;
    std::string foilname;
    int nx(7), ny(1);

    do
    {
        strange = stream.readLine();
        if(strange.isNull()) break;
        if(strange.contains("Airfoil File Name", Qt::CaseSensitive))
        {
            // Airfoil File Name is used as start marker for the section
            QStringList fields = strange.split(",");
            if(fields.count()>=2)
            {
                QString FoilName = fields.at(1).trimmed();
                int pos = FoilName.lastIndexOf(".dat");
                FoilName.truncate(pos);
                foilname = FoilName.toStdString();
                if(!airfoilfilenames.contains(FoilName))  airfoilfilenames.append(FoilName);
            }

            // Continue reading the section data
            // section is considered valid if the following fields have been read:
            //     wingname
            //     LE and TE
            //     airfoil file name
            WingXfl *pWingXfl(nullptr);
            std::string wingname;

            Vector3d LE, TE;
            double chord(0);
            bool bLE(false), bTE(false);
            do
            {
                strange = stream.readLine();
                if (strange.contains("Geom Name"))
                {
                    fields = strange.split(",");
                    wingname = fields.at(1).trimmed().toStdString();
                    pWingXfl = nullptr;
                    for(unsigned int j=0; j<winglist.size(); j++)
                    {
                        if(winglist.at(j)->name().compare(wingname)==0)
                        {
                            pWingXfl = winglist.at(j);
                        }
                    }
                }
                else if (strange.contains("Leading Edge Point"))
                {
                    fields = strange.split(",");
                    if(fields.length()==4)
                    {
                        LE.x = fields.at(1).toDouble();
                        LE.y = fields.at(2).toDouble();
                        LE.z = fields.at(3).toDouble();
                        bLE = true;
                    }
                }
                else if (strange.contains("Trailing Edge Point"))
                {
                    fields = strange.split(",");
                    if(fields.length()==4)
                    {
                        TE.x = fields.at(1).toDouble();
                        TE.y = fields.at(2).toDouble();
                        TE.z = fields.at(3).toDouble();
                        bTE = true;
                    }
                }
                else if (strange.contains("Chord"))
                {
                    fields = strange.split(",");
                    if(fields.length()==2)
                    {
                        chord = fields.at(1).toDouble();
                    }
                }
            }
            while(!strange.contains("########")); // assuming this closes the section

            if(bLE && bTE && pWingXfl)
            {
//                double length = LE.distanceTo(TE); // same as chord?

                double twist = atan2(TE.z-LE.z, TE.x-LE.x) * 180.0 / PI;
                WingSection sec;// = WingSection(chord, twist, yPos, dihedral, xOffset, nx, ny, xfl::TANH, xfl::UNIFORM, foilname, foilname);
                sec.setChord(chord);
                sec.setTwist(twist);
                sec.setYPosition(LE.y); // recalculated at the next step
                sec.setzPos(LE.z); // temporary
                sec.setDihedral(0.0);   // recalculated at the next step
                sec.setXOffset(LE.x);   // recalculated at the next step
                sec.setNX(nx);
                sec.setNY(ny);
                sec.setXDistType(xfl::TANH);
                sec.setYDistType(xfl::UNIFORM);
                sec.setRightFoilName(foilname);
                sec.setLeftFoilName(foilname);

                pWingXfl->appendSection(sec);

            }
        }

    }while (!strange.isNull());

    // recalculate dihedrals and y positions
    for(WingXfl *pWing : winglist)
    {
        // the wing's position is defined by its first section
/*        WingSection const & root = pWing->rootSection();
        Vector3d WingPosition = Vector3d(root.offset(), root.yPosition(), root.zPos());

        for(int iws=0; iws<pWing->nSections(); iws++)
        {
            pWing->
        }*/

        for(int iws=0; iws<pWing->nSections()-1; iws++)
        {
            WingSection       &seci  = pWing->section(iws);
            WingSection const &seci1 = pWing->section(iws+1);
            double dihedral = atan2(seci1.zPos()-seci.zPos(), seci1.yPosition()-seci.yPosition()) * 180.0/PI;
            seci.setDihedral(dihedral);
            seci.setYPosition(seci.yPosition()/cos(dihedral*PI/180.0)); // confirm to planform
        }


    }
    // translate the wing



    logmsg += QString::asprintf("Found %d airfoils to load from file:\n", int(airfoilfilenames.size()));
//    for(int i=0; i<airfoilfilenames.size(); i++) logmsg += QString("   ")+airfoilfilenames.at(i)+"\n";


    QStringList filter = {"*.dat"};
    QStringList files = io::findFiles(fi.absolutePath(), filter, false);
    fl5Color clr = xfl::Orchid;
    int iError(0);
    for(QString file : files)
    {

        Foil *pFoil = new Foil;
        if(foil::readFoilFile(file.toStdString(), pFoil, iError))
        {
            QString FoilName = QString::fromStdString(pFoil->name());

            int pos = FoilName.length()-FoilName.lastIndexOf("/");
            FoilName = FoilName.right(pos-1);
            pos = FoilName.lastIndexOf(".dat");
            FoilName.truncate(pos);
            pFoil->setName(FoilName.toStdString());

            pFoil->setLineColor(clr);
            clr = clr.darker(105);
            Objects2d::insertThisFoil(pFoil);
        }
        else
            delete  pFoil;
    }

    return true;
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

/*                    QString lenlab, arealab, masslab, speedlab;
                    lenlab = Units::lengthUnitQLabel();
                    arealab = Units::areaUnitQLabel();
                    masslab = Units::massUnitQLabel();
                    speedlab = Units::speedUnitQLabel();
*/
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


bool io::readPolarFile(QFile &plrFile, std::vector<Foil*> &foilList, std::vector<Polar*> &polarList)
{
    Foil* pFoil(nullptr);
    Polar *pPolar(nullptr);
    Polar *pOldPolar(nullptr);
    int n(0);

    QDataStream ar(&plrFile);
    ar.setVersion(QDataStream::Qt_4_5);
    ar.setByteOrder(QDataStream::LittleEndian);

    ar >> n;

    if(n<100000)
    {
        // deprecated format
        return false;
    }
    else if (n >=100000 && n<200000)
    {
        //new format XFLR5 v1.99+
        //first read all available foils
        ar>>n;
        for (int i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!serial::serializeFoil(pFoil, ar))
            {
                delete pFoil;
                return false;
            }
            foilList.push_back(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0; i<n; i++)
        {
            pPolar = new Polar();

            if (!serial::serializePolarv6(pPolar, ar, false))
            {
                delete pPolar;
                return false;
            }
            for (uint l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);
                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.erase(polarList.begin()+l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.push_back(pPolar);
        }
    }
    else if (n >=500000 && n<600000)
    {
        // v7 format
        // number of foils to read
        ar>>n;
        for (int i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!serial::serializeFoilFl5(pFoil, ar, false))
            {
                delete pFoil;
                return false;
            }
            foilList.push_back(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0;i<n; i++)
        {
            pPolar = new Polar();

            if (!serial::serializePolarFl5(pPolar, ar, false))
            {
                delete pPolar;
                return false;
            }

            for (uint l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);

                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.erase(polarList.begin()+l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.push_back(pPolar);
        }
    }
    return true;
}


bool io::saveBoatAsProject(Boat *pBoat, QString const &pathname)
{
    QString strong;

    QFile fp(pathname);

    if (!fp.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream ar(&fp);
    FileIO saver;

    saver.serializeProjectMetaDataFl5(ar, true);

    std::vector<Foil*> FoilList;
    for(int is=0; is<pBoat->nSails(); is++)
    {
        SailWing const *pWS = dynamic_cast<SailWing const*>(pBoat->sailAt(is));
        if(pWS)
        {
            for(int is=0; is<pWS->sectionCount(); is++)
            {
                WingSailSection const &ws = pWS->sectionAt(is);
                Foil *pFoil = Objects2d::foil(ws.foilName());
                if(pFoil)
                {
                    if(std::find(FoilList.begin(), FoilList.end(), pFoil)==FoilList.end())
                        FoilList.push_back(pFoil);
                }
            }
        }
    }

    saver.storeFoilsFl5(FoilList, ar, true);

    ar << 0; //planes
    ar << 0; //wpolars
    ar << 0; //wpolars external
    ar << 0; //popps

    ar << 500001;
    // save the Boats...
    ar << 1;
    serial::serializeBoatFl5(pBoat, ar, true);

    // save the BtPolars
    int polarcount = 0;
    for(int i=0; i<SailObjects::nBtPolars(); i++)
    {
        if(SailObjects::btPolar(i)->boatName()==pBoat->name()) polarcount++;
    }
    ar << polarcount;
    for (int i=0; i<polarcount;i++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(i);
        if(pBtPolar->boatName()==pBoat->name()) serial::serializeBoatPolarFl5v750(pBtPolar, ar, true);
    }

    // not forgetting their BtOpps
    int btoppcount = 0;
    for(int i=0; i<SailObjects::nBtOpps(); i++)
    {
        if(SailObjects::btOpp(i)->boatName()==pBoat->name()) btoppcount++;
    }
    ar << btoppcount;
    for (int i=0; i<btoppcount; i++)
    {
        BoatOpp *pBOpp = SailObjects::btOpp(i);
        if(pBOpp->boatName()==pBoat->name()) serial::serializeBoatOppFl5(pBOpp, ar, true);
    }

    // dynamic space allocation for the future storage of more data, without need to change the format
    int nIntSpares=0;
    ar << nIntSpares;
    int n=0;
    for (int i=0; i<nIntSpares; i++) ar << n;
    int nDbleSpares=0;
    double dble=0.0;
    ar << nDbleSpares;
    for (int i=0; i<nDbleSpares; i++) ar << dble;

    fp.close();
    return true;
}



Polar *io::importXFoilPolar(QFile &txtFile, QString &logmsg)
{
    double Re(0), alpha(0), CL(0), CD(0), CDp(0), CM(0), Xt(0), Xb(0),Cpmn(0), HMom(0);
    QString FoilName;
    QString strong, strange, str;
    bool bRead = false;

    if (!txtFile.open(QIODevice::ReadOnly))
    {
        strange = "Could not open the file "+txtFile.fileName();
        logmsg += strange;
        return nullptr;
    }
    Polar *pPolar = new Polar;

    QTextStream in(&txtFile);
    int Line = 0;
    bool bOK=false, bOK2=false;

    io::readAVLString(in, Line, strong);    // XFoil or XFLR5 version
    io::readAVLString(in, Line, strong);    // Foil Name

    FoilName = strong.right(strong.length()-22).trimmed();
//    FoilName = FoilName.trimmed();

    pPolar->setFoilName(FoilName.toStdString());

    io::readAVLString(in, Line, strong);// analysis type

    int retype = strong.mid(0,2).toInt(&bOK);
    if(bOK) pPolar->setReType(retype);
    int matype = strong.mid(2,2).toInt(&bOK2);
    if(bOK) pPolar->setMaType(matype);

    if(!bOK || !bOK2)
    {
        str = QString::asprintf("Error reading line %d: Unrecognized Mach and Reynolds type.\nThe polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";

        return nullptr;
    }
    if     (pPolar->ReType() ==1 && pPolar->MaType() ==1) pPolar->setType(xfl::T1POLAR);
    else if(pPolar->ReType() ==2 && pPolar->MaType() ==2) pPolar->setType(xfl::T2POLAR);
    else if(pPolar->ReType() ==3 && pPolar->MaType() ==1) pPolar->setType(xfl::T3POLAR);
    else                                                  pPolar->setType(xfl::T1POLAR);

    bRead = io::readAVLString(in, Line, strong);
    if(!bRead || strong.length() < 34)
    {
        str = QString::asprintf("Error reading line %d. The polar(s) will not be stored.",Line);
        delete pPolar;

        logmsg += str+"\n";
        return nullptr;
    }

    double xtr = strong.mid(9,6).toDouble(&bOK);
    if(bOK) pPolar->setXTripBot(xtr);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Bottom Transition value at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    xtr = strong.mid(28,6).toDouble(&bOK);
    if(bOK) pPolar->setXTripTop(xtr);

    if(!bOK)
    {
        str = QString::asprintf("Error reading Top Transition value at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;

        logmsg += str+"\n";
        return nullptr;
    }

    // Mach     Re     NCrit
    bRead = io::readAVLString(in, Line, strong);// blank line
    if(!bRead || strong.length() < 50)
    {
        str = QString::asprintf("Error reading line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    double Ma = strong.mid(8,6).toDouble(&bOK);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Mach Number at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }
    else
        pPolar->setMach(Ma);

    Re = strong.mid(24,10).toDouble(&bOK);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Reynolds Number at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }
    Re *=1000000.0;
    pPolar->setReynolds(Re);

    double ncrit = strong.mid(52,8).toDouble(&bOK);
    if(bOK) pPolar->setNCrit(ncrit);
    if(!bOK)
    {
        str = QString::asprintf("Error reading NCrit at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    io::readAVLString(in, Line, strong);// column titles
    bRead = io::readAVLString(in, Line, strong);// underscores


    while(bRead && !in.atEnd())
    {
        bRead = io::readAVLString(in, Line, strong);// polar data
        if(strong.length())
        {
            if(strong.length())
            {
                //                textline = strong.toLatin1();
                //                text = textline.constData();
                //                res = sscanf(text, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &alpha, &CL, &CD, &CDp, &CM, &Xt, &Xb, &Cpmn, &HMom);

                //Do this the Qt way
                QStringList values;
#if QT_VERSION >= 0x050F00
                values = strong.split(" ", Qt::SkipEmptyParts);
#else
                values = strong.split(" ", QString::SkipEmptyParts);
#endif

                if(values.length()>=7)
                {
                    alpha  = values.at(0).toDouble();
                    CL     = values.at(1).toDouble();
                    CD     = values.at(2).toDouble();
                    CDp    = values.at(3).toDouble();
                    CM     = values.at(4).toDouble();
                    Xt     = values.at(5).toDouble();
                    Xb     = values.at(6).toDouble();

                    if(values.length() >= 9)
                    {
                        Cpmn    = values.at(7).toDouble();
                        HMom    = values.at(8).toDouble();
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, Cpmn, HMom, Re, 0, 0, Xt, Xb, 0, 0, 0, 0);
                    }
                    else
                    {
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, 0.0, 0.0,Re,0.0,0.0, Xt, Xb, 0, 0, 0, 0);

                    }
                }
            }
        }
    }
    txtFile.close();

    Re = pPolar->Reynolds()/1000000.0;
    QString name = QString("T%1_Re%2_M%3")
            .arg(pPolar->type()+1)
            .arg(Re,0,'f',2)
            .arg(pPolar->Mach(),0,'f',2);
    str = QString("_N%1").arg(pPolar->NCrit(),0,'f',1);
    name += str;
    pPolar->setName(name.toStdString());


    return pPolar;
}


void io::exportSTEP(QString const & filename, TopoDS_ListOfShape const &m_ShapesToExport, int index, QString &logmsg)
{
    // inform OCC that internal units are meters
    STEPControl_Writer aWriter;

    STEPControl_StepModelType aValue = STEPControl_AsIs;

    // set the units after the writer is created
//qDebug("%s",UnitsAPI::CurrentUnit("LENGTH"));
//UnitsAPI::SetCurrentUnit("LENGTH","meter");
//qDebug()<<UnitsAPI::CurrentUnit("LENGTH");
//Interface_Static::SetCVal("write.step.unit", "M");
//qDebug() << Interface_Static::SetIVal("write.step.unit", 0);
//qDebug()    <<"exportSTEP"<<Interface_Static::CVal("write.step.unit")<<Interface_Static::IVal("write.step.unit")<<Interface_Static::RVal("write.step.unit");

    switch(index)
    {
        case 0:
            aValue = STEPControl_AsIs;
            break;
        case 1:
            aValue = STEPControl_ManifoldSolidBrep;
            break;
        case 2:
            aValue = STEPControl_BrepWithVoids;
            break;
        case 3:
            aValue = STEPControl_FacetedBrep;
            break;
        case 4:
            aValue = STEPControl_FacetedBrepAndBrepWithVoids;
            break;
        case 5:
            aValue = STEPControl_ShellBasedSurfaceModel;
            break;
        case 6:
            aValue = STEPControl_GeometricCurveSet;
            break;
        case 7:
            aValue = STEPControl_Hybrid;
            break;
        default:
            aValue = STEPControl_AsIs;
            break;
    }

    std::stringstream buffer;
    std::streambuf *originalBuffer = std::cout.rdbuf(buffer.rdbuf());

    //    aWriter.Transfer(solid, STEPControl_AsIs);
    //    aWriter.Write("cylinder.step");

    int nShapes=0;
    TopoDS_ListIteratorOfListOfShape iterator;
    TopoDS_Shape exportshape;
    for (iterator.Initialize(m_ShapesToExport); iterator.More(); iterator.Next())
    {

        //OCC assumes internal dimensions are mm, so scale by a factor 1000 before exporting
        gp_Trsf Scale;
        Scale.SetScale(gp_Pnt(0.0,0.0,0.0), 1000.0);
        try {
            BRepBuilderAPI_Transform thescaler(Scale);
            thescaler.Perform(iterator.Value(), Standard_True);
            exportshape = thescaler.Shape();

        }  catch (StdFail_NotDone &) {
            logmsg +="Error scaling the model: StdFail_NotDone\n";
             return;
        }  catch (Standard_NoSuchObject &) {
            logmsg +="Error scaling the model: Standard_NoSuchObject\n";
            return;
        }  catch (...) {
            logmsg +="Error scaling the model: Something unexpected happened....\n";
            return;
        }
        if(exportshape.IsNull())
        {
            logmsg +="exportshape is null - cannot export.";
            return;
        }


        switch(aWriter.Transfer(exportshape, aValue))
        {
            case IFSelect_RetVoid:
                logmsg +="Normal execution - created nothing or no data to process\n";
                return;
            case IFSelect_RetError:
                logmsg +="Error in command or input data, no execution\n";
                return;
            case IFSelect_RetFail:
                logmsg +="Execution was run and has failed\n";
                return;
            case IFSelect_RetStop:
                logmsg +="End or stop (such as Raise)\n";
                return;
            default:
                break;
        }
        nShapes++;
    }


    aWriter.Write(filename.toStdString().c_str());

    QString strong;
    // buffer.str() contains now the output from STEPControl
    std::cout.rdbuf(originalBuffer);
    strong = QString::fromStdString(buffer.str()) + "\n";

    logmsg +=strong;

    strong = QString::asprintf("Exported %d shape(s) to file %s", nShapes, filename.toStdString().c_str());
    logmsg +=strong;
}


void io::exportBRep(QString const & filename, const TopoDS_ListOfShape &m_ShapesToExport, QString &logmsg)
{
    std::ofstream brepfile;
    brepfile.open(filename.toStdString());
    if(brepfile.is_open())
    {
        TopoDS_ListIteratorOfListOfShape iterator;
        for (iterator.Initialize(m_ShapesToExport); iterator.More(); iterator.Next())
        {
            BRepTools::Write(iterator.Value(), brepfile);
        }
        brepfile.close();
    }
    QString strong;
    logmsg = QString::asprintf("Exported shape(s) to file %s", filename.toStdString().c_str());

}




