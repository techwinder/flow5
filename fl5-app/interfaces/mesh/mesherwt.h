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

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>

#include <TopoDS_Shape.hxx>
#include <TopoDS_ListOfShape.hxx>

#include <api/triangle3d.h>
#include <api/edgesplit.h>

class IntEdit;
class FloatEdit;
class Segment3d;
class Sail;
class Vector2d;
class PSLG2d;
class AFMesher;

class MesherWt : public QFrame
{
    Q_OBJECT

    friend class FuseMesherDlg;
    friend class PartMergerDlg;
    friend class SailDlg;
    friend class SailOccDlg;

    public:
        MesherWt(QWidget *pParent);

        void hideEvent(QHideEvent*pEvent) override;

        void initWt(const TopoDS_Shape &shape, const std::vector<std::vector<EdgeSplit> > &edgesplits, bool bSplittableInnerPSLG);
        void initWt(const QVector<TopoDS_Shape> &shapes, bool bSplittableInnerPSLG);
        void initWt(const TopoDS_ListOfShape &shells, double maxedgelength, bool bMakeXZSymmetric, bool bSplittableInnerPSLG);
        void initWt(Sail *pSail);

        void setEdgeSplit(std::vector<std::vector<EdgeSplit>> &EdgeSplit) {m_EdgeSplit=EdgeSplit;}

        bool isMeshing() const {return m_bIsMeshing;}

        void showPickEdge(bool bShow) {m_pchPickEdge->setVisible(bShow);}
        void showDebugBox(bool bShow) {m_pTraceBox->setVisible(bShow);}

        QSize sizeHint() const override {return QSize(300,150);}
        QSize minimumSizeHint() const override {return QSize(100,50);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onMakeMesh();
        void onMakeSailMesh();
        void onReadParams();
        void onMaxEdgeSize();
        void onMeshFinished();
        void onReadMeshDebugParams();
        void onAnimation();
        void onSlittableInnerPSLG();

    signals:
        void updateFuseView();
        void updateShapeView();
        void outputMsg(QString);

    private:
        void setupLayout();
        void connectSignals();
        void setControls();

    private:
        QWidget *m_pParent;

        TopoDS_ListOfShape m_Shapes;

        AFMesher *m_pMesher; // build it on the heap so that it can be moved to a distinct thread?
        std::vector<std::vector<EdgeSplit>> m_EdgeSplit; // for each face<each edge>
        bool m_bIsMeshing;
        bool m_bSplittableInnerPSLG;
        bool m_bMakexzSymmetric;
        std::vector<Triangle3d> m_Triangles; /**< the resulting triangles */

        Sail *m_pSail;

        FloatEdit *m_pfeMaxEdgeSize;
        IntEdit *m_pieMaxPanelCount;
        FloatEdit *m_pdeNodeMergeDistance;
        FloatEdit *m_pdeSearchRadiusFactor;
        FloatEdit *m_pdeGrowthFactor;
        QCheckBox *m_pchDelaunayFlip;

        QPushButton *m_ppbMakeTriMesh;

        QGroupBox *m_pTraceBox;

        IntEdit *m_pieFaceIdx, *m_pieIter;
        QCheckBox *m_pchPickEdge;

        QCheckBox *m_pchAnimate;
        IntEdit *m_pieAnimInterval;

        QCheckBox *m_pchSplitInnerPSLG;

};




