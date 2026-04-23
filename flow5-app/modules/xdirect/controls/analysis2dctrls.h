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

#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

#include <api/analysisrange.h>


class XDirect;
class FloatEdit;
class AnalysisRangeTable;

class Analysis2dCtrls : public QWidget
{
	Q_OBJECT

    public:
        Analysis2dCtrls(QWidget *pParent = nullptr);

        void enableAnalyze(bool b);

        QVector<AnalysisRange> ranges() const;

        static void setXDirect(XDirect *pXDirect) {s_pXDirect = pXDirect;}

    private:
        void setupLayout();
        void connectSignals();
        void fillAnalysisTable();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(50,500);}

    public slots:
        void onReadAnalysisData();
        void onSetControls();

    private slots:
        void onStoreOpp();
        void onSpec();
        void onInputChanged();

    private:
        QRadioButton *m_prbAlpha, *m_prbCl, *m_prbReynolds, *m_prbTheta;
        AnalysisRangeTable *m_pRangeTable;
        QPushButton *m_ppbAnalyze;

        QCheckBox *m_pchStoreOpp;

        static XDirect* s_pXDirect;
};

