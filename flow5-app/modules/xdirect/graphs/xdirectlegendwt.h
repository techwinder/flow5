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


#include <interfaces/graphs/containers/legendwt.h>

class XDirect;
class Polar;


class XDirectLegendWt : public LegendWt
{
    Q_OBJECT

    public:
        XDirectLegendWt(QWidget *pParent = nullptr);

        static void setXDirect(XDirect*pXDirect)  {s_pXDirect=pXDirect;}

        void makeLegend(bool bHighlight) override;

    private:

        void makePolarLegendBtns(bool bHighlight);
        void makeOppLegendBtns(bool bHighlight);

    private slots:
        void onClickedPolarBtn();
        void onClickedPolarBtnLine(LineStyle ls);
        void onClickedOppBtn();
        void onClickedOppBtnLine(LineStyle ls);

    private:
        static XDirect *s_pXDirect;
};


