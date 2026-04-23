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

#include <QString>



#include <xflmesh.h>
#include <objects_global.h>

double XflMesh::s_NodeMergeDistance=1.e-4; // 0.1mm
std::vector<Vector3d> XflMesh::s_DebugPts;



void XflMesh::listNodes()
{
    QString strange, str;
    for(uint in=0; in<m_Node.size(); in++)
    {
        Node const &nd = m_Node.at(in);
        strange = QString::asprintf("node[%3d] (%9g %9g %9g)\n", nd.index(), nd.x, nd.y,nd.z);
        str = QString::asprintf("  normal (%9g %9g %9g)\n", nd.normal().x, nd.normal().y,nd.normal().z);
        strange += str;

        strange += "  " + QString::fromStdString(objects::surfacePosition(nd.surfacePosition())) + "\n";

        str = "  Connected nodes:";
        for(int i=0; i<nd.neighbourNodeCount(); i++)
            str += QString::asprintf("  %d", nd.neigbourNodeIndex(i));
        strange += str + "\n";

        str = "  Triangles:";
        for(int i=0; i<nd.triangleCount(); i++)
            str += QString::asprintf("  %d", nd.triangleIndex(i));
        strange += str + "\n";
        qDebug("%s", strange.toStdString().c_str());
    }
}
