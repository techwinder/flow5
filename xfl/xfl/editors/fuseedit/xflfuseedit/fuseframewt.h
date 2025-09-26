/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QLabel>

#include <xflwidgets/view/section2dwt.h>
#include <xflgeom/geom3d/vector3d.h>

class FuseXfl;

class FuseFrameWt : public Section2dWt
{
    Q_OBJECT

    public:
        FuseFrameWt(QWidget *pParent=nullptr, FuseXfl *pBody=nullptr);

        void setBody(FuseXfl *pBody);
        void drawFrameLines();
        void drawFramePoints();
        void drawScaleLegend(QPainter &painter);


        QPointF defaultOffset() const override {return QPointF(rect().width()/2, rect().height()/2);}
        double defaultScale() const override;

        void paint(QPainter &painter) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;

        int highlightPoint(QPointF const &real) override;
        int selectPoint(QPointF const &real) override;
        void dragSelectedPoint(double x, double y) override;

        void createContextMenu();
        void setFrameProperties(QString const &data);

    private:
        int isFrame(QPoint pointer);

    signals:
        void scaleBody(bool bFrameOnly);
        void insertPoint(Vector3d);
        void removePoint(int);
        void frameSelected(int);

    private slots:
        void onInsertPt() override;
        void onRemovePt() override;

        void onScaleFrame();
        void onShowCurFrameOnly();

    private:
        FuseXfl *m_pFuseXfl;
        QAction *m_pShowCurFrameOnly;

        QVector<QPolygonF> m_FramePolyline;

//        QLabel *m_plabFrameProps;

        static bool s_bCurFrameOnly;
};

