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

#define _MATH_DEFINES_DEFINED


#include <api/plane.h>

#include "xobjects.h"

#include <api/boatopp.h>
#include <api/fusexfl.h>
#include <api/objects3d.h>
#include <api/planepolar.h>
#include <api/polar.h>
#include <api/planeopp.h>
#include <api/planexfl.h>
#include <api/boat.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailspline.h>
#include <api/sailstl.h>
#include <api/sailocc.h>
#include <api/sailwing.h>
#include <api/flow5-io.h>
#include <api/utils-io.h>

#include <core/xflcore.h>
#include <globals/mainframe.h>
#include <interfaces/controls/w3dprefs.h>
#include <api/gmesh_globals.h>
#include <interfaces/widgets/customdlg/renamedlg.h>


MainFrame *Objects3d::g_pMainFrame = nullptr;


QStringList Objects3d::planeNames()
{
    QStringList names;
    for(int i=0; i<nPlanes(); i++)
    {
        names.push_back(QString::fromStdString(planeAt(i)->name()));
    }
    return names;
}


QStringList Objects3d::polarNames(Plane const*pPlane)
{
    QStringList names;
    if(pPlane)
    {
        for(int i=0; i<nPolars(); i++)
        {
            PlanePolar const *pPolar = planePolarAt(i);
            if(pPolar->planeName()==pPlane->name())
                names.push_back(QString::fromStdString(pPolar->name()));
        }
    }
    return names;
}

/**
 * Inserts a modified Plane object in the array, i.a.w. user instructions
 * @param pModPlane a pointer to the instance of the Plane object to be inserted
 * @return a pointer to the Plane object which was successfully inserted, false otherwise
 */
Plane * Objects3d::setModifiedPlane(Plane *pModPlane)
{
    if(!pModPlane) return nullptr;

    bool bExists = planeExists(pModPlane->name());
    if(!bExists && pModPlane->name().length())
    {
        insertPlane(pModPlane);
        return pModPlane;
    }
    int resp = 0;

    QString OldName = QString::fromStdString(pModPlane->name());

    RenameDlg renDlg(g_pMainFrame);
    renDlg.initDialog(QString::fromStdString(pModPlane->name()), planeNames(), QObject::tr("Enter the new name for the plane:"));

    while (bExists || pModPlane->name().length()==0)
    {
        resp = renDlg.exec();
        if(resp==QDialog::Accepted)
        {
            if (OldName == renDlg.newName()) return pModPlane;

            //Is the new name already used?
            bExists = planeExists(renDlg.newName().toStdString());

            if(!bExists)
            {
                // we have a valid name
                // rename the plane
                pModPlane->setName(renDlg.newName().toStdString());

                insertPlane(pModPlane);
                break;

            }
        }
        else if(resp ==10)
        {
            //the user wants to overwrite the old plane/wing

            Plane *pExistingPlane = plane(renDlg.newName().toStdString());
            deletePlaneResults(pExistingPlane, false);
            deletePlane(pExistingPlane);

            pModPlane->setName(renDlg.newName().toStdString());

            //place the Plane in alphabetical order in the array
            //remove the current Plane from the array
            for (int l=0; l<Objects3d::nPlanes(); l++)
            {
                Plane *pPlane = planeAt(l);
                if(pPlane == pModPlane)
                {
                    removePlaneAt(l);
                    // but don't delete it !
                    break;
                }
            }
            //and re-insert it
            bool bInserted = false;
            for (int l=0; l<nPlanes(); l++)
            {
                Plane *pOldPlane = planeAt(l);
                if(pModPlane->name().compare(pOldPlane->name())<0)
                {
                    //then insert before
                    insertPlane(l, pModPlane);
                    bInserted = true;
                    break;
                }
            }
            if(!bInserted) appendPlane(pModPlane);
            bExists = false;
        }
        else
        {
            return nullptr; //cancelled
        }
    }
    return pModPlane;
}


PlanePolar* Objects3d::insertNewPolar(PlanePolar *pNewWPolar, Plane const*pCurPlane)
{
    if(!pNewWPolar) return nullptr;

    bool bExists = true;

    //check if this WPolar is already inserted
    for(int ip=0; ip<nPolars(); ip++)
    {
         PlanePolar *pOldWPolar = planePolarAt(ip);
        if(pOldWPolar==pNewWPolar)
        {
            // already in the array, nothing to insert
            return nullptr;
        }
    }

    //make a list of existing names
    QStringList NameList;
    for(int k=0; k<nPolars(); k++)
    {
        PlanePolar *pWPolar = planePolarAt(k);
        if(pCurPlane && pWPolar->planeName()==pCurPlane->name())
            NameList.append(QString::fromStdString(pWPolar->name()));
    }

    //Is the new WPolar's name already used?
    bExists = false;
    for (int k=0; k<NameList.count(); k++)
    {
        if(pNewWPolar->name()==NameList.at(k).toStdString())
        {
            bExists = true;
            break;
        }
    }

    if(!bExists)
    {
        //just insert the WPolar in alphabetical order
        insertPlanePolar(pNewWPolar);
        return pNewWPolar;
    }

    // an old object with the WPolar's name exists for this Plane, ask for a new one
    RenameDlg dlg(g_pMainFrame);
    dlg.initDialog(QString::fromStdString(pNewWPolar->name()), polarNames(pCurPlane), "Enter the Polar's new name:");
    int resp = dlg.exec();

    if(resp==10)
    {
        //user wants to overwrite an existing name
        //so find the existing PlanePolar with that name
        PlanePolar *pWPolar = nullptr;
        for(int ipb=0; ipb<nPolars(); ipb++)
        {
             PlanePolar *pOldWPolar = planePolarAt(ipb);
            if(pCurPlane && pOldWPolar->name()==dlg.newName().toStdString() &&
               pOldWPolar->planeName()==pCurPlane->name())
            {
                pWPolar = pOldWPolar;
                break;
            }
        }

        if(pWPolar)
        {
            //remove and delete its children POpps from the array
            deletePlanePolar(pWPolar);
/*            for (int l=nPOpps()-1;l>=0; l--)
            {
                PlaneOpp *pPOpp = POppAt(l);
                if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->name())
                {
                    removePOppAt(l);
                    delete pPOpp;
                }
            }

            for(int ipb=0; ipb<nPolars(); ipb++)
            {
                 WPolar *pOldWPolar = wPolarAt(ipb);
                if(pOldWPolar==pWPolar)
                {
                    removeWPolarAt(ipb);
                    delete pOldWPolar;
                    break;
                }
            }*/
        }

        //room has been made, insert the new WPolar in alphabetical order
        pNewWPolar->setName(dlg.newName().toStdString());

        insertPlanePolar(pNewWPolar);
        return pNewWPolar;

    }
    else if(resp==QDialog::Rejected)
    {
        return nullptr;
    }
    else if(resp==QDialog::Accepted)
    {
        //not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
        pNewWPolar->setName(dlg.newName().toStdString());

        insertPlanePolar(pNewWPolar);
        return pNewWPolar;

    }
    return nullptr; //should never get here
}

/**
 * Renames the active wing or plane
 * Updates the references in child polars and oppoints
 * @param PlaneName the new name for the wing or plane
 */
void Objects3d::renamePlane(QString const &PlaneName)
{
    QString OldName;
    Plane *pPlane = plane(PlaneName.toStdString());

    if(pPlane)
    {
        OldName = QString::fromStdString(pPlane->name());
        setModifiedPlane(pPlane);

        for (int l=nPolars()-1;l>=0; l--)
        {
            PlanePolar *pWPolar = planePolarAt(l);
            if (pWPolar->planeName() == OldName.toStdString())
            {
                pWPolar->setPlaneName(pPlane->name());
            }
        }
        for (int l=nPOpps()-1;l>=0; l--)
        {
            PlaneOpp *pPOpp = POppAt(l);
            if (pPOpp->planeName() == OldName.toStdString())
            {
                pPOpp->setPlaneName(pPlane->name());
            }
        }
    }
}


void Objects3d::renamePlanePolar(PlanePolar *pWPolar, Plane const *pPlane)
{
    if(!pWPolar) return;
    PlanePolar *pOldWPolar(nullptr);

    RenameDlg dlg(g_pMainFrame);
    dlg.initDialog(QString::fromStdString(pWPolar->name()), Objects3d::polarNames(pPlane), "Enter the polar's new name:");
    int resp = dlg.exec();
    if(resp==QDialog::Rejected)
    {
        return;
    }
    else if(resp==10)
    {
        //the user wants to overwrite an existing name
        if(dlg.newName().toStdString()==pWPolar->name()) return; //what's the point?

        // it's a real overwrite
        // so find and delete the existing WPolar with the new name
        for(int ipb=0; ipb<Objects3d::nPolars(); ipb++)
        {
            pOldWPolar = Objects3d::planePolarAt(ipb);
            if(pOldWPolar->name()==dlg.newName().toStdString() && pOldWPolar->planeName()==pPlane->name())
            {
                Objects3d::deletePlanePolar(pOldWPolar);
                break;
            }
        }
    }

    //ready to insert
    //remove the WPolar from its current position in the array
    for (int l=0; l<Objects3d::nPolars();l++)
    {
        pOldWPolar = Objects3d::planePolarAt(l);
        if(pOldWPolar==pWPolar)
        {
            Objects3d::removePlanePolarAt(l);
            break;
        }
    }

    //set the new name
    for (int l=Objects3d::nPOpps()-1;l>=0; l--)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(l);
        if (pWPolar->hasPOpp(pPOpp))
        {
            pPOpp->setPolarName(dlg.newName().toStdString());
        }
    }

    pWPolar->setName(dlg.newName().toStdString());

    Objects3d::insertPlanePolar(pWPolar);
}


void Objects3d::fillSectionCp3Uniform(PlaneXfl const* pPlaneXfl, PlaneOpp const*pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
{
//    qDebug()<<"istrip"<<iStrip;
    pts.clear();
    Cp.clear();

    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;
    int i3=0;
    bool bFound = false;

    std::vector<Panel3> const &panels = pPlaneXfl->triPanels();

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

    assert(p3->isTrailing());
    do
    {
        Cp.push_back(pPOpp->Cp(p3->index()*3));
        pts.push_back(p3->CoG());
        pts.back().setNormal(p3->normal());
        if(p3->iPU()==-1) p3=nullptr;
        else              p3=panels.data()+p3->index()+1;
    }
    while (p3);
}


void Objects3d::fillSectionCp3Linear(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip,
                                   std::vector<double> &Cp, std::vector<Node> &pts)
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

    std::vector<Panel3> const &panel3 = pPlaneXfl->triPanels();

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
        Cp.push_back(pPOpp->nodeValue(in));
        pts.push_back(nd);
    }
}


void Objects3d::fillSectionCp4(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
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

    std::vector<Panel4> const &panel4 = pPlaneXfl->quadpanels();
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

    assert(strip<pWing->nStations());

    if(bFound)
    {
        for (int pp=i4; pp<i4+coef*pWing->surfaceAt(0).NXPanels(); pp++)
        {
            Panel4 const &p4 = panel4.at(pWing->firstPanel4Index() + pp);
            Cp.push_back(wopp.m_dCp[pp]);
//            pts.push_back(p4.m_CollPt-WingLE);
            pts.push_back(p4.m_CollPt);
            pts.back().setNormal(p4.normal());
        }
    }
}


void Objects3d::fillSectionCp3Uniform(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iSail<0 || iSail>=pBoat->nSails()) return;

    Sail const*pWing = pBoat->sailAt(iSail);

    if(iStrip<0 || iStrip>pWing->nStations()) return;
    int i3=0;
    bool bFound = false;

    std::vector<Panel3> const &panel3 = pBoat->triPanels();

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
        assert(iStrip>=0 && iStrip<pWing->nStations());
        assert(p3->isTrailing());
        do
        {
            Cp.push_back(pBtOpp->Cp(p3->index()*3));
            pts.push_back(p3->CoG());
            pts.back().setNormal(p3->normal());
            if(p3->iPU()==-1) p3=nullptr;
            else              p3=panel3.data()+p3->index()+1;
        }
        while (p3);
    }
}


void Objects3d::fillSectionCp3Linear(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
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

    std::vector<Panel3> const &panel3 = pBoat->triPanels();

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
        Cp.push_back(pBtOpp->nodeValue(in));
        pts.push_back(nd);
    }
}


void Objects3d::makePlaneTriangulation(Plane *pPlane)
{
    std::string logmsg;
    for(int i=0; i<pPlane->nFuse(); i++)
    {
        Fuse *pFuse =  pPlane->fuse(i);
        if(pFuse->isStlType())
        {
            // do nothing, using STl triangles
        }
        else if(pFuse->isXflType())
        {
            FuseXfl* pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);
            pFuseXfl->makeSurfaceTriangulation(W3dPrefs::bodyAxialRes(), W3dPrefs::bodyHoopRes());
        }
        else if(pFuse->isOccType())
            gmesh::makeFuseTriangulation(pFuse, logmsg);
    }
}


void Objects3d::makeBoatTriangulation(Boat *pBoat)
{
    std::string logmsg;
    for(int i=0; i<pBoat->nHulls(); i++)
    {
        Fuse *pFuse =  pBoat->hull(i);
        gmesh::makeFuseTriangulation(pFuse, logmsg);
    }

    for(int is=0; is<pBoat->nSails(); is++)
    {
        Objects3d::makeSailTriangulation(pBoat->sail(is));
    }
}


// moved here to remove dependency of fl5-lib to gmsh
void Objects3d::makeSailTriangulation(Sail *pSail, int nx, int nz)
{
    SailOcc *pSailOcc = dynamic_cast<SailOcc*>(pSail);
    if(pSailOcc)
    {
        gmesh::makeSailOccTriangulation(pSailOcc);
    }
    else if(pSail)
        pSail->makeTriangulation(nx, nz);
}














