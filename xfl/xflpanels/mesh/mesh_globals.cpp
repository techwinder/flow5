/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#include "mesh_globals.h"
#include <xflpanels/panels/panel4.h>



void rotateQuadMesh(QVector<Panel4> &panel4, double alpha, double beta)
{
    Vector3d Origin;

    for(int i4=0; i4<panel4.size(); i4++)
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
