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
#include <QGroupBox>
#include <QSettings>

#include <interfaces/widgets/view/grid.h>

class FloatEdit;
class IntEdit;
class LineBtn;
class GridControl;

class Section2dOptions : public QWidget
{
    Q_OBJECT

    public:
        Section2dOptions(QWidget *pParent = nullptr);

        void initWidgets();
        void readData();
        void showBox(int iBox);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static bool isModified() {return s_bModified;}
        static void setModified(bool bModified);

        static Grid const &grid() {return s_RefGrid;}

    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onResetDefaults();
        void onSetModified(bool bModified);
        void onHighStyle(LineStyle ls);
        void onSelStyle(LineStyle ls);
        void onSpinAnimation(bool bSpin);

    private:
        QCheckBox *m_pchSpinAnimation;
        FloatEdit *m_pfeSpinDamping;

        QCheckBox *m_pchAntiAliasing;
        IntEdit *m_pieSelectionPixels;
        IntEdit *m_pieSymbolSize;

        LineBtn *m_plbHigh, *m_plbSelect;

        QVector<QGroupBox *>m_pGroupBox;

        GridControl *m_pGridControl;

        static Grid s_RefGrid;
        static bool s_bModified;
};

