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

#pragma once

#include <QColor>
#include <QSettings>

#include <api/linestyle.h>



class Grid
{
    public:
        Grid();

        void duplicate(Grid const &grid);

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings) const;

        int nDecimals() const {return m_nDecimals;}
        void setnDecimals(int nDecimals) {m_nDecimals=nDecimals;}

        void xAxis(bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width);
        void setXAxis(bool state, fl5Color clr, Line::enumLineStipple stipple, int width);

        void yAxis(bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width);
        void setYAxis(bool state, fl5Color clr, Line::enumLineStipple stipple, int width);


        void xMajGrid(bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width);
        void yMajGrid(int iy, bool &bstate, fl5Color &clr, Line::enumLineStipple &stipple, int &width);
        void xMinGrid(bool &bstate, bool &bAutoCurveList, fl5Color &clr, Line::enumLineStipple &stipple, int &width);
        void yMinGrid(int iy, bool &bstate, bool &bAutoCurveList, fl5Color &clr, Line::enumLineStipple &stipple, int &width);

        void setXMajGrid(bool state, fl5Color clr, Line::enumLineStipple stipple, int width);
        void setXMinGrid(bool state, bool bAutoCurveList, fl5Color clr, Line::enumLineStipple stipple, int width);
        void showXMajGrid(bool bShow) {m_bXMajGrid = bShow;}
        void showXMinGrid(bool bShow) {m_bXMinGrid = bShow;}

        void setYMajGrid(int iy, bool state, fl5Color clr, Line::enumLineStipple stipple, int width);
        void setYMinGrid(int iy, bool state, bool bAutoCurveList, fl5Color clr, Line::enumLineStipple stipple, int width);
        void showYMajGrid(int iy, bool bShow) {m_bYMajGrid[iy] = bShow;}
        void showYMinGrid(int iy, bool bShow) {m_bYMinGrid[iy] = bShow;}

        void setAutoXMinGrid(bool bAuto) {m_bXAutoMinGrid=bAuto;}
        void setAutoYMinGrid(int iy, bool bAuto) {m_bYAutoMinGrid[iy]=bAuto;}

        void setXMajColor(QColor const &clr);
        void setXMinColor(QColor const &clr);
        void setYMajColor(int iy, QColor const &clr);
        void setYMinColor(int iy, QColor const &clr);

        void setDefaults();

        void setXAxisColor(QColor const &clr);
        void setXAxisStyle(LineStyle const &ls) {m_XAxisStyle=ls;}
        void setYAxisColor(QColor const &clr);
        void setYAxisStyle(LineStyle const &ls) {m_YAxisStyle=ls;}

        void setXMajStyle(        LineStyle const &ls) {m_XMajStyle=ls;}
        void setXMinStyle(        LineStyle const &ls) {m_XMinStyle=ls;}
        void setYMajStyle(int iy, LineStyle const &ls) {m_YMajStyle[iy]=ls;}
        void setYMinStyle(int iy, LineStyle const &ls) {m_YMinStyle[iy]=ls;}

        LineStyle const & xAxisStyle()      const {return m_XAxisStyle;}
        LineStyle const & yAxisStyle()      const {return m_YAxisStyle;}
        LineStyle const & xMajStyle()       const {return m_XMajStyle;}
        LineStyle const & xMinStyle()       const {return m_XMinStyle;}
        LineStyle const & yMajStyle(int iy) const {return m_YMajStyle[iy];}
        LineStyle const & yMinStyle(int iy) const {return m_YMinStyle[iy];}

        Line::enumLineStipple xAxisStipple() const {return m_XAxisStyle.m_Stipple;}
        int xAxisWidth() const {return m_XAxisStyle.m_Width;}
        QColor xAxisColor() const;

        Line::enumLineStipple yAxisStipple() const {return m_YAxisStyle.m_Stipple;}
        int yAxisWidth() const {return m_YAxisStyle.m_Width;}
        QColor yAxisColor() const;

        Line::enumLineStipple xMajStipple() const {return m_XMajStyle.m_Stipple;}
        int xMajWidth() const {return m_XMajStyle.m_Width;}
        QColor xMajColor() const;

        Line::enumLineStipple xMinStipple() const {return m_XMinStyle.m_Stipple;}
        int xMinWidth() const {return m_XMinStyle.m_Width;}
        QColor xMinColor() const;

        Line::enumLineStipple yMajStipple(int iy) const {return m_YMajStyle[iy].m_Stipple;}
        int yMajWidth(int iy) const {return m_YMajStyle[iy].m_Width;}
        QColor yMajColor(int iy) const;

        Line::enumLineStipple yMinStipple(int iy) const {return m_YMinStyle[iy].m_Stipple;}
        int yMinWidth(int iy) const {return m_YMinStyle[iy].m_Width;}
        QColor yMinColor(int iy) const;

        void setXMajUnit(double unit) {m_XMajUnit=unit;}
        void setXMinUnit(double unit) {m_XMinUnit=unit;}

        void setYMajUnit(int iy, double unit) {m_YMajUnit[iy]=unit;}
        void setYMinUnit(int iy, double unit) {m_YMinUnit[iy]=unit;}

        double xMajUnit() const {return m_XMajUnit;}
        double xMinUnit() const {return m_XMinUnit;}

        double yMajUnit(int iy) const {return m_YMajUnit[iy];}
        double yMinUnit(int iy) const {return m_YMinUnit[iy];}

        bool bXMajGrid() const {return m_bXMajGrid;}
        bool bXMinGrid() const {return m_bXMinGrid;}

        bool bYMajGrid(int iy) const {return m_bYMajGrid[iy];}
        bool bYMinGrid(int iy) const {return m_bYMinGrid[iy];}

        bool bXAutoMinGrid() const {return m_bXAutoMinGrid;}
        bool bYAutoMinGrid(int iy) const {return m_bYAutoMinGrid[iy];}

        bool bXAxis() const {return m_bXAxis;}
        void showXAxis(bool bShow) {m_bXAxis=bShow;}

        bool bYAxis() const {return m_bXAxis;}
        void showYAxis(bool bShow) {m_bXAxis=bShow;}

    private:

        bool m_bXAutoMinGrid, m_bYAutoMinGrid[2];

        bool m_bXAxis;                       /**< true if the x-axis line should be displayed */
        LineStyle m_XAxisStyle;              /**< the style of the x-axis */

        bool m_bYAxis;                       /**< true if the y-axis line should be displayed */
        LineStyle m_YAxisStyle;              /**< the style of the y-axis */

        bool m_bXMajGrid;                      /**< true if the X main grid (vertical lines) should be displayed */
        double m_XMajUnit;                     /**< the unit of the main X-grid */
        LineStyle m_XMajStyle;                 /**< the style of the major x-grid */

        bool m_bXMinGrid;                      /**< true if the X minor grid (vertical lines) should be displayed */
        double m_XMinUnit;                     /**< the unit of the minor X-grid */
        LineStyle m_XMinStyle;                 /**< the style of the minor x-grid */

        bool m_bYMajGrid[2];                      /**< true if the Y main grid (vertical lines) should be displayed */
        double m_YMajUnit[2];                     /**< the unit of the main Y-grid */
        LineStyle m_YMajStyle[2];                 /**< the style of the major x-grid */

        bool m_bYMinGrid[2];                      /**< true if the Y minor grid (vertical lines) should be displayed */
        double m_YMinUnit[2];                     /**< the unit of the minor Y-grid */
        LineStyle m_YMinStyle[2];                 /**< the style of the minor x-grid */


        int m_nDecimals;
};

