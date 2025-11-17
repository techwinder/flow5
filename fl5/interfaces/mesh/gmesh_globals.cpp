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

#include <QDebug>

#include <filesystem>



#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>


#include <gmsh.h>

#include <interfaces/mesh/gmesh_globals.h>

#include <api/fuse.h>
#include <api/fuseflatfaces.h>
#include <api/fusenurbs.h>
#include <api/gmshparams.h>
#include <api/triangle3d.h>
#include <api/utils.h>
#include <api/vector3d.h>
#include <api/wingxfl.h>
#include <api/sailocc.h>
#include <api/occ_globals.h>



void gmesh::listMainOptions(std::string &list)
{
    list  = "gmsh main options:\n";
    list += EOLstr;
    list += "   General:\n";
    list += "      " + getStringOption("General.Version")                      + EOLstr;
    list += "      " + getStringOption("General.BuildInfo")                    + EOLstr;
    list += "      " + getNumberOption("General.Terminal")                     + EOLstr;
    list += "      " + getNumberOption("General.Verbosity")                    + EOLstr;
    list += "      " + getNumberOption("General.NumThreads")                   + EOLstr; /** @todo test */

    list += EOLstr;
    list += "   Geometry:\n";
    list += "      " + getNumberOption("Geometry.OCCBooleanGlue")              + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCBooleanSimplify")          + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCBoundsUseStl")             + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCBrepFormatVersion")        + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCFixDegenerated")           + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCFixSmallEdges")            + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCFixSmallFaces")            + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCParallel")                 + EOLstr; /** @todo test */
    list += "      " + getNumberOption("Geometry.OCCFastUnbind")               + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCSewFaces")                 + EOLstr;
    list += "      " + getNumberOption("Geometry.OCCUseGenericClosestPoint")   + EOLstr; /** @todo test */
    list += EOLstr;
}


std::string gmesh::getNumberOption(std::string name)
{
    int nchar = 37;
    double number(0);
    gmsh::option::getNumber(name, number);
    name.resize(nchar, ' ');
    QString str = QString::fromStdString(name) + QString::asprintf(":  %g", number);
    return str.toStdString();
}


std::string gmesh::getStringOption(std::string name)
{
    int nchar = 37;
    std::string optionvalue;
    gmsh::option::getString(name, optionvalue);
    name.resize(nchar, ' ');
    std::string str = name + optionvalue;
    return str;
}


void gmesh::listModelEntities(QString &list)
{
    list = "Model entities:\n";

    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags);
    std::string entityType;
    for(uint k=0; k<modelenditiesdimTags.size(); k++)
    {
        std::pair<int, int> const &entity = modelenditiesdimTags.at(k);
        gmsh::model::getEntityType(entity.first, entity.second, entityType);
        list += QString::asprintf("   entity dim=%d, tag=%d, name=",entity.first, entity.second)+QString::fromStdString(entityType)+EOLch;
    }
}


void gmesh::listModel(QString &list)
{
    std::vector<std::pair<int, int> > entities;
    gmsh::model::getEntities(entities);

    if(entities.size()==0)
    {
        list = "No entities found in model\n\n";
        return;
    }

    QString strange;

    for(uint i=0; i<entities.size(); i++)
    {
        // get the mesh nodes for each elementary entity
        std::vector<std::size_t> nodeTags;
        std::vector<double> nodeCoords, nodeParams;
        int dim = entities[i].first, tag = entities[i].second;
        gmsh::model::mesh::getNodes(nodeTags, nodeCoords, nodeParams, dim, tag);

        // get the mesh elements for each elementary entity
        std::vector<int> elemTypes;
        std::vector<std::vector<std::size_t> > elemTags, elemNodeTags;
        gmsh::model::mesh::getElements(elemTypes, elemTags, elemNodeTags, dim, tag);

        // report some statistics
        int numElem = 0;
        for(uint j=0; j<elemTags.size(); j++)
            numElem += int(elemTags[j].size());
        std::string type;
        gmsh::model::getType(dim, tag, type);

        strange  = QString::asprintf("entity %d: dim=%d, tag%d, ", i, dim, tag)+ QString::fromStdString(type);
        strange += QString::asprintf(" nNodes=%d, nElements=%d", int(nodeTags.size()), numElem);
        list += strange+EOLch;


        if(elemTypes.size())  list += "Element types:\n";
        for(uint j=0; j<elemTypes.size(); j++)
        {
            std::string name;
            int d, order, numv, numpv;
            std::vector<double> param;
            gmsh::model::mesh::getElementProperties(elemTypes[j], name, d, order,
                                                    numv, param, numpv);
            list += "Element type:" + QString::fromStdString(name);
            list += QString::asprintf(", order %d with %d nodes in param coord:", order, numv);
            for(uint k=0; k<param.size(); k++)  list += QString::asprintf("   %11g", param[k]);
            list += EOLch;
        }
    }
}


void gmesh::convertFromGmsh(std::vector<Triangle3d> &triangles, QString &log)
{
    QString strange;
    std::vector<std::size_t> elementTags;
    std::vector<std::size_t> nodeTags;
    const int ElementType = 2; // triangles

    gmsh::model::mesh::getElementsByType(ElementType, elementTags, nodeTags);

    strange = QString::asprintf("Built %d type 2 elements\n", int(elementTags.size()));
    log += strange;

    if(elementTags.size()<=0)
    {
        log += "No triangles to convert\n";
        return;
    }

    std::vector<std::size_t> nodetags;
    std::vector<double> coord;
    std::vector<double> parametricCoord;
    int dim = -1;
    int tag = -1;
    gmsh::model::mesh::getNodes(nodetags,
                                coord,
                                parametricCoord,
                                dim, tag,
                                false, false);
    strange = QString::asprintf("Built %d nodes and %d coordinates\n", int(nodetags.size()), int(coord.size()));
    log += strange;
    assert(coord.size() == 3*nodetags.size());

    if(nodetags.size()<=0)
    {
        log += "No nodes found, aborting\n\n";
        return;
    }

    // need to make an array of nodes for quick access
    // option 1: make a map (tag, position); slow?
    // option 2: make an array with index=tag;
    size_t maxtag = *std::max_element(nodetags.begin(), nodetags.end());
    std::vector<Vector3d> node(maxtag+1);
    for(uint j=0; j<nodetags.size(); j++)
    {
        int inode = int(nodetags.at(j));
        node[inode].set(&coord.at(3*j+0));
    }

    convertTriangles(elementTags, node, triangles, log);
}


void gmesh::convertTriangles(std::vector<std::size_t>const&elementTags,
                             std::vector<Vector3d> const &node, std::vector<Triangle3d> &m_Triangles, QString &log)
{
    std::vector<std::size_t> elementnodetags;
    int elementtype = -1;
    std::vector<Triangle3d> triangles(elementTags.size());
    int dim(0), tag(0);

    double minsize(LARGEVALUE), maxsize(0);
    for(uint i=0; i<elementTags.size(); i++)
    {
        int ielementtag = int(elementTags.at(i));
        gmsh::model::mesh::getElement(ielementtag,
                                      elementtype,
                                      elementnodetags,
                                      dim,
                                      tag);
        assert(elementtype==2); // Triangle
        assert(dim==2); // Surface type element

        Triangle3d &t3d = triangles[i];
        for(uint j=0; j<elementnodetags.size(); j++)
        {
            int nodetag = int(elementnodetags.at(j));
            t3d.setVertex(j, node.at(nodetag));
        }
        t3d.setTriangle();
        minsize = std::min(minsize, t3d.minEdgeLength());
        maxsize = std::max(maxsize, t3d.maxEdgeLength());
    }
    m_Triangles.insert(m_Triangles.end(), triangles.begin(), triangles.end());

    log += QString::asprintf("Min. element size = %g\n", minsize);
    log += QString::asprintf("Max. element size = %g\n", maxsize);
}


void gmesh::makeModelCurves(std::vector<std::vector<Vector3d>> &curves)
{
    const int DIM = 1; // lines
    const int RES = 20;
    std::vector<double> param(RES+1);
    std::vector<double> coord;
    for(uint i=0; i<=RES; i++) param[i] = double(i)/double(RES);

    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags);

    for(uint k=0; k<modelenditiesdimTags.size(); k++)
    {
        std::pair<int, int> const &entity = modelenditiesdimTags.at(k);
//        gmsh::model::getEntityType(entity.first, entity.second, entityType);
//        m_pptoF5->onAppendThisPlainText("   entity dim=%d, tag=%d, name="+std::string::fromStdString(entityType)+EOLCHAR);
        if(entity.first==DIM) // lines
        {
            gmsh::model::getValue(entity.first, entity.second, param, coord);

            curves.push_back(std::vector<Vector3d>(RES+1));
            std::vector<Vector3d>& Curve = curves.back();

            for(uint l=0; l<param.size(); l++)
            {
                Vector3d &pt = Curve[l];
                pt.x = coord[3*l];
                pt.y = coord[3*l+1];
                pt.z = coord[3*l+2];
            }
        }
    }
}


bool gmesh::getLine(int tag, Vector3d &v0, Vector3d &v1)
{
    // assumes occ is synchronized

    const int DIM0 = 0; // points
    const int DIM1 = 1; // lines
    std::vector<double> coord;
    std::vector<double> pmin, pmax;

    gmsh::vectorpair pointdimtags, linedimtags;
    gmsh::model::getEntities(pointdimtags, DIM0);
    gmsh::model::getEntities(linedimtags, DIM1);

    for(uint k=0; k<linedimtags.size(); k++)
    {
        std::pair<int, int> const &line = linedimtags.at(k);
        assert(line.first==DIM1);
        if(line.second==tag)
        {
            gmsh::model::getParametrizationBounds(DIM1, line.second, pmin, pmax);
            assert(pmin.size()==1 && pmax.size()==1);
            gmsh::model::getValue(DIM1, tag, {pmin.front(), pmax.front()}, coord);
            assert(coord.size()==6);
            v0.set(coord.at(0), coord.at(1), coord.at(2));
            v1.set(coord.at(3), coord.at(4), coord.at(5));
            return true;
        }
    }
    return false;
}


bool gmesh::getVertex(int tag, Vector3d &vertex)
{
    const int DIM0 = 0; // points
    std::vector<double> coord;

    gmsh::vectorpair pointdimtags;
    gmsh::model::getEntities(pointdimtags, DIM0);

    for(uint k=0; k<pointdimtags.size(); k++)
    {
        std::pair<int, int> const &point = pointdimtags.at(k);
        assert(point.first==DIM0);
        if(point.second==tag) // point
        {
            gmsh::model::getValue(point.first, point.second, {}, coord);
            vertex.set(coord[0], coord[1], coord[2]);
            return true;
        }
    }
    return false;
}


void gmesh::makeModelVertices(std::vector<Vector3d> &vertices)
{
    const int DIM = 0; // points
    std::vector<double> coord;

    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags);

    for(uint k=0; k<modelenditiesdimTags.size(); k++)
    {
        std::pair<int, int> const &entity = modelenditiesdimTags.at(k);
        if(entity.first==DIM) // point
        {
            gmsh::model::getValue(entity.first, entity.second, {}, coord);
            vertices.push_back({coord[0], coord[1], coord[2]});
        }
    }
}

std::string gmesh::tempFile()
{
    std::string temppath;
    temppath = std::filesystem::temp_directory_path().string();
    temppath +=            std::filesystem::path::preferred_separator;
    temppath += "gmsh.brep";
//    qDebug("%s", temppath.c_str());
    return temppath;
}


bool gmesh::importBRepList(std::vector<std::string> const &breps, std::string &brep)
{
    // no option to gmsh::merge from a string in v4.14.1,
    // so write them one by one to a file and import back
    std::string temppath = tempFile();

    for(uint i=0; i<breps.size(); i++)
    {
        if(!xfl::stringToFile(breps.at(i), temppath))
            return false;

        try
        {
            gmsh::merge(temppath);
        }
        catch(...)
        {
            return false;
        }
    }

    // write the merged BReps
    try
    {
        gmsh::write(temppath);
    }
    catch(...)
    {
        return false;
    }

    //and import them in one BREP for flow5
    return xfl::stringFromFile(brep, temppath);

}


double gmesh::wettedArea()
{
    const int DIM = 2; // faces

//    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::occ::getEntities(modelenditiesdimTags, DIM);

    double totalmass = 0.0;
    double mass(0);

    for(uint k=0; k<modelenditiesdimTags.size(); k++)
    {
        std::pair<int, int> const &entity = modelenditiesdimTags.at(k);
        if(entity.first==DIM) // Faces
        {
            gmsh::model::occ::getMass(entity.first, entity.second, mass);
            totalmass += mass;
        }
    }
    return totalmass; // m²
}


bool gmesh::gmshtoBRep(std::string &brep)
{
    std::string temppath = tempFile();

    try
    {
        gmsh::write(temppath);
    }
    catch(...)
    {
        return false;
    }

    //and import them in one BREP for flow5

    //and import them in one BREP for flow5
    return xfl::stringFromFile(brep, temppath);
}


/** It would really be nice to have gmesh::merge(std::string) */
bool gmesh::BReptoGmsh(std::string const &brep)
{
    std::string temppath = tempFile();


    if(!xfl::stringToFile(brep, temppath)) return false;

    try
    {
        gmsh::merge(temppath);
    }
    catch(...)
    {
        return false;
    }

    return true;
}


bool gmesh::BRepstoGmsh(const std::vector<std::string> &brep)
{
    std::string temppath = tempFile();


    for(uint i=0; i<brep.size(); i++)
    {
        if(!xfl::stringToFile(brep.at(i), temppath)) return false;

        try
        {
            gmsh::merge(temppath);
        }
        catch(...)
        {
            return false;
        }
    }

    return true;
}


void gmesh::bRepToStepFile(const std::string &brep, const std::string &pathname)
{
    std::string temppath = tempFile();

    xfl::stringToFile(brep, temppath);
    gmsh::clear();
    gmsh::model::add("Export");
    gmsh::merge(temppath);

    // Specify
    gmsh::option::setString("Geometry.OCCTargetUnit", "M");
    // before merging the STEP file, OpenCASCADE to converted the units to meters (instead of the default, which is millimeters).

    gmsh::write(pathname); // assumes extension has been set
}


void gmesh::getBoundingBox(Vector3d &botleft, Vector3d &topright)
{
    // Get the bounding box of the volume:
    double xmin(0), ymin(0), zmin(0), xmax(0), ymax(0), zmax(0);
    botleft.set(LARGEVALUE,LARGEVALUE,LARGEVALUE);
    topright.set(-LARGEVALUE, -LARGEVALUE, -LARGEVALUE);

    const int DIM = 2; // faces

    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags, DIM);

    for(uint k=0; k<modelenditiesdimTags.size(); k++)
    {
        std::pair<int, int> const &entity = modelenditiesdimTags.at(k);

        gmsh::model::occ::getBoundingBox(entity.first, entity.second,
                                         xmin, ymin, zmin,
                                         xmax, ymax, zmax);
        botleft.x  = std::min(botleft.x, xmin);
        botleft.y  = std::min(botleft.y, ymin);
        botleft.z  = std::min(botleft.z, zmin);
        topright.x = std::max(topright.x, xmax);
        topright.y = std::max(topright.y, ymax);
        topright.z = std::max(topright.z, zmax);
    }
}


bool gmesh::rotateBrep(std::string const&brep, Vector3d const &O, Vector3d const &axis, double theta, std::string &rotated)
{
    if(fabs(theta)<ANGLEPRECISION) return false;

    gmsh::clear();
    gmsh::model::add("BRep");

    BReptoGmsh(brep);
    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags);
    try
    {
        gmsh::model::occ::rotate(modelenditiesdimTags, O.x, O.y, O.z, axis.x, axis.y, axis.z, theta*PI/180.0);
    }
    catch(...)
    {
        return false;
    }

    gmsh::model::occ::synchronize();
    gmshtoBRep(rotated);
    return true;
}


bool gmesh::scaleBrep(std::string const&brep, Vector3d const &O, double sx, double sy, double sz, std::string &scaled)
{
    gmsh::clear();
    gmsh::model::add("BRep");

    BReptoGmsh(brep);
    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags);
    try
    {
        gmsh::model::occ::dilate(modelenditiesdimTags, O.x, O.y, O.z, sx, sy, sz);
    }
    catch(...)
    {
        return false;
    }

    gmsh::model::occ::synchronize();
    gmshtoBRep(scaled);
    return true;
}


bool gmesh::translateBrep(std::string const&brep, Vector3d const &T, std::string &translated)
{
    gmsh::clear();
    gmsh::model::add("BRep");

    BReptoGmsh(brep);
    gmsh::model::occ::synchronize();
    gmsh::vectorpair modelenditiesdimTags;
    gmsh::model::getEntities(modelenditiesdimTags);
    try
    {
        gmsh::model::occ::translate(modelenditiesdimTags, T.x, T.y, T.z);
    }
    catch(...)
    {
        return false;
    }

    gmsh::model::occ::synchronize();
    gmshtoBRep(translated);
    return true;
}


// untested
bool gmesh::wingToBRep(const WingXfl *pWing, std::string &brep, QString &log)
{
    if(!pWing) return false;

    gmsh::clear();
    gmsh::model::add("BRep");

    int wingtag = 100;
    std::vector<int> sectiontags;
    std::vector<int> pointtags;
    std::vector<int> linetags;
    for(int is=0; is<pWing->nSurfaces(); is++)
    {
        pointtags.clear();
        linetags.clear();
        Surface const &surf = pWing->surfaceAt(is);
        //LEFT SIDE TIP
        for(uint i=0; i<surf.m_SideA_Bot.size(); i++)
        {
            Node const& nd = surf.m_SideA_Bot.at(i);
            pointtags.push_back(gmsh::model::occ::addPoint(nd.x, nd.y, nd.z));
        }
        for(int i=int(surf.m_SideA_Top.size())-2; i>0; i--)
        {
            Node const& nd = surf.m_SideA_Top.at(i);
            pointtags.push_back(gmsh::model::occ::addPoint(nd.x, nd.y, nd.z));
        }


        for(uint i=0; i<pointtags.size()-1; i++)
        {
            linetags.push_back(gmsh::model::occ::addLine(pointtags.at(i), pointtags.at(i+1)));
        }

        try
        {
            // close the loop
            linetags.push_back(gmsh::model::occ::addLine(pointtags.back(), pointtags.front()));
            int looptagA = gmsh::model::occ::addCurveLoop(linetags);

            // make the section
            sectiontags.push_back(gmsh::model::occ::addPlaneSurface({looptagA}));
        }
        catch(...)
        {
            log += QString::asprintf("Error making wing section %d... aborting\n", is);
            return false;
        }
    }

    //RIGHT SIDE TIP
    Surface const &surf = pWing->lastSurface();
    pointtags.clear();
    linetags.clear();
    for(uint i=0; i<surf.m_SideA_Bot.size(); i++)
    {
        Node const& nd = surf.m_SideB_Bot.at(i);
        pointtags.push_back(gmsh::model::occ::addPoint(nd.x, nd.y, nd.z));
    }
    for(int i=int(surf.m_SideA_Top.size())-2; i>0; i--)
    {
        Node const& nd = surf.m_SideB_Top.at(i);
        pointtags.push_back(gmsh::model::occ::addPoint(nd.x, nd.y, nd.z));
    }

    linetags.clear();
    for(uint i=0; i<pointtags.size()-1; i++)
    {
        linetags.push_back(gmsh::model::occ::addLine(pointtags.at(i), pointtags.at(i+1)));
    }

    try
    {
        // close the loop
        linetags.push_back(gmsh::model::occ::addLine(pointtags.back(), pointtags.front()));
        int looptagB = gmsh::model::occ::addCurveLoop(linetags);

        // make the section
        sectiontags.push_back(gmsh::model::occ::addPlaneSurface({looptagB}));
    }
    catch(...)
    {
        log += "Error making right tip section %d... aborting\n";
        return false;
    }

    //SWEEP THROUGH

    std::vector<std::pair<int, int> > out;
    try
    {
        gmsh::model::occ::addThruSections(sectiontags, out, wingtag, true);

        gmsh::vectorpair sections;
        for(uint i=0; i<sectiontags.size(); i++)
            sections.push_back({2, sectiontags.at(i)});
        gmsh::model::occ::remove(sections);
    }
    catch(...)
    {
        log += "Error sweeping through the sections... aborting\n";
        return false;
    }

    gmsh::model::occ::synchronize();
    gmshtoBRep(brep);

    return true;

}


bool gmesh::fuseQuadsToBRep(FuseFlatFaces const*pFuse, std::string &brep, std::string &log)
{
    if(!pFuse) return false;

    gmsh::clear();
    gmsh::model::add("BRep");

    int r00(0), r01(0), r10(0), r11(0);
    int l00(0), l01(0), l10(0), l11(0);

    std::vector<int> rightlines, leftlines;
    try
    {
        std::vector<int> surf;
        for(int i=0; i<pFuse->nurbs().nFrames()-1; i++)
        {
            Frame const &f0 = pFuse->nurbs().frameAt(i);
            Frame const &f1 = pFuse->nurbs().frameAt(i+1);
            for(int j=0; j<f0.nCtrlPoints()-1; j++)
            {
                const Vector3d &p00 = f0.ctrlPointAt(j);
                const Vector3d &p01 = f0.ctrlPointAt(j+1);
                const Vector3d &p10 = f1.ctrlPointAt(j);
                const Vector3d &p11 = f1.ctrlPointAt(j+1);

                r00 = gmsh::model::occ::addPoint(p00.x, p00.y, p00.z);
                r01 = gmsh::model::occ::addPoint(p01.x, p01.y, p01.z);
                r10 = gmsh::model::occ::addPoint(p10.x, p10.y, p10.z);
                r11 = gmsh::model::occ::addPoint(p11.x, p11.y, p11.z);

                l00 = gmsh::model::occ::addPoint(p00.x, -p00.y, p00.z);
                l01 = gmsh::model::occ::addPoint(p01.x, -p01.y, p01.z);
                l10 = gmsh::model::occ::addPoint(p10.x, -p10.y, p10.z);
                l11 = gmsh::model::occ::addPoint(p11.x, -p11.y, p11.z);

                rightlines.clear();
                leftlines.clear();
                if(!p00.isSame(p10))
                {
                    rightlines.push_back(gmsh::model::occ::addLine(r00, r10));
                    leftlines.push_back( gmsh::model::occ::addLine(l10, l00));
                }
                if(!p10.isSame(p11))
                {
                    rightlines.push_back(gmsh::model::occ::addLine(r10, r11));
                    leftlines.push_back( gmsh::model::occ::addLine(l11, l10));
                }
                if(!p11.isSame(p01))
                {
                    rightlines.push_back(gmsh::model::occ::addLine(r11, r01));
                    leftlines.push_back( gmsh::model::occ::addLine(l01, l11));
                }
                if(!p01.isSame(p00))
                {
                    rightlines.push_back(gmsh::model::occ::addLine(r01, r00));
                    leftlines.push_back( gmsh::model::occ::addLine(l00, l01));
                }

                if(rightlines.size()>2)
                {
                    int rightloop = gmsh::model::occ::addCurveLoop(rightlines);
                    surf.push_back(gmsh::model::occ::addSurfaceFilling(rightloop));
                    int leftloop = gmsh::model::occ::addCurveLoop(leftlines);
                    surf.push_back(gmsh::model::occ::addSurfaceFilling(leftloop));
                }
            }
        }

        gmsh::model::occ::addSurfaceLoop(surf);
    }
    catch(...)
    {
        log += "Error making gmsh Quads surface... aborting\n";
        return false;
    }

    gmsh::model::occ::synchronize();
    gmshtoBRep(brep);

    return true;
}


bool gmesh::fuseNurbsToBRep(FuseNurbs const*pFuse, std::string &brep, std::string &log)
{
    if(!pFuse) return false;

    gmsh::clear();
    gmsh::model::add("BRep");

    try
    {
        std::vector<int> PointsU;
        for(int i=0; i<pFuse->nurbs().nFrames(); i++)
        {
            Frame const &frame = pFuse->nurbs().frameAt(i);
            for(int j=0; j<frame.nCtrlPoints(); j++)
            {
                const Vector3d &pt = frame.ctrlPointAt(j);
                PointsU.push_back(gmsh::model::occ::addPoint(pt.x, -pt.y, pt.z));
            }
        }
        int leftsurftag = gmsh::model::occ::addBSplineSurface(PointsU, pFuse->sideLineCount());
        gmsh::vectorpair copiedtags;
        gmsh::model::occ::copy({{2, leftsurftag}}, copiedtags);
        gmsh::model::occ::mirror(copiedtags,0.0,1.0,0.0,0.0);
//        int rightsurfacetag = copiedtags.front().second;
    }
    catch(...)
    {
        log += "Error making gmsh NURBS surface... aborting\n";
        return false;
    }

    gmsh::model::occ::synchronize();
    gmshtoBRep(brep);

    return true;
}


bool gmesh::intersectBrep(std::string const &brep, std::vector<Node> const &A, std::vector<Node> const &B,
                          std::vector<Vector3d> &I, std::vector<bool> &bIntersect)
{
    assert(A.size()==B.size());
    assert(A.size()==I.size());
    assert(A.size()==bIntersect.size());

    std::fill(bIntersect.begin(), bIntersect.end(), false);

    const int DIM1 = 1; // lines
    const int DIM2 = 2; // faces


    gmsh::clear();
    gmsh::model::add("BRep");
    BReptoGmsh(brep);

    // add the points
    std::vector<int> Adimtags, Bdimtags;
    for(uint l=0; l<A.size(); l++)
    {
        Adimtags.push_back(gmsh::model::occ::addPoint(A.at(l).x, A.at(l).y, A.at(l).z));
        Bdimtags.push_back(gmsh::model::occ::addPoint(B.at(l).x, B.at(l).y, B.at(l).z));
    }

    // make the lines
    gmsh::vectorpair Ldimtags(Adimtags.size());
    for(uint k=0; k<Adimtags.size(); k++)
    {
        Ldimtags[k].first  = DIM1;
        Ldimtags[k].second = gmsh::model::occ::addLine(Adimtags.at(k), Bdimtags.at(k));
    }


    // get all the faces
    gmsh::vectorpair Facedimtags;
    gmsh::model::occ::getEntities(Facedimtags, DIM2);

    // intersect the lines
    gmsh::vectorpair outDimTags;
    std::vector<gmsh::vectorpair> outDimTagsMap;

    Vector3d v0, v1;

    auto t0 = std::chrono::high_resolution_clock::now();

    gmsh::model::occ::fragment(Facedimtags, Ldimtags, outDimTags, outDimTagsMap, -1, false, false); // slow...

    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug() << QString::asprintf("gmesh::fragment: %7g ms\n", float(duration)/1000.0);

    gmsh::model::occ::synchronize();
    for(uint l=0; l<outDimTags.size(); l++)
    {
        std::pair<int,int> const &frag = outDimTags.at(l);
        if(frag.first != DIM1) continue;
        getLine(frag.second, v0, v1);
        for(uint k=0; k<A.size(); k++)
        {
            if(v0.isSame(A.at(k), 1.0e-4))
            {
                I[k] = v1;
                bIntersect[k] = true;
                break;
            }
            else if(v1.isSame(B.at(k), 1.0e-4))
            {
                I[k] = v0;
                bIntersect[k] = true;
                break;
            }
        }
    }

/*    // fragment lines one by one to ensure identification
    for(uint k=0; k<Ldimtags.size(); k++)
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        gmsh::model::occ::fragment(Facedimtags, {Ldimtags[k]}, outDimTags, outDimTagsMap, -1, false, false);
        auto t1 = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        qDebug("gmesh::fragment: %7g ms", float(duration)/1000.0);

        gmsh::model::occ::synchronize();// slow...
        for(uint l=0; l<outDimTags.size(); l++)
        {
            std::pair<int,int> const &frag = outDimTags.at(l);
            if(frag.first != DIM1) continue;
            getLine(frag.second, v0, v1);
            if(v0.isSame(A.at(k), 1.0e-4))
            {
                I[k] = v1;
                bIntersect[k] = true;
                break;
            }
            else if(v1.isSame(B.at(k), 1.0e-4))
            {
                I[k] = v0;
                bIntersect[k] = true;
                break;
            }
        }
    }*/

//    gmsh::model::occ::synchronize();

/*    std::string strange;
    qDebug()<<"dim 1 fragments:";
    int index = 0;
    for(uint k=0; k<Idimtags.size(); k++)
    {
        std::pair<int,int> const &frag = Idimtags.at(k);
        if(frag.first != DIM1) continue;
        getLine(frag.second, v0, v1);
        strange = std::string::asprintf("tag=%d:  (%9.3g %9.3g %9.3g) <--> (%9.3g %9.3g %9.3g) ", frag.second, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z);
        qDebug("%s",strange.toStdString().c_str());

        if(isEven(k))
        {
            I[index].set(v1);
            bIntersect[index]=true;
            index++;
        }
    }*/

/*    qDebug()<<"Mapping:";
    Vector3d vi0, vi1, vo0, vo1;
    for(uint k=0; k<outDimTagsMap.size(); k++)
    {
        gmsh::vectorpair const &map = outDimTagsMap.at(k);
        if(map.size()==2)
        {
            // there is a correspondence
            std::pair<int, int> const &in  = map.at(0);
            std::pair<int, int> const &out = map.at(1);

            if(in.first==DIM1 && out.first==DIM1)
            {
                strange = std::string::asprintf("  In(%d, %d) --> out(%d, %d)", in.first, in.second, out.first, out.second);
                qDebug()<<strange;
                getLine(in.second,  vi0, vi1);
                getLine(out.second, vo0, vo1);
                strange = std::string::asprintf("      (%.3g %.3g %.3g)(%.3g %.3g %.3g) --> (%.3g %.3g %.3g)(%.3g %.3g %.3g)",
                                            vi0.x, vi0.y, vi0.z, vi1.x, vi1.y, vi1.z,
                                            vo0.x, vo0.y, vo0.z, vo1.x, vo1.y, vo1.z);
                qDebug()<<strange;
            }
        }
    }*/



//    makeModelVertices(I); // not necessarily in the right order

    return true;
}


void gmesh::tessellateBRep(std::string const&BRep, GmshParams const &params, std::vector<Triangle3d> &triangles, QString &log)
{
    gmsh::clear();
    gmsh::model::add("BRep");

    BReptoGmsh(BRep);

    try
    {
        gmsh::option::setNumber("Mesh.MeshSizeMin",           params.m_MinSize);
        gmsh::option::setNumber("Mesh.MeshSizeMax",           params.m_MaxSize);
        gmsh::option::setNumber("Mesh.MeshSizeFromCurvature", params.m_nCurvature);

        gmsh::model::mesh::generate(2);
    }
    catch(...)
    {
        log.append("Error making triangulation\n");
        return;
    }


    gmesh::convertFromGmsh(triangles, log);
}


void gmesh::tessellateShape(TopoDS_Shape const&Shape, GmshParams const &params, std::vector<Triangle3d> &triangles, QString &log)
{
    gmsh::clear();
    gmsh::model::add("TopoDS_Shape");

    TopExp_Explorer shapeExplorer;

    for (shapeExplorer.Init(Shape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Face aFace = TopoDS::Face(shapeExplorer.Current());
        tessellateFace(aFace, params, triangles, log);

    }
}


/** @todo unusable: importShapesNativePointer throws an unknown exception */
void gmesh::tessellateFace(TopoDS_Face const&Face, GmshParams const &params, std::vector<Triangle3d> &triangles, QString &log)
{
    gmsh::clear();
    gmsh::model::add("Face");

    gmsh::vectorpair outDimTags;
    const bool bHighestDimOnly = true;

    try
    {
        gmsh::option::setNumber("Mesh.MeshSizeMin",           params.m_MinSize);
        gmsh::option::setNumber("Mesh.MeshSizeMax",           params.m_MaxSize);
        gmsh::option::setNumber("Mesh.MeshSizeFromCurvature", params.m_nCurvature);

        gmsh::model::occ::importShapesNativePointer(&Face, outDimTags, bHighestDimOnly);

        gmsh::model::mesh::generate(2);
        gmesh::convertFromGmsh(triangles, log);
    }
    catch(...)
    {
        log.append("Error making triangulation\n");
        return;
    }
}


/** alternative to using the in-class makeShellTriangulation method */
void gmesh::makeSailOccTriangulation(SailOcc *pSailOcc)
{
    QString strong, prefix;

    QString logmsg;

    pSailOcc->clearTriangles();

    std::vector<Triangle3d> triangles;

/*    if(Part::s_bOccTessellator)
    {
        for(TopTools_ListIteratorOfListOfShape shellitt(m_Shape); shellitt.More(); shellitt.Next())
        {
            TopExp_Explorer shapeExplorer;
            for (shapeExplorer.Init(shellitt.Value(), TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
            {
                try
                {
                    BRepTools::Clean(shellitt.Value());

                    TopoDS_Shell shell = TopoDS::Shell(shapeExplorer.Current());
                    if(!shell.IsNull())
                    {
                        occ::shellTriangulationWithOcc(shell, m_OccTessParams, triangles);
                    }
                }
                catch(Standard_TypeMismatch &)
                {
                }
                catch(...)
                {
                }
            }
        }
    }
    else*/
    {
        for(uint i=0; i<pSailOcc->bReps().size(); i++)
            gmesh::tessellateBRep(pSailOcc->bReps().at(i), pSailOcc->gmshTessParams(), triangles, logmsg);
    }


    pSailOcc->setTriangles(triangles);

    strong = QString::asprintf("Made %d triangles\n", pSailOcc->nTriangles());
    logmsg +=prefix + strong;
    int nnodes = pSailOcc->triangulation().makeNodes();

    strong = QString::asprintf("Made %d nodes\n", nnodes);
    logmsg +=prefix + strong;
    pSailOcc->makeNodeNormals(false);

    strong = "Made node normals\n";
    logmsg +=prefix + strong;
}


int gmesh::makeFuseTriangulation(Fuse *pFuse, QString &logmsg, const QString &prefix)
{
    QString strong;
    logmsg.clear();

    pFuse->clearTriangles();

    std::vector<Triangle3d> triangles;

    int iShell = 0;

    TopoDS_ListOfShape *pShells(nullptr);

    FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
    if(pFuseXfl) pShells = &pFuseXfl->rightSideShells();
    else         pShells = &pFuse->shells();


    for(TopTools_ListIteratorOfListOfShape shellitt(*pShells); shellitt.More(); shellitt.Next())
    {
/*        if(s_bOccTessellator)
        {
            clearOccTriangulation();
            occ::shapeTriangulationWithOcc(shellitt.Value(), m_OccTessParams, triangles);
        }
        else*/
        {
            std::string brep;
            if(occ::shapeToBrep(shellitt.Value(), brep))
            {
                gmesh::tessellateBRep(brep, pFuse->gmshTessParams(), triangles, logmsg);
            }
            else
            {
                logmsg += prefix + QString::asprintf("Error tessellating shell %d\n", iShell);
            }
        }
        iShell++;
    }

    pFuse->setTriangles(triangles);

    if(pFuseXfl) pFuseXfl->triangulation().makeXZsymmetric();

    strong = QString::asprintf("Made %d triangles\n", pFuse->nTriangles());
    logmsg +=prefix + strong;
    int nnodes = pFuse->makeTriangleNodes();

    strong = QString::asprintf("Made %d nodes\n", nnodes);
    logmsg +=prefix + strong;
    pFuse->makeNodeNormals(false);

    strong = "Made node normals\n";
    logmsg +=prefix + strong;

    return pFuse->nTriangles();
}










