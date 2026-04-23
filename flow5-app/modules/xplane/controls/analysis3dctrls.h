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
#include <QStandardItemModel>
#include <QStackedWidget>



#include <api/analysisrange.h>

#include <api/t8opp.h>

class IntEdit;
class FloatEdit;
class XPlane;


class AnalysisRangeTable;
class T8RangeTable;

class Analysis3dCtrls : public QWidget
{
    Q_OBJECT

    public:
        Analysis3dCtrls(QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        void enableAnalyze(bool b) {m_ppbAnalyze->setEnabled(b);}

        void setParameterLabels();
        double minValue() const;
        double maxValue() const;
        double incValue() const;
        bool isSequentialAnalysis() const;
        void setAnalysisRange();

        std::vector<double> oppList() const;
        void getXRanges(std::vector<T8Opp> &ranges) const;

        static void setXPlane(XPlane*pXPlane) {s_pXPlane=pXPlane;}

    private:
        void setupLayout();
        void connectSignals();

    public slots:
        void onSetControls();

    private slots:
        void onOption();


    private:
        AnalysisRangeTable *m_pAnalysisRangeTable;
        T8RangeTable *m_pXRangeTable;

        QStackedWidget *m_pswTables;

        QCheckBox *m_pchStorePOpps;
        QCheckBox *m_pchStabDerivatives;


        QLabel *m_plabParamName;


        static XPlane *s_pXPlane;

    public:
        QPushButton *m_ppbAnalyze;
};

