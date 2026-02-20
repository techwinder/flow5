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

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QSettings>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QSplitter>

#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/color/textclrbtn.h>
#include <interfaces/widgets/color/colorbtn.h>

class FloatEdit;
class GraphOptions;
class SaveOptionsWt;
class Section2dOptions;
class UnitsWt;
class PlainTextOutput;
class W3dPrefs;

class PrefsDlg : public QDialog
{
    Q_OBJECT

    public:
        PrefsDlg(QWidget *pParent=nullptr);

        void reject();
        void showEvent(QShowEvent *pEvent);
        void hideEvent(QHideEvent *pEvent);
        QSize sizeHint() const {return QSize(1000,800);}

        void initWidgets();


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setStyleName(QString const &name){s_StyleName=name;}
        static QString const &styleName()  {return s_StyleName;}

        static void setLanguage(QString const &lang) { s_Language = lang; }
        static QString const &language() { return s_Language; }


    private:
        void setupLayout();
        void connectSignals();
        void readData();
        void fillTreeWidget();
        void showStyleBox(int iBox);
        void setButtonFonts();

    private slots:
        void onBackgroundColor();
        void onButton(QAbstractButton *pButton);
        void onItemChanged(QTreeWidgetItem*pNewItem, QTreeWidgetItem *pPreviousItem);
        void onLocalization();
        void onLanguageChanged(int index);
        void onMultiThreading();
        void onOK();
        void onStyleChanged(const QString &StyleName);
        void onStyleSheet(bool);
        void onTableFont();
        void onTextColor();
        void onTextFont();
        void onTheme();
        void onToolTipFont();
        void onTreeFont();

    private:

        QSplitter *m_pHSplitter;

        QTreeWidget *m_ptwOptionsTree;
        QTreeWidgetItem *m_pStyleItem, *m_pSaveItem, *m_pDisplayItem, *m_pLanguageItem, *m_pUnitItem;
        QTreeWidgetItem *m_p2dItem, *m_p3dItem;

        QStackedWidget *m_pPageStack;

        QVector<QWidget*>m_pStyleWidgets;
        QRadioButton *m_prbDark, *m_prbLight, *m_prbCustomStyle;
        ColorBtn *m_pcbBackColor;
        TextClrBtn *m_ptcbTextClr;
        QPushButton *m_ppbTextFont, *m_ppbTableFont, *m_ppbTreeFont, *m_ppbToolTipFont;

        QWidget *m_pStyleOptions;
        QWidget *m_pMultiThreadOptions;
        SaveOptionsWt *m_pSaveOptionsWt;
        UnitsWt *m_pUnitsWt;
        GraphOptions *m_pGraphOptionsWt;
        Section2dOptions *m_p2dViewOptions;
        W3dPrefs *m_p3dPrefsWt;

        QCheckBox *m_pchMultiThreading;
        IntEdit *m_pieMaxThreadCount;
        QComboBox *m_pcbThreadPriority;

        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchConfirmDiscard;
        QCheckBox *m_pchDontUseNativeMacDlg;

        QCheckBox *m_pchLocale;
        QLabel *m_plabLocalOutput;
        QComboBox *m_pcbLanguage;

        FloatEdit *m_pdeScaleFactor;
        IntEdit *m_pieIconSize;

        QComboBox *m_pcbStyles;
        QCheckBox *m_pchStyleSheetOverride;

        static QString s_StyleName;
        static QString s_Language;



        static int s_CurrentIndex;

        static int s_ExitRow;
        static int s_ExitChildRow;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_WindowGeometry;
};


