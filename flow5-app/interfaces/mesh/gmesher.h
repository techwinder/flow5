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

#include <QObject>

#include <api/flow5events.h>


class GMesher : public QObject
{
    Q_OBJECT
    public:
        GMesher(QObject *parent = nullptr);

        void setGmshParams(double emin, double emax, int iAlgo, int iCurvature);

    public slots:
        void onMeshCurrentModel();

    signals:
        void meshDone(bool bError);
        void displayMessage(QString const &msg);

    private:

};

