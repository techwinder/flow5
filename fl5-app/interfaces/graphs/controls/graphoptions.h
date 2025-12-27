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

#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>

#include <api/linestyle.h>
#include <core/fontstruct.h>
#include <interfaces/graphs/graph/graph.h>

class LineBtn;
class TextClrBtn;
class ColorBtn;
class FloatEdit;
class IntEdit;
class LineCbBox;

class GraphOptions : public QWidget
{
	Q_OBJECT

    public:
        GraphOptions(QWidget *parent = nullptr);

        void showBox(int iBox);
        void initWidget();
        void setButtonColors();
        void readData();

        static bool bMouseTrack() {return s_bMouseTracking;}
        static void setMouseTrack(bool bTrack) {s_bMouseTracking=bTrack;}

        static void setSpinAnimation(bool b) {s_bSpinAnimation=b;}
        static void setSpinDamping(double damp) {s_SpinDamping=damp;}

        static bool bSpinAnimation() {return s_bSpinAnimation;}
        static double spinDamping() {return s_SpinDamping;}


        static void setTitleFontStruct(FontStruct const &fntstruct);
        static void setLabelFontStruct(FontStruct const &fntstruct);
        static void setLegendFontStruct(FontStruct const &fntstruct);

        static bool isGraphModified()    {return s_bIsGraphModified;}
        static void setGraphModified(bool bModified) {s_bIsGraphModified=bModified;}

        static void setDefaults(bool bDark);
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void resetGraphSettings(Graph &graph);

    private:
        void connectSignals();
        void makeWidgets();

    public slots:
        void onAlignChildren();
        void onAxisStyle();
        void onBorderStyle(LineStyle ls);
        void onColorIncrement();
        void onDefaultLineWidth(int index);
        void onGraphBackColor();
        void onGraphBorder(bool bShow);
        void onLabelColor();
        void onLabelFont();
        void onLegendColor();
        void onLegendFont();
        void onMargin();
        void onTitleColor();
        void onTitleFont();
        void onXMajGridShow(bool bShow);
        void onXMajGridStyle();
        void onXMinGridShow(bool bShow);
        void onXMinGridStyle();
        void onYMajGridShow(bool bShow);
        void onYMajGridStyle();
        void onYMinGridShow(bool bShow);
        void onYMinGridStyle();
        void onSpinAnimation(bool);

    private:

        // font and color widgets
        QPushButton *m_ppbTitles, *m_ppbLabels, *m_ppbLegend;
        TextClrBtn *m_ptcbTitleClr, *m_ptcbLabelClr, *m_ptcbLegendClr;

        QCheckBox *m_pchGraphBorder;
        ColorBtn *m_pcobGraphBack;
        LineBtn *m_plbBorderStyle;
        IntEdit *m_pieLMargin, *m_pieRMargin, *m_pieTMargin, *m_pieBMargin;
        QFont *m_pTitleFont, *m_pLabelFont;

        QCheckBox *m_pchXMajGridShow, *m_pchXMinGridShow;
        QCheckBox *m_pchYMajGridShow[2], *m_pchYMinGridShow[2];

        LineBtn *m_plbXAxisStyle, *m_plbXMajGridStyle, *m_plbXMinGridStyle;
        LineBtn *m_plbYAxisStyle[2], *m_plbYMajGridStyle[2], *m_plbYMinGridStyle[2];

        QCheckBox *m_pchSpinAnimation;
        FloatEdit *m_pfeSpinDamping;
        QCheckBox *m_pchMouseTracking;
        QCheckBox *m_pchShowMousePos;

        QCheckBox *m_pchAlignChilren;
        LineCbBox *m_pcbLine;
        IntEdit *m_pieColorIncrement;

        QCheckBox *m_pchAntiAliasing;

        IntEdit *m_pieSVGRefFontSize;
        QCheckBox *m_pchSVGFillBackground;

        QVector<QGroupBox *>m_pGroupBox;

        // graph data
        static bool s_bMouseTracking;

        static bool s_bSpinAnimation;
        static double s_SpinDamping;

        static bool s_bBorder;
        static bool s_bShowLegend;

        static Qt::Alignment s_LegendPosition;

        static LineStyle s_theBorderStyle;
        static int s_Margin[4]; // left, right, top, bottom

        static Axis s_XAxis;
        static Axis s_YAxis[2]; /**< the two Y axes */

        static Grid s_Grid;

        static QColor s_BackColor;

        static QColor s_TitleColor;
        static QColor s_LabelColor;
        static QColor s_LegendColor;

        static FontStruct s_TitleFontStruct;
        static FontStruct s_LabelFontStruct;
        static FontStruct s_LegendFontStruct;

        static bool s_bIsGraphModified;

};

