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




#include <QDebug>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>

#include "gl3daxesview.h"

#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <api/objects_global.h>
#include <test/test3d/gl3daxesview.h>


Quaternion gl3dAxesView::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);
QByteArray gl3dAxesView::s_Geometry;

LineStyle gl3dAxesView::s_WindVecsStyle{true, Line::SOLID, 2, fl5Color(100,100,100),  Line::NOSYMBOL, "Wind vectors"};

LineStyle gl3dAxesView::s_StabStyle = {true, Line::SOLID, 3, fl5Color(100,155,225),   Line::NOSYMBOL, "Stability axes"};
LineStyle gl3dAxesView::s_BodyStyle = {true, Line::DASH,  3, fl5Color(225,100,155),   Line::NOSYMBOL, "Body axes"};
LineStyle gl3dAxesView::s_WindStyle = {true, Line::DOT,   3, fl5Color(155,225,100),   Line::NOSYMBOL, "Wind axes"};


double gl3dAxesView::s_Alpha = 0.0;
double gl3dAxesView::s_Beta  = 0.0;

Vector3d gl3dAxesView::s_Vec;


gl3dAxesView::gl3dAxesView(QWidget *pParent) : gl3dXflView(pParent)
{
    setWindowTitle(tr("Axes test"));
    setupLayout();
    connectSignals();

    m_RefLength = 5.0;

    onUpdateAxes();
}


void gl3dAxesView::setupLayout()
{
    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);
        QVBoxLayout *pFrLayout = new QVBoxLayout;
        {
            QGridLayout *pDataLayout = new QGridLayout;
            {
                QLabel *plabAlpha = new QLabel("<p>&alpha;=</p>");
                m_pdeAlpha = new FloatEdit(s_Alpha);
                QLabel *plabBeta = new QLabel("<p>&beta;=</p>");
                m_pdeBeta = new FloatEdit(s_Beta);
                QLabel *plabDeg0 = new QLabel("<p>&deg;</p>");
                QLabel *plabDeg1 = new QLabel("<p>&deg;</p>");

                m_pchWindVec = new QCheckBox(tr("Wind vectors"));
                m_pchWindVec->setChecked(true);
                m_plbWindVecs = new LineBtn(s_WindVecsStyle);

                pDataLayout->addWidget(plabAlpha,     1,1, Qt::AlignRight);
                pDataLayout->addWidget(m_pdeAlpha,    1,2);
                pDataLayout->addWidget(plabDeg0,      1,3);
                pDataLayout->addWidget(plabBeta,      2,1, Qt::AlignRight);
                pDataLayout->addWidget(m_pdeBeta,     2,2);
                pDataLayout->addWidget(plabDeg1,      2,3);
                pDataLayout->addWidget(m_pchWindVec,  3,1);
                pDataLayout->addWidget(m_plbWindVecs, 3,2);
                pDataLayout->setColumnStretch(3,1);
            }

            QGroupBox *pDisplayBox = new QGroupBox(tr("Display"));
            {
                QGridLayout *pDisplayLayout = new QGridLayout;
                {
                    m_pchGeomAxes      = new QCheckBox(tr("Geometric axes"));
                    m_pchGeomAxes->setChecked(m_bAxes);
                    m_pchWindAxes      = new QCheckBox(tr("Wind axes"));
                    m_pchBodyAxes      = new QCheckBox(tr("Body axes"));
                    m_pchStabilityAxes = new QCheckBox(tr("Stability axes"));
                    m_pchWindAxes->setChecked(true);
                    m_pchStabilityAxes->setChecked(true);
                    m_plbWind = new LineBtn(s_WindStyle);
                    m_plbStab = new LineBtn(s_StabStyle);
                    m_plbBody = new LineBtn(s_BodyStyle);
                    pDisplayLayout->addWidget(m_pchGeomAxes,      1, 1);
                    pDisplayLayout->addWidget(m_pchBodyAxes,      2, 1);
                    pDisplayLayout->addWidget(m_plbBody,          2, 2);
                    pDisplayLayout->addWidget(m_pchWindAxes,      3, 1);
                    pDisplayLayout->addWidget(m_plbWind,          3, 2);
                    pDisplayLayout->addWidget(m_pchStabilityAxes, 4, 1);
                    pDisplayLayout->addWidget(m_plbStab,          4, 2);
                }
                pDisplayBox->setLayout(pDisplayLayout);
            }

            QGroupBox *pVectorBox = new QGroupBox(tr("Vector"));
            {
                QGridLayout * pVectorLayout = new QGridLayout;
                {
                    m_pchVector = new QCheckBox(tr("Show"));
                    m_pchVector->setChecked(false);

                    QLabel *plabX = new QLabel(tr("x="));
                    QLabel *plabY = new QLabel(tr("y="));
                    QLabel *plabZ = new QLabel(tr("z="));
                    m_pdeX = new FloatEdit(s_Vec.x);
                    m_pdeY = new FloatEdit(s_Vec.y);
                    m_pdeZ = new FloatEdit(s_Vec.z);

                    m_ppbConvert = new QPushButton(tr("Convert vector"));

                    pVectorLayout->addWidget(m_pchVector, 1,1,1,2);

                    pVectorLayout->addWidget(plabX,  2,1);
                    pVectorLayout->addWidget(m_pdeX, 2,2);
                    pVectorLayout->addWidget(plabY,  3,1);
                    pVectorLayout->addWidget(m_pdeY, 3,2);
                    pVectorLayout->addWidget(plabZ,  4,1);
                    pVectorLayout->addWidget(m_pdeZ, 4,2);
                    pVectorLayout->addWidget(m_ppbConvert, 5,1,1,3);
                    pVectorLayout->setColumnStretch(3,1);

                }
                pVectorBox->setLayout(pVectorLayout);
            }


            m_ppto = new PlainTextOutput;
            m_ppto->setMinimumSize(QSize(450,700));

            pFrLayout->addLayout(pDataLayout);
            pFrLayout->addWidget(pDisplayBox);
            pFrLayout->addWidget(pVectorBox);
            pFrLayout->addWidget(m_ppto);
        }
        pFrame->setLayout(pFrLayout);
    }

}

void gl3dAxesView::connectSignals()
{
    connect(m_pdeAlpha,         SIGNAL(floatChanged(float)), SLOT(onUpdateAxes()));
    connect(m_pdeBeta,          SIGNAL(floatChanged(float)), SLOT(onUpdateAxes()));

    connect(m_pdeX,             SIGNAL(floatChanged(float)), SLOT(onConvert()));
    connect(m_pdeY,             SIGNAL(floatChanged(float)), SLOT(onConvert()));
    connect(m_pdeZ,             SIGNAL(floatChanged(float)), SLOT(onConvert()));

    connect(m_pchWindVec,       SIGNAL(clicked(bool)),  SLOT(update()));

    connect(m_pchVector,        SIGNAL(clicked(bool)),  SLOT(update()));

    connect(m_pchBodyAxes,      SIGNAL(clicked(bool)),  SLOT(onUpdateAxes()));
    connect(m_pchStabilityAxes, SIGNAL(clicked(bool)),  SLOT(onUpdateAxes()));
    connect(m_pchWindAxes,      SIGNAL(clicked(bool)),  SLOT(onUpdateAxes()));
    connect(m_ppbConvert,       SIGNAL(clicked(bool)),  SLOT(onConvert()));

    connect(m_plbWindVecs,      SIGNAL(clickedLB(LineStyle)),  SLOT(onWindVecsStyle()));
    connect(m_plbBody,          SIGNAL(clickedLB(LineStyle)),  SLOT(onBodyLineStyle()));
    connect(m_plbWind,          SIGNAL(clickedLB(LineStyle)),  SLOT(onWindLineStyle()));
    connect(m_plbStab,          SIGNAL(clickedLB(LineStyle)),  SLOT(onStabLineStyle()));

    connect(m_pchGeomAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));
}


void gl3dAxesView::onWindVecsStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_WindVecsStyle);
    lm.exec(QCursor::pos());

    s_WindVecsStyle = lm.theStyle();
    m_plbWindVecs->setTheStyle(s_WindVecsStyle);
}


void gl3dAxesView::onBodyLineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_BodyStyle);
    lm.exec(QCursor::pos());

    s_BodyStyle = lm.theStyle();
    m_plbBody->setTheStyle(s_BodyStyle);
}


void gl3dAxesView::onWindLineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_WindStyle);
    lm.exec(QCursor::pos());

    s_WindStyle = lm.theStyle();
    m_plbWind->setTheStyle(s_WindStyle);
}


void gl3dAxesView::onStabLineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_StabStyle);
    lm.exec(QCursor::pos());

    s_StabStyle = lm.theStyle();
    m_plbStab->setTheStyle(s_StabStyle);
}


void gl3dAxesView::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAxesView");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_Alpha = settings.value("alpha",  s_Alpha).toDouble();
        s_Beta  = settings.value("beta",   s_Beta).toDouble();

        s_Vec.x     = settings.value("X",  s_Vec.x).toDouble();
        s_Vec.y     = settings.value("Y",  s_Vec.y).toDouble();
        s_Vec.z     = settings.value("Z",  s_Vec.z).toDouble();

        xfl::loadLineSettings(settings, s_WindVecsStyle, "WindVecsStyle");
        xfl::loadLineSettings(settings, s_BodyStyle, "BodyStyle");
        xfl::loadLineSettings(settings, s_WindStyle, "WindStyle");
        xfl::loadLineSettings(settings, s_StabStyle, "StabStyle");
    }
    settings.endGroup();
}


void gl3dAxesView::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAxesView");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("alpha", s_Alpha);
        settings.setValue("beta",  s_Beta);

        settings.setValue("X", s_Vec.x);
        settings.setValue("Y", s_Vec.y);
        settings.setValue("Z", s_Vec.z);


        xfl::saveLineSettings(settings, s_WindVecsStyle, "WindVecsStyle");
        xfl::saveLineSettings(settings, s_BodyStyle, "BodyStyle");
        xfl::saveLineSettings(settings, s_WindStyle, "WindStyle");
        xfl::saveLineSettings(settings, s_StabStyle, "StabStyle");
    }
    settings.endGroup();
}


void gl3dAxesView::showEvent(QShowEvent *pEvent)
{
    if(W3dPrefs::s_bSaveViewPoints) restoreViewPoint(s_ab_quat);
    restoreGeometry(s_Geometry);
    reset3dScale();
    pEvent->ignore();
}


void gl3dAxesView::hideEvent(QHideEvent *)
{
        if(W3dPrefs::s_bSaveViewPoints) saveViewPoint(s_ab_quat);
        s_Geometry = saveGeometry();
}


void gl3dAxesView::glRenderView()
{
    Vector3d origin;

    if(m_pchWindVec->isChecked())
    {
        paintThinArrow(origin, m_WindDir,    s_WindVecsStyle);
        paintThinArrow(origin, m_WindNormal, s_WindVecsStyle);
        paintThinArrow(origin, m_WindSide,   s_WindVecsStyle);

        glRenderText(m_WindDir,    "wind_dir",    s_WindVecsStyle.m_Color);
        glRenderText(m_WindNormal+Vector3d(0,0,-0.05), "wind_normal", s_WindVecsStyle.m_Color);
        glRenderText(m_WindSide+Vector3d(0,0,+0.05),   "wind_side",   s_WindVecsStyle.m_Color);
    }

    if(m_pchVector->isChecked())   paintThinArrow(origin, s_Vec, Qt::red, 2, Line::SOLID);

    if(m_pchBodyAxes->isChecked())
    {
        paintThinArrow(origin, m_CFBody.Idir(), s_BodyStyle);
        paintThinArrow(origin, m_CFBody.Jdir(), s_BodyStyle);
        paintThinArrow(origin, m_CFBody.Kdir(), s_BodyStyle);

        glRenderText(m_CFBody.Idir(), "x_body", s_BodyStyle.m_Color);
        glRenderText(m_CFBody.Jdir(), "y_body", s_BodyStyle.m_Color);
        glRenderText(m_CFBody.Kdir(), "z_body", s_BodyStyle.m_Color);
    }

    if(m_pchWindAxes->isChecked())
    {
        CartesianFrame const &m_CFWind = m_AF.CFWind();
        paintThinArrow(origin, m_CFWind.Idir(), s_WindStyle);
        paintThinArrow(origin, m_CFWind.Jdir(), s_WindStyle);
        paintThinArrow(origin, m_CFWind.Kdir(), s_WindStyle);

        glRenderText(m_CFWind.Idir(), "x_wind", s_WindStyle.m_Color);
        glRenderText(m_CFWind.Jdir(), "y_wind", s_WindStyle.m_Color);
        glRenderText(m_CFWind.Kdir()+Vector3d(0,0,+0.05), "z_wind", s_WindStyle.m_Color);
    }

    if(m_pchStabilityAxes->isChecked())
    {
        CartesianFrame const &m_CFStab = m_AF.CFStab();
        paintThinArrow(origin, m_CFStab.Idir(), s_StabStyle);
        paintThinArrow(origin, m_CFStab.Jdir(), s_StabStyle);
        paintThinArrow(origin, m_CFStab.Kdir(), s_StabStyle);

        glRenderText(m_CFStab.Idir(), "x_stab", s_StabStyle.m_Color);
        glRenderText(m_CFStab.Jdir()+Vector3d(0,0,-0.05), "y_stab", s_StabStyle.m_Color);
        glRenderText(m_CFStab.Kdir(), "z_stab", s_StabStyle.m_Color);
    }
}


void gl3dAxesView::glMake3dObjects()
{
}


void gl3dAxesView::onUpdateAxes()
{
    readData();

    m_WindDir    = objects::windDirection(s_Alpha, s_Beta);
    m_WindSide   = objects::windSide(s_Alpha, s_Beta);
    m_WindNormal = objects::windNormal(s_Alpha, s_Beta);

    m_CFBody.setFrame(Vector3d(), {-1, 0, 0},  {0,1,0}, { 0, 0, -1});

    m_AF.setOpp(s_Alpha, s_Beta, 0.0, 1.0);

/*    m_ppto->onAppendQText(QString::asprintf("\n Body RH rule: %d\n",   m_CFBody.checkRHRule()));
    m_ppto->onAppendQText(QString::asprintf(  " Wind RH rule: %d\n",   m_AF.CFWind().checkRHRule()));
    m_ppto->onAppendQText(QString::asprintf(  " Stab RH rule: %d\n\n", m_AF.CFStab().checkRHRule()));
*/
    update();
}


void gl3dAxesView::readData()
{
    s_Alpha = m_pdeAlpha->value();
    s_Beta  = m_pdeBeta->value();
    s_Vec.x = m_pdeX->value();
    s_Vec.y = m_pdeY->value();
    s_Vec.z = m_pdeZ->value();
}


void gl3dAxesView::onConvert()
{
    update();

    if(!m_pchVector->isChecked()) return;

    readData();

    QString strange;

    Vector3d VBody = m_CFBody.globalToLocal(s_Vec);
    Vector3d VWind = m_AF.CFWind().globalToLocal(s_Vec);
    Vector3d VStab = m_AF.CFStab().globalToLocal(s_Vec);

    strange = " to   Body     Wind     Stab\n";
    strange += QString::asprintf("x= %7g  %7g  %7g\n", VBody.x, VWind.x, VStab.x);
    strange += QString::asprintf("y= %7g  %7g  %7g\n", VBody.y, VWind.y, VStab.y);
    strange += QString::asprintf("z= %7g  %7g  %7g\n", VBody.z, VWind.z, VStab.z);
    m_ppto->onAppendQText(strange);
    // and back
    VBody = m_CFBody.localToGlobal(VBody);
    VWind = m_AF.CFWind().localToGlobal(VWind);
    VStab = m_AF.CFStab().localToGlobal(VStab);
    strange  = "from  Body     Wind     Stab\n";
    strange += QString::asprintf("x= %7g  %7g  %7g\n", VBody.x, VWind.x, VStab.x);
    strange += QString::asprintf("y= %7g  %7g  %7g\n", VBody.y, VWind.y, VStab.y);
    strange += QString::asprintf("z= %7g  %7g  %7g\n", VBody.z, VWind.z, VStab.z);
    m_ppto->onAppendQText(strange +"\n\n");

/*    m_ppto->onAppendQText("GeomToWindAxes:\n");
    Vector3d Vg = windToGeomAxes(s_Vec, s_Alpha, s_Beta);
    strange = QString::asprintf("   x=%7g\n   y=%7g\n   z=%7g\n\n", Vg.x, Vg.y, Vg.z);
    m_ppto->onAppendQText(strange);*/
}

