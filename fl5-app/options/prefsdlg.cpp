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

#define _MATH_DEFINES_DEFINED

#include <QApplication>
#include <QColorDialog>
#include <QFontDialog>
#include <QHeaderView>
#include <QStyleFactory>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QStandardItem>
#include <QToolTip>
#include <QAbstractItemView>

#include "prefsdlg.h"

#include <api/panelanalysis.h>
#include <api/sail.h>
#include <api/surface.h>
#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/view/section2doptions.h>
#include <options/saveoptionswt.h>
#include <options/unitswt.h>


int PrefsDlg::s_CurrentIndex = 0;

QString PrefsDlg::s_StyleName = "Fusion";


int PrefsDlg::s_ExitRow = 0;
int PrefsDlg::s_ExitChildRow = -1;

QByteArray PrefsDlg::s_HSplitterSizes;
QByteArray PrefsDlg::s_WindowGeometry;


PrefsDlg::PrefsDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
    connectSignals();
}


void PrefsDlg::connectSignals()
{
    connect(m_ptwOptionsTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(onItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    connect(m_pcbBackColor,          SIGNAL(clicked()),                    SLOT(onBackgroundColor()));
    connect(m_prbDark,               SIGNAL(clicked(bool)),                SLOT(onTheme()));
    connect(m_prbLight,              SIGNAL(clicked(bool)),                SLOT(onTheme()));
    connect(m_prbCustomStyle,        SIGNAL(clicked(bool)),                SLOT(onTheme()));
    connect(m_ptcbTextClr,           SIGNAL(clickedTB()),                  SLOT(onTextColor()));
    connect(m_ppbTextFont,           SIGNAL(clicked()),                    SLOT(onTextFont()));
    connect(m_ppbTableFont,          SIGNAL(clicked()),                    SLOT(onTableFont()));
    connect(m_ppbTreeFont,           SIGNAL(clicked()),                    SLOT(onTreeFont()));
    connect(m_ppbToolTipFont,        SIGNAL(clicked()),                    SLOT(onToolTipFont()));
    connect(m_pchLocale,             SIGNAL(clicked()),                    SLOT(onLocalization()));
    connect(m_pcbStyles,             SIGNAL(currentTextChanged(QString)),  SLOT(onStyleChanged(QString)));
    connect(m_pchStyleSheetOverride, SIGNAL(clicked(bool)),                SLOT(onStyleSheet(bool)));
    connect(m_pUnitsWt,              SIGNAL(unitsChanged()), m_p3dPrefsWt, SLOT(onUpdateUnits()));
}


void PrefsDlg::reject()
{
    onOK();
}


void PrefsDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
    m_pHSplitter->restoreState(s_HSplitterSizes);

    QModelIndex index = m_ptwOptionsTree->model()->index(s_ExitRow, 0);
    if(index.isValid())
    {
        QModelIndex subindex = m_ptwOptionsTree->model()->index(s_ExitChildRow, 0, index);
        if(subindex.isValid())
        {
            m_ptwOptionsTree->expand(subindex);
            m_ptwOptionsTree->setCurrentIndex(subindex);
        }
        else
        {
            m_ptwOptionsTree->setCurrentIndex(subindex);
        }
    }
}


void PrefsDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_HSplitterSizes = m_pHSplitter->saveState();
    s_WindowGeometry = saveGeometry();
}


void PrefsDlg::onOK()
{
    readData();
    s_CurrentIndex = m_ptwOptionsTree->currentIndex().row();
    accept();
}


void PrefsDlg::setupLayout()
{
    m_pStyleOptions = new QWidget(this);
    {
        m_pStyleOptions->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        QVBoxLayout *pWidgetStyleLayout = new QVBoxLayout;
        {
            QGroupBox *pAppStyle = new QGroupBox("Application style");
            {
                QVBoxLayout *pAppStyleLayout = new QVBoxLayout;
                {
                    m_pcbStyles = new QComboBox;
                    {
                        QString defaultStyle = "Fusion";

                        m_pcbStyles->addItems(QStyleFactory::keys());
                        m_pcbStyles->setCurrentIndex(m_pcbStyles->findText(defaultStyle));
                    }

                    m_pchStyleSheetOverride = new QCheckBox("Application dark mode override");
                    m_pchStyleSheetOverride->setToolTip("<p>Set a dark mode for the application's buttons, menus, toolbars and other widgets.<br>"
                                                        "Customize by editing the text file flow5_dark.css located in the application's directory.<br>"
                                                        "This option should be used together with the UI dark theme activated.</p>");

                    QHBoxLayout *pIconLayout = new QHBoxLayout;
                    {
                        QLabel *plabIconSize = new QLabel("Toolbar icon size:");
                        m_pieIconSize = new IntEdit;
                        QLabel *plabMaxIconsSize = new QLabel("pixels; hint only; application restart required");
                        pIconLayout->addWidget(plabIconSize);
                        pIconLayout->addWidget(m_pieIconSize);
                        pIconLayout->addWidget(plabMaxIconsSize);
                        pIconLayout->addStretch();
                    }

                    pAppStyleLayout->addWidget(m_pcbStyles);
                    pAppStyleLayout->addWidget(m_pchStyleSheetOverride);
                    pAppStyleLayout->addLayout(pIconLayout);
                }
                pAppStyle->setLayout(pAppStyleLayout);
                m_pStyleWidgets.push_back(pAppStyle);
            }

            QGroupBox *pThemeBox = new QGroupBox("Display theme");
            {
                QVBoxLayout *pThemeLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pThemeClrLayout = new QHBoxLayout;
                    {
                        m_prbDark   = new QRadioButton("Dark");
                        m_prbLight  = new QRadioButton("Light");
                        m_prbCustomStyle = new QRadioButton("Custom");
                        pThemeClrLayout->addWidget(m_prbDark);
                        pThemeClrLayout->addWidget(m_prbLight);
                        pThemeClrLayout->addWidget(m_prbCustomStyle);
                    }
                    QHBoxLayout *pBackLayout = new QHBoxLayout;
                    {
                        QLabel *pBackLab = new QLabel("Background");
                        pBackLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        m_pcbBackColor      = new ColorBtn(this);
                        pBackLayout->addWidget(pBackLab);
                        pBackLayout->addWidget(m_pcbBackColor);
                        pBackLayout->setStretchFactor(pBackLab, 1);
                        pBackLayout->setStretchFactor(m_pcbBackColor, 7);
                    }
                    QGroupBox *pFontBox = new QGroupBox("Fonts");
                    {
                        QGridLayout *pMainFontLayout = new QGridLayout;
                        {
                            QLabel *plabMain = new QLabel("Main display font:");
                            m_ppbTextFont = new QPushButton;
                            m_ppbTextFont->setToolTip("<p>The font used in the central views;\nuse preferably a fix-spaced font</p>");
                            m_ptcbTextClr  = new TextClrBtn(this);

                            QLabel *plabTable = new QLabel("Table font:");
                            m_ppbTableFont = new QPushButton;
                            m_ppbTableFont->setToolTip("<p>The font used to display the contents of tables and output panes</p>");

                            QLabel *plabTree = new QLabel("Object explorer font:");
                            m_ppbTreeFont = new QPushButton;
                            m_ppbTreeFont->setToolTip("<p>The font used to display the contents of the object explorer views</p>");

                            QLabel *plabToolTip = new QLabel("Tool tip font:");
                            m_ppbToolTipFont = new QPushButton;
                            m_ppbToolTipFont->setToolTip("<p>The font used to display pop-pup contextual tips</p>");

                            pMainFontLayout->addWidget(plabMain,         1,1, Qt::AlignRight);
                            pMainFontLayout->addWidget(m_ppbTextFont,    1,2);
                            pMainFontLayout->addWidget(m_ptcbTextClr,    1,3);
                            pMainFontLayout->addWidget(plabTable,        2,1, Qt::AlignRight);
                            pMainFontLayout->addWidget(m_ppbTableFont,   2,2);
                            pMainFontLayout->addWidget(plabTree,         3,1, Qt::AlignRight);
                            pMainFontLayout->addWidget(m_ppbTreeFont,    3,2);
                            pMainFontLayout->addWidget(plabToolTip,      4,1, Qt::AlignRight);
                            pMainFontLayout->addWidget(m_ppbToolTipFont, 4,2);
                            pMainFontLayout->setColumnStretch(4,1);
                        }
                        pFontBox->setLayout(pMainFontLayout);
                    }


                    pThemeLayout->addLayout(pThemeClrLayout);
                    pThemeLayout->addLayout(pBackLayout);
                    pThemeLayout->addWidget(pFontBox);
                }
                pThemeBox->setLayout(pThemeLayout);
            }
            m_pStyleWidgets.push_back(pThemeBox);

            QGroupBox *pMiscBox = new QGroupBox("Misc.");
            {
                QVBoxLayout *pMiscLayout = new QVBoxLayout;
                {
                    m_pchConfirmDiscard = new QCheckBox("Confirm discard command when exiting editors");

                    m_pchDontUseNativeMacDlg  = new QCheckBox("Don't use native color and font dialog (macOS)");
#ifdef Q_OS_MAC
                    m_pchDontUseNativeMacDlg->setEnabled(true);
                    QString tip ="Currently, the native dialog for font selection is never used, but this is likely to change in future Qt releases.";
                    m_pchDontUseNativeMacDlg->setToolTip(tip);
#else
                    m_pchDontUseNativeMacDlg->setEnabled(false);
#endif
                    QHBoxLayout *pScaleLayout = new QHBoxLayout;
                    {
                        QLabel *pLabMouseWheel = new QLabel("Mouse wheel scale factor:");
                        m_pdeScaleFactor = new FloatEdit;
                        m_pdeScaleFactor->setToolTip("Define the percentage increase by which the views should be zoomed in or out\n"
                                                     "when using the mouse wheel.");
                        QLabel *pLabComment = new QLabel("%; Set a negative value to reverse the direction");

                        pScaleLayout->addWidget(pLabMouseWheel);
                        pScaleLayout->addWidget(m_pdeScaleFactor);
                        pScaleLayout->addWidget(pLabComment);

                        pScaleLayout->addStretch();
                    }

                    pMiscLayout->addWidget(m_pchConfirmDiscard);
                    pMiscLayout->addWidget(m_pchDontUseNativeMacDlg);
                    pMiscLayout->addLayout(pScaleLayout);
                }
                pMiscBox->setLayout(pMiscLayout);
            }
            m_pStyleWidgets.push_back(pMiscBox);

            QGroupBox *pLocaleBox = new QGroupBox("Localization");
            {
                QHBoxLayout *pLocaleLayout = new QHBoxLayout;
                {
                    m_pchLocale = new QCheckBox("Use locale settings for number formatting");
                    m_plabLocalOutput = new QLabel("1.23456\n10,000");
                    pLocaleLayout->addWidget(m_pchLocale);
                    pLocaleLayout->addStretch();
                    pLocaleLayout->addWidget(m_plabLocalOutput);
                }
                pLocaleBox->setLayout(pLocaleLayout);
            }
            m_pStyleWidgets.push_back(pLocaleBox);


            for(int i=0; i<m_pStyleWidgets.size(); i++)
            {
                pWidgetStyleLayout->addWidget(m_pStyleWidgets.at(i));
            }
            pWidgetStyleLayout->addStretch();
        }
        m_pStyleOptions->setLayout(pWidgetStyleLayout);
    }

    m_pMultiThreadOptions = new QGroupBox("Multithreading");
    {
        QVBoxLayout *pAllLayout = new QVBoxLayout;
        {
            m_pchMultiThreading = new QCheckBox("Allow multithreading");
            connect(m_pchMultiThreading, SIGNAL(clicked(bool)), SLOT(onMultiThreading()));
            QString strong;
            strong = QString::asprintf("%2d", QThread::idealThreadCount());
            QLabel *pIdealCountLab = new QLabel("Maximum thread count supported by the OS = " + strong);
            pIdealCountLab->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            QHBoxLayout *pMaxThreadLayout = new QHBoxLayout;
            {
                QLabel *pMaxThreadLab = new QLabel("Maximum thread count to use = ");
                m_pieMaxThreadCount = new IntEdit(1);
                pMaxThreadLayout->addStretch();
                pMaxThreadLayout->addWidget(pMaxThreadLab);
                pMaxThreadLayout->addWidget(m_pieMaxThreadCount);
            }
            QHBoxLayout *pThreadPriorityLayout = new QHBoxLayout;
            {
                QLabel *pPriorityLabel = new QLabel("Thread priority");
                m_pcbThreadPriority = new QComboBox;
                {
                    m_pcbThreadPriority->addItem("Idle");
                    m_pcbThreadPriority->addItem("Lowest");
                    m_pcbThreadPriority->addItem("Low");
                    m_pcbThreadPriority->addItem("Normal");
                    m_pcbThreadPriority->addItem("High");
                    m_pcbThreadPriority->addItem("Highest");
                    m_pcbThreadPriority->addItem("TimeCritical");

                    QString strong;
                    strong  =
                              "IdlePriority:        \tscheduled only when no other threads are running.\n"
                              "LowestPriority:      \tscheduled less often than LowPriority.\n"
                              "LowPriority:         \tscheduled less often than NormalPriority.\n"
                              "NormalPriority:      \tthe default priority of the operating system.\n"
                              "HighPriority:        \tscheduled more often than NormalPriority.\n"
                              "HighestPriority:     \tscheduled more often than HighPriority.\n"
                              "TimeCriticalPriority:\tscheduled as often as possible.";
                    m_pcbThreadPriority->setToolTip(strong);
                }
                pThreadPriorityLayout->addStretch();
                pThreadPriorityLayout->addWidget(pPriorityLabel);
                pThreadPriorityLayout->addWidget(m_pcbThreadPriority);
            }

            pAllLayout->addWidget(m_pchMultiThreading);
            pAllLayout->addWidget(pIdealCountLab);
            pAllLayout->addLayout(pMaxThreadLayout);
            pAllLayout->addLayout(pThreadPriorityLayout);
            QLabel *pLabPriorityDisable = new QLabel("Linux OS does not support thread priority");
            pLabPriorityDisable->setStyleSheet("font: italic");
            pLabPriorityDisable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            pAllLayout->addWidget(pLabPriorityDisable);
#if defined Q_OS_MAC
            pLabPriorityDisable->hide();
#elif defined Q_OS_LINUX
            pLabPriorityDisable->show();
            m_pcbThreadPriority->setEnabled(false);
#else
            pLabPriorityDisable->hide();
#endif
        }
        pAllLayout->addStretch();
        m_pMultiThreadOptions->setLayout(pAllLayout);
    }

    m_pGraphOptionsWt    = new GraphOptions(this);
    m_pSaveOptionsWt     = new SaveOptionsWt(this);
    m_pUnitsWt           = new UnitsWt(this);
    m_p2dViewOptions     = new Section2dOptions(this);
    m_p3dPrefsWt         = new W3dPrefs(this);

    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        m_ptwOptionsTree = new QTreeWidget;

        m_ptwOptionsTree->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
        fillTreeWidget();

        QScrollArea * pScrollArea = new QScrollArea;
        {
            m_pPageStack = new QStackedWidget;
            m_pPageStack->addWidget(m_pStyleOptions);
            m_pPageStack->addWidget(m_pSaveOptionsWt);
            m_pPageStack->addWidget(m_pUnitsWt);
            m_pPageStack->addWidget(m_pGraphOptionsWt);
            m_pPageStack->addWidget(m_p2dViewOptions);
            m_pPageStack->addWidget(m_p3dPrefsWt);
            m_pPageStack->addWidget(m_pMultiThreadOptions);

            pScrollArea->setWidgetResizable(true);
            pScrollArea->setWidget(m_pPageStack);
        }
        m_pHSplitter->addWidget(m_ptwOptionsTree);
        m_pHSplitter->addWidget(pScrollArea);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void PrefsDlg::initWidgets()
{
    if(m_pcbStyles->findText(s_StyleName)>=0)
        m_pcbStyles->setCurrentIndex(m_pcbStyles->findText(s_StyleName));

    m_pchStyleSheetOverride->setChecked(DisplayOptions::bStyleSheetOverride());

    m_prbCustomStyle->setChecked(true);
    m_prbCustomStyle->setEnabled(false);

    m_pcbBackColor->setColor(DisplayOptions::backgroundColor());

    setButtonFonts();

    m_pchLocale->setChecked(xfl::isLocalized());
    onLocalization();

    m_pchDontUseNativeMacDlg->setChecked(xfl::dontUseNativeMacDlg());
    m_pdeScaleFactor->setValue(DisplayOptions::scaleFactor()*100.0);
    m_pieIconSize->setValue(DisplayOptions::iconSize());

    m_pUnitsWt->initWidget();
    m_pGraphOptionsWt->initWidget();

//    m_pOptionsTree->expandAll();
    m_ptwOptionsTree->header()->hide();

    m_pSaveOptionsWt->initWidget();

    m_p2dViewOptions->initWidgets();
    m_p3dPrefsWt->initWidgets();

    m_pieMaxThreadCount->setValue(xfl::g_MaxThreadCount);
    m_pchMultiThreading->setChecked(xfl::isMultiThreaded());
    m_pcbThreadPriority->setCurrentIndex(xfl::g_ThreadPriority);
    onMultiThreading();

    m_pchConfirmDiscard->setChecked(xfl::bConfirmDiscard());
}


void PrefsDlg::setButtonFonts()
{
    m_ptcbTextClr->setTextColor(DisplayOptions::textColor());
    m_ptcbTextClr->setBackgroundColor(DisplayOptions::backgroundColor());


    m_ppbTextFont->setText(DisplayOptions::textFont().family() + QString::asprintf(" %d",DisplayOptions::textFont().pointSize()));
    m_ppbTextFont->setFont(DisplayOptions::textFont());
    m_ptcbTextClr->setText("Text colour");
    QString stylestring = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFont().family().toStdString().c_str(),
                                           DisplayOptions::textFont().pointSize());
    m_ptcbTextClr->setStyleSheet(stylestring);


    QString tableFontName = DisplayOptions::tableFont().family() + QString(" %1").arg(DisplayOptions::tableFont().pointSize());
    m_ppbTableFont->setText(tableFontName);
    m_ppbTableFont->setFont(DisplayOptions::tableFont());

    QString treeFontName = DisplayOptions::treeFont().family() + QString(" %1").arg(DisplayOptions::treeFont().pointSize());
    m_ppbTreeFont->setText(treeFontName);
    m_ppbTreeFont->setFont(DisplayOptions::treeFont());

    QString tooltipFontName = DisplayOptions::toolTipFont().family() + QString(" %1").arg(DisplayOptions::toolTipFont().pointSize());
    m_ppbToolTipFont->setText(tooltipFontName);
    m_ppbToolTipFont->setFont(DisplayOptions::toolTipFont());
}


void PrefsDlg::onStyleChanged(QString const &StyleName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    qApp->setStyle(StyleName);
    s_StyleName = StyleName;
    QApplication::restoreOverrideCursor();
}


void PrefsDlg::onStyleSheet(bool bSheet)
{
    DisplayOptions::setStyleSheetOverride(bSheet);
    QFile stylefile;
    if(bSheet)
    {
        QString qssPathName =  qApp->applicationDirPath() + QDir::separator() +"/flow5_dark.css";
        QFileInfo fi(qssPathName);
        if(fi.exists())
            stylefile.setFileName(qssPathName);
        else
            stylefile.setFileName(QStringLiteral(":/qss/flow5_dark.css"));

        if (stylefile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            QString qsStylesheet = QString::fromLatin1(stylefile.readAll());
            qApp->setStyleSheet(qsStylesheet);
            stylefile.close();
            QApplication::restoreOverrideCursor();
        }
    }
    else
    {
        qApp->setStyleSheet(QString());
    }
}


void PrefsDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("Preferences");
    {
        s_WindowGeometry    = settings.value("Geometry",  s_WindowGeometry).toByteArray();
        s_HSplitterSizes    = settings.value("HSplitter", s_HSplitterSizes).toByteArray();
    }
    settings.endGroup();
}


void PrefsDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("Preferences");
    {
        settings.setValue("Geometry",               s_WindowGeometry);
        settings.setValue("HSplitter",              s_HSplitterSizes);
    }
    settings.endGroup();
}


void PrefsDlg::readData()
{
    DisplayOptions::setScaleFactor(m_pdeScaleFactor->value()/100.0);
    DisplayOptions::setIconSize(m_pieIconSize->value());
    xfl::setLocalized(m_pchLocale->isChecked());
    xfl::setDontUseNativeColorDlg(m_pchDontUseNativeMacDlg->isChecked());
    xfl::setConfirmDiscard(m_pchConfirmDiscard->isChecked());

    m_pSaveOptionsWt->readData();
    m_pGraphOptionsWt->readData();

    xfl::setMultiThreaded(m_pchMultiThreading->isChecked());
    xfl::setMaxThreadCount(m_pieMaxThreadCount->value());
    xfl::setThreadPriority(QThread::Priority(m_pcbThreadPriority->currentIndex()));

    m_p2dViewOptions->readData();
    m_p3dPrefsWt->readData();
}


void PrefsDlg::fillTreeWidget()
{
    m_ptwOptionsTree->setColumnCount(1);

    m_pStyleItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_pStyleItem->setText(0, "Application");
        QTreeWidgetItem *pSub0 = new QTreeWidgetItem(m_pStyleItem);
        pSub0->setText(0,"Style");
        m_pStyleItem->addChild(pSub0);

        pSub0 = new QTreeWidgetItem(m_pStyleItem);
        pSub0->setText(0,"Theme");
        m_pStyleItem->addChild(pSub0);

        pSub0 = new QTreeWidgetItem(m_pStyleItem);
        pSub0->setText(0,"Misc.");
        m_pStyleItem->addChild(pSub0);

        pSub0 = new QTreeWidgetItem(m_pStyleItem);
        pSub0->setText(0,"Localization");
        m_pStyleItem->addChild(pSub0);
    }

    m_pSaveItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_pSaveItem->setText(0,"Save, load, export");
        QTreeWidgetItem *pSub1;

        pSub1 = new QTreeWidgetItem(m_pSaveItem);
        pSub1->setText(0,"General");
        m_pSaveItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pSaveItem);
        pSub1->setText(0,"Directories");
        m_pSaveItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pSaveItem);
        pSub1->setText(0,"Export options");
        m_pSaveItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pSaveItem);
        pSub1->setText(0,"Foil to SVG");
        m_pSaveItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pSaveItem);
        pSub1->setText(0, "Operating points");
        m_pSaveItem->addChild(pSub1);
    }

    m_pUnitItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_pUnitItem->setText(0, "Units");
    }

    QTreeWidgetItem *m_pGraphItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_pGraphItem->setText(0, "Graphs");
        QTreeWidgetItem *pSub1;

        pSub1 = new QTreeWidgetItem(m_pGraphItem);
        pSub1->setText(0, "Fonts");
        m_pGraphItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pGraphItem);
        pSub1->setText(0, "Background");
        m_pGraphItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pGraphItem);
        pSub1->setText(0, "Padding");
        m_pGraphItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pGraphItem);
        pSub1->setText(0, "Axes and grids");
        m_pGraphItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pGraphItem);
        pSub1->setText(0, "Curves");
        m_pGraphItem->addChild(pSub1);

        pSub1 = new QTreeWidgetItem(m_pGraphItem);
        pSub1->setText(0, "Other");
        m_pGraphItem->addChild(pSub1);
    }

    m_p2dItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_p2dItem->setText(0, "2d views");
        QTreeWidgetItem *pSub2;

        pSub2 = new QTreeWidgetItem(m_p2dItem);
        pSub2->setText(0, "General");
        m_p2dItem->addChild(pSub2);

        pSub2 = new QTreeWidgetItem(m_p2dItem);
        pSub2->setText(0, "Axes and grids");
        m_p2dItem->addChild(pSub2);
    }

    m_p3dItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_p3dItem->setText(0, "3d views");
        QTreeWidgetItem *pSub2;

        pSub2 = new QTreeWidgetItem(m_p3dItem);
        pSub2->setText(0, "Colours");
        m_p3dItem->addChild(pSub2);

        pSub2 = new QTreeWidgetItem(m_p3dItem);
        pSub2->setText(0,"Tessellation");
        m_p3dItem->addChild(pSub2);

        pSub2 = new QTreeWidgetItem(m_p3dItem);
        pSub2->setText(0,"Other");
        m_p3dItem->addChild(pSub2);
    }

    QTreeWidgetItem *m_pMultiThreadItem = new QTreeWidgetItem(m_ptwOptionsTree);
    {
        m_pMultiThreadItem->setText(0, "Multithreading");
    }

//    m_pOptionsTree->insertTopLevelItems(0, items);
    m_ptwOptionsTree->setHeaderLabels(QStringList() << "Option");
}


void PrefsDlg::onItemChanged(QTreeWidgetItem*pNewItem, QTreeWidgetItem*)
{
    s_ExitRow =-1;
    s_ExitChildRow=-1;
    QTreeWidgetItem *pParentItem = pNewItem->parent();

    if(pParentItem)
    {
        s_ExitChildRow = pParentItem->indexOfChild(pNewItem);
        s_ExitRow = m_ptwOptionsTree->indexOfTopLevelItem(pParentItem);
    }
    else
    {
        s_ExitRow = m_ptwOptionsTree->indexOfTopLevelItem(pNewItem);
    }

    m_pPageStack->setCurrentIndex(s_ExitRow);

    switch(s_ExitRow)
    {
        case 0:
        {
            showStyleBox(s_ExitChildRow);
            break;
        }
        case 1:
        {
            m_pSaveOptionsWt->showBox(s_ExitChildRow);
            break;
        }
        case 3:
        {
            m_pGraphOptionsWt->showBox(s_ExitChildRow);
            break;
        }
        case 4:
        {
            m_p2dViewOptions->showBox(s_ExitChildRow);
            break;
        }
        case 5:
        {
            m_p3dPrefsWt->showBox(s_ExitChildRow);
            break;
        }
        default:
            break;
    }
}


void PrefsDlg::showStyleBox(int iBox)
{
    if(iBox<0)
    {
        for(int i=0; i<m_pStyleWidgets.size(); i++)
            m_pStyleWidgets[i]->setVisible(true);
    }
    else
    {
        for(int i=0; i<m_pStyleWidgets.size(); i++)
            m_pStyleWidgets[i]->setVisible(i==iBox);
    }
}


void PrefsDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)
        onOK();
}


void PrefsDlg::onTheme()
{
    if (m_prbDark->isChecked())
    {
        DisplayOptions::setTheme(DisplayOptions::DARKTHEME);
        DisplayOptions::setBackgroundColor(QColor(3, 7, 13));
        DisplayOptions::setTextColor(QColor(237,237,237));
        GraphOptions::setDefaults(true);
    }
    else if(m_prbLight->isChecked())
    {
        DisplayOptions::setTheme(DisplayOptions::LIGHTTHEME);
        DisplayOptions::setBackgroundColor(QColor(237, 237, 237));
        DisplayOptions::setTextColor(QColor(0,0,0));
        GraphOptions::setDefaults(false);
    }
    else
    {
        return;
    }
    m_pcbBackColor->setColor(DisplayOptions::backgroundColor());
    m_ptcbTextClr->setTextColor(DisplayOptions::textColor());
    m_ptcbTextClr->setBackgroundColor(DisplayOptions::backgroundColor());

    GraphOptions::setGraphModified(true);
    m_pGraphOptionsWt->setButtonColors();
}


void PrefsDlg::onBackgroundColor()
{
    QColor Color = QColorDialog::getColor(DisplayOptions::backgroundColor(), this, "Background colour", QColorDialog::ShowAlphaChannel);
    if(Color.isValid()) DisplayOptions::setBackgroundColor(Color);

    m_pcbBackColor->setColor(DisplayOptions::backgroundColor());
}


void PrefsDlg::onTextColor()
{
    QColor Color = QColorDialog::getColor(DisplayOptions::textColor());
    if(Color.isValid()) DisplayOptions::setTextColor(Color);
    m_ptcbTextClr->setTextColor(DisplayOptions::textColor());
}


void PrefsDlg::onTextFont()
{
    bool bOK(false);
    QFont TextFont;

    QFontDialog::FontDialogOptions dialogoptions = QFontDialog::MonospacedFonts;

    TextFont = QFontDialog::getFont(&bOK, DisplayOptions::textFont(), this,
                                    QString("Display font"), dialogoptions);

    if (bOK)
    {
        DisplayOptions::setTextFont(TextFont);
        setButtonFonts();
    }
}


void PrefsDlg::onTableFont()
{
    QFontDialog::FontDialogOptions dialogoptions = QFontDialog::MonospacedFonts;

    bool bOK(false);
    QFont TableFont = QFontDialog::getFont(&bOK, DisplayOptions::tableFont(), this,
                                           QString("Table font"), dialogoptions);

    if (bOK)
    {
        DisplayOptions::setTableFont(TableFont);
        setButtonFonts();
    }
}


void PrefsDlg::onTreeFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOK=false;
    QFont TreeFont = QFontDialog::getFont(&bOK, DisplayOptions::treeFont(), this, QString("Tree font"), dialogoptions);

    if (bOK)
    {
        DisplayOptions::setTreeFont(TreeFont);
        setButtonFonts();
    }
}


void PrefsDlg::onToolTipFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOK=false;
    QFont ToolTipFont = QFontDialog::getFont(&bOK, DisplayOptions::toolTipFont(), this, QString("Tooltip font"), dialogoptions);

    if (bOK)
    {
        QToolTip::setFont(ToolTipFont);
        DisplayOptions::setToolTipFont(ToolTipFont);
        setButtonFonts();
    }
}


void PrefsDlg::onMultiThreading()
{
    bool bMTH = m_pchMultiThreading->isChecked();
    m_pieMaxThreadCount->setEnabled(bMTH);

#if defined Q_OS_MAC
        m_pcbThreadPriority->setEnabled(bMTH);
#elif defined Q_OS_LINUX
        m_pcbThreadPriority->setEnabled(false);
#else
        m_pcbThreadPriority->setEnabled(bMTH);
#endif
}


void PrefsDlg::onLocalization()
{
    xfl::setLocalized(m_pchLocale->isChecked());
    double dble = 123456.789;
    QString dblestrange;
    if(xfl::isLocalized())
    {
        dblestrange = QString("%L1").arg(dble ,0,'f', 3);
    }
    else
    {
        dblestrange = QString::asprintf("%.3f", dble);
    }
    m_plabLocalOutput->setText(dblestrange);
}


