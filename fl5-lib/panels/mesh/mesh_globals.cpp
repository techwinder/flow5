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


#include <mesh_globals.h>
#include <panel4.h>



void rotateQuadMesh(std::vector<Panel4> &panel4, double alpha, double beta)
{
    Vector3d Origin;

    for(uint i4=0; i4<panel4.size(); i4++)
    {
        Panel4 &p4 = panel4[i4];
        for(int in=0; in<4; in++)
        {
            p4.m_Node[in].rotateZ(Origin, beta);
            p4.m_Node[in].rotateY(Origin, alpha);
        }
        p4.setPanelFrame();
    }
}
