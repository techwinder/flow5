/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QSettings>

#include <xflwidgets/view/grid.h>

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
        FloatEdit *m_pdeSpinDamping;

        QCheckBox *m_pchAntiAliasing;
        IntEdit *m_pieSelectionPixels;
        IntEdit *m_pieSymbolSize;

        LineBtn *m_plbHigh, *m_plbSelect;

        QVector<QGroupBox *>m_pGroupBox;

        GridControl *m_pGridControl;

        static Grid s_RefGrid;
        static bool s_bModified;
};

