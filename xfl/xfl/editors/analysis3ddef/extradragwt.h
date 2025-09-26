/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QStandardItemModel>


#include <xflobjects/objects3d/analysis/extradrag.h>
#include <xflgeom/geom2d/splines/bspline.h>


class CPTableView;
class XflDelegate;
class SplinedGraphWt;
class Polar3d;
class WPolar;

class ExtraDragWt : public QWidget
{
    Q_OBJECT
    public:
        ExtraDragWt(QWidget *pParent=nullptr);
        ~ExtraDragWt();

        void initWt(const QVector<ExtraDrag> &extradrag);
        void initWt(const WPolar *pPolar3d);
        void setExtraDragData(WPolar &pWPolar);
        void setExtraDragData(QVector<ExtraDrag> &extra);

    private:
        void setupLayout();
        void fillExtraDrag();
        void readExtraDragData();

    private slots:
        void onExtraDragChanged();

    private:
        CPTableView *m_pcptExtraDragTable;
        QStandardItemModel *m_pExtraDragModel;
        XflDelegate *m_pExtraDragDelegate;

        QCheckBox *m_pchAVLDrag;
        SplinedGraphWt *m_pParabolicGraphWt;

        QVector<ExtraDrag> m_ExtraDrag;
        bool m_bAVLDrag;
        BSpline m_AVLSpline;

        bool m_bChanged;

};

