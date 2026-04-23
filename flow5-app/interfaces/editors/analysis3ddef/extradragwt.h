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

#include <QCheckBox>
#include <QStandardItemModel>


#include <api/extradrag.h>
#include <api/bspline.h>


class CPTableView;
class XflDelegate;
class SplinedGraphWt;
class Polar3d;
class PlanePolar;

class ExtraDragWt : public QWidget
{
    Q_OBJECT
    public:
        ExtraDragWt(QWidget *pParent=nullptr);
        ~ExtraDragWt();

        void initWt(const std::vector<ExtraDrag> &extradrag);
        void initWt(const PlanePolar *pPolar3d);
        void setExtraDragData(PlanePolar &pWPolar);
        void setExtraDragData(std::vector<ExtraDrag> &extra);

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

        std::vector<ExtraDrag> m_ExtraDrag;
        bool m_bAVLDrag;
        BSpline m_AVLSpline;

        bool m_bChanged;

};

