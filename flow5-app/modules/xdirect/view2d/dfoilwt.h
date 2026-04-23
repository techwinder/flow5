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


#include <interfaces/widgets/view/section2dwt.h>

class XDirect;
class Foil;
class DFoilLegendWt;

class DFoilWt : public Section2dWt
{
    Q_OBJECT

    public:
        DFoilWt(QWidget *pParent=nullptr);
        ~DFoilWt() override;

        void setXDirect(XDirect *pXDirect) {s_pXDirect = pXDirect;}

        void resetLegend() {m_bResetLegend=true;}
        void showLegend(bool bShow) override;
        void updateView();

        void selectFoil(Foil *pFoil);

        static bool isLECircleVisible() {return s_bLECircle;}
        static void setLECircleVisible(bool bLECircle){s_bLECircle=bLECircle;}
        static double LECircleRadius() {return s_LERad;}
        static void setLECircleRadius(double radius) {s_LERad = radius;}

        static void showLEPosition(bool bShowLE) {s_bShowLE=bShowLE;}
        static bool isLEPositionVisible() {return s_bShowLE;}

        static void setFilling(bool bFill) {s_bFillSelected=bFill;}
        static bool isFilling() {return s_bFillSelected;}

        static void showTEHinge(bool b) {s_bShowTEHinge=b;}
        static bool isTEHingeVisible() {return s_bShowTEHinge;}

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

    private slots:
        void onHovered();
        void onSaveToSvg() override;
        void onFillFoil(bool bFill);

    signals:
        void foilSelected(Foil *);

    private:
        void paintFoils(QPainter &painter);
        void paintLECircle(QPainter &painter);

        Foil* isFoil(QPointF pos);

        QPointF defaultOffset() const override {return QPointF(rect().width()/8, rect().height()/2);}
        double defaultScale() const override {return double(rect().width())*6.0/8.0;}

        void paint(QPainter &painter) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        void createActions();

        void makeLegend();


    private:
        bool m_bResetLegend;

        DFoilLegendWt *m_pFoilLegendWt;

        QTimer *m_pHoverTimer;

        static XDirect *s_pXDirect;

        static bool s_bShowLE;             /**< true if the LE point should be displayed */
        static bool s_bLECircle;           /**< true if the leading edge circle should be displayed >*/
        static double s_LERad;             /**< the radius of the leading edge circle to draw >*/

        static bool s_bFillSelected;
        static bool s_bShowTEHinge;
        static Foil * s_pHighFoil;
};




