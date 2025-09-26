/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

#include <xflgeom/geom3d/triangle3d.h>
#include <xflpanels/mesh/shell/edgesplit.h>

class IntEdit;
class FloatEdit;
class Segment3d;
class Sail;
class Vector2d;
class PSLG2d;
class AFMesher;

class MesherWt : public QGroupBox
{
    Q_OBJECT

    friend class FuseMesherDlg;
    friend class PartMergerDlg;
    friend class SailDlg;
    friend class OccSailDlg;

    public:
        MesherWt(QWidget *pParent);

        void hideEvent(QHideEvent*pEvent) override;

        void initDialog(const TopoDS_Shape &shape, const QVector<QVector<EdgeSplit>> &edgesplits, bool bSplittableInnerPSLG);
        void initDialog(const QVector<TopoDS_Shape> &shapes, bool bSplittableInnerPSLG);
        void initDialog(const TopoDS_ListOfShape &shells, double maxedgelength, bool bMakeXZSymmetric, bool bSplittableInnerPSLG);
        void initDialog(Sail *pSail);

        QVector<Triangle3d> const &triangles() const {return m_Triangles;}
        void setEdgeSplit(QVector<QVector<EdgeSplit>> &EdgeSplit) {m_EdgeSplit=EdgeSplit;}

        bool isMeshing() const {return m_bIsMeshing;}

        void showPickEdge(bool bShow) {m_pchPickEdge->setVisible(bShow);}
        void showDebugBox(bool bShow) {m_pTraceBox->setVisible(bShow);}

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
        QVector<QVector<EdgeSplit>> m_EdgeSplit; // for each face<each edge>
        bool m_bIsMeshing;
        bool m_bSplittableInnerPSLG;
        bool m_bMakexzSymmetric;
        QVector<Triangle3d> m_Triangles; /**< the resulting triangles */

        Sail *m_pSail;

        FloatEdit *m_pdeMaxEdgeSize;
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




