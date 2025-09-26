/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflwidgets/view/section2dwt.h>
#include <xflgeom/geom3d/vector3d.h>

class FuseXfl;

class FuseLineWt : public Section2dWt
{
    Q_OBJECT

    public:
        FuseLineWt(QWidget *pParent, FuseXfl *pBody=nullptr);

        void setXflFuse(FuseXfl *pBody);
        void drawFuseLines();
        void drawFusePoints();

        void paint(QPainter &painter) override;

        QPointF defaultOffset() const override {return QPointF(rect().width()/4.0, rect().height()/2.0);}
        double defaultScale() const override;

        int highlightPoint(const QPointF &real) override;
        int selectPoint(QPointF const &real) override;
        void dragSelectedPoint(double x, double y) override;
        void createContextMenu();

    signals:
        void scaleFuse(bool bFrameOnly);
        void translateFuse();
        void insertFrame(Vector3d);
        void removeFrame(int);

    private slots:
        void onInsertPt() override;
        void onRemovePt() override;
        void onScaleBody();
        void onTranslateBody();

    private:
        FuseXfl *m_pFuseXfl;
};

