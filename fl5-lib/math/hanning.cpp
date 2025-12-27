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

#include <iostream>
#include <cmath>
#include <QString>


#include <hanning.h>
#include <constants.h>

#define FILTERORDER 40


Hanning::Hanning(double frequency, double cutfrequency)
{
    m_Frequency = frequency;
    m_FilterCutFreq=cutfrequency;

    setHanningWindow();
    setFilterCoefs();
}


void Hanning::setHanningWindow()
{
    m_Window.resize(FILTERORDER/2);

    m_Window[0] = 1.0;
    for(int iw=1; iw<FILTERORDER/2; iw++)
    {
        m_Window[iw] = 0.5 *(1+cos(2.0*PI*double(iw) / double(FILTERORDER)));
    }
}


void Hanning::setFilterCoefs()
{
    double t;
    m_FilterCoef.resize(FILTERORDER/2);

    m_FilterCoef[0] = 2.0* m_FilterCutFreq/ m_Frequency;
    for(int iw=1; iw<FILTERORDER/2; iw++)
    {
        t = double(iw) / m_Frequency;
        m_FilterCoef[iw] = sin(2.0 *PI* m_FilterCutFreq * t)/PI/t/ m_Frequency;
    }
}



void Hanning::filterSeries(std::vector<double> const & doubleData, std::vector<double> &y)
{
//    std::vector<double> x(doubleData.size());
    y.resize(doubleData.size());

    //filter out y
    for (unsigned int ip=0; ip<doubleData.size(); ip++)
    {
//        x[ip] = double(ip) / m_Frequency;
/*        if (ip<FILTERORDER/2)
        {
            y[ip] = doubleData[ip];
        }
        else if (ip>doubleData.size() - FILTERORDER/2 - 1)
        {
            y[ip] = doubleData[ip];
        }
        else*/
        {
            y[ip] = doubleData[ip] * m_FilterCoef[0] * m_Window[0];

            for (unsigned int iw=1; iw<FILTERORDER/2; iw++)
            {
                if(ip>iw)                   y[ip] += m_FilterCoef[iw] * m_Window[iw] * doubleData[ip - iw];
                if(ip+iw<doubleData.size()) y[ip] += m_FilterCoef[iw] * m_Window[iw] * doubleData[ip + iw];
            }
        }
    }
}


void testHanning(std::vector<double> &xt, std::vector<double> &series, std::vector<double> &yF)
{
    double sigfreq = 5.0;
    double cutofffreq = 0.3;
    //make raw curve
    double f0=0.08;
    double f1=1.0;
    double f2=2.0;
    double Amp0=3.0;
    double Amp1=1.0;
    double Amp2=3.0;

    double main, pert1, pert2;

    for(int it=0; it<=100; it++)
    {
        double t = double(it)/sigfreq;
        main  = Amp0*cos(2.0*PI*f0*t);
        pert1 = Amp1*sin(2.0*PI*f1*t+0.23234);
        pert2 = Amp2*sin(2.0*PI*f2*t-0.345);
        double signal = main+pert1+pert2;
        xt.push_back(t);
        series.push_back(signal);

//        pCurve->appendPoint(t,signal);
    }

    Hanning han(sigfreq, cutofffreq);
    han.filterSeries(series, yF);
    std::cout << ("            x       signal     filtered")  << std::endl;
    for(unsigned int i=0; i<series.size(); i++)
    {
        std::cout << QString::asprintf("  %11.5g  %11.5g  %11.5g", xt.at(i), series.at(i), yF.at(i)).toStdString() << std::endl;
    }
}


