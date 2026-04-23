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

#pragma once

#include <QDataStream>

#include <fl5lib_global.h>


struct FL5LIB_EXPORT fl5Color
{
    fl5Color() {}

    fl5Color(short red, short green, short blue) : m_Red(red), m_Green(green), m_Blue(blue)
    {
    }

    fl5Color(short red, short green, short blue, short alpha) : m_Red(red), m_Green(green), m_Blue(blue), m_Alpha(alpha)
    {
    }

    short m_Red = 255;
    short m_Green = 255;
    short m_Blue = 255;
    short m_Alpha = 255;

    inline int red()   const {return m_Red;}
    inline int green() const {return m_Green;}
    inline int blue()  const {return m_Blue;}
    inline int alpha() const {return m_Alpha;}

    inline float redF()   const {return float(m_Red)/255.0f;}
    inline float greenF() const {return float(m_Green)/255.0f;}
    inline float blueF()  const {return float(m_Blue)/255.0f;}
    inline float alphaF() const {return float(m_Alpha)/255.0f;}

    inline void setRed(short r)   {m_Red   = r;}
    inline void setGreen(short g) {m_Green = g;}
    inline void setBlue(short b)  {m_Blue  = b;}
    inline void setAlpha(short a) {m_Alpha = a;}

    inline void setRgb(short r, short g, short b) {m_Red=r; m_Green=g; m_Blue=b;}
    inline void setRgba(short r, short g, int b, short a) {m_Red=r; m_Green=g; m_Blue=b; m_Alpha=a;}

    inline void fromFloat(float r, float g, float b) {m_Red = short(r*255.0); m_Green = short(g*255.00); m_Blue = short(b*255.0);}

    fl5Color darker(int f) const;

    // in place replacement for QColor serialization
    void serialize(QDataStream &stream, bool bIsStoring);
};





