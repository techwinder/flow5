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

#define _MATH_DEFINES_DEFINED

#include <QGridLayout>

#include "thinsaildlg.h"

#include <api/mathelem.h>
#include <api/sail.h>
#include <interfaces/mesh/gmesherwt.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>


ThinSailDlg::ThinSailDlg(QWidget *pParent) : SailDlg(pParent)
{
}


void ThinSailDlg::onRuledMesh()
{
    bool bRuled = m_prbRuledMesh->isChecked();

    m_pglSailView->clearHighlightList();
    m_pglSailView->clearSegments();

    m_pfrRuledMesh->setVisible(bRuled);
    m_pGMesherWt->setVisible(!bRuled);

    m_pSail->setRuledMesh(bRuled);
    if(bRuled)
    {
        m_ppto->onAppendQText("Making sail ruled mesh\n");
        m_pSail->makeRuledMesh(Vector3d()); // fast and automatic
        m_ppto->onAppendQText(QString::asprintf("   made %d triangles\n\n", int(m_pSail->refTriangles().size())));
    }
    else
    {
        m_pSail->clearRefTriangles();
        m_pSail->clearTEIndexes();
        m_pSail->updateStations();
    }

    m_pSail->makeTriPanels(Vector3d());
    QApplication::restoreOverrideCursor();

    onConnectPanels();

    m_pglSailView->resetgl3dMesh();
    m_pglSailView->update();
    m_bChanged = true;
}


void ThinSailDlg::onUpdateMesh()
{
    readMeshData();
    if(m_pSail->bRuledMesh()) onRuledMesh();

    updateTriMesh();
    m_pglSailView->update();
    m_bChanged = true;
}

