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

#include <QEvent>
#include <QRunnable>

#include <api/vector3d.h>
#include <api/flow5events.h>

class PlaneOpp;
class Vector3d;
class Polar3d;
class P4Analysis;
class P3Analysis;
class PlaneXfl;
class Boat;
class Panel3;
class Panel4;


class StreamlineMaker : public QRunnable
{
    public:
        StreamlineMaker(QObject *pParent=nullptr);
        ~StreamlineMaker();

    public:
        void run();
        void initializeLineMaker(int index, float *pStreamVertexArray, Vector3d const &C0, Vector3d const &VA, Vector3d const &TC, int NX, double L0, double XFactor);

        void setOpp(Polar3d const *pPolar3d, double QInf, double alpha, double beta, const double *mu, const double *sigma);


        void setP4Analysis(P4Analysis *p4a) {m_pP4Analysis=p4a;}
        void setP3Analysis(P3Analysis *p3a) {m_pP3Analysis=p3a;}

        static void cancelTasks(bool bCancel) {s_bCancel = bCancel;}
        static bool isCancelled() {return s_bCancel;}


    private:

        float *m_pStreamVertexArray;
        Vector3d m_C0, m_TC;      /**< the postion of the streamline's starting point */
        Vector3d m_UnitDir0;      /**< the direction of the streamline's first segment */
        int m_Index;

    public:
        QObject* m_pParent;

        Polar3d const *m_pPolar3d;

        P4Analysis *m_pP4Analysis;
        P3Analysis *m_pP3Analysis;

        QVector<Panel3> const * panel3;
        QVector<Panel3> const * wakepanel3;
        QVector<Panel4> const * panel4;
        QVector<Panel4> const * wakepanel4;

        double const *m_Mu;
        double const *m_Sigma;

        double m_QInf, m_Alpha, m_Beta;

        int m_NX;
        double m_L0;
        double m_XFactor;

        static bool s_bCancel;

};

