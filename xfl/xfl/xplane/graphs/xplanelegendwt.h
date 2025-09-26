/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflgraph/containers/legendwt.h>
#include <xfl/xplane/xplane.h>


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


