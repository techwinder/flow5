/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QString>


#include <QDataStream>


#include <api/foil.h>

#include <api/bspline.h>
#include <api/constants.h>
#include <api/geom_global.h>
#include <api/mathelem.h>
#include <api/node2d.h>
#include <api/utils.h>


#define MIDPOINTCOUNT 103


Foil::Foil()
{
    resetFoil();
}


Foil::Foil(const Foil *pFoil)
{
    if(pFoil) copy(pFoil);
}


Foil::Foil(Spline const *pSpline)
{
    resetFoil();
    for (int k=0;k<pSpline->outputSize(); k++)
    {
        appendBasePoint(pSpline->outputPt(k).x, pSpline->outputPt(k).y);
    }
    initGeometry();
}


void Foil::resetFoil()
{
    m_theStyle.m_Color      = fl5Color(155,155,155);
    m_theStyle.m_Symbol     = Line::NOSYMBOL; //no points to start with
    m_theStyle.m_Stipple    = Line::SOLID;
    m_theStyle.m_Width      = 2;
    m_theStyle.m_bIsVisible = true;

    m_iSelectedPt = -1;
    m_iLE = -1;
    m_bFill = false;


    m_BunchAmp = 0.0;
    m_BunchType = Spline::DOUBLESIG; // All other options are disabled
    m_CubicSpline.setBunchParameters(m_BunchType, m_BunchAmp);
    m_CubicSpline.setColor(fl5Color(255,100,100));

    m_bCamberLine   = false;

    m_Name = "";
    m_Description = "";

    clearPointArrays();

    m_Top.clear();
    m_Bot.clear();
    m_BaseTop.clear();
    m_BaseBot.clear();

    m_bTEFlap     = false;
    m_TEFlapAngle = 0.0;
    m_TEXHinge    = 0.7;
    m_TEYHinge    = 0.5;

    m_bLEFlap     = false;
    m_LEFlapAngle = 0.0;
    m_LEXHinge    = 0.2;
    m_LEYHinge    = 0.5;

    m_CSfracLE = 0.0;

}


void Foil::setDefaultMidLinePointDistribution(int nMidPoints)
{
    // reset the mid line point positions
    std::vector<double> frac;
    xfl::getPointDistribution(frac, nMidPoints-1, xfl::COSINE);
    m_BaseCbLine.resize(nMidPoints);
    m_Thickness.resize(nMidPoints);
    for(int i=0; i<nMidPoints; i++)
    {
        m_BaseCbLine[i].x = frac.at(i);
    }
}


void Foil::makeBaseMidLine()
{
    Vector2d U = (m_TE-m_LE).normalized();
    Vector2d N(-U.y, U.x);
    Vector2d Top, Bot;

    setDefaultMidLinePointDistribution(MIDPOINTCOUNT);

    m_BaseCbLine.front() = m_LE;
    m_Thickness.front() = 0.0;

    for (uint l=1; l<m_BaseCbLine.size(); l++)
    {
        Vector2d pt = m_LE + U *m_BaseCbLine.at(l).x;

        m_CubicSpline.getPoint(true,  m_CSfracLE, pt, pt+N, Top);
        m_CubicSpline.getPoint(false, m_CSfracLE, pt, pt-N, Bot);

        // update values
        m_BaseCbLine[l] = (Top+Bot)/2.0;
        m_Thickness[l] = (Top-Bot).norm();
    }

    // force LE position
    m_BaseCbLine.front() = m_LE;

}


double Foil::camber(double x) const
{
    if (x <= m_BaseCbLine.front().x)
    {
        return m_BaseCbLine.front().y;
    }
    for (uint i=0; i<m_BaseCbLine.size()-1; i++)
    {
        if ((m_BaseCbLine.at(i).x <= x) && (x < m_BaseCbLine.at(i+1).x))
        {
            double dx = x-m_BaseCbLine.at(i).x;
            return m_BaseCbLine.at(i).y + dx * (m_BaseCbLine.at(i+1).y-m_BaseCbLine.at(i).y)
                                             / (m_BaseCbLine.at(i+1).x-m_BaseCbLine.at(i).x);
        }
    }
    return m_BaseCbLine.back().y;
}


/**
 * Returns the camber value at a specified chord position, in normalized units.
 * @param &x the chordwise position
 * @return the camber value
 */
double Foil::thickness(double x) const
{
    if (x <= m_BaseCbLine.front().x)
    {
        return m_Thickness.front();
    }

    for (uint i=0; i<m_Thickness.size()-1; i++)
    {
        if ((m_BaseCbLine.at(i).x<=x) && (x<m_BaseCbLine.at(i+1).x))
        {
            double dx = x-m_BaseCbLine.at(i).x;
            return m_Thickness.at(i) + dx * (m_Thickness.at(i+1)-m_Thickness.at(i)) / (m_BaseCbLine.at(i+1).x-m_BaseCbLine.at(i).x);
        }
    }
    return  m_Thickness.back();
}


/**
 * Returns the camber angle in degrees at a specified chord position.
 * @param &x the chordwise position
 * @return the camber angle, in degrees
 */
double Foil::camberSlope(double x) const
{
    //returns the camber slope at position x
    for (uint i=0; i<m_BaseCbLine.size()-1; i++)
    {
        if ((m_BaseCbLine[i].x <= x) && (x < m_BaseCbLine[i+1].x))
        {
            double dx = m_BaseCbLine[i+1].x-m_BaseCbLine[i].x;
            double dy = m_BaseCbLine[i+1].y-m_BaseCbLine[i].y;
            return atan2(dy,dx);
        }
    }
    if(x>=1.0)
    {
        double dx = m_BaseCbLine.back().x-m_BaseCbLine[m_BaseCbLine.size()-2].x;
        double dy = m_BaseCbLine.back().y-m_BaseCbLine[m_BaseCbLine.size()-2].y;
        return atan2(dy,dx);
    }
    return 0.0;
}


double Foil::maxCamber() const
{
    double Camber = 0.0;
    for (uint l=1; l<m_BaseCbLine.size(); l++)
    {
        if(fabs(m_BaseCbLine.at(l).y)>fabs(Camber))
        {
            Camber  = m_BaseCbLine.at(l).y;
        }
    }
    return Camber;
}


double Foil::xCamber() const
{
    double camb = 0.0;
    double XCamber = 0.0;
    for (uint l=1; l<m_BaseCbLine.size(); l++)
    {
        if(fabs(m_BaseCbLine.at(l).y)>fabs(camb))
        {
            XCamber = m_BaseCbLine.at(l).x;
            camb = m_BaseCbLine.at(l).y;
        }
    }
    return XCamber;
}


double Foil::maxThickness() const
{
    double thick = 0.0;
    for(uint i=0; i<m_Thickness.size(); i++)
    {
        if(fabs(m_Thickness[i])>thick) thick = m_Thickness.at(i);
    }
    return thick;
}


double Foil::xThickness() const
{
    double xthick = 0.0;
    double thick = 0.0;
    for(uint i=0; i<m_Thickness.size(); i++)
    {
        if(fabs(m_Thickness.at(i))>thick)
        {
            thick = fabs(m_Thickness.at(i));
            xthick = m_BaseCbLine.at(i).x;
        }
    }
    return xthick;
}


void Foil::setThickness(double xthick, double thick)
{
    std::vector<double> newth = m_Thickness; // the new thickness profile
    double t0 = maxThickness();
    double xt0 = xThickness();
    double tr = thick/t0;
    double xtr = xthick/xt0;

    if(fabs(xtr-1.0)>0.001)
    {
        for(uint i=0; i<newth.size(); i++)
        {
            double xc = m_BaseCbLine[i].x;
            if(xc<=xthick)
            {
                // make a homothety with origin at the LE
                xtr = xthick/xt0;
                double xp = xc/xtr;
                newth[i] = thickness(xp);
            }
            else
            {
                // make a homothety with origin at the TE
                xtr = (xthick-1.0)/(xt0-1.0);
                double xp = m_BaseCbLine.back().x + (xc - m_BaseCbLine.back().x) / xtr;
                newth[i] = thickness(xp);
            }
        }
    }

    //scale the thickness
    if(fabs(tr-1.0)>0.001)
    {
        for(uint i=0; i<newth.size(); i++)
        {
            newth[i] *= tr;
        }
    }

    m_Thickness = newth;
}


void Foil::setCamber(double xcamb, double camb)
{
    std::vector<Node2d> newc = m_BaseCbLine; // the new camber line
    double c0 = maxCamber();
    double xc0 = xCamber();
    double cr = camb/c0;
    double xcr = xcamb/xc0;

    if(fabs(xcr-1.0)>0.001)
    {
        for(uint i=0; i<newc.size(); i++)
        {
            if(newc[i].x<=xcamb)
            {
                // make a homothety with origin at the LE
                xcr = xcamb/xc0;
                double xp = newc[i].x/xcr;
                newc[i].y = camber(xp);
            }
            else
            {
                // make a homothety with origin at the TE
                xcr = (xcamb-1.0)/(xc0-1.0);
                double xp = newc.back().x + (newc[i].x - newc.back().x) / xcr;
                newc[i].y = camber(xp);
            }
        }
    }

    //scale the camber
    if(fabs(cr-1.0)>0.001)
    {
        for(uint i=0; i<newc.size(); i++)
        {
            newc[i].y *= cr;
        }
    }

    m_BaseCbLine = newc;
}


void Foil::makeBaseFromCamberAndThickness()
{
    double c{0},t{0};

    m_BaseTop.resize(m_BaseCbLine.size());
    m_BaseBot.resize(m_BaseCbLine.size());

    for(uint i=0; i<m_BaseCbLine.size(); i++)
    {
        c = camber(m_BaseCbLine.at(i).x);
        t = thickness(m_BaseCbLine.at(i).x);
        m_BaseTop[i].x = m_BaseCbLine.at(i).x;
        m_BaseTop[i].y = c + 0.5*t;
    }
    for(uint i=0; i<m_BaseCbLine.size(); i++)
    {
        c = camber(m_BaseCbLine.at(i).x);
        t = thickness(m_BaseCbLine.at(i).x);
        m_BaseBot[i].x = m_BaseCbLine.at(i).x;
        m_BaseBot[i].y = c - 0.5*t;
    }
}


void Foil::rebuildPointSequenceFromBase()
{
    // rebuild the current foil from the top and bottom surfaces
    int newsize = int(m_BaseTop.size() + m_BaseBot.size()-1);
    if(newsize<=0) return; //safeguard

    m_BaseNode.resize(newsize);

    int topsize = int(m_BaseTop.size());
    for (int i=topsize-1; i>=0; i--)
    {
        m_BaseNode[topsize-1-i].x = m_BaseTop.at(i).x;
        m_BaseNode[topsize-1-i].y = m_BaseTop.at(i).y;
    }

    for (uint i=0; i<m_BaseBot.size(); i++)
    {
        m_BaseNode[topsize+i-1].x = m_BaseBot.at(i).x;
        m_BaseNode[topsize+i-1].y = m_BaseBot.at(i).y;
    }
}


void Foil::applyBase()
{
    m_Top = m_BaseTop;
    m_Bot = m_BaseBot;
    m_Node = m_BaseNode;
    m_CbLine = m_BaseCbLine;

    if(m_bLEFlap && fabs(m_LEFlapAngle)>FLAPANGLEPRECISION)
    {
        // the L.E., unlike the T.E. is considered to be part of the base geometry
        setLEFlap();

        // rebuild the current foil from the flapped top and bottom sides
        int newSize = int(m_Top.size() + m_Bot.size()-1);
        m_Node.resize(newSize);

        int sizeExt = int(m_Top.size());
        for (int i=sizeExt-1; i>=0; i--)
        {
            m_Node[sizeExt-1-i].x = m_Top.at(i).x;
            m_Node[sizeExt-1-i].y = m_Top.at(i).y;
        }

        for (uint i=0; i<m_Bot.size(); i++)
        {
            m_Node[sizeExt+i-1].x = m_Bot.at(i).x;
            m_Node[sizeExt+i-1].y = m_Bot.at(i).y;
        }
    }
}


/** v7.50: option added to modify the base foil to include the deflected flaps */
void Foil::makeModPermanent()
{
    m_bTEFlap = m_bLEFlap = false;
    m_TEFlapAngle = m_LEFlapAngle = 0.0;

    m_BaseNode = m_Node;

    // redundant with initFoilGeometry
    m_BaseTop = m_Top;
    m_BaseBot = m_Bot;
    m_BaseCbLine = m_CbLine;

}

double Foil::TEGap() const
{
    if(m_Node.size()==0) return 0;
    double gap = sqrt(  (m_Node.front().x-m_Node.back().x)*(m_Node.front().x-m_Node.back().x)
                      + (m_Node.front().y-m_Node.back().y)*(m_Node.front().y-m_Node.back().y));
    if(m_Node.front().y-m_Node.back().y>=0.0)
        return gap;
    else
        return -gap; // incorrectly constructed foil with crossover
}


void Foil::copy(Foil const *pSrcFoil, bool bMeta)
{
    if(pSrcFoil) copy(*pSrcFoil, bMeta);
}


void Foil::copy(Foil const &SrcFoil, bool bMeta, bool bForceDeepCopy)
{
    if(bMeta)
    {
        m_Name = SrcFoil.name();
        m_theStyle = SrcFoil.m_theStyle;
    }

    m_BaseNode = SrcFoil.m_BaseNode;
    m_Node = SrcFoil.m_Node;

    m_BaseTop = SrcFoil.m_BaseTop;
    m_BaseBot = SrcFoil.m_BaseBot;

    m_BaseCbLine = SrcFoil.m_BaseCbLine;
    m_Thickness  = SrcFoil.m_Thickness;

    m_CbLine  = SrcFoil.m_CbLine;
    m_Top   = SrcFoil.m_Top;
    m_Bot   = SrcFoil.m_Bot;

    if(bForceDeepCopy)
    {
        // force deep copies for future multithreaded use
/*        m_BaseNode.detach();
        m_Node.detach();
        m_BaseTop.detach();
        m_BaseBot.detach();
        m_BaseCbLine.detach();
        m_Thickness.detach();
        m_CbLine.detach();
        m_Top.detach();
        m_Bot.detach();*/
    }

    m_TE = SrcFoil.m_TE;
    m_LE = SrcFoil.m_LE;

    m_bLEFlap     = SrcFoil.m_bLEFlap;
    m_LEFlapAngle = SrcFoil.m_LEFlapAngle;
    m_LEXHinge    = SrcFoil.m_LEXHinge;
    m_LEYHinge    = SrcFoil.m_LEYHinge;

    m_bTEFlap     = SrcFoil.m_bTEFlap;
    m_TEFlapAngle = SrcFoil.m_TEFlapAngle;
    m_TEXHinge    = SrcFoil.m_TEXHinge;
    m_TEYHinge    = SrcFoil.m_TEYHinge;

    m_bCamberLine = SrcFoil.m_bCamberLine;

    m_CubicSpline = SrcFoil.m_CubicSpline;
    m_CSfracLE    = SrcFoil.m_CSfracLE;

    m_iLE         = SrcFoil.m_iLE;

    m_BunchAmp    = SrcFoil.m_BunchAmp;
    m_BunchType   = SrcFoil.m_BunchType;
}


double Foil::midLineAngle() const
{
    double angle = atan2(m_TE.y-m_LE.y, m_TE.x-m_LE.x);
    return angle * 180.0/PI;
}


double Foil::deRotate()
{
    // first translate the leading edge to the origin point
    for (int i=0; i<nBaseNodes(); i++)
    {
        m_BaseNode[i].x -= m_LE.x;
        m_BaseNode[i].y -= m_LE.y;
    }

    for (int i=0; i<nNodes(); i++)
    {
        m_Node[i].x -= m_LE.x;
        m_Node[i].y -= m_LE.y;
    }

    m_TE.x-= m_LE.x;
    m_TE.y-= m_LE.y;
    m_LE.set(0.0,0.0);

    // find current angle
    double angle = atan2(m_TE.y-m_LE.y, m_TE.x-m_LE.x);

    //rotate about the L.E.
    double cosa = cos(-angle);
    double sina = sin(-angle);

    double xr(0), yr(0);
    for (int i=0; i<nBaseNodes(); i++)
    {
        xr = xb(i)*cosa - yb(i)*sina;
        yr = xb(i)*sina + yb(i)*cosa;
        m_BaseNode[i].x = xr;
        m_BaseNode[i].y = yr;
    }

    for (int i=0; i<nNodes(); i++)
    {
        xr = m_Node.at(i).x*cosa - m_Node.at(i).y*sina;
        yr = m_Node.at(i).x*sina + m_Node.at(i).y*cosa;
        m_Node[i].x = xr;
        m_Node[i].y = yr;
    }

    xr = m_TE.x*cosa - m_TE.y*sina;
    yr = m_TE.x*sina +m_TE.y*cosa;
    m_TE.x = xr;
    m_TE.y = yr;

    return angle*180.0/PI;
}


/**
 * Exports the foil geometry to a text .dat file.
 * @param out the stream to which the output will be directed
 * @return true if the operation has been successful, false otherwise
 */
bool Foil::exportFoilToDat(std::string &out) const
{
    QString strOut;

    out = m_Name +"\n";

    for (int i=0; i<nNodes(); i++)
    {
        strOut = QString::asprintf(" %11.7f    %11.7f\n", m_Node.at(i).x, m_Node.at(i).y);
        out += strOut.toStdString();
    }

    return true;
}


/**
 * Returns the area defined by the foil's contour, in normalized units.
 * @return the foil's internal area
 */
double Foil::area() const
{
    double area = 0.0;
    for (int i=0; i<nBaseNodes()-1; i++)
    {
        area +=  fabs((yb(i+1)+yb(i))/2.0 * (xb(i+1)-xb(i)));
    }
    return area;
}



/**
 * Returns the y-position on the lower surface, at a specified chord position.
 * @param &x the chordwise position
 * @return the y-position
 */
double Foil::baseLowerY(double x) const
{
    double y;

    x = m_BaseBot.front().x + x*(m_BaseBot.back().x-m_BaseBot.front().x);//in case there is a flap which reduces the length

    for (uint i=0; i<m_BaseBot.size()-1; i++)
    {
        if (m_BaseBot.at(i).x <m_BaseBot.at(i+1).x  &&  m_BaseBot.at(i).x <= x && x<=m_BaseBot.at(i+1).x )
        {
            y = (m_BaseBot.at(i).y   +(m_BaseBot.at(i+1).y-m_BaseBot.at(i).y)
                 /(m_BaseBot.at(i+1).x - m_BaseBot.at(i).x) * (x-m_BaseBot.at(i).x));
            return y;
        }
    }
    return 0.0;
}



/**
 * Returns the y-position on the upper surface, at a specified chord position
 * @param &x the chordwise position
 * @return the y-position
 */
double Foil::baseUpperY(double x) const
{
    double y;

    x = m_BaseTop.front().x + x*(m_BaseTop.back().x-m_BaseTop.front().x);//in case there is a flap which reduces the length

    for (uint i=0; i<m_BaseTop.size()-1; i++)
    {
        if (m_BaseTop.at(i).x <m_BaseTop.at(i+1).x  &&  m_BaseTop.at(i).x <= x && x<=m_BaseTop.at(i+1).x )
        {
            y = (m_BaseTop.at(i).y   +(m_BaseTop.at(i+1).y-m_BaseTop.at(i).y)
                 /(m_BaseTop.at(i+1).x - m_BaseTop.at(i).x) * (x-m_BaseTop.at(i).x));
            return y;
        }
    }
    return 0.0;
}


/**
 * Returns the slope in radians on the lower surface, at a specified chord position.
 * @param &x the chordwise position
 * @return the slope in radians
 */
double Foil::bottomSlope(double x) const
{
    for (uint i=0; i<m_Bot.size(); i++)
    {
        if ((m_Bot[i].x <= x) && (x < m_Bot[i+1].x))
        {
            double dx = m_Bot[i+1].x-m_Bot[i].x;
            double dy = m_Bot[i+1].y-m_Bot[i].y;
            return -atan2(dy,dx);
        }
    }
    return 0.0;
}


/**
 * Returns the slope in radians on the upperer surface, at a specified chord position.
 * @param &x the chordwise position
 * @return the slope in radians
 */
double Foil::topSlope(double x) const
{
    //returns the upper slope at position x
    for (uint i=0; i<m_Top.size(); i++)
    {
        if ((m_Top[i].x <= x) && (x < m_Top[i+1].x))
        {
            double dx = m_Top[i+1].x-m_Top[i].x;
            double dy = m_Top[i+1].y-m_Top[i].y;
            return -atan2(dy,dx);
        }
    }
    return 0.0;
}


/**
 * Returns the foil's length.
 * @return the foil's length, in relative units
*/
double Foil::length() const
{
    return  (m_LE-m_TE).norm();
//    return std::max(m_BaseTop.back().x-m_BaseTop.front().x, m_BaseBot.back().x-m_BaseBot.front().x);
}


/**
* Returns the y-coordinate on the current foil's mid line at the x position.
* @param x the chordwise position
* @return the position on the mid line
*/
Vector2d Foil::midYRel(double sRel, Vector2d &N) const
{
    if(!m_CbLine.size()) {N.x = N.y = 0.0; return Vector2d();}

    double x = m_CbLine.front().x + sRel*(m_CbLine.back().x-m_CbLine.front().x);

    if(x<=m_CbLine.front().x)
    {
        int i=0;
        double nabs = sqrt(  (m_CbLine.at(i+1).x-m_CbLine.at(i).x) * (m_CbLine.at(i+1).x-m_CbLine.at(i).x)
                           + (m_CbLine.at(i+1).y-m_CbLine.at(i).y) * (m_CbLine.at(i+1).y-m_CbLine.at(i).y));
        N.x = (-m_CbLine.at(i+1).y + m_CbLine.at(i).y)/nabs;
        N.y = ( m_CbLine.at(i+1).x - m_CbLine.at(i).x)/nabs;

        return m_CbLine.front();
    }
    else if (x>=m_CbLine.back().x)
    {
        int i = int(m_CbLine.size()-2);
        double nabs = sqrt(  (m_CbLine.at(i+1).x-m_CbLine.at(i).x) * (m_CbLine.at(i+1).x-m_CbLine.at(i).x)
                           + (m_CbLine.at(i+1).y-m_CbLine.at(i).y) * (m_CbLine.at(i+1).y-m_CbLine.at(i).y));
        N.x = (-m_CbLine.at(i+1).y + m_CbLine.at(i).y)/nabs;
        N.y = ( m_CbLine.at(i+1).x - m_CbLine.at(i).x)/nabs;
        return m_CbLine.back();
    }

    for (uint i=0; i<m_CbLine.size()-1; i++)
    {
        if (m_CbLine.at(i).x < m_CbLine.at(i+1).x && m_CbLine.at(i).x <= x && x<=m_CbLine.at(i+1).x )
        {
            double nabs = sqrt(  (m_CbLine.at(i+1).x-m_CbLine.at(i).x) * (m_CbLine.at(i+1).x-m_CbLine.at(i).x)
                               + (m_CbLine.at(i+1).y-m_CbLine.at(i).y) * (m_CbLine.at(i+1).y-m_CbLine.at(i).y));
            N.x = (-m_CbLine.at(i+1).y + m_CbLine.at(i).y)/nabs;
            N.y = ( m_CbLine.at(i+1).x - m_CbLine.at(i).x)/nabs;
            return m_CbLine.at(i) + (m_CbLine.at(i+1)-m_CbLine.at(i)) /(m_CbLine.at(i+1).x-m_CbLine.at(i).x) * (x-m_CbLine.at(i).x);
        }
    }
    assert(true); // should never get here

    return m_CbLine.back();
}


/**
* Returns the y-coordinate on the current foil's upper surface at the x position.
* @param x the chordwise position
* @return the position on the upper surface
*/
Vector2d Foil::upperYRel(double xRel, Vector2d &N) const
{
    if(!m_Top.size()) return Vector2d();
    double x = m_Top.front().x + xRel*(m_Top.back().x-m_Top.front().x);

    if(x<=m_Top.front().x)
    {
        int i= 0;
        double nabs = sqrt(  (m_Top.at(i+1).x-m_Top.at(i).x) * (m_Top.at(i+1).x-m_Top.at(i).x)
                           + (m_Top.at(i+1).y-m_Top.at(i).y) * (m_Top.at(i+1).y-m_Top.at(i).y));
        N.x = (-m_Top.at(i+1).y + m_Top.at(i).y)/nabs;
        N.y = ( m_Top.at(i+1).x - m_Top.at(i).x)/nabs;

        return m_Top.front();
    }
    else if (x>=m_Top.back().x)
    {
        int i = int(m_Top.size()-2);
        double nabs = sqrt(  (m_Top.at(i+1).x-m_Top.at(i).x) * (m_Top.at(i+1).x-m_Top.at(i).x)
                           + (m_Top.at(i+1).y-m_Top.at(i).y) * (m_Top.at(i+1).y-m_Top.at(i).y));
        N.x = (-m_Top.at(i+1).y + m_Top.at(i).y)/nabs;
        N.y = ( m_Top.at(i+1).x - m_Top.at(i).x)/nabs;

        return m_Top.back();
    }


    for (uint i=0; i<m_Top.size()-1; i++)
    {
        if (m_Top.at(i).x < m_Top.at(i+1).x && m_Top.at(i).x <= x && x<=m_Top.at(i+1).x )
        {
            double nabs = sqrt(  (m_Top.at(i+1).x-m_Top.at(i).x) * (m_Top.at(i+1).x-m_Top.at(i).x)
                               + (m_Top.at(i+1).y-m_Top.at(i).y) * (m_Top.at(i+1).y-m_Top.at(i).y));
            N.x = (-m_Top.at(i+1).y + m_Top.at(i).y)/nabs;
            N.y = ( m_Top.at(i+1).x - m_Top.at(i).x)/nabs;
            return m_Top.at(i) + (m_Top.at(i+1)-m_Top.at(i)) /(m_Top.at(i+1).x-m_Top.at(i).x) * (x-m_Top.at(i).x);
        }
    }
    assert(true); // should never get here
    N.x = 1.0;
    N.y = 0.0;
    return m_Top.back();
}



/**
* Returns the y-coordinate on the current foil's lower surface at the x position.
* @param x the chordwise position
* @return the position on the upper surface
*/
Vector2d Foil::lowerYRel(double xRel, Vector2d &N) const
{
    if(!m_Bot.size()) return Vector2d();

    double x = m_Bot.front().x + xRel*(m_Bot.back().x-m_Bot.front().x);

    if(x<=m_Bot.front().x)
    {
        int i=0;
        double nabs = sqrt((m_Bot.at(i+1).x-m_Bot.at(i).x) * (m_Bot.at(i+1).x-m_Bot.at(i).x)
                           + (m_Bot.at(i+1).y-m_Bot.at(i).y) * (m_Bot.at(i+1).y-m_Bot.at(i).y));
        N.x = ( m_Bot.at(i+1).y - m_Bot.at(i).y)/nabs;
        N.y = (-m_Bot.at(i+1).x + m_Bot.at(i).x)/nabs;

        return m_Bot.front();
    }
    else if(x>=m_Bot.back().x)
    {
        int i=int(m_Bot.size()-2);
        double nabs = sqrt((m_Bot.at(i+1).x-m_Bot.at(i).x) * (m_Bot.at(i+1).x-m_Bot.at(i).x)
                           + (m_Bot.at(i+1).y-m_Bot.at(i).y) * (m_Bot.at(i+1).y-m_Bot.at(i).y));
        N.x = ( m_Bot.at(i+1).y - m_Bot.at(i).y)/nabs;
        N.y = (-m_Bot.at(i+1).x + m_Bot.at(i).x)/nabs;

        return m_Bot.back();
    }

    for(uint i=0; i<m_Bot.size()-1; i++)
    {
        if (m_Bot.at(i).x<m_Bot.at(i+1).x && m_Bot.at(i).x<= x && x<=m_Bot.at(i+1).x )
        {
            double nabs = sqrt((m_Bot.at(i+1).x-m_Bot.at(i).x) * (m_Bot.at(i+1).x-m_Bot.at(i).x)
                               + (m_Bot.at(i+1).y-m_Bot.at(i).y) * (m_Bot.at(i+1).y-m_Bot.at(i).y));
            N.x = ( m_Bot.at(i+1).y - m_Bot.at(i).y)/nabs;
            N.y = (-m_Bot.at(i+1).x + m_Bot.at(i).x)/nabs;
            return (m_Bot.at(i) + (m_Bot.at(i+1)-m_Bot.at(i))
                    /(m_Bot.at(i+1).x-m_Bot.at(i).x) * (x-m_Bot.at(i).x));
        }
    }

    assert(true); //should never get here
    N.x = 1.0;
    N.y = 0.0;
    return m_Bot.back();
}


void Foil::getY(std::vector<Vector2d> const &vec, double x, double &y) const
{
//    x = vec.front().x + x*(vec.back().x-vec.front().x);

    if(x<=vec.front().x)
    {
        y = vec.front().y;
        return;
    }

    for(uint i=0; i<vec.size()-1; i++)
    {
        if (vec.at(i).x <vec.at(i+1).x  &&  vec.at(i).x<=x && x<=vec.at(i+1).x )
        {
            y = (vec.at(i).y + (vec.at(i+1).y-vec.at(i).y) /(vec.at(i+1).x-vec.at(i).x) * (x-vec.at(i).x));

            return;
        }
    }

    y = vec.back().y;
}


/** XFoil method: LE is the point such that the local tangent is
 *  perpendicular to the line linking LE to TE */
void Foil::setLEFromCubicSpline()
{
    // find the LE by dichotomy
    double dx{0}, dy{0}, tx{0}, ty{0}, nt{0}, nd{0}, sc{0}, s0{0}, s1{0};

    const double precision = 1.0e-5;

    Vector2d ps;

    double t0 = 0.;
    int iter{0};
    do
    {
        t0+=0.05;
        ps = m_CubicSpline.splinePoint(t0);
        iter++;
    }
    while(ps.x>0.15 && iter<10);

    iter=0;
    double t1 = 1.0;
    do
    {
        t1 -=0.05;
        ps = m_CubicSpline.splinePoint(t1);
        iter++;
    }
    while(ps.x>0.15 && iter<10);

    ps = m_CubicSpline.splinePoint(t0);
    //make normalized vector from TE to point
    tx = m_TE.x - ps.x;
    ty = m_TE.y - ps.y;
    nt = sqrt(tx*tx+ty*ty);
    tx *= 1.0/nt;
    ty *= 1.0/nt;
    m_CubicSpline.splineDerivative(t0, dx, dy);
    nd = sqrt(dx*dx+dy*dy);
    dx *= 1.0/nd;
    dy *= 1.0/nd;
    s0 = tx*dx + ty*dy;

    ps = m_CubicSpline.splinePoint(t1);
    //make normalized vector from TE to point
    tx = m_TE.x - ps.x;
    ty = m_TE.y - ps.y;
    nt = sqrt(tx*tx+ty*ty);
    tx *= 1.0/nt;
    ty *= 1.0/nt;
    m_CubicSpline.splineDerivative(t1, dx, dy);
    nd = sqrt(dx*dx+dy*dy);
    dx *= 1.0/nd;
    dy *= 1.0/nd;
    s1 =  tx*dx + ty*dy;

    if(s1<s0) {double tmp=t1; t1=t0; t0=tmp;}

    double t=0.5;
    iter=0;
    do
    {
        t=(t0+t1)/2.0;
        ps = m_CubicSpline.splinePoint(t);
        //make normalized vector from TE to point
        tx = m_TE.x - ps.x;
        ty = m_TE.y - ps.y;
        nt = sqrt(tx*tx+ty*ty);
        tx *= 1.0/nt;
        ty *= 1.0/nt;
        m_CubicSpline.splineDerivative(t, dx, dy);
        nd = sqrt(dx*dx+dy*dy);
        dx *= 1.0/nd;
        dy *= 1.0/nd;
        sc = tx*dx + ty*dy;

        if(sc<0.0) t0=t;
        else       t1=t;

        if(fabs(t1-t0)<precision)  break;
        iter++;
    }while(iter<100);

    if(iter<100)
    {
        m_CSfracLE = (t0+t1)/2.0;
        m_LE = m_CubicSpline.splinePoint(m_CSfracLE);
        if(fabs(m_LE.y)<1.0e-4) m_LE.y = 0;
    }
    else
    {
        m_CSfracLE = 0.0;
        m_LE.set(0.0,0.0);
    }
}


bool Foil::makeApproxBSpline(BSpline &bs, int deg, int nCtrlPts, int nOutputPts) const
{
    bs.setOutputSize(nOutputPts);

/*    std::vector<Node2d> points(nBaseNodes());
    for(int i=0; i<nBaseNodes(); i++)
    {
        points[i].x = xb(i);
        points[i].y = yb(i);
    }*/

    return bs.approximate(deg, nCtrlPts, m_BaseNode);
}


bool Foil::makeTopBotSurfaces()
{
    m_BaseTop.clear();
    m_BaseBot.clear();

    // construct an array for the top side nodes and another for the bottom side nodes
    // these array are sorted from LE to TE

    // first count the number of nodes on top and bottom sides, including LE and TE
    // find the index of the node corresponding to the LE

    for(int i=1; i<nBaseNodes(); i++)
    {
        if(xb(i)>xb(i-1))
        {
            m_iLE = i-1;
            break;
        }
    }

    if(nBaseNodes()-m_iLE<=0) return false;
    if(m_iLE>=nBaseNodes())   return false;
    if(m_iLE<0)       return false;

    m_BaseTop.resize(m_iLE+1);
    m_BaseBot.resize(nBaseNodes() - m_iLE);

    // fill top and bottom arrays
    for(int i=int(m_iLE); i>=0; i--)
    {
        m_BaseTop[m_iLE-i].set(xb(i), yb(i));
    }
    for(int i=m_iLE; i<nBaseNodes(); i++)
    {
        m_BaseBot[i-m_iLE].set(xb(i), yb(i));
    }

    return true;
}


/**
* Normalizes the base foil's lengh to unity.
* The current foil's length is modified by the same ratio.
* @return the foil's former length
*/
double Foil::normalizeGeometry()
{
    double xmin =  LARGEVALUE;
    double xmax = -LARGEVALUE;

    for (int i=0; i<nBaseNodes(); i++)
    {
        xmin = std::min(xmin, m_BaseNode.at(i).x);
        xmax = std::max(xmax, m_BaseNode.at(i).x);
    }


    double length = xmax - xmin;

    //reset origin
    for (int i=0; i<nBaseNodes(); i++) m_BaseNode[i].x -= xmin;

    //set length to 1. and cancel y offset
    for(int i=0; i<nBaseNodes(); i++)
    {
        m_BaseNode[i].x = xb(i)/length;
        m_BaseNode[i].y = yb(i)/length;
    }
    double yTrans = yb(0);
    for(int i=0; i<nBaseNodes(); i++)    m_BaseNode[i].y -= yTrans;

    //reset origin
    for (int i=0; i<nNodes(); i++)
    {
        m_Node[i].x -= xmin;
    }

    //set length to 1. and cancel y offset
    for(int i=0; i<nNodes(); i++)
    {
        m_Node[i].x = m_Node.at(i).x/length;
        m_Node[i].y = m_Node.at(i).y/length;
    }
    yTrans = m_Node.at(0).y;
    for(int i=0; i<nNodes(); i++)
        m_Node[i].y -= yTrans;

    return length;
}


void Foil::scaleHingeLocations()
{
    m_LEXHinge /= 100.0;
    m_LEYHinge /= 100.0;
    m_TEXHinge /= 100.0;
    m_TEYHinge /= 100.0;
}


void Foil::setTEFlapData(bool bFlap, double xhinge, double yhinge, double angle)
{
    m_bTEFlap     = bFlap;
    m_TEXHinge    = xhinge;
    m_TEYHinge    = yhinge;
    m_TEFlapAngle = angle;
}


void Foil::setLEFlapData(bool bFlap, double xhinge, double yhinge, double angle)
{
    m_bLEFlap     = bFlap;
    m_LEXHinge    = xhinge;
    m_LEYHinge    = yhinge;
    m_LEFlapAngle = angle;
}


void Foil::setLEFlap()
{
    bool bIntersect{false};
    uint l{0}, p{0}, i1{0}, i2{0};
    double xh{0}, yh{0}, dx{0}, dy{0};
    Vector2d M;

    double cosa = cos(m_LEFlapAngle*PI/180.0);
    double sina = sin(m_LEFlapAngle*PI/180.0);
    //first convert xhinge and yhinge in absolute coordinates
    xh = m_LEXHinge;
    double ymin = baseLowerY(xh);
    double ymax = baseUpperY(xh);
    yh = ymin + m_LEYHinge * (ymax-ymin);

    // insert a breakpoint at xhinge location, if there isn't one already
    int iUpperh = 0;
    int iLowerh = 0;
    for (uint i=0; i<m_Top.size(); i++)
    {
        if(fabs(m_Top[i].x-xh)<0.001)
        {
            //then no need to add an extra point, just break
            iUpperh = i;
            break;
        }
        else if(m_Top[i].x>xh)
        {
            //insert one
/*            m_rpTop.push_back({});
            for(int j=m_rpTop.size(); j>i; j--)
            {
                m_rpTop[j].x = m_rpTop[j-1].x;
                m_rpTop[j].y = m_rpTop[j-1].y;
            }*/
//            m_Top.insert(i, {xh, ymax});
            m_Top.insert(m_Top.begin()+i, {xh, ymax});
/*            m_rpTop[i].x = xh;
            m_rpTop[i].y = ymax;*/
            iUpperh = i;
            break;
        }
    }

    for (uint i=0; i<m_Bot.size(); i++)
    {
        if(fabs(m_Bot[i].x-xh)<0.001)
        {
            //then no need to add an Intra point, just break
            iLowerh = i;
            break;
        }
        else if(m_Bot[i].x>xh)
        {
            //insert one
/*            m_rpTop.push_back(Vector2d(0.0,0.0));
            for(int j=m_rpBot.size(); j>i; j--)
            {
                m_rpBot[j].x = m_rpBot[j-1].x;
                m_rpBot[j].y = m_rpBot[j-1].y;
            }*/
//            m_Bot.insert(i, {xh,ymin});
            m_Bot.insert(m_Bot.begin()+i, {xh,ymin});
/*            m_rpBot[i].x = xh;
            m_rpBot[i].y = ymin;*/
            iLowerh = i;
            break;
        }
    }

    if(m_LEFlapAngle>FLAPANGLEPRECISION)
    {   //Flap is down
        //insert an extra point on intrados
/*        m_rpBot.push_back({});
        for (int i=m_rpBot.size(); i>iLowerh; i--)
        {
            m_rpBot[i] = m_rpBot[i-1];
        }
        m_rpBot[iLowerh] = m_rpBot[iLowerh+1];*/

//        m_Bot.insert(iLowerh, m_Bot.at(iLowerh));
        m_Bot.insert(m_Bot.begin()+iLowerh, m_Bot.at(iLowerh));

        iLowerh++;

        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_Bot[iLowerh-1].x += 30.0*(m_Bot[iLowerh-1].x-m_Bot[iLowerh-2].x);
        m_Bot[iLowerh-1].y += 30.0*(m_Bot[iLowerh-1].y-m_Bot[iLowerh-2].y);
        m_Bot[iLowerh].x   += 30.0*(m_Bot[iLowerh].x  - m_Bot[iLowerh+1].x);
        m_Bot[iLowerh].y   += 30.0*(m_Bot[iLowerh].y  - m_Bot[iLowerh+1].y);
    }

    if(m_LEFlapAngle<-FLAPANGLEPRECISION)
    {   //Flap is up
        //insert an extra point on extrados
/*        m_rpTop.push_back(Vector2d(0.0,0.0));
        for (int i= m_rpTop.size(); i>iUpperh; i--)
        {
            m_rpTop[i] = m_rpTop[i-1];
        }
        m_rpTop[iUpperh] = m_rpTop[iUpperh+1]; */

//        m_Top.insert(iUpperh, m_Top.at(iUpperh));
        m_Top.insert(m_Top.begin() + iUpperh, m_Top.at(iUpperh));

        iUpperh++;

        // extend to infinity last segments around hinge on flap external side to make sure
        // they intersect the spline on the other side
        m_Top[iUpperh-1].x += 30.0 * (m_Top[iUpperh-1].x - m_Top[iUpperh-2].x);
        m_Top[iUpperh-1].y += 30.0 * (m_Top[iUpperh-1].y - m_Top[iUpperh-2].y);
        m_Top[iUpperh].x   += 30.0 * (m_Top[iUpperh].x   - m_Top[iUpperh+1].x);
        m_Top[iUpperh].y   += 30.0 * (m_Top[iUpperh].y   - m_Top[iUpperh+1].y);
    }

    // rotate all points upstream of xh
    for (int i=0; i<iUpperh; i++)
    {
        dx = m_Top[i].x-xh;
        dy = m_Top[i].y-yh;
        m_Top[i].x = xh + cosa * dx - sina * dy;
        m_Top[i].y = yh + sina * dx + cosa * dy;
    }
    for (int i=0; i<iLowerh; i++)
    {
        dx = m_Bot[i].x-xh;
        dy = m_Bot[i].y-yh;
        m_Bot[i].x = xh + cosa * dx - sina * dy;
        m_Bot[i].y = yh + sina * dx + cosa * dy;
    }

    BSpline linkSpline;
    linkSpline.setOutputSize(4);
    linkSpline.setDegree(2);
    linkSpline.clearControlPoints();

    if(m_LEFlapAngle>FLAPANGLEPRECISION)
    {
        //define a 3 ctrl-pt spline to smooth the connection between foil and flap on bottom side
        geom::intersect(m_Bot.at(iLowerh-2), m_Bot.at(iLowerh-1),
                        m_Bot.at(iLowerh),   m_Bot.at(iLowerh+1), M);
        //sanity check
        if(M.x<=m_Bot.at(iLowerh-1).x || M.x>=m_Bot.at(iLowerh).x)
            M = (m_Bot.at(iLowerh-1) + m_Bot.at(iLowerh))/2.0;

        linkSpline.insertCtrlPoint(m_Bot.at(iLowerh-1).x,m_Bot.at(iLowerh-1).y);
        linkSpline.insertCtrlPoint(M.x, M.y);
        linkSpline.insertCtrlPoint(m_Bot.at(iLowerh).x,m_Bot.at(iLowerh).y);
        linkSpline.updateSpline();
        linkSpline.makeCurve();
        //retrieve point 1 and 2 and insert them
/*        m_rpBot.push_back({});
        m_rpBot.push_back({});
        for (int i=m_rpBot.size(); i>=iLowerh; i--)
        {
            m_rpBot[i+2].x = m_rpBot[i].x;
            m_rpBot[i+2].y = m_rpBot[i].y;
        }*/
//        m_Bot.insert(iLowerh, 2, {});
        m_Bot.insert(m_Bot.begin() + iLowerh, 2, {});

        m_Bot[iLowerh+1].x = linkSpline.outputPt(2).x;
        m_Bot[iLowerh+1].y = linkSpline.outputPt(2).y;
        m_Bot[iLowerh].x   = linkSpline.outputPt(1).x;
        m_Bot[iLowerh].y   = linkSpline.outputPt(1).y;
    }

    if(m_LEFlapAngle<-FLAPANGLEPRECISION)
    {
        //define a 3 ctrl-pt spline to smooth the connection between foil and flap on bottom side
        bIntersect = geom::intersect(m_Top.at(iUpperh-2), m_Top.at(iUpperh-1),
                                     m_Top.at(iUpperh),   m_Top.at(iUpperh+1), M);

        //sanity check
        if(M.x<=m_Top.at(iUpperh-1).x || M.x>=m_Top.at(iUpperh).x)
            M = (m_Top.at(iUpperh-1) + m_Top.at(iUpperh))/2.0;

        linkSpline.insertCtrlPoint(m_Top.at(iUpperh-1).x, m_Top.at(iUpperh-1).y);
        linkSpline.insertCtrlPoint(M.x, M.y);
        linkSpline.insertCtrlPoint(m_Top.at(iUpperh).x, m_Top.at(iUpperh).y);
        linkSpline.updateSpline();
        linkSpline.makeCurve();

        //retrieve point 1 and 2 and insert them
/*        m_rpTop.push_back({0,0});
        m_rpTop.push_back({0,0});
        for (int i=m_rpTop.size(); i>=iUpperh; i--)
        {
            m_rpTop[i+2].x = m_rpTop.at(i).x;
            m_rpTop[i+2].y = m_rpTop.at(i).y;
        } */

//        m_Top.insert(iUpperh, 2, {});
        m_Top.insert(m_Top.begin()+iUpperh, 2, {});

        m_Top[iUpperh+1].x = linkSpline.outputPt(2).x;
        m_Top[iUpperh+1].y = linkSpline.outputPt(2).y;
        m_Top[iUpperh].x   = linkSpline.outputPt(1).x;
        m_Top[iUpperh].y   = linkSpline.outputPt(1).y;

    }
    // trim upper surface first
    i1 = iUpperh;
    i2 = iUpperh-1;

    bIntersect = false;
    uint j=0;
    uint k=0;
    for (j=i2-1; j>0; j--)
    {
        for (k=i1; k<m_Top.size()-1; k++)
        {
            if(geom::intersect(m_Top.at(j), m_Top.at(j+1),
                               m_Top.at(k), m_Top.at(k+1), M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_Top[j+1].x = M.x;
        m_Top[j+1].y = M.y;
        p=1;
        for (l=k+1; l<m_Top.size(); l++)
        {
            m_Top[j+1+p]  = m_Top.at(l);
            p++;
        }
    }

    // trim lower surface next
    i1 = iLowerh;
    i2 = iLowerh-1;

    bIntersect = false;
    for (j=i2-1; j>0; j--)
    {
        for (k=i1; k<m_Bot.size()-1; k++)
        {
            if(geom::intersect(m_Bot.at(j), m_Bot.at(j+1),
                               m_Bot.at(k), m_Bot.at(k+1), M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_Bot[j+1].x = M.x;
        m_Bot[j+1].y = M.y;
        p=1;
        for (l=k+1;l<m_Bot.size(); l++)
        {
            m_Bot[j+1+p]  = m_Bot.at(l);
            p++;
        }
    }
}


Vector2d Foil::LEHinge() const
{
    double xh = m_LEXHinge;
    double ymin = baseLowerY(xh);
    double ymax = baseUpperY(xh);
    double yh = ymin + m_LEYHinge * (ymax-ymin);
    return Vector2d(xh, yh);
}


Vector2d Foil::TEHinge() const
{
    double xh = m_TEXHinge;
    double ymin = baseLowerY(xh);
    double ymax = baseUpperY(xh);
    double yh = ymin + m_TEYHinge * (ymax-ymin);
    return Vector2d(xh, yh);
}


void Foil::setTEFlap()
{
    uint i{0};
    int j{0}, k{0}, l{0}, p{0}, i1{0}, i2{0};

    bool bIntersect = false;

    Vector2d M = TEHinge();

    // insert a breakpoint at xhinge location, if there isn't one already
    int iUpperh = 0;
    int iLowerh = 0;
    for (uint i=0; i<m_Top.size(); i++)
    {
        if(fabs(m_Top.at(i).x-M.x)<0.001)
        {
            //then no need to add an extra point, just break
            iUpperh = i;
            break;
        }
        else if(m_Top.at(i).x>M.x)
        {
            iUpperh = i;
            break;
        }
    }

    for (i=0; i<m_Bot.size(); i++)
    {
        if(fabs(m_Bot.at(i).x-M.x)<0.001)
        {
            //then no need to add an extra point, just break
            iLowerh = i;
            break;
        }
        else if(m_Bot.at(i).x>M.x)
        {
            iLowerh = i;
            break;
        }
    }

    if(iUpperh<=0 || iUpperh>=int(m_Bot.size()))  return;
    if(iLowerh<=0 || iLowerh>=int(m_Bot.size()))  return;

    if(m_TEFlapAngle< -FLAPANGLEPRECISION)
    {
        //insert an extra point on top side
        m_Top.push_back(Vector2d(0.0,0.0));
        for (int i=int(m_Top.size()-1); i>iUpperh; i--)
        {
            m_Top[i] = m_Top[i-1];
        }

        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_Top[iUpperh+1].x += 30.0*(m_Top[iUpperh+1].x-m_Top[iUpperh+2].x);
        m_Top[iUpperh+1].y += 30.0*(m_Top[iUpperh+1].y-m_Top[iUpperh+2].y);
        m_Top[iUpperh].x   += 30.0 * (m_Top[iUpperh].x-m_Top[iUpperh-1].x);
        m_Top[iUpperh].y   += 30.0 * (m_Top[iUpperh].y-m_Top[iUpperh-1].y);
    }

    if(m_TEFlapAngle>FLAPANGLEPRECISION)
    {
        //insert an extra point on bottom side
        m_Bot.push_back(Vector2d(0.0,0.0));
        for (int i=int(m_Bot.size()-1); i>iLowerh; i--)
        {
            m_Bot[i] = m_Bot[i-1];
        }

        // extend to infinity last segments around hinge on flap internal side to make sure
        // they intersect the spline on the other side
        m_Bot[iLowerh+1].x += 30.0*(m_Bot[iLowerh+1].x - m_Bot[iLowerh+2].x);
        m_Bot[iLowerh+1].y += 30.0*(m_Bot[iLowerh+1].y - m_Bot[iLowerh+2].y);
        m_Bot[iLowerh].x   += 30.0*(m_Bot[iLowerh].x   - m_Bot[iLowerh-1].x);
        m_Bot[iLowerh].y   += 30.0*(m_Bot[iLowerh].y   - m_Bot[iLowerh-1].y);
    }

    // rotate all points downstream of xh
    for (i=iUpperh+1; i<m_Top.size(); i++)
    {
        m_Top[i].rotateZ(M, -m_TEFlapAngle);
    }
    for (i=iLowerh+1; i<m_Bot.size(); i++)
    {
        m_Bot[i].rotateZ(M, -m_TEFlapAngle);
    }


    // trim upper surface first
    i1 = iUpperh;
    i2 = iUpperh+1;

    bIntersect = false;

    k=0;
    for (j=i2; j<int(m_Top.size()-1); j++)
    {
        for (k=i1;k>0; k--)
        {
            if(geom::intersect(m_Top[j], m_Top[j+1], m_Top[k], m_Top[k-1], M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_Top[k] = M;
        p=1;
        for (l=j+1;l<int(m_Top.size()); l++)
        {
            m_Top[k+p]  = m_Top[l];
            p++;
        }
        //remove extra points
        for(int l=int(m_Top.size()-1); l>=k+p; l--)
            m_Top.pop_back();
    }

    //for(int i=0; i<m_rpTop.size(); i++) qDebug("%13.5f  %13.5f", m_rpTop.at(i).x, m_rpTop.at(i).y);

    // trim lower surface next
    i1 = iLowerh;
    i2 = iLowerh+1;

    bIntersect = false;
    for (j=i2; j<int(m_Bot.size()-1); j++)
    {
        for (k=i1;k>0; k--)
        {
            if(geom::intersect(m_Bot[j], m_Bot[j+1],
                               m_Bot[k], m_Bot[k-1], M))
            {
                bIntersect = true;
                break;
            }
        }
        if(bIntersect) break;
    }

    if(bIntersect)
    {
        m_Bot[k] = M;
        p=1;
        for (l=j+1;l<int(m_Bot.size()); l++)
        {
            m_Bot[k+p]  = m_Bot[l];
            p++;
        }

        //remove extra points
        for(int l=int(m_Bot.size()-1); l>=k+p; l--)
            m_Bot.pop_back();
    }

    // rotate the camber line

    uint im = 0;
    while(im<m_CbLine.size())
    {
        if(m_CbLine[im].x>=M.x)
        {
            // rotate around XHinge
            m_CbLine[im].rotateZ(M, -m_TEFlapAngle);
        }
        im++;
    }
}


void Foil::setFlaps()
{
    // copy the base foil to the current foil
    m_Top = m_BaseTop;
    m_Bot = m_BaseBot;
    m_CbLine = m_BaseCbLine;

    if(!m_bLEFlap && !m_bTEFlap) return;

    // modify the current geometry, not the base geometry
    if(m_bLEFlap && fabs(m_LEFlapAngle)>FLAPANGLEPRECISION)
        setLEFlap();
    if(m_bTEFlap && fabs(m_TEFlapAngle)>FLAPANGLEPRECISION)
        setTEFlap();

    // rebuild the current foil from the flapped top and bottom sides
    int newSize = int(m_Top.size() + m_Bot.size()-1);
    m_Node.resize(newSize);

    int sizeExt = int(m_Top.size());
    for (int i=sizeExt-1; i>=0; i--)
    {
        m_Node[sizeExt-1-i].x = m_Top.at(i).x;
        m_Node[sizeExt-1-i].y = m_Top.at(i).y;
    }

    for (uint i=0; i<m_Bot.size(); i++)
    {
        m_Node[sizeExt+i-1].x = m_Bot.at(i).x;
        m_Node[sizeExt+i-1].y = m_Bot.at(i).y;
    }
}


void Foil::clearPointArrays()
{
    m_BaseNode.clear();
    m_Node.clear();
}


void Foil::resizePointArrays(int size)
{
    m_BaseNode.resize(size);
    m_Node.resize(size);
}


void Foil::appendBasePoint(double x, double y)
{
    m_BaseNode.push_back({x,y});
}


bool Foil::sharpTE() const
{
    if(m_Node.size()<=2) return false;
    return (fabs(m_Node.front().x-m_Node.back().x)<length()/1000.0 &&
            fabs(m_Node.front().y-m_Node.back().y)<length()/1000.0);
}


Vector2d Foil::TEbisector() const
{
    Vector2d Vu(m_Node.at(0).x    -m_Node.at(1).x,     m_Node.at(0).y    -m_Node.at(1).y);
    Vector2d Vb(m_Node.at(nNodes()-1).x-m_Node.at(nNodes()-2).x, m_Node.at(nNodes()-1).y-m_Node.at(nNodes()-2).y);
    Vu.normalize();
    Vb.normalize();
    Vector2d V = (Vu+Vb);
    V.normalize();

    return V;
}


void Foil::makeNormalsFromCubic()
{
    double dx{0}, dy{0};
    for(int t=0; t<nBaseNodes(); t++)
    {
        m_CubicSpline.splineDerivative(double(t)/double(nBaseNodes()-1), dx, dy);
        double norm = sqrt(dx*dx+dy*dy);
        m_BaseNode[t].setNormal(dy/norm, -dx/norm);
    }
}


void Foil::makeCubicSpline(int nCtrlPts)
{
    m_CubicSpline.setBunchParameters(m_BunchType, m_BunchAmp);
    makeCubicSpline(m_CubicSpline, nCtrlPts);
}


void Foil::makeCubicSpline(CubicSpline &cs, int nCtrlPts) const
{
    cs.approximate(nCtrlPts, m_BaseNode);
}


bool Foil::serializeXfl(QDataStream &ar, bool bIsStoring)
{
    int k(0);
    int ArchiveFormat = 100007;
    // 100006 : first version of new xfl format
    // 100007: foil new style
    QString strange;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(m_Name);
        ar << QString::fromStdString(m_Description);

        m_theStyle.serializeXfl(ar, bIsStoring);

        ar << m_bCamberLine << m_bLEFlap << m_bTEFlap;
        ar << m_LEFlapAngle << m_LEXHinge*100.0 << m_LEYHinge*100.0;
        ar << m_TEFlapAngle << m_TEXHinge*100.0 << m_TEYHinge*100.0;
        ar << nBaseNodes();
        for (int j=0; j<nBaseNodes(); j++)
        {
            ar << m_BaseNode.at(j).x << m_BaseNode.at(j).y;
        }
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        ar >> strange;   m_Name = strange.toStdString();
        ar >> strange;   m_Description = strange.toStdString();

        if(ArchiveFormat<100007)
        {
            ar >> k; m_theStyle.setStipple(k);
            ar >> m_theStyle.m_Width;

            m_theStyle.m_Color.serialize(ar, false);
            ar >> m_theStyle.m_bIsVisible;

            qint8 b = 0x00;
            ar >> b; m_theStyle.setPointStyle(LineStyle::convertSymbol(int(b)));
        }
        else
            m_theStyle.serializeXfl(ar, bIsStoring);

        ar >> m_bCamberLine >> m_bLEFlap >> m_bTEFlap;
        ar >> m_LEFlapAngle >> m_LEXHinge >> m_LEYHinge;
        ar >> m_TEFlapAngle >> m_TEXHinge >> m_TEYHinge;

        m_LEXHinge /= 100.0;
        m_LEYHinge /= 100.0;
        m_TEXHinge /= 100.0;
        m_TEYHinge /= 100.0;

        int n;
        ar >> n;
        if(n>604) return false;

        for (int jl=0; jl<n; jl++)
        {
            double x, y;
            ar>>x>>y;
            appendBasePoint(x,y);
        }
        initGeometry();
        return true;
    }
}


bool Foil::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int kj(0), n(0);
    QString strange;

    int ArchiveFormat = 500753;
    // 500001: first version of new fl5 format
    // 500750: v7.50 making legacy TE flaps permanent
    // 500753: v7.53 added spline bunch parameters

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << QString::fromStdString(m_Name);
        ar << QString::fromStdString(m_Description);
        ar << LineStyle::convertLineStyle(m_theStyle.m_Stipple);
        ar << m_theStyle.m_Width;
        ar << LineStyle::convertSymbol(m_theStyle.m_Symbol);
        m_theStyle.m_Color.serialize(ar, true);

        ar << m_theStyle.m_bIsVisible;
        ar << m_bCamberLine << m_bLEFlap << m_bTEFlap;
        ar << m_LEFlapAngle << m_LEXHinge << m_LEYHinge;
        ar << m_TEFlapAngle << m_TEXHinge << m_TEYHinge;

        ar << m_BunchAmp;
        n=0;
        switch(m_BunchType)
        {
            case Spline::NOBUNCH:    n=0;    break;
            case Spline::UNIFORM:    n=1;    break;
            case Spline::SIGMOID:    n=2;    break;
            case Spline::DOUBLESIG:  n=3;    break;
        }
        ar << n;

        ar << nBaseNodes(); // store as int!
        for (int l=0; l<nBaseNodes(); l++)
        {
            ar <<m_BaseNode[l].x << m_BaseNode[l].y;
        }

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>550000) return false;
        ar >> strange;   m_Name = strange.toStdString();
        ar >> strange;   m_Description = strange.toStdString();;
        ar >> kj;   m_theStyle.m_Stipple=LineStyle::convertLineStyle(kj);
        ar >> m_theStyle.m_Width;
        ar >> kj;   m_theStyle.m_Symbol=LineStyle::convertSymbol(kj);
        m_theStyle.m_Color.serialize(ar, false);
        ar >> m_theStyle.m_bIsVisible;

        ar >> m_bCamberLine >> m_bLEFlap >> m_bTEFlap;
        ar >> m_LEFlapAngle >> m_LEXHinge >> m_LEYHinge;
        ar >> m_TEFlapAngle >> m_TEXHinge >> m_TEYHinge;

        if(ArchiveFormat>=500753)
        {
            ar >> m_BunchAmp;
            ar >> n;
            switch(n)
            {
                case 0: m_BunchType = Spline::NOBUNCH;    break;
                case 1: m_BunchType = Spline::UNIFORM;    break;
                case 2: m_BunchType = Spline::SIGMOID;    break;
                default:
                case 3: m_BunchType = Spline::DOUBLESIG;  break;
            }
        }
        m_BunchType = Spline::DOUBLESIG; // force it

        int n(0);
        double x(0), y(0);
        ar >> n;

        m_BaseNode.resize(n);
        for (int j=0; j<n; j++)
        {
            ar>>x>>y;
            m_BaseNode[j].set(x,y);
        }

        if(ArchiveFormat<500750)
        {
            if(m_bTEFlap && fabs(m_TEFlapAngle)>FLAPANGLEPRECISION)
            {
                initGeometry();
                setFlaps();
                makeModPermanent();
            }
        }

        m_TEFlapAngle = 0.0;


        initGeometry();
        return true;
    }
}


std::string Foil::listCoords(bool bBaseCoords)
{
    QString strCoords = "           x               y\n";

    if(bBaseCoords)
    {
        for(int i=0; i<nBaseNodes(); i++)
        {
            strCoords += QString::asprintf(" %13.5f   %13.5f\n", xb(i), yb(i));
        }
    }
    else
    {
        for(int i=0; i<nNodes(); i++)
        {
            strCoords += QString::asprintf(" %13.5f   %13.5f\n", x(i), y(i));
        }
    }
    return strCoords.toStdString();
}


std::string Foil::properties(bool bLong) const
{
    QString props;
    QString strange;

    if(bLong && m_Description.length()!=0) props +=  QString::fromStdString(m_Description)+EOLch;

    strange = QString::asprintf("Length           = %7.3f\n",   length());
    props += strange;
    strange = QString::asprintf("Mid-line angle   = %7.3f",     midLineAngle()) + DEGch + EOLch;
    props += strange;

    strange = QString::asprintf("Thickness        = %7.3f%%\n", maxThickness()*100.0);
    props += strange;
    strange = QString::asprintf("Max. thick. pos. = %7.3f%%\n", xThickness()*100.0);
    props += strange;
    strange = QString::asprintf("Camber           = %7.3f%%\n", maxCamber()*100.0);
    props += strange;
    strange = QString::asprintf("Max. camber pos. = %7.3f%%\n", xCamber()*100.0);
    props += strange;
    strange = QString::asprintf("T.E. gap         = %7.3f%%\n", TEGap()*100.0);
    props += strange;

    if(m_bLEFlap)
    {
        props += "L.E. flap:\n";
        strange = QString::asprintf("   hinge pos. = (%g%%, %g%%)\n", m_LEXHinge*100.0, m_LEYHinge*100.0);
        props += strange;
        strange = QString::asprintf("   flap angle = %5.2f", m_LEFlapAngle) + DEGch + "\n";
        props += strange;
    }


    if(m_bTEFlap)
    {
        strange = QString::asprintf("T.E. flap hinge pos. = (%g%%, %g%%)\n", m_TEXHinge*100.0, m_TEYHinge*100.0);
        props += strange;
    }
    else
        props += "No T.E. flap\n";

    strange = QString::asprintf("Number of nodes  = %d", nNodes());
    props += strange;


    return props.toStdString();
}

/*
void Foil::setBaseNodes(std::vector<Node2d> const &Nodes)
{
    m_BaseNode.resize(Nodes.size());
    for(uint i=0; i<Nodes.size(); i++)
        m_BaseNode[i] = Nodes.at(i);
}


void Foil::setBaseTop(std::vector<Node2d> const &nodes)
{
    m_BaseTop.resize(nodes.size());
    for(uint i=0; i<nodes.size(); i++)
        m_BaseTop[i] = nodes.at(i);
}


void Foil::setBaseBot(std::vector<Node2d> const &nodes)
{
    m_BaseBot.resize(nodes.size());
    for(uint i=0; i<nodes.size(); i++)
        m_BaseBot[i] = nodes.at(i);
}


void Foil::setBaseCamberLine(std::vector<Node2d> const &cb)
{
    m_BaseCbLine.resize(cb.size());
    for(uint i=0; i<cb.size(); i++)
        m_BaseCbLine[i] = cb[i];
}


void Foil::setThicknessLine(std::vector<double> const &th)
{
    m_Thickness.resize(th.size());
    for(uint i=0; i<th.size(); i++)
        m_Thickness[i] = th[i];
}*/


Node2d Foil::node(int index) const
{
    return m_Node.at(index);
}


void Foil::interpolate(Foil const *pFoil1, Foil const *pFoil2, double frac)
{
    std::vector<Node2d> const &cline1 = pFoil1->m_BaseCbLine;
    std::vector<Node2d> const &cline2 = pFoil2->m_BaseCbLine;

    std::vector<double> const &th1 = pFoil1->m_Thickness;
    std::vector<double> const &th2 = pFoil2->m_Thickness;

    assert(cline1.size()==cline2.size());
    assert(th1.size()==th2.size());
    assert(cline1.size()==th1.size());

    for(uint i=0; i<m_BaseCbLine.size(); i++)
    {
        m_BaseCbLine[i].y = (1.0-frac) * cline1[i].y + frac*cline2[i].y;
        m_Thickness[i] = (1.0-frac) * th1[i] + frac*th2[i];
    }
}


/**
 * "Base arrays" = raw airfoil as imported or constructed, with coarse panelling and no flap
 * "Active arrays" = Base + repanelling and flaps
 *
 * Use the BaseNodes + re-panelling data to
 *   - find the LE and TE
 *   - build the camber line and thickness array
 *   - build the top and bot base arrays, including the normals
 *
 * Procedure:
 *   0. Make the cubic spline C3S using BaseNodes
 *   1. repanel using the C3S and the bunch parameters and store in Nodes
 *   2. set LE and midline from Nodes
 *   3. Create top and bot surfaces base + current
 *   4. set the flaps
 */
bool Foil::initGeometry()
{
    if(nBaseNodes()<=0) return false;

    // set the Trailing edge
    m_TE.x = (m_BaseNode.front().x+m_BaseNode.back().x)/2.0;
    m_TE.y = (m_BaseNode.front().y+m_BaseNode.back().y)/2.0;

    makeCubicSpline(); // using base nodes

    makeNormalsFromCubic(); // using base nodes

    setLEFromCubicSpline(); // set the leading edge using the cubic spline

    makeBaseMidLine();

    if(!makeTopBotSurfaces()) return false;

    applyBase(); // copy base to the active set of nodes

    setFlaps(); // set flaps on the active set of nodes

    return true;
}







