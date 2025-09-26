/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <xflgraph/containers/legendwt.h>

class XSail;
class BoatPolar;
class XSailLegendWt : public LegendWt
{
    Q_OBJECT

    public:
        XSailLegendWt(QWidget *pParent = nullptr);

        static void setXSail(XSail*pXSail)   {s_pXSail=pXSail;}


        void makeLegend(bool bHighlight) override;

    private:

        bool isPolarVisible(BoatPolar *pPolar);
        void makePolarLegendBtns(bool bHighlight);

    private slots:
        void onClickedPolarBtn();
        void onClickedPolarBtnLine(LineStyle ls);

    private:
        static XSail *s_pXSail;
};

