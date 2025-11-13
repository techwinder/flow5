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


#include <xflobjects/objects_globals/fl5object.h>
#include <xflobjects/objects3d/fuse/fuse.h>

class BoatOpp;
class BoatPolar;
class Fuse;
class FuseXfl;
class NURBSSail;
class Node;

class Sail;
class Vector3d;

class Boat : public fl5Object
{
    friend class BoatDlg;
    friend class BoatTask;

    public:
        Boat();
        Boat(const Boat &aboat);
        ~Boat();

        void makeDefaultBoat();
        void triangulateSails();

        void makeConnections();
        void makeRefTriMesh(bool bIncludeHull, bool bMultiThread);

        void duplicate(const Boat *pBoat);

        Sail *sail(QString const &SailName);
        Sail *sail(int index) {if (index>=0&&index<m_Sail.size()) return m_Sail.at(index); else return nullptr;}
        Sail const *sailAt(int index) const {if (index>=0&&index<m_Sail.size()) return m_Sail.at(index); else return nullptr;}

        NURBSSail *appendNewNurbsSail();
        void appendSail(Sail *pSail);
        bool removeSail(int iSail);

        QString properties(bool bFull) const;

        void clearSails();
        void clearHulls();

        bool serializeBoatFl5(QDataStream &ar, bool bIsStoring);

        int sailCount() const {return m_Sail.size();}
        int nSails()    const {return m_Sail.size();}

        Fuse *hull(QString const &BodyName) const;
        Fuse *hull(int index) const {if(index>=0&&index<m_Hull.size()) return m_Hull.at(index); else return nullptr;}
        Fuse const *hullAt(int index) const {if(index>=0&&index<m_Hull.size()) return m_Hull.at(index); else return nullptr;}

        Fuse *makeNewHull(Fuse::enumType bodytype);
        void removeFuse(Fuse *pFuse);
        FuseXfl *appendNewXflHull();
        void appendHull(Fuse *pFuse);
        bool removeHull(int iHull);

        int nHulls() const {return m_Hull.size();}
        int hullCount() const {return m_Hull.size();}
        int fuseCount() const {return m_Hull.size();}
        int xflFuseCount() const;
        int occFuseCount() const;
        int stlFuseCount() const;

        Vector3d fusePos(int idx) const;
        void setFusePos(int idx, Vector3d pos);

        Vector3d hullLE(int iHull);
        void setHullLE(int iHull, Vector3d const &LEPos);

        bool hasSail() const {return m_Sail.size()>0;}
        bool hasJib()  const {return m_Sail.size()>1;}
        bool hasHull() const {return m_Hull.size()>0;}

        double length() const;
        double height() const;
        double referenceLength() const {return std::max(length(), height());} //to scale properly the 3d display

        Sail *mainSail();
        Sail const *mainSail() const;
        Sail *jib();
        Sail const *jib() const;
        Fuse * hull();
        Fuse const * hull() const;

        bool hasBtPolar(const BoatPolar *pBPolar) const;
        bool hasBOpp(const BoatOpp *pBOpp) const;


        void rotateMesh(const BoatPolar *pBtPolar, double phi, double Ry, double ctrl, QVector<Panel3> &panels) const;


    private:
        QVector <Sail*> m_Sail;
        QVector <Fuse*> m_Hull;

};


