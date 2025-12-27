/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <fl5lib_global.h>

class Foil;
class Plane;
class Polar;
class PlanePolar;


// must use QObject to get signal/slot mechanism for interactive IO
class FL5LIB_EXPORT FileIO : public QObject
{
    Q_OBJECT
    public:
        FileIO();

    public:
        bool serializeProjectFl5(QDataStream &ar, bool bIsStoring);
        int serializeProjectMetaDataFl5(QDataStream &ar, bool bIsStoring);


        bool serializeProjectXfl(QDataStream &ar, bool bIsStoring, PlanePolar *pMetaWPolar);

        bool serializeBtObjectsFl5(QDataStream &ar, bool bIsStoring);

        bool serialize2dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat);
        bool serialize3dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat);

        bool storeFoilsFl5(const std::vector<Foil *> &FoilSelection, QDataStream &ar, bool bAll);


        bool storePlaneFl5(Plane *pPlane, QDataStream &ar);

        static bool bOpps()   {return s_bSaveOpps;}
        static bool bPOpps()  {return s_bSavePOpps;}
        static bool bBtOpps() {return s_bSaveBtOpps;}

        static void saveOpps(bool b)   {s_bSaveOpps  =b;}
        static void savePOpps(bool b)  {s_bSavePOpps =b;}
        static void saveBtOpps(bool b) {s_bSaveBtOpps=b;}


    private:
        void outputMessage(QString const &msg);


    public slots:
        void onLoadProject(const QString &filename);

    signals:
        void fileLoaded(bool bError);
        void displayMessage(QString const &msg);

    private:
        static bool s_bSaveOpps;
        static bool s_bSavePOpps;
        static bool s_bSaveBtOpps;

};


