/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QEvent>
#include <QString>



// Custom event identifier
const QEvent::Type FOIL_END_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 3);
const QEvent::Type FOIL_END_OPP_EVENT  = static_cast<QEvent::Type>(QEvent::User + 4);


class Polar;
class Foil;
class OpPoint;


class FoilTaskEvent : public QEvent
{

    public:
        FoilTaskEvent(void *pFoilTask): QEvent(FOIL_END_TASK_EVENT),
            m_pFoilTask(pFoilTask)
        {
        }

        void * foilTaskPtr() const	{return m_pFoilTask;}

    private:
        void *m_pFoilTask=nullptr;
};


/** posts back the operating point to the parent */
class FoilOppEvent : public QEvent
{
    public:
        FoilOppEvent(Foil * pFoil, Polar *pPolar, OpPoint *pOpp): QEvent(FOIL_END_OPP_EVENT),
            m_pFoil(pFoil),
            m_pPolar(pPolar),
            m_pOpp(pOpp)
        {
        }

        Foil  *foilPtr()  const {return m_pFoil;}
        Polar *polarPtr() const {return m_pPolar;}
        OpPoint *opPoint() const {return m_pOpp;}

    private:
        Foil    *m_pFoil=nullptr;
        Polar   *m_pPolar=nullptr;
        OpPoint *m_pOpp=nullptr;
};



