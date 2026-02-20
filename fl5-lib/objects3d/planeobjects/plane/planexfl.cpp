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

#include <QElapsedTimer>
#include <QString>
#include <QDebug>
#include <QCoreApplication>

#include <planexfl.h>
#include <fusenurbs.h>
#include <fuseflatfaces.h>
#include <fusesections.h>
#include <fuseocc.h>
#include <fusestl.h>
#include <surface.h>
#include <pointmass.h>
#include <wingxfl.h>
#include <planepolar.h>
#include <planeopp.h>
#include <geom_global.h>
#include <panel3.h>
#include <panel4.h>
#include <units.h>
#include <utils.h>
#include <constants.h>



PlaneXfl::PlaneXfl(bool bDefaultPlane) : Plane()
{
    m_bThickBuild = true; // to be consistent with legacy builds <7.54
    if(bDefaultPlane) makeDefaultPlane();
}



PlaneXfl::PlaneXfl(const PlaneXfl &aPlane)
{
    m_bLocked = false;

    m_Wing.clear();

    clearPointMasses();

    m_Name  = "Plane name";

    m_bIsInitialized = false;

    duplicate(&aPlane);
}


void PlaneXfl::makeDefaultPlane()
{
    int NWings = 3;
    m_PartIndexes.clear();

    clearFuse();


    m_Wing.clear();
    for(int iw=0; iw<NWings; iw++) addWing();

    makeUniqueIndexList();

    m_Wing[0].setName("Main wing");
    m_Wing[0].makeDefaultWing();
    m_Wing[0].computeGeometry();

    m_Wing[1].setName("Elevator");
    m_Wing[1].makeDefaultStab();
    m_Wing[1].computeGeometry();

    m_Wing[2].setName("Fin");
    m_Wing[2].makeDefaultFin();
    m_Wing[2].computeGeometry();

    m_Wing[0].setPosition(0.400, 0.000, 0.000);

    m_Wing[1].setPosition(1.350, 0.000, 0.025);
    m_Wing[1].setRy(-1.5);

    m_Wing[2].setPosition(1.350, 0.000, 0.050);
    m_Wing[2].setRx(m_Wing[2].isFin() ? -90 : 0.0);

    m_Inertia.reset();

    clearPointMasses();

    m_Name  = "Plane name";

    m_bIsInitialized = false;
}


PlaneXfl::~PlaneXfl()
{
    clearPointMasses();
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        delete m_Fuse[ifuse];
    }
}

int PlaneXfl::nStations() const
{
    int n = 0;
    for(int is=0; is<nWings(); is++)
        n += m_Wing.at(is).nStations();
    return n;
}


/**
* Calculates and returns the Plane's tail volume = lever_arm_elev x Area_Elev / MAC_Wing / Area_Wing
*/
double PlaneXfl::tailVolumeHorizontal() const
{
    double HTV=0.0;

    WingXfl const *pMainWing=nullptr;
    WingXfl const *pStab = nullptr;
    Vector3d MainWingLE, StabLE;
    for(int iw=0; iw<nWings(); iw++)
    {
        if (m_Wing[iw].wingType()==xfl::Main)
        {
            pMainWing = &m_Wing[iw];
            MainWingLE = m_Wing[iw].position();
        }

        if (m_Wing[iw].wingType()==xfl::Elevator)
        {
            pStab = &m_Wing[iw];
            StabLE = m_Wing[iw].position();
        }
    }

    if(pMainWing && pStab)
    {
        double SLA = StabLE.x + pStab->chord(0)/4.0 - (MainWingLE.x + pMainWing->chord(0)/4.0);
        double area = pMainWing->projectedArea();
        //        if(m_bBiplane) area += m_Wing[1].m_ProjectedArea;

        double ProjectedArea = 0.0;
        for (int i=0;i<pStab->nSections()-1; i++)
        {
            ProjectedArea += pStab->sectionLength(i+1)*(pStab->chord(i)+pStab->chord(i+1))/2.0
                    *cos(pStab->dihedral(i)*PI/180.0)*cos(pStab->dihedral(i)*PI/180.0);//m2

        }
        ProjectedArea *=2.0;
        HTV = ProjectedArea * SLA / area/pMainWing->MAC();
    }
    else HTV = 0.0;

    return HTV;
}


/**
* Calculates and returns the Plane's tail volume = lever_arm_elev x Area_Elev / MAC_Wing / Area_Wing
*/
double PlaneXfl::tailVolumeVertical() const
{
    double VTV=0.0;

    WingXfl const *pMainWing=nullptr;
    WingXfl const *pFin = nullptr;
    Vector3d MainWingLE, FinLE;
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const *pWing = wingAt(iw);
        if (pWing->wingType()==xfl::Main)
        {
            pMainWing = pWing;
            MainWingLE = pWing->position();
        }

        if (pWing->wingType()==xfl::Fin)
        {
            pFin = pWing;
            FinLE = pWing->position();
        }
    }

    if(pMainWing && pFin)
    {
        double SLA = FinLE.x + pFin->chord(0)/4.0 - (MainWingLE.x + pMainWing->chord(0)/4.0);
        double area = pMainWing->projectedArea();

        double ProjectedFinArea = 0.0;
        for(int iw=0; iw<nWings(); iw++)
        {
            WingXfl const *pWing = wingAt(iw);
            if (pWing->wingType()==xfl::Fin)
            {
                ProjectedFinArea += pWing->projectedArea();
            }
        }

        VTV = ProjectedFinArea * SLA / area/pMainWing->projectedSpan();
    }
    else VTV = 0.0;

    return VTV;
}


void PlaneXfl::duplicate(Plane const*pPlane)
{
    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const *>(pPlane);
    if(!pPlaneXfl) return;

    Plane::duplicate(pPlane);

    m_bThickBuild = pPlaneXfl->m_bThickBuild;

    m_PartIndexes.clear();

    m_Wing.resize(pPlaneXfl->nWings());
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        m_Wing[iw].setUniqueIndex();
        m_Wing[iw].duplicate(pPlaneXfl->m_Wing.at(iw));
    }
//    m_Wing.detach();

    clearFuse();

    if(pPlaneXfl->hasFuse())
    {
        Fuse *pFuseCopy=nullptr;
        for(int ifuse=0; ifuse<pPlaneXfl->fuseCount(); ifuse++)
        {
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);
            if(pFuse->isFlatFaceType())
            {
                pFuseCopy = new FuseFlatFaces(*dynamic_cast<FuseFlatFaces const*>(pFuse));
            }
            else if(pFuse->isSplineType())
            {
                pFuseCopy = new FuseNurbs(*dynamic_cast<FuseNurbs const*>(pFuse));
            }
            else if(pFuse->isSectionType())
            {
                pFuseCopy = new FuseSections(*dynamic_cast<FuseSections const*>(pFuse));
            }
            else if(pFuse->isOccType())
            {
                pFuseCopy = new FuseOcc(*dynamic_cast<FuseOcc const*>(pFuse));
            }
            else if(pFuse->isStlType())
            {
                pFuseCopy = new FuseStl(*dynamic_cast<FuseStl const*>(pFuse));
            }

            m_Fuse.push_back(pFuseCopy);
            m_Fuse.back()->setUniqueIndex();
        }
    }

    duplicatePanels(pPlane);

    makeUniqueIndexList();
}


void PlaneXfl::duplicatePanels(Plane const *pPlane)
{
    m_RefTriMesh  = pPlane->refTriMesh();
    m_TriMesh     = pPlane->triMesh();

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
    if(pPlaneXfl)
    {
        m_RefQuadMesh = pPlaneXfl->m_RefQuadMesh;
        m_QuadMesh    = pPlaneXfl->m_QuadMesh;
    }
}


void PlaneXfl::copyMetaData(const Plane *pOtherPlane)
{
    PlaneXfl const *pOtherXFlPlane = dynamic_cast<PlaneXfl const*>(pOtherPlane);

    m_Description  = pOtherPlane->description();
    m_theStyle     = pOtherPlane->theStyle();

    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl *pWing = wing(iw);
        WingXfl const *pModWing = pOtherXFlPlane->wingAt(iw);
        if(pWing && pModWing && pWing->wingType()==pModWing->wingType())
        {
            pWing->setName(pModWing->name());
        }
    }

    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        Fuse *pFuse = fuse(ifuse);
        Fuse const *pModFuse = pOtherXFlPlane->fuseAt(ifuse);
        if(pFuse && pModFuse && pFuse->fuseType()==pModFuse->fuseType())
        {
            pFuse->setName(pModFuse->name());
        }
    }
}


void PlaneXfl::createSurfaces()
{
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl *pWing = wing(iw);
        Vector3d LE = pWing->position();
        double rx = pWing->rx();
        double ry = pWing->ry();
        pWing->createSurfaces(LE, rx, ry);
        pWing->computeGeometry();
        if(pWing->bAutoInertia()) pWing->computeStructuralInertia(pWing->position());
    }

    createWingSideNodes();

    for(int iw=0; iw<nWings(); iw++) wing(iw)->computeStations();
}


void PlaneXfl::createWingSideNodes()
{
    Fuse *pTranslatedFuse = nullptr;
    if(hasFuse() && fabs(m_Fuse.front()->position().y)<0.001)
    {
        pTranslatedFuse = m_Fuse.front()->clone();
        pTranslatedFuse->translate(fusePos(0));
    }

    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl *pWing = wing(iw);
        for (int j=0; j<pWing->nSurfaces(); j++)
            pWing->surface(j).makeSideNodes(pTranslatedFuse);
    }
    if(pTranslatedFuse) delete pTranslatedFuse;
}


/**
 * Returns the number of mesh panels defined on this Plane's surfaces.
 * Assumes thin surfaces for the wings.
 * @return the number of mesh panels
 */
int PlaneXfl::VLMPanelTotal() const
{
    int total = 0;
    for(int iw=0; iw<nWings(); iw++)
    {

        total += m_Wing[iw].quadTotal(true);
    }

    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(fuseAt(ifuse)->isXflType())
        {
            FuseXfl const *pFuseXfl = dynamic_cast<FuseXfl const*>(fuseAt(ifuse));
            total += pFuseXfl->quadCount();
        }
    }

    return total;
}

int PlaneXfl::quadCount() const
{
    int total = 0;
    for(int iw=0; iw<nWings(); iw++)
    {
        total += m_Wing[iw].quadTotal(false);
    }

    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(fuseAt(ifuse)->isXflType())
        {
            FuseXfl const*pFuseXfl = dynamic_cast<FuseXfl const*>(fuseAt(ifuse));
            total += pFuseXfl->quadCount();
        }
    }

    return total;
}


int PlaneXfl::triangleCount() const
{
    int tricount = 0;
    for(int iw=0; iw<nWings(); iw++)
    {
        tricount += m_Wing.at(iw).nTriangles();
    }
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        tricount += m_Fuse.at(ifuse)->nTriangles();
    }
    return tricount;
}


/**
 * Returns a pointer to the wing with index iw, or NULL if this plane's wing is not active
 *  @param iw the index of the wing
 *  @return a pointer to the wing, or NULL if none;
 */
WingXfl *PlaneXfl::wing(xfl::enumType wingType)
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wing(iw))
        {
            if(wing(iw)->wingType()==wingType) return wing(iw);
        }
    }
    return nullptr;
}


/** Returns a pointer to the Plane's wing with index iw, or NULL if none has been defined.  */
WingXfl *PlaneXfl::wing(int iw)
{
    if(iw<0 || iw>=nWings()) return nullptr;
    return &m_Wing[iw];
}

/** Returns a pointer to the Plane's wing with index iw, or NULL if none has been defined.  */
WingXfl const *PlaneXfl::wingAt(int iw) const
{
    if(iw<0 || iw>=nWings()) return nullptr;
    return &m_Wing.at(iw);
}


WingXfl *PlaneXfl::mainWing()
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(m_Wing.at(iw).isMainWing())   return &m_Wing[iw];
    }
    return nullptr;
}


WingXfl const *PlaneXfl::mainWing() const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(m_Wing.at(iw).isMainWing())   return &m_Wing[iw];
    }
    return nullptr;
}


WingXfl *PlaneXfl::stab()
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw) && wingAt(iw)->isElevator())   return wing(iw);
    }
    return nullptr;
}

/*
Wing*PlaneXfl::wing2()
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wing(iw) && wing(iw)->isSecondWing())   return wing(iw);
    }
    return nullptr;
}*/


WingXfl*PlaneXfl::fin()
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wing(iw) && wing(iw)->isFin())   return wing(iw);
    }
    return nullptr;
}


Fuse *PlaneXfl::fuse(const std::string &fusename)
{
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(m_Fuse.at(ifuse)->name().compare(fusename)==0)   return fuse(ifuse);
    }
    return nullptr;
}


/**
 * Loads or Saves the data of this Plane to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool PlaneXfl::serializePlaneXFL(QDataStream &ar, bool bIsStoring)
{
    int i(0), k(0);
    double dble(0), mass(0), px(0), py(0), pz(0);
    bool bDouble(false), bSym(false), bl(false), bBiplane(false), bStab(false), bFin(false), bFuse(false);
    QString str, strange;

    int ArchiveFormat(0);// identifies the format of the file
    if (bIsStoring)
    {
        // using xf7 format instead
        return true;
    }
    else
    {    // loading code

        ar >> ArchiveFormat;
        if (ArchiveFormat <100001 || ArchiveFormat>110000)
        {
            return false;
        }

        int nw=4; //MAXWINGS
        m_Wing.clear();
        for(int iw=0; iw<nw; iw++)
        {
            addWing();
        }

        ar >> strange;    m_Name = strange.trimmed().toStdString();
        ar >> strange;    m_Description = strange.toStdString();;

        if(ArchiveFormat>=100002)  m_theStyle.serializeFl5(ar, bIsStoring);

        for(int iw=0; iw<nWings(); iw++)
        {
            m_Wing[iw].serializePartXFL(ar, bIsStoring);
        }

        if(ArchiveFormat<100003)
        {
            m_Wing[0].setWingType(xfl::Main);
            m_Wing[1].setWingType(xfl::OtherWing);
            m_Wing[2].setWingType(xfl::Elevator) ;
            m_Wing[3].setWingType(xfl::Fin);
            m_Wing[3].setClosedInnerSide(true);
        }

        ar >> bBiplane>> bStab >>bFin >> bDouble>> bSym>> bl; // m_bDoubleSymFin;
        for(int iw=0; iw<nWings(); iw++)
        {
            ar >> px >> py >> pz >> dble;
            // correcting past errors
            if(std::isnan(px))   px = 0.0;
            if(std::isnan(py))   py = 0.0;
            if(std::isnan(pz))   pz = 0.0;
            if(std::isnan(dble)) dble = 0.0;
            if(fabs(px)  <LENGTHPRECISION) px = 0.0;
            if(fabs(py)  <LENGTHPRECISION) py = 0.0;
            if(fabs(pz)  <LENGTHPRECISION) pz = 0.0;
            if(fabs(dble)<LENGTHPRECISION) dble = 0.0;
            if(fabs(px)  >1000.0) px = 0.0;
            if(fabs(py)  >1000.0) py = 0.0;
            if(fabs(pz)  >1000.0) pz = 0.0;
            if(fabs(dble)>1000.0) dble = 0.0;

            m_Wing[iw].setPosition(px, py, pz);
            m_Wing[iw].setRy(dble);
            if(m_Wing[iw].isFin())
            {
                m_Wing[iw].setTwoSided(bDouble);
            }
        }

        if(ArchiveFormat<100003 && nWings()>=4 && m_Wing[3].isFin())
        {
            //            m_Wing[3].isDoubleFin() = bDouble;
            //            m_Wing[3].setSymFin(bSym);
            m_Wing[3].setRx(-90.0);
        }

        ar >> bFuse;
        ar >> px >> pz;
        if(bFuse)
        {
            QString BodyName;
            ar >> BodyName; //unused
            int format=0;
            ar >> format;
            if(100000<=format && format<200000)
            {
                clearFuse();
                FuseXfl *pBody = new FuseNurbs;
                pBody->serializePartXFL(ar, bIsStoring, format);
                addFuse(pBody);
                m_Fuse[0]->setPosition(px,0.0,pz);
            }
            else if(500000<=format && format<600000)
            {
                clearFuse();
                FuseOcc *pBodyOcc = new FuseOcc;
                pBodyOcc->serializePartFl5(ar, bIsStoring);
                addFuse(pBodyOcc);
                m_Fuse[0]->setPosition(px,0.0,pz);
            }
            else if(600000<=format && format<700000)
            {
                clearFuse();
                FuseStl *pBodyStl = new FuseStl;
                pBodyStl->serializePartFl5(ar, bIsStoring);
                addFuse(pBodyStl);
                m_Fuse[0]->setPosition(px,0.0,pz);
            }

            if(std::find(m_PartIndexes.begin(), m_PartIndexes.end(), fuse(0)->uniqueIndex()) == m_PartIndexes.end())
                m_PartIndexes.insert(m_PartIndexes.begin(), fuse(0)->uniqueIndex());
        }

        clearPointMasses();

        ar >> k;
        for(i=0; i<k; i++)
        {
            ar >> mass >> px >>py >> pz;
            ar >> str;
            m_Inertia.appendPointMass(mass, Vector3d(px, py, pz), str.toStdString());
        }

        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;

        if(ArchiveFormat<100003)
        {
            if(!bFin)
            {
                int index = m_Wing.back().uniqueIndex();

                std::vector<int>::iterator it = std::find(m_PartIndexes.begin(), m_PartIndexes.end(), index);
                if(it!=m_PartIndexes.end()) m_PartIndexes.erase(it);
                m_Wing.pop_back();
            }
            if(!bStab)
            {
                int index = m_Wing.at(2).uniqueIndex();
                std::vector<int>::iterator it = std::find(m_PartIndexes.begin(), m_PartIndexes.end(), index);
                if(it!=m_PartIndexes.end()) m_PartIndexes.erase(it);
                m_Wing.erase(m_Wing.begin()+2);
            }

            if(!bBiplane)
            {
                int index = m_Wing.at(1).uniqueIndex();
                std::vector<int>::iterator it = std::find(m_PartIndexes.begin(), m_PartIndexes.end(), index);
                if(it!=m_PartIndexes.end()) m_PartIndexes.erase(it);
                m_Wing.erase(m_Wing.begin()+1);
            }
        }

        return true;
    }
}


/**
 * Loads or Saves the data of this Plane to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool PlaneXfl::serializePlaneFl5(QDataStream &ar, bool bIsStoring)
{
    double dble=0.0, m=0.0, px=0.0, py=0.0, pz=0.0;

    QString str, strange;

    int ArchiveFormat;// identifies the format of the file
    // 500001: new fl5 format
    // 500002: added m_bInertiaFromParts flag
    // 500003: beta 11; added new sub classes for FuseXfl
    ArchiveFormat = 500003;

    if (bIsStoring)
    {
        ar << ArchiveFormat;

        ar << nWings();

        ar << QString::fromStdString(m_Name);
        ar << QString::fromStdString(m_Description);
        ar << m_theStyle.m_Stipple << m_theStyle.m_Width << m_theStyle.m_Symbol;
        m_theStyle.m_Color.serialize(ar, true);

        for(int iw=0; iw<nWings(); iw++)
        {
            m_Wing[iw].serializePartFl5(ar, bIsStoring);
        }

        ar << nFuse();

        for(int ifuse=0; ifuse<nFuse(); ifuse++)
        {
            Fuse *pFuse = fuse(ifuse);
            if(pFuse->isFlatFaceType())
            {
                ar << 100004;
                FuseFlatFaces *pBody = dynamic_cast<FuseFlatFaces*>(pFuse);
                pBody->serializePartFl5(ar, true);
            }
            else if(pFuse->isSplineType())
            {
                ar << 100005;
                FuseNurbs *pBody = dynamic_cast<FuseNurbs*>(pFuse);
                pBody->serializePartFl5(ar, true);
            }
            else if(pFuse->isSectionType())
            {
                ar << 100006;
                FuseSections *pBody = dynamic_cast<FuseSections*>(pFuse);
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

        ar << m_bAutoInertia;
        m_Inertia.serializeFl5(ar, bIsStoring);

        //        serializeTriMesh(ar, bIsStoring);

        // space allocation for the future storage of more data, without need to change the format
        int nSpares=1;
        ar << nSpares;
        ar << int(m_bThickBuild);
//        for (int i=0; i<nSpares; i++) ar << 0;
        int nDbleSpares = 0;
        ar << nDbleSpares;
//        for (int i=0; i<nSpares; i++) ar << 0.0;

        return true;
    }
    else
    {    // loading code
        int k=0;
        int nw=0;
        ar >> ArchiveFormat;
        if (ArchiveFormat <500000 || ArchiveFormat>500010) return false;

        ar >> nw;
        m_Wing.clear();
        for(int iw=0; iw<nw; iw++)
        {
            addWing();
        }

        ar >> strange;    m_Name = strange.trimmed().toStdString();
        ar >> strange;    m_Description = strange.toStdString();
        ar >> k; m_theStyle.m_Stipple = LineStyle::convertLineStyle(k);
        ar >> m_theStyle.m_Width;
        ar >> k; m_theStyle.m_Symbol=LineStyle::convertSymbol(k);
        m_theStyle.m_Color.serialize(ar, false);

        for(int iw=0; iw<nWings(); iw++)
        {
            m_Wing[iw].serializePartFl5(ar, bIsStoring);
        }

        int nFuse;
        ar >> nFuse;
        clearFuse();

        if(nFuse<0 || nFuse>10000)
            return false;

        for(int ifuse=0; ifuse<nFuse; ifuse++)
        {
            int format=0;
            ar >> format;
            if(format==100001)
            {
                FuseNurbs *pFuseNurbs = new FuseNurbs;
                pFuseNurbs->serializePartFl5(ar, bIsStoring);
                if(pFuseNurbs)
                {
                    if(pFuseNurbs->fuseType()==Fuse::FlatFace)
                    {
                        // clean old mess
                        FuseFlatFaces *pFuseFF = new FuseFlatFaces();
                        pFuseFF->duplicateFuseXfl(*pFuseNurbs);
                        pFuseFF->setFuseType(Fuse::FlatFace);
                        pFuseFF->makeFuseGeometry();
                        if(pFuseFF->bAutoInertia()) pFuseFF->computeStructuralInertia(Vector3d());
                        std::string logmsg;
                        pFuseFF->makeDefaultTriMesh(logmsg, "");

                        delete pFuseNurbs;
                        m_Fuse.push_back(pFuseFF);
                    }
                    else
                    {
                        m_Fuse.push_back(pFuseNurbs);
                    }
                }
            }
            else if(format==100002)
            {
                FuseOcc *pBodyOcc = new FuseOcc;
                pBodyOcc->serializePartFl5(ar, bIsStoring);
                if(pBodyOcc) m_Fuse.push_back(pBodyOcc);
            }
            else if(format==100003)
            {
                FuseStl *pBodyStl = new FuseStl;
                pBodyStl->serializePartFl5(ar, bIsStoring);
                if(pBodyStl) m_Fuse.push_back(pBodyStl);
            }
            else if(format==100004)
            {
                FuseFlatFaces *pBodyFF = new FuseFlatFaces;
                pBodyFF->serializePartFl5(ar, bIsStoring);
                if(pBodyFF) m_Fuse.push_back(pBodyFF);
            }
            else if(format==100005)
            {
                FuseNurbs *pBodyNurbs = new FuseNurbs;
                pBodyNurbs->serializePartFl5(ar, bIsStoring);
                if(pBodyNurbs) m_Fuse.push_back(pBodyNurbs);
            }
            else if(format==100006)
            {
                FuseSections *pBodyFromPts = new FuseSections;
                pBodyFromPts->serializePartFl5(ar, bIsStoring);
                if(pBodyFromPts) m_Fuse.push_back(pBodyFromPts);
            }
            Fuse *pFuse = m_Fuse.back();
            pFuse->triMesh().serializePanelsFl5(ar, bIsStoring);

            // compatibility with legacy project formats
            if(pFuse->nPanel3()==0)
            {
                std::string strange;
                pFuse->makeDefaultTriMesh(strange, "");
            }

            for(uint in=0; in<pFuse->nodes().size(); in++)
            {
                pFuse->nodes()[in].setSurfacePosition(xfl::FUSESURFACE);
            }
            pFuse->setUniqueIndex();
        }

        makeUniqueIndexList();

        if(ArchiveFormat>=500002)
        {
            ar >> m_bAutoInertia;
            m_Inertia.serializeFl5(ar, bIsStoring);
        }
        else
        {
            clearPointMasses();
            ar >> k;
            for(int i=0; i<k; i++)
            {
                ar >> m >> px >>py >> pz;
                ar >> str;
                m_Inertia.appendPointMass(m, Vector3d(px, py, pz), str.toStdString());
            }
        }

        // space allocation
        int nSpares=0;
        ar >> nSpares;
        if(nSpares>0)
        {
            ar >> k;  m_bThickBuild = bool(k); // v7.54
        }

        for (int i=1; i<nSpares; i++) ar >> k;
        int nDbleSpares = 0;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        return true;
    }
}


bool PlaneXfl::hasMainWing() const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw) && wingAt(iw)->isMainWing()) return true;
    }
    return false;
}



bool PlaneXfl::hasOtherWing() const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw) && wingAt(iw)->isOtherWing()) return true;
    }
    return false;
}

/*
bool PlaneXfl::hasWing2() const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw) && wingAt(iw)->isSecondWing()) return true;
    }
    return false;
}*/



bool PlaneXfl::hasStab() const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw) && wingAt(iw)->isElevator()) return true;
    }
    return false;
}



bool PlaneXfl::hasFin() const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw) && wingAt(iw)->isFin()) return true;
    }
    return false;
}


void PlaneXfl::makeUniqueIndexList()
{
    m_PartIndexes.clear();
    for(int iw=0; iw<nWings(); iw++)
        m_PartIndexes.push_back(wing(iw)->uniqueIndex());
    for(int iFuse=0; iFuse<nFuse(); iFuse++)
        m_PartIndexes.push_back(fuse(iFuse)->uniqueIndex());
}


WingXfl* PlaneXfl::addWing(xfl::enumType wingtype)
{
    m_Wing.push_back({wingtype});

    m_Wing.back().setUniqueIndex();
    makeUniqueIndexList();

    QString strange;
    strange = QString::asprintf("Wing_%d", nWings());
    m_Wing.back().setName(strange.toStdString());

    return &m_Wing.back();
}


WingXfl *PlaneXfl::addWing(WingXfl *pNewWing)
{
    m_Wing.push_back(*pNewWing);
    delete pNewWing;
    m_Wing.back().setUniqueIndex();
    makeUniqueIndexList();

    return &m_Wing.back();
}


void PlaneXfl::clearFuse()
{
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        delete m_Fuse[ifuse];
    }
    m_Fuse.clear();
}


void PlaneXfl::removeFuse(Fuse *pFuse)
{
    if(!pFuse) return;
    for(int ifuse=0; ifuse<fuseCount(); ifuse++)
    {
        if(fuse(ifuse)==pFuse)
        {
            m_Fuse.erase(m_Fuse.begin()+ifuse);
            delete pFuse;

            return;
        }
    }
}


void PlaneXfl::addFuse(Fuse *pFuse)
{
    if(!pFuse) return;
    pFuse->setUniqueIndex();
    m_Fuse.push_back(pFuse);
    makeUniqueIndexList();
}


WingXfl* PlaneXfl::duplicateWing(int iWing)
{
    if(!wing(iWing)) return nullptr;

    m_Wing.push_back(m_Wing[iWing]);
    if(m_Wing[iWing].isMainWing()) m_Wing.back().setWingType(xfl::OtherWing);

    QString strange;
    strange = QString::asprintf("Wing_%d", nWings());
    m_Wing.back().setName(strange.toStdString());

    createSurfaces();
    return &m_Wing.back();
}


Fuse* PlaneXfl::duplicateFuse(int iFuse)
{
    if(!fuse(iFuse)) return nullptr;

    Fuse *pFuse = m_Fuse[iFuse]->clone();
    m_Fuse.push_back(pFuse);

    QString strange;
    strange = QString::asprintf("Fuse_%d", nFuse());
    m_Fuse.back()->setName(strange.toStdString());

    return m_Fuse.back();
}


void PlaneXfl::removeWing(WingXfl*pWing)
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(pWing==wing(iw))
        {
            removeWing(iw);
            return;
        }
    }
}


void PlaneXfl::removeWing(int iWing)
{
    m_Wing.erase(m_Wing.begin()+iWing);

    makeUniqueIndexList();
}


void PlaneXfl::removeWings()
{
    m_Wing.clear();

    makePlane(true, false, true);
}


Fuse * PlaneXfl::setFuse(bool bFuse, Fuse::enumType bodytype)
{
    if(bFuse)
    {
        clearFuse();
        Fuse *pFuse = nullptr;
        switch(bodytype)
        {
            default:
            case Fuse::FlatFace:
            {
                pFuse = new FuseFlatFaces();
                break;
            }
            case Fuse::NURBS:
            {
                pFuse = new FuseNurbs();
                break;
            }
            case Fuse::Sections:
            {
                pFuse = new FuseSections();
                break;
            }
            case Fuse::Occ:
            {
                pFuse = new FuseOcc();
                break;
            }
            case Fuse::Stl:
            {
                pFuse = new FuseStl();
                break;
            }
        }
        addFuse(pFuse);
        return pFuse;
    }

    clearFuse();
    return nullptr;
}


void PlaneXfl::removeFuse(int iFuse)
{
    if(hasFuse() && iFuse>=0 && iFuse<fuseCount())
    {
        Fuse *pFuse = fuse(iFuse);
        delete pFuse;
        m_Fuse.erase(m_Fuse.begin()+iFuse);

        pFuse = nullptr;
    }
    makeUniqueIndexList();
}


void PlaneXfl::swapWings(int iWing1, int iWing2)
{
    if(iWing1>=0 && iWing2>=0 && iWing1!=iWing2 && iWing1<nWings() && iWing2<nWings())
    {
/*        WingXfl pWingTmp = m_Wing[iWing2];
        m_Wing[iWing2] = m_Wing[iWing1];
        m_Wing[iWing1] = pWingTmp;*/
        std::swap(m_Wing[iWing1], m_Wing[iWing2]);
    }
}


void PlaneXfl::swapFuses(int iFuse1, int iFuse2)
{
    if(iFuse1>=0 && iFuse2>=0 && iFuse1!=iFuse2 && iFuse1<nFuse() && iFuse2<nFuse())
        std::swap(m_Fuse[iFuse1], m_Fuse[iFuse2]);
}


void PlaneXfl::lock()
{
    for(int iw=0; iw<nWings(); iw++)
        wing(iw)->lock();

    for(int ifuse=0; ifuse<fuseCount(); ifuse++)
        fuse(ifuse)->lock();

    m_bLocked = true;
}


void PlaneXfl::unlock()
{
    for(int iw=0; iw<nWings(); iw++)
        wing(iw)->unlock();


    for(int ifuse=0; ifuse<fuseCount(); ifuse++)
        fuse(ifuse)->unlock();

    m_bLocked = false;
}


Part const *PlaneXfl::partAt(int iPart) const
{
    if(iPart<0 || iPart>nParts()) return nullptr;

    if(iPart<nWings()) return wingAt(iPart);

    return fuseAt(iPart-nWings());
}


Part const*PlaneXfl::partFromIndex(int UniqueIndex) const
{
    for(int ifuse=0; ifuse<fuseCount(); ifuse++)
    {
        if(fuseAt(ifuse) && fuseAt(ifuse)->uniqueIndex()==UniqueIndex) return fuseAt(ifuse);
    }

    for(int iw=0; iw<nWings(); iw++)
    {
        if(m_Wing.at(iw).uniqueIndex()==UniqueIndex) return wingAt(iw);
    }
    return nullptr;
}


WingXfl const *PlaneXfl::wingFromName(const std::string &name) const
{
    if(name.length()==0) return nullptr;

    for(int i=0; i<nWings(); i++)
    {
        if(wingAt(i)->name()==name) return wingAt(i);
    }
    return nullptr;
}


Fuse const *PlaneXfl::fuseFromName(const std::string &name) const
{
    if(name.length()==0) return nullptr;

    for(int i=0; i<nFuse(); i++)
    {
        if(fuseAt(i)->name()==name) return fuseAt(i);
    }
    return nullptr;
}


int PlaneXfl::wingIndex(WingXfl* pWing) const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        if(wingAt(iw)==pWing) return iw;
    }
    return -1;
}


int PlaneXfl::fuseIndex(Fuse *pFuse) const
{
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(fuseAt(ifuse)==pFuse) return ifuse;
    }
    return -1;
}


void PlaneXfl::swapIndexes(int k, int l)
{
    if(k==l) return;
    if(k<0 || l<0) return;
    if(k>=int(m_PartIndexes.size()) || l>=int(m_PartIndexes.size())) return;
    std::swap(m_PartIndexes[k], m_PartIndexes[l]);
}


void PlaneXfl::makePlane(bool bThickSurfaces, bool bIgnoreFusePanels, bool bMakeTriMesh)
{
    // start with the fuse, needed to construct surfaces
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        Fuse *pFuse = fuse(ifuse);
        if(pFuse->bAutoInertia()) pFuse->computeStructuralInertia(Vector3d());
    }

    createSurfaces();

    if(m_bAutoInertia)
        computeStructuralInertia();

    makeQuadMesh(bThickSurfaces, bIgnoreFusePanels);

    if(bMakeTriMesh)  makeTriMesh(bThickSurfaces);

    m_bIsInitialized = true;
}


void PlaneXfl::makeQuadMesh(bool bThickSurfaces, bool bIgnoreFusePanels)
{
    int m_nWakeColumn = 0;
    m_RefQuadMesh.clearMesh();
    int Nel = 0;

    for(int iw=0; iw<nWings(); iw++)
    {
        wing(iw)->m_nPanel4 = 0;
        wing(iw)->setFirstPanel4Index(m_RefQuadMesh.nPanels());
        for(int jSurf=0; jSurf<wing(iw)->nSurfaces(); jSurf++)
        {
            Surface &surf = wing(iw)->m_Surface[jSurf];
            Nel = surf.makeQuadPanels(m_RefQuadMesh.panels(), m_nWakeColumn, bThickSurfaces, wingAt(iw)->nTipStrips());
            wing(iw)->m_nPanel4 += Nel;
        }
    }

    if(!bIgnoreFusePanels)
    {
        for(int ifuse=0; ifuse<fuseCount(); ifuse++)
        {
            if(fuse(ifuse))
            {
                if(fuse(ifuse)->isSplineType() || fuse(ifuse)->isFlatFaceType())
                {
                    FuseXfl *pFuseXfl = dynamic_cast<FuseXfl*>(fuse(ifuse));
                    int i40=m_RefQuadMesh.nPanels();
                    pFuseXfl->setFirstPanel4Index(i40);

                    for(int i4f=0; i4f<pFuseXfl->nPanel4(); i4f++)
                    {
                        Panel4 p4 = pFuseXfl->panel4(i4f);
                        p4.setIndex(p4.index()+i40);
                        if(p4.m_iPL>=0) p4.m_iPL += i40;
                        if(p4.m_iPR>=0) p4.m_iPR += i40;
                        if(p4.m_iPD>=0) p4.m_iPD += i40;
                        if(p4.m_iPU>=0) p4.m_iPU += i40;
                        p4.translate(fusePos(ifuse));
                        m_RefQuadMesh.addPanel(p4);
                    }
                }
                else if(fuse(ifuse)->isSectionType())
                {
                    FuseSections *pFuseSections = dynamic_cast<FuseSections*>(fuse(ifuse));
                    int i40=m_RefQuadMesh.nPanels();
                    pFuseSections->setFirstPanel4Index(i40);

                    for(int i4f=0; i4f<pFuseSections->nPanel4(); i4f++)
                    {
                        Panel4 p4 = pFuseSections->panel4(i4f);
                        p4.setIndex(p4.index()+i40);
                        if(p4.m_iPL>=0) p4.m_iPL += i40;
                        if(p4.m_iPR>=0) p4.m_iPR += i40;
                        if(p4.m_iPD>=0) p4.m_iPD += i40;
                        if(p4.m_iPU>=0) p4.m_iPU += i40;
                        p4.translate(fusePos(ifuse));
                        m_RefQuadMesh.addPanel(p4);
                    }
                }
            }
        }
    }

    //Connect quad panels of adjacent surfaces
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl *pWing = wing(iw);
        for(int jSurf=0; jSurf<pWing->nSurfaces()-1; jSurf++)
        {
            if(!pWing->surfaceAt(jSurf).isTipRight() && !pWing->surfaceAt(jSurf).isClosedRightSide())
            {
                joinSurfaces(pWing->surfaceAt(jSurf), pWing->surfaceAt(jSurf+1));
            }
        }
    }

    m_QuadMesh = m_RefQuadMesh;
}


void PlaneXfl::makeTriMesh(bool bThickSurfaces)
{
    m_RefTriMesh.clearMesh();

    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl &wing = m_Wing[iw];
        wing.makeTriPanels(m_RefTriMesh.nPanels(), m_RefTriMesh.nNodes(), bThickSurfaces);
        m_RefTriMesh.appendMesh(wing.triMesh());
        if(wing.isFin()) m_RefTriMesh.lastPanel().m_iPD = -1; // because there is no right tip patch
    }

    for(int in=0; in<m_RefTriMesh.nodeCount(); in++) m_RefTriMesh.node(in).setIndex(in);

    for(int ifuse=0; ifuse<fuseCount(); ifuse++)
    {
        Fuse *pFuse = fuse(ifuse);
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
                S[in].translate(m_Fuse.at(ifuse)->position());
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
            m_RefTriMesh.node(ndindex).translate(m_Fuse.at(ifuse)->position());
            m_RefTriMesh.node(ndindex).setIndex(ndindex);
            ndindex++;
        }
    }

    m_RefTriMesh.setNodePanels();
    m_RefTriMesh.makeNodeNormals();

    m_TriMesh = m_RefTriMesh;
}


/** Potentially lengthy task, so on-demand only */
bool PlaneXfl::connectTriMesh(bool bRefTriMesh, bool bConnectTE, bool )
{
    TriMesh *pTriMesh = bRefTriMesh ? &m_RefTriMesh : &m_TriMesh;

    QElapsedTimer t; t.start();

    //make internal fuse connections
    for(int ifuse=0; ifuse<fuseCount(); ifuse++)
    {
        Fuse *pFuse = fuse(ifuse);
        int i1 = pFuse->firstPanel3Index();
        int n1 = pFuse->nPanel3();
        pTriMesh->makeConnectionsFromNodePosition2(i1, n1, LENGTHPRECISION);
    }

    // make internal wing connections
    for(int iw=0; iw<nWings(); iw++)
    {
        // first the surface
        WingXfl const &wing = m_Wing.at(iw);
        int i1 = wing.firstPanel3Index();
        int n1 = wing.nPanel3();
        pTriMesh->makeConnectionsFromNodePosition2(i1, n1, 1.0e-4);
    }

    pTriMesh->connectNodes();

    if(bConnectTE)
    {
        std::vector<int>errorlist;
        if(!pTriMesh->connectTrailingEdges(errorlist))    return false;
    }

    if(bRefTriMesh)
        m_TriMesh.copyConnections(m_RefTriMesh);

    return true;
}


bool PlaneXfl::checkFoils(std::string &log) const
{
    bool bMissing = false;
    for(int iw=0; iw<nWings(); iw++)
    {
        std::string strange;
        if(!wingAt(iw)->checkFoils(strange))
        {
            log = wingAt(iw)->name() + ":\n" + strange;
            bMissing = true;
        }
    }
    return bMissing;
}


Fuse *PlaneXfl::makeNewFuse(Fuse::enumType bodytype)
{
    Fuse *pFuse = nullptr;
    switch(bodytype)
    {
        case Fuse::FlatFace:
        {
            FuseXfl *pFuseXfl = new FuseFlatFaces();
            pFuseXfl->makeDefaultFuse();
            pFuse = pFuseXfl;
            break;
        }
        default:
        case Fuse::NURBS:
        {
            FuseXfl *pFuseXfl = new FuseNurbs();
            pFuseXfl->makeDefaultFuse();
            pFuse = pFuseXfl;
            break;
        }
        case Fuse::Sections:
        {
            FuseSections*pFuseXfl = new FuseSections();
            pFuseXfl->makeDefaultFuse();
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
    if(pFuse) addFuse(pFuse);
    return pFuse;
}


std::string PlaneXfl::planeData(bool bOtherWings) const
{
    QString Result;
    QString str1;
    QString strange;

    QString lengthlab = Units::lengthUnitQLabel();
    QString arealab = Units::areaUnitQLabel();
    QString masslab = Units::massUnitQLabel();

    WingXfl const *pMainWing = mainWing();

    constexpr int labelWidth = 15;
    auto label = [=](const char *sourceText) {
        return QCoreApplication::translate("PlaneXfl", sourceText).leftJustified(labelWidth, ' ');
    };

    str1 = QString("%1 = %2 ").arg(label("Wing span")).arg(planformSpan()*Units::mtoUnit(), 9, 'f', 3);
    strange += str1 + lengthlab + "\n";

    str1 = QString("%1 = %2 ").arg(label("xyProj. span")).arg(projectedSpan()*Units::mtoUnit(), 9, 'f', 3);
    strange += str1 + lengthlab + "\n";

    str1 = QString("%1 = %2 ").arg(label("Wing area")).arg(planformArea(bOtherWings) * Units::m2toUnit(), 9, 'f', 3);
    strange += str1 + arealab + "\n";

    str1 = QString("%1 = %2 ").arg(label("Projected area")).arg(projectedArea(bOtherWings) * Units::m2toUnit(), 9, 'f', 3);
    strange += str1 + arealab + "\n";

    Result = QString("%1 = %2 ").arg(label("Mass")).arg(totalMass()*Units::kgtoUnit(), 9, 'f', 3);
    strange += Result + masslab + "\n";

    Result = QString("%1 = (%2, %3, %4) ")
                 .arg(QCoreApplication::translate("PlaneXfl", "CoG"))
                 .arg(m_Inertia.CoG_t().x*Units::mtoUnit(), 0, 'f', 3)
                 .arg(m_Inertia.CoG_t().y*Units::mtoUnit(), 0, 'f', 3)
                 .arg(m_Inertia.CoG_t().z*Units::mtoUnit(), 0, 'f', 3);
    strange += Result + lengthlab + "\n";

    if(pMainWing)
    {
        Result = QString("%1 = %2")
                     .arg(label("Wing load"))
                     .arg(totalMass()*Units::kgtoUnit()/projectedArea(bOtherWings)/Units::m2toUnit(), 9, 'f', 3);
        strange += Result + " " + masslab + "/" + arealab + "\n";
    }

    if(hasStab())
    {
        str1 = QString("%1 = %2").arg(label("Tail volume (H)")).arg(tailVolumeHorizontal(), 9, 'f', 3);
        strange += str1 + "\n";
    }

    if(hasFin())
    {
        str1 = QString("%1 = %2").arg(label("Tail volume (V)")).arg(tailVolumeVertical(), 9, 'f', 3);
        strange += str1 + "\n";
    }

    if(pMainWing)
    {
        str1 = QString("%1 = %2 ").arg(label("Root chord")).arg(pMainWing->rootChord()*Units::mtoUnit(), 9, 'f', 3);
        strange += str1 + lengthlab + "\n";
    }

    str1 = QString("%1 = %2 ").arg(label("MAC")).arg(mac()*Units::mtoUnit(), 9, 'f', 3);
    strange += str1 + lengthlab + "\n";

    if(pMainWing)
    {
        str1 = QString("%1 = %2").arg(label("Tip twist")).arg(pMainWing->tipTwist(), 9, 'f', 3) + DEGch;
        strange += str1 + "\n";
    }

    str1 = QString("%1 = %2").arg(label("Aspect Ratio")).arg(aspectRatio(), 9, 'f', 3);
    strange += str1 + "\n";

    str1 = QString("%1 = %2").arg(label("Taper Ratio")).arg(taperRatio(), 9, 'f', 3);
    strange += str1 + "\n";

    if(pMainWing)
    {
        str1 = QString("%1 = %2").arg(label("Root-Tip Sweep")).arg(pMainWing->averageSweep(), 9, 'f', 3) + DEGch;
        strange += str1;
    }

    return strange.toStdString();
}


void PlaneXfl::restoreMesh()
{
    m_QuadMesh = m_RefQuadMesh;
    m_TriMesh  = m_RefTriMesh;
}


int PlaneXfl::xflFuseCount() const
{
    int count=0;
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(m_Fuse.at(ifuse)->isXflType()) count++;
    }
    return count;
}


int PlaneXfl::occFuseCount() const
{
    int count=0;
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(m_Fuse.at(ifuse)->isOccType()) count++;
    }
    return count;
}


int PlaneXfl::stlFuseCount() const
{
    int count=0;
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        if(m_Fuse.at(ifuse)->isStlType()) count++;
    }
    return count;
}


bool PlaneXfl::hasPolar(PlanePolar const*pWPolar) const {return pWPolar->planeName().compare(m_Name)==0;}
bool PlaneXfl::hasPOpp(PlaneOpp const*pPOpp)   const {return pPOpp->planeName().compare(m_Name)==0;}


/**
 * In the case of a plane, the structural mass is the sum of the
 * total mass of each part, i.e. including their point masses.
 * The plane's structural inertia is the sum of the TOTAL inertias
 * of each part, i.e. including their point masses.
 *
 * Sign modification of the products of inertia in v7.13 from negative to positive
 */
void PlaneXfl::computeStructuralInertia()
{
    if(!m_bAutoInertia) return; // keep the custom inertia

    Vector3d cogs;
    double mass_s(0.0);

    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const &aWing = m_Wing.at(iw);
        cogs += (aWing.CoG_t()+aWing.position()) * aWing.totalMass();
        mass_s += aWing.totalMass();
    }

    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        Fuse const *pFuse = m_Fuse.at(ifuse);
        cogs += (pFuse->CoG_t()+pFuse->position()) * pFuse->totalMass();
        mass_s += pFuse->totalMass();
    }

    if(fabs(mass_s)>0.0) cogs *= 1.0/mass_s;
    else                 cogs.set(0.0,0.0,0.);
    m_Inertia.setCoG_s(cogs);
    m_Inertia.setStructuralMass(mass_s);

    double ixx_s(0.0), ixy_s(0.0), ixz_s(0.0), iyy_s(0.0), iyz_s(0.0), izz_s(0.0);

    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const &aWing = m_Wing.at(iw);
        Vector3d d = (aWing.CoG_t()+aWing.position()) - cogs;

        ixx_s += aWing.Ixx_t() + aWing.totalMass()*(d.y*d.y+d.z*d.z);
        ixy_s += aWing.Ixy_t() + aWing.totalMass()*(d.x*d.y);
        ixz_s += aWing.Ixz_t() + aWing.totalMass()*(d.x*d.z);
        iyy_s += aWing.Iyy_t() + aWing.totalMass()*(d.x*d.x+d.z*d.z);
        iyz_s += aWing.Iyz_t() + aWing.totalMass()*(d.y*d.z);
        izz_s += aWing.Izz_t() + aWing.totalMass()*(d.x*d.x+d.y*d.y);
    }

    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        Fuse const *pFuse = m_Fuse.at(ifuse);
        Vector3d d = (pFuse->CoG_t()+pFuse->position()) - cogs;

        ixx_s += pFuse->Ixx_t() + pFuse->totalMass()*(d.y*d.y+d.z*d.z);
        ixy_s += pFuse->Ixy_t() + pFuse->totalMass()*(d.x*d.y);
        ixz_s += pFuse->Ixz_t() + pFuse->totalMass()*(d.x*d.z);
        iyy_s += pFuse->Iyy_t() + pFuse->totalMass()*(d.x*d.x+d.z*d.z);
        iyz_s += pFuse->Iyz_t() + pFuse->totalMass()*(d.y*d.z);
        izz_s += pFuse->Izz_t() + pFuse->totalMass()*(d.x*d.x+d.y*d.y);
    }

    m_Inertia.setIxx_s(ixx_s);
    m_Inertia.setIxy_s(ixy_s);
    m_Inertia.setIxz_s(ixz_s);
    m_Inertia.setIyy_s(iyy_s);
    m_Inertia.setIyz_s(iyz_s);
    m_Inertia.setIzz_s(izz_s);
}


/**
 * At panels on the side of the surfaces, connects the quad elements to the next surface
*/
void PlaneXfl::joinSurfaces(Surface const &LeftSurf, Surface const &RightSurf)
{
    std::vector<Panel4> &panels = m_RefQuadMesh.panels();

    for(uint il=0; il<LeftSurf.panel4List().size(); il++)
    {
        int idx0 = LeftSurf.panel4List().at(il);
        Panel4 &pl = panels[idx0];

        if(pl.isFlapPanel()) continue; // do not connect flaps to adjacent surface

        for(uint ir=0; ir<RightSurf.panel4List().size(); ir++)
        {
            int idx1 = RightSurf.panel4List().at(ir);
            Panel4 &pr = panels[idx1];

            if(pr.isFlapPanel()) continue; // do not connect flaps to adjacent surface

            if(pl.isTopPanel() && pr.isTopPanel())
            {
                if(pl.LB().isSame(pr.LA()) && pl.TB().isSame(pr.TA()))
                {
                    pl.m_iPR = idx1;
                    pr.m_iPL = idx0;
                }
            }
            else if(pl.isBotPanel() && pr.isBotPanel())
            {
                if(pl.LA().isSame(pr.LB()) && pl.TA().isSame(pr.TB()))
                {
                    pl.m_iPL = idx1;
                    pr.m_iPR = idx0;
                }
            }
            else if(pl.isMidPanel() && pr.isMidPanel())
            {
                if(pl.LB().isSame(pr.LA()) && pl.TB().isSame(pr.TA()))
                {
                    pl.m_iPR = idx1;
                    pr.m_iPL = idx0;
                }
            }
        }
    }
}


double PlaneXfl::projectedArea(bool bOtherWings)  const
{
    double area = 0;

    for(int iw=0; iw<nWings(); iw++)
    {
        if(m_Wing.at(iw).isMainWing() )
            area += m_Wing.at(iw).m_ProjectedArea;
        if(bOtherWings)
        {
            if(m_Wing.at(iw).isOtherWing())
            area += m_Wing.at(iw).m_ProjectedArea;
        }
    }

    return area;
}


double PlaneXfl::planformArea(bool bOtherWings)   const
{
    double area = 0;

    for(int iw=0; iw<nWings(); iw++)
    {
        if(m_Wing.at(iw).isMainWing() )
            area += m_Wing.at(iw).m_PlanformArea;
        if(bOtherWings)
        {
            if(m_Wing.at(iw).isOtherWing())
            area += m_Wing.at(iw).m_PlanformArea;
        }
    }

    return area;
}


Vector3d PlaneXfl::rootQuarterPoint(int iw) const
{
    return wingLE(iw) + Vector3d(wingAt(iw)->rootChord()/4,0,0);
}


void PlaneXfl::translate(Vector3d const &T)
{
    for(int iw=0; iw<nWings(); iw++)
    {
        setWingLE(iw, wingLE(iw)+T);
    }
    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        setFusePos(ifuse, fusePos(ifuse)+T);
    }

//    m_RefTriMesh.translatePanels(T.x, T.y, T.z);
//    m_TriMesh = m_RefTriMesh;
    m_Inertia.translateMasses(T);
    if(m_bAutoInertia)
        computeStructuralInertia();
}


void PlaneXfl::scale(double scalefactor)
{
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl &wing = m_Wing[iw];
        wing.scaleSpan(wing.planformSpan()*scalefactor);
        wing.scaleChord(wing.rootChord()*scalefactor);

        wing.setPosition(wing.position()*scalefactor);

        wing.inertia().scaleMassPositions(scalefactor);
        wing.createSurfaces(wing.position(), wing.rx(), wing.ry());
        wing.computeGeometry();
        if(wing.bAutoInertia()) wing.computeStructuralInertia(wing.position());
    }

    for(int ifuse=0; ifuse<nFuse(); ifuse++)
    {
        Fuse *pFuse = m_Fuse[ifuse];
        pFuse->scale(scalefactor, scalefactor, scalefactor);
        pFuse->setPosition(pFuse->position()*scalefactor);

        pFuse->inertia().scaleMassPositions(scalefactor);

        pFuse->makeFuseGeometry();
        if(pFuse->bAutoInertia())pFuse->computeStructuralInertia(pFuse->position());
    }

    m_Inertia.scaleMassPositions(scalefactor);

    if(m_bAutoInertia)
        computeStructuralInertia();
}


int PlaneXfl::nAVLGains() const
{
    int iCtrl = 0;
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const &wing = m_Wing.at(iw);

        for(int ic=0; ic<wing.nFlaps(); ic++)
        {
            iCtrl++;
        }
    }
    return iCtrl;
}


std::string PlaneXfl::flapName(int iFlap) const
{
    int ic = 0;
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const &wing = m_Wing.at(iw);

        for(int iflap=0; iflap<wing.nFlaps(); iflap++)
        {
            if(iFlap==ic) return wing.name() + QString::asprintf("_flap_%d", iflap+1).toStdString();
            ic++;
        }
    }
    return std::string();
}


double PlaneXfl::flapAngle(int iWing, int iFlap) const
{
    WingXfl const &wing = m_Wing.at(iWing);
    int ifl=0;
    for (int jSurf=0; jSurf<wing.nSurfaces(); jSurf++)
    {
        Surface const &surf = wing.surfaceAt(jSurf);
        if(surf.hasTEFlap())
        {
            if(ifl==iFlap)
                return (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
            ifl++;
        }
    }

    return 0.0;
}


int PlaneXfl::nFlaps(int iWing) const
{
    int ifl=0;

    WingXfl const &wing = m_Wing.at(iWing);

    for (int jSurf=0; jSurf<wing.nSurfaces(); jSurf++)
    {
        Surface const &surf = wing.surfaceAt(jSurf);
        if(surf.hasTEFlap())
        {
            ifl++;
        }
    }

    return ifl;
}


int PlaneXfl::nFlaps() const
{
    int ifl=0;
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const &wing = m_Wing.at(iw);

        for (int jSurf=0; jSurf<wing.nSurfaces(); jSurf++)
        {
            Surface const &surf = wing.surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                ifl++;
            }
        }
    }
    return ifl;
}


std::string PlaneXfl::controlSurfaceName(int iCtrl) const
{
    int ic = 0;
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const &wing = m_Wing.at(iw);
        for(int iflap=0; iflap<wing.nFlaps(); iflap++)
        {
            if(iCtrl==ic) return wing.name() + QString::asprintf("_flap_%d", iflap+1).toStdString();
            ic++;
        }
    }
    return std::string();
}


void PlaneXfl::setRangePositions4(PlanePolar const *pWPolar, double t, std::string &outstr)
{
    assert(pWPolar->isType6());

    Vector3d H, Origin;
    Vector3d YVector(0.0, 1.0, 0.0);
    QString strange;
    QString outstring;

    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const *pWing = wingAt(iw);
        int iCtrl=0;

        CtrlRange range = pWPolar->angleRange(iw,0);
        double deltaangle = range.ctrlVal(t);

        // first control value is the overall Y rotation
        if(fabs(deltaangle)>FLAPANGLEPRECISION)
        {
            //rotate the normals and control point positions
            H.set(0.0, 1.0, 0.0);

            double totalAngle = ryAngle(iw) + deltaangle;
            strange = "      Rotating " + QString::fromStdString(pWing->name());
            outstring += strange +  QString::asprintf(" by %f°, total angle is %f", deltaangle, totalAngle) + DEGch + EOLch;

            Origin = wingLE(iw);

            if(pWPolar->isQuadMethod())
            {
                for(int i=0; i<pWing->nPanel4(); i++)
                {
                    int p = pWing->firstPanel4Index() + i;
                    m_QuadMesh.panel(p).rotate(Origin, YVector, deltaangle);
                }
            }
        }

        // following control values are for the flaps
        iCtrl++;
        for (int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                CtrlRange range = pWPolar->angleRange(iw,iCtrl);
                deltaangle = range.ctrlVal(t);

                if (fabs(deltaangle)>FLAPANGLEPRECISION)
                {
                    strange = QString::asprintf("- rotating flap %d by %f°", iCtrl, deltaangle);

                    strange = "      " + QString::fromStdString(pWing->name()) +strange + EOLch;
                    outstring +=strange;

                    if(pWPolar->isQuadMethod())
                    {
                        for(int i4=0; i4<m_QuadMesh.nPanels(); i4++)
                        {
                            if(surf.hasFlapPanel4(i4))
                            {
                                m_QuadMesh.panel(i4).rotate(surf.hingePoint(), surf.hingeVector(), deltaangle);
                            }
                        }
                    }
                }
                iCtrl++;
            }
        }
    }

    outstring  +="\n";

    outstr = outstring.toStdString();
}


void PlaneXfl::setRangePositions3(PlanePolar const *pWPolar, double t, std::string &outstr)
{
    assert(pWPolar->isType6());
    assert(pWPolar->isTriangleMethod());
    QString outstring;

    Vector3d H, Origin;
    Vector3d YVector(0.0, 1.0, 0.0);
    QString strange;
    double totalAngle(0), deltaangle(0);

    if(pWPolar->isTriLinearMethod())
    {
        // make the node normals on the fly
        // node normals are required to compute local Cp coefficients at nodes
        triMesh().makeNodeNormals(false); // or after rotations....
    }
    std::vector<Node> &nodes = triMesh().nodes();

    // set the angles
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const *pWing = wingAt(iw);
        int iCtrl=0;
        deltaangle = pWPolar->angleRange(iw,0).ctrlVal(t);

        // first control value is the overall Y rotation
        if(fabs(deltaangle)>FLAPANGLEPRECISION)
        {
            //rotate the normals and control point positions
            H.set(0.0, 1.0, 0.0);

            totalAngle = ryAngle(iw) + deltaangle;
            strange = "      Rotating " + QString::fromStdString(pWing->name());
            outstring += strange + QString::asprintf(" by %.3f°, total angle is %.3f°\n", deltaangle, totalAngle);

            Origin = wingLE(iw);
            rotateWingNodes(triPanels(), nodes, pWing, Origin, YVector, deltaangle);
        }

        // following control values are for the flaps
        iCtrl++;
        for (int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                deltaangle = pWPolar->angleRange(iw,iCtrl).ctrlVal(t);

                if (fabs(deltaangle)>FLAPANGLEPRECISION)
                {
                    //Add delta rotations to initial control setting and to wing or flap delta rotation
                    if(fabs(surf.foilA()->TEFlapAngle())>0.0 && fabs(surf.foilB()->TEFlapAngle())>0.0)
                        totalAngle = deltaangle + (surf.foilA()->TEFlapAngle() + surf.foilB()->TEFlapAngle())/2.0;
                    else
                        totalAngle = deltaangle;

                    strange = QString::asprintf("- rotating flap %d by %.3f°, total flap angle is %.3f°", iCtrl, deltaangle, totalAngle);

                    strange = "      " + QString::fromStdString(pWing->name()) + strange + EOLch;
                    outstring += strange;

                    rotateFlapNodes(triPanels(), nodes, surf, surf.hingePoint(), surf.hingeVector(), deltaangle);
                }
                iCtrl++;
            }
        }
    }


    TriMesh::rebuildPanelsFromNodes(triPanels(), nodes);

    outstring  +="\n";

    outstr = outstring.toStdString();
}


void PlaneXfl::rotateWingNodes(std::vector<Panel3> const &panel3, std::vector<Node> &node, WingXfl const *pWing,
                                Vector3d const &hingePoint, Vector3d const & hingeVector, double alpha) const
{
//    auto t0 = std::chrono::high_resolution_clock::now();

    bool bFound=false;

    for(uint iNode=0; iNode<node.size(); iNode++)
    {
        bFound = false;
        for(uint i3=0; i3<panel3.size(); i3++)
        {
            if(pWing->hasPanel3(i3) && panel3.at(i3).hasVertex(iNode))
            {
                bFound = true;
                break;
            }
        }
        if(bFound) node[iNode].rotate(hingePoint, hingeVector, alpha);
    }

/*
    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("PlaneTask::rotateWingNodes: %gms", double(duration)/1000.0);*/
}


void PlaneXfl::rotateFlapNodes(const std::vector<Panel3> &panel3, std::vector<Node> &node, Surface const &surf,
                               Vector3d const &hingePoint, Vector3d const & hingeVector, double theta) const
{
    bool bFound = false;
    // scan nodes one at a time so as not to rotate shared nodes multiple times
    for(uint iNode=0; iNode<node.size(); iNode++)
    {
        bFound = false;
        for(uint i=0; i<surf.flapPanel3().size(); i++)
        {
            int i3 = surf.flapPanel3().at(i);
            if(panel3.at(i3).hasVertex(iNode))
            {
                bFound = true;
                break;
            }
        }
        if(bFound)
        {
            node[iNode].rotate(hingePoint, hingeVector, theta);
        }
    }
}


double PlaneXfl::flapPosition(AngleControl const &avlc, int iWing, int iFlap) const
{
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const *pWing = wingAt(iw);

        int iCtrl = 0;
        for (int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                double flapangle = avlc.value(iCtrl);

                if(iw==iWing && iCtrl==iFlap)
                    return flapangle;
                iCtrl++;
            }
        }
    }
    return 0.0;
}


void PlaneXfl::setFlaps(PlanePolar const *pWPolar, std::string &outstr)
{
//    auto t0 = std::chrono::high_resolution_clock::now();
    assert(pWPolar->isType123458() || pWPolar->isType7());

    QString outstring;
    QString strange;

    outstring += "Setting flap positions\n";

    if(pWPolar->isTriLinearMethod())
    {
        // make the node normals on the fly
        // node normals are required to compute local Cp coefficients at nodes
        m_TriMesh.makeNodeNormals(false); // or after rotations....
    }

    std::vector<Node> &nodes = m_TriMesh.nodes();

    // set the angles
    for(int iw=0; iw<nWings(); iw++)
    {
        WingXfl const *pWing = wingAt(iw);

        if(iw>=pWPolar->nFlapCtrls())
        {
            outstring += "      No flap settings defined for " + QString::fromStdString(pWing->name()) + " ... skipping" + EOLch;
            break; // correcting past errors
        }

        AngleControl const &avlc = pWPolar->flapCtrls(iw);
        if(!avlc.hasActiveAngle())
            continue;

        int iFlap=0;

        outstring += "   " + QString::fromStdString(pWing->name()) +":\n";

        for (int jSurf=0; jSurf<pWing->nSurfaces(); jSurf++)
        {
            Surface const &surf = pWing->surfaceAt(jSurf);
            if(surf.hasTEFlap())
            {
                double flapangle = avlc.value(iFlap);

                if (fabs(flapangle)>FLAPANGLEPRECISION)
                {
                    strange = QString::asprintf("      rotating flap %d by %g", iFlap, flapangle) + DEGch + EOLch;

                    outstring += strange;

                    if(pWPolar->isTriangleMethod())
                    {
                        for(uint i=0; i<surf.flapPanel3().size(); i++)
                        {
                            int idx = surf.flapPanel3().at(i);
                            m_TriMesh.panel(idx).rotate(surf.hingePoint(), surf.hingeVector(), flapangle);
                        }
                        rotateFlapNodes(m_TriMesh.panels(), nodes, surf, surf.hingePoint(), surf.hingeVector(), flapangle);
                    }
                    else if(pWPolar->isQuadMethod())
                    {
                        for(int i4=0; i4<m_QuadMesh.nPanels(); i4++)
                        {
                            if(surf.hasFlapPanel4(i4))
                            {
                                m_QuadMesh.panel(i4).rotate(surf.hingePoint(), surf.hingeVector(), flapangle);
                            }
                        }
                    }
                }
                iFlap++;
            }
        }
    }

//    if(pWPolar->isTriangleMethod())        TriMesh::rebuildPanelsFromNodes(m_TriMesh.panels(), nodes);

    outstring  +="\n";

    outstr = outstring.toStdString();
/*
    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("Setting flaps1: %gms",  double(duration)/1000.0);;*/

}


