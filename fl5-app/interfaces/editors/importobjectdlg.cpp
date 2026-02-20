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


#define _MATH_DEFINES_DEFINED


#include <QVBoxLayout>

#include "importobjectdlg.h"

#include <api/objects3d.h>
#include <api/plane.h>
#include <api/wingxfl.h>
#include <api/wingxfl.h>
#include <api/fuse.h>


ImportObjectDlg::ImportObjectDlg(QWidget *pParent, bool bWings) : XflDialog(pParent), m_bWings(bWings)
{
    setupLayout(bWings);

    for(int ip=0; ip<Objects3d::nPlanes(); ip++)
    {
        Plane const *pPlane = Objects3d::planeAt(ip);
        if(pPlane) m_plvPlanes->addItem(QString::fromStdString(pPlane->name()));
    }

    m_plvObjects->setCurrentRow(0, QItemSelectionModel::Select);

    fillObjects();

    connect(m_plvPlanes, SIGNAL(currentRowChanged(int)), SLOT(onPlaneChanged()));
}


void ImportObjectDlg::setupLayout(bool bWings)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QLabel *pLabPlanes  = new QLabel(tr("Planes:"));
        QLabel *pLabObjects = new QLabel(bWings ? tr("Wings:") : tr("Fuselages:"));

        m_plvPlanes  = new QListWidget;
        m_plvObjects = new QListWidget;

        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);


        pMainLayout->addWidget(pLabPlanes);
        pMainLayout->addWidget(m_plvPlanes);
        pMainLayout->addWidget(pLabObjects);
        pMainLayout->addWidget(m_plvObjects);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


QString ImportObjectDlg::planeName()  const
{
    QListWidgetItem *pItem = m_plvPlanes->currentItem();
    if(!pItem) return QString();
    return pItem->text();
}


QString ImportObjectDlg::objectName() const
{
    QListWidgetItem *pItem = m_plvObjects->currentItem();
    if(!pItem) return QString();
    return pItem->text();
}


void ImportObjectDlg::onPlaneChanged()
{
    fillObjects();
}


void ImportObjectDlg::fillObjects()
{
    m_plvObjects->clear();

    QListWidgetItem *pItem = m_plvPlanes->currentItem();
    if(!pItem) return;
    Plane const *pPlane = Objects3d::plane(pItem->text().toStdString());
    if(!pPlane) return;

    if(m_bWings)
    {
        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            m_plvObjects->addItem(QString::fromStdString(pPlane->wingAt(iw)->name()));
        }
    }
    else
    {
        for(int iFuse=0; iFuse<pPlane->nFuse(); iFuse++)
        {
            m_plvObjects->addItem(QString::fromStdString(pPlane->fuseAt(iFuse)->name()));
        }
    }
}


