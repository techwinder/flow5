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

#include <QVector>
#include <QEvent>
#include <QString>

// Custom event identifier
const QEvent::Type MESSAGE_EVENT             = static_cast<QEvent::Type>(QEvent::User + 101);
const QEvent::Type STREAMLINE_END_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 102);
const QEvent::Type XFOIL_END_TASK_EVENT      = static_cast<QEvent::Type>(QEvent::User + 103);
const QEvent::Type XFOIL_END_OPP_EVENT       = static_cast<QEvent::Type>(QEvent::User + 104);
const QEvent::Type PLANE_END_TASK_EVENT      = static_cast<QEvent::Type>(QEvent::User + 105);
const QEvent::Type PLANE_END_POPP_EVENT      = static_cast<QEvent::Type>(QEvent::User + 106);
const QEvent::Type VPW_UPDATE_EVENT          = static_cast<QEvent::Type>(QEvent::User + 107);
const QEvent::Type OPTIM_ITER_EVENT          = static_cast<QEvent::Type>(QEvent::User + 108);
const QEvent::Type OPTIM_END_EVENT           = static_cast<QEvent::Type>(QEvent::User + 109);
const QEvent::Type OPTIM_PARTICLE_EVENT      = static_cast<QEvent::Type>(QEvent::User + 110);
const QEvent::Type OPTIM_MAKESWARM_EVENT     = static_cast<QEvent::Type>(QEvent::User + 111);
const QEvent::Type LLT_OPP_EVENT             = static_cast<QEvent::Type>(QEvent::User + 112);
const QEvent::Type MESH_UPDATE_EVENT         = static_cast<QEvent::Type>(QEvent::User + 113);
const QEvent::Type MESH2D_UPDATE_EVENT       = static_cast<QEvent::Type>(QEvent::User + 114);


class Foil;
class Polar;
class OpPoint;
class Particle;
class XFoilTask;

class MessageEvent : public QEvent
{
    public:
        MessageEvent(QString const &msg): QEvent(MESSAGE_EVENT)
        {
            m_Msg = msg;
        }

        QString const & msg() const {return m_Msg;}

    private:
        QString m_Msg;
};


class StreamEndTaskEvent : public QEvent
{
    public:
        StreamEndTaskEvent(int index): QEvent(STREAMLINE_END_TASK_EVENT),
            m_Index(index)
        {
            m_Index=-1;
        }

        int index() const {return m_Index;}

    private:
        int m_Index=-1;
};


class PlaneTaskEvent : public QEvent
{
    public:
        PlaneTaskEvent(void * pPlane, void *pWPolar): QEvent(PLANE_END_TASK_EVENT),
            m_pPlane(pPlane),
            m_pWPolar(pWPolar)
        {
        }

        void * planePtr() const {return m_pPlane;}
        void * wPolarPtr() const {return m_pWPolar;}

    private:
        void *m_pPlane;
        void *m_pWPolar;
};


class LLTOppEvent : public QEvent
{
    public:
        LLTOppEvent(double alpha, QVector<double>const &xc, QVector<double>const &yc): QEvent(LLT_OPP_EVENT),
            m_alpha(alpha), m_xc(xc), m_yc(yc)
        {
        }

        double alpha() const {return m_alpha;}
        QVector<double> const &xc() const {return m_xc;}
        QVector<double> const &yc() const {return m_yc;}

    private:
        double m_alpha;
        QVector<double> m_xc, m_yc;
};



class XFoilTaskEvent : public QEvent
{
    public:
        XFoilTaskEvent(XFoilTask *pTask, Foil const*pFoil, Polar *pPolar): QEvent(XFOIL_END_TASK_EVENT),
            m_pXFoilTask(pTask),
            m_pFoil(pFoil),
            m_pPolar(pPolar)
        {
        }

        XFoilTask*task() {return m_pXFoilTask;}
        Foil const*foil() const  {return m_pFoil;}
        Polar * polar() const    {return m_pPolar;}

    private:
        XFoilTask *m_pXFoilTask = nullptr;
        Foil const*m_pFoil=nullptr;
        Polar *m_pPolar=nullptr;
};


class XFoilOppEvent : public QEvent
{
    public:
        XFoilOppEvent(Foil * pFoil, Polar *pPolar, OpPoint *pOpPoint): QEvent(XFOIL_END_OPP_EVENT),
            m_pFoil(pFoil),
            m_pPolar(pPolar)
        {
    //  memcpy(&m_XFoil, pXFoilPtr, sizeof(XFoil));
            m_pOpPoint = pOpPoint;
        }


        Foil * foilPtr() const {return m_pFoil;}
        Polar * polarPtr() const {return m_pPolar;}
        OpPoint * theOpPoint() {return m_pOpPoint;}

    private:
        Foil *m_pFoil=nullptr;
        Polar *m_pPolar=nullptr;
        OpPoint *m_pOpPoint=nullptr;    /** need to store current XFoil results */
};
