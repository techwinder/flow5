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

#include <QLabel>

#include <interfaces/widgets/view/section2dwt.h>
#include <api/vector3d.h>

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

