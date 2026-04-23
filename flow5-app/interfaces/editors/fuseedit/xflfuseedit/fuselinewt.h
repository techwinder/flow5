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

#include <interfaces/widgets/view/section2dwt.h>
#include <api/vector3d.h>

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

