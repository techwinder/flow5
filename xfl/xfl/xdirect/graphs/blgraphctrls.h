/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QCheckBox>

#include <xfl/graphs/graphtilectrls.h>

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

