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


/** @file
 * This file implements the interface for the analysis of the active Foil and Polar objects in QXDirect.
*/

#pragma once

#include <QDialog>
#include <QSettings>
#include <QFile>
#include <QTextEdit>

#include <QCheckBox>


class XFoilTask;
class Foil;
class Polar;
class OpPoint;
class PlainTextOutput;
struct AnalysisRange;

/**
* @class XFoilAnalysisDlg
* This class provides an interface to manage the analysis of the active Foil and Polar pair.
*/
class XFoilAnalysisDlg : public QDialog
{
	Q_OBJECT

    public:
        XFoilAnalysisDlg(QWidget *pParent=nullptr);
        ~XFoilAnalysisDlg();

        void initializeAnalysis(Foil *pFoil, Polar *pPolar, const QVector<AnalysisRange> &ranges);
        void start();

        void customEvent(QEvent * pEvent) override;


        OpPoint *lastOpp() {return m_pLastOpp;}
        bool hasErrors() const {return m_bErrors;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onCancelClose();
        void onKeepOpen(bool bChecked);
        void onOutputMessage(const QString &msg);

    private:
        void runAsync();


    signals:
        void analysisFinished();

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QSize sizeHint() const override;

        void setFileHeader();
        void setupLayout();
        void onTaskFinished();

   private:

        //variables
        PlainTextOutput *m_ppto;
        QPushButton* m_ppbCancel; /** @todo ButtonBox */
        QCheckBox* m_pchKeepOpen;

        bool m_bErrors;                /**< true if some points are unconverged. Used by the calling class to know if the window should be kept visible at the end of the analysis.>*/

        QFile *m_pXFile;               /**< a pointer to the log file>*/

        XFoilTask *m_pXFoilTask;       /**< A pointer to the instance of the INTERACTIVE XFoilTask>*/

        Foil *m_pFoil;
        Polar *m_pPolar;

        OpPoint *m_pLastOpp;


        static QByteArray s_WindowGeometry;
};









