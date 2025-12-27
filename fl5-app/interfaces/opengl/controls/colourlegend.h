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

#include <QString>
#include <QColor>
#include <QSettings>
#include <QPixmap>
#include <QLinearGradient>

class ColourLegend
{
    public:
        ColourLegend();

        void setVisible(bool b) {m_bVisible=b;}
        bool isVisible() const {return m_bVisible;}

        void setTitle(QString const & title);
        QString const &legendTitle() const {return m_LegendTitle;}

        void setRange(double vmin, double vmax) {m_vMin=vmin; m_vMax=vmax;}

        void setColorGradient();

        void makeLegend();
        void resize(int w, int h, int dpr);

        static int colourCount() {return s_Clr.size();}
        static QVector<QColor> &colours() {return s_Clr;}
        static void setColours(QVector<QColor> const &clr) {s_Clr=clr;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static QColor colour(float tau);

    public:
        QPixmap m_pix;
    private:

        QLinearGradient m_ClrGradient;
        bool m_bVisible;
        double m_vMin, m_vMax;

        QRectF m_GradientRect;
        int m_CharHeight, m_AverageCharWidth;
        QString m_LegendTitle;

        qreal m_DevicePixelRatio; // depends on the window thtat is being targeted; cf QGuiApplication::devicePixelRatio()

    public:
        static QVector<QColor> s_Clr;

};


