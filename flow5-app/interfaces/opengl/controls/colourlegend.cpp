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

#include "colourlegend.h"


#include <QPainter>
#include <QPaintEvent>



#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>


#define NCPCOLORS 20

QVector<QColor> ColourLegend::s_Clr = {Qt::blue, Qt::green, Qt::red};

/** @todo pix is not needed; redefine class as a widget and override paintEvent */
ColourLegend::ColourLegend()
{
    m_bVisible = false;
    m_vMin = 0.0;
    m_vMax = 0.0;

    m_DevicePixelRatio = 1;

    m_pix = QPixmap(300,500);
    m_pix.fill(Qt::darkRed);

    m_LegendTitle = "--Cp--";
    QFontMetrics fm(DisplayOptions::tableFont());
    m_CharHeight = fm.height();
    m_AverageCharWidth = fm.averageCharWidth();
}


void ColourLegend::setTitle(QString const & title)
{
    m_LegendTitle=title;
}


void ColourLegend::resize(int w, int h, int dpr)
{
    m_DevicePixelRatio = dpr;
    m_pix = QPixmap(w*dpr*2, h*dpr);

    int topoffset = m_CharHeight*3*m_DevicePixelRatio;
    m_GradientRect = m_pix.rect();
    m_GradientRect.adjust(m_GradientRect.width()-m_AverageCharWidth*3*m_DevicePixelRatio,0,0, -topoffset);
    m_GradientRect.moveTop(topoffset);

    setColorGradient();
}


void ColourLegend::setColorGradient()
{
    m_ClrGradient.setStart    (m_GradientRect.center().x(), m_GradientRect.bottom());
    m_ClrGradient.setFinalStop(m_GradientRect.center().x(), m_GradientRect.top());

    for (int i=0; i<NCPCOLORS; i++)
    {
        float fi = float(i)/float(NCPCOLORS-1);
        QColor clr = colour(fi);
        m_ClrGradient.setColorAt(double(fi), clr);
    }
}


void ColourLegend::makeLegend()
{
    m_pix.fill(Qt::transparent);
    QPainter painter(&m_pix);
    painter.save();

    QPen textpen(DisplayOptions::textColor());
    painter.setPen(textpen);
    QFont fnt = DisplayOptions::tableFont();
    fnt.setPointSize(DisplayOptions::tableFont().pointSize() * m_DevicePixelRatio);
    painter.setFont(fnt);

    FontStruct const &fs = DisplayOptions::textFontStruct();
    int w = fs.width(m_LegendTitle+" ");
    painter.drawText(m_GradientRect.right()-w *m_DevicePixelRatio, m_CharHeight*2*m_DevicePixelRatio, m_LegendTitle);

    painter.fillRect(m_GradientRect, m_ClrGradient);


    float range = 2.0f;
    float f0 = 1.0f;
    f0 = m_vMax;
    range = m_vMax-m_vMin;

    int nLabels = W3dPrefs::s_NContourLines+1;
    float delta = -range / float(nLabels);

    double yPos  = m_GradientRect.top()+m_CharHeight;
    double h = m_GradientRect.height()-m_CharHeight;
    double dy  = (h)/double(nLabels-1);

    QString strong;

    for (int i=0; i<=nLabels; i++)
    {
        float f = f0 + float(i) * delta;
        if(xfl::isLocalized()) strong = QString("%L1").arg(f, 9, 'f', 2);
        else                   strong = QString("%1" ).arg(f, 9, 'f', 2);
        double x = m_GradientRect.left()-fs.width(strong)*(m_DevicePixelRatio*1.1);
        painter.drawText(x, yPos, strong);
        yPos += dy;
    }

    painter.restore();
}


QColor ColourLegend::colour(float tau)
{
    return xfl::colour(s_Clr, tau);
}


void ColourLegend::loadSettings(QSettings &settings)
{
    QString strange;
    settings.beginGroup("ColourLegend");
    {
        if(settings.contains("NColours"))
        {
            int nColours = settings.value("NColours",3).toInt();
            s_Clr.resize(nColours);
            for(int ic=0; ic<nColours; ic++)
            {
                strange = QString::asprintf("Color_%d", ic);
                s_Clr[ic] = settings.value(strange, QColor(200,200,200)).value<QColor>();
            }
        }
    }
    settings.endGroup();
}


void ColourLegend::saveSettings(QSettings &settings)
{
    QString strange;
    settings.beginGroup("ColourLegend");
    {
        settings.setValue("NColours", s_Clr.size());
        for(int ic=0; ic<s_Clr.size(); ic++)
        {
            strange = QString::asprintf("Color_%d", ic);
            settings.setValue(strange, s_Clr.at(ic));
        }
    }
    settings.endGroup();
}
