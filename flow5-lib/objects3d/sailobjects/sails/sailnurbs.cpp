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




#include <Geom_BSplineSurface.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>

#define _MATH_DEFINES_DEFINED


#include <sailnurbs.h>

#include <constants.h>
#include <units.h>

SailNurbs::SailNurbs() : Sail()
{
    m_theStyle.m_Color.setRgba(195,195,235,155);
    m_nurbs.setColor(m_theStyle.m_Color);
    m_nurbs.setUAxis(2);  // is z
    m_nurbs.setVAxis(0);  // is x

    m_NZPanels = 11;
    m_ZDistrib = xfl::TANH;
    m_EdgeSplit.resize(1);
    m_EdgeSplit.front().resize(4); // 4 edges for this sail

    m_Description = "NURBS type sail";
}


void SailNurbs::makeDefaultSail()
{
    m_nurbs.clearFrames();
    m_NZPanels = 5;

    Frame firstsection;
    firstsection.appendPoint(Vector3d(0.0, 0.00, 0.0));
    firstsection.appendPoint(Vector3d(1.0, 0.41, 0.0));
    firstsection.appendPoint(Vector3d(5.0, 0.39, 0.0));
    firstsection.setZPosition(0.0);
    m_nurbs.appendFrame(firstsection);
    m_ZDistrib = xfl::UNIFORM;

    Frame secondsection;
    secondsection.appendPoint(Vector3d(0.0, 0.0, 3.5));
    secondsection.appendPoint(Vector3d(0.7, 0.23, 3.5));
    secondsection.appendPoint(Vector3d(3.5, 0.23, 3.5));
    secondsection.setZPosition(3.5);
    m_nurbs.appendFrame(secondsection);
    secondsection.setAngle(-7);

    Frame thirdsection;
    thirdsection.appendPoint(Vector3d(0.0, 0.0, 7.0));
    thirdsection.appendPoint(Vector3d(0.3, 0.1, 7.0));
    thirdsection.appendPoint(Vector3d(0.7, 0.1, 7.0));
    thirdsection.setZPosition(7.0);
    thirdsection.setAngle(-15);
    m_nurbs.appendFrame(thirdsection);

    m_nurbs.setuDegree(2);
    m_nurbs.setvDegree(2);

    makeSurface();
    makeRuledMesh(Vector3d());
    makeTriPanels(Vector3d());
}


Node SailNurbs::edgeNode(double xrel, double zrel, xfl::enumSurfacePosition ) const
{
    Node nd;
    m_nurbs.getPoint(zrel, xrel, nd);
    m_nurbs.getNormal(zrel, xrel, nd.normal());
    return nd;
}


Vector3d SailNurbs::point(double xrel, double zrel, xfl::enumSurfacePosition ) const
{
    Vector3d Pt;
    m_nurbs.getPoint(zrel, xrel, Pt);
    return Pt;
}


Vector3d SailNurbs::normal(double xrel, double zrel, xfl::enumSurfacePosition ) const
{
    Vector3d N;
    m_nurbs.getNormal(zrel, xrel, N);
    return N;
}


void SailNurbs::createSection(int iSection)
{
    if(iSection<=0)
    {
        m_nurbs.prependFrame(m_nurbs.firstFrame()); // duplicate then modify
        Frame &newframe = m_nurbs.firstFrame();
        Vector3d pos = newframe.position();
        newframe.setZPosition(pos.z-1.0);
        for(int ic=0; ic<newframe.nCtrlPoints(); ic++)
            newframe.ctrlPoint(ic).z = newframe.position().z;
    }
    else if(iSection>=int(m_nurbs.m_Frame.size()))
    {
        m_nurbs.appendFrame(m_nurbs.lastFrame()); // duplicate then modify
        Frame &newframe = m_nurbs.lastFrame();
        Vector3d pos = newframe.position();
        newframe.setZPosition(pos.z+1.0);
        for(int ic=0; ic<newframe.nCtrlPoints(); ic++)
        {
            newframe.ctrlPoint(ic).z = newframe.position().z;
        }
    }
    else
    {
        Frame &previous = m_nurbs.frame(iSection-1);
        Frame &next     = m_nurbs.frame(iSection);

        Frame newframe(previous);
        newframe.setZPosition((previous.position().z+next.position().z)  /2.0);
        newframe.setAngle((previous.angle()+previous.angle())/2.0);

        for(int ic=0; ic<newframe.nCtrlPoints(); ic++)
        {
            newframe.ctrlPoint(ic) = (previous.ctrlPoint(ic)+next.ctrlPoint(ic))/2.0;
        }
        m_nurbs.insertFrame(iSection, newframe);
    }

    makeSurface();
}


void SailNurbs::deleteSection(int iSection)
{
    if(iSection<0 || iSection>=m_nurbs.frameCount()) return;
    m_nurbs.removeFrame(iSection);
    makeSurface();
}


void SailNurbs::makeSurface()
{
    if(!m_nurbs.m_Frame.size()) return;
    if(! m_nurbs.firstFrame().nCtrlPoints()) return;

    // set unused corner points
    m_Tack = m_nurbs.firstFrame().firstControlPoint();
    m_Clew = m_nurbs.firstFrame().lastControlPoint();
    m_Clew.rotateY(m_nurbs.firstFrame().position(), m_nurbs.firstFrame().angle());
    m_Head = m_nurbs.lastFrame().firstControlPoint();
    m_Peak = m_nurbs.lastFrame().lastControlPoint();
    m_Peak.rotateY(m_nurbs.lastFrame().position(), m_nurbs.lastFrame().angle());

    //Set the number of control points in each direction
//    m_nurbs.m_nvLines = m_nurbs.m_pFrame.front()->ctrlPointCount();//assumes all sections have the same number of control points--> TODO force


    for(uint is=0; is<m_nurbs.m_Frame.size(); is++)
    {
        m_nurbs.frame(is).setTipFrame((is==0)||(is==m_nurbs.m_Frame.size()-1));
    }
    //set the knots and we're done
    m_nurbs.setKnots();
}


bool SailNurbs::serializeSailFl5(QDataStream &ar, bool bIsStoring)
{
    Sail::serializeSailFl5(ar, bIsStoring);

    int k(0), n(0);
    float xf(0),yf(0),zf(0);
    Vector3d V0, V1, V2;

    //500001: first .fl5 format
    //500002: added refpanels in beta19
    //500003: added EdgeSplits in v7.03
    int ArchiveFormat=500004;// identifies the format of the file

    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        m_nurbs.serializeFl5(ar, bIsStoring);

        ar << int(m_RefTriangles.size());
        for(uint i=0; i<m_RefTriangles.size(); i++)
        {
            Triangle3d const &t3d = m_RefTriangles.at(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }

        ar << int(m_BotMidTEIndexes.size());
        for(int idx : m_BotMidTEIndexes)   ar << idx;

        ar << int(m_TopTEIndexes.size());
        for (int idx : m_TopTEIndexes)      ar << idx;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;

        nDbleSpares=0;
        ar << nDbleSpares;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat<500001 || ArchiveFormat>500100)  return false;

        if(!m_nurbs.serializeFl5(ar, bIsStoring)) return false;
        m_nurbs.setColor(color());


        if( ArchiveFormat>=500002)
        {
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

                m_RefTriangles[i3].setTriangle(V0, V1, V2);
            }

            m_Triangulation.setTriangles(m_RefTriangles);
            m_Triangulation.makeNodes();
            m_Triangulation.makeNodeNormals();

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                m_BotMidTEIndexes.push_back(k);
            }

            ar >> n;
            for(int i=0; i<n; i++)
            {
                ar >> k;
                m_TopTEIndexes.push_back(k);
            }
            if(m_RefTriangles.size()==0) clearTEIndexes(); // clean-up past mistakes
        }


        if(ArchiveFormat<500004)
        {
            m_EdgeSplit.resize(1);
            std::vector<EdgeSplit> & es = m_EdgeSplit.front();
            es.resize(4);
            for(int iEdge=0; iEdge<4; iEdge++)
                es[iEdge].serialize(ar, bIsStoring);
        }

        // clean up past serialization errors
        if(m_EdgeSplit.size()!=1) m_EdgeSplit.resize(1); // 1 face in the case of a NURBS sail
        std::vector<EdgeSplit> & es = m_EdgeSplit.front();
        if(es.size()!=4)
        {
            es.resize(4); // A nurbs has 4 edges
            es[0].setSplit(m_NZPanels, xfl::UNIFORM);
            es[1].setSplit(m_NXPanels, xfl::UNIFORM);
            es[2].setSplit(m_NZPanels, xfl::UNIFORM);
            es[3].setSplit(m_NXPanels, xfl::UNIFORM);
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;


        makeSurface();
        updateStations();

        // compute luff angle
        Vector3d LE = m_nurbs.leadingEdgeAxis();
        m_LuffAngle = atan2(LE.x, LE.z) * 180./PI;

        return true;
    }
}


void SailNurbs::duplicate(const Sail *pSail)
{
    Sail::duplicate(pSail);
    if(!pSail->isNURBSSail()) return;
    SailNurbs const*pNSail = dynamic_cast<SailNurbs const*>(pSail);

    //deep copy
    m_nurbs.copy(pNSail->nurbs());

    m_NZPanels      = pNSail->m_NZPanels;
    m_ZDistrib      = pNSail->m_ZDistrib;

    makeSurface();
}


double SailNurbs::chord(double zrel) const
{
    return (point(1.0, zrel) - point(0.0, zrel)).norm();
}


double SailNurbs::luffLength() const
{
    if(!m_nurbs.m_Frame.size()) return 0.0;
    if(!m_nurbs.firstFrame().nCtrlPoints()) return 0.0;
    return (m_nurbs.lastFrame().firstControlPoint() - m_nurbs.firstFrame().firstControlPoint()).norm();
}


double SailNurbs::leechLength() const
{
    if(!m_nurbs.m_Frame.size()) return 0.0;
    if(!m_nurbs.firstFrame().nCtrlPoints()) return 0.0;

    if(!m_nurbs.lastFrame().nCtrlPoints() || !m_nurbs.firstFrame().nCtrlPoints()) return 0.0;

    return (m_nurbs.lastFrame().lastControlPoint()-m_nurbs.firstFrame().lastControlPoint()).norm();
}


double SailNurbs::footLength() const
{
    if(!m_nurbs.m_Frame.size() || !m_nurbs.firstFrame().nCtrlPoints()) return 0.0;
    return (m_nurbs.firstFrame().lastControlPoint()-m_nurbs.firstFrame().firstControlPoint()).norm();
}


void SailNurbs::flipXZ()
{
    Sail::flipXZ();
    for(int i=0; i<frameCount(); i++)
    {
        Frame &pFrame = frame(i);
        pFrame.setYPosition(-pFrame.position().y);
    }

    for(int i=0; i<frameCount(); i++)
    {
        Frame &pFrame = frame(i);
        for(int ic=0; ic<pFrame.nCtrlPoints(); ic++)
        {
            Vector3d &Pt = pFrame.point(ic);
            Pt.y = -Pt.y;
        }
    }
    makeSurface();
}


void SailNurbs::scale(double XFactor, double YFactor, double ZFactor)
{
    Sail::scale(XFactor, YFactor, ZFactor);

    for(int i=0; i<frameCount(); i++)
    {
        Frame &pFrame = frame(i);
        pFrame.setXPosition(pFrame.position().x * XFactor);
        pFrame.setYPosition(pFrame.position().y * YFactor);
        pFrame.setZPosition(pFrame.position().z * ZFactor);
    }

    for(int i=0; i<frameCount(); i++)
    {
        Frame &pFrame = frame(i);
        for(int ic=0; ic<pFrame.nCtrlPoints(); ic++)
        {
            Vector3d &Pt = pFrame.point(ic);
            Pt.x = Pt.x * XFactor;
            Pt.y = Pt.y * YFactor;
            Pt.z = pFrame.position().z;
        }
    }
    makeSurface();
}


void SailNurbs::translate(const Vector3d &T)
{
    Sail::translate(T);

    m_nurbs.translate(T);
    makeSurface();
}


/**
 * Returns the longitudinal position of a Frame defined by its index
 * @param iFrame the index of the Frame object
 * @return the absolute longitudinal position, in meters
 */
double SailNurbs::framePosition(int iFrame) const
{
    if(frameCount()==0) return 0.0;
    if(iFrame<0 || iFrame>=frameCount()) return 0.0;
    return m_nurbs.frameAt(iFrame).position().x;
}


double SailNurbs::twist() const
{
    if(m_nurbs.frameCount()<2) return 0.0;

    Frame const &pFirstFrame = m_nurbs.firstFrame();

    double dx = pFirstFrame.lastControlPoint().x - pFirstFrame.firstControlPoint().x;
    double dy = pFirstFrame.lastControlPoint().y - pFirstFrame.firstControlPoint().y;
    double alfa0 = atan2(dy, dx)*180.0/PI;

    Frame  const &pLastFrame = m_nurbs.lastFrame();
    dx = pLastFrame.lastControlPoint().x - pLastFrame.firstControlPoint().x;
    dy = pLastFrame.lastControlPoint().y - pLastFrame.firstControlPoint().y;
    double alfa1 = atan2(dy, dx)*180.0/PI;

    return alfa1-alfa0;
}


void SailNurbs::scaleTwist(double newtwist)
{
    double oldtwist = twist();
    if(fabs(oldtwist)<ANGLEPRECISION) return; // Nothing to scale
    double ratio = newtwist/oldtwist;
    for(int iFrame=0; iFrame<frameCount(); iFrame++)
    {
        Frame &frm = m_nurbs.frame(iFrame);
        double dx = frm.lastControlPoint().x - frm.firstControlPoint().x;
        double dy = frm.lastControlPoint().y - frm.firstControlPoint().y;
        double sectiontwist = atan2(dy, dx)*180.0/PI;
        double deltaangle = ratio*sectiontwist-sectiontwist;
        for(int ic=0; ic<frm.nCtrlPoints(); ic++)
        {
            Vector3d &pt = frm.ctrlPoint(ic);
            pt.rotateZ(frm.firstControlPoint(), deltaangle);
        }
    }
}


void SailNurbs::scaleAR(double newAR)
{
    double ar = aspectRatio();
    if(ar<0.001)  return;
    if(newAR<0.001) return;

    double ratio = sqrt(newAR/ar);

    for(int iFrame=0; iFrame<frameCount(); iFrame++)
    {
        Frame &frm = m_nurbs.frame(iFrame);
        frm.scale(1.0/ratio);
        frm.setZPosition(frm.position().z*ratio);
        for(int ic=0; ic<frm.nCtrlPoints(); ic++)
            frm.ctrlPoint(ic).z = frm.position().z;
    }
}


double SailNurbs::leadingAngle(int iSection) const
{
    if(iSection<0 || iSection>=m_nurbs.frameCount()) return 0.0;
    Frame const &pFrame = m_nurbs.frameAt(iSection);
    double dx = pFrame.pointAt(1).x - pFrame.pointAt(0).x;
    double dy = pFrame.pointAt(1).y - pFrame.pointAt(0).y;
    return atan2(dy, dx)*180.0/PI;
}


double SailNurbs::trailingAngle(int iSection) const
{
    if(iSection<0 || iSection>=m_nurbs.frameCount()) return 0.0;
    Frame const &pFrame = m_nurbs.frameAt(iSection);

    int n = pFrame.nCtrlPoints();
    double dx = pFrame.lastControlPoint().x - pFrame.pointAt(n-2).x;
    double dy = pFrame.lastControlPoint().y - pFrame.pointAt(n-2).y;
    return atan2(dy, dx)*180.0/PI;
}


void SailNurbs::setColor(const fl5Color &clr)
{
    m_theStyle.m_Color = clr;
    m_nurbs.setColor(clr);
}


bool SailNurbs::makeOccShell(TopoDS_Shape &sailshape, std::string &tracelog) const
{
    std::string strong;

    if(frameCount()<=0) return false;

    TColgp_Array2OfPnt RightPoles(0, frameCount()-1, 0, sideLineCount()-1);
    TColgp_Array2OfPnt LeftPoles( 0, frameCount()-1, 0, sideLineCount()-1);

    //------Store the control point in OCC format-----
    for(int iFrame=0; iFrame<frameCount(); iFrame++)
    {
        Frame const &pFrame = frameAt(iFrame);
        for(int j=0; j<pFrame.nCtrlPoints(); j++)
        {
            Vector3d pt = pFrame.pointAt(j);
            RightPoles.SetValue(iFrame, j, gp_Pnt(pt.x, pt.y, pt.z));
            LeftPoles.SetValue(iFrame, j, gp_Pnt(pt.x, -pt.y, pt.z));
        }
    }

    //------OCC requires that the knots are strictly crescending, so define a minimal increment.
    double eps = 1.e-4;

    //------Make the knots-----
    int p = nurbs().uDegree();
    int uSize = nurbs().uKnotCount()-2*p+2;
    TColStd_Array1OfReal    uKnots(0, uSize-1);
    TColStd_Array1OfInteger uMults(0, uSize-1);
    uKnots.SetValue(0, 0.0);
    uMults.SetValue(0, p);
    uKnots.SetValue(uSize-1, 1.0);
    uMults.SetValue(uSize-1, p);
    for(int iu=1; iu<nurbs().uKnotCount()-2*p+1; iu++)
    {
        double knot = nurbs().uKnot(p+iu-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        uKnots.SetValue(iu, knot);
        uMults.SetValue(iu, 1);
    }

    p = nurbs().vDegree();
    int vSize = nurbs().vKnotCount()-2*p+2;
    TColStd_Array1OfReal    vKnots(0, vSize-1);
    TColStd_Array1OfInteger vMults(0, vSize-1);
    vKnots.SetValue(0, 0.0);
    vMults.SetValue(0, p);
    vKnots.SetValue(vSize-1, 1.0);
    vMults.SetValue(vSize-1, p);
    for(int iv=1; iv<nurbs().vKnotCount()-2*p+1; iv++)
    {
        double knot = nurbs().vKnot(p+iv-1);
        // occ requires that the knot values are strictly increasing
        if(fabs(knot)<PRECISION)     knot=eps;
        if(fabs(1.0-knot)<PRECISION) knot=1.0-eps;
        vKnots.SetValue(iv, knot);
        vMults.SetValue(iv, 1);
    }

    //------Build the NURBS surface-----
    Handle(Geom_BSplineSurface )HSailSurface;
    try{
        HSailSurface = new Geom_BSplineSurface(RightPoles, uKnots, vKnots, uMults, vMults,
                                           nurbs().uDegree(), nurbs().vDegree());
    }
    catch (Standard_ConstructionError &e)
    {
        strong = "   Spline construction error... "+std::string(e.GetMessageString()) + "\n";
        tracelog += strong;
        return false;
    }
    catch(Standard_Failure &s)
    {
        strong = "   Standard failure... "+ std::string(s.GetMessageString())+"\n";
        tracelog += strong;
        return false;
    }
    catch (...)
    {
        tracelog += "   Unknown failure\n";
        return false;
    }

    BRepBuilderAPI_MakeShell ShellMaker(HSailSurface);
    if(!ShellMaker.IsDone())
    {
        return false;
    }
    sailshape = ShellMaker.Shell();

    return !sailshape.IsNull();
}


void SailNurbs::resizeSections(int nSections, int nPoints)
{
    nurbs().resizeFrames(nSections, nPoints);
}


double SailNurbs::length() const
{
    double l = 1.0;
    for(int i=0; i<m_nurbs.frameCount(); i++)
    {
        Frame const& fr = m_nurbs.frameAt(i);
        if(fr.nCtrlPoints()>0)
        {
            l = std::max(l, fr.firstControlPoint().distanceTo(fr.lastControlPoint()));
        }
    }
    return l;
}


bool SailNurbs::intersect(const Vector3d &A, const Vector3d &B, Vector3d &I, Vector3d &N) const
{
    double u=0, v=0;

    bool bIntersect = m_nurbs.intersect(A, B, u, v, I);
    if(bIntersect) m_nurbs.getNormal(u,v,N);
    return bIntersect;
}


