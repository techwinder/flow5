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

#define _MATH_DEFINES_DEFINED


#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>



#include <fusexfl.h>
#include <objects2d.h>
#include <objects3d.h>
#include <part.h>
#include <plane.h>
#include <planeopp.h>
#include <planestl.h>
#include <planexfl.h>
#include <surface.h>
#include <utils.h>
#include <planepolar.h>
#include <planepolarext.h>


int Objects3d::s_Index=0;
std::vector<Plane*>        Objects3d::s_oaPlane;
std::vector<PlanePolar*>   Objects3d::s_oaPlanePolar;
std::vector<PlaneOpp*>     Objects3d::s_oaPlaneOpp;


int Objects3d::newUniquePartIndex()
{
    // may be accessed by different threads simultaneously e.g. from MOPSO3d class
    QMutex mutex;
    mutex.lock();
    s_Index++;
    mutex.unlock();
    return s_Index;
}


bool Objects3d::planeExists(std::string const &planeName)
{
    Plane *pOldPlane = nullptr;

    for (int i=0; i<nPlanes(); i++)
    {
        pOldPlane = s_oaPlane.at(i);
        if (pOldPlane->name() == planeName)
        {
            return true;
        }
    }
    return false;
}


Plane *Objects3d::addPlane(Plane *pPlane)
{
    for (int i=0; i<nPlanes(); i++)
    {
        Plane *pOldPlane = s_oaPlane.at(i);
        if (pOldPlane->name().compare(pPlane->name())==0)
        {
            //a plane with this name already exists
            // if its the same plane, just return
            if(pOldPlane==pPlane) return pPlane;

            // if its an old plane with the same name, delete and insert at its place
            deletePlane(pOldPlane);
            s_oaPlane.insert(s_oaPlane.begin()+i, pPlane);
            return pPlane;
        }
    }

    // the plane does not exist, just insert in alphabetical order
    for (int j=0; j<nPlanes(); j++)
    {
        Plane const *pOldPlane = s_oaPlane.at(j);
        if (pPlane->name().compare(pOldPlane->name())<0)
        {
            s_oaPlane.insert(s_oaPlane.begin()+j, pPlane);
            return pPlane;
        }
    }

    //could not be inserted, append
    s_oaPlane.push_back(pPlane);
    return pPlane;
}


void Objects3d::insertPlaneOpp(PlaneOpp *pPOpp)
{
    PlaneOpp *pOldPOpp = nullptr;
    bool bIsInserted = false;

    for (int i=0; i<nPOpps(); i++)
    {
        pOldPOpp = s_oaPlaneOpp.at(i);
        if (pPOpp->planeName().compare(pOldPOpp->planeName())==0)
        {
            if (pPOpp->polarName().compare(pOldPOpp->polarName())==0)
            {
                switch(pPOpp->polarType())
                {
                    case xfl::T1POLAR:
                    case xfl::T2POLAR:
                    case xfl::T3POLAR:
                    {
                        if(fabs(pPOpp->alpha() - pOldPOpp->alpha())<0.0005)
                        {
                            //replace the existing point
                            pPOpp->setTheStyle(pOldPOpp->theStyle());

                            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
                            delete pOldPOpp;
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        else if (pPOpp->alpha() < pOldPOpp->alpha())
                        {
                            //insert point
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        break;
                    }
                    case xfl::T5POLAR:
                    {
                        if(fabs(pPOpp->beta() - pOldPOpp->beta())<0.0005)
                        {
                            //replace the existing point
                            pPOpp->setTheStyle(pOldPOpp->theStyle());

                            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
                            delete pOldPOpp;
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        else if (pPOpp->beta() < pOldPOpp->beta())
                        {
                            //insert point
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        break;
                    }
                    case xfl::T7POLAR:
                    case xfl::T6POLAR:
                    {
                        if(fabs(pPOpp->ctrl() - pOldPOpp->ctrl())<0.001)
                        {
                            //replace the existing point
                            pPOpp->setTheStyle(pOldPOpp->theStyle());

                            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
                            delete pOldPOpp;
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        else if (pPOpp->ctrl() < pOldPOpp->ctrl())
                        {
                            //insert point
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        break;
                    }
                    case xfl::T8POLAR:
                    {
                        // Type 8 analysis, sort by alpha then beta then QInf
                        if(fabs(pPOpp->alpha() - pOldPOpp->alpha())<0.0005)
                        {
                            if(fabs(pPOpp->beta() - pOldPOpp->beta())<0.0005)
                            {
                                if(fabs(pPOpp->QInf() - pOldPOpp->QInf())<0.0005)
                                {
                                    //replace the existing point
                                    pPOpp->setTheStyle(pOldPOpp->theStyle());

                                    s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
                                    delete pOldPOpp;
                                    s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                                    return;
                                }
                                else if (pPOpp->QInf() < pOldPOpp->QInf())
                                {
                                    //insert point
                                    s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                                    return;
                                }
                            }
                            else if (pPOpp->beta() < pOldPOpp->beta())
                            {
                                //insert point
                                s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                                return;
                            }
                        }
                        else if (pPOpp->alpha() < pOldPOpp->alpha())
                        {
                            //insert point
                            s_oaPlaneOpp.insert(s_oaPlaneOpp.begin()+i, pPOpp);
                            return;
                        }
                        break;
                    }
                    default: // rubberchord, beta, fixedaoa
                        break;
                }
            }
        }
    }

    if (!bIsInserted)     s_oaPlaneOpp.push_back(pPOpp);
}


void Objects3d::addPlPolar(PlanePolar *pWPolar)
{
    if(!pWPolar) return;
    Plane const *pPlane = planeAt(pWPolar->planeName());
    pWPolar->setPlane(pPlane);

    for (int ip=0; ip<nPolars(); ip++)
    {
        PlanePolar *pOldWPlr = s_oaPlanePolar.at(ip);
        if (pOldWPlr->name()==pWPolar->name() && pOldWPlr->planeName()==pWPolar->planeName())
        {
            s_oaPlanePolar.erase(s_oaPlanePolar.begin()+ip);
            delete pOldWPlr;
            s_oaPlanePolar.insert(s_oaPlanePolar.begin()+ip, pWPolar);
            return;
        }
    }

    //if it doesn't exist, find its place in alphabetical order and insert it
    for (int j=0; j<nPolars(); j++)
    {
        PlanePolar *pOldWPlr = s_oaPlanePolar.at(j);
        //first key is the Plane name
        if(pWPolar->planeName().compare(pOldWPlr->planeName())<0)
        {
            s_oaPlanePolar.insert(s_oaPlanePolar.begin()+j, pWPolar);
            return;
        }
        else if (pWPolar->planeName() == pOldWPlr->planeName())
        {
            // sort by polar name
            if(pWPolar->name().compare(pOldWPlr->name())<0)
            {
                s_oaPlanePolar.insert(s_oaPlanePolar.begin()+j, pWPolar);
                return;
            }
        }
    }

    s_oaPlanePolar.push_back(pWPolar);
}


void Objects3d::appendPlPolar(PlanePolar *pWPolar)
{
    if(!pWPolar) return;
    Plane const *pPlane = planeAt(pWPolar->planeName());
    pWPolar->setPlane(pPlane);

    s_oaPlanePolar.push_back(pWPolar);
}


/**
 * Deletes a Plane object from the array.
 * @param pPlane a pointer to the Plane object to be deleted
 */
void Objects3d::deletePlane(Plane *pPlane, bool bDeleteResults)
{
    if(!pPlane || !pPlane->name().length()) return;

    if(bDeleteResults)
        deletePlaneResults(pPlane);

    for (int i=nPlanes()-1; i>=0; i--)
    {
        Plane *pOldPlane = s_oaPlane.at(i);
        if (pOldPlane == pPlane)
        {
            s_oaPlane.erase(s_oaPlane.begin()+i);
            delete pPlane;
            break;
        }
    }
}


/**
 * Deletes the WPolar and its PlaneOpp objects.
 * @param pPlPolar a pointer to the WPolar object which will be deleted
 */
void Objects3d::deletePlPolar(PlanePolar *pPlPolar)
{
    //remove and delete its children POpps from the array
    if(!pPlPolar)return;

    for (int l=nPOpps()-1;l>=0; l--)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(l);
        if (pPOpp->planeName()==pPlPolar->planeName() && pPOpp->polarName()==pPlPolar->name())
        {
            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+l);
            delete pPOpp;
        }
    }

    for(int ipb=0; ipb<nPolars(); ipb++)
    {
        PlanePolar *pOldWPolar = s_oaPlanePolar.at(ipb);
        if(pOldWPolar==pPlPolar)
        {
            s_oaPlanePolar.erase(s_oaPlanePolar.begin()+ipb);
            delete pPlPolar;
            break;
        }
    }
}


/**
 * Deletes the WPolar and its PlaneOpp objects.
 * @param pWPolar a pointer to the WPolar object which will be deleted
 */
void Objects3d::deletePlaneOpp(PlaneOpp *pPOpp)
{
    //remove and delete the POpp from the array
    if(!pPOpp)return;

    for (int l=nPOpps()-1;l>=0; l--)
    {
        PlaneOpp *pOldPOpp = s_oaPlaneOpp.at(l);
        if (pOldPOpp==pPOpp)
        {
            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+l);
            delete pOldPOpp;
        }
    }
}


/**
 * Deletes the WPolar and PlaneOpp objects associated to the plane.
 * @param pPlane a pointer to the Plane object for which the results will be deleted
 */
void Objects3d::deletePlaneResults(const Plane *pPlane, bool bDeletePolars)
{
    if(!pPlane || !pPlane->name().length()) return;

    //first remove all POpps associated to the plane
    for (int i=nPOpps()-1; i>=0; i--)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
            delete pPOpp;
        }
    }

    //next delete all WPolars associated to the plane
    for (int i=nPolars()-1; i>=0; i--)
    {
        PlanePolar* pWPolar = s_oaPlanePolar.at(i);
        if (pWPolar->planeName() == pPlane->name())
        {
            if(bDeletePolars)
            {
                s_oaPlanePolar.erase(s_oaPlanePolar.begin()+i);
                delete pWPolar;
            }
            else
            {
                pWPolar->clearWPolarData();
                pWPolar->resetAngleRanges(pPlane);
            }
        }
    }
}


void Objects3d::deleteExternalPolars(Plane const*pPlane)
{
    if(!pPlane) return;

    //next delete all WPolars associated to the plane
    for (int i=nPolars()-1; i>=0; i--)
    {
        PlanePolar* pWPolar = s_oaPlanePolar.at(i);
        if (pWPolar->planeName() == pPlane->name() && pWPolar->isExternalPolar())
        {
            s_oaPlanePolar.erase(s_oaPlanePolar.begin()+i);
            delete pWPolar;
        }
    }
}


void Objects3d::deleteWPolarResults(PlanePolar *pWPolar)
{
    pWPolar->clearWPolarData();
    for(int i=Objects3d::nPOpps()-1; i>=0; --i)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(i);
        if(pPOpp->polarName()==pWPolar->name() && pPOpp->planeName()==pWPolar->planeName())
        {
            Objects3d::removePOppAt(i);
            delete pPOpp;
        }
    }
}


PlaneOpp *Objects3d::planeOpp(Plane const *pPlane, PlanePolar const*pWPolar, std::string const &oppname)
{
    if(!pPlane || !pWPolar) return nullptr;
    std::string planename = pPlane->name();
    std::string plpolarname = pWPolar->name();

    for (int i=0; i<nPOpps(); i++)
    {
        PlaneOpp* pPOpp = s_oaPlaneOpp.at(i);
        std::string poppplanename = pPOpp->planeName();
        std::string popppolarname = pPOpp->polarName();
        std::string poppname = pPOpp->name();

        if (poppplanename==planename && popppolarname==plpolarname && poppname==oppname)
        {
            return pPOpp;
        }
    }
    return nullptr;
}


PlanePolar* Objects3d::wPolar(const Plane *pPlane, std::string const &WPolarName)
{
    if(!pPlane) return nullptr;

    for (int i=0; i<nPolars(); i++)
    {
        PlanePolar *pWPolar = s_oaPlanePolar.at(i);
        if (pWPolar->planeName()==pPlane->name() && pWPolar->name()== WPolarName)
            return pWPolar;
    }
    return nullptr;
}


Plane * Objects3d::plane(std::string const &PlaneName)
{
    Plane* pPlane = nullptr;
    for (int i=0; i<nPlanes(); i++)
    {
        pPlane = s_oaPlane.at(i);
        if (pPlane->name() == PlaneName) return pPlane;
    }
    return nullptr;
}


const Plane *Objects3d::planeAt(std::string const &PlaneName)
{
    Plane* pPlane = nullptr;
    for (int i=0; i<nPlanes(); i++)
    {
        pPlane = s_oaPlane.at(i);
        if (pPlane->name() == PlaneName) return pPlane;
    }
    return nullptr;
}

void Objects3d::deleteObjects()
{
    for (int i=nPlanes()-1; i>=0; i--)
    {
        Plane *pPlane = s_oaPlane.at(i);
        s_oaPlane.erase(s_oaPlane.begin()+i);
        if(pPlane) delete pPlane;
    }

    for (int i=nPolars()-1; i>=0; i--)
    {
        PlanePolar *pWPolar = s_oaPlanePolar.at(i);
        s_oaPlanePolar.erase(s_oaPlanePolar.begin()+i);
        if(pWPolar) delete pWPolar;
    }

    for (int i=nPOpps()-1; i>=0; i--)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(i);
        s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
        if(pPOpp) delete pPOpp;
    }
}


void Objects3d::setPlPolarColor(Plane const *pPlane, PlanePolar *pWPolar, int darkfactor)
{
    if(!pPlane || !pWPolar) return;
    fl5Color clr = pPlane->lineColor();
    for(int ip=0; ip<nPolars(); ip++)
    {
        if(s_oaPlanePolar.at(ip)->planeName().compare(pPlane->name())==0)
        {
            clr = clr.darker(darkfactor);
        }
    }
    pWPolar->setLineColor(clr);
}


void Objects3d::setPlaneStyle(Plane *pPlane, LineStyle const &ls, bool bStipple, bool bWidth, bool bColor, bool bPoints, int darkfactor)
{
    if(!pPlane) return;
    pPlane->setTheStyle(ls);
    PlanePolar *pLastWPolar = nullptr;

    for(int iwp=0; iwp<nPolars(); iwp++)
    {
        PlanePolar *pWPolar = s_oaPlanePolar.at(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            if(bStipple) pWPolar->setLineStipple(ls.m_Stipple);
            if(bWidth) pWPolar->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastWPolar) pWPolar->setLineColor(ls.m_Color);
                else             pWPolar->setLineColor(pLastWPolar->lineColor().darker(darkfactor));
            }
            if(bPoints) pWPolar->setPointStyle(ls.m_Symbol);

            setPlPolarPOppStyle(pWPolar, bStipple, bWidth, bColor, bPoints, darkfactor);

            pLastWPolar = pWPolar;
        }
    }
}


void Objects3d::setPlaneVisible(const Plane *pPlane, bool bVisible, bool bStabilityPolarsOnly)
{
    if(!pPlane) return;

    for(int iwp=0; iwp<nPolars(); iwp++)
    {
        PlanePolar *pWPolar = s_oaPlanePolar.at(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            /*            if(bStabilityPolarsOnly)
            {
                if(pWPolar->isStabilityPolar()) pWPolar->setVisible(bVisible);
            }
            else*/
            pWPolar->setVisible(bVisible);
        }
    }
    for(int ipp=0; ipp<nPOpps(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(ipp);
        if(pPOpp->planeName().compare(pPlane->name())==0)
        {
            if(bStabilityPolarsOnly)
            {
                if(pPOpp->isType7()) pPOpp->setVisible(bVisible);
            }
            else
                pPOpp->setVisible(bVisible);
        }
    }
}


void Objects3d::setPlPolarVisible(PlanePolar *pWPolar, bool bVisible)
{
    if(!pWPolar) return;
    pWPolar->setVisible(bVisible);
    Plane const*pPlane = plane(pWPolar->planeName());
    if(!pPlane) return;

    for(int ipp=0; ipp<nPOpps(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(ipp);
        if(pPOpp->planeName().compare(pPlane->name())==0)
        {
            if(pPOpp->polarName().compare(pWPolar->name())==0)
                pPOpp->setVisible(bVisible);
        }
    }
}


bool Objects3d::hasResults(const Plane *pPlane)
{
    for(int j=0; j<nPolars(); j++)
    {
        PlanePolar const *pWPolar = s_oaPlanePolar.at(j);
        if(pWPolar->planeName()==pPlane->name() && pWPolar->dataSize())
        {
            return true;
        }
    }

    for (int i=0; i<nPOpps(); i++)
    {
        PlaneOpp const *pPOpp = s_oaPlaneOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            return true;
        }
    }

    return false;
}


bool Objects3d::hasPOpps(const Plane *pPlane)
{
    for (int i=0; i<nPOpps(); i++)
    {
        PlaneOpp const *pPOpp = s_oaPlaneOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            return true;
        }
    }

    return false;
}


bool Objects3d::hasPOpps(PlanePolar const *pWPolar)
{
    for (int i=0; i<nPOpps(); i++)
    {
        PlaneOpp const *pPOpp = s_oaPlaneOpp.at(i);
        if(pPOpp->planeName() == pWPolar->planeName() && pPOpp->polarName()==pWPolar->name())
        {
            return true;
        }
    }

    return false;
}


void Objects3d::insertPlPolar(PlanePolar *pWPolar)
{
    if(!pWPolar) return;

    for (int j=0; j<nPolars();j++)
    {
        PlanePolar const *pOldWPolar = Objects3d::plPolarAt(j);

        //first index is the parent plane's name
        if (pWPolar->planeName().compare(pOldWPolar->planeName())<0)
        {
            s_oaPlanePolar.insert(s_oaPlanePolar.begin()+j, pWPolar);
            return;
        }
        else if(pOldWPolar->planeName().compare(pWPolar->planeName())==0)
        {
            // second index is the polar name
            if(pWPolar->name().compare(pOldWPolar->name())<0)
            {
                s_oaPlanePolar.insert(s_oaPlanePolar.begin()+j, pWPolar);
                return;
            }
            else if(pWPolar->name().compare(pOldWPolar->name())==0)
            {
                delete pOldWPolar; // delete the old polar
                s_oaPlanePolar[j] = pWPolar; // and replace it
                return;
            }
        }
    }

    //something went wrong, no parent plane for this WPolar
    Objects3d::appendPlPolar(pWPolar);
}


void Objects3d::setPlPolarStyle(PlanePolar *pWPolar, LineStyle const&ls, bool bStyle, bool bWidth, bool bColor, bool bPoints, int darkfactor)
{
    if(!pWPolar) return;
    pWPolar->setTheStyle(ls);

    setPlPolarPOppStyle(pWPolar, bStyle, bWidth, bColor, bPoints, darkfactor);
}


void Objects3d::setPlPolarPOppStyle(PlanePolar const* pWPolar, bool bStipple, bool bWidth, bool bColor, bool bPoints, int darkfactor)
{
    if(!pWPolar) return;
    PlaneOpp *pLastPOpp = nullptr;
    for(int ipp=0; ipp<nPOpps(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(ipp);
        if(pWPolar->hasPOpp(pPOpp)) pLastPOpp = pPOpp;
    }

    if(!pLastPOpp) return;

    if(bStipple)  pLastPOpp->setLineStipple(pWPolar->lineStipple());
    if(bWidth)    pLastPOpp->setLineWidth(pWPolar->lineWidth());
    if(bColor)    pLastPOpp->setLineColor(pWPolar->lineColor());
    if(bPoints)   pLastPOpp->setPointStyle(pWPolar->pointStyle());

    for(int ipp=nPOpps()-1; ipp>=0; ipp--)
    {
        PlaneOpp *pPOpp = s_oaPlaneOpp.at(ipp);
        if(pWPolar->hasPOpp(pPOpp))
        {
            if(bStipple) pPOpp->setLineStipple(pWPolar->lineStipple());
            if(bWidth)   pPOpp->setLineWidth(pWPolar->lineWidth());
            if(bColor)   pPOpp->setLineColor(pLastPOpp->lineColor().darker(darkfactor));
            if(bPoints)  pPOpp->setPointStyle(pWPolar->pointStyle());

            pLastPOpp = pPOpp;
        }
    }
}


void Objects3d::insertPlane(Plane *pModPlane)
{
    bool bInserted = false;

    QString planename = QString::fromStdString(pModPlane->name());

    for (int l=0; l<nPlanes();l++)
    {
        Plane *pPlane = planeAt(l);
        if(pPlane == pModPlane)
        {
            // remove the mod Plane from the array
            Objects3d::removePlaneAt(l);
            // but don't delete it !
            break;
        }
    }
    //and re-insert it
    for (int l=0; l<nPlanes();l++)
    {
        Plane *pPlane = planeAt(l);
        if(QString::fromStdString(pPlane->name()).compare(planename, Qt::CaseInsensitive)>0)
        {
            //then insert before
            insertPlane(l, pModPlane);
            bInserted = true;
            break;
        }
    }
    if(!bInserted)    appendPlane(pModPlane);
}


void Objects3d::updateFoilName(std::string const &oldName, std::string const &newName)
{
    for(int ip=0; ip<nPlanes(); ip++)
    {
        Plane  *pPlane = Objects3d::s_oaPlane.at(ip);
        for(int iw=0;  iw<pPlane->nWings(); iw++)
        {
            WingXfl *pWingXfl = pPlane->wing(iw);
            for(int is=0; is<pWingXfl->nSections(); is++)
            {
                WingSection &ws = pWingXfl->section(is);
                if(ws.rightFoilName()==oldName)  ws.setRightFoilName(newName);
                if(ws.leftFoilName() ==oldName)  ws.setLeftFoilName( newName);
            }
        }
    }
}


void Objects3d::cleanObjects(std::string &log)
{
    log.clear();
    for (int i=nPOpps()-1; i>=0; i--)
    {
        PlaneOpp* pPOpp = s_oaPlaneOpp.at(i);
        Plane const*pPlane = plane(pPOpp->planeName());
        PlanePolar const *pWPolar = wPolar(pPlane, pPOpp->polarName());
        if(!pPlane || !pWPolar)
        {
            // remove the orphan
            log += "Deleting the orphan plane operating point " +
                    pPOpp->planeName() + " / " +
                    pPOpp->polarName() + " / " +
                    pPOpp->name()      + "\n";
            delete pPOpp;
            s_oaPlaneOpp.erase(s_oaPlaneOpp.begin()+i);
        }
    }

    for(int i=nPolars()-1; i>=0; i--)
    {
        PlanePolar  *pOldWPolar = s_oaPlanePolar[i];
        Plane const*pPlane = plane(pOldWPolar->planeName());
        if(!pPlane)
        {
            log += "Deleting the orphan plane polar " +
                    pOldWPolar->name() + "\n";
            deletePlPolar(pOldWPolar);
        }
    }
}


void Objects3d::updatePlPolarstoV750()
{
    for(int i=nPolars()-1; i>=0; i--)
    {
        PlanePolar  *pOldWPolar = s_oaPlanePolar[i];
        Plane const*pPlane = plane(pOldWPolar->planeName());
        if(!pPlane)
        {
            // redundant safety check
            deletePlPolar(pOldWPolar);
        }
        else
        {
            PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            if(pPlaneXfl)
                pOldWPolar->resizeFlapCtrls(pPlaneXfl);
        }
    }
}


void Objects3d::renamePlane(Plane*pPlane, std::string const &newname)
{
    if(!pPlane) return;

    std::string const &OldName = pPlane->name();

    pPlane->setName(newname);

    for (int l=0; l<nPolars(); l++)
    {
        PlanePolar *pWPolar = plPolarAt(l);
        if (pWPolar->planeName() == OldName)
        {
            pWPolar->setPlaneName(newname);
        }
    }
    for (int l=0; l<nPOpps(); l++)
    {
        PlaneOpp *pPOpp = POppAt(l);
        if (pPOpp->planeName() == OldName)
        {
            pPOpp->setPlaneName(newname);
        }
    }
}


void Objects3d::renamePlPolar(PlanePolar *pWPolar, const std::string &newname)
{
    if(!pWPolar) return;

    std::string const &planename = pWPolar->planeName();
    std::string const &OldName = pWPolar->name();

    pWPolar->setName(newname);

    for (int l=0; l<nPOpps(); l++)
    {
        PlaneOpp *pPOpp = POppAt(l);
        if (pPOpp->planeName()==planename && pPOpp->polarName()==OldName)
        {
            pPOpp->setPolarName(newname);
        }
    }
}


bool Objects3d::containsPOpp(PlaneOpp *pPOpp)
{
    if(std::find(s_oaPlaneOpp.begin(), s_oaPlaneOpp.end(), pPOpp) != s_oaPlaneOpp.end())
    {
        return true;
    }

    return false;
}


PlaneOpp *Objects3d::storePlaneOpps(std::vector<PlaneOpp*> const &POppList)
{
    QList<PlaneOpp*> QPOppList;
    for(PlaneOpp *pPOpp : POppList)
    {
        QPOppList.append(pPOpp);
    }
    return storePlaneOpps(QPOppList);
}


PlaneOpp *Objects3d::storePlaneOpps(QList<PlaneOpp*> const &POppList)
{
    PlaneOpp * pSelectedPOpp = nullptr;
    for(int iPOpp=0; iPOpp<POppList.size(); iPOpp++)
    {
        PlaneOpp *pPOpp = POppList.at(iPOpp);
        if(!pPOpp->isOut())
        {
            pSelectedPOpp = pPOpp; // return a valid PlaneOpp
            Objects3d::insertPlaneOpp(pPOpp);
        }
        else
        {
            delete pPOpp;
            pPOpp = nullptr;
        }
    }
    return pSelectedPOpp;
}













