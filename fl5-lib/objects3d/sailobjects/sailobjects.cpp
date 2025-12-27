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

#include <sailobjects.h>

#include <boat.h>
#include <boatpolar.h>
#include <boatopp.h>

std::vector<Boat*> SailObjects::s_oaBoat;
std::vector<BoatPolar*> SailObjects::s_oaBtPolar;
std::vector<BoatOpp*> SailObjects::s_oaBtOpp;

BoatOpp *SailObjects::s_pLastBtOpp = nullptr;

int SailObjects::s_SailDarkFactor=105;



void SailObjects::deleteObjects()
{
    for(int i=0; i<nBoats(); i++)
    {
        delete s_oaBoat[i];
    }
    s_oaBoat.clear();

    for(int i=0; i<nBtPolars(); i++)
    {
        delete s_oaBtPolar[i];
    }
    s_oaBtPolar.clear();

    for(int i=0; i<nBtOpps(); i++)
    {
        delete s_oaBtOpp[i];
    }
    s_oaBtOpp.clear();
}


void SailObjects::deleteBoat(Boat *pBoat, bool bDeleteBtPolars)
{
    if(!pBoat) return;

    if(bDeleteBtPolars) deleteBtPolars(pBoat);

    for(int i=0; i<nBoats(); i++)
    {
        if(s_oaBoat.at(i)==pBoat)
        {
            delete pBoat;
            s_oaBoat.erase(s_oaBoat.begin()+i);
            return;
        }
    }
}

void SailObjects::deleteBtPolars(Boat *pBoat)
{
    if(!pBoat) return;
    for(int i=nBtPolars()-1; i>=0; i--)
    {
        BoatPolar *pBtPlr = s_oaBtPolar.at(i);
        if(pBtPlr->boatName()==pBoat->name())
        {
            deleteBtPolar(pBtPlr);
        }
    }
}


void SailObjects::deleteBtPolar(BoatPolar *pBtPolar)
{
    deleteBtPolarOpps(pBtPolar);
    for(int i=0; i<nBtPolars(); i++)
    {
        if(s_oaBtPolar.at(i) == pBtPolar)
        {
            delete pBtPolar;
            s_oaBtPolar.erase(s_oaBtPolar.begin()+i);
            return;
        }
    }
}

/** @todo do not expose, does not free */
void SailObjects::removeBoatAt(int idx)
{
    if(idx>=0 && idx<nBoats())
    s_oaBoat.erase(s_oaBoat.begin()+idx);
}


void SailObjects::removeBtPolarAt(int idx)
{
    if(idx>=0 && idx<nBtPolars())
    s_oaBtPolar.erase(s_oaBtPolar.begin()+idx);
}


void SailObjects::removeBtOppAt(int idx)
{
    if(idx>=0 && idx<nBtOpps())
    s_oaBtOpp.erase(s_oaBtOpp.begin()+idx);
}


void SailObjects::deleteAllBtOpps()
{
    for(int i=0; i<nBtOpps(); i++)
    {
        delete s_oaBtOpp.at(i);
    }
    s_oaBtOpp.clear();
}


void SailObjects::deleteBoatBtOpps(Boat *pBoat)
{
    if(!pBoat) return;
    for(int i=nBtOpps()-1; i>=0; i--)
    {
        BoatOpp *pBtOpp = s_oaBtOpp.at(i);
        if(pBtOpp->boatName()==pBoat->name())
        {
            delete pBtOpp;
            s_oaBtOpp.erase(s_oaBtOpp.begin()+i);
        }
    }
}


void SailObjects::deleteBtPolarOpps(BoatPolar *pBtPolar)
{
    if(!pBtPolar) return;
    for(int i=nBtOpps()-1; i>=0; i--)
    {
        BoatOpp *pBtOpp = s_oaBtOpp.at(i);
        if(pBtOpp->boatName()==pBtPolar->boatName() && pBtOpp->polarName()==pBtPolar->name())
        {
            delete pBtOpp;
            s_oaBtOpp.erase(s_oaBtOpp.begin()+i);
        }
    }
}


void SailObjects::setBoatStyle(Boat *pBoat, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints)
{
    if(!pBoat) return;
    pBoat->setTheStyle(ls);
    BoatPolar *pLastBPolar = nullptr;
    BoatOpp *pLastBOpp = nullptr;
    for(int iwp=0; iwp<nBtPolars(); iwp++)
    {
        BoatPolar *pBtPolar = s_oaBtPolar.at(iwp);
        if(pBtPolar->boatName().compare(pBoat->name())==0)
        {
            if(bStyle) pBtPolar->setLineStipple(ls.m_Stipple);
            if(bWidth) pBtPolar->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastBPolar) pBtPolar->setLineColor(ls.m_Color);
                else             pBtPolar->setLineColor(pLastBPolar->lineColor().darker(s_SailDarkFactor));
            }
            if(bPoints) pBtPolar->setPointStyle(ls.m_Symbol);

            pLastBPolar = pBtPolar;
        }
    }
    for(int ipp=0; ipp<nBtOpps(); ipp++)
    {
        BoatOpp *pBtOpp = s_oaBtOpp.at(ipp);
        if(pBtOpp->boatName().compare(pBoat->name())==0)
        {
            if(bStyle) pBtOpp->setLineStipple(ls.m_Stipple);
            if(bWidth) pBtOpp->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastBOpp) pBtOpp->setLineColor(ls.m_Color);
                else           pBtOpp->setLineColor(pLastBOpp->lineColor().darker(s_SailDarkFactor));
            }
            if(bPoints) pBtOpp->setPointStyle(ls.m_Symbol);

            pLastBOpp = pBtOpp;
        }
    }
}


void SailObjects::setBPolarStyle(BoatPolar *pBPolar, LineStyle const &ls, bool bStyle, bool bWidth, bool bColor, bool bPoints)
{
    if(!pBPolar) return;
    pBPolar->setTheStyle(ls);

    BoatOpp *pLastPOpp = nullptr;
    Boat *pBoat = boat(pBPolar->boatName());
    if(!pBoat) return;

    for(int ipp=nBtOpps()-1; ipp>=0; ipp--)
    {
        BoatOpp *pBtOpp = s_oaBtOpp.at(ipp);
        if(pBtOpp->boatName().compare(pBoat->name())==0 && pBtOpp->polarName().compare(pBPolar->name())==0)
        {
            if(bStyle) pBtOpp->setLineStipple(ls.m_Stipple);
            if(bWidth) pBtOpp->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastPOpp) pBtOpp->setLineColor(ls.m_Color);
                else           pBtOpp->setLineColor(pLastPOpp->lineColor().darker(s_SailDarkFactor));
            }
            if(bPoints) pBtOpp->setPointStyle(ls.m_Symbol);

            pLastPOpp = pBtOpp;
        }
    }
}


Boat * SailObjects::boat(const std::string &boatName)
{
    for (int i=0; i<nBoats(); i++)
    {
        Boat* pBoat = s_oaBoat.at(i);
        if (pBoat->name() == boatName) return pBoat;
    }
    return nullptr;
}


/**
* Returns a pointer to the OpPoint with the name of the current plane and current WPolar, and which matches the input parameter
* @param x the aoa, of velocity, or control parameter for which the POpp object is requested
* @return a pointer to the instance of the POpp object, or NULL if non has been found
*/
BoatOpp * SailObjects::getBoatOpp(Boat const *pBoat, BoatPolar const* pBPolar, double x)
{
    if(!pBoat || !pBPolar) return nullptr;

    BoatOpp* pBOpp;

    for (int i=0; i<nBtOpps(); i++)
    {
        pBOpp = s_oaBtOpp.at(i);
        if ((pBOpp->boatName() == pBoat->name()) && (pBOpp->polarName() == pBPolar->name()))
        {
            if (fabs(pBOpp->ctrl() - x)<0.001)  return pBOpp;
        }
    }
    return nullptr;
}



/**
* Returns a pointer to the polar with the name of the input parameter
* @param WPolarName the name of the WPolar object
* @return a pointer to the instance of the WPolar object, or NULL if non has been found
*/
BoatPolar* SailObjects::btPolar(Boat const *pBoat, std::string const &BtPolarName)
{
    if(!pBoat) return nullptr;

    for (int i=0; i<nBtPolars(); i++)
    {
        BoatPolar *pWPolar = s_oaBtPolar.at(i);
        if (pWPolar->boatName()==pBoat->name() && pWPolar->name()== BtPolarName)
            return pWPolar;
    }
    return nullptr;
}


void SailObjects::setWPolarColor(Boat *pBoat, BoatPolar *pBPolar)
{
    if(!pBoat || !pBPolar) return;
    fl5Color clr = pBoat->lineColor();
    for(int ip=0; ip<nBtPolars(); ip++)
    {
        if(s_oaBtPolar.at(ip)->boatName().compare(pBoat->name())==0)
        {
            clr = clr.darker(s_SailDarkFactor);
        }
    }
    pBPolar->setLineColor(clr);
}



void SailObjects::setBoatVisible(Boat *pPlane, bool bVisible)
{
    if(!pPlane) return;

    for(int iwp=0; iwp<nBtPolars(); iwp++)
    {
        BoatPolar *pBPolar = s_oaBtPolar.at(iwp);
        if(pBPolar->boatName().compare(pPlane->name())==0)
        {
            pBPolar->setVisible(bVisible);
        }
    }
    for(int ipp=0; ipp<nBtOpps(); ipp++)
    {
        BoatOpp *pBOpp = s_oaBtOpp.at(ipp);
        if(pBOpp->boatName().compare(pPlane->name())==0)
        {
            pBOpp->setVisible(bVisible);
        }
    }
}


void SailObjects::setBPolarVisible(BoatPolar *pBPolar, bool bVisible)
{
    if(!pBPolar) return;
    pBPolar->setVisible(bVisible);
    Boat *pBoat = boat(pBPolar->boatName());
    if(!pBoat) return;

    for(int ipp=0; ipp<nBtOpps(); ipp++)
    {
        BoatOpp *pBtOpp = s_oaBtOpp.at(ipp);
        if(pBtOpp->boatName().compare(pBoat->name())==0)
        {
            if(pBtOpp->polarName().compare(pBPolar->name())==0)
                pBtOpp->setVisible(bVisible);
        }
    }
}



/** Checks the array for an existing plane with the name in the input
 * @param planeName: the plane's name to check
 * @return true if a plane with the same name already exists, false otherwise
*/
bool SailObjects::boatExists(std::string const &boatname)
{
    Boat *pOldBoat;

    for (int i=0; i<nBoats(); i++)
    {
        pOldBoat = s_oaBoat.at(i);
        if (pOldBoat->name() == boatname)
        {
            return true;
        }
    }
    return false;
}


/**
 * Deletes the WPolar and PlaneOpp objects associated to the plane.
 * @param pPlane a pointer to the Plane object for which the results will be deleted
 */
void SailObjects::deleteBoatResults(Boat *pBoat, bool bDeletePolars)
{
    if(!pBoat || !pBoat->name().length()) return ;
    BoatPolar* pBtPolar;
    BoatOpp * pBtOpp;

    //first remove all POpps associated to the Boat
    for (int i=nBtOpps()-1; i>=0; i--)
    {
        pBtOpp = s_oaBtOpp.at(i);
        if(pBtOpp->boatName() == pBoat->name())
        {
            s_oaBtOpp.erase(s_oaBtOpp.begin()+i);
            delete pBtOpp;
        }
    }

    //next delete all WPolars associated to the Boat
    for (int i=nBtPolars()-1; i>=0; i--)
    {
        pBtPolar = s_oaBtPolar.at(i);
        if (pBtPolar->boatName() == pBoat->name())
        {
            if(bDeletePolars /*|| pWPolar->isControlPolar() || pWPolar->isStabilityPolar()*/)
            {
                s_oaBtPolar.erase(s_oaBtPolar.begin()+i);
                delete pBtPolar;
                pBtPolar = nullptr;
            }
            else
            {
                pBtPolar->clearData();
            }
        }
    }
}


void SailObjects::insertBtOpp(BoatOpp *pBtOpp)
{
    BoatOpp *pOldBtOpp = nullptr;
    bool bIsInserted = false;

    for (int i=0; i<nBtOpps(); i++)
    {
        pOldBtOpp = s_oaBtOpp.at(i);
        if (pBtOpp->boatName() == pOldBtOpp->boatName())
        {
            if (pBtOpp->polarName() == pOldBtOpp->polarName())
            {
                if(fabs(pBtOpp->ctrl() - pOldBtOpp->ctrl())<0.0001)
                {
                    //replace existing point
                    pBtOpp->setTheStyle(pOldBtOpp->theStyle());

                    s_oaBtOpp.erase(s_oaBtOpp.begin()+i);
                    delete pOldBtOpp;
                    s_oaBtOpp.insert(s_oaBtOpp.begin()+i, pBtOpp);
                    bIsInserted = true;
                    i = nBtOpps();// to break
                }
                else if (pBtOpp->ctrl() > pOldBtOpp->ctrl())
                {
                    //insert point
                    s_oaBtOpp.insert(s_oaBtOpp.begin()+i, pBtOpp);
                    bIsInserted = true;
                    i = nBtOpps();// to break
                }
            }
        }
    }

    if (!bIsInserted)     s_oaBtOpp.push_back(pBtOpp);
}


void SailObjects::storeBtOpps(BoatPolar *pBtPolar, std::vector<BoatOpp*> const &BtOppList)
{
    if(!pBtPolar || BtOppList.size()==0)
    {
        s_pLastBtOpp = nullptr;
        return;
    }
    fl5Color clr = pBtPolar->lineColor();

    for(uint k=0; k<BtOppList.size(); k++)
    {
        BoatOpp *pBtOpp = BtOppList.at(k);

        pBtOpp->setLineColor(clr);
        clr = clr.darker(s_SailDarkFactor);
        bool bIsInserted = false;

        for (int i=0; i<nBtOpps(); i++)
        {
            BoatOpp *pOldBtOpp = s_oaBtOpp.at(i);
            if (pBtOpp->boatName() == pOldBtOpp->boatName())
            {
                if (pBtOpp->polarName() == pOldBtOpp->polarName())
                {
                    if(fabs(pBtOpp->ctrl() - pOldBtOpp->ctrl())<0.0001)
                    {
                        //replace existing point
                        pBtOpp->setTheStyle(pOldBtOpp->theStyle());

                        s_oaBtOpp.erase(s_oaBtOpp.begin()+i);
                        delete pOldBtOpp;
                        s_oaBtOpp.insert(s_oaBtOpp.begin()+i, pBtOpp);
                        bIsInserted = true;
                        i = nBtOpps();// to break
                    }
                    else if (pBtOpp->ctrl() > pOldBtOpp->ctrl())
                    {
                        //insert point
                        s_oaBtOpp.insert(s_oaBtOpp.begin()+i, pBtOpp);
                        bIsInserted = true;
                        i = nBtOpps();// to break
                    }

                }
            }
        }
        if (!bIsInserted) s_oaBtOpp.push_back(pBtOpp);
        s_pLastBtOpp = pBtOpp;
    }
}


BoatOpp* SailObjects::getBoatOppObject(Boat*pBoat, BoatPolar *pBtPolar,
                                       double ctrl)
{
    if(!pBoat || !pBtPolar) return nullptr;

    BoatOpp *pBtOpp = SailObjects::getBoatOpp(pBoat, pBtPolar, ctrl);

    if(!pBtOpp)
    {
        //try to select the opp with control value closest to ctrl
        double ctrlmax = 1e10;
        for(int iBtOpp=0; iBtOpp<SailObjects::nBtOpps(); iBtOpp++)
        {
            BoatOpp *pOldBtOpp = SailObjects::btOpp(iBtOpp);
            if(pOldBtOpp->boatName()==pBoat->name() && pOldBtOpp->polarName()==pBtPolar->name())
            {
                if(fabs(pOldBtOpp->ctrl()-ctrl)<ctrlmax)  pBtOpp = pOldBtOpp;
            }
        }
    }

    return pBtOpp;
}


void SailObjects::insertThisBoat(Boat*pNewBoat)
{
    for(int ib=0; ib<nBoats(); ib++)
    {
        Boat *const pOldBoat = s_oaBoat.at(ib);
        if(pNewBoat->name().compare(pOldBoat->name())<0)
        {
            s_oaBoat.insert(s_oaBoat.begin()+ib, pNewBoat);
            return;
        }
    }
    s_oaBoat.push_back(pNewBoat);
}


void SailObjects::insertBtPolar(BoatPolar *pBtPolar)
{
    for(int ib=0; ib<nBtPolars(); ib++)
    {
        BoatPolar *const pOldBtPolar = s_oaBtPolar.at(ib);
        if(pBtPolar->name().compare(pOldBtPolar->name())<0)
        {
            s_oaBtPolar.insert(s_oaBtPolar.begin()+ib, pBtPolar);
            return;
        }
        else if(pBtPolar->name().compare(pOldBtPolar->name())==0)
        {
            if(pBtPolar->boatName().compare(pOldBtPolar->boatName())==0)
            {
                deleteBtPolar(pOldBtPolar);
                s_oaBtPolar.insert(s_oaBtPolar.begin()+ib, pBtPolar);
            }
            else s_oaBtPolar.insert(s_oaBtPolar.begin()+ib, pBtPolar);
            return;
        }
    }
    s_oaBtPolar.push_back(pBtPolar);
}


// Check if this boat has results
bool SailObjects::hasResults(Boat const*pBoat)
{
    BoatPolar *pBtPolar;
    for(int j=0; j<nBtPolars(); j++)
    {
        pBtPolar = s_oaBtPolar.at(j);
        if(pBtPolar->name()==pBoat->name() && pBtPolar->dataSize())
        {
            return true;
        }
    }

    for (int i=0; i<nBtOpps(); i++)
    {
        BoatOpp *pBtOpp = s_oaBtOpp.at(i);
        if(pBtOpp->boatName() == pBoat->name())
        {
            return true;
        }
    }

    return false;
}


std::vector<std::string> SailObjects::boatNames()
{
    std::vector<std::string> names;
    for(int i=0; i<nBoats(); i++)
    {
        names.push_back(boat(i)->name());
    }
    return names;
}


std::vector<std::string> SailObjects::polarNames(Boat const*pBoat)
{
    std::vector<std::string> names;
    if(pBoat)
    {
        for(int i=0; i<nBtPolars(); i++)
        {
            BoatPolar const *pPolar = s_oaBtPolar.at(i);
            if(pPolar->boatName()==pBoat->name())
                names.push_back(pPolar->name());
        }
    }
    return names;
}


BoatOpp *SailObjects::btOpp(Boat const *pBoat, BoatPolar const*pPolar, std::string const &oppname)
{
    if(!pBoat || !pPolar) return nullptr;
    std::string btname = pBoat->name();
    std::string polarname = pPolar->name();

    for (BoatOpp *pBtOpp : s_oaBtOpp)
    {
        std::string poppbtname = pBtOpp->boatName();
        std::string popppolarname = pBtOpp->polarName();
        std::string poppname = pBtOpp->name();

        if (poppbtname==btname && popppolarname==polarname && poppname==oppname)
        {
            return pBtOpp;
        }
    }
    return nullptr;
}
