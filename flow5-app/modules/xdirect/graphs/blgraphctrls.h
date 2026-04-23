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

#include <QCheckBox>

#include <interfaces/graphs/controls/graphtilectrls.h>

class MainFrame;
class XDirect;
class LineBtn;

class BLGraphCtrls : public GraphTileCtrls
{
    Q_OBJECT

    public:
        BLGraphCtrls(GraphTiles *pParent=nullptr);

        void setControls() override;

        void setupLayout() override;
        void connectSignals() override;

    public slots:
        void onBLSide();
        void onInviscid();
        void onTopCurveStyle(LineStyle);
        void onBotCurveStyle(LineStyle);
    private:

        QCheckBox *m_pchShowInviscid;
        QCheckBox *m_pchTop, *m_pchBot;


        LineBtn *m_plbTopStyle, *m_plbBotStyle;

};

