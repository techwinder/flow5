/****************************************************************************

    flow5
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



