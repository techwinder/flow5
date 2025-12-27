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

#include <QString>


#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <StdFail_NotDone.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>


#include <units.h>
#include <bezierspline.h>
#include <bspline.h>
#include <cubicspline.h>
#include <pointspline.h>
#include <bspline3d.h>
#include <constants.h>
#include <occ_globals.h>
#include <sailspline.h>



SailSpline:: SailSpline(Spline::enumType type) : Sail()
{
    m_iActiveSection=0;
    m_SplineType = type;
    switch(m_SplineType)
    {
        case Spline::BSPLINE:         m_Description = "B-Spline type sail";           break;
        case Spline::CUBIC:     m_Description = "Cubic spline type sail";       break;
        case Spline::BEZIER:    m_Description = "Bezier spline type sail";      break;
        case Spline::POINT:     m_Description = "Point spline type sail";       break;
        default:                   m_Description = "Unsupported spline type";      break;
    }
    m_theStyle.m_Color.setRgba(175,155,235,155);
    m_Name = "Spline type sail";

    m_EdgeSplit.resize(1);
    m_EdgeSplit.front().resize(4);
}


SailSpline::SailSpline(SailSpline const &ssail) : Sail(ssail)
{
    m_SplineType = ssail.splineType();
    clearSections();
    for(int i=0; i<ssail.sectionCount(); i++)
    {
        Spline *pSpline = ssail.splineAt(i)->clone();
        m_Spline.push_back(pSpline);
    }
    m_Position = ssail.m_Position;
    m_Ry = ssail.m_Ry;
    m_NZPanels = ssail.m_NZPanels;
    m_ZDistrib = ssail.m_ZDistrib;

    m_EdgeSplit = ssail.m_EdgeSplit;
}


void SailSpline::makeDefaultSail()
{
    m_Spline.clear();
    m_Position.clear();
    m_Ry.clear();
    m_NZPanels = 11;
    m_ZDistrib =xfl::TANH;


    Spline *pSpline=nullptr;

    m_Position.push_back(Vector3d(0,0,0));
    m_Ry.push_back(0.0);
    switch(m_SplineType)
    {
        default:
        case Spline::BSPLINE:       pSpline=new BSpline();       break;
        case Spline::BEZIER:  pSpline=new BezierSpline();  break;
        case Spline::CUBIC:   pSpline=new CubicSpline();   break;
        case Spline::POINT:   pSpline=new PointSpline();   break;
    }
    m_Spline.push_back(pSpline);
    pSpline->resetSpline();
    pSpline->setDegree(2);
    pSpline->appendControlPoint(0.0,0.0);
    pSpline->appendControlPoint(1.5,0.1);
    pSpline->appendControlPoint(5.0,0.3);
    pSpline->updateSpline();
    pSpline->makeCurve();

    m_Position.push_back(Vector3d(0,0,7));
    m_Ry.push_back(-10.0);

    switch(m_SplineType)
    {
        default:
        case Spline::BSPLINE:       pSpline=new BSpline();       break;
        case Spline::BEZIER:  pSpline=new BezierSpline();  break;
        case Spline::CUBIC:   pSpline=new CubicSpline();   break;
        case Spline::POINT:   pSpline=new PointSpline();   break;
    }
    m_Spline.push_back(pSpline);
    pSpline->resetSpline();
    pSpline->setDegree(2);
    pSpline->appendControlPoint(0.0,0.0);
    pSpline->appendControlPoint(0.2,0.05);
    pSpline->appendControlPoint(0.5,0.1);
    pSpline->updateSpline();
    pSpline->makeCurve();

    makeSurface();
    makeRuledMesh(Vector3d());
    makeTriPanels(Vector3d());
}


SailSpline::~SailSpline()
{
    for(int is=0; is<nSections(); is++)
    {
        delete m_Spline[is];
    }
}


void SailSpline::duplicate(const Sail *pSail)
{
    Sail::duplicate(pSail);
    if(!pSail->isSplineSail()) return;

    SailSpline const *pSS = dynamic_cast<SailSpline const*>(pSail);
    m_Position = pSS->m_Position;
    m_Ry       = pSS->m_Ry;

    //  deep copy
    for(int i=0; i<nSections(); i++)  delete m_Spline[i];
    m_Spline.clear();
    for(int is=0; is<pSS->nSections(); is++)
    {
        Spline *pSpline = pSS->m_Spline[is]->clone();
        m_Spline.push_back(pSpline);
    }
    m_NZPanels     = pSS->m_NZPanels;
    m_ZDistrib     = pSS->m_ZDistrib;

    makeSurface();
}


void SailSpline::createSection(int iSection)
{
    if(iSection>=sectionCount())
    {
        m_Position.push_back(m_Position.back());
        m_Position.back().z += 0.5;
        m_Ry.push_back(m_Ry.back());
        Spline *pSpline = m_Spline.back()->clone();
        m_Spline.push_back(pSpline);
    }
    else
    {
        if(iSection>0)
        {
            // insert before so that this new section has index iSection - is what the user expects
            m_Position.insert(m_Position.begin()+iSection, Vector3d());
            m_Position[iSection].z = (m_Position[iSection-1].z+m_Position[iSection+1].z)/2.0;
            m_Ry.insert(m_Ry.begin()+iSection, 0.0);
            m_Ry[iSection] = (m_Ry[iSection-1]+m_Ry[iSection+1])/2.0;

            //make a new averaged spline
            Spline *pSpline = m_Spline.at(iSection)->clone();
            m_Spline.insert(m_Spline.begin()+iSection, pSpline);
            Spline const *bs0 = m_Spline.at(iSection-1);
            Spline const *bs1 = m_Spline.at(iSection+1);
            for(int ic=0; ic<pSpline->ctrlPointCount(); ic++)
            {
                pSpline->setCtrlPoint(ic, (bs0->controlPoint(ic)+bs1->controlPoint(ic))/2.0);
                pSpline->setWeight(ic, (bs0->weight(ic)+bs1->weight(ic))/2.0);
            }
         }
        else // iSection=0
        {
            m_Position.insert(m_Position.begin()+iSection, Vector3d());
            m_Position[iSection].z = m_Position[iSection+1].z-0.5;
            m_Ry.insert(m_Ry.begin()+iSection, 0.0);
            m_Ry[iSection] = m_Ry[iSection+1];
            Spline *pSpline = m_Spline.at(iSection)->clone();
            m_Spline.insert(m_Spline.begin()+iSection, pSpline);
        }
    }

    makeSurface();
}


void SailSpline::deleteSection(int iSection)
{
    if(iSection<0 || iSection>=sectionCount()) return;

    m_Position.erase(m_Position.begin()+iSection);
    m_Ry.erase(m_Ry.begin()+iSection);

    Spline *pSpline = m_Spline.at(iSection);
    m_Spline.erase(m_Spline.begin()+iSection);
    delete pSpline;
}


void SailSpline::insertSection(int is, Spline *pSpline, Vector3d position, double ry)
{
    if(is>=sectionCount())
    {
        m_Spline.push_back(pSpline);
        m_Position.push_back(position);
        m_Ry.push_back(ry);
    }
    else
    {
        m_Spline.insert(m_Spline.begin()+is, pSpline);
        m_Position.insert(m_Position.begin()+is, position);
        m_Ry.insert(m_Ry.begin()+is, ry);
    }
}


bool SailSpline::serializeSailFl5(QDataStream &ar, bool bIsStoring)
{
    Sail::serializeSailFl5(ar, bIsStoring);

    int k(0), n(0);
    float xf(0),yf(0),zf(0);
    Vector3d V0, V1, V2;

    //500001: first .fl5 format
    //500002: added refpanels in beta19
    //500003: v7.03 added NZPanels and ZDist arrays
    int ArchiveFormat=500003;// identifies the format of the file

    int nIntSpares=0;
    int nDbleSpares=0;

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        switch(m_SplineType)
        {
            default:
            case Spline::BSPLINE:         ar<<0;    break;
            case Spline::CUBIC:     ar<<1;    break;
            case Spline::BEZIER:    ar<<2;    break;
            case Spline::POINT:     ar<<3;    break;
            case Spline::ARC:       ar<<4;    break;
            case Spline::NACA4:     ar<<5;    break;
        }

        ar<<nSections();
        for(int i=0; i<sectionCount(); i++)
        {
            if      (m_Spline.at(i)->isBSpline())      ar<<1;
            else if (m_Spline.at(i)->isBezierSpline()) ar<<2;
            else if (m_Spline.at(i)->isCubicSpline())  ar<<3;
            else if (m_Spline.at(i)->isPointSpline())  ar<<4;
            m_Spline.at(i)->serializeFl5(ar, bIsStoring);

            ar << m_Position.at(i).x<<m_Position.at(i).y<<m_Position.at(i).z;
            ar << m_Ry.at(i);
            n=1; ar <<n;  //          ar << m_nZPanels.at(i);
            n=0; ar <<n;  //          ar <<  xfl::UNIFORM;  break;
        }

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
        for(int idx : m_TopTEIndexes)      ar << idx;

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

        ar >> n;
        switch(n)
        {
            default:
            case 0: m_SplineType = Spline::BSPLINE;             break;
            case 1: m_SplineType = Spline::CUBIC;         break;
            case 2: m_SplineType = Spline::BEZIER;        break;
            case 3: m_SplineType = Spline::POINT;         break;
            case 4: m_SplineType = Spline::ARC;           break;
            case 5: m_SplineType = Spline::NACA4;         break;
        }

        ar>>n;
        m_Spline.resize(n);
        m_Position.resize(n);
        m_Ry.resize(n);

        for(int i=0; i<n; i++)
        {
            int type=0;
            ar >> type;
            switch (type) {
                case 1:
                {
                    BSpline *pbs = new BSpline;
                    m_Spline[i] = pbs;
                    break;
                }
                case 2:
                {
                    BezierSpline *pbzs = new BezierSpline;
                    m_Spline[i] = pbzs;
                    break;
                }
                case 3:
                {
                    CubicSpline *pcs = new CubicSpline;
                    m_Spline[i] = pcs;
                    break;
                }
                case 4:
                {
                    PointSpline *pps = new PointSpline;
                    m_Spline[i] = pps;
                    break;
                }
                default:
                    return false;
            }
            m_Spline[i]->serializeFl5(ar, bIsStoring);

            ar >> m_Position[i].x >> m_Position[i].y >> m_Position[i].z;
            ar >> m_Ry[i];
            if(ArchiveFormat>=500003)
            {
                ar >> k;
                ar >> k;
            }
            else
            {
            }
        }

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
        else
        {
            makeRuledMesh(Vector3d());
        }

        // space allocation
        ar >> nIntSpares;
        ar >> nDbleSpares;

        makeSurface();
        updateStations();
        return true;
    }
}


Vector3d SailSpline::point(double xrel, double zrel, xfl::enumSurfacePosition ) const
{
    // interpolate between splines
    if(xrel<0 || xrel>1.0) return Vector3d();
 //   if(zrel<0 || zrel>1.0) return Vector3d();

    if(xrel<=0.0) xrel = 0.00001;
    if(xrel>=1.0) xrel = 0.99999;
//    if(zrel<=0.0) zrel = 0.00001;
//    if(zrel>=1.0) zrel = 0.99999;

    double z = m_Position.front().z + zrel * (m_Position.back().z-m_Position.front().z);

    // find the two splines surrounding zrel
    int isec = -1;
    double tau=1.0;
    for(int i=0; i<sectionCount()-1; i++)
    {
        if(m_Position.at(i).z<=z && z <=m_Position.at(i+1).z)
        {
            double dz =  (m_Position[i+1].z - m_Position[i].z);
            if(fabs(dz)>0.0) tau = (z-m_Position[i].z) / dz;
            else             tau = 0.0;
            isec = i;
            break;
        }
    }
    if(isec<0) return Vector3d();

    Spline const *spl0 = m_Spline.at(isec);
    Spline const *spl1 = m_Spline.at(isec+1);

    Vector2d p0, p1;
    p0 = spl0->splinePoint(xrel);
    p1 = spl1->splinePoint(xrel);

    Vector3d pt0, pt1;
    pt0.set(p0.x, p0.y, m_Position.at(isec).z);
    pt1.set(p1.x, p1.y, m_Position.at(isec+1).z);
    pt0.rotateY(m_Position.at(isec), m_Ry.at(isec));
    pt1.rotateY(m_Position.at(isec+1), m_Ry.at(isec+1));
    return Vector3d(pt0.x*(1.0-tau)+pt1.x*tau, pt0.y*(1.0-tau)+pt1.y*tau, pt0.z*(1.0-tau)+pt1.z*tau);
}


double SailSpline::luffLength() const
{
    if(m_Position.size()<2) return 1.0;
    return (m_Position.back() - m_Position.front()).norm();
}


double SailSpline::leechLength() const
{
    Vector3d pt0 = point(1.0,0.0);
    Vector3d pt1 = point(1.0,1.0);
    return (pt0-pt1).norm();
}


double SailSpline::footLength() const
{
    /** @todo make splinePoint method robust for end points */
    Vector3d pt0 = point(0.00001,0.0);
    Vector3d pt1 = point(0.99999,0.0);
    return (pt0-pt1).norm();
}


Vector3d SailSpline::leadingEdgeAxis() const
{
    if(m_Position.size()<2) return Vector3d(0,0,1); // something wrong going on

    Vector3d theTack = m_Position.front();
    Spline const *pTackSpline = m_Spline.front();
    theTack += Vector3d(pTackSpline->firstCtrlPoint().x, pTackSpline->firstCtrlPoint().y, 0.0);

    Vector3d theHead = m_Position.back();
    Spline const *pHeadSpline = m_Spline.back();
    theHead += Vector3d(pHeadSpline->firstCtrlPoint().x, pHeadSpline->firstCtrlPoint().y, 0.0);

    return (theHead-theTack).normalized();
}


void SailSpline::makeSurface()
{
    // set unused corner points
    m_Tack = point(0,0);
    m_Clew = point(1,0);
    m_Head = point(0,1);
    m_Peak = point(1,1);

    for(int i=0; i<nSections(); i++)
    {
        m_Spline[i]->updateSpline();
    }
}


void SailSpline::flipXZ()
{
    Sail::flipXZ();

    for(int i=0; i<sectionCount(); i++)
    {
        m_Position[i].y = -m_Position[i].y;
    }

    for(int i=0; i<nSections(); i++)
    {
        Spline *pSpline = m_Spline[i];
        for(int ic=0; ic<pSpline->ctrlPointCount(); ic++)
        {
            Vector2d &Pt = pSpline->controlPoint(ic);
            Pt.y = -Pt.y;
        }
        pSpline->updateSpline();
    }
    makeSurface();
}


void SailSpline::scale(double XFactor, double YFactor, double ZFactor)
{
    Sail::scale(XFactor, YFactor, ZFactor);

    // scale the sail from the tack
    Vector3d const &tack = m_Position[0];
    for(int i=0; i<sectionCount(); i++)
    {
        m_Position[i].x = tack.x + (m_Position[i].x-tack.x) * XFactor;
        m_Position[i].y = tack.y + (m_Position[i].y-tack.y) * YFactor;
        m_Position[i].z = tack.z + (m_Position[i].z-tack.z) * ZFactor;
    }

    for(int i=0; i<nSections(); i++)
    {
        Spline *pSpline = m_Spline[i];
        Vector2d const &O = pSpline->firstCtrlPoint();
        for(int ic=0; ic<pSpline->ctrlPointCount(); ic++)
        {
            Vector2d Pt = pSpline->controlPoint(ic);
            Pt.x = O.x + (Pt.x-O.x) * XFactor;
            Pt.y = O.y + (Pt.y-O.y) * YFactor;
            pSpline->setCtrlPoint(ic, Pt);
        }
        pSpline->updateSpline();
    }
    makeSurface();
}


void SailSpline::translate(const Vector3d &T)
{
    Sail::translate(T);

    for(int i=0; i<int(m_Position.size()); i++)
        m_Position[i].translate(T);
    makeSurface();
}


void SailSpline::updateActiveSpline()
{
    if(m_iActiveSection<0 || m_iActiveSection>sectionCount()) return;
    m_Spline[m_iActiveSection]->updateSpline();
    m_Spline[m_iActiveSection]->makeCurve();
}


void SailSpline::clearSections()
{
    for(int i=0; i<nSections(); i++)
    {
        delete m_Spline[i];
    }
    m_Spline.clear();
    m_Position.clear();
    m_Ry.clear();
}


void SailSpline::appendSpline(Spline *pSpline, Vector3d Pos, double Ry)
{
    m_Spline.push_back(pSpline);
    m_Position.push_back(Pos);
    m_Ry.push_back(Ry);
}


double SailSpline::twist() const
{
    Spline *pSpline = m_Spline.front();
    double dx = pSpline->lastCtrlPoint().x - pSpline->firstCtrlPoint().x;
    double dy = pSpline->lastCtrlPoint().y - pSpline->firstCtrlPoint().y;
    double alfa0 = atan2(dy, dx)*180.0/PI;

    pSpline = m_Spline.back();
    dx = pSpline->lastCtrlPoint().x - pSpline->firstCtrlPoint().x;
    dy = pSpline->lastCtrlPoint().y - pSpline->firstCtrlPoint().y;
    double alfa1 = atan2(dy, dx)*180.0/PI;

    return alfa1-alfa0;
}


void SailSpline::scaleTwist(double newtwist)
{
    double oldtwist = twist();
    if(fabs(oldtwist)<ANGLEPRECISION) return; // Nothing to scale
    double ratio = newtwist/oldtwist;
    for(int i=0; i<nSections(); i++)
    {
        Spline *pSpline = m_Spline.at(i);
        double dx = pSpline->lastCtrlPoint().x - pSpline->firstCtrlPoint().x;
        double dy = pSpline->lastCtrlPoint().y - pSpline->firstCtrlPoint().y;
        double splinetwist = atan2(dy, dx)*180.0/PI;
        double deltaangle = ratio*splinetwist-splinetwist;
        for(int ic=0; ic<pSpline->ctrlPointCount(); ic++)
        {
            Vector2d &pt = pSpline->controlPoint(ic);
            pt.rotateZ(pSpline->firstCtrlPoint(), deltaangle);
        }
        pSpline->updateSpline();
    }
    // don't; do it only once in case of consecutive scale operations
//    makeSurface();
}


void SailSpline::scaleAR(double newAR)
{
    double ar = aspectRatio();
    if(ar<0.001)  return;
    if(newAR<0.001) return;

    double ratio = sqrt(newAR/ar);

    for(int i=0; i<nSections(); i++)
    {
        Spline *pSpline = m_Spline.at(i);
        pSpline->scale(1.0/ratio);

        m_Position[i] *= ratio;
    }
}


double SailSpline::leadingAngle(int iSection) const
{
    Spline const *pSpline = m_Spline.at(iSection);
    double dx = pSpline->controlPoint(1).x - pSpline->firstCtrlPoint().x;
    double dy = pSpline->controlPoint(1).y - pSpline->firstCtrlPoint().y;
    return atan2(dy, dx)*180.0/PI;
}


double SailSpline::trailingAngle(int iSection) const
{
    Spline const *pSpline = m_Spline.at(iSection);
    int n = pSpline->ctrlPointCount();
    double dx = pSpline->lastCtrlPoint().x - pSpline->controlPoint(n-2).x;
    double dy = pSpline->lastCtrlPoint().y - pSpline->controlPoint(n-2).y;
    return atan2(dy, dx)*180.0/PI;
}


void SailSpline::setColor(const fl5Color &clr)
{
    m_theStyle.m_Color = clr;
    for(int is=0; is<nSections(); is++)
        m_Spline[is]->setColor(clr);
}


//BSPLINE, CUBICSPLINE, BEZIERSPLINE, POINTSPLINE, ARCSPLINE, NACA4SPLINE, OTHERSPLINE
void SailSpline::convertSplines(Spline::enumType newtype)
{
    std::vector<Spline*> oldsplines = m_Spline; // copy the pointers
    m_Spline.clear();

    for(Spline *pOldSpline : oldsplines)
    {
        Spline *pNewSpline=nullptr;
        switch (newtype)
        {
            default:
            case Spline::BSPLINE:            pNewSpline = new BSpline;               break;
            case Spline::BEZIER:       pNewSpline = new BezierSpline;          break;
            case Spline::CUBIC:        pNewSpline = new CubicSpline;           break;
            case Spline::POINT:        pNewSpline = new PointSpline;           break;
         }
        pNewSpline->setCtrlPoints(pOldSpline->ctrlPts());
        pNewSpline->updateSpline();
        pNewSpline->makeCurve();
        m_Spline.push_back(pNewSpline);
        delete pOldSpline;
    }

    m_SplineType = newtype;
}


void SailSpline::resizeSections(int nSections, int nPoints)
{
    // keep the top and bottom sections, and recreate as many as required in between

    int N = sectionCount();
    for(int is=N-2; is>0; is--)
    {
        deleteSection(is);
    }

    int nnew = nSections-2;
    Spline *pBot = spline(0);
    Spline *pTop = spline(N-1);
    Vector3d bot = sectionPosition(0);
    Vector3d top = sectionPosition(N-1);
    double ry0 = sectionAngle(0);
    double ry1 = sectionAngle(N-1);
    for(int j=0; j<nnew; j++)
    {
        double tau = double(j+1)/double(nnew+1);
        Vector3d newpos = bot + (top-bot) * tau;
        double ry = ry0 + tau*(ry1-ry0);
        Spline *pSpline = pBot->clone();
        for(int ic=0; ic<int(pSpline->ctrlPointCount()); ic++)
        {
            Vector2d pt = pSpline->controlPoint(ic);
            pt = pBot->controlPoint(ic) + (pTop->controlPoint(ic)-pBot->controlPoint(ic))*tau;
            pSpline->setCtrlPoint(ic, pt);
        }
        insertSection(j+1, pSpline, newpos, ry);
    }
    setActiveSection(0);


    //resize the number of points;
    for(int is=0; is<sectionCount(); is++)
    {
        Spline *pSpline = spline(is);
        pSpline->resizeControlPoints(nPoints);
        pSpline->updateSpline();
        pSpline->makeCurve();
    }
}


double SailSpline::length() const
{
    double l = 1.0;
    for(int i=0; i<nSections(); i++)
    {
        Spline const *pSpline = m_Spline.at(i);

        if(pSpline->ctrlPointCount()>0)
        {
            l = std::max(l, pSpline->firstCtrlPoint().distanceTo(pSpline->lastCtrlPoint()));
        }
    }
    return l;
}


bool SailSpline::makeOccShell(TopoDS_Shape &sailshape, std::string &logmsg) const
{
    QString logg;
    double stitchprecision = 1.e-6;
    QString strong = "Processing wing "+ QString::fromStdString(m_Name) + "\n";
    logg += strong;

    std::string occstr;
    BRepBuilderAPI_Sewing stitcher(stitchprecision);

    BSpline3d b3d0, b3d1;
    TopoDS_Wire Wire0, Wire1;

    for(int iSpline=0; iSpline<sectionCount()-1; iSpline++)
    {
        switch(m_SplineType)
        {
            case Spline::BSPLINE:
            {
                makeBSpline3d(iSpline,   b3d0);
                makeBSpline3d(iSpline+1, b3d1);
                if(!occ::makeSplineWire(b3d0, Wire0, occstr))
                {
                    logg += QString::asprintf("   Error making spline wire %d", iSpline);
                    return false;
                }

                logg += QString::fromStdString(occstr);
                occstr.clear();

                if(!occ::makeSplineWire(b3d1, Wire1, occstr))
                {
                    logg += QString::asprintf("   Error making spline wire %d", iSpline+1);
                    return false;
                }

                logg += QString::fromStdString(occstr);
                occstr.clear();

                break;
            }
            default:
            {
                Vector3d pt3d;
                Spline const *spl0 = m_Spline.at(iSpline);
                Spline const *spl1 = m_Spline.at(iSpline+1);

                BRepBuilderAPI_MakePolygon PolyMaker0;
                for(int ic=0; ic<spl0->ctrlPointCount(); ic++)
                {
                    Vector2d const & pt = spl0->controlPoint(ic);

                    pt3d.set(pt.x, pt.y, m_Position.at(iSpline).z);
                    pt3d.rotateY(m_Position.at(iSpline), m_Ry.at(iSpline));
                    PolyMaker0.Add(gp_Pnt(pt3d.x, pt3d.y, pt3d.z));
                }
                Wire0 = PolyMaker0.Wire();
                if(!PolyMaker0.IsDone() || Wire0.IsNull())
                {
                    logg += QString::asprintf("   error making wire %d\n", iSpline);
                    return false;
                }

                BRepBuilderAPI_MakePolygon PolyMaker1;
                for(int ic=0; ic<spl1->ctrlPointCount(); ic++)
                {
                    Vector2d const & pt = spl1->controlPoint(ic);

                    pt3d.set(pt.x, pt.y, m_Position.at(iSpline+1).z);
                    pt3d.rotateY(m_Position.at(iSpline+1), m_Ry.at(iSpline+1));
                    PolyMaker1.Add(gp_Pnt(pt3d.x, pt3d.y, pt3d.z));
                }
                Wire1 = PolyMaker1.Wire();
                if(!PolyMaker1.IsDone() || Wire1.IsNull())
                {
                    logg += QString::asprintf("   error making wire %d\n", iSpline+1);
                    return false;
                }
            }
        }

        // Sweep
        BRepOffsetAPI_ThruSections Sweeper(false, false, 1.0e-4);
/*        Sweeper.CheckCompatibility(true);
        Sweeper.SetSmoothing(false);
        Sweeper.SetParType(Approx_IsoParametric);
        Sweeper.SetContinuity(GeomAbs_C0); */

        Sweeper.AddWire(Wire0);
        Sweeper.AddWire(Wire1);

        try
        {
//            Sweeper.Build(); // does nothing according to OCC doc
            TopoDS_Shape TopSweptShape = Sweeper.Shape(); // calls Build according to OCC doc
            stitcher.Add(TopSweptShape);
        }
        catch(Standard_DomainError &e)
        {
            logg += QString("     Standard_DomainError sweeping wires\n") + QString(e.GetMessageString()) + "\n";
        }
        catch (StdFail_NotDone &e)
        {
            logg += QString("   StdFail_NotDone sweeping wires") + QString(e.GetMessageString()) + "\n";
            return false;
        }
        catch (...)
        {
            logg += "   Unknown error sweeping section wires\n";
            return false;
        }
    }

    // stitch
    stitcher.Perform();

//    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
//    After a successful sewing operation all faces have a coherent orientation.

    try
    {
        sailshape = stitcher.SewedShape();
    }
    catch(Standard_TypeMismatch const &)
    {
        logg += "     SailShapes:: Type mismatch error\n";
        return false;
    }

    logg += "\n";

    logmsg = logg.toStdString();
    return !sailshape.IsNull();
}


bool SailSpline::makeBSpline3d(int ispl, BSpline3d &spline3d) const
{
    if(ispl<0 || ispl>=sectionCount()) return false;

    Spline const *spl = m_Spline.at(ispl);
    BSpline const *bspl = dynamic_cast<BSpline const*>(spl);
    if(!bspl) return false;

    spline3d.resizeControlPoints(spl->ctrlPointCount());

    for(int ic=0; ic<spl->ctrlPointCount(); ic++)
    {
        Vector2d const & pt = spl->controlPoint(ic);
        Vector3d &pt3d = spline3d.controlPoint(ic);
        pt3d.set(pt.x, pt.y, m_Position.at(ispl).z);
        pt3d.rotateY(m_Position.at(ispl), m_Ry.at(ispl));
    }
    spline3d.setDegree(bspl->degree());
    spline3d.updateSpline();
    return !spline3d.isSingular();
}


Vector3d SailSpline::sectionPoint(int ispl, int iPt) const
{
    Spline const *spl = m_Spline.at(ispl);
    Vector2d const & pt = spl->controlPoint(iPt);
    Vector3d pt3d;
    pt3d.set(pt.x, pt.y, m_Position.at(ispl).z);
    pt3d.rotateY(m_Position.at(ispl), m_Ry.at(ispl));
    return pt3d;
}






