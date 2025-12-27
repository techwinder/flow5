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


#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSettings>

#include "streamlinesctrls.h"

#include <globals/mainframe.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/widgets/customwts/exponentialslider.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xsail/view/gl3dxsailview.h>

#include <api/units.h>

StreamLineCtrls::eStreamStart StreamLineCtrls::s_pos = StreamLineCtrls::TRAILINGEDGE;
int StreamLineCtrls::s_NX = 50;
double StreamLineCtrls::s_L0 = 0.01;
double StreamLineCtrls::s_XFactor = 1.05;
double StreamLineCtrls::s_XOffset = 0.0;
double StreamLineCtrls::s_YOffset = 0.0;
double StreamLineCtrls::s_ZOffset = 0.0;
int    StreamLineCtrls::s_NStreamLines = 11;
double StreamLineCtrls::s_DeltaL=0.05;



StreamLineCtrls::StreamLineCtrls(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();
}

void StreamLineCtrls::setupLayout()
{

    QVBoxLayout *pStreamlineLayout = new QVBoxLayout;
    {
        QFrame *pfrStatic = new QFrame;
        {
            QGridLayout *pLengthLayout = new QGridLayout;
            {
                QString tip = "<p>Adjust the total number of segments, the initial segment length "
                              "and the geometric progression factor to get the desired streamline length. "
                              "Note that if necessary, the 1st segment's length will be adjusted so that "
                              "the streamline's length does not exceed the polar's total wake length.</p>";
                pfrStatic->setAutoFillBackground(true);
                pfrStatic->setToolTip(tip);
                m_pieNXPoint = new IntEdit(s_NX);
                m_pieNXPoint->setToolTip(tip);
                m_pfeL0 = new FloatEdit(s_L0* Units::mtoUnit());
                m_pfeL0->setToolTip(tip);
                m_pfeXFactor = new FloatEdit(s_XFactor);
                m_pfeXFactor->setToolTip(tip);
                m_pfeMaxLength = new FloatEdit;
                m_pfeMaxLength->setToolTip(tip);
                m_pfeMaxLength->setEnabled(false);

                QLabel *pLab4 = new QLabel("Streamwise segments:");
                QLabel *pLab5 = new QLabel("1<sup>st</sup> segment:");
                QLabel *pLab6 = new QLabel("Progression factor:");
                QLabel *pLab7 = new QLabel("Total length:");

                m_plabLengthUnit0 = new QLabel("parsecs");
                m_plabLengthUnit1 = new QLabel("miles");

                pLengthLayout->addWidget(pLab4,             2,1, Qt::AlignVCenter |Qt::AlignRight);
                pLengthLayout->addWidget(m_pieNXPoint,      2,2);
                pLengthLayout->addWidget(pLab5,             3,1, Qt::AlignVCenter |Qt::AlignRight);
                pLengthLayout->addWidget(m_pfeL0,           3,2);
                pLengthLayout->addWidget(m_plabLengthUnit0, 3,3);
                pLengthLayout->addWidget(pLab6,             4,1, Qt::AlignVCenter |Qt::AlignRight);
                pLengthLayout->addWidget(m_pfeXFactor,      4,2);
                pLengthLayout->addWidget(pLab7,             5,1, Qt::AlignVCenter |Qt::AlignRight);
                pLengthLayout->addWidget(m_pfeMaxLength,    5,2);
                pLengthLayout->addWidget(m_plabLengthUnit1, 5,3);

                pLengthLayout->setColumnStretch(1,1);
                pLengthLayout->setColumnStretch(3,1);
            }
            pfrStatic->setLayout(pLengthLayout);
        }

        QHBoxLayout *pStreamStyleLayout = new QHBoxLayout;
        {
            QLabel *pLabStream = new QLabel("Style");
            m_plbStreamLines   = new LineBtn(this);
            m_pchUseWingColour = new QCheckBox("Use wing or sail colour");

            pStreamStyleLayout->addWidget(pLabStream);
            pStreamStyleLayout->addWidget(m_plbStreamLines);
            pStreamStyleLayout->addWidget(m_pchUseWingColour);
            pStreamStyleLayout->addStretch();
        }

        QGroupBox *pStartBox = new QGroupBox("Start streamlines at");
        {
            QVBoxLayout *pStartLayout = new QVBoxLayout;
            {
                m_plabLengthUnit2 = new QLabel("km");
                m_plabLengthUnit3 = new QLabel("m");
                m_plabLengthUnit4 = new QLabel("in");
                m_plabLengthUnit5 = new QLabel("mm");

                QHBoxLayout *pLineLayout = new QHBoxLayout;
                {
                    m_prbTE    = new QRadioButton("T.E.");
                    m_prbYLine = new QRadioButton("Y-Line");
                    m_prbZLine = new QRadioButton("Z-Line");
                    pLineLayout->addStretch();
                    pLineLayout->addWidget(m_prbTE);
                    pLineLayout->addWidget(m_prbYLine);
                    pLineLayout->addWidget(m_prbZLine);
                    pLineLayout->addStretch();
                }
                QGridLayout *pOffsetLayout = new QGridLayout;
                {
                    QLabel *pLabXOffset = new QLabel("<p>&Delta;X=</p>");
                    QLabel *pLabYOffset = new QLabel("<p>&Delta;Y=</p>");
                    QLabel *pLabZOffset = new QLabel("<p>&Delta;Z=</p>");

                    QString tip5 = "<p>Defines the offset from the trailing edge of the streamline's starting point. "
                                      "Numerical issues occur if the offset is zero in both X and Z directions "
                                      "due to the singularity of the velocity field at the panel edges.</p>";
                    m_pslXOffset = new QSlider(Qt::Horizontal);
                    m_pslXOffset->setTickPosition(QSlider::TicksBelow);
                    m_pslXOffset->setRange(-100,100);
                    m_pslXOffset->setTickInterval(20);
                    m_pslXOffset->setValue(0);
                    m_pslXOffset->setSingleStep(1);
                    m_pslXOffset->setToolTip(tip5);

                    m_pslYOffset = new ExponentialSlider(true, 7.0, Qt::Horizontal);
                    m_pslYOffset->setTickPosition(QSlider::TicksBelow);
                    m_pslYOffset->setRange(-100,100);
                    m_pslYOffset->setTickInterval(20);
                    m_pslYOffset->setValue(0);
                    m_pslYOffset->setSingleStep(1);
                    m_pslYOffset->setToolTip(tip5);

                    m_pslZOffset = new QSlider(Qt::Horizontal);
                    m_pslZOffset->setTickPosition(QSlider::TicksBelow);
                    m_pslZOffset->setRange(-100,100);
                    m_pslZOffset->setTickInterval(20);
                    m_pslZOffset->setValue(0);
                    m_pslZOffset->setSingleStep(1);
                    m_pslZOffset->setToolTip(tip5);

                    m_pfeXOffset = new FloatEdit(s_XOffset);
                    m_pfeYOffset = new FloatEdit(s_YOffset);
                    m_pfeZOffset = new FloatEdit(s_ZOffset);

                    pOffsetLayout->addWidget(pLabXOffset,        1,1, Qt::AlignRight);
                    pOffsetLayout->addWidget(m_pslXOffset,       1,2);
                    pOffsetLayout->addWidget(m_pfeXOffset,       1,3);
                    pOffsetLayout->addWidget(m_plabLengthUnit2,  1,4);
                    pOffsetLayout->addWidget(pLabYOffset,        2,1, Qt::AlignRight);
                    pOffsetLayout->addWidget(m_pslYOffset,       2,2);
                    pOffsetLayout->addWidget(m_pfeYOffset,       2,3);
                    pOffsetLayout->addWidget(m_plabLengthUnit3,  2,4);
                    pOffsetLayout->addWidget(pLabZOffset,        3,1, Qt::AlignRight);
                    pOffsetLayout->addWidget(m_pslZOffset,       3,2);
                    pOffsetLayout->addWidget(m_pfeZOffset,       3,3);
                    pOffsetLayout->addWidget(m_plabLengthUnit4,  3,4);
                    pOffsetLayout->setColumnStretch(2,1);
                }

                QGridLayout *pLinesLayout = new QGridLayout;
                {
                    QLabel *plabNStreamLines = new QLabel("Nbr. of streamlines:");
                    QLabel *plabDeltaL       = new QLabel("Increment:");
                    plabNStreamLines->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
                    plabDeltaL->setAlignment(Qt::AlignVCenter |Qt::AlignRight);

                    m_pieNStreamLines = new IntEdit(s_NStreamLines);
                    m_pieNStreamLines->setToolTip("<p>Defines the number of strealines to be drawn along the Y or Z line</p>");
                    m_pfeDeltaPos     = new FloatEdit(s_DeltaL*Units::mtoUnit());
                    m_pfeDeltaPos->setToolTip("<p>Defines the distance between the starting points of two adjacent streamlines along the Y or Z directions</p>");

                    pLinesLayout->addWidget(plabNStreamLines,   4,1);
                    pLinesLayout->addWidget(m_pieNStreamLines,  4,2);
                    pLinesLayout->addWidget(plabDeltaL,         5,1);
                    pLinesLayout->addWidget(m_pfeDeltaPos,      5,2);
                    pLinesLayout->addWidget(m_plabLengthUnit5,  5,3);
                    pLinesLayout->setColumnStretch(3,1);
                }
                pStartLayout->addLayout(pLineLayout);
                pStartLayout->addLayout(pOffsetLayout);
                pStartLayout->addLayout(pLinesLayout);
            }
            pStartBox->setLayout(pStartLayout);
        }

        m_ppbUpdateStreamLines = new QPushButton("Apply");


        pStreamlineLayout->addWidget(pfrStatic);
        pStreamlineLayout->addWidget(pStartBox);
        pStreamlineLayout->addLayout(pStreamStyleLayout);
        pStreamlineLayout->addStretch();

        pStreamlineLayout->addWidget(m_ppbUpdateStreamLines);
        pStreamlineLayout->addStretch(1);
    }
    setLayout(pStreamlineLayout);

}


void StreamLineCtrls::connectSignals()
{
    //streamlines
    connect(m_ppbUpdateStreamLines, SIGNAL(clicked()),             SLOT(onCalcStreamlines()));

    connect(m_prbTE,                SIGNAL(clicked(bool)),         SLOT(onCalcStreamlines()));
    connect(m_prbYLine,             SIGNAL(clicked(bool)),         SLOT(onCalcStreamlines()));
    connect(m_prbZLine,             SIGNAL(clicked(bool)),         SLOT(onCalcStreamlines()));

    connect(m_pieNXPoint,           SIGNAL(intChanged(int)),       SLOT(onCalcStreamlines()));
    connect(m_pfeL0,                SIGNAL(floatChanged(float)),   SLOT(onCalcStreamlines()));
    connect(m_pfeXFactor,           SIGNAL(floatChanged(float)),   SLOT(onCalcStreamlines()));

    connect(m_pslXOffset,           SIGNAL(sliderReleased()),      SLOT(onSliderOffset()));
    connect(m_pslYOffset,           SIGNAL(sliderReleased()),      SLOT(onSliderOffset()));
    connect(m_pslZOffset,           SIGNAL(sliderReleased()),      SLOT(onSliderOffset()));
    connect(m_pfeXOffset,           SIGNAL(floatChanged(float)),   SLOT(onFloatOffset()));
    connect(m_pfeYOffset,           SIGNAL(floatChanged(float)),   SLOT(onFloatOffset()));
    connect(m_pfeZOffset,           SIGNAL(floatChanged(float)),   SLOT(onFloatOffset()));
    connect(m_pfeDeltaPos,          SIGNAL(floatChanged(float)),   SLOT(onCalcStreamlines()));
    connect(m_pieNStreamLines,      SIGNAL(intChanged(int)),       SLOT(onCalcStreamlines()));

    connect(m_pchUseWingColour,     SIGNAL(clicked(bool)),         SLOT(onCalcStreamlines()));
    connect(m_plbStreamLines,       SIGNAL(clickedLB(LineStyle)),  SLOT(onStreamStyle()));
}


void StreamLineCtrls::initWidget()
{
    updateUnits();

    m_pfeXFactor->setValue(s_XFactor);
    m_pieNXPoint->setValue(s_NX);

    m_prbTE->setChecked(   s_pos==TRAILINGEDGE);
    m_prbYLine->setChecked(s_pos==Y_LINE);
    m_prbZLine->setChecked(s_pos==Z_LINE);
    enableStreamControls();

    m_pslXOffset->setValue(s_XOffset);
    m_pslYOffset->setValue(s_YOffset);
    m_pslZOffset->setValue(s_ZOffset);
    m_plbStreamLines->setTheStyle(W3dPrefs::s_StreamStyle);
    m_pchUseWingColour->setChecked(W3dPrefs::s_bUseWingColour);

    m_pieNStreamLines->setValue(s_NStreamLines);
    m_pfeDeltaPos->setValue(s_DeltaL*Units::mtoUnit());
    m_pieNStreamLines->setEnabled(s_pos==Y_LINE || s_pos==Z_LINE);
    m_pfeDeltaPos->setEnabled(    s_pos==Y_LINE || s_pos==Z_LINE);
    onCalcStreamlines();
}


void StreamLineCtrls::updateUnits()
{
    QString str;

    str = Units::lengthUnitQLabel();
    m_plabLengthUnit0->setText(str);
    m_plabLengthUnit1->setText(str);
    m_plabLengthUnit2->setText(str);
    m_plabLengthUnit3->setText(str);
    m_plabLengthUnit4->setText(str);
    m_plabLengthUnit5->setText(str);

    m_pfeDeltaPos->setValue(s_DeltaL *Units::mtoUnit());
    m_pfeL0->setValue(s_L0* Units::mtoUnit());
    m_pfeXOffset->setValue(s_XOffset*Units::mtoUnit());
    m_pfeYOffset->setValue(s_YOffset*Units::mtoUnit());
    m_pfeZOffset->setValue(s_ZOffset*Units::mtoUnit());
    onCalcStreamlines();
}


void StreamLineCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("StreamLinesCtrl");
    {
        // streamlines
        switch(settings.value("Position", s_pos).toInt())
        {
            default:
            case 0: s_pos = TRAILINGEDGE; break;
            case 1: s_pos = Y_LINE; break;
            case 2: s_pos = Z_LINE; break;
        }

        s_NX           = settings.value("NX",           s_NX).toInt();
        s_L0           = settings.value("L0",           s_L0).toDouble();
        s_XFactor      = settings.value("XFactor",      s_XFactor).toDouble();

        s_NStreamLines = settings.value("NStreamLines", s_NStreamLines).toInt();
        s_DeltaL       = settings.value("DeltaPos",     s_DeltaL).toDouble();


    }
    settings.endGroup();
}


void StreamLineCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("StreamLinesCtrl");
    {
        // streamlines
        settings.setValue("Position",       s_pos);
        settings.setValue("NX",             s_NX);
        settings.setValue("L0",             s_L0);
        settings.setValue("XFactor",        s_XFactor);
        settings.setValue("NStreamLines",   s_NStreamLines);
        settings.setValue("DeltaPos",       s_DeltaL);

    }
    settings.endGroup();
}


void StreamLineCtrls::onStreamStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(W3dPrefs::s_StreamStyle);
    lm.exec(QCursor::pos());

    W3dPrefs::s_StreamStyle = lm.theStyle();
    m_plbStreamLines->setTheStyle(W3dPrefs::s_StreamStyle);
    emit update3dStreamlines();
}


void StreamLineCtrls::readStreamParams()
{
    if     (m_prbTE->isChecked())    s_pos=TRAILINGEDGE;
    else if(m_prbYLine->isChecked()) s_pos=Y_LINE;
    else if(m_prbZLine->isChecked()) s_pos=Z_LINE;

    s_XOffset = m_pslXOffset->value();
    s_YOffset = m_pslYOffset->value();
    s_ZOffset = m_pslZOffset->value();

    s_NX      = m_pieNXPoint->value();
    s_L0      = m_pfeL0->value()  / Units::mtoUnit();
    s_XFactor = m_pfeXFactor->value();

    s_NStreamLines = m_pieNStreamLines->value();
    s_DeltaL = m_pfeDeltaPos->value()/Units::mtoUnit();

    m_pieNStreamLines->setEnabled(s_pos==Y_LINE || s_pos==Z_LINE);
    m_pfeDeltaPos->setEnabled(    s_pos==Y_LINE || s_pos==Z_LINE);

    W3dPrefs::s_bUseWingColour = m_pchUseWingColour->isChecked();
}


void StreamLineCtrls::onSliderOffset()
{
    s_XOffset = m_pslXOffset->value();
    s_YOffset = m_pslYOffset->value();
    s_ZOffset = m_pslZOffset->value();

    double length = 1.0;
    if      (MainFrame::xflApp()==xfl::XPLANE) length = m_pgl3dXPlaneView->objectReferenceLength();
    else if (MainFrame::xflApp()==xfl::XSAIL)  length = m_pgl3dXSailView->objectReferenceLength();

    m_pfeXOffset->setValue(s_XOffset/100.0*length*Units::mtoUnit());
    m_pfeYOffset->setValue(s_YOffset/100.0*length*Units::mtoUnit());
    m_pfeZOffset->setValue(s_ZOffset/100.0*length*Units::mtoUnit());
    onCalcStreamlines();
}


void StreamLineCtrls::onFloatOffset()
{
    double length = 1.0;
     if      (MainFrame::xflApp()==xfl::XPLANE) length = m_pgl3dXPlaneView->objectReferenceLength();
     else if (MainFrame::xflApp()==xfl::XSAIL)  length = m_pgl3dXSailView->objectReferenceLength();

    s_XOffset = m_pfeXOffset->value()/length/Units::mtoUnit();
    s_YOffset = m_pfeYOffset->value()/length/Units::mtoUnit();
    s_ZOffset = m_pfeZOffset->value()/length/Units::mtoUnit();

    m_pslXOffset->setValue(s_XOffset*100.0);
    m_pslYOffset->setValue(s_YOffset*100.0);
    m_pslZOffset->setValue(s_ZOffset*100.0);
    onCalcStreamlines();
}


void StreamLineCtrls::onCalcStreamlines()
{
    readStreamParams();
    enableStreamControls();
    double series=0.0, r=1.0;
    for(int p=0; p<StreamLineCtrls::nX(); p++)
    {
        series +=r;
        r*=s_XFactor;
    }
    double l0 = m_pfeL0->value()/Units::mtoUnit();
    m_pfeMaxLength->setValue(series*l0*Units::mtoUnit()); //double conversion superfluous

    emit update3dStreamlines();
}


void StreamLineCtrls::enableStreamControls()
{
/*    if     (m_prbYLine->isChecked()) s_pos = Y_LINE;
    else if(m_prbZLine->isChecked()) s_pos = Z_LINE;
    else                             s_pos = AT_TE;*/
    m_pslXOffset->setEnabled(s_pos!=TRAILINGEDGE);
    m_pslYOffset->setEnabled(s_pos!=TRAILINGEDGE);
    m_pslZOffset->setEnabled(s_pos!=TRAILINGEDGE);
    m_pieNStreamLines->setEnabled(s_pos!=TRAILINGEDGE);
    m_pfeDeltaPos->setEnabled(s_pos!=TRAILINGEDGE);
    m_pchUseWingColour->setEnabled(s_pos==TRAILINGEDGE);
}


double StreamLineCtrls::initialLength()
{
    if(StreamLineCtrls::nX()==0 || fabs(s_XFactor)<PRECISION) return 1.0;
    double series=0.0, r=1.0;
    for(int p=0; p<StreamLineCtrls::nX(); p++)
    {
        series +=r;
        r*=s_XFactor;
    }
    return 1.0/series;
}
