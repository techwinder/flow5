/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QSlider>


class ExponentialSlider : public QSlider
{
    public:
        ExponentialSlider(QWidget * pParent = nullptr);
        ExponentialSlider(Qt::Orientation orientation, QWidget * pParent = nullptr);
        ExponentialSlider(bool bCentered, double expo, Qt::Orientation orientation, QWidget * pParent = nullptr);

        double expValue() const;
        float expValuef() const;

        void setExpValue(double expVal);
        void setExpValuef(float expVal) {setExpValue(double(expVal));}

        void setExponential(double expo){m_exponential = expo;}
        void setCentered(bool bCentered){m_bCentered = bCentered;}

    private:
        double m_exponential;
        bool m_bCentered;
};


