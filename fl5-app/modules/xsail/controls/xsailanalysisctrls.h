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
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSettings>

#include <api/analysisrange.h>


class AnalysisRangeTable;

class XSail;
class BoatPolar;

class XSailAnalysisCtrls : public QWidget
{
    Q_OBJECT

    friend class XSail;

    public:
        XSailAnalysisCtrls(XSail *pXSail);

        void showEvent(QShowEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        std::vector<double> oppList() const;

        void setPolar3d(BoatPolar const *pBtPolar);

        static void setXPlane(XSail*pXSail) {s_pXSail=pXSail;}

    private:
        void setupLayout();
        void connectSignals();
        void setAnalysisRange();

    private slots:
        void onStoreBtOpp();
        void onSetControls();

    private:
        AnalysisRangeTable *m_pAnalysisRangeTable;


        QCheckBox *m_pchStoreBtOpps;
        QPushButton *m_ppbAnalyze;

        static XSail *s_pXSail;
};


