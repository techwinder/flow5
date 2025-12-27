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

#include <api/flow5events.h>
#include <api/triangle3d.h>

#include <interfaces/mesh/slg3d.h>


class MeshEvent : public QEvent
{

    public:
        MeshEvent(): QEvent(MESH_UPDATE_EVENT)
        {
            m_bFinal  = false;
        }

        MeshEvent(std::vector<Triangle3d> const & triangles): QEvent(MESH_UPDATE_EVENT)
        {
            m_Triangles = triangles;
//            m_SLG = AFMesher::s_SLG;
            m_bFinal  = false;
        }

        MeshEvent(std::vector<Triangle3d> const &triangles, SLG3d const &slg): QEvent(MESH_UPDATE_EVENT)
        {
            m_Triangles = triangles;
            m_SLG = slg;
            m_bFinal  = false;
        }

        std::vector<Triangle3d> const & triangles() const {return m_Triangles;}
        SLG3d const &SLG() const {return m_SLG;}
        bool isFinal() const {return m_bFinal;}
        void setFinal(bool bFinal) {m_bFinal=bFinal;}

    private:
        SLG3d m_SLG;
        std::vector<Triangle3d> m_Triangles;
        bool m_bFinal; // if true, the mesh is complete
};
