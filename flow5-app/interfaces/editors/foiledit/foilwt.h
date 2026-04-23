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

#include <QSettings>
#include <QCheckBox>

#include <interfaces/widgets/view/section2dwt.h>

class Foil;
class Spline;
class SplineFoil;
class LineMenu;
class LineBtn;

class FoilWt : public Section2dWt
{
    Q_OBJECT

    friend class FoilDlg;
    friend class InterpolateFoilsDlg;

    public:
        FoilWt(QWidget *pParent = nullptr);

        void clearFoils() {m_oaFoil.clear();}
        void addFoil(const Foil *pFoil) {if(pFoil && !m_oaFoil.contains(pFoil)) m_oaFoil.append(pFoil);}
        QVector<Foil const*> const &foils() const {return m_oaFoil;}

        void clearSplines() {m_oaSpline.clear();}
        void addSpline(Spline *pSpline) {if(pSpline) m_oaSpline.append(pSpline);}
        QVector<Spline*> const &splines() const {return m_oaSpline;}
        void setSpline(Spline *pSpline) {m_oaSpline.clear(); if(pSpline) m_oaSpline.append(pSpline);}

        void setBufferFoil(Foil *pBufferFoil) {m_pBufferFoil = pBufferFoil;}
        void setSplineFoil(SplineFoil *pSF) {m_pSF = pSF;}
        void setLECircleRadius(double radius){m_LECircleRadius = radius;}

        void freezeSpline(bool bFreeze=true) {m_bFrozenSpline=bFreeze;}
        bool isSplineFrozen() const {return m_bFrozenSpline;}

        void setIn01(bool bIn01) {m_bIn01Interval=bIn01;}
        bool isIn01() const {return m_bIn01Interval;}

        void showHinges(bool b) {m_bShowHinges=b;}

        static bool isFilledBufferFoil() {return s_bFillBufferFoil;}
        static void setFilledBufferFoil(bool bFill) {s_bFillBufferFoil=bFill;}

        static bool bCamberLines() {return s_bCamberLines;}
        static void showCamberLines(bool bShow) {s_bCamberLines=bShow;}

        static LineStyle &bufferFoilStyle() {return s_BufferStyle;}
        static void setBufferFoilStyle(LineStyle const &ls)  {s_BufferStyle=ls;}

    protected:
        void paint(QPainter &painter) override;

        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent (QResizeEvent *pEvent) override;
        void createContextMenu();
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        QPointF defaultOffset() const override {return QPointF(rect().width()/8, rect().height()/2);}
        double defaultScale() const override {return double(rect().width())*6.0/8.0;}
        int highlightPoint(QPointF const &real) override;
        int selectPoint(QPointF const &real) override;
        void dragSelectedPoint(double, double) override;
        void onInsertPt() override;
        void onRemovePt() override;

    protected slots:
        void onOverlayFoil();
        void onShowCamberLines(bool bShow);
        void onBufferShow();

    private:
        void paintFoils(QPainter &painter);
        void paintFoil(QPainter &painter, const Foil *pFoil);
        void paintSplines(QPainter &painter);
        void paintSpline(QPainter &painter, const Spline *pSpline);
        void paintSplineFoil(QPainter &painter);
        void paintLECircle(QPainter &painter);
        void paintFoilLegend(QPainter &painter);
        void drawHinges(QPainter &painter, const Foil *pFoil);

    private:

        bool m_bIn01Interval;          /**< if true force spline points to be in x = [0,1] interval */
        bool m_bFrozenSpline;          /**< if true, the spline's control points cannot be dragged */
        bool m_bShowHinges;

        double m_LECircleRadius;
        QVector<Foil const*> m_oaFoil;
        QVector<Spline *> m_oaSpline;

        SplineFoil *m_pSF;

        Foil *m_pBufferFoil; /** a pointer to enable access to the foil style in the context menu */
        LineMenu *m_pBufferLineMenu;
        QMenu *m_pBufferMenu;

        QAction *m_pShowBufferFoil, *m_pFillBufferFoil;
        QAction *m_pOverlayFoil, *m_pShowCamberLines;

        QPointF m_LegendPos;


        static bool s_bCamberLines;
        static bool s_bFillBufferFoil;
        static LineStyle s_BufferStyle;
};








