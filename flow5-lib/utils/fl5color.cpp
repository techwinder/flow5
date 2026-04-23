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



#include <fl5color.h>




fl5Color fl5Color::darker(int f) const
{
    return {short(float(m_Red)  *(100.0/float(f))),
            short(float(m_Green)*(100.0/float(f))),
            short(float(m_Blue) *(100.0/float(f)))};
}


unsigned int qt_div_257_floor(unsigned int x) { return  (x - (x >> 8)) >> 8; }
unsigned int qt_div_257(unsigned int x) { return qt_div_257_floor(x + 128); }


// in place replacement for QColor serialization
void fl5Color::serialize(QDataStream &stream, bool bIsStoring)
{
    if(bIsStoring)
    {
        signed char s(0);
        unsigned short a = m_Alpha * 0x101;
        unsigned short r = m_Red   * 0x101;
        unsigned short g = m_Green * 0x101;
        unsigned short b = m_Blue  * 0x101;
        unsigned short p(0);
        stream << s;
        stream << a;
        stream << r;
        stream << g;
        stream << b;
        stream << p;
    }
    else
    {
        signed char s(0);
        unsigned short a(0), r(0), g(0), b(0), p(0);
        stream >> s;
        stream >> a;
        stream >> r;
        stream >> g;
        stream >> b;
        stream >> p;

        m_Alpha = qt_div_257(a);
        m_Red   = qt_div_257(r);
        m_Green = qt_div_257(g);
        m_Blue  = qt_div_257(b);
    }
}






