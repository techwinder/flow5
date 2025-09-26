/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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


