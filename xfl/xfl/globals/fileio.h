/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

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

        void readFoilFile(QFile &xFoilFile, Foil *pFoil);
        bool readPolarFile(QFile &plrFile, QVector<Foil*> &foilList, QVector<Polar*> &polarList);

        bool serializeFoil(Foil*pFoil, QDataStream &ar, bool bIsStoring);
        bool serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring);

        Polar *importXFoilPolar(QFile & txtFile, QString &logmsg);

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


