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

#include <node2d.h>



void Node2d::rotateZ(Vector2d const &O, double beta)
{
    Vector2d::rotateZ(O, beta);
    m_N.rotateZ(beta);
}


void Node2d::rotateZ(double beta)
{
    Vector2d::rotateZ(beta);
    m_N.rotateZ(beta);
}

void Node2d::setNode(Node2d const &nd)
{
    x = nd.x;
    y = nd.y;
    m_N.set(nd.m_N);
    m_Index = nd.index();
}

