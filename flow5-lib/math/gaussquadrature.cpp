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
#include <QString>


#include <gaussquadrature.h>
#include <mathelem.h>

using namespace std;


#define TOLERANCE   1.e-8
#define MAXBAIRSTOWITER 100
#define MAXTRIES        10
#define PI             3.14159265358979




GaussQuadrature::GaussQuadrature(int order)
{
    _n = order;
    m_bError = !makeCoefficients(_n);
}


bool GaussQuadrature::makeCoefficients(int degree)
{
    _n = degree;

    _p.resize(_n+1);
    _x.resize(_n);
    _w.resize(_n);
    fill(_p.begin(), _p.end(), 0);
    fill(_x.begin(), _x.end(), 0);
    fill(_w.begin(), _w.end(), 0);

    if(degree<=17)
    {
        hardcodeWeigths();
        return true;
    }
    else
    {
        return computeWeights();
    }
}


bool GaussQuadrature::computeWeights()
{
    //compute polynomial coefficients
    Legendre(_n, _p.data());

    //compute roots, all real
    complex<double> *roots = new complex<double>[_n+1];
    std::vector<double> pol(_n+1,0);

    if(!LinBairstow(pol.data(), roots, _n))
    {
        //clean up
        delete [] roots;
        return false;
    }

    //compute weights
    for(int i=0; i<_n; i++)
    {
        _x[i] = roots[i].real();
        double pp = polynomialDerivative(_n, _p.data(), _x[i]);
        _w[i] = 2.0/(1.0-_x[i]*_x[i])/pp/pp;
    }

    //clean up
    delete [] roots;

    return true;
}



double GaussQuadrature::quadrature(double (*pFunc)(double), double a, double b) const
{
    if(fabs(b-a)<TOLERANCE) return 0.0;

    double sum = 0.0;
    for(int i=0; i<_n; i++)
    {
        sum += weight(i,a,b) * pFunc(xrel(i,a,b));
    }
    return sum;
}


double GaussQuadrature::quadrature2(double (*pFunc2)(double, double), double a0, double b0, double (*pFunc)(double)) const
{
    if(fabs(b0-a0)<TOLERANCE) return 0.0;

    double sum = 0.0;
    double a1 = 0.0;
    for(int i=0; i<_n; i++)
    {
        // change origin and scale by 1/2 since GQ points are defined in [-1,1]
        double xi = (a0+b0)/2.0+ (b0-a0)/2.0 * _x[i];
        double b1 = pFunc(xi);
        double sumj = 0.0;
        for(int j=0; j<_n; j++)
        {
            sumj += _w[j] * pFunc2(xi, (a1+b1)/2.0 + (b1-a1)/2.0 * _x[j]);
        }
        sumj *= (b1-a1)/2.0;

        sum+= _w[i] * sumj;
    }
    return sum * (b0-a0)/2.0;
}


double GaussQuadrature::quadrature2(double (*pFunc2)(double, double), double a0, double b0, double a1, double b1) const
{
    if(fabs(b0-a0)<TOLERANCE || fabs(b1-a1)<TOLERANCE) return 0.0;

    double sum = 0.0;
    for(int i=0; i<_n; i++)
    {
        for(int j=0; j<_n; j++)
        {
            sum += _w[i] * _w[j] * pFunc2((b0-a0)/2.0 * _x[i] + (a0+b0)/2.0, (b1-a1)/2.0 * _x[j] + (a1+b1)/2.0);
        }
    }
    return sum * (b0-a0)/2.0* (b1-a1)/2.0;
}



/** Returns the value at point x of the polynomial defined by the array of n coefficients p */
double GaussQuadrature::polynomial(int n, double *p, double x) const
{
    double pol = 0.0;
    for(int i=0; i<=n; i++)
        pol += pow(x, i) * p[i];
    return pol;
}



/** Returns the value at point x of the derivative of the polynomial defined by the array of n coefficients p */
double GaussQuadrature::polynomialDerivative(int n, double *p, double x) const
{
    double polDer = 0.0;
    for(int i=1; i<=n; i++)
        polDer += double(i) * pow(x, i-1) * p[i];
    return polDer;
}


void GaussQuadrature::hardcodeWeigths()
{
    switch(_n)
    {
        case 1:
        {
            _x[0]=      0.000000000;    _w[0]=      2.000000000;
            break;
        }
        case 2:
        {
            _x[0]=     -0.577350269;    _w[0]=      1.000000000;
            _x[1]=      0.577350269;    _w[1]=      1.000000000;
            break;
        }
        case 3:
        {
            _x[0]=     -0.774596669;    _w[0]=      0.555555557;
            _x[1]=                0;    _w[1]=      0.888888889;
            _x[2]=      0.774596669;    _w[2]=      0.555555556;
            break;
        }
        case 4:
        {
            _x[0]=     -0.861136312;    _w[0]=      0.347854845;
            _x[1]=     -0.339981043;    _w[1]=      0.652145155;
            _x[2]=      0.339981043;    _w[2]=      0.652145155;
            _x[3]=      0.861136312;    _w[3]=      0.347854845;
            break;
        }
        case 5:
        {
            _x[0]=     -0.906179846;    _w[0]=      0.236926885;
            _x[1]=     -0.538469308;    _w[1]=      0.478628672;
            _x[2]=      0.000000000;    _w[2]=      0.568888889;
            _x[3]=      0.538469311;    _w[3]=       0.47862867;
            _x[4]=      0.906179846;    _w[4]=      0.236926885;
            break;
        }
        case 6:
        {
            _x[0]=     -0.932469514;    _w[0]=      0.171324492;
            _x[1]=      -0.66120937;    _w[1]=      0.360761587;
            _x[2]=     -0.238619207;    _w[2]=       0.46791393;
            _x[3]=      0.238619206;    _w[3]=       0.46791393;
            _x[4]=      0.661209373;    _w[4]=      0.360761585;
            _x[5]=      0.932469514;    _w[5]=      0.171324492;
            break;
        }
        case 7:
        {
            _x[0]=     -0.949107912;    _w[0]=      0.129484966;
            _x[1]=     -0.741531186;    _w[1]=      0.279705392;
            _x[2]=     -0.405845151;    _w[2]=      0.381830050;
            _x[3]=      0.000000000;    _w[3]=      0.417959184;
            _x[4]=      0.405845151;    _w[4]=      0.381830051;
            _x[5]=      0.741531186;    _w[5]=      0.279705391;
            _x[6]=      0.949107912;    _w[6]=      0.129484966;
            break;
        }
        case 8:
        {
            _x[0]=     -0.960289856;    _w[0]=      0.101228536;
            _x[1]=     -0.796666477;    _w[1]=      0.222381034;
            _x[2]=     -0.525532412;    _w[2]=      0.313706646;
            _x[3]=     -0.183434643;    _w[3]=      0.362683783;
            _x[4]=      0.183434642;    _w[4]=      0.362683783;
            _x[5]=      0.525532412;    _w[5]=      0.313706645;
            _x[6]=      0.796666475;    _w[6]=      0.222381037;
            _x[7]=      0.960289856;    _w[7]=      0.101228537;
            break;
        }
        case 9:
        {
            _x[0]=      -0.96816024;    _w[0]=     0.0812743884;
            _x[1]=     -0.836031107;    _w[1]=      0.180648161;
            _x[2]=     -0.613371431;    _w[2]=      0.260610697;
            _x[3]=     -0.324253426;    _w[3]=      0.312347077;
            _x[4]=      0.324253421;    _w[4]=      0.312347077;
            _x[5]=      0.000000000;    _w[5]=      0.330239355;
            _x[6]=      0.613371434;    _w[6]=      0.260610696;
            _x[7]=      0.836031106;    _w[7]=      0.180648162;
            _x[8]=       0.96816024;    _w[8]=     0.0812743877;
            break;
        }
        case 10:
        {
            _x[0]=     -0.973906529;    _w[0]=     0.0666713443;
            _x[1]=     -0.865063367;    _w[1]=      0.149451349;
            _x[2]=     -0.679409568;    _w[2]=      0.219086363;
            _x[3]=     -0.433395394;    _w[3]=      0.269266719;
            _x[4]=     -0.148874339;    _w[4]=      0.295524225;
            _x[5]=      0.148874339;    _w[5]=      0.295524225;
            _x[6]=      0.433395394;    _w[6]=      0.269266719;
            _x[7]=      0.679409568;    _w[7]=      0.219086362;
            _x[8]=      0.865063367;    _w[8]=      0.149451349;
            _x[9]=      0.973906529;    _w[9]=     0.0666713443;
            break;
        }
        case 11:
        {
            _x[0]=     -0.978228658;    _w[0]=     0.0556685671;
            _x[1]=       -0.8870626;    _w[1]=     0.125580369;
            _x[2]=     -0.730152009;    _w[2]=      0.186290209;
            _x[3]=     -0.519096125;    _w[3]=      0.233193766;
            _x[4]=     -0.269543149;    _w[4]=      0.262804546;
            _x[5]=      0.000000000;    _w[5]=      0.272925087;
            _x[6]=      0.269543173;    _w[6]=      0.262804542;
            _x[7]=       0.51909615;    _w[7]=      0.233193758;
            _x[8]=      0.730151981;    _w[8]=      0.186290226;
            _x[9]=      0.887062591;    _w[9]=      0.125580379;
            _x[10]=     0.978228661;    _w[10]=    0.0556685599;
            break;
        }
        case 12:
        {
            _x[0]=      0.125233377;    _w[0]=      0.249147048;
            _x[1]=      0.367831533;    _w[1]=       0.23349253;
            _x[2]=     -0.125233393;    _w[2]=      0.249147047;
            _x[3]=      0.587317934;    _w[3]=      0.203167434;
            _x[4]=     -0.367831499;    _w[4]=      0.233492537;
            _x[5]=      0.769902674;    _w[5]=      0.160078329;
            _x[6]=     -0.587317954;    _w[6]=      0.203167427;
            _x[7]=      0.904117256;    _w[7]=      0.106939326;
            _x[8]=     -0.769902674;    _w[8]=      0.160078329;
            _x[9]=      0.981560634;    _w[9]=     0.0471753364;
            _x[10]=    -0.981560634;    _w[10]=    0.0471753364;
            _x[11]=    -0.904117256;    _w[11]=     0.106939326;
            break;
        }
        case 13:
        {
            _x[0]=      0.917598398;    _w[0]=     0.0921215013;
            _x[1]=      0.642349339;    _w[1]=      0.178145981;
            _x[2]=      0.801578092;    _w[2]=       0.13887351;
            _x[3]=      0.230458316;    _w[3]=       0.22628318;
            _x[4]=      0.448492751;    _w[4]=      0.207816048;
            _x[5]=     -0.801578091;    _w[5]=       0.13887351;
            _x[6]=      0.984183054;    _w[6]=     0.0404840061;
            _x[7]=     -0.642349339;    _w[7]=      0.178145981;
            _x[8]=      0.000000000;    _w[8]=      0.232551553;
            _x[9]=     -0.448492751;    _w[9]=      0.207816048;
            _x[10]=    -0.230458316;    _w[10]=      0.22628318;
            _x[11]=    -0.984183055;    _w[11]=    0.0404840048;
            _x[12]=    -0.917598399;    _w[12]=    0.0921214998;
            break;
        }
        case 14:
        {
            _x[0]=      0.108054943;    _w[0]=      0.215263854;
            _x[1]=      0.319112369;    _w[1]=      0.205198464;
            _x[2]=     -0.108054952;    _w[2]=      0.215263853;
            _x[3]=      0.515248656;    _w[3]=      0.185538392;
            _x[4]=     -0.319112356;    _w[4]=      0.205198466;
            _x[5]=       0.68729287;    _w[5]=      0.157203182;
            _x[6]=      -0.51524864;    _w[6]=      0.185538397;
            _x[7]=      0.827201343;    _w[7]=      0.121518553;
            _x[8]=     -0.687292907;    _w[8]=      0.157203166;
            _x[9]=      0.928434866;    _w[9]=     0.0801581059;
            _x[10]=    -0.827201311;    _w[10]=     0.121518574;
            _x[11]=     0.986283814;    _w[11]=     0.035119447;
            _x[12]=    -0.986283809;    _w[12]=    0.0351194603;
            _x[13]=    -0.928434884;    _w[13]=    0.0801580872;
            break;
        }
        case 15:
        {
            _x[0]=       0.20119409;    _w[0]=      0.198431486;
            _x[1]=       0.00000000;    _w[1]=      0.202578242;
            _x[2]=       0.39415135;    _w[2]=      0.186160999;
            _x[3]=     -0.201194096;    _w[3]=      0.198431485;
            _x[4]=       0.57097217;    _w[4]=      0.166269206;
            _x[5]=     -0.394151346;    _w[5]=         0.186161;
            _x[6]=      0.724417732;    _w[6]=      0.139570678;
            _x[7]=     -0.570972173;    _w[7]=      0.166269206;
            _x[8]=      0.937273393;    _w[8]=     0.0703660471;
            _x[9]=     -0.724417731;    _w[9]=      0.139570678;
            _x[10]=     0.848206583;    _w[10]=     0.107159221;
            _x[11]=    -0.848206583;    _w[11]=      0.10715922;
            _x[12]=     0.987992518;    _w[12]=    0.0307532423;
            _x[13]=    -0.987992518;    _w[13]=     0.030753242;
            _x[14]=    -0.937273392;    _w[14]=    0.0703660475;
            break;
        }
        case 16:
        {
            _x[0]=     0.0950124906;    _w[0]=      0.189450611;
            _x[1]=      0.281603579;    _w[1]=      0.182603412;
            _x[2]=    -0.0950125163;    _w[2]=       0.18945061;
            _x[3]=      0.458016758;    _w[3]=      0.169156523;
            _x[4]=     -0.281603522;    _w[4]=      0.182603418;
            _x[5]=      0.617876255;    _w[5]=      0.149595986;
            _x[6]=     -0.458016801;    _w[6]=      0.169156515;
            _x[7]=      0.755404389;    _w[7]=       0.12462898;
            _x[8]=     -0.617876229;    _w[8]=      0.149595993;
            _x[9]=      0.865631222;    _w[9]=     0.0951584986;
            _x[10]=    -0.755404417;    _w[10]=     0.124628967;
            _x[11]=     0.944575009;    _w[11]=    0.0622535388;
            _x[12]=    -0.865631198;    _w[12]=    0.0951585143;
            _x[13]=      0.98940093;    _w[13]=    0.0271524729;
            _x[14]=    -0.989400935;    _w[14]=    0.0271524594;
            _x[15]=    -0.944575023;    _w[15]=    0.0622535239;
            break;
        }
        case 17:
        {
            _x[0]=    -0.990575475;    _w[0]=    0.0241483029;
            _x[1]=    -0.950675522;    _w[1]=    0.0554595294;
            _x[2]=    -0.880239153;    _w[2]=    0.0850361489;
            _x[3]=    -0.781514015;    _w[3]=     0.111883842;
            _x[4]=    -0.657671136;    _w[4]=     0.135136376;
            _x[5]=    -0.512690576;    _w[5]=     0.154045753;
            _x[6]=    -0.351231705;    _w[6]=      0.16800411;
            _x[7]=    -0.178484261;    _w[7]=       0.1765627;
            _x[8]=     0.000000000;    _w[8]=      0.17944647;
            _x[9]=     0.178484069;    _w[9]=     0.176562713;
            _x[10]=    0.351231883;    _w[10]=    0.168004086;
            _x[11]=    0.512690419;    _w[11]=    0.154045786;
            _x[12]=    0.657671267;    _w[12]=    0.135136335;
            _x[13]=    0.781513916;    _w[13]=    0.111883887;
            _x[14]=    0.880239216;    _w[14]=   0.0850361068;
            _x[15]=    0.950675487;    _w[15]=   0.0554595673;
            _x[16]=    0.990575486;    _w[16]=   0.0241482764;
            break;
        }

        default:
        {
            break;
        }
    }
}


void testLegendrePolynomials()
{
/*    GaussQuadrature gq;
    int n=3;
    std::vector<double> a(n+1, 0);

    Legendre(n, a.data());
    qDebug()<<a[10]<<a[9]<<a[8]<<a[7]<<a[6]<<a[5]<<a[4]<<a[3]<<a[2]<<a[1]<<a[0];*/
}


void testPolynomial()
{
    GaussQuadrature gq;
    int n=4;
    std::vector<double> p(n+1, 0);

    p[0] = 5;
    p[1] =-2;
    p[2] = 3;
    p[3] = -1;
    for(int i=0; i<=10; i++)
    {
        double t = double(i)/10.0;
        std::cout << QString::asprintf("%13.7f  %13.7f  %13.7f  ", t, gq.polynomial(n, p.data(), t), gq.polynomialDerivative(n, p.data(), t)).toStdString() << std::endl;
    }
}


void testQuadrature(int n)
{
    GaussQuadrature gq;
    gq.makeCoefficients(n);
    std::cout << QString::asprintf("Sum sin(x)cos(x) [0, PI/2] = %g", gq.quadrature(func, 0, PI/2)).toStdString() << std::endl;
}


double func(double x)
{
    return cos(3*x)*sin(x);
}


double func2(double x, double y)
{
    return x*cos(y);
}



