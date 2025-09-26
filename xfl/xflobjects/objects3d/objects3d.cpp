/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QCoreApplication>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include <QMutex>

#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/analysis/wpolarext.h>
#include <xflwidgets/customdlg/renamedlg.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/part.h>
#include <xflobjects/objects3d/plane/plane.h>
#include <xflobjects/objects3d/plane/planestl.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/wing/surface.h>
#include <xflcore/flow5events.h>
#include <xflcore/xflcore.h>


int Objects3d::s_Index=0;
QList <Plane*>    Objects3d::s_oaPlane;
QList <WPolar*>   Objects3d::s_oaWPolar;
QList <PlaneOpp*> Objects3d::s_oaPOpp;


int Objects3d::newUniquePartIndex()
{
    // may be accessed by different threads simultaneously e.g. from MOPSO3d class
    QMutex mutex;
    mutex.lock();
    s_Index++;
    mutex.unlock();
    return s_Index;
}


bool Objects3d::planeExists(QString const &planeName)
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
            s_oaPlane.insert(i, pPlane);
            return pPlane;
        }
    }

    // the plane does not exist, just insert in alphabetical order
    for (int j=0; j<nPlanes(); j++)
    {
        Plane const *pOldPlane = s_oaPlane.at(j);
        if (pPlane->name().compare(pOldPlane->name(), Qt::CaseInsensitive)<0)
        {
            s_oaPlane.insert(j, pPlane);
            return pPlane;
        }
    }

    //could not be inserted, append
    s_oaPlane.append(pPlane);
    return pPlane;
}


void Objects3d::insertPOpp(PlaneOpp *pPOpp)
{
    PlaneOpp *pOldPOpp = nullptr;
    bool bIsInserted = false;

    for (int i=0; i<nPOpps(); i++)
    {
        pOldPOpp = s_oaPOpp.at(i);
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

                            s_oaPOpp.removeAt(i);
                            delete pOldPOpp;
                            s_oaPOpp.insert(i, pPOpp);
                            return;
                        }
                        else if (pPOpp->alpha() < pOldPOpp->alpha())
                        {
                            //insert point
                            s_oaPOpp.insert(i, pPOpp);
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

                            s_oaPOpp.removeAt(i);
                            delete pOldPOpp;
                            s_oaPOpp.insert(i, pPOpp);
                            return;
                        }
                        else if (pPOpp->beta() < pOldPOpp->beta())
                        {
                            //insert point
                            s_oaPOpp.insert(i, pPOpp);
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

                            s_oaPOpp.removeAt(i);
                            delete pOldPOpp;
                            s_oaPOpp.insert(i, pPOpp);
                            return;
                        }
                        else if (pPOpp->ctrl() < pOldPOpp->ctrl())
                        {
                            //insert point
                            s_oaPOpp.insert(i, pPOpp);
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

                                    s_oaPOpp.removeAt(i);
                                    delete pOldPOpp;
                                    s_oaPOpp.insert(i, pPOpp);
                                    return;
                                }
                                else if (pPOpp->QInf() < pOldPOpp->QInf())
                                {
                                    //insert point
                                    s_oaPOpp.insert(i, pPOpp);
                                    return;
                                }
                            }
                            else if (pPOpp->beta() < pOldPOpp->beta())
                            {
                                //insert point
                                s_oaPOpp.insert(i, pPOpp);
                                return;
                            }
                        }
                        else if (pPOpp->alpha() < pOldPOpp->alpha())
                        {
                            //insert point
                            s_oaPOpp.insert(i, pPOpp);
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

    if (!bIsInserted)     s_oaPOpp.append(pPOpp);
}


void Objects3d::addWPolar(WPolar *pWPolar)
{
    if(!pWPolar) return;
    Plane const *pPlane = planeAt(pWPolar->planeName());
    pWPolar->setPlane(pPlane);

    for (int ip=0; ip<nPolars(); ip++)
    {
        WPolar *pOldWPlr = s_oaWPolar.at(ip);
        if (pOldWPlr->name()==pWPolar->name() && pOldWPlr->planeName()==pWPolar->planeName())
        {
            s_oaWPolar.removeAt(ip);
            delete pOldWPlr;
            s_oaWPolar.insert(ip, pWPolar);
            return;
        }
    }

    //if it doesn't exist, find its place in alphabetical order and insert it
    for (int j=0; j<nPolars(); j++)
    {
        WPolar *pOldWPlr = s_oaWPolar.at(j);
        //first key is the Plane name
        if(pWPolar->planeName().compare(pOldWPlr->planeName(), Qt::CaseInsensitive)<0)
        {
            s_oaWPolar.insert(j, pWPolar);
            return;
        }
        else if (pWPolar->planeName() == pOldWPlr->planeName())
        {
            // sort by polar name
            if(pWPolar->name().compare(pOldWPlr->name(), Qt::CaseInsensitive)<0)
            {
                s_oaWPolar.insert(j, pWPolar);
                return;
            }
        }
    }

    s_oaWPolar.append(pWPolar);
}


void Objects3d::appendWPolar(WPolar *pWPolar)
{
    if(!pWPolar) return;
    Plane const *pPlane = planeAt(pWPolar->planeName());
    pWPolar->setPlane(pPlane);

    s_oaWPolar.push_back(pWPolar);
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
            s_oaPlane.removeAt(i);
            delete pPlane;
            break;
        }
    }
}


/**
 * Deletes the WPolar and its PlaneOpp objects.
 * @param pWPolar a pointer to the WPolar object which will be deleted
 */
void Objects3d::deleteWPolar(WPolar *pWPolar)
{
    //remove and delete its children POpps from the array
    if(!pWPolar)return;

    for (int l=nPOpps()-1;l>=0; l--)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(l);
        if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->name())
        {
            s_oaPOpp.removeAt(l);
            delete pPOpp;
        }
    }

    for(int ipb=0; ipb<nPolars(); ipb++)
    {
        WPolar *pOldWPolar = s_oaWPolar.at(ipb);
        if(pOldWPolar==pWPolar)
        {
            s_oaWPolar.removeAt(ipb);
            delete pWPolar;
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
        PlaneOpp *pOldPOpp = s_oaPOpp.at(l);
        if (pOldPOpp==pPOpp)
        {
            s_oaPOpp.removeAt(l);
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
        PlaneOpp *pPOpp = s_oaPOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            s_oaPOpp.removeAt(i);
            delete pPOpp;
        }
    }

    //next delete all WPolars associated to the plane
    for (int i=nPolars()-1; i>=0; i--)
    {
        WPolar* pWPolar = s_oaWPolar.at(i);
        if (pWPolar->planeName() == pPlane->name())
        {
            if(bDeletePolars)
            {
                s_oaWPolar.removeAt(i);
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
        WPolar* pWPolar = s_oaWPolar.at(i);
        if (pWPolar->planeName() == pPlane->name() && pWPolar->isExternalPolar())
        {
            s_oaWPolar.removeAt(i);
            delete pWPolar;
        }
    }
}


void Objects3d::deleteWPolarResults(WPolar *pWPolar)
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


/**
* Returns a pointer to the OpPoint with the name of the current plane and current WPolar, and which matches the input parameter
* @param x the aoa, of velocity, or control parameter for which the CPOpp object is requested
* @return a pointer to the instance of the CPOpp object, or NULL if non has been found
*/
PlaneOpp *Objects3d::getPlaneOpp(Plane const *pPlane, WPolar const*pWPolar, QString const &oppname)
{
    if(!pPlane || !pWPolar) return nullptr;
    for (int i=0; i<s_oaPOpp.size(); i++)
    {
        PlaneOpp* pPOpp = s_oaPOpp.at(i);
        if (pPOpp->planeName()==pPlane->name() &&pPOpp->polarName()==pWPolar->name() && pPOpp->name()==oppname)
        {
            return pPOpp;
        }
    }
    return nullptr;
}


/**
* Returns a pointer to the polar with the name of the input parameter
* @param WPolarName the name of the WPolar object
* @return a pointer to the instance of the WPolar object, or NULL if none has been found
*/
WPolar* Objects3d::wPolar(const Plane *pPlane, QString const &WPolarName)
{
    if(!pPlane) return nullptr;

    for (int i=0; i<nPolars(); i++)
    {
        WPolar *pWPolar = s_oaWPolar.at(i);
        if (pWPolar->planeName()==pPlane->name() && pWPolar->name()== WPolarName)
            return pWPolar;
    }
    return nullptr;
}


Plane * Objects3d::plane(QString const &PlaneName)
{
    Plane* pPlane = nullptr;
    for (int i=0; i<nPlanes(); i++)
    {
        pPlane = s_oaPlane.at(i);
        if (pPlane->name() == PlaneName) return pPlane;
    }
    return nullptr;
}


const Plane *Objects3d::planeAt(QString const &PlaneName)
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
        s_oaPlane.removeAt(i);
        if(pPlane) delete pPlane;
    }

    for (int i=nPolars()-1; i>=0; i--)
    {
        WPolar *pWPolar = s_oaWPolar.at(i);
        s_oaWPolar.removeAt(i);
        if(pWPolar) delete pWPolar;
    }

    for (int i=nPOpps()-1; i>=0; i--)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(i);
        s_oaPOpp.removeAt(i);
        if(pPOpp) delete pPOpp;
    }
}


void Objects3d::setWPolarColor(Plane const *pPlane, WPolar *pWPolar)
{
    if(!pPlane || !pWPolar) return;
    QColor clr = pPlane->lineColor();
    for(int ip=0; ip<s_oaWPolar.size(); ip++)
    {
        if(s_oaWPolar.at(ip)->planeName().compare(pPlane->name())==0)
        {
            clr = clr.darker(xfl::darkFactor());
        }
    }
    pWPolar->setLineColor(clr);
}


void Objects3d::setPlaneStyle(Plane *pPlane, LineStyle const &ls, bool bStipple, bool bWidth, bool bColor, bool bPoints)
{
    if(!pPlane) return;
    pPlane->setTheStyle(ls);
    WPolar *pLastWPolar = nullptr;

    for(int iwp=0; iwp<s_oaWPolar.size(); iwp++)
    {
        WPolar *pWPolar = s_oaWPolar.at(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            if(bStipple) pWPolar->setLineStipple(ls.m_Stipple);
            if(bWidth) pWPolar->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastWPolar) pWPolar->setLineColor(ls.m_Color);
                else             pWPolar->setLineColor(pLastWPolar->lineColor().darker(xfl::darkFactor()));
            }
            if(bPoints) pWPolar->setPointStyle(ls.m_Symbol);

            setWPolarPOppStyle(pWPolar, bStipple, bWidth, bColor, bPoints);

            pLastWPolar = pWPolar;
        }
    }
}


void Objects3d::setPlaneVisible(const Plane *pPlane, bool bVisible, bool bStabilityPolarsOnly)
{
    if(!pPlane) return;

    for(int iwp=0; iwp<s_oaWPolar.size(); iwp++)
    {
        WPolar *pWPolar = s_oaWPolar.at(iwp);
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
    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
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


void Objects3d::setWPolarVisible(WPolar *pWPolar, bool bVisible)
{
    if(!pWPolar) return;
    pWPolar->setVisible(bVisible);
    Plane const*pPlane = plane(pWPolar->planeName());
    if(!pPlane) return;

    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pPOpp->planeName().compare(pPlane->name())==0)
        {
            if(pPOpp->polarName().compare(pWPolar->name())==0)
                pPOpp->setVisible(bVisible);
        }
    }
}


bool Objects3d::hasResults(const Plane *pPlane)
{
    for(int j=0; j<s_oaWPolar.size(); j++)
    {
        WPolar const *pWPolar = s_oaWPolar.at(j);
        if(pWPolar->planeName()==pPlane->name() && pWPolar->dataSize())
        {
            return true;
        }
    }

    for (int i=0; i<s_oaPOpp.size(); i++)
    {
        PlaneOpp const *pPOpp = s_oaPOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            return true;
        }
    }

    return false;
}


bool Objects3d::hasPOpps(const Plane *pPlane)
{
    for (int i=0; i<s_oaPOpp.size(); i++)
    {
        PlaneOpp const *pPOpp = s_oaPOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            return true;
        }
    }

    return false;
}


bool Objects3d::hasPOpps(WPolar const *pWPolar)
{
    for (int i=0; i<s_oaPOpp.size(); i++)
    {
        PlaneOpp const *pPOpp = s_oaPOpp.at(i);
        if(pPOpp->planeName() == pWPolar->planeName() && pPOpp->polarName()==pWPolar->name())
        {
            return true;
        }
    }

    return false;
}


void Objects3d::insertWPolar(WPolar *pWPolar)
{
    if(!pWPolar) return;

    for (int j=0; j<s_oaWPolar.size();j++)
    {
        WPolar const *pOldWPolar = Objects3d::wPolarAt(j);

        //first index is the parent plane's name
        if (pWPolar->planeName().compare(pOldWPolar->planeName(), Qt::CaseInsensitive)<0)
        {
            s_oaWPolar.insert(j, pWPolar);
            return;
        }
        else if(pOldWPolar->planeName().compare(pWPolar->planeName(), Qt::CaseInsensitive)==0)
        {
            // second index is the polar name
            if(pWPolar->name().compare(pOldWPolar->name(), Qt::CaseInsensitive)<0)
            {
                s_oaWPolar.insert(j, pWPolar);
                return;
            }
            else if(pWPolar->name().compare(pOldWPolar->name(), Qt::CaseInsensitive)==0)
            {
                delete pOldWPolar; // delete the old polar
                s_oaWPolar[j] = pWPolar; // and replace it
                return;
            }
        }
    }

    //something went wrong, no parent plane for this WPolar
    Objects3d::appendWPolar(pWPolar);
}


void Objects3d::setWPolarStyle(WPolar *pWPolar, LineStyle const&ls, bool bStyle, bool bWidth, bool bColor, bool bPoints)
{
    if(!pWPolar) return;
    pWPolar->setTheStyle(ls);

    setWPolarPOppStyle(pWPolar, bStyle, bWidth, bColor, bPoints);
}


void Objects3d::setWPolarPOppStyle(WPolar const* pWPolar, bool bStipple, bool bWidth, bool bColor, bool bPoints)
{
    if(!pWPolar) return;
    PlaneOpp *pLastPOpp = nullptr;
    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pWPolar->hasPOpp(pPOpp)) pLastPOpp = pPOpp;
    }

    if(!pLastPOpp) return;

    if(bStipple)  pLastPOpp->setLineStipple(pWPolar->lineStipple());
    if(bWidth)    pLastPOpp->setLineWidth(pWPolar->lineWidth());
    if(bColor)    pLastPOpp->setLineColor(pWPolar->lineColor());
    if(bPoints)   pLastPOpp->setPointStyle(pWPolar->pointStyle());

    for(int ipp=s_oaPOpp.size()-1; ipp>=0; ipp--)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pWPolar->hasPOpp(pPOpp))
        {
            if(bStipple) pPOpp->setLineStipple(pWPolar->lineStipple());
            if(bWidth)   pPOpp->setLineWidth(pWPolar->lineWidth());
            if(bColor)   pPOpp->setLineColor(pLastPOpp->lineColor().darker(xfl::darkFactor()));
            if(bPoints)  pPOpp->setPointStyle(pWPolar->pointStyle());

            pLastPOpp = pPOpp;
        }
    }
}


void Objects3d::insertPlane(Plane *pModPlane)
{
    bool bInserted = false;
    for (int l=0; l<nPlanes();l++)
    {
        Plane *pPlane = planeAt(l);
        if(pPlane == pModPlane)
        {
            // remove the current Plane from the array
            Objects3d::removePlaneAt(l);
            // but don't delete it !
            break;
        }
    }
    //and re-insert it
    for (int l=0; l<nPlanes();l++)
    {
        Plane *pPlane = planeAt(l);
        if(pPlane->name().compare(pModPlane->name(), Qt::CaseInsensitive) >0)
        {
            //then insert before
            insertPlane(l, pModPlane);
            bInserted = true;
            break;
        }
    }
    if(!bInserted)    appendPlane(pModPlane);
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
    int resp = 0;

    QString OldName = pModPlane->name();

    RenameDlg renDlg;
    renDlg.initDialog(pModPlane->name(), planeNames(), "Enter the new name for the Plane :");

    while (bExists)
    {
        resp = renDlg.exec();
        if(resp==QDialog::Accepted)
        {
            if (OldName == renDlg.newName()) return pModPlane;

            //Is the new name already used?
            bExists = planeExists(renDlg.newName());

            if(!bExists)
            {
                // we have a valid name
                // rename the plane
                pModPlane->setName(renDlg.newName());

                insertPlane(pModPlane);
                break;

            }
        }
        else if(resp ==10)
        {
            //the user wants to overwrite the old plane/wing

            Plane *pExistingPlane = plane(renDlg.newName());
            deletePlaneResults(pExistingPlane, false);
            deletePlane(pExistingPlane);

            pModPlane->setName(renDlg.newName());

            //place the Plane in alphabetical order in the array
            //remove the current Plane from the array
            for (int l=0; l<s_oaPlane.size();l++)
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
                if(pModPlane->name().compare(pOldPlane->name(), Qt::CaseInsensitive)<0)
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


/**
 * Renames the active wing or plane
 * Updates the references in child polars and oppoints
 * @param PlaneName the new name for the wing or plane
 */
void Objects3d::renamePlane(QString const &PlaneName)
{
    QString OldName;
    Plane *pPlane = plane(PlaneName);

    if(pPlane)
    {
        OldName = pPlane->name();
       setModifiedPlane(pPlane);

        for (int l=nPolars()-1;l>=0; l--)
        {
            WPolar *pWPolar = wPolarAt(l);
            if (pWPolar->planeName() == OldName)
            {
                pWPolar->setPlaneName(pPlane->name());
            }
        }
        for (int l=nPOpps()-1;l>=0; l--)
        {
            PlaneOpp *pPOpp = POppAt(l);
            if (pPOpp->planeName() == OldName)
            {
                pPOpp->setPlaneName(pPlane->name());
            }
        }
    }
}


WPolar* Objects3d::insertNewWPolar(WPolar *pNewWPolar, Plane const*pCurPlane)
{
    if(!pNewWPolar) return nullptr;

    bool bExists = true;

    //check if this WPolar is already inserted
    for(int ip=0; ip<nPolars(); ip++)
    {
         WPolar *pOldWPolar = wPolarAt(ip);
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
        WPolar *pWPolar = wPolarAt(k);
        if(pCurPlane && pWPolar->planeName()==pCurPlane->name()) NameList.append(pWPolar->name());
    }

    //Is the new WPolar's name already used?
    bExists = false;
    for (int k=0; k<NameList.count(); k++)
    {
        if(pNewWPolar->name()==NameList.at(k))
        {
            bExists = true;
            break;
        }
    }

    if(!bExists)
    {
        //just insert the WPolar in alphabetical order
        insertWPolar(pNewWPolar);
        return pNewWPolar;
    }

    // an old object with the WPolar's name exists for this Plane, ask for a new one
    RenameDlg dlg;
    dlg.initDialog(pNewWPolar->name(), polarNames(pCurPlane), "Enter the Polar's new name:");
    int resp = dlg.exec();

    if(resp==10)
    {
        //user wants to overwrite an existing name
        //so find the existing WPolar with that name
        WPolar *pWPolar = nullptr;
        for(int ipb=0; ipb<nPolars(); ipb++)
        {
             WPolar *pOldWPolar = wPolarAt(ipb);
            if(pCurPlane && pOldWPolar->name()==dlg.newName() && pOldWPolar->planeName()==pCurPlane->name())
            {
                pWPolar = pOldWPolar;
                break;
            }
        }

        if(pWPolar)
        {
            //remove and delete its children POpps from the array
            deleteWPolar(pWPolar);
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
        pNewWPolar->setName(dlg.newName());

        insertWPolar(pNewWPolar);
        return pNewWPolar;

    }
    else if(resp==QDialog::Rejected)
    {
        return nullptr;
    }
    else if(resp==QDialog::Accepted)
    {
        //not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
        pNewWPolar->setName(dlg.newName());

        insertWPolar(pNewWPolar);
        return pNewWPolar;

    }
    return nullptr; //should never get here
}


void Objects3d::updateFoilName(QString const &oldName, QString const &newName)
{
    for(int ip=0; ip<Objects3d::s_oaPlane.size(); ip++)
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


void Objects3d::renameWPolar(WPolar *pWPolar, Plane const *pPlane)
{
    if(!pWPolar) return;
    WPolar *pOldWPolar(nullptr);

    RenameDlg dlg;
    dlg.initDialog(pWPolar->name(), Objects3d::polarNames(pPlane), "Enter the polar's new name:");
    int resp = dlg.exec();
    if(resp==QDialog::Rejected)
    {
        return;
    }
    else if(resp==10)
    {
        //the user wants to overwrite an existing name
        if(dlg.newName()==pWPolar->name()) return; //what's the point?

        // it's a real overwrite
        // so find and delete the existing WPolar with the new name
        for(int ipb=0; ipb<Objects3d::nPolars(); ipb++)
        {
            pOldWPolar = Objects3d::wPolarAt(ipb);
            if(pOldWPolar->name()==dlg.newName() && pOldWPolar->planeName()==pPlane->name())
            {
                Objects3d::deleteWPolar(pOldWPolar);
                break;
            }
        }
    }

    //ready to insert
    //remove the WPolar from its current position in the array
    for (int l=0; l<Objects3d::nPolars();l++)
    {
        pOldWPolar = Objects3d::wPolarAt(l);
        if(pOldWPolar==pWPolar)
        {
            Objects3d::removeWPolarAt(l);
            break;
        }
    }

    //set the new name
    for (int l=Objects3d::nPOpps()-1;l>=0; l--)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(l);
        if (pWPolar->hasPOpp(pPOpp))
        {
            pPOpp->setPolarName(dlg.newName());
        }
    }

    pWPolar->setName(dlg.newName());

    Objects3d::insertWPolar(pWPolar);
}


QStringList Objects3d::planeNames()
{
    QStringList names;
    for(int i=0; i<nPlanes(); i++)
    {
        names.append(planeAt(i)->name());
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
            WPolar const *pPolar = s_oaWPolar.at(i);
            if(pPolar->planeName()==pPlane->name())
                names.append(pPolar->name());
        }
    }
    return names;
}


void Objects3d::cleanObjects(QString &log)
{
    log.clear();
    for (int i=s_oaPOpp.size()-1; i>=0; i--)
    {
        PlaneOpp* pPOpp = s_oaPOpp.at(i);
        Plane const*pPlane = plane(pPOpp->planeName());
        WPolar const *pWPolar = wPolar(pPlane, pPOpp->polarName());
        if(!pPlane || !pWPolar)
        {
            // remove the orphan
            log += "Deleting the orphan plane operating point " + pPOpp->planeName() + " / " + pPOpp->polarName() + " / " + pPOpp->name() + EOLCHAR;
            delete pPOpp;
            s_oaPOpp.removeAt(i);
        }
    }

    for(int i=nPolars()-1; i>=0; i--)
    {
        WPolar  *pOldWPolar = s_oaWPolar[i];
        Plane const*pPlane = plane(pOldWPolar->planeName());
        if(!pPlane)
        {
            log += "Deleting the orphan plane polar " + pOldWPolar->name() + EOLCHAR;
            deleteWPolar(pOldWPolar);
        }
    }
}


void Objects3d::updateWPolarstoV750()
{
    for(int i=nPolars()-1; i>=0; i--)
    {
        WPolar  *pOldWPolar = s_oaWPolar[i];
        Plane const*pPlane = plane(pOldWPolar->planeName());
        if(!pPlane)
        {
            // redundant safety check
            deleteWPolar(pOldWPolar);
        }
        else
        {
            PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
            if(pPlaneXfl)
                pOldWPolar->resizeFlapCtrls(pPlaneXfl);
        }
    }
}
