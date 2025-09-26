/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHideEvent>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>

#include "editplrdlg.h"


#include <xflobjects/objects3d/analysis/boatpolar.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/analysis/wpolarext.h>
#include <xflfoil/objects2d/polar.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/xfldelegate.h>
#include <xflwidgets/customdlg/intvaluedlg.h>

QByteArray EditPlrDlg::s_WindowGeometry;
QByteArray EditPlrDlg::s_HSplitterSizes;

QVector<bool> EditPlrDlg::s_PolarVariables   = {true, true, true};
QVector<bool> EditPlrDlg::s_WPolarVariables  = {false, true, false, true, true};
QVector<bool> EditPlrDlg::s_BtPolarVariables = {true, true, true, true};


EditPlrDlg::EditPlrDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Polar data points");

    m_pPolar   = nullptr;
    m_pWPolar  = nullptr;
    m_pBtPolar = nullptr;

    m_pcptPoint = nullptr;
    m_pPointModel     = nullptr;
    m_pActionDelegate  = nullptr;

    m_bDataChanged = false;


    setupLayout();
    connectSignals();
}


void EditPlrDlg::setupLayout()
{
    m_pHSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        m_pHSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QFrame *pLeftSide = new QFrame;
        {
            QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
            {
                m_plabPlaneName = new QLabel;
                m_plabPolarName = new QLabel;

                m_pcptPoint = new CPTableView(this);

                m_pcptPoint->horizontalHeader()->setStretchLastSection(true);
                m_pcptPoint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                m_pcptPoint->setToolTip("Use Ctrl+C to copy selection to the clipboard");

                m_pPointModel = new ActionItemModel(this);
                m_pPointModel->setRowCount(10);//temporary
                m_pcptPoint->setModel(m_pPointModel);

                m_pActionDelegate = new XflDelegate(this);
                m_pcptPoint->setItemDelegate(m_pActionDelegate);


                pLeftSideLayout->addWidget(m_plabPlaneName);
                pLeftSideLayout->addWidget(m_plabPolarName);
                pLeftSideLayout->addWidget(m_pcptPoint);
            }

            pLeftSide->setLayout(pLeftSideLayout);
        }
        QFrame *pRightSide = new QFrame;
        {
            QVBoxLayout *pRightSideLayout = new QVBoxLayout;
            {
                // layout and checkboxes are added at initialization time depending on the polar type
                m_pScrollArea = new QScrollArea;
                {
                }
                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
                {
                    m_ppbResizeDataBtn = new QPushButton("Resize data");
                    m_pButtonBox->addButton(m_ppbResizeDataBtn, QDialogButtonBox::ActionRole);
                }

                pRightSideLayout->addWidget(m_pScrollArea);
                pRightSideLayout->addWidget(m_pButtonBox);
            }
            pRightSide->setLayout(pRightSideLayout);
        }

        m_pHSplitter->addWidget(pLeftSide);
        m_pHSplitter->addWidget(pRightSide);
        m_pHSplitter->setStretchFactor(0,1);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addWidget(m_pHSplitter);
    }

    setLayout(pMainLayout);
}


void EditPlrDlg::connectSignals()
{
    connect(m_pHSplitter,      SIGNAL(splitterMoved(int,int)),    SLOT(onSplitterMoved()));
    connect(m_pcptPoint,       SIGNAL(clicked(QModelIndex)),      SLOT(onTableClicked(QModelIndex)));
    connect(m_pcptPoint,       SIGNAL(dataPasted()),              SLOT(onDataChanged()));
    connect(m_pActionDelegate, SIGNAL(closeEditor(QWidget*)),     SLOT(onDataChanged()));

    connect(m_pButtonBox,      SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
}


void EditPlrDlg::onDataChanged()
{
    if(m_pWPolar && m_pWPolar->isExternalPolar())
        readWPolarExtData();
    m_bDataChanged=true;
}


void EditPlrDlg::initDialog(Polar *pPolar, WPolar*pWPolar, BoatPolar *pBtPolar)
{
    m_pPolar = pPolar;
    m_pWPolar = pWPolar;
    m_pBtPolar = pBtPolar;

    m_pcptPoint->setEditable(pWPolar && pWPolar->isExternalPolar());

    int nCols(0);

    if(m_pPolar)
    {
        nCols = Polar::variableCount()+1;
        m_pPointModel->setColumnCount(nCols);
        for(int i=0; i<Polar::variableCount(); i++)
        {
            m_pPointModel->setHeaderData(i, Qt::Horizontal, Polar::variableName(i));
        }
        m_pPointModel->setHeaderData(nCols-1, Qt::Horizontal, "Actions");
        m_pPointModel->setActionColumn(nCols-1);
        m_plabPlaneName->setText(m_pPolar->foilName());
        m_plabPolarName->setText(m_pPolar->name());
        createCheckBoxes();
        fillPolarData();
    }
    else if(m_pWPolar)
    {
        nCols = WPolar::variableCount()+1;
        m_pPointModel->setColumnCount(nCols);
        for(int i=0; i<WPolar::variableCount(); i++)
        {
            m_pPointModel->setHeaderData(i, Qt::Horizontal, WPolar::variableName(i));
        }

        m_pPointModel->setHeaderData(nCols-1, Qt::Horizontal, "Actions");
        m_pPointModel->setActionColumn(nCols-1);
        m_plabPlaneName->setText(m_pWPolar->planeName());
        m_plabPolarName->setText(m_pWPolar->name());
        createCheckBoxes();
        fillWPolarData();
    }
    else if(m_pBtPolar)
    {
        nCols = BoatPolar::variableCount()+1;
        m_pPointModel->setColumnCount(nCols);
        for(int i=0; i<BoatPolar::variableCount(); i++)
        {
            m_pPointModel->setHeaderData(i, Qt::Horizontal, BoatPolar::variableName(i));
        }

        m_pPointModel->setHeaderData(nCols-1, Qt::Horizontal, "Actions");
        m_pPointModel->setActionColumn(nCols-1);
        m_plabPlaneName->setText(m_pBtPolar->boatName());
        m_plabPolarName->setText(m_pBtPolar->name());
        createCheckBoxes();
        fillBtPolarData();
    }

    QVector<int>precision(nCols, 5);
    m_pActionDelegate->setDigits(precision);
    QVector<XflDelegate::enumItemType> itemtypes(nCols, XflDelegate::DOUBLE);
    itemtypes.back() = XflDelegate::ACTION;
    m_pActionDelegate->setItemTypes(itemtypes);
    m_pActionDelegate->setActionColumn(nCols-1);
}


void EditPlrDlg::createCheckBoxes()
{
    QStringList names;
    QVector<bool> checks;

    if(m_pPolar)
    {
        names = Polar::variableNames();
        checks = s_PolarVariables;
    }
    else if(m_pWPolar)
    {
        names = WPolar::variableNames();
        checks = s_WPolarVariables;
    }
    else if(m_pBtPolar)
    {
        names = BoatPolar::variableNames();
        checks = s_BtPolarVariables;
    }

    QFrame *m_pfrVariables = new QFrame;
    {
        QVBoxLayout *pVarFrameLayout = new QVBoxLayout;
        {
            for(int j=0; j<names.size(); j++)
            {
                QCheckBox *pCheckBox = new QCheckBox(names.at(j));
                pCheckBox->setChecked(checks.at(j));
                m_pchVariables.append(pCheckBox);
                if(m_pPolar)
                    connect(pCheckBox, &QCheckBox::clicked, this, &EditPlrDlg::onPolarVariable);
                else if(m_pWPolar)
                    connect(pCheckBox, &QCheckBox::clicked, this, &EditPlrDlg::onWPolarVariable);
                else if(m_pBtPolar)
                    connect(pCheckBox, &QCheckBox::clicked, this, &EditPlrDlg::onBtPolarVariable);
                pVarFrameLayout->addWidget(pCheckBox);
            }
        }
        m_pfrVariables->setLayout(pVarFrameLayout);
    }
    m_pScrollArea->setWidget(m_pfrVariables);
}


void EditPlrDlg::fillPolarData()
{
    m_pPointModel->setRowCount(m_pPolar->m_Alpha.size());
    QModelIndex index;
    for(int j=0; j<Polar::variableCount(); j++)
    {
        QVector<double> const &var = m_pPolar->getVariable(j);
        for (int i=0; i<var.size(); i++)
        {
            index = m_pPointModel->index(i, j, QModelIndex());
            m_pPointModel->setData(index, var[i]);
        }
    }
    m_pcptPoint->resizeRowsToContents();


    // set visible columns
    for(int j=0; j<s_PolarVariables.size(); j++)
    {
        if(!s_PolarVariables.at(j))
            m_pcptPoint->hideColumn(j);
    }
}


void EditPlrDlg::fillWPolarData()
{
    m_pPointModel->setRowCount(m_pWPolar->dataSize());
    QModelIndex index;
    for (int i=0; i<m_pWPolar->dataSize(); i++)
    {
        for(int j=0; j<WPolar::variableCount(); j++)
        {
            index = m_pPointModel->index(i, j, QModelIndex());
            m_pPointModel->setData(index, m_pWPolar->getVariable(j,i));
        }

    }
    m_pcptPoint->resizeRowsToContents();

    // set visible columns
    for(int j=0; j<s_WPolarVariables.size(); j++)
    {
        if(!s_WPolarVariables.at(j))
            m_pcptPoint->hideColumn(j);
    }
}


void EditPlrDlg::readWPolarExtData()
{
    if(!m_pWPolar || !m_pWPolar->isExternalPolar()) return;

    WPolarExt *pWPolarExt = dynamic_cast<WPolarExt*>(m_pWPolar);
    if(!pWPolarExt) return;

    if(pWPolarExt->dataSize()!=m_pPointModel->rowCount())
    {
        qDebug()<<"problem here";
        pWPolarExt->resizeData(m_pPointModel->rowCount()); //redundant
    }

    for(int irow=0; irow<m_pPointModel->rowCount(); irow++)
    {
        for(int icol=0; icol<pWPolarExt->variableCount(); icol++)
        {
            QModelIndex index = m_pPointModel->index(irow, icol);
            double val = m_pPointModel->data(index).toDouble();
            pWPolarExt->setData(icol, irow, val);
        }
    }
}


void EditPlrDlg::fillBtPolarData()
{
    m_pPointModel->setRowCount(m_pBtPolar->dataSize());
    QModelIndex index;
    for (int i=0; i<m_pBtPolar->dataSize(); i++)
    {
        for(int j=0; j<WPolar::variableCount(); j++)
        {
            index = m_pPointModel->index(i, j, QModelIndex());
            m_pPointModel->setData(index, m_pBtPolar->variable(j,i));
        }

    }
    m_pcptPoint->resizeRowsToContents();

    // set visible columns
    for(int j=0; j<s_BtPolarVariables.size(); j++)
    {
        if(!s_BtPolarVariables.at(j))
            m_pcptPoint->hideColumn(j);
    }
}


void EditPlrDlg::onPolarVariable()
{
    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if (!pSenderBox) return;
    for(int j=0; j<m_pchVariables.size(); j++)
    {
        if(pSenderBox==m_pchVariables.at(j))
        {
            s_PolarVariables[j] = !s_PolarVariables[j];
            if(s_PolarVariables[j])
                m_pcptPoint->showColumn(j);
            else
                m_pcptPoint->hideColumn(j);
            resizeColumns();
            return;
        }
    }
}


void EditPlrDlg::onWPolarVariable()
{
    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if (!pSenderBox) return;
    for(int j=0; j<m_pchVariables.size(); j++)
    {
        if(pSenderBox==m_pchVariables.at(j))
        {
            s_WPolarVariables[j] = !s_WPolarVariables[j];
            if(s_WPolarVariables[j])
                m_pcptPoint->showColumn(j);
            else
                m_pcptPoint->hideColumn(j);
            resizeColumns();
            return;
        }
    }
}


void EditPlrDlg::onBtPolarVariable()
{
    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());
    if (!pSenderBox) return;
    for(int j=0; j<m_pchVariables.size(); j++)
    {
        if(pSenderBox==m_pchVariables.at(j))
        {
            s_BtPolarVariables[j] = !s_BtPolarVariables[j];
            if(s_BtPolarVariables[j])
                m_pcptPoint->showColumn(j);
            else
                m_pcptPoint->hideColumn(j);
            resizeColumns();
            return;
        }
    }
}


void EditPlrDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    if(pEvent->matches(QKeySequence::Copy))
    {
        m_pcptPoint->copySelection();
        return;
    }

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            QDialog::reject();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void EditPlrDlg::onInsertRowBefore()
{
    QModelIndex index = m_pcptPoint->currentIndex();

    if(m_pPolar)
    {
        m_pPolar->insertPoint(index.row());
        fillPolarData();
        m_bDataChanged = true;
    }
    else if(m_pWPolar)
    {
        m_pWPolar->insertDataPointAt(index.row(), false);
        fillWPolarData();
        m_bDataChanged = true;
    }
    else if(m_pBtPolar)
    {
        m_pBtPolar->insertDataPointAt(index.row(), false);
        fillBtPolarData();
        m_bDataChanged = true;
    }

    m_pcptPoint->setCurrentIndex(index);

    m_pcptPoint->resizeRowsToContents();
}


void EditPlrDlg::onInsertRowAfter()
{
    QModelIndex index = m_pcptPoint->currentIndex();

    if(m_pPolar)
    {
        m_pPolar->insertPoint(index.row());
        fillPolarData();
        m_bDataChanged = true;
    }
    else if(m_pWPolar)
    {
        m_pWPolar->insertDataPointAt(index.row(), true);
        fillWPolarData();
        m_bDataChanged = true;
    }
    else if(m_pBtPolar)
    {
        m_pBtPolar->insertDataPointAt(index.row(), true);
        fillBtPolarData();
        m_bDataChanged = true;
    }
    m_pcptPoint->setCurrentIndex(index);

    m_pcptPoint->resizeRowsToContents();
}


void EditPlrDlg::onDeleteRow()
{
    QModelIndex index = m_pcptPoint->currentIndex();

    if(m_pPolar)
    {
        m_pPolar->removePoint(index.row());
        fillPolarData();
        m_bDataChanged = true;
    }
    else if(m_pWPolar)
    {
        m_pWPolar->removeAt(index.row());
        fillWPolarData();
        m_bDataChanged = true;
    }
    else if(m_pBtPolar)
    {
        m_pBtPolar->remove(index.row());
        fillBtPolarData();
        m_bDataChanged = true;
    }

    if(index.row()>=m_pPointModel->rowCount()-1)
    {
        index = m_pPointModel->index(m_pPointModel->rowCount()-1,0);
    }
    if(m_pPointModel->rowCount()) m_pcptPoint->setCurrentIndex(index);

    m_pcptPoint->resizeRowsToContents();
}


void EditPlrDlg::onDeleteAllPoints()
{
    if(m_pPolar)
    {
        m_pPolar->reset();
        fillPolarData();
        m_bDataChanged = true;

    }
    else if(m_pWPolar)
    {
        m_pWPolar->clearWPolarData();
        fillWPolarData();
        m_bDataChanged = true;
    }
}


void EditPlrDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
    else if (pButton==m_ppbResizeDataBtn) onResizeData();
}


void EditPlrDlg::accept()
{
    if(m_bDataChanged)
    {
        if(m_pPolar)
        {
        }
        else if(m_pWPolar && m_pWPolar->isExternalPolar())
        {
            readWPolarExtData();
        }
        else if(m_pBtPolar)
        {
        }
    }
    QDialog::accept();
    return;
}


void EditPlrDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);

    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);

    resizeColumns();

    m_pcptPoint->resizeRowsToContents();

    pEvent->accept();
}


void EditPlrDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();

    s_HSplitterSizes = m_pHSplitter->saveState();
    pEvent->accept();
}


void EditPlrDlg::resizeEvent(QResizeEvent*pEvent)
{
    if(!m_pPointModel || !m_pcptPoint) return;

    resizeColumns();
    pEvent->accept();
}


void EditPlrDlg::resizeColumns()
{
    int n = m_pPointModel->actionColumn();
    int m=m_pPointModel->columnCount()-1;
    for(int p=0; p<m_pPointModel->columnCount()-1; p++ )
    {
        if(m_pcptPoint->isColumnHidden(p)) m--; //no isColumnVisible() method
    }

    QHeaderView *pHHeader = m_pcptPoint->horizontalHeader();
    //pHHeader->setDefaultSectionSize(1);
    pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
    pHHeader->resizeSection(n, 1);

    double w = double(m_pcptPoint->width());
    int w15 = int(w/double(m+1));

    for(int i=0; i<m_pPointModel->columnCount()-1; i++)
        m_pcptPoint->setColumnWidth(i,w15);
}


void EditPlrDlg::onSplitterMoved()
{
    resizeColumns();
}


void EditPlrDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("EditPlrDlg");
    {
        s_WindowGeometry = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();

        int n=0;
        n = settings.value("PolarVarCount", s_PolarVariables.size()).toInt();
        s_PolarVariables.resize(std::max(n, Polar::variableCount()));
        for(int i=0; i<s_PolarVariables.size(); i++)
            s_PolarVariables[i] = settings.value(QString::asprintf("Polar_%d", i), false).toBool();

        n = settings.value("WPolarVarCount", s_WPolarVariables.size()).toInt();
        s_WPolarVariables.resize(std::max(n, WPolar::variableCount()));
        for(int i=0; i<s_WPolarVariables.size(); i++)
            s_WPolarVariables[i] = settings.value(QString::asprintf("WPolar_%d", i), false).toBool();

        n = settings.value("BtPolarVarCount", s_BtPolarVariables.size()).toInt();
        s_BtPolarVariables.resize(std::max(n, BoatPolar::variableCount()));
        for(int i=0; i<s_BtPolarVariables.size(); i++)
            s_BtPolarVariables[i] = settings.value(QString::asprintf("BtPolar_%d", i), false).toBool();
    }
    settings.endGroup();
}


void EditPlrDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("EditPlrDlg");
    {
        settings.setValue("WindowGeometry", s_WindowGeometry);
        settings.setValue("HSplitterSizes", s_HSplitterSizes);

        settings.setValue("PolarVarCount", s_PolarVariables.size());
        for(int i=0; i<s_PolarVariables.size(); i++)
            settings.setValue(QString::asprintf("Polar_%d", i), s_PolarVariables.at(i));

        settings.setValue("WPolarVarCount", s_WPolarVariables.size());
        for(int i=0; i<s_WPolarVariables.size(); i++)
            settings.setValue(QString::asprintf("WPolar_%d", i), s_WPolarVariables.at(i));

        settings.setValue("BtPolarVarCount", s_BtPolarVariables.size());
        for(int i=0; i<s_BtPolarVariables.size(); i++)
            settings.setValue(QString::asprintf("BtPolar_%d", i), s_BtPolarVariables.at(i));
    }
    settings.endGroup();
}


void EditPlrDlg::onTableClicked(QModelIndex index)
{
    if(!index.isValid()) return;

    if(index.column() == m_pPointModel->actionColumn())
    {
        QRect itemrect = m_pcptPoint->visualRect(index);
        QPoint menupos = m_pcptPoint->mapToGlobal(itemrect.topLeft());
        QMenu *pRowMenu = new QMenu("Section",this);
        QAction *pInsertRowBefore = new QAction("Insert before", this);
        QAction *pInsertRowAfter  = new QAction("Insert after", this);
        QAction *pDeleteRow = new QAction("Delete row",    this);
        connect(pInsertRowBefore, SIGNAL(triggered(bool)), SLOT(onInsertRowBefore()));
        connect(pInsertRowAfter,  SIGNAL(triggered(bool)), SLOT(onInsertRowAfter()));
        connect(pDeleteRow,       SIGNAL(triggered(bool)), SLOT(onDeleteRow()));
        pRowMenu->addAction(pInsertRowBefore);
        pRowMenu->addAction(pInsertRowAfter);
        pRowMenu->addAction(pDeleteRow);
        pRowMenu->exec(menupos);
    }
}


void EditPlrDlg::onResizeData()
{
    readWPolarExtData();

    if(m_pPolar)
    {

    }
    else if(m_pWPolar)
    {
        IntValueDlg dlg(this);
        dlg.setValue(m_pWPolar->dataSize());
        dlg.setLeftLabel("Nb. of data points:");
        if(dlg.exec()==QDialog::Accepted)
        {
            m_pWPolar->resizeData(dlg.value());
            fillWPolarData();
            m_bDataChanged = true;
        }
    }
    else if(m_pBtPolar)
    {

    }
}




