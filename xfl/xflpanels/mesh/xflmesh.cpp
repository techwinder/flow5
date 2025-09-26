/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xflmesh.h"
#include <xflobjects/objects_globals/objects_global.h>

double XflMesh::s_NodeMergeDistance=1.e-4; // 0.1mm
QVector<Vector3d> XflMesh::s_DebugPts;



void XflMesh::listNodes()
{
    QString strange, str;
    for(int in=0; in<m_Node.size(); in++)
    {
        Node const &nd = m_Node.at(in);
        strange = QString::asprintf("node[%3d] (%9g %9g %9g)\n", nd.index(), nd.x, nd.y,nd.z);
        str = QString::asprintf("  normal (%9g %9g %9g)\n", nd.normal().x, nd.normal().y,nd.normal().z);
        strange += str;

        strange += "  " + surfacePosition(nd.surfacePosition()) + "\n";

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
