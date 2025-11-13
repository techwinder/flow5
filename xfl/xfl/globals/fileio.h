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

#include <QObject>
#include <QFile>

#include <xflcore/enums_core.h>

class Foil;
class Plane;
class Polar;
class WPolar;


// must use QObject to get signal/slot mechanism for interactive IO
class FileIO : public QObject
{
    Q_OBJECT
    public:
        FileIO();

    public:
        bool serializeProjectFl5(QDataStream &ar, bool bIsStoring);
        int serializeProjectMetaDataFl5(QDataStream &ar, bool bIsStoring);


        bool serializeProjectXfl(QDataStream &ar, bool bIsStoring, WPolar *pMetaWPolar);

        bool serializeBtObjectsFl5(QDataStream &ar, bool bIsStoring);

        bool serialize2dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat);
        bool serialize3dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat);

        bool storeFoilsFl5(QVector<Foil*>const &FoilSelection, QDataStream &ar, bool bAll);


        bool storePlaneFl5(Plane *pPlane, QDataStream &ar);

    private:
        void outputMessage(QString const &msg);
        void customEvent(QEvent *pEvent) override;


    public slots:
        void onLoadProject(const QString &filename);

    signals:
//        void fileLoaded(xfl::enumApp iApp, bool bError); // Qt6
        void fileLoaded(int iApp, bool bError); // Qt5
        void displayMessage(QString const &msg);


};


