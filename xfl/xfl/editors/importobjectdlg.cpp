/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED


#include <QVBoxLayout>

#include "importobjectdlg.h"

#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane/plane.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/objects3d/fuse/fuse.h>


ImportObjectDlg::ImportObjectDlg(QWidget *pParent, bool bWings) : XflDialog(pParent), m_bWings(bWings)
{
    setupLayout(bWings);

    for(int ip=0; ip<Objects3d::nPlanes(); ip++)
    {
        Plane const *pPlane = Objects3d::planeAt(ip);
        if(pPlane) m_plvPlanes->addItem(pPlane->name());
    }

    m_plvObjects->setCurrentRow(0, QItemSelectionModel::Select);

    fillObjects();

    connect(m_plvPlanes, SIGNAL(currentRowChanged(int)), SLOT(onPlaneChanged()));
}


void ImportObjectDlg::setupLayout(bool bWings)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QLabel *pLabPlanes  = new QLabel("Planes:");
        QLabel *pLabObjects = new QLabel(bWings ? "Wings:" : "Fuselages:");

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
    Plane const *pPlane = Objects3d::plane(pItem->text());
    if(!pPlane) return;

    if(m_bWings)
    {
        for(int iw=0; iw<pPlane->nWings(); iw++)
        {
            m_plvObjects->addItem(pPlane->wingAt(iw)->name());
        }
    }
    else
    {
        for(int iFuse=0; iFuse<pPlane->nFuse(); iFuse++)
        {
            m_plvObjects->addItem(pPlane->fuseAt(iFuse)->name());
        }
    }
}


