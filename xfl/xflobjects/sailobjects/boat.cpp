/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include "boat.h"
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>
#include <xflobjects/sailobjects/sails/occsail.h>
#include <xflobjects/sailobjects/sails/stlsail.h>
#include <xflobjects/objects3d/analysis/boatpolar.h>
#include <xflobjects/objects3d/analysis/boatopp.h>
#include <xflobjects/objects3d/fuse/fuse.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflobjects/objects3d/fuse/fusestl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/fuse/fusenurbs.h>
#include <xflobjects/objects3d/fuse/fuseflatfaces.h>
#include <xflobjects/objects3d/fuse/fusesections.h>



Boat::Boat()
{
    m_Sail.clear();
    m_Hull.clear();
    m_Name = "The boat's name";

    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 2;
}


Boat::Boat(Boat const &aboat)
{
    m_Name = aboat.m_Name;
    m_Description = aboat.m_Description;

    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 2;

    // deep copy sails and hulls
    m_Sail.clear();
    m_Hull.clear();
    for(int is=0; is<aboat.m_Sail.size(); is++)
    {
        Sail *pSail = aboat.sailAt(is)->clone();
        m_Sail.append(pSail);
    }

    m_Hull.clear();
    for(int ih=0; ih<aboat.m_Hull.size(); ih++)
    {
        Fuse *pHull = aboat.hullAt(ih)->clone();
        m_Hull.append(pHull);
    }
}


Boat::~Boat()
{
    for(int ih=0; ih<m_Hull.size(); ih++)
    {
        delete m_Hull[ih];
    }
    m_Hull.clear();

    for(int is=0; is<m_Sail.size(); is++)
    {
        delete m_Sail[is];
    }
    m_Sail.clear();
}


/** Makes the reference triangular mesh, with sails in their rest position */
void Boat::makeRefTriMesh(bool bIncludeHull, bool bMultiThread)
{
    (void)bMultiThread;

    m_RefTriMesh.clearMesh();

    int p0=0;
    for(int is=0; is<m_Sail.size(); is++)
    {
        Sail *pSail = m_Sail[is];

        pSail->makeTriPanels(pSail->m_LE);

        m_RefTriMesh.appendMesh(pSail->triMesh());
        pSail->setFirstPanel3Index(p0);
        p0 += pSail->nPanel3();
    }

    for(int i3=0; i3<m_RefTriMesh.nPanels(); i3++)
        m_RefTriMesh.panel(i3).setIndex(i3);

    // set the trailing edges
    for(int is=0; is<m_Sail.size(); is++)
    {
        Sail const *pSail = m_Sail[is];
        int n0 = pSail->firstPanel3Index();
        for(int i3=0; i3<pSail->nPanel3(); i3++)
        {
            Panel3 const &p3s = pSail->triMesh().panelAt(i3);
            if(p3s.isTrailing())
            {
                Panel3 &p3bt = m_RefTriMesh.panel(n0 + p3s.index());
                p3bt.setOppositeIndex(n0 + p3s.oppositeIndex());
            }
        }
    }

    if(bIncludeHull)
    {
        for(int ihull=0; ihull<fuseCount(); ihull++)
        {
            Fuse *pFuse = m_Hull.at(ihull);
            if(!pFuse) continue;

            pFuse->setFirstPanel3Index(m_RefTriMesh.nPanels());
            pFuse->setFirstNodeIndex(m_RefTriMesh.nNodes());

            int p3index = m_RefTriMesh.nPanels();
            // global resize faster than pushing the panels one by one
            m_RefTriMesh.panels().resize(m_RefTriMesh.nPanels()+pFuse->nPanel3());

            // set the panel and nodes one by one with new indexes and positions
            Node S[3];
            int n0 = m_RefTriMesh.nNodes();
            for(int i3=0; i3<pFuse->nPanel3(); i3++)
            {
                Panel3 const &pf3 = pFuse->panel3At(i3);
                for(int in=0; in<3; in++)
                {
                    S[in].setNode(pf3.vertexAt(in));
                    S[in].translate(m_Hull.at(ihull)->position());
                }

                m_RefTriMesh.setPanel(p3index, Panel3(S[0], S[1], S[2]));

                Panel3 &p3 = m_RefTriMesh.panel(p3index);
                p3.setSurfacePosition(xfl::FUSESURFACE);
                p3.setIndex(p3index);
                p3.setNodeIndexes(pf3.nodeIndex(0)+n0, pf3.nodeIndex(1)+n0, pf3.nodeIndex(2)+n0);

                p3index++;
            }

            int ndindex = n0;
            m_RefTriMesh.nodes().resize(m_RefTriMesh.nNodes()+pFuse->nodes().size());

            for(int in=0; in<pFuse->panel3NodeCount(); in++)
            {
                m_RefTriMesh.setNode(ndindex, pFuse->panel3Node(in));
                m_RefTriMesh.node(ndindex).translate(m_Hull.at(ihull)->position());
                m_RefTriMesh.node(ndindex).setIndex(ndindex);
                ndindex++;
            }
        }
    }

    m_RefTriMesh.setNodePanels();
}


void Boat::makeConnections()
{
    for(int is=0; is<m_Sail.size(); is++)
    {
        Sail const*pSail = m_Sail[is];
        m_RefTriMesh.makeConnectionsFromNodePosition(pSail->firstPanel3Index(), pSail->nPanel3(), 0.0001, true);
//        m_RefTriMesh.makeConnectionsFromNodeIndexes(pSail->firstPanel3Index(), pSail->nPanel3(), pSail->firstPanel3Index(), pSail->nPanel3());
    }
}


void Boat::makeDefaultBoat()
{
    NURBSSail *pSail = appendNewNurbsSail();
    pSail->setName("Nurbs sail");
    pSail->makeDefaultSail();
    pSail->setPosition(Vector3d(0.0,0.0,0.5));

    Fuse *pHull = appendNewXflHull();
    pHull->setPartName("the Hull");
    pHull->makeFuseGeometry();
    QString logmsg;
    pHull->makeDefaultTriMesh(logmsg, QString());
}


Sail *Boat::sail(const QString &SailName)
{
    for(int is=0; is<m_Sail.size(); is++)
    {
        Sail* pSail = m_Sail.at(is);
        if(pSail->name()==SailName) return pSail;
    }
    return nullptr;
}


Fuse *Boat::hull(const QString &BodyName) const
{
    for(int is=0; is<m_Hull.size(); is++)
    {
        Fuse* pBody = m_Hull.at(is);
        if(pBody->name()==BodyName) return pBody;
    }
    return nullptr;
}


bool Boat::serializeBoatFl5(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;// identifies the format of the file
    int k(0), n(0);
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {    // storing code
        ar << 500009;
        //500001: initial format
        //500002: added theStyle in beta 17

        ar << m_Name;
        ar << m_Description;

        m_theStyle.serializeFl5(ar, bIsStoring);

        ar << int(m_Sail.size());
        for(int is=0; is<m_Sail.size(); is++)
        {
            Sail *pSail = m_Sail.at(is);
            if(!pSail) return false;

            if     (pSail->isSplineSail()) ar<<1;
            else if(pSail->isNURBSSail())  ar<<2;
            else if(pSail->isWingSail())   ar<<3;
            else if(pSail->isStlSail())    ar<<4;
            else if(pSail->isOccSail())    ar<<5;
            else                           ar<<0;

            if(!pSail->serializeSailFl5(ar, true)) return false;
        }

        ar << int(m_Hull.size());

        for(int ifuse=0; ifuse<m_Hull.size(); ifuse++)
        {
            Fuse *pFuse = hull(ifuse);
            if(pFuse->isXflType())
            {
                if(pFuse->fuseType()==Fuse::NURBS) ar << 100001;
                else                               ar << 100004; // flat faces
                FuseXfl *pBody = dynamic_cast<FuseXfl*>(pFuse);
                pBody->serializePartFl5(ar, true);
            }
            else if(pFuse->isOccType())
            {
                ar << 100002;
                FuseOcc *pBodyOcc = dynamic_cast<FuseOcc*>(pFuse);
                pBodyOcc->serializePartFl5(ar, true);
            }
            else if(pFuse->isStlType())
            {
                ar << 100003;
                FuseStl *pBodyStl = dynamic_cast<FuseStl*>(pFuse);
                pBodyStl->serializePartFl5(ar, true);
            }
            pFuse->triMesh().serializePanelsFl5(ar, bIsStoring);
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;
        if (ArchiveFormat<500000 || ArchiveFormat>500100)  return false;

        ar >> m_Name;
        ar >> m_Description;
        if(ArchiveFormat>=500002)
            m_theStyle.serializeFl5(ar, bIsStoring);

        ar>>n;
        for(int is=0; is<n; is++)
        {
            ar >> k;
            if(k==1)
            {
                SplineSail *pSSail = new SplineSail;
                if(!pSSail->serializeSailFl5(ar, false)) return false;
                m_Sail.append(pSSail);
            }
            else if(k==2)
            {
                NURBSSail *pNSail = new NURBSSail;
                if(!pNSail->serializeSailFl5(ar, false)) return false;
                m_Sail.append(pNSail);
            }
            else if(k==3)
            {
                WingSail *pWSail = new WingSail;
                if(!pWSail->serializeSailFl5(ar, false)) return false;
                m_Sail.append(pWSail);
            }
            else if(k==4)
            {
                STLSail *pStlSail = new STLSail;
                if(!pStlSail->serializeSailFl5(ar, false)) return false;
                m_Sail.append(pStlSail);
            }
            else if(k==5)
            {
                OccSail *pOccSail = new OccSail;
                if(!pOccSail->serializeSailFl5(ar, false)) return false;
                m_Sail.append(pOccSail);
            }
            else return false;
        }

        int nFuse(0);
        ar >> nFuse;
        clearHulls();

        for(int ifuse=0; ifuse<nFuse; ifuse++)
        {
            int format=0;
            ar >> format;
            if(format==100001)
            {
                FuseNurbs *pBody = new FuseNurbs;
                pBody->serializePartFl5(ar, bIsStoring);
                if(pBody) m_Hull.append(pBody);
            }
            else if(format==100004)
            {
                FuseFlatFaces *pBody = new FuseFlatFaces;
                pBody->serializePartFl5(ar, bIsStoring);
                if(pBody) m_Hull.append(pBody);
            }
            else if(format==100002)
            {
                FuseOcc *pBodyOcc = new FuseOcc;
                pBodyOcc->serializePartFl5(ar, bIsStoring);
                if(pBodyOcc) m_Hull.append(pBodyOcc);
            }
            else if(format==100003)
            {
                FuseStl *pBodyStl = new FuseStl;
                pBodyStl->serializePartFl5(ar, bIsStoring);
                if(pBodyStl) m_Hull.append(pBodyStl);
            }
            m_Hull.last()->triMesh().serializePanelsFl5(ar, bIsStoring);
            m_Hull.last()->saveBaseTriangulation();

            // compatibility
            if(m_Hull.last()->nPanel3()==0)
            {
                QString strange;
                m_Hull.last()->makeDefaultTriMesh(strange, QString());
            }
            m_Hull.last()->setUniqueIndex();
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        return true;
    }
}


void Boat::duplicate(Boat const*pBoat)
{
    clearSails();
    for(int is=0; is<pBoat->m_Sail.size(); is++)
    {
/*        Sail *pNewSail = nullptr;
        Sail *pSail = pBoat->m_Sail.at(is);
        if     (pSail->isNURBSSail())  pNewSail = new NURBSSail;
        else if(pSail->isSplineSail()) pNewSail = new SplineSail;

        if(pNewSail)
        {
            pNewSail->duplicate(pSail);
            m_Sail.append(pNewSail);
        }*/
        Sail *pSail = pBoat->m_Sail.at(is)->clone();
        m_Sail.append(pSail);
    }

    clearHulls();
    for(int ih=0; ih<pBoat->m_Hull.size(); ih++)
    {
        Fuse *pHull = pBoat->m_Hull.at(ih)->clone();
        m_Hull.append(pHull);
    }

    m_Name = pBoat->m_Name;
    m_Description = pBoat->m_Description;
    m_theStyle = pBoat->m_theStyle;
}


void Boat::clearSails()
{
    for(int iSail=0; iSail<m_Sail.size(); iSail++)
    {
        delete m_Sail[iSail];
    }
    m_Sail.clear();
}

void Boat::clearHulls()
{
    for(int ihull=0; ihull<m_Hull.size(); ihull++)
    {
        delete m_Hull[ihull];
    }
    m_Hull.clear();
}


FuseXfl *Boat::appendNewXflHull()
{
    FuseNurbs *pHullXfl = new FuseNurbs();
    pHullXfl->makeDefaultHull();
    m_Hull.append(pHullXfl);
    return pHullXfl;
}


bool Boat::removeHull(int iHull)
{
    if(iHull>=0 && iHull<m_Hull.size())
    {
        delete m_Hull[iHull];
        m_Hull.removeAt(iHull);
        return true;
    }
    return false;
}




NURBSSail *Boat::appendNewNurbsSail()
{
    NURBSSail *pSail = new NURBSSail;
    m_Sail.append(pSail);
    return pSail;
}


void Boat::setHullLE(int iHull, Vector3d const &LEPos)
{
    if(iHull>=0 && iHull<m_Hull.size())
        m_Hull[iHull]->setPosition(LEPos);
}


Vector3d Boat::hullLE(int iHull)
{
    if(iHull>=0 && iHull<m_Hull.size())
        return m_Hull.at(iHull)->position();
    return Vector3d();
}


double Boat::length() const
{
    double l=0.0;
    if(m_Hull.size())
    {
        for(int ih=0; ih<m_Hull.size(); ih++)
        {
            if(m_Hull.at(ih)->length()>l) l = m_Hull.at(ih)->length();
        }
    }

    double xmin = 1e10;
    double xmax = -1e10;
    for (int is=0; is<m_Sail.size(); is++)
    {
        Sail const *pSail = m_Sail.at(is);
        xmax = std::max(pSail->position().x + pSail->m_Clew.x, xmax);
        xmin = std::min(pSail->position().x + pSail->m_Tack.x, xmin);
    }

    return std::max(l, xmax-xmin);
}


/**  Returns and approximation of the boat's height */
double Boat::height() const
{
    double maxh = 0.0;
    for (int is=0; is<m_Sail.size(); is++)
    {
        Sail const *pSail = m_Sail.at(is);
        maxh = std::max(maxh, pSail->luffLength() + pSail->position().z);
    }
    return maxh;
}


Vector3d Boat::fusePos(int idx) const
{
    return m_Hull.at(idx)->position();
}


void Boat::setFusePos(int idx, Vector3d pos)
{
    m_Hull[idx]->setPosition(pos);
}


Fuse * Boat::makeNewHull(Fuse::enumType bodytype)
{
    Fuse *pFuse = nullptr;
    switch(bodytype)
    {
        case Fuse::FlatFace:
        {
            FuseFlatFaces *pFuseXfl = new FuseFlatFaces();
            pFuseXfl->makeDefaultHull();
            pFuse = pFuseXfl;
            break;
        }
        default:
        case Fuse::NURBS:
        {
            FuseNurbs *pFuseXfl = new FuseNurbs();
            pFuseXfl->makeDefaultHull();
            pFuse = pFuseXfl;
            break;
        }
        case Fuse::Sections:
        {
            FuseSections *pFuseXfl = new FuseSections();
            pFuseXfl->makeDefaultHull();
            pFuse = pFuseXfl;
            break;
        }
        case Fuse::Occ:
        {
            pFuse = new FuseOcc;
            break;
        }
        case Fuse::Stl:
        {
            pFuse = new FuseStl;
            break;
        }
    }
    if(pFuse) appendHull(pFuse);
    return pFuse;
}


void Boat::removeFuse(Fuse *pFuse)
{
    if(!pFuse) return;
    for(int ifuse=0; ifuse<hullCount(); ifuse++)
    {
        if(hull(ifuse)==pFuse)
        {
            m_Hull.removeAt(ifuse);
            delete pFuse;

            return;
        }
    }
}


void Boat::appendSail(Sail *pSail)
{
    if(!pSail) return;
    pSail->setUniqueIndex();
    m_Sail.append(pSail);
}


bool Boat::removeSail(int iSail)
{
    if(iSail>=0 && iSail<m_Sail.size())
    {
        delete m_Sail[iSail];
        m_Sail.removeAt(iSail);
        return true;
    }
    return false;
}


void Boat::appendHull(Fuse *pFuse)
{
    if(!pFuse) return;
    pFuse->setUniqueIndex();
    m_Hull.append(pFuse);
}


int Boat::xflFuseCount() const
{
    int count=0;
    for(int ifuse=0; ifuse<nHulls(); ifuse++)
    {
        if(m_Hull.at(ifuse)->isXflType()) count++;
    }
    return count;
}

int Boat::occFuseCount() const
{
    int count=0;
    for(int ifuse=0; ifuse<nHulls(); ifuse++)
    {
        if(m_Hull.at(ifuse)->isOccType()) count++;
    }
    return count;
}

int Boat::stlFuseCount() const
{
    int count=0;
    for(int ifuse=0; ifuse<nHulls(); ifuse++)
    {
        if(m_Hull.at(ifuse)->isStlType()) count++;
    }
    return count;
}



bool Boat::hasBtPolar(BoatPolar const *pBtPolar) const {return pBtPolar->boatName().compare(m_Name)==0;}
bool Boat::hasBOpp(BoatOpp const *pBOpp) const  {return pBOpp->boatName().compare(m_Name)==0;}


Sail *Boat::mainSail()
{
    if(m_Sail.size()==0) return nullptr;
        return m_Sail.front();
}

Sail const *Boat::mainSail() const
{
    if(m_Sail.size()==0) return nullptr;
        return m_Sail.front();
}

Sail *Boat::jib()
{
    if(m_Sail.size()<2) return nullptr;
    return m_Sail.at(1);

}

Sail const *Boat::jib() const
{
    if(m_Sail.size()<2) return nullptr;
    return m_Sail.at(1);

}

Fuse * Boat::hull()
{
    if(m_Hull.size()<=0) return nullptr;
    return m_Hull.at(0);
}

Fuse const * Boat::hull() const
{
    if(m_Hull.size()<=0) return nullptr;
    return m_Hull.at(0);
}


QString Boat::properties(bool bFull) const
{
    QString props, strange, frontspacer;
    QString format = xfl::isLocalized() ? "%L1" : "%1";

    if(m_Description.length())
        props = m_Description +"\n";

    if(m_Sail.size()<=1)
        props += QString::asprintf("Boat is made of %d sail\n", int(m_Sail.size()));
    else
        props += QString::asprintf("Boat is made of %d sails\n", int(m_Sail.size()));


    if(bFull)
    {
        for(int is=0; is<m_Sail.size(); is++)
        {
            m_Sail.at(is)->properties(strange, frontspacer);
            props += strange;
            props += "\n";
        }
    }
    else
    {
        if(m_Sail.size()>0)
        {
            props += QString("Main sail area  = "+format).arg(m_Sail.at(0)->refArea()*Units::m2toUnit());
            props += " "+Units::areaUnitLabel() + "\n";
        }
        if(m_Sail.size()>1)
        {
            props += QString("Jib area        = "+format).arg(m_Sail.at(1)->refArea()*Units::m2toUnit());
            props += " "+Units::areaUnitLabel() + "\n";
        }
    }

    strange = QString::asprintf("Triangle panels = %d", refTriMesh().nPanels());
    props += strange;

    return props;
}


void Boat::rotateMesh(BoatPolar const*pBtPolar, double phi, double Ry, double ctrl, QVector<Panel3> &panels) const
{
    if(!pBtPolar) return;

    //rotate the sails around their leading edges
    for(int is=0; is<sailCount(); is++)
    {
        Sail const*pSail = sailAt(is);
        Vector3d axis = pSail->leadingEdgeAxis().normalized();
        Vector3d tack = pSail->position() + pSail->tack();

        double angle = pBtPolar->sailAngle(is, ctrl);
        if(fabs(angle)>ANGLEPRECISION)
        {
            if(pBtPolar->isTriangleMethod())
            {
                for(int ip=pSail->firstPanel3Index(); ip<pSail->firstPanel3Index()+pSail->nPanel3(); ip++)
                {
                    panels[ip].rotate(tack, axis, angle);
                }
            }
        }
    }

    //apply the rotation angle around the Y axis
    if(pBtPolar->isTriangleMethod())
    {
        Vector3d O(0.0,0.0,0.0);
        if(fabs(Ry)>ANGLEPRECISION)
        {
            for(int i3=0; i3<panels.size(); i3++)
                panels[i3].rotate(O,Vector3d(0.0,1.0,0.0), Ry);
        }

        //apply the bank angle

        if(fabs(phi)>ANGLEPRECISION)
        {
            for(int i3=0; i3<panels.size(); i3++)
                panels[i3].rotate(O,Vector3d(1.0,0.0,0.0), phi);
        }
    }
}


void Boat::triangulateSails()
{
    for(int i=0; i<m_Sail.size(); i++)
        m_Sail[i]->makeTriangulation();
}

