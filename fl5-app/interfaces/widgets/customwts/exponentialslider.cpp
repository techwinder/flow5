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

#include <cmath>


#include <interfaces/widgets/customwts/exponentialslider.h>


//caution, use only for "symmetrical" sliders ranging from -range to +range

ExponentialSlider::ExponentialSlider(QWidget * pParent) :
    QSlider( Qt::Horizontal, pParent), m_exponential(1.0), m_bCentered(false)
{
;
}


ExponentialSlider::ExponentialSlider(Qt::Orientation orientation, QWidget * pParent) :
     QSlider(orientation, pParent), m_exponential(1.0), m_bCentered(false)
{
}


ExponentialSlider::ExponentialSlider(bool bCentered, double expo, Qt::Orientation orientation, QWidget * pParent):
    QSlider(orientation, pParent), m_exponential(expo), m_bCentered(bCentered)
{
}


double ExponentialSlider::expValue() const
{
    if(m_bCentered)
    {
        double mid      = (double(minimum()) + double(maximum()))/2.0;
        double span     = double(maximum() - minimum());
        double halfspan = span/2.0;
        double position = double(value());
        double frac     = (position-mid)/halfspan;

        if(frac>=0.0) return + pow( frac, m_exponential) * halfspan;
        else          return - pow(-frac, m_exponential) * halfspan;
    }
    else
    {
        double span     = double(maximum() - minimum());
        double position = double(value());
        double frac     = position/span;
        return pow( frac, m_exponential) * span;
    }
}


float ExponentialSlider::expValuef() const
{
    if(m_bCentered)
    {
        float mid      = (float(minimum()) + float(maximum()))/2.0f;
        float span     = float(maximum() - minimum());
        float halfspan = span/2.0f;
        float position = float(value());
        float frac     = (position-mid)/halfspan;

        if(frac>=0.0f) return mid + powf( frac, float(m_exponential)) * halfspan;
        else           return mid - powf(-frac, float(m_exponential)) * halfspan;
    }
    else
    {
//        qDebug()<<"exxxp"<<minimum()<<maximum()<<value();
        float span     = float(maximum() - minimum());
        float position = float(value());
        float frac     = position/span;
        return powf( frac, float(m_exponential)) * span;
    }
}


void ExponentialSlider::setExpValue(double ExpVal)
{
    if(m_bCentered)
    {
        int mid    = int((minimum() + maximum())/2.0);
        int span   = maximum() - minimum();
        double halfspan = double(span)/2.0;

        double yRel = ExpVal / halfspan;
        yRel = std::min(yRel,  1.0);
        yRel = std::max(yRel, -1.0);

        double xRel = pow(fabs(yRel), 1.0/m_exponential);

        if(yRel<0.0) xRel = -xRel;

        setValue(mid + int(xRel*halfspan));
    }
    else
    {
        int span   = maximum() - minimum();

        double yRel = ExpVal / span;
        yRel = std::min(yRel, 1.0);
        yRel = std::max(yRel, 0.0);

        double xRel = pow(yRel, 1.0/m_exponential);

        setValue(int(xRel*span));
    }
}


















