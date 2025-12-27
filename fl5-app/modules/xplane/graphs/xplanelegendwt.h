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
#include <modules/xplane/xplane.h>


class XPlaneLegendWt : public LegendWt
{
    Q_OBJECT

    public:
        XPlaneLegendWt(QWidget *pParent = nullptr);

        static void setXPlane(XPlane*pXPlane) {s_pXPlane=pXPlane;}

        void makeLegend(bool bHighlight) override;


    protected:

        void makeWPolarLegendBtns(XPlane::enumViews eXPlaneView, bool bHighlight);

        void makePOppLegendBtns(bool bHighlight);


    private slots:
        void onClickedWPolarBtn();
        void onClickedWPolarBtnLine(LineStyle ls);
        void onClickedPOppBtn();
        void onClickedPOppBtnLine(LineStyle ls);

    private:
        static XPlane *s_pXPlane;

};


