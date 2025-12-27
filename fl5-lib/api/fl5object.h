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

/**
 * @brief The fl5Object class FL5LIB_EXPORT is the base class FL5LIB_EXPORT for planes and boats
 */

#include <xflobject.h>
#include <enums_objects.h>
#include <linestyle.h>
#include <trimesh.h>

class FL5LIB_EXPORT fl5Object : public XflObject
{
    public:
        fl5Object() = default;
        virtual ~fl5Object() = default;

    public:

        const std::string & description() const {return m_Description;}
        void setDescription(std::string const &description) {m_Description = description;}


        int nPanel3() const                     {return m_TriMesh.nPanels();}
        int nNodes()  const                     {return m_TriMesh.nNodes();}
        Panel3 const &panel3At(int index) const {return m_TriMesh.panelAt(index);}
        Panel3       &panel3(int index)         {return m_TriMesh.panel(index);}
        Node const &node(int index) const       {return m_TriMesh.nodeAt(index);}
        TriMesh const &triMesh() const          {return m_TriMesh;}
        TriMesh &triMesh()                      {return m_TriMesh;}
        TriMesh const & refTriMesh() const      {return m_RefTriMesh;}
        TriMesh & refTriMesh()                  {return m_RefTriMesh;}
        virtual void restoreMesh() {m_TriMesh = m_RefTriMesh;}



        std::vector<Panel3> &triPanels()             {return m_TriMesh.panels();}
        std::vector<Panel3> const &triPanels() const {return m_TriMesh.panels();}

        void setRefTriMesh(TriMesh const &trimesh) {m_RefTriMesh=trimesh;}
        void setTriMesh(   TriMesh const &trimesh) {m_TriMesh=trimesh;}


    protected:
        std::string m_Description;
        TriMesh m_RefTriMesh; /** The reference triangular mesh, with non-rotated panels */
        TriMesh m_TriMesh;     /** The active triangular mesh, with panels rotated with bank angle and sail angle */

};


