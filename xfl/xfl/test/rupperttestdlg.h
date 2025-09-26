/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QGroupBox>
#include <QLabel>
#include <QAbstractButton>
#include <QPushButton>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QSettings>

#include <xflmath/mathelem.h>
#include <xflgeom/geom_globals/ruppertmesher.h>


class IntEdit;
class FloatEdit;

class PSLGTestView;
class Vector2d;
class Segment2d;
class Triangle2d;

class PlainTextOutput;

class RuppertTestDlg : public QDialog
{
    friend class FuseMesherDlg;
	Q_OBJECT

    public:
        RuppertTestDlg(QWidget *parent = nullptr);
        ~RuppertTestDlg() = default;

        void initDialog();
        void initDialog(const PSLG2d &contourPSLG, const QVector<PSLG2d> &innerPSLG, double minedgelength, double maxedgelength);

        QSize sizeHint() const override {return QSize(700, 700);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onButton(QAbstractButton *pButton);
        void onDoAllSteps();
        void onMakeDelaunay();
        void onMakeConstrainedDelaunay();
        void onClearFoilPSLG();
        void onClearContourPSLG();
        void onMakePSLG();
        void onDoRuppert();
        void onIndexes();
        void onRestorePSLG();
        void onSplitOversizePSLG();
        void onSplitOversizeTriangles();
        void onFlipDelaunay();

    private:
        void setupLayout();
        void connectSignals();

        double makeUniqueVertexList(const QVector<Triangle2d> &m_pPanelList, QVector<Vector2d> &m_vertexList);
        void readParams();
        void setRuppParams();
        void makeContourPSLG();
        void makeFoilPSLG();

        void updateRuppStatus();

        void updateOutput(QString const &msg);

    private:
        RuppertMesher m_RuppMesher;
        PSLG2d m_ContourPSLG;
        PSLG2d m_FoilPSLG;  /** the hole*/
        PSLG2d m_PSLG;

        xfl::enumDistribution m_xDist, m_yDist;

        QGroupBox *m_pgbPSLG;
        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbDoAllSteps;
        QPushButton *m_ppbDefaultPSLG;
        QPushButton *m_ppbMakeDelaunay, *m_ppbMakeCDelaunay;
        QPushButton *m_ppbDoRuppert;
        QPushButton *m_ppbRestorePSLG, *m_ppbSplitOversizePSLG;
        QPushButton *m_ppbSplitOversize;
        QPushButton *m_ppbFlipDelaunay;

        QPushButton *m_ppbClearFoilPSLG, *m_ppbClearContourPSLG;

        QCheckBox *m_pchIncludeFoilPSLG, *m_pchIncludeFreeEdgePSLG;

        PSLGTestView *m_pRuppView;

        QRadioButton *m_prbTrianglePSLG, *m_prbComplexPSLG;

        FloatEdit *m_pdeYSide, *m_pdeYMid, *m_pdeXLength;
        FloatEdit *m_pdeFoilXPos, *m_pdeFoilYPos;
        IntEdit *m_pieNx, *m_pieNy;
        IntEdit *m_pieMaxMeshIters;
        IntEdit *m_pieMaxPanelCount;
        FloatEdit *m_pdeQualityBound;
        FloatEdit *m_pdeMinEdgeSize, *m_pdeMaxEdgeSize;

        PlainTextOutput *m_ppto;

        QCheckBox *m_pchTriIndexes, *m_pchPSLGIndexes;

        static bool s_bTrianglePSLG;

        static int s_Nx, s_Ny;
        static double s_ySide, s_yMid, s_xLength;
        static double s_FoilXPos, s_FoilYPos;

        static bool s_bIncludeFoilPSLG, s_bIncludeFreeEdgePSLG;
        static double s_MinEdgeLength, s_MaxEdgeLength;

        static int s_MaxMeshIters, s_MaxPanelCount;

        static QByteArray s_Geometry;
};

