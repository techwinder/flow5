/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QLabel>
#include <QVector>


#include <xflwidgets/view/section2dwt.h>
#include <xflgeom/geom2d/triangle2d.h>
#include <xflgeom/geom2d/segment2d.h>
#include <xflgeom/geom2d/pslg2d.h>

class PSLGTestView : public Section2dWt
{
    public:
        PSLGTestView(QWidget *pParent);

        void paint(QPainter &painter) override;
        int highlightPoint(QPointF const &real) override;
        int selectPoint(QPointF const &real) override;
        void dragSelectedPoint(double x, double y) override;
        void showEvent(QShowEvent *pEvent) override;

        QPointF defaultOffset() const override {return QPointF(rect().width()/5, rect().height()/2);}
        double defaultScale() const override {return double(rect().width())/m_RefLength *0.7;}

        void setReferenceLength();
        void setReferenceLength(double length) {m_RefLength=length;}

        void paintPoints(QPainter &painter);
        void paintTriangles(QPainter &painter);
        void paintIndexes(QPainter &painter);

        void setPoints(QVector<Vector2d> const &pts) {m_Points=pts;}

        void setTriangles(QVector<Triangle2d> const &PanelList)    {m_Triangles = PanelList;}
        void setTriangles(QVector<QVector<Triangle2d>> const &triangles);
        void setPSLG(PSLG2d const &PSLG) {m_PSLG = PSLG;}

        void setEncroachedSegments(QVector<int> const &EncroachedList) {m_EncroachedSegList  = EncroachedList;}
        void setSkinny(QVector<int>             const &SkinnyList)     {m_SkinnyTriList = SkinnyList;}
        void setLongTriangles(QVector<int>      const &LongTriList)    {m_LongTriList = LongTriList;}

        void setFlipList(QVector<int>const &fliplist) {m_FlipList=fliplist;}

        void clearFlipList() {m_FlipList.clear();}
        void clearTriangles() {m_Triangles.clear();}
        void clearPSLG() {m_PSLG.clear();}

        void clearEncroached() {m_EncroachedSegList.clear();}

        void listNonCompliant(double length);

        void showTriangleIndexes(bool bPaint) {m_bPaintTriangleIndexes=bPaint; update();}
        void showPSLGIndexes(bool bPaint)     {m_bPaintPSLGIndexes=bPaint; update();}


    public slots:
        void onInsertPt() override;
        void onRemovePt() override;

    private:
        int listSkinnyTriangles();
        int listLongTriangles(double length);
        int listEncroachedSegments();

    private:

        QVector<Triangle2d> m_Triangles;
        PSLG2d m_PSLG;

        QVector<int> m_EncroachedSegList, m_SkinnyTriList, m_LongTriList;
        QVector<int> m_FlipList;

        QVector<Vector2d> m_Points;

        int m_iSelected;
        bool m_bPaintTriangleIndexes;
        bool m_bPaintPSLGIndexes;
        double m_RefLength;
};
