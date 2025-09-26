/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include <QDebug>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "occsail.h"
#include <xflocc/occ_globals.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>


OccSail::OccSail() : ExternalSail()
{

}


void OccSail::duplicate(Sail const*pSail)
{
    ExternalSail::duplicate(pSail);
    OccSail const* pOccSail = dynamic_cast<OccSail const*>(pSail);
    m_Shape = pOccSail->m_Shape;
}


bool OccSail::serializeSailFl5(QDataStream &ar, bool bIsStoring)
{
    Sail::serializeSailFl5(ar, bIsStoring);

    //500001 : new fl5 format;
    int k=0, n=0;
    float xf=0,yf=0,zf=0;

    Vector3d V0,V1,V2;

    // 500001: first fl5 format in beta18
    // 500002: added OccMeshParams in beta 18
    int ArchiveFormat = 500002;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_bThinSurface;

        ar << int(m_BotMidTEIndexes.size());
        foreach(int idx, m_BotMidTEIndexes)   ar << idx;

        ar << int(m_TopTEIndexes.size());
        foreach(int idx, m_TopTEIndexes)      ar << idx;

        ar << m_MaxElementSize;

        ar << int(m_Shape.Size());

        std::stringstream sstream;

        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            sstream.str(std::string()); // clear the stream
            BRepTools::Write(shapeit.Value(), sstream); // stream the brep to the stringstream
            std::string s = sstream.str();
            QString brepstr = QString::fromStdString(s);
            ar << brepstr; // write the QString to the archive file
        }

        ar << int(m_RefTriangles.size());
        for(int i=0; i<m_RefTriangles.size(); i++)
        {
            Triangle3d const &t3d = m_RefTriangles.at(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        m_OccMeshParams.serializeParams(ar, bIsStoring);
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>500100) return false;

        ar >> m_bThinSurface;

        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> k;
            m_BotMidTEIndexes.append(k);
        }

        ar >> n;
        for(int i=0; i<n; i++)
        {
            ar >> k;
            m_TopTEIndexes.append(k);
        }

        ar >> m_MaxElementSize;

        m_Shape.Clear();
        int nShapes;
        ar >> nShapes;
        for(int iShape=0; iShape<nShapes; iShape++)
        {
            QString brepstr;
            ar >> brepstr;

            // do a deep copy before using the QString in the istream
            QByteArray ba = brepstr.toLatin1();

            char *strong = new char[ulong(brepstr.size()+1000)];
            strcpy(strong, ba.constData());

            try
            {
                std::stringstream sstream;

                sstream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);

                sstream << strong;

                TopoDS_Shape shape;
                BRep_Builder aBuilder;
                BRepTools::Read(shape, sstream, aBuilder);
                m_Shape.Append(shape);
                delete [] strong;
            }
            catch(...)
            {
                qDebug()<<"reading exception";
                delete [] strong;
                return false;
            }
        }
        ar >> n;
        m_RefTriangles.resize(n);
        for(int i3=0; i3<n; i3++)
        {
            ar >> xf >> yf >> zf;
            V0.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V1.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V2.set(double(xf), double(yf), double(zf));

            m_RefTriangles[i3].setTriangle(V0,V1, V2);
        }

        if(ArchiveFormat>=500002)
            m_OccMeshParams.serializeParams(ar, bIsStoring);

        makeTriangulation();
        computeProperties();
        updateStations();
    }
    return true;
}


void OccSail::makeTriangulation(int , int )
{
    QString strong, prefix;

    m_LogMsg.clear();

    m_Triangulation.clear();

    QVector<Triangle3d> triangles;

//    int iShell = 0;

    for(TopTools_ListIteratorOfListOfShape shellitt(m_Shape); shellitt.More(); shellitt.Next())
    {
        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(shellitt.Value(), TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            try
            {
                TopoDS_Shell shell = TopoDS::Shell(shapeExplorer.Current());
                if(!shell.IsNull())
                {
                    occ::shapeTriangulationWithOcc(shell, m_OccMeshParams, triangles);

//                    iShell++;
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

    m_Triangulation.setTriangles(triangles);

    strong = QString::asprintf("Made %d triangles\n", m_Triangulation.nTriangles());
    m_LogMsg +=prefix + strong;
    int nnodes = m_Triangulation.makeNodes();

    strong = QString::asprintf("Made %d nodes\n", nnodes);
    m_LogMsg +=prefix + strong;
    m_Triangulation.makeNodeNormals(false);

    strong = QString::asprintf("Made node normals\n");
    m_LogMsg +=prefix + strong;
}


void OccSail::properties(QString &props, QString const &frontspacer, bool bFull) const
{
    QString strlength = Units::lengthUnitLabel();
    QString strarea = Units::areaUnitLabel();
    QString strange;

    Vector3d foot = (m_Clew-m_Tack);
    Vector3d gaff = m_Peak-m_Head;
    double bottwist = atan2(foot.y, foot.x)*180.0/PI;
    double toptwist = atan2(gaff.y, gaff.x)*180.0/PI;

    props.clear();
    props += frontspacer + m_Name +"\n";
    if(bFull)
    {
        props += frontspacer + "   CAD type sail\n";
    }
    strange = QString::asprintf("   Luff length    = %7.3g ", luffLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Leech length   = %7.3g ", leechLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Foot length    = %7.3g ", footLength()*Units::mtoUnit());
    props += frontspacer + strange + strlength+"\n";
    strange = QString::asprintf("   Wetted area    = %7.3g ", m_WettedArea*Units::m2toUnit());
    props += frontspacer + strange + strarea+"\n";
    strange = QString::asprintf("   Head twist   = %7.3g ", toptwist);
    props += frontspacer + strange+ DEGCHAR + "\n";
    strange = QString::asprintf("   Foot twist   = %7.3g ", bottwist);
    props += frontspacer + strange+ DEGCHAR + "\n";
    strange = QString::asprintf("   Aspect ratio   = %7.3g ", aspectRatio());
    props += frontspacer + strange + "\n";
    strange = QString::asprintf("   Triangle count = %d", int(m_RefTriangles.size()));
    props += frontspacer + strange + "\n";

    if(m_Shape.Size()<=1)
        strange = QString::asprintf("Sail is made of %d shape", m_Shape.Size());
    else
        strange = QString::asprintf("Sail is made of %d shapes", m_Shape.Size());

    props += frontspacer+strange;
    if(bFull)
    {
        props +="\n";
        for(TopTools_ListIteratorOfListOfShape shapeit(m_Shape); shapeit.More(); shapeit.Next())
        {
            occ::listShapeContent(shapeit.Value(), strange, frontspacer);
            props += frontspacer+strange;
        }
    }
}


void OccSail::flipXZ()
{
    ExternalSail::flipXZ();

    occ::flipShapesXZ(m_Shape);
    for(int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].flipXZ();
    }
    makeTriPanels(Vector3d());
}


void OccSail::scale(double XFactor, double , double )
{
    occ::scaleShapes(m_Shape, XFactor);
    m_Triangulation.scale(XFactor, XFactor, XFactor);
    for(int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].scale(XFactor, XFactor, XFactor);
    }
    makeTriPanels(Vector3d());

    m_Clew *= XFactor;
    m_Peak *= XFactor;
    m_Head *= XFactor;
    m_Tack *= XFactor;
    computeProperties();
}


void OccSail::scaleAR(double newAR)
{
    double ar = aspectRatio();
    if(ar<0.001)  return;
    if(newAR<0.001) return;

    double ratio = sqrt(newAR/ar);

    occ::scaleShapes(m_Shape, 1.0/ratio, 1.0, ratio);

    //tack is unchanged
    m_Clew.x = m_Tack.x + (m_Clew.x-m_Tack.x)/ratio;
    m_Clew.z = m_Tack.z + (m_Clew.z-m_Tack.z)*ratio;

    m_Peak.x = m_Head.x + (m_Peak.x-m_Head.x)/ratio;
    m_Peak.z = m_Tack.z + (m_Peak.z-m_Tack.z)*ratio;

    m_Head.z = m_Tack.z + (m_Head.z-m_Tack.z)*ratio;
}


void OccSail::translate(Vector3d const &T)
{
    Sail::translate(T);

    occ::translateShapes(m_Shape, T);
    makeTriPanels(Vector3d());

    computeProperties();
}


void OccSail::rotate(const Vector3d &origin, const Vector3d &axis, double theta)
{
    occ::rotateShapes(m_Shape, origin, axis, theta);
    m_Triangulation.rotate(origin, axis, theta);
    for(int it=0; it<m_RefTriangles.size(); it++)
    {
        m_RefTriangles[it].rotate(origin, axis, theta);
    }
    makeTriPanels(Vector3d());

    m_Clew.rotate(origin, axis, theta);
    m_Tack.rotate(origin, axis, theta);
    m_Peak.rotate(origin, axis, theta);
    m_Head.rotate(origin, axis, theta);

    computeProperties();
}


