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

#pragma once

#include <vector>
#include <QEvent>

// Custom event identifier
const QEvent::Type MESSAGE_EVENT             = static_cast<QEvent::Type>(QEvent::User + 101);
const QEvent::Type STREAMLINE_END_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 102);
const QEvent::Type XFOIL_TASK_END_EVENT      = static_cast<QEvent::Type>(QEvent::User + 103);
const QEvent::Type XFOIL_BATCH_END_EVENT     = static_cast<QEvent::Type>(QEvent::User + 104);
const QEvent::Type PLANE_END_TASK_EVENT      = static_cast<QEvent::Type>(QEvent::User + 105);
const QEvent::Type PLANE_END_POPP_EVENT      = static_cast<QEvent::Type>(QEvent::User + 106);
const QEvent::Type VPW_UPDATE_EVENT          = static_cast<QEvent::Type>(QEvent::User + 107);
const QEvent::Type OPTIM_ITER_EVENT          = static_cast<QEvent::Type>(QEvent::User + 108);
const QEvent::Type OPTIM_END_EVENT           = static_cast<QEvent::Type>(QEvent::User + 109);
const QEvent::Type OPTIM_PARTICLE_EVENT      = static_cast<QEvent::Type>(QEvent::User + 110);
const QEvent::Type OPTIM_MAKESWARM_EVENT     = static_cast<QEvent::Type>(QEvent::User + 111);
const QEvent::Type MESH_UPDATE_EVENT         = static_cast<QEvent::Type>(QEvent::User + 113);
const QEvent::Type MESH2D_UPDATE_EVENT       = static_cast<QEvent::Type>(QEvent::User + 114);
const QEvent::Type LLT_OPP_EVENT             = static_cast<QEvent::Type>(QEvent::User + 115);
const QEvent::Type TASK3D_END_EVENT        = static_cast<QEvent::Type>(QEvent::User + 116);



#include <fl5lib_global.h>

class Foil;
class Polar;
class OpPoint;
class Particle;
class XFoilTask;


class FL5LIB_EXPORT MessageEvent : public QEvent
{
    public:
        MessageEvent(std::string const &msg): QEvent(MESSAGE_EVENT)
        {
            m_Msg = QString::fromStdString(msg);
        }

        MessageEvent(QString const &msg): QEvent(MESSAGE_EVENT)
        {
            m_Msg = msg;
        }

        QString const & msg() const {return m_Msg;}

    private:
        QString m_Msg;
};


class FL5LIB_EXPORT StreamEndTaskEvent : public QEvent
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


class FL5LIB_EXPORT PlaneTaskEvent : public QEvent
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


class FL5LIB_EXPORT LLTOppEvent : public QEvent
{
    public:
        LLTOppEvent(double alpha, std::vector<double>const &max_a, std::string const &msg): QEvent(LLT_OPP_EVENT)
        {
            m_alpha = alpha;
            m_max_a = max_a;
            m_Msg = msg;
        }

        double alpha() const {return m_alpha;}
        std::vector<double> const &max_a() const {return m_max_a;}
        std::string const &message() const {return m_Msg;}

    private:
        double m_alpha;
        std::vector<double> m_max_a;
        std::string m_Msg;
};



class FL5LIB_EXPORT XFoilTaskEvent : public QEvent
{
    public:
        XFoilTaskEvent(XFoilTask *pTask): QEvent(XFOIL_TASK_END_EVENT),
            m_pXFoilTask(pTask)
        {
        }

        XFoilTask*task() {return m_pXFoilTask;}

    private:
        XFoilTask *m_pXFoilTask = nullptr;
};

