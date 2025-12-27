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


#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFontDialog>
#include <QColorDialog>

#include "graphoptions.h"
#include <core/xflcore.h>
#include <interfaces/graphs/globals/graphsvgwriter.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/color/textclrbtn.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/line/linecbbox.h>


bool GraphOptions::s_bMouseTracking = true;

bool GraphOptions::s_bSpinAnimation = true;
double GraphOptions::s_SpinDamping = 0.01;

bool GraphOptions::s_bIsGraphModified = false;

FontStruct GraphOptions::s_TitleFontStruct;
FontStruct GraphOptions::s_LabelFontStruct;
FontStruct GraphOptions::s_LegendFontStruct;

QColor GraphOptions::s_TitleColor=Qt::black;
QColor GraphOptions::s_LabelColor=Qt::black;
QColor GraphOptions::s_LegendColor=Qt::black;

QColor GraphOptions::s_BackColor=Qt::white;
Grid GraphOptions::s_Grid;

LineStyle GraphOptions::s_theBorderStyle;
int GraphOptions::s_Margin[4] = {0,0,0,0}; // left, right, top, bottom

Axis GraphOptions::s_XAxis;
Axis GraphOptions::s_YAxis[2]; /**< the two Y axes */


bool GraphOptions::s_bBorder=true; /**< if false, the right axis is disabled altogether for this graph; the default is false */
bool GraphOptions::s_bShowLegend=false;

Qt::Alignment GraphOptions::s_LegendPosition = Qt::AlignTop | Qt::AlignHCenter;

GraphOptions::GraphOptions(QWidget *parent) : QWidget(parent)
{
    makeWidgets();
    connectSignals();
}


void GraphOptions::resetGraphSettings(Graph &graph)
{
    graph.setBorder(s_bBorder);
    graph.setTheBorderStyle(s_theBorderStyle);
    graph.setBkColor(s_BackColor);

    graph.setAxisStyle(AXIS::XAXIS, s_XAxis.theStyle());
    graph.setAxisStyle(AXIS::LEFTYAXIS, s_YAxis[0].theStyle());
    graph.setAxisStyle(AXIS::RIGHTYAXIS, s_YAxis[1].theStyle());

    graph.setMargins(s_Margin);
//    graph.setLegendVisible(s_bShowLegend);
//    graph.setLegendPosition(s_LegendPosition);

    graph.setTitleColor(s_TitleColor);
    graph.setLabelColor(s_LabelColor);
    graph.setLegendColor(s_LegendColor);

    graph.setTitleFont(s_TitleFontStruct.font());
    graph.setLabelFont(s_LabelFontStruct.font());
    graph.setLegendFont(s_LegendFontStruct.font());

    graph.setGrid(s_Grid);
}


void GraphOptions::setDefaults(bool bDark)
{
    s_bBorder  = true;
    s_theBorderStyle.m_Stipple = Line::SOLID;
    s_theBorderStyle.m_Width = 2;

    s_Margin[0] = 51;
    s_Margin[1] = s_Margin[2] = s_Margin[3] = 43;

    s_bShowLegend   = false;
    s_LegendPosition = Qt::AlignTop | Qt::AlignHCenter;

    if(bDark)
    {
        s_BackColor = QColor(0,9,13);
        s_theBorderStyle.m_Color = fl5Color(200,200,200);

        QColor axiscolor(200,200,200);
        s_XAxis.setColor(axiscolor);
        s_YAxis[0].setColor(axiscolor);
        s_YAxis[1].setColor(axiscolor);

        s_TitleColor  = QColor(255,255,255);
        s_LabelColor  = QColor(255,255,255);
        s_LegendColor = QColor(255,255,255);

        s_Grid.setXMajColor(QColor(90,90,90));
        s_Grid.setXMinColor(QColor(50,50,50));

        s_Grid.setYMajColor(0, QColor(90,90,90));
        s_Grid.setYMinColor(0, QColor(50,50,50));
        s_Grid.setYMajColor(1, QColor(90,90,90));
        s_Grid.setYMinColor(1, QColor(50,50,50));
    }
    else
    {
        s_BackColor = QColor(255,255,255);
        s_theBorderStyle.m_Color = fl5Color(55,55,55);

        QColor axiscolor(55,55,55);
        s_XAxis.setColor(axiscolor);
        s_YAxis[0].setColor(axiscolor);
        s_YAxis[1].setColor(axiscolor);

        s_TitleColor  = QColor(0,0,0);
        s_LabelColor  = QColor(0,0,0);
        s_LegendColor = QColor(0,0,0);

        s_Grid.setXMajColor(QColor(165,165,165));
        s_Grid.setXMinColor(QColor(205,205,205));

        s_Grid.setYMajColor(0, QColor(165,165,165));
        s_Grid.setYMinColor(0, QColor(205,205,205));
        s_Grid.setYMajColor(1, QColor(165,165,165));
        s_Grid.setYMinColor(1, QColor(205,205,205));
    }

    s_theBorderStyle.m_Stipple = Line::SOLID;
    s_theBorderStyle.m_Width = 3;

    s_XAxis.setAxis(AXIS::XAXIS);
    s_XAxis.setStipple(Line::SOLID);
    s_XAxis.setWidth(1);

    s_YAxis[0].setAxis(AXIS::LEFTYAXIS);
    s_YAxis[0].setStipple(Line::SOLID);
    s_YAxis[0].setWidth(1);

    s_YAxis[1].setAxis(AXIS::RIGHTYAXIS);
    s_YAxis[1].setStipple(Line::SOLID);
    s_YAxis[1].setWidth(1);
}

void GraphOptions::connectSignals()
{
    connect(m_ppbTitles, SIGNAL(clicked()),  SLOT(onTitleFont()));
    connect(m_ppbLabels, SIGNAL(clicked()),  SLOT(onLabelFont()));
    connect(m_ppbLegend, SIGNAL(clicked()),  SLOT(onLegendFont()));

    connect(m_ptcbTitleClr,  SIGNAL(clickedTB()),  SLOT(onTitleColor()));
    connect(m_ptcbLabelClr,  SIGNAL(clickedTB()),  SLOT(onLabelColor()));
    connect(m_ptcbLegendClr, SIGNAL(clickedTB()),  SLOT(onLegendColor()));

    connect(m_pchXMajGridShow, SIGNAL(clicked(bool)), SLOT(onXMajGridShow(bool)));
    connect(m_pchXMinGridShow, SIGNAL(clicked(bool)), SLOT(onXMinGridShow(bool)));

    connect(m_plbXAxisStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onAxisStyle()));
    connect(m_plbXMajGridStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onXMajGridStyle()));
    connect(m_plbXMinGridStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onXMinGridStyle()));

    for(int iy=0; iy<2; iy++)
    {
        connect(m_plbYAxisStyle[iy],    SIGNAL(clickedLB(LineStyle)), SLOT(onAxisStyle()));
        connect(m_pchYMajGridShow[iy],    SIGNAL(clicked(bool)),        SLOT(onYMajGridShow(bool)));
        connect(m_pchYMinGridShow[iy],    SIGNAL(clicked(bool)),        SLOT(onYMinGridShow(bool)));
        connect(m_plbYMinGridStyle[iy], SIGNAL(clickedLB(LineStyle)), SLOT(onYMinGridStyle()));
        connect(m_plbYMajGridStyle[iy], SIGNAL(clickedLB(LineStyle)), SLOT(onYMajGridStyle()));
    }

    connect(m_pcbLine, SIGNAL(activated(int)), SLOT(onDefaultLineWidth(int)));

    connect(m_pchGraphBorder, SIGNAL(clicked(bool)), SLOT(onGraphBorder(bool)));
    connect(m_plbBorderStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onBorderStyle(LineStyle)));
    connect(m_pcobGraphBack,   SIGNAL(clicked()), SLOT(onGraphBackColor()));

    connect(m_pchAlignChilren, SIGNAL(toggled(bool)), SLOT(onAlignChildren()));
    connect(m_pieColorIncrement, SIGNAL(intChanged(int)), SLOT(onColorIncrement()));

    connect(m_pieLMargin, SIGNAL(intChanged(int)), SLOT(onMargin()));
    connect(m_pieRMargin, SIGNAL(intChanged(int)), SLOT(onMargin()));
    connect(m_pieTMargin, SIGNAL(intChanged(int)), SLOT(onMargin()));
    connect(m_pieBMargin, SIGNAL(intChanged(int)), SLOT(onMargin()));

    connect(m_pchSpinAnimation, SIGNAL(clicked(bool)), SLOT(onSpinAnimation(bool)));
}


void GraphOptions::makeWidgets()
{
    //________Font Page___________________________
    m_pGroupBox.push_back(new QGroupBox("Fonts"));
    {
        QGridLayout *pFontButtonsLayout = new QGridLayout;
        {
            QLabel *plab1    = new QLabel("Axis titles:");
            QLabel *plab2    = new QLabel("Axis labels:");
            QLabel *plab3    = new QLabel("In-graph legend:");
            QLabel *plab402  = new QLabel("Font");
            QLabel *plab403  = new QLabel("Colour");
            plab1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            plab2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            plab3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            plab402->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
            plab403->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
            pFontButtonsLayout->addWidget(plab402, 1,2);
            pFontButtonsLayout->addWidget(plab403, 1,3);
            pFontButtonsLayout->addWidget(plab1,   2,1);
            pFontButtonsLayout->addWidget(plab2,   3,1);
            pFontButtonsLayout->addWidget(plab3,   4,1);

            m_ppbTitles = new QPushButton();
            m_ppbLabels = new QPushButton();
            m_ppbLegend = new QPushButton();

            pFontButtonsLayout->addWidget(m_ppbTitles,2,2);
            pFontButtonsLayout->addWidget(m_ppbLabels,3,2);
            pFontButtonsLayout->addWidget(m_ppbLegend,4,2);

            m_ptcbTitleClr  = new TextClrBtn(this);
            m_ptcbTitleClr->setText("Title colour");
            m_ptcbLabelClr  = new TextClrBtn(this);
            m_ptcbLabelClr->setText("Label colour");
            m_ptcbLegendClr  = new TextClrBtn(this);
            m_ptcbLegendClr->setText("Legend colour");

            pFontButtonsLayout->addWidget(m_ptcbTitleClr,2,3);
            pFontButtonsLayout->addWidget(m_ptcbLabelClr,3,3);
            pFontButtonsLayout->addWidget(m_ptcbLegendClr,4,3);
        }
        m_pGroupBox.back()->setLayout(pFontButtonsLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Background"));
    {
        QGridLayout *pBackDataLayout = new QGridLayout;
        {
            QLabel *GraphBackLabel = new QLabel("Graph background");
            GraphBackLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_pchGraphBorder = new QCheckBox("Graph border");

            m_pcobGraphBack = new ColorBtn;
            m_plbBorderStyle = new LineBtn(this);

            pBackDataLayout->addWidget(GraphBackLabel,1,1);
            pBackDataLayout->addWidget(m_pchGraphBorder,2,1,1,1,Qt::AlignRight | Qt::AlignVCenter);

            pBackDataLayout->addWidget(m_pcobGraphBack,1,2);
            pBackDataLayout->addWidget(m_plbBorderStyle,2,2);

            pBackDataLayout->setColumnStretch(1,7);
            pBackDataLayout->setColumnStretch(2,3);
        }
        m_pGroupBox.back()->setLayout(pBackDataLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Padding"));
    {
        QGridLayout *pPaddingLayout = new QGridLayout;
        {
            QLabel *plabMarginUnit = new QLabel("pixels");
            m_pieLMargin = new IntEdit(31, this);
            m_pieRMargin = new IntEdit(31, this);
            m_pieTMargin = new IntEdit(31, this);
            m_pieBMargin = new IntEdit(31, this);

            pPaddingLayout->addWidget(m_pieLMargin, 2, 2);
            pPaddingLayout->addWidget(m_pieRMargin, 2, 4);
            pPaddingLayout->addWidget(m_pieTMargin, 1, 3);
            pPaddingLayout->addWidget(m_pieBMargin, 3, 3);
            pPaddingLayout->addWidget(plabMarginUnit,2,5);
        }
        m_pGroupBox.back()->setLayout(pPaddingLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Axes and grids"));
    {
        QVBoxLayout *pAxisDataLayout = new QVBoxLayout;
        {
            QGroupBox *pgbXBox = new QGroupBox("X");
            {
                QGridLayout *pXBoxLayout = new QGridLayout;
                {
                    QLabel *pAxisStyleLabel = new QLabel("Axis Style");
                    m_plbXAxisStyle = new LineBtn(this);

                    m_pchXMajGridShow = new QCheckBox("X Major Grid");
                    m_pchXMinGridShow = new QCheckBox("X Minor Grid");

                    m_plbXMajGridStyle = new LineBtn(this);
                    m_plbXMinGridStyle = new LineBtn(this);

                    pXBoxLayout->addWidget(pAxisStyleLabel,1,1,Qt::AlignRight);
                    pXBoxLayout->addWidget(m_plbXAxisStyle,1,2);
                    pXBoxLayout->addWidget(m_pchXMajGridShow,2,1,Qt::AlignRight);
                    pXBoxLayout->addWidget(m_pchXMinGridShow,3,1,Qt::AlignRight);
                    pXBoxLayout->addWidget(m_plbXMajGridStyle,2,2);
                    pXBoxLayout->addWidget(m_plbXMinGridStyle,3,2);
                }
                pgbXBox->setLayout(pXBoxLayout);
            }
            QGroupBox *pgbYBox[]{nullptr, nullptr};
            for(int iy=0; iy<2; iy++)
            {
                QString name = iy==0 ? "Y-left" : "Y-right";
                pgbYBox[iy] = new QGroupBox(name);
                {
                    QGridLayout *pYBoxLayout = new QGridLayout;
                    {
                        QLabel *pAxisStyleLabel = new QLabel("Axes style");
                        m_plbYAxisStyle[iy] = new LineBtn(this);

                        m_pchYMajGridShow[iy] = new QCheckBox("Y major grid - left");
                        m_pchYMinGridShow[iy] = new QCheckBox("Y minor grid - left");
                        m_plbYMajGridStyle[iy] = new LineBtn(this);
                        m_plbYMinGridStyle[iy] = new LineBtn(this);
                        pYBoxLayout->addWidget(pAxisStyleLabel,1,1,Qt::AlignRight);
                        pYBoxLayout->addWidget(m_plbYAxisStyle[iy],1,2);
                        pYBoxLayout->addWidget(m_pchYMajGridShow[iy],4,1,Qt::AlignRight);
                        pYBoxLayout->addWidget(m_pchYMinGridShow[iy],5,1,Qt::AlignRight);
                        pYBoxLayout->addWidget(m_plbYMajGridStyle[iy],4,2);
                        pYBoxLayout->addWidget(m_plbYMinGridStyle[iy],5,2);
                    }
                    pgbYBox[iy]->setLayout(pYBoxLayout);
                }
            }


            pAxisDataLayout->addWidget(pgbXBox);
            pAxisDataLayout->addWidget(pgbYBox[0]);
            pAxisDataLayout->addWidget(pgbYBox[1]);
            pAxisDataLayout->addStretch();
        }
        m_pGroupBox.back()->setLayout(pAxisDataLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Curves"));
    {
        QGridLayout *pDefaultLineLayout = new QGridLayout;
        {
            QLabel *plabLine = new QLabel("Default curve width:");
            plabLine->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_pcbLine = new LineCbBox;

            m_pchAlignChilren = new QCheckBox("Make children curve's style same as parent's style");

            QLabel *plabIncrement = new QLabel("Color darker factor:");
            QString tip("<p>Set a value between 100 and 300 to increase the darkness of each polar "
                        "or operating point in the sequence of their creation.</p>");
            m_pieColorIncrement = new IntEdit;
            m_pieColorIncrement->setToolTip(tip);


            pDefaultLineLayout->addWidget(plabLine,            1,1);
            pDefaultLineLayout->addWidget(m_pcbLine,           1,2);
            pDefaultLineLayout->addWidget(m_pchAlignChilren,   2,1,1,2);
            pDefaultLineLayout->addWidget(plabIncrement,       3,1);
            pDefaultLineLayout->addWidget(m_pieColorIncrement, 3,2);
            pDefaultLineLayout->setColumnStretch(3,1);
        }
        m_pGroupBox.back()->setLayout(pDefaultLineLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Other"));
    {
        QVBoxLayout *pOtherLayout = new QVBoxLayout;
        {
            QHBoxLayout *pDynamicLayout = new QHBoxLayout;
            {
                m_pchSpinAnimation = new QCheckBox("Enable mouse animations");
                m_pfeSpinDamping = new FloatEdit;
                m_pfeSpinDamping->setToolTip("<p>Defines the damping of the animation at each frame update.<br>"
                                             "Set to 0 for perpetual movement.</p>");
                QLabel *plabpcDamping = new QLabel("% damping");
                pDynamicLayout->addWidget(m_pchSpinAnimation);
                pDynamicLayout->addWidget(m_pfeSpinDamping);
                pDynamicLayout->addWidget(plabpcDamping);
                pDynamicLayout->addStretch();
            }

            m_pchMouseTracking = new QCheckBox("Graphs steal focus on mouse move");
            m_pchShowMousePos = new QCheckBox("Show mouse coordinates");
            m_pchShowMousePos->setToolTip("<p>Display the coordinates of the mouse on the top right corner of the graph</p>");
            m_pchAntiAliasing  = new QCheckBox("Enable anti-aliasing");

            QGridLayout *pGraphOtherLayout = new QGridLayout;
            {


                QLabel *plabSVGFont = new QLabel("SVG font export: 1em=");
                m_pieSVGRefFontSize = new IntEdit;
                m_pieSVGRefFontSize->setToolTip("<p>This value defines the reference font-size used when exporting to SVG.<br>"
                                                "Reduce this value to increase the size of fonts of the SVG image, and increase "
                                                "it to obtain smaller fonts.<br>"
                                                "Recommendation: size=10 to 12</p>");
                QLabel *plabPoints = new QLabel("points");


                pGraphOtherLayout->addWidget(plabSVGFont,         2, 1);
                pGraphOtherLayout->addWidget(m_pieSVGRefFontSize, 2, 2);
                pGraphOtherLayout->addWidget(plabPoints,          2, 3);
                pGraphOtherLayout->setColumnStretch(4,1);
            }

            m_pchSVGFillBackground = new QCheckBox("Fill background when exporting to SVG");

            pOtherLayout->addLayout(pDynamicLayout);
            pOtherLayout->addWidget(m_pchMouseTracking);
            pOtherLayout->addWidget(m_pchShowMousePos);
            pOtherLayout->addWidget(m_pchAntiAliasing);
            pOtherLayout->addWidget(m_pchSVGFillBackground);
            pOtherLayout->addLayout(pGraphOtherLayout);
        }
        m_pGroupBox.back()->setLayout(pOtherLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
        {
            pMainLayout->addWidget(m_pGroupBox.at(i));
        }
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);
}


void GraphOptions::showBox(int iBox)
{
    if(iBox<0)
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(true);
    }
    else
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(i==iBox);
    }
}


void GraphOptions::setButtonColors()
{
    m_ptcbTitleClr->setTextColor(s_TitleColor);
    m_ptcbTitleClr->setBackgroundColor(s_BackColor);

    m_ptcbLabelClr->setTextColor(s_LabelColor);
    m_ptcbLabelClr->setBackgroundColor(s_BackColor);

    m_ptcbLegendClr->setTextColor(s_LegendColor);
    m_ptcbLegendClr->setBackgroundColor(s_BackColor);

    m_pcobGraphBack->setColor(s_BackColor);
}


void GraphOptions::initWidget()
{
    setButtonColors();

    QFont const &titlefont = s_TitleFontStruct.font();
    m_ppbTitles->setText(titlefont.family()+QString(" %1").arg(titlefont.pointSize()));
    m_ppbTitles->setFont(titlefont);

    QFont const &labelfont = s_LabelFontStruct.font();
    m_ppbLabels->setText(labelfont.family()+QString(" %1").arg(labelfont.pointSize()));
    m_ppbLabels->setFont(labelfont);

    QFont const &legendfont = s_LegendFontStruct.font();
    m_ppbLegend->setText(legendfont.family()+QString(" %1").arg(legendfont.pointSize()));
    m_ppbLegend->setFont(legendfont);

    m_plbXAxisStyle->setTheStyle(s_XAxis.theStyle());

    m_pchXMajGridShow->setChecked(s_Grid.bXMajGrid());
    m_plbXMajGridStyle->setTheStyle(s_Grid.xMajStyle());
    m_plbXMajGridStyle->setEnabled(s_Grid.bXMajGrid());

    m_pchXMinGridShow->setChecked(s_Grid.bXMinGrid());
    m_plbXMinGridStyle->setTheStyle(s_Grid.xMinStyle());
    m_plbXMinGridStyle->setEnabled(s_Grid.bXMinGrid());

    for(int iy=0; iy<2; iy++)
    {
        Axis const &axis = s_YAxis[iy];
        m_plbYAxisStyle[iy]->setTheStyle(axis.theStyle());

        m_pchYMajGridShow[iy]->setChecked(s_Grid.bYMajGrid(iy));
        m_plbYMajGridStyle[iy]->setTheStyle(s_Grid.yMajStyle(iy));
        m_plbYMajGridStyle[iy]->setEnabled(s_Grid.bYMajGrid(iy));

        m_pchYMinGridShow[iy]->setChecked(s_Grid.bYMinGrid(iy));
        m_plbYMinGridStyle[iy]->setTheStyle(s_Grid.yMinStyle(iy));
        m_plbYMinGridStyle[iy]->setEnabled(s_Grid.bYMinGrid(iy));
    }


    m_pchGraphBorder->setChecked(s_bBorder);
    m_plbBorderStyle->setTheStyle(s_theBorderStyle);

    m_pcobGraphBack->setColor(s_BackColor);

    m_pieLMargin->setValue(s_Margin[0]);
    m_pieRMargin->setValue(s_Margin[1]);
    m_pieTMargin->setValue(s_Margin[2]);
    m_pieBMargin->setValue(s_Margin[3]);

    m_pchSpinAnimation->setChecked(s_bSpinAnimation);
    m_pfeSpinDamping->setValue(s_SpinDamping*100.0);
    m_pfeSpinDamping->setEnabled(s_bSpinAnimation);

    m_pchMouseTracking->setChecked(s_bMouseTracking);
    m_pchShowMousePos->setChecked(Graph::bMousePos());

    m_pchSVGFillBackground->setChecked(GraphSVGWriter::bFillBackground());
    m_pieSVGRefFontSize->setValue(GraphSVGWriter::refFontSize());

    QPalette pal;
    //    LineStyle ls(true, 0, Curve::defaultLineWidth(), pal.windowText().color(), 0);
    LineStyle ls = LineStyle(true, Line::SOLID, Curve::defaultLineWidth(), fl5Color(255,0,0), Line::NOSYMBOL);
    m_pcbLine->setPalette(pal);
    m_pcbLine->setLine(ls);
    m_pcbLine->setCurrentIndex(Curve::defaultLineWidth()-1);

    int LStyle[5];
    int LWidth[5];
    QVector<Line::enumPointStyle>  LPoints;
    for (int i=0; i<5;i++)
    {
        LWidth[i] = i+1;
        LStyle[i] = ls.m_Stipple;
        LPoints.push_back(ls.m_Symbol);
    }

    m_pcbLine->lineDelegate()->setLineColor(xfl::fromfl5Clr(ls.m_Color));
    m_pcbLine->lineDelegate()->setLineStyle(LStyle);
    m_pcbLine->lineDelegate()->setLineWidth(LWidth);
    m_pcbLine->lineDelegate()->setPointStyle(LPoints);

    m_pchAlignChilren->setChecked(Curve::alignChildren());
    m_pieColorIncrement->setValue(xfl::darkFactor());
    m_pieColorIncrement->setEnabled(Curve::alignChildren());

    m_pchAntiAliasing->setChecked(Graph::antiAliasing());

    setGraphModified(false);
}


void GraphOptions::onSpinAnimation(bool bSpin)
{
    m_pfeSpinDamping->setEnabled(bSpin);
}


void GraphOptions::readData()
{
    // dynamically read when something is modified
    /** @todo set all read instructions here instead*/

    setSpinAnimation(m_pchSpinAnimation->isChecked());
    setSpinDamping(m_pfeSpinDamping->value()/100.0);

    setMouseTrack(m_pchMouseTracking->isChecked());
    Graph::showMousePos(m_pchShowMousePos->isChecked());
    Graph::setAntiAliasing(m_pchAntiAliasing->isChecked());

    GraphSVGWriter::setFillBackground(m_pchSVGFillBackground->isChecked());
    GraphSVGWriter::setRefFontSize(std::max(m_pieSVGRefFontSize->value(),4));
}


void GraphOptions::setTitleFontStruct(FontStruct const &fntstruct)
{
    s_TitleFontStruct = fntstruct;
    setGraphModified(true);
}


void GraphOptions::setLabelFontStruct(FontStruct const &fntstruct)
{
    s_LabelFontStruct = fntstruct;
    setGraphModified(true);
}


void GraphOptions::setLegendFontStruct(FontStruct const &fntstruct)
{
    s_LegendFontStruct = fntstruct;
    setGraphModified(true);
}


void GraphOptions::onDefaultLineWidth(int index)
{
    LineStyle ls(true, Line::SOLID, index+1, fl5Color(255,0,0), Line::NOSYMBOL);
    m_pcbLine->setLine(ls);
    m_pcbLine->setCurrentIndex(index+1-1);

    Curve::setDefaultLineWidth(index+1);
    setGraphModified(true);
}


void GraphOptions::onAxisStyle()
{
    LineBtn *pAxisBtn = dynamic_cast<LineBtn*>(sender());

    Axis *pAxis=nullptr;
    if     (pAxisBtn==m_plbXAxisStyle)    pAxis = &s_XAxis;
    else if(pAxisBtn==m_plbYAxisStyle[0]) pAxis = s_YAxis;
    else if(pAxisBtn==m_plbYAxisStyle[1]) pAxis = s_YAxis+1;
    if(!pAxis) return;

    LineMenu lineMenu(this, false);
    lineMenu.initMenu(pAxis->theStyle());
    lineMenu.exec(QCursor::pos());
    LineStyle ls = lineMenu.theStyle();
    pAxis->setTheStyle(ls);
    pAxisBtn->setTheStyle(ls);

    setGraphModified(true);
}


void GraphOptions::onBorderStyle(LineStyle ls)
{
    LineMenu lineMenu(nullptr);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(s_theBorderStyle);
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    s_theBorderStyle = ls;
    m_plbBorderStyle->setTheStyle(ls);

    setGraphModified(true);
}


void GraphOptions::onMargin()
{
    s_Margin[0] = m_pieLMargin->value();
    s_Margin[1] = m_pieRMargin->value();
    s_Margin[2] = m_pieTMargin->value();
    s_Margin[3] = m_pieBMargin->value();
    setGraphModified(true);
}


void GraphOptions::onGraphBorder(bool bShow)
{
    s_bBorder = bShow;
    setGraphModified(true);
}


void GraphOptions::onGraphBackColor()
{
    QColor BkColor = s_BackColor;
    BkColor = QColorDialog::getColor(BkColor, nullptr, "Background", QColorDialog::ShowAlphaChannel);
    if(BkColor.isValid()) s_BackColor = BkColor;

    m_pcobGraphBack->setColor(s_BackColor);
    setButtonColors();
    setGraphModified(true);
}


void GraphOptions::onTitleColor()
{
    QColor color = s_TitleColor;
    color = QColorDialog::getColor(color, this, "Title", QColorDialog::ShowAlphaChannel);
    if(color.isValid())
        s_TitleColor = color;
    m_ptcbTitleClr->setTextColor(color);

    setGraphModified(true);
    update();
}

void GraphOptions::onTitleFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOk=false;
    QFont TitleFont = s_TitleFontStruct.font();

    QFont font = QFontDialog::getFont(&bOk, TitleFont, this, "Axis titles", dialogoptions);

    if (bOk)
    {
        m_ppbTitles->setFont(font);
        m_ppbTitles->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbTitleClr->setFont(font);
        s_TitleFontStruct.setFont(font, QFont::SansSerif);

        setGraphModified(true);
    }
}


void GraphOptions::onLabelColor()
{
    QColor color = s_LabelColor;
    color = QColorDialog::getColor(color, this, "Labels", QColorDialog::ShowAlphaChannel);
    if(color.isValid())
        s_LabelColor = color;
    m_ptcbLabelClr->setTextColor(color);

    setGraphModified(true);
    update();
}


void GraphOptions::onLegendColor()
{
    QColor color = s_LegendColor;
    color = QColorDialog::getColor(color, this, "Labels", QColorDialog::ShowAlphaChannel);
    if(color.isValid())
        s_LegendColor = color;
    m_ptcbLegendClr->setTextColor(color);

    setGraphModified(true);
    update();
}

void GraphOptions::onLegendFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOk=false;
    QFont LegendFont = s_LegendFontStruct.font();
    QFont font = QFontDialog::getFont(&bOk, LegendFont, this, "Legend", dialogoptions);

    if (bOk)
    {
        m_ppbLegend->setFont(font);
        m_ppbLegend->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbLegendClr->setFont(font);
        s_LegendFontStruct.setFont(font, QFont::SansSerif);
        setGraphModified(true);
    }
}


void GraphOptions::onLabelFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOk=false;
    QFont LabelFont = s_LabelFontStruct.font();
    QFont font = QFontDialog::getFont(&bOk, LabelFont, this, "Labels", dialogoptions);

    if (bOk)
    {
        m_ppbLabels->setFont(font);
        m_ppbLabels->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbLabelClr->setFont(font);
        s_LabelFontStruct.setFont(font, QFont::SansSerif);
        setGraphModified(true);
    }
}


void GraphOptions::onXMajGridStyle()
{
    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(s_Grid.xMajStyle());
    lineMenu.exec(QCursor::pos());
    s_Grid.setXMajStyle(lineMenu.theStyle());

    m_plbXMajGridStyle->setTheStyle(lineMenu.theStyle());

    setGraphModified(true);
}


void GraphOptions::onXMinGridStyle()
{
    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(s_Grid.xMinStyle());
    lineMenu.exec(QCursor::pos());
    s_Grid.setXMinStyle(lineMenu.theStyle());
    m_plbXMinGridStyle->setTheStyle(lineMenu.theStyle());

    setGraphModified(true);
}


void GraphOptions::onXMajGridShow(bool bShow)
{
    s_Grid.showXMajGrid(bShow);
    m_plbXMajGridStyle->setEnabled(bShow);

    setGraphModified(true);
}


void GraphOptions::onXMinGridShow(bool bShow)
{
    s_Grid.showXMinGrid(bShow);
    m_plbXMinGridStyle->setEnabled(bShow);

    setGraphModified(true);
}


void GraphOptions::onYMajGridShow(bool bShow)
{
    QCheckBox *pAutoY = dynamic_cast<QCheckBox*>(sender());
    int iy=0;
    if(pAutoY==m_pchYMajGridShow[1]) iy=1;

    s_Grid.showYMajGrid(iy, bShow);
    m_plbYMajGridStyle[iy]->setEnabled(bShow);

    setGraphModified(true);
}


void GraphOptions::onYMinGridShow(bool bShow)
{
    QCheckBox *pAutoY = dynamic_cast<QCheckBox*>(sender());
    int iy=0;
    if(pAutoY==m_pchYMinGridShow[1]) iy=1;

    s_Grid.showYMinGrid(iy, bShow);
    m_plbYMinGridStyle[iy]->setEnabled(bShow);

    setGraphModified(true);
}


void GraphOptions::onYMajGridStyle()
{
    LineBtn *pMajY = dynamic_cast<LineBtn*>(sender());
    int iy=0;

    if     (pMajY==m_plbYMajGridStyle[0]) iy=0;
    else if(pMajY==m_plbYMajGridStyle[1]) iy=1;
    else return;


    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(s_Grid.yMajStyle(iy));
    lineMenu.exec(QCursor::pos());
    s_Grid.setYMajStyle(iy, lineMenu.theStyle());

    m_plbYMajGridStyle[iy]->setTheStyle(lineMenu.theStyle());

    setGraphModified(true);
}


void GraphOptions::onYMinGridStyle()
{
    LineBtn *pMajY = dynamic_cast<LineBtn*>(sender());
    int iy=0;
    if     (pMajY==m_plbYMinGridStyle[0]) iy=0;
    else if(pMajY==m_plbYMinGridStyle[1]) iy=1;
    else return;

    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(s_Grid.yMinStyle(iy));
    lineMenu.exec(QCursor::pos());
    s_Grid.setYMinStyle(iy, lineMenu.theStyle());
    m_plbYMinGridStyle[iy]->setTheStyle(lineMenu.theStyle());

    setGraphModified(true);
}


void GraphOptions::onAlignChildren()
{
    Curve::setAlignChildren(m_pchAlignChilren->isChecked());
    m_pieColorIncrement->setEnabled(Curve::alignChildren());
}


void GraphOptions::onColorIncrement()
{
    int darkerinc = m_pieColorIncrement->value();
    if (darkerinc<100) darkerinc = 100;
    if (darkerinc>300) darkerinc = 300;
    xfl::setDarkFactor(darkerinc);
}


void GraphOptions::loadSettings(QSettings &settings)
{
    settings.beginGroup("GraphOptions");
    {
        s_TitleFontStruct.loadSettings(settings, "titlefont");
        s_LabelFontStruct.loadSettings(settings, "labelfont");
        s_LegendFontStruct.loadSettings(settings, "legendfont");

        //read variables
        if(settings.contains("TitleColor")) s_TitleColor  = settings.value("TitleColor", QColor(0,0,0)).value<QColor>();
        if(settings.contains("LabelColor")) s_LabelColor  = settings.value("LabelColor", QColor(0,0,0)).value<QColor>();
        if(settings.contains("LabelColor")) s_LegendColor = settings.value("LegendColor", QColor(0,0,0)).value<QColor>();

        if(settings.contains("BorderStyle")) xfl::loadLineSettings(settings, s_theBorderStyle, "BorderStyle");
        if(settings.contains("BorderShow")) s_bBorder = settings.value("BorderShow", true).toBool();

        if(settings.contains("BackgroundColor")) s_BackColor  = settings.value("BackgroundColor", QColor(255,255,255)).value<QColor>();

        if(settings.contains("lmargin")) s_Margin[0] = settings.value("lmargin", s_Margin[0]).toInt();
        if(settings.contains("rmargin")) s_Margin[1] = settings.value("rmargin", s_Margin[1]).toInt();
        if(settings.contains("tmargin")) s_Margin[2] = settings.value("tmargin", s_Margin[2]).toInt();
        if(settings.contains("bmargin")) s_Margin[3] = settings.value("bmargin", s_Margin[3]).toInt();

        if(settings.contains("ShowLegend")) s_bShowLegend = settings.value("ShowLegend", false).toBool();

        s_bMouseTracking = settings.value("GraphMouseTracking", s_bMouseTracking).toBool();
        s_bSpinAnimation = settings.value("bDynMouse",          s_bSpinAnimation).toBool();
        s_SpinDamping    = settings.value("MouseDamping",       s_SpinDamping).toDouble();

        GraphSVGWriter::setFillBackground(settings.value("SVGFillBackground", GraphSVGWriter::bFillBackground()).toBool());
        GraphSVGWriter::setRefFontSize(settings.value("SVGRefFontSize", GraphSVGWriter::refFontSize()).toInt());

        if(settings.contains("LegendPosition"))
        {
            int pos = settings.value("LegendPosition",4).toInt();
            switch(pos)
            {
                case 0: s_LegendPosition = Qt::AlignRight  | Qt::AlignVCenter;    break;
                case 1: s_LegendPosition = Qt::AlignLeft   | Qt::AlignVCenter;    break;
                case 2: s_LegendPosition = Qt::AlignTop    | Qt::AlignHCenter;    break;
                case 3: s_LegendPosition = Qt::AlignTop    | Qt::AlignLeft;       break;
                default:
                case 4: s_LegendPosition = Qt::AlignTop    | Qt::AlignRight;      break;
                case 5: s_LegendPosition = Qt::AlignBottom | Qt::AlignHCenter;    break;
                case 6: s_LegendPosition = Qt::AlignBottom | Qt::AlignLeft;       break;
                case 7: s_LegendPosition = Qt::AlignBottom | Qt::AlignRight;      break;
            }
        }

        s_Grid.loadSettings(settings);
        s_XAxis.loadSettings(settings);
        s_YAxis[0].loadSettings(settings);
        s_YAxis[1].loadSettings(settings);

        Graph::showMousePos(   settings.value("ShowMousePosition", Graph::bMousePos()).toBool());
        Graph::setHighLighting(settings.value("HighlightObject",   Graph::isHighLighting()).toBool());
        Graph::setHighLighting(true); // forced in v7.50
        Graph::setAntiAliasing(settings.value("AntiAliasing",   Graph::antiAliasing()).toBool());

        Curve::setDefaultLineWidth(settings.value("DefaultCurveWidth",  Curve::defaultLineWidth()).toInt());
        Curve::setAlignChildren(   settings.value("AlignChidrenCurves", Curve::alignChildren()).toBool());
    }
    settings.endGroup();
}


void GraphOptions::saveSettings(QSettings &settings)
{
    settings.beginGroup("GraphOptions");
    {
        s_TitleFontStruct.saveSettings(settings, "titlefont");
        s_LabelFontStruct.saveSettings(settings, "labelfont");
        s_LegendFontStruct.saveSettings(settings, "legendfont");

        settings.setValue("TitleColor",  s_TitleColor);
        settings.setValue("LabelColor",  s_LabelColor);
        settings.setValue("LegendColor", s_LegendColor);

        xfl::saveLineSettings(settings, s_theBorderStyle, "BorderStyle");
        settings.setValue("BorderShow", s_bBorder);

        settings.setValue("BackgroundColor", s_BackColor);

        settings.setValue("lmargin", s_Margin[0]);
        settings.setValue("rmargin", s_Margin[1]);
        settings.setValue("tmargin", s_Margin[2]);
        settings.setValue("bmargin", s_Margin[3]);

        settings.setValue("ShowLegend",         s_bShowLegend);
        settings.setValue("bDynMouse",          s_bSpinAnimation);
        settings.setValue("MouseDamping",       s_SpinDamping);
        settings.setValue("GraphMouseTracking", s_bMouseTracking);

        settings.setValue("SVGFillBackground", GraphSVGWriter::bFillBackground());
        settings.setValue("SVGRefFontSize",    GraphSVGWriter::refFontSize());

        if     (s_LegendPosition == (Qt::AlignRight  | Qt::AlignVCenter))  settings.setValue("LegendPosition", 0);
        else if(s_LegendPosition == (Qt::AlignLeft   | Qt::AlignVCenter))  settings.setValue("LegendPosition", 1);
        else if(s_LegendPosition == (Qt::AlignTop    | Qt::AlignHCenter))  settings.setValue("LegendPosition", 2);
        else if(s_LegendPosition == (Qt::AlignTop    | Qt::AlignLeft))     settings.setValue("LegendPosition", 3);
        else if(s_LegendPosition == (Qt::AlignTop    | Qt::AlignRight))    settings.setValue("LegendPosition", 4);
        else if(s_LegendPosition == (Qt::AlignBottom | Qt::AlignHCenter))  settings.setValue("LegendPosition", 5);
        else if(s_LegendPosition == (Qt::AlignBottom | Qt::AlignLeft))     settings.setValue("LegendPosition", 6);
        else if(s_LegendPosition == (Qt::AlignBottom | Qt::AlignRight))    settings.setValue("LegendPosition", 7);

        s_Grid.saveSettings(settings);
        s_XAxis.saveSettings(settings);
        s_YAxis[0].saveSettings(settings);
        s_YAxis[1].saveSettings(settings);

        settings.setValue("ShowMousePosition",  Graph::bMousePos());
        settings.setValue("HighlightObject",    Graph::isHighLighting());

        settings.setValue("AntiAliasing",   Graph::antiAliasing());

        settings.setValue("DefaultCurveWidth",  Curve::defaultLineWidth());
        settings.setValue("AlignChidrenCurves", Curve::alignChildren());
    }
    settings.endGroup();
}


