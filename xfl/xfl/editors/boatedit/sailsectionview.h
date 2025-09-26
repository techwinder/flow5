/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflwidgets/view/section2dwt.h>


class Sail;
class NURBSSail;
class NURBSSurface;

class SailSectionView : public Section2dWt
{
    Q_OBJECT

    public:
        SailSectionView();
        void setSail(Sail *pSail);
        void setNurbs(NURBSSurface *pNurbs) {m_pNurbs=pNurbs;}

        void paint(QPainter &painter) override;

//        void createActions() override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        int highlightPoint(QPointF const &pos) override;
        int selectPoint(QPointF const &pos) override;
        void dragSelectedPoint(double x, double y) override;
        void createContextMenu();

        QPointF defaultOffset() const override {return QPointF(rect().width()/6, rect().height()/2);}
        double defaultScale() const override;

        int isSection(QPointF const &pointer);

        //Spline Sail
        void drawSplineLines(QPainter &painter);
        void drawSplineCtrlPoints(QPainter &painter);
        void drawSplineNormals(QPainter &painter);
        void drawSplineTangents(QPainter &painter);

        // NURBS Sail
        void drawFrameLines(QPainter &painter);
        void drawFramePoints(QPainter &painter);
        void drawFrameTangents(QPainter &painter);

        // Wing Sail
        void drawFoils(QPainter &painter);

        static bool bFill() {return s_bFill;}
        static void setFilled(bool b) {s_bFill=b;}

        static void setSectionStyle(LineStyle const&ls) {s_SectionStyle=ls;}
        static LineStyle &sectionStyle() {return s_SectionStyle;}

    public slots:
        void onInsertPt() override;
        void onRemovePt() override;
        void onShowNormals();
        void onFillFoil(bool bFill);

    signals:
        void sectionSelected(int);


    private:
        Sail *m_pSail;
        NURBSSurface *m_pNurbs;

        QVector<QPolygonF> m_FramePolyline;

        QAction *m_pShowNormals;

        static bool s_bFill;
        static int s_iSectionHighlight;
        static LineStyle s_SectionStyle;
};
