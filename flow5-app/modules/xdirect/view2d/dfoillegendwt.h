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

#include <QWidget>
#include <QMap>

#include <api/linestyle.h>
#include <core/enums_core.h>

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


