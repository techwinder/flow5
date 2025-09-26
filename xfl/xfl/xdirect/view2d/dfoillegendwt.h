/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QMap>

#include <xflcore/linestyle.h>
#include <xflcore/enums_core.h>

class Foil;
class LegendBtn;
class XDirect;

/**
* @class DFoilLegendWidget
* A selectable legend widget.
*/

class DFoilLegendWt : public QWidget
{
    Q_OBJECT
    public:
        DFoilLegendWt(QWidget *pParent = nullptr);
        ~DFoilLegendWt();

//        QSize sizeHint() const override {return QSize(300,300);}

        int nButtons() const {return m_FoilMap.size();}
        QMap<LegendBtn*, Foil*> legendBtns() {return m_FoilMap;}


        void makeDFoilLegend();

        void selectFoil(Foil *pFoil);

        static void setXDirect(XDirect* pXDirect) {s_pXDirect = pXDirect;}

    private:


    private slots:
//        void onClickedFoilBtn();


    public:
        static XDirect *s_pXDirect;


    private:
        QMap<LegendBtn*, Foil*> m_FoilMap;
//        QSize m_SizeHint;

};


