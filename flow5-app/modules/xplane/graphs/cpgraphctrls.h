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
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>

#include <api/linestyle.h>


class Curve;
class LineBtn;
class MainFrame;
class XPlane;
class XSail;
class ExponentialSlider;
struct LineStyle;

class CpGraphCtrls : public QWidget
{
    Q_OBJECT

    public:
        CpGraphCtrls(MainFrame*pMainFrame, XPlane *pXPlane, XSail *pXSail);

        void setControls();

        int currentWingIndex() const {return m_pcbWingList->currentIndex();}
        int iStrip() const {return m_iStrip;}
        double spanRelPos() const {return m_SpanRelativePos;}
        double CpSectionScale() const;


    public slots:
        void onCpSectionSlider(int pos);
        void onCpScale(int);
        void onClearCpCurves();

    private:
        void setupLayout();
        void connectSignals();

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:

        QComboBox *m_pcbWingList;
        QPushButton *m_ppbKeepCpCurve, *m_ppbClearCpCurves;
        QSlider *m_pslCpSectionSlider;
        ExponentialSlider *m_peslScale;
//        QSlider *m_peslScale;

        double m_SpanRelativePos;        /**< Span position for the Cp Graph  */
        int m_iStrip;
        LineStyle m_CpLineStyle;    /**< the style of the lines displayed in the comboboxes*/


        MainFrame *s_pMainFrame;
        XPlane *s_pXPlane;
        XSail *s_pXSail;
};

