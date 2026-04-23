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

#include <core/xflcore.h>

#include <interfaces/widgets/view/grid.h>

Grid::Grid()
{
    setDefaults();
}


void Grid::setDefaults()
{
    m_bXAutoMinGrid = true;
    m_bYAutoMinGrid[0] = m_bYAutoMinGrid[1] = true;

    m_bXAxis = true;
    m_XAxisStyle.m_Stipple = Line::SOLID;
    m_XAxisStyle.m_Width = 2;
    m_XAxisStyle.m_Color = fl5Color(55,55,55);

    m_bYAxis = true;
    m_YAxisStyle.m_Stipple = Line::SOLID;
    m_YAxisStyle.m_Width = 2;
    m_YAxisStyle.m_Color = fl5Color(55,55,55);

    m_bXMajGrid  = true;
    m_XMajUnit  = 0.05;
    m_XMajStyle.m_Stipple = Line::DASH;
    m_XMajStyle.m_Width = 1;
    m_XMajStyle.m_Color = fl5Color(95,95,95);

    m_bXMinGrid  = false;
    m_XMinUnit   = 0.01;
    m_XMinStyle.m_Stipple = Line::DOT;
    m_XMinStyle.m_Width  = 1;
    m_XMinStyle.m_Color  = fl5Color(70,70,70);

    for(int iy=0; iy<2; iy++)
    {
        m_bYMajGrid[iy] = true;
        m_YMajUnit[iy]  = 0.05;
        m_YMajStyle[iy].m_Stipple = Line::DASH;
        m_YMajStyle[iy].m_Width = 1;
        m_YMajStyle[iy].m_Color = fl5Color(95,95,95);

        m_bYMinGrid[iy] = false;
        m_YMinUnit[iy]  = 0.01;
        m_YMinStyle[iy].m_Stipple  = Line::DOT;
        m_YMinStyle[iy].m_Width  = 1;
        m_YMinStyle[iy].m_Color  = fl5Color(70,70,70);
    }

    m_nDecimals  = 2;
}


void Grid::duplicate(const Grid &grid)
{
    m_bXAxis     = grid.m_bXAxis;
    m_XAxisStyle = grid.m_XAxisStyle;

    m_bYAxis     = grid.m_bYAxis;
    m_YAxisStyle = grid.m_YAxisStyle;

    m_bXMajGrid = grid.m_bXMajGrid;
    m_XMajUnit  = grid.m_XMajUnit;
    m_XMajStyle = grid.m_XMajStyle;

    m_bXMinGrid  = grid.m_bXMinGrid;
    m_XMinUnit   = grid.m_XMinUnit;
    m_XMinStyle  = grid.m_XMinStyle;

    for(int iy=0; iy<2; iy++)
    {
        m_bYMajGrid[iy] = grid.m_bYMajGrid[iy];
        m_YMajUnit[iy]  = grid.m_YMajUnit[iy];
        m_YMajStyle[iy] = grid.m_YMajStyle[iy];

        m_bYMinGrid[iy] = grid.m_bYMinGrid[iy];
        m_YMinUnit[iy]  = grid.m_YMinUnit[iy];
        m_YMinStyle[iy] = grid.m_YMinStyle[iy];
    }

    m_nDecimals  = grid.m_nDecimals;
}


void Grid::loadSettings(QSettings &settings)
{
    settings.beginGroup("Grid");
    {
        if(settings.contains("XAxis")) m_bXAxis = settings.value("XAxis", true).toBool();
        xfl::loadLineSettings(settings, m_XAxisStyle,"XAxisStyle");

        if(settings.contains("YAxis")) m_bYAxis = settings.value("YAxis", true).toBool();
        xfl::loadLineSettings(settings, m_YAxisStyle,"YAxisStyle");

        if(settings.contains("XGrid")) m_bXMajGrid = settings.value("XGrid", true).toBool();
        xfl::loadLineSettings(settings, m_XMajStyle, "XMajStyle");

        if(settings.contains("XMinGrid")) m_bXMinGrid = settings.value("XMinGrid", false).toBool();
        xfl::loadLineSettings(settings, m_XMinStyle, "XMinStyle");

        if(settings.contains("Y0Grid")) m_bYMajGrid[0] = settings.value("Y0Grid", true).toBool();
        xfl::loadLineSettings(settings, m_YMajStyle[0], "Y0MajStyle");

        if(settings.contains("Y0MinGrid")) m_bYMinGrid[0] = settings.value("Y0MinGrid", false).toBool();
        xfl::loadLineSettings(settings, m_YMinStyle[0], "Y0MinStyle");

        if(settings.contains("Y1Grid")) m_bYMajGrid[1] = settings.value("Y1Grid", true).toBool();
        xfl::loadLineSettings(settings, m_YMajStyle[1], "Y1MajStyle");

        if(settings.contains("Y1MinGrid")) m_bYMinGrid[1] = settings.value("Y1MinGrid", false).toBool();
        xfl::loadLineSettings(settings, m_YMinStyle[1], "Y1MinStyle");
    }
    settings.endGroup();
}


void Grid::saveSettings(QSettings &settings) const
{
    settings.beginGroup("Grid");
    {
        settings.setValue("XAxis", m_bXAxis);
        xfl::saveLineSettings(settings, m_XAxisStyle, "XAxisStyle");

        settings.setValue("YAxis", m_bYAxis);
        xfl::saveLineSettings(settings, m_YAxisStyle, "YAxisStyle");

        settings.setValue("XGrid", m_bXMajGrid);
        xfl::saveLineSettings(settings, m_XMajStyle, "XMajStyle");

        settings.setValue("XMinGrid", m_bXMinGrid);
        xfl::saveLineSettings(settings, m_XMinStyle, "XMinStyle");

        settings.setValue("Y0Grid", m_bYMajGrid[0]);
        xfl::saveLineSettings(settings, m_YMajStyle[0], "Y0MajStyle");

        settings.setValue("Y0MinGrid", m_bYMinGrid[0]);
        xfl::saveLineSettings(settings, m_YMinStyle[0], "Y0MinStyle");

        settings.setValue("Y1Grid", m_bYMajGrid[1]);
        xfl::saveLineSettings(settings, m_YMajStyle[1], "Y1MajStyle");

        settings.setValue("Y1MinGrid", m_bYMinGrid[1]);
        xfl::saveLineSettings(settings, m_YMinStyle[1], "Y1MinStyle");
    }
    settings.endGroup();
}


void Grid::setXAxis(bool state, fl5Color clr, Line::enumLineStipple stipple, int width)
{
    m_bXAxis = state;
    m_XAxisStyle.m_Color = clr;
    m_XAxisStyle.m_Stipple = stipple;
    m_XAxisStyle.m_Width = width;
}


void Grid::setYAxis(bool state, fl5Color clr, Line::enumLineStipple stipple, int width)
{
    m_bYAxis = state;
    m_YAxisStyle.m_Color = clr;
    m_YAxisStyle.m_Stipple = stipple;
    m_YAxisStyle.m_Width = width;
}


void Grid::setXMajGrid(bool state, fl5Color clr, Line::enumLineStipple stipple, int width)
{
    m_bXMajGrid = state;
    m_XMajStyle.m_Stipple = stipple;
    m_XMajStyle.m_Width   = width;
    m_XMajStyle.m_Color   = clr;
}


void Grid::setXMinGrid(bool state, bool bAuto, fl5Color clr, Line::enumLineStipple stipple, int width)
{
    m_bXMinGrid = state;
    m_bXAutoMinGrid = bAuto;
    m_XMinStyle.m_Stipple = stipple;
    m_XMinStyle.m_Width   = width;
    m_XMinStyle.m_Color   = clr;
}



void Grid::setYMajGrid(int iy, bool state, fl5Color clr, Line::enumLineStipple stipple, int width)
{
    m_bYMajGrid[iy] = state;
    m_YMajStyle[iy].m_Color   = clr;
    m_YMajStyle[iy].m_Stipple = stipple;
    m_YMajStyle[iy].m_Width = width;
}



void Grid::setYMinGrid(int iy, bool state, bool bAuto, fl5Color clr, Line::enumLineStipple stipple, int width)
{
    m_bYMinGrid[iy] = state;
    m_bYAutoMinGrid[iy] = bAuto;
    m_YMinStyle[iy].m_Color   = clr;
    m_YMinStyle[iy].m_Stipple = stipple;
    m_YMinStyle[iy].m_Width = width;
}


void Grid::xAxis(bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width)
{
    bstate  = m_bXAxis;
    clr     = m_XAxisStyle.m_Color;
    stipple = m_XAxisStyle.m_Stipple;
    width   = m_XAxisStyle.m_Width;
}


void Grid::yAxis(bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width)
{
    bstate  = m_bYAxis;
    clr     = m_YAxisStyle.m_Color;
    stipple = m_YAxisStyle.m_Stipple;
    width   = m_YAxisStyle.m_Width;
}


void Grid::xMajGrid(bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width)
{
    bstate = m_bXMajGrid;
    clr   = m_XMajStyle.m_Color;
    stipple = m_XMajStyle.m_Stipple;
    width = m_XMajStyle.m_Width;
}


void Grid::xMinGrid(bool &state, bool &bAuto, fl5Color &clr, Line::enumLineStipple &stipple, int &width)
{
    state = m_bXMinGrid;
    bAuto = m_bXAutoMinGrid;
    clr   = m_XMinStyle.m_Color;
    stipple = m_XMinStyle.m_Stipple;
    width = m_XMinStyle.m_Width;
}


void Grid::yMajGrid(int iy, bool &state, fl5Color &clr, Line::enumLineStipple &stipple, int &width)
{
    state = m_bYMajGrid[iy];
    clr   = m_YMajStyle[iy].m_Color;
    stipple = m_YMajStyle[iy].m_Stipple;
    width = m_YMajStyle[iy].m_Width;
}


void Grid::yMinGrid(int iy, bool &state, bool &bAuto, fl5Color &clr, Line::enumLineStipple &stipple, int &width)
{
    state = m_bYMinGrid[iy];
    bAuto = m_bYAutoMinGrid[iy];
    clr   = m_YMinStyle[iy].m_Color;
    stipple = m_YMinStyle[iy].m_Stipple;
    width = m_YMinStyle[iy].m_Width;
}


void Grid::setXMajColor(const QColor &clr)         {m_XMajStyle.m_Color=xfl::tofl5Clr(clr);}
void Grid::setXMinColor(const QColor &clr)         {m_XMinStyle.m_Color=xfl::tofl5Clr(clr);}
void Grid::setYMajColor(int iy, const QColor &clr) {m_YMajStyle[iy].m_Color=xfl::tofl5Clr(clr);}
void Grid::setYMinColor(int iy, const QColor &clr) {m_YMinStyle[iy].m_Color=xfl::tofl5Clr(clr);}
void Grid::setXAxisColor(const QColor &clr)        {m_XAxisStyle.m_Color=xfl::tofl5Clr(clr);}
void Grid::setYAxisColor(const QColor &clr)        {m_YAxisStyle.m_Color=xfl::tofl5Clr(clr);}

QColor Grid::xAxisColor()      const {return xfl::fromfl5Clr(m_XAxisStyle.m_Color);}
QColor Grid::yAxisColor()      const {return xfl::fromfl5Clr(m_YAxisStyle.m_Color);}
QColor Grid::xMajColor()       const {return xfl::fromfl5Clr(m_XMajStyle.m_Color);}
QColor Grid::xMinColor()       const {return xfl::fromfl5Clr(m_XMinStyle.m_Color);}
QColor Grid::yMajColor(int iy) const {return xfl::fromfl5Clr(m_YMajStyle[iy].m_Color);}
QColor Grid::yMinColor(int iy) const {return xfl::fromfl5Clr(m_YMinStyle[iy].m_Color);}




