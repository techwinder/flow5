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

#include <QVBoxLayout>
#include <QMenu>

#include "sailstldlg.h"

#include <api/sailstl.h>
#include <api/units.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>



SailStlDlg::SailStlDlg(QWidget *pParent) : ExternalSailDlg(pParent)
{
    setWindowTitle("STL sail editor");

    setupLayout();
    connectSignals();

    m_pglSailControls->enableCtrlPts(false);
}


void SailStlDlg::initDialog(Sail *pSail)
{
    SailDlg::initDialog(pSail);

    SailStl *pSTLSail = dynamic_cast<SailStl*>(m_pSail);
    m_prbThin->setChecked(pSTLSail->isThinSurface());
    m_prbThick->setChecked(!pSTLSail->isThinSurface());

    m_pglSailView->setReferenceLength(2.0*pSTLSail->size());
    m_pfeTEAngle->setValue(s_TEMaxAngle);

    setControls();
    setSailData();
}


void SailStlDlg::setupLayout()
{
    QPushButton *ppbSailActions = new QPushButton("Sail actions");
    {
        QMenu *ppbSailMenu = new QMenu("Sail actions");
        {
            ppbSailMenu->addAction(m_pDefinitions);
            ppbSailMenu->addSeparator();
            ppbSailMenu->addAction(m_pTranslate);
            ppbSailMenu->addAction(m_pRotate);
            ppbSailMenu->addAction(m_pScaleShape);
            ppbSailMenu->addAction(m_pScaleSize);
            ppbSailMenu->addSeparator();
            ppbSailMenu->addAction(m_pFlipXZ);
            ppbSailMenu->addSeparator();
            ppbSailMenu->addAction(m_pExportTrianglesToSTL);
        }
        ppbSailActions->setMenu(ppbSailMenu);
        m_pButtonBox->addButton(ppbSailActions, QDialogButtonBox::ActionRole);
    }

    QFrame *pLeftFrame = new QFrame;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            m_pTabWidget = new QTabWidget(this);
            {
                connect(m_pTabWidget, SIGNAL(currentChanged(int)), SLOT(onTabChanged()));

                QFrame *pMetaFrame = new QFrame;
                {
                    QVBoxLayout *pMetaLayout = new QVBoxLayout;
                    {
                        pMetaLayout->addWidget(m_pfrMeta);
                        pMetaLayout->addWidget(m_pCornersBox);
                    }
                    pMetaFrame->setLayout(pMetaLayout);
                }

                m_pTabWidget->addTab(pMetaFrame, "Meta");
                m_pTabWidget->addTab(m_pfrMesh,  "Mesh");
                m_pTabWidget->addTab(m_pfrTE,    "Trailing edge");

                m_pTabWidget->setTabVisible(1, false);
            }
            pLeftLayout->addWidget(m_pTabWidget);
            pLeftLayout->addWidget(m_ppto);
            pLeftLayout->addWidget(m_pFlow5Link);
            pLeftLayout->addWidget(m_pButtonBox);
            pLeftLayout->setStretchFactor(m_pTabWidget, 1);
            pLeftLayout->setStretchFactor(m_ppto, 5);
            pLeftLayout->setStretchFactor(m_pButtonBox, 1);
        }
        pLeftFrame->setLayout(pLeftLayout);
    }

    m_pExternalSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pExternalSplitter->setChildrenCollapsible(false);
        m_pExternalSplitter->addWidget(pLeftFrame);
        m_pExternalSplitter->addWidget(m_pfr3dView);
        m_pExternalSplitter->setStretchFactor(0, 3);
        m_pExternalSplitter->setStretchFactor(1, 1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pExternalSplitter);
    }

    setLayout(pMainLayout);
}


void SailStlDlg::onTabChanged()
{
    deselectButtons();
}


void SailStlDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_1:
        {
            if(bCtrl)
            {
                m_pTabWidget->setCurrentIndex(0);
            }
            break;
        }
        case Qt::Key_2:
        {
            if(bCtrl)
            {
                m_pTabWidget->setCurrentIndex(1);
            }
            break;
        }
#ifdef QT_DEBUG
        case Qt::Key_F5:
        {
            SailStl *pExtSail = dynamic_cast<SailStl*>(m_pSail);

            Vector3d O, axis;
            axis.set(0.0,1.0,0.0);
            pExtSail->rotate(O, axis, -90);
            axis.set(0.0,0.0,1.0);
            pExtSail->rotate(O, axis, -90);

            pExtSail->translate({8,0,0});

            m_ppto->onAppendQText("Rotated 90/x + Translated 8\n");
            m_pglSailView->resetglSail();
            updateView();

            m_pfeRefArea->setValue(1);
            m_pfeRefChord->setValue(1);

            m_bChanged = true;

            break;
        }
#endif
        default: break;
    }
    ExternalSailDlg::keyPressEvent(pEvent);
}


