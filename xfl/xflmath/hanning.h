/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>


class Hanning
{
    public:
        Hanning(double frequency, double cutfrequency);

        void setFrequency(double freq) {m_Frequency=freq;}
        void setCutFrequency(double freq) {m_FilterCutFreq=freq;}
        void setFilterCoefs();
        void setHanningWindow();

        void filterSeries(const QVector<double> &doubleData, QVector<double> &y);


    private:
        QVector<double> m_Window;
        QVector<double> m_FilterCoef;

        double m_FilterCutFreq;

        double m_Frequency;
};

void testHanning(QVector<double> &xt, QVector<double> &series, QVector<double> &yF);

