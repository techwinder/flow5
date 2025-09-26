/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xflgraph/containers/legendwt.h>

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


