/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QRadioButton>

class IntEdit;
class FloatEdit;

class SaveOptionsWt : public QWidget
{
    Q_OBJECT
    public:
        SaveOptionsWt(QWidget *parent = nullptr);
        void initWidget();
        void showBox(int iBox);

    private:
        void setupLayout();

    public slots:
        void onLastUsedDir();
        void onActiveDir();
        void onDatFoilDir();
        void onExportFormat();
        void onPlrPolarDir();
        void onXmlPolarDir();
        void onXmlPlaneDir();
        void onXmlWPolarDir();
        void onXmlScriptDir();
        void onCADDir();
        void onSTLDir();
        bool onCheckTempDir();
        void onTempDir();
        void readData();

    private:
        QVector<QGroupBox *>m_pGroupBox;

        QLineEdit *m_pleXmlPlaneDir, *m_pleXmlWPolarDir, *m_pleXmlScriptDir, *m_pleTempDir, *m_pleApplicationDir;
        QLineEdit *m_pleLastDir;
        QLineEdit *m_pleDatFoilDir, *m_plePlrPolarDir;
        QLineEdit *m_pleXmlPolarDir, *m_pleCADDir, *m_pleSTLDir;

        IntEdit *m_pieSaveInterval;
        QCheckBox *m_pchOpps, *m_pchPOpps, *m_pchBtOpps;
        QCheckBox *m_pchAutoSave, *m_pchAutoLoadLast;
        QCheckBox *m_pchCleanOnExit;

        QRadioButton *m_prbUseLastDir, *m_prbUseFixedDir;

        QCheckBox *m_pchXmlWingFoils;

        QRadioButton *m_pCSV, *m_pTXT;
        QCheckBox *m_pchSVGExportStyle, *m_pchSVGCloseTE, *m_pchSVGFillFoil;
        FloatEdit *m_pdeSVGScaleFactor, *m_pdeSVGMargin;
        QLineEdit *m_pleCsvSeparator;
};

