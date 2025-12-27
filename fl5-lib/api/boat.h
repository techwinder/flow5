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


#include <fl5object.h>
#include <fuse.h>

class BoatOpp;
class BoatPolar;
class Fuse;
class FuseXfl;
class SailNurbs;
class Node;

class Sail;
class Vector3d;

class FL5LIB_EXPORT Boat : public fl5Object
{

    public:
        Boat();
        Boat(const Boat &aboat);
        ~Boat();

        void makeDefaultBoat();

        void makeConnections();
        void makeRefTriMesh(bool bIncludeHull, bool bMultiThread);

        void duplicate(const Boat *pBoat);

        Sail *sail(std::string const &SailName);
        Sail *sail(int index) {if (index>=0&&index<int(m_Sail.size())) return m_Sail.at(index); else return nullptr;}
        Sail const *sailAt(int index) const {if (index>=0&&index<int(m_Sail.size())) return m_Sail.at(index); else return nullptr;}

        SailNurbs *appendNewNurbsSail();
        void appendSail(Sail *pSail);
        void insertSailAt(int index, Sail*pSail);
        bool removeSailAt(int iSail); /** @todo do not expose, doesn't delete*/
        bool deleteSail(int iSail);

        std::string properties(bool bFull) const;

        void clearSails();
        void clearHulls();

        bool serializeBoatFl5(QDataStream &ar, bool bIsStoring);

        int nSails() const {return int(m_Sail.size());}

        Fuse *hull(std::string const &BodyName) const;
        Fuse *hull(int index) const {if(index>=0&&index<int(m_Hull.size())) return m_Hull.at(index); else return nullptr;}
        Fuse const *hullAt(int index) const {if(index>=0&&index<int(m_Hull.size())) return m_Hull.at(index); else return nullptr;}

        Fuse *makeNewHull(Fuse::enumType bodytype);
        FuseXfl *appendNewXflHull();
        void appendHull(Fuse *pFuse);
        void insertHullAt(int index, Fuse *pFuse);
        bool removeHullAt(int iHull); /** @todo do not expose, doesn't delete*/
        bool deleteHull(int iHull);
        void deleteFuse(Fuse *pFuse);


        int nHulls() const {return int(m_Hull.size());}
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


        void rotateMesh(const BoatPolar *pBtPolar, double phi, double Ry, double ctrl, std::vector<Panel3> &panels) const;


    private:
        std::vector <Sail*> m_Sail;
        std::vector <Fuse*> m_Hull;

};


