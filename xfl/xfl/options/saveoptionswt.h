/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

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

