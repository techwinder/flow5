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


#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>

#include "flatfaceconverterdlg.h"
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <api/mathelem.h>
#include <api/fuseflatfaces.h>
#include <api/fusenurbs.h>


int FlatFaceConverterDlg::s_Nx = 9;
int FlatFaceConverterDlg::s_Nh = 5;
double FlatFaceConverterDlg::s_BunchAmp = 0.0;
double FlatFaceConverterDlg::s_BunchDist = 0.0;

QByteArray FlatFaceConverterDlg::s_Geometry;
QByteArray FlatFaceConverterDlg::s_HSplitterSizes;


FlatFaceConverterDlg::FlatFaceConverterDlg(QWidget *pWidget) : QDialog(pWidget)
{
    m_pFlatFaceFuse = new FuseFlatFaces();

    setupLayout();
    connectSignals();
}


FlatFaceConverterDlg::~FlatFaceConverterDlg()
{
    delete m_pFlatFaceFuse;
}


void FlatFaceConverterDlg::connectSignals()
{
    connect(m_pieFlatFaceNx, SIGNAL(intChanged(int)), SLOT(onFlatFace()));
    connect(m_pieFlatFaceNh, SIGNAL(intChanged(int)), SLOT(onFlatFace()));
    connect(m_pslBunchAmp,   SIGNAL(sliderMoved(int)),  SLOT(onFlatFace()));
    connect(m_pslBunchDist,  SIGNAL(sliderMoved(int)),  SLOT(onFlatFace()));
}


void FlatFaceConverterDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter(Qt::Horizontal);
        {
            m_pHSplitter->setChildrenCollapsible(false);
            QFrame *pViewFrame = new QFrame;
            {
                QVBoxLayout *pVBoxLayout = new QVBoxLayout;
                {
                    m_pglFuseView = new gl3dFuseView;

                    m_pglControls  = new gl3dGeomControls(m_pglFuseView, FuseLayout, true);
                    m_pglControls->disableFoilCtrl();

                    pVBoxLayout->addWidget(m_pglFuseView);
                    pVBoxLayout->addWidget(m_pglControls);
                }
                pViewFrame->setLayout(pVBoxLayout);
            }

            QFrame *pRightSideFrame = new QFrame;
            {
                QVBoxLayout *pRightSideLayout = new QVBoxLayout;
                {

                    QGridLayout *pBunchParamsLayout = new QGridLayout;
                    {
                        QLabel *plabNoAmp = new QLabel("No bunching");
                        QLabel *plabAmp = new QLabel("Full bunching");
                        m_pslBunchAmp = new QSlider(Qt::Horizontal);
                        m_pslBunchAmp->setRange(0, 100);
                        m_pslBunchAmp->setTickInterval(5);

                        m_pslBunchAmp->setTickPosition(QSlider::TicksBelow);
                        QLabel *plabCenter = new QLabel("Middle");
                        QLabel *plabEndPoints = new QLabel("EndPoints");
                        m_pslBunchDist = new QSlider(Qt::Horizontal);
                        m_pslBunchDist->setRange(0, 100);
                        m_pslBunchDist->setTickInterval(5);

                        m_pslBunchDist->setTickPosition(QSlider::TicksBelow);
                        QString tip;
                        tip = "<p>Use the sliders to bunch the panels. "
                                 "The amplitude slider determines the intensity of the bunching. "
                                 "The distribution slider determines whether the bunching is "
                                 "is towards the middle point or the endpoints.</p>";
                        m_pslBunchAmp->setToolTip(tip);
                        m_pslBunchDist->setToolTip(tip);

                        pBunchParamsLayout->addWidget(plabNoAmp,       1,1);
                        pBunchParamsLayout->addWidget(plabAmp,         1,3);
                        pBunchParamsLayout->addWidget(m_pslBunchAmp,   2,1,1,3);
                        pBunchParamsLayout->addWidget(plabCenter,      3,1);
                        pBunchParamsLayout->addWidget(plabEndPoints,   3,3);
                        pBunchParamsLayout->addWidget(m_pslBunchDist,  4,1,1,3);
                    }

                    QFormLayout *pFFLayout  = new QFormLayout;
                    {
                        QLabel *plabNx = new QLabel("Nx=");
                        QLabel *plabNh = new QLabel("Nh=");
                        plabNh->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
                        plabNx->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
                        m_pieFlatFaceNx = new IntEdit;
                        QString tip = "<p>The number of panels to generate in the x-direction</p>";
                        m_pieFlatFaceNx->setToolTip(tip);
                        m_pieFlatFaceNh = new IntEdit;
                        tip = "<p>The number of panels to generate in the hoop-direction</p>";
                        m_pieFlatFaceNh->setToolTip(tip);
                        pFFLayout->addRow(plabNx, m_pieFlatFaceNx);
                        pFFLayout->addRow(plabNh,m_pieFlatFaceNh);
                    }

                    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
                    {
                        connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
                        connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
                    }
                    pRightSideLayout->addLayout(pBunchParamsLayout);
                    pRightSideLayout->addLayout(pFFLayout);
                    pRightSideLayout->addStretch();
                    pRightSideLayout->addWidget(m_pButtonBox);
                }
                pRightSideFrame->setLayout(pRightSideLayout);
            }

            m_pHSplitter->addWidget(pViewFrame);
            m_pHSplitter->addWidget(pRightSideFrame);
        }
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void FlatFaceConverterDlg::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;*/

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            QPushButton *pSaveBtn = m_pButtonBox->button(QDialogButtonBox::Save);
            if(pSaveBtn) pSaveBtn->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }

        default:
            QDialog::keyPressEvent(pEvent);
            break;
    }
}


void FlatFaceConverterDlg::initDialog(FuseXfl const *pFuseXfl)
{
    m_pFuseXfl = pFuseXfl;
    m_pFlatFaceFuse->duplicateFuseXfl(*m_pFuseXfl);

    m_pglFuseView->setFuse(m_pFlatFaceFuse);

    m_pieFlatFaceNx->setValue(s_Nx);
    m_pieFlatFaceNh->setValue(s_Nh);

    int vamp = int(s_BunchAmp*100.0);
    m_pslBunchAmp->setValue(vamp);
    int vdist = int((1.0-s_BunchDist)*100.0);
    m_pslBunchDist->setValue(vdist);

    onFlatFace();
}


void FlatFaceConverterDlg::onFlatFace()
{
    s_Nx = m_pieFlatFaceNx->value();
    s_Nh = m_pieFlatFaceNh->value();

    int val0 = m_pslBunchAmp->value();
    double amp = double(val0)/100.0; // k=0.0 --> uniform weight, k=1-->full varying weights;

    int val1 = m_pslBunchDist->value(); // k=0: weight on center ctrl points, k=1 weigth on end ctrl points
    double dist = double(100.0-val1)/100.0;

    std::vector<double> xPanelPos;
    for(int i=0; i<=s_Nx; i++)
    {
        double x = double(i)/double(s_Nx);
        xPanelPos.push_back(bunchedParameter(dist, amp, x));
    }

    m_pFlatFaceFuse->duplicateFuseXfl(*m_pFuseXfl);
    m_pFlatFaceFuse->toFlatType(xPanelPos, s_Nh);

    m_pglFuseView->resetFuse();
    m_pglFuseView->update();
}


void FlatFaceConverterDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
}


void FlatFaceConverterDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Nx = m_pieFlatFaceNx->value();
    s_Nh = m_pieFlatFaceNh->value();
    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void FlatFaceConverterDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FlatFaceConverterDlg");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSize").toByteArray();

        s_Nx = settings.value("FFNx", 9).toInt();
        s_Nh = settings.value("FFNh", 5).toInt();
        s_BunchAmp  = settings.value("BunchAmp", 0.0).toDouble();
        s_BunchDist = settings.value("BunchDist", 0.0).toDouble();
    }
    settings.endGroup();
}


void FlatFaceConverterDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FlatFaceConverterDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitterSize", s_HSplitterSizes);

        settings.setValue("FFNx", s_Nx);
        settings.setValue("FFNh", s_Nh);
        settings.setValue("BunchAmp", s_BunchAmp);
        settings.setValue("BunchDist", s_BunchDist);
    }
    settings.endGroup();
}
