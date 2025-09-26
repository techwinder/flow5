/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDebug>
#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QAction>
#include <QMenu>

#include "analysisrangetable.h"
#include <xfl/xplane/analysis/analysis3dsettings.h>
#include <xflwidgets/customwts/xfldelegate.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>

QVector<AnalysisRange> AnalysisRangeTable::s_AlphaRange;
QVector<AnalysisRange> AnalysisRangeTable::s_ClRange;
QVector<AnalysisRange> AnalysisRangeTable::s_ReRange;
QVector<AnalysisRange> AnalysisRangeTable::s_ThetaRange;

QVector<AnalysisRange> AnalysisRangeTable::s_T12Range;
QVector<AnalysisRange> AnalysisRangeTable::s_T3Range;
QVector<AnalysisRange> AnalysisRangeTable::s_T5Range;
QVector<AnalysisRange> AnalysisRangeTable::s_T6Range;
QVector<AnalysisRange> AnalysisRangeTable::s_T7Range = {{false, 0,0,0}};
QVector<AnalysisRange> AnalysisRangeTable::s_BtRange;


AnalysisRangeTable::AnalysisRangeTable(QWidget *pParent) : CPTableView(pParent)
{
    m_PolarType = xfl::T1POLAR;
    m_bFoilPolar = false;
    m_eRangeType = AnalysisRange::ALPHA;

    QString tip(
            "<p>Use this table to define one or more ranges for &alpha; or the control parameter.<br>"
            "Click on the first column to activate/deactivate a range.<br>"
            "Use the context menu to add or remove ranges.<br>"
            "Duplicate requests will be removed before running the analysis.</p>");
    setToolTip(tip);

    setCharSize(3,5);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    setEditable(true);
    horizontalHeader()->setStretchLastSection(true);
    setEditTriggers(QAbstractItemView::EditKeyPressed |
                    QAbstractItemView::AnyKeyPressed  |
                    QAbstractItemView::DoubleClicked  |
                    QAbstractItemView::SelectedClicked);


    m_pRangeModel = new QStandardItemModel(this);
    m_pRangeModel->setRowCount(1);//temporary
    m_pRangeModel->setColumnCount(4);
    m_pRangeModel->setHeaderData(0, Qt::Horizontal, QString());
    m_pRangeModel->setHeaderData(1, Qt::Horizontal, "Min.");
    m_pRangeModel->setHeaderData(2, Qt::Horizontal, "Max.");
    m_pRangeModel->setHeaderData(3, Qt::Horizontal, DELTACAPCHAR);
    setModel(m_pRangeModel);

    m_pRangeDelegate = new XflDelegate(this);
    m_pRangeDelegate->setName("Ranges");
    m_pRangeDelegate->setCheckColumn(0);
    m_pRangeDelegate->setActionColumn(-1);
    m_pRangeDelegate->setDigits({0, 3, 3, 3});
    m_pRangeDelegate->setItemTypes({XflDelegate::CHECKBOX, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE});
    setItemDelegate(m_pRangeDelegate);


    int nCols=m_pRangeModel->columnCount()-1;

    QHeaderView *pVHeader = verticalHeader();
    pVHeader->setDefaultSectionSize(DisplayOptions::tableFontStruct().height()*2);
    pVHeader->setSectionResizeMode(QHeaderView::Fixed);

    QHeaderView *pHHeader = horizontalHeader();
    pHHeader->setSectionResizeMode(nCols, QHeaderView::Stretch);
    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically

    connectSignals();
}


AnalysisRangeTable::~AnalysisRangeTable()
{
    if(m_pRangeModel) delete m_pRangeModel;
    m_pRangeModel = nullptr;
}


void AnalysisRangeTable::connectSignals()
{
    connect(m_pRangeDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onRangeModelChanged()));
    connect(this,             SIGNAL(pressed(QModelIndex)),  SLOT(onRangeTableClicked(QModelIndex)));
}


/** debug use only */
void AnalysisRangeTable::setName(QString const &name)
{
    m_Name=name;
    m_pRangeDelegate->setName(name);
}


void AnalysisRangeTable::keyPressEvent(QKeyEvent *pEvent)
{
    if(pEvent->key()==Qt::Key_Space)
    {
        int row = currentIndex().row();

        QVector<AnalysisRange> *pRange = activeRange();
        if(pRange)
        {
            if(row>=0 && row<pRange->size())
            {
                QModelIndex ind = m_pRangeModel->index(row, 0);
                bool bChecked = m_pRangeModel->data(ind, Qt::UserRole).toBool();
                bChecked = !bChecked;
                m_pRangeModel->setData(ind, bChecked, Qt::UserRole);
                (*pRange)[row].setActive(bChecked);
                emit pressed(QModelIndex());
            }
        }

        return;
    }

    CPTableView::keyPressEvent(pEvent);
}


void AnalysisRangeTable::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QAction *pCopyAction = new QAction("Copy", this);
    pCopyAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_C));

    QAction *pPasteAction = new QAction("Paste", this);
    pPasteAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_V));
    pPasteAction->setEnabled(m_bIsEditable);

    QAction *pActivate     = new QAction("Activate/de-activate", this);
    pActivate->setShortcut(Qt::Key_Space);
    QAction *pMoveUp       = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),   "Move up", this);
    QAction *pMoveDown     = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Move down", this);
    QAction *pDuplicate    = new QAction("Duplicate", this);
    QAction *pDelete       = new QAction("Delete", this);
    QAction *pInsertBefore = new QAction("Insert before", this);
    QAction *pInsertAfter  = new QAction("Insert after", this);

    connect(pActivate,     SIGNAL(triggered(bool)), SLOT(onActivate()));
    connect(pMoveUp,       SIGNAL(triggered(bool)), SLOT(onMoveUp()));
    connect(pMoveDown,     SIGNAL(triggered(bool)), SLOT(onMoveDown()));
    connect(pDuplicate,    SIGNAL(triggered(bool)), SLOT(onDuplicateRow()));
    connect(pDelete,       SIGNAL(triggered(bool)), SLOT(onDeleteRow()));
    connect(pInsertBefore, SIGNAL(triggered(bool)), SLOT(onInsertBefore()));
    connect(pInsertAfter,  SIGNAL(triggered(bool)), SLOT(onInsertAfter()));
    connect(pCopyAction,   SIGNAL(triggered(bool)), SLOT(onCopySelection()));
    connect(pPasteAction,  SIGNAL(triggered(bool)), SLOT(onPaste()));

    QMenu *pRangeTableMenu = new QMenu("context menu", this);
    {
        pRangeTableMenu->addAction(pActivate);
        pRangeTableMenu->addSeparator();
        pRangeTableMenu->addAction(pMoveUp);
        pRangeTableMenu->addAction(pMoveDown);
        pRangeTableMenu->addSeparator();
        pRangeTableMenu->addAction(pInsertBefore);
        pRangeTableMenu->addAction(pInsertAfter);
        pRangeTableMenu->addAction(pDelete);
        pRangeTableMenu->addSeparator();
        pRangeTableMenu->addAction(pDuplicate);
        pRangeTableMenu->addSeparator();
        pRangeTableMenu->addAction(pCopyAction);
        pRangeTableMenu->addAction(pPasteAction);
    }
    pRangeTableMenu->exec(pEvent->globalPos());
}


void AnalysisRangeTable::showEvent(QShowEvent *pEvent)
{
    QFont fnt(DisplayOptions::tableFont());
    fnt.setPointSize(DisplayOptions::tableFont().pointSize());
    setFont(fnt);
    horizontalHeader()->setFont(fnt);
    verticalHeader()->setFont(fnt);
    onResizeColumns();
    if(pEvent) pEvent->ignore();
}


void AnalysisRangeTable::resizeEvent(QResizeEvent *pEvent)
{
    onResizeColumns();
    pEvent->ignore();
}


void AnalysisRangeTable::onResizeColumns()
{
    int nCols=m_pRangeModel->columnCount()-1;

    double w = double(width());
    int w0 = int(w/10.0);
    int w3 = int((w-w0)/double(3)*0.85);

    setColumnWidth(0, w0);
    for(int i=1; i<nCols; i++)
        setColumnWidth(i,w3);
}


void AnalysisRangeTable::fillTable()
{
    int row = 0;
    // fill the range Table

    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(pRange->size()==0) pRange->resize(1);
        m_pRangeModel->setRowCount(pRange->size());
        for(row=0; row<pRange->size(); row++)
        {
            AnalysisRange const & range = pRange->at(row);
            QModelIndex ind;
            ind = m_pRangeModel->index(row, 0, QModelIndex());
            m_pRangeModel->setData(ind, range.m_bActive, Qt::UserRole);

            ind = m_pRangeModel->index(row, 1, QModelIndex());
            m_pRangeModel->setData(ind, range.m_vMin);

            ind = m_pRangeModel->index(row, 2, QModelIndex());
            m_pRangeModel->setData(ind, range.m_vMax);

            ind = m_pRangeModel->index(row, 3, QModelIndex());
            m_pRangeModel->setData(ind, range.m_vInc);
         }
    }
    else
        m_pRangeModel->setRowCount(0);

}


void AnalysisRangeTable::setRowEnabled(int row, bool bEnabled)
{
    QModelIndex ind;
    for(int col=0; col<m_pRangeModel->columnCount(); col++)
    {
        ind = m_pRangeModel->index(row, col, QModelIndex());
        m_pRangeModel->setData(ind, bEnabled, Qt::UserRole); // used to display the row as enabled or disables
    }
}


int AnalysisRangeTable::readTable(QVector<AnalysisRange> &Range)
{
    Range.clear();

    if(m_pRangeModel->rowCount()<=0) return 0;

    bool bOk0=false, bOk1=false, bOk2=false;

    QString strange;
    bool bChecked = false;
    double vmin=0, vmax=0, vinc=0;
    for(int sel=0; sel<m_pRangeModel->rowCount(); sel++)
    {
        QModelIndex ind = m_pRangeModel->index(sel, 0);
        bChecked = m_pRangeModel->data(ind, Qt::UserRole).toBool();

        ind = m_pRangeModel->index(sel, 1);
        strange = m_pRangeModel->data(ind).toString();
        strange.replace(" ","");
        vmin = strange.toDouble(&bOk0);

        ind = m_pRangeModel->index(sel, 2);
        strange = m_pRangeModel->data(ind).toString();
        strange.replace(" ","");
        vmax = strange.toDouble(&bOk1);

        ind = m_pRangeModel->index(sel, 3);
        strange = m_pRangeModel->data(ind).toString();
        strange.replace(" ","");
        vinc = strange.toDouble(&bOk2);
        if(bOk0 && bOk1 && bOk2)
        {
            Range.append({bChecked, vmin, vmax, vinc}); // append even if inconsistent data - will be filtered later
        }
    }

    if(m_bFoilPolar)
    {
        switch(m_eRangeType)
        {
            default:
            case AnalysisRange::ALPHA:       s_AlphaRange = Range;    break;
            case AnalysisRange::CL:          s_ClRange    = Range;    break;
            case AnalysisRange::REYNOLDS:    s_ReRange    = Range;    break;
            case AnalysisRange::THETA:       s_ThetaRange = Range;    break;
        }
    }
    else
    {
        switch (m_PolarType)
        {
            case xfl::T1POLAR:
            case xfl::T2POLAR:
            {
                s_T12Range = Range;
                break;
            }
            case xfl::T3POLAR:
            {
                s_T3Range = Range;
                break;
            }
            case xfl::T5POLAR:
            {
                s_T5Range = Range;
                break;
            }
            case xfl::T6POLAR:
            {
                s_T6Range = Range;
                break;
            }
            case xfl::T7POLAR:
            {
                Range = {{true, 0,0,0}}; // dummy operating point
                s_T7Range = Range;
                break;
            }
            case xfl::BOATPOLAR:
            {
                s_BtRange = Range;
                break;
            }
            default: break;
        }
    }

    return Range.size();
}


void AnalysisRangeTable::onRangeModelChanged()
{
    QVector<AnalysisRange> ranges;
    int nRows = readTable(ranges);

    m_pRangeModel->setRowCount(nRows);

    setRowEnabled(nRows, false);
    update();
}


void AnalysisRangeTable::onRangeTableClicked(QModelIndex index)
{
    if(index.column()!=0) return;

    bool bActive = false;
    QVector<AnalysisRange> *pRange = activeRange();

    if(pRange)
    {
        if(index.row()>=pRange->size()) return;
        bActive = m_pRangeModel->data(index, Qt::UserRole).toBool(); // use a QVariant with the EditRole rather thant the QtCheckStateRole - not interested in Qt::PartiallyChecked
        bActive = !bActive; // toggle
        // update the range
        (*pRange)[index.row()].setActive(bActive);
    }

    setControls(m_PolarType); // deactivate lines
    update();
}


QVector<AnalysisRange> *AnalysisRangeTable::activeRange() const
{
    if(m_bFoilPolar)
    {
        switch(m_eRangeType)
        {
            default:
            case AnalysisRange::ALPHA:       return &s_AlphaRange;
            case AnalysisRange::CL:          return &s_ClRange;
            case AnalysisRange::REYNOLDS:    return &s_ReRange;
            case AnalysisRange::THETA:       return &s_ThetaRange;
        }
    }
    else
    {
        switch(m_PolarType)
        {
            case xfl::T1POLAR:     return &s_T12Range;
            case xfl::T2POLAR:     return &s_T12Range;
            case xfl::T3POLAR:     return &s_T3Range;
            case xfl::T5POLAR:     return &s_T5Range;
            case xfl::T6POLAR:     return &s_T6Range;
            case xfl::T7POLAR:     return &s_T7Range;
            case xfl::BOATPOLAR:   return &s_BtRange;
            default: break;
        }
    }
    return nullptr;
}


bool AnalysisRangeTable::hasActiveAnalysis() const
{
    QVector<AnalysisRange> const*pRange = activeRange();
    if(!pRange) return false;

    for(int row=0; row<pRange->size(); row++)
    {
        if(pRange->at(row).isActive() && pRange->at(row).nValues()>0) return true;
    }

    return false;
}


void AnalysisRangeTable::setControls(xfl::enumPolarType type)
{
    if(m_bFoilPolar)
    {

    }
    else
    {
        if(type!=m_PolarType || type==xfl::T7POLAR)
        {
            setEnabled(false);
            return;
        }
    }

    setEnabled(true);

    QVector<AnalysisRange> const*pRange = activeRange();
    if(pRange)
    {
        for(int row=0; row<pRange->size(); row++)
            setRowEnabled(row, pRange->at(row).isActive());
    }
}


void AnalysisRangeTable::onActivate()
{
    int row = currentIndex().row();

    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(row<0 || row>=pRange->size()) return;
        (*pRange)[row].setActive(!pRange->at(row).isActive());
        emit pressed(QModelIndex());
    }

    fillTable();
}


void AnalysisRangeTable::onMoveUp()
{
    int row = currentIndex().row();

    QVector<AnalysisRange> *pRange = activeRange();

    if(pRange)
    {
        if(row<=0 || row>=pRange->size()) return;
        AnalysisRange range = pRange->takeAt(row);
        pRange->insert(row-1, range);
        emit pressed(QModelIndex());
    }

     fillTable();
}


void AnalysisRangeTable::onMoveDown()
{
    int row = currentIndex().row();

    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(row<0 || row>=pRange->size()-1) return;
        AnalysisRange range = pRange->takeAt(row);
        pRange->insert(row+1, range);
        emit pressed(QModelIndex());
    }

    fillTable();
}


void AnalysisRangeTable::onDeleteRow()
{
    int row = currentIndex().row();

    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(row<0 || row>=pRange->size()) return;
        if(pRange->size()<=1) return; // leave at least one row
        pRange->takeAt(row);
        emit pressed(QModelIndex());
    }

    fillTable();
}


void AnalysisRangeTable::onDuplicateRow()
{
    int row = currentIndex().row();
    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(row<0) return;
        else
        {
            AnalysisRange range = pRange->at(row);
            pRange->insert(row+1, range);
        }
        emit pressed(QModelIndex());
    }

    fillTable();
}


void AnalysisRangeTable::onInsertBefore()
{
    int row = currentIndex().row();
    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(row<0)
            pRange->append(AnalysisRange(true, 0, 1, 0.5));
        else
            pRange->insert(row, AnalysisRange(true, 0, 1, 0.5));
        emit pressed(QModelIndex());
    }

    fillTable();
}


void AnalysisRangeTable::onInsertAfter()
{
    int row = currentIndex().row();
    QVector<AnalysisRange> *pRange = activeRange();
    if(pRange)
    {
        if(row<0)
            pRange->append(AnalysisRange(true, 0, 1, 0.5));
        else
            pRange->insert(row+1, AnalysisRange(true, 0, 1, 0.5));
        emit pressed(QModelIndex());
    }

    fillTable();
}


void AnalysisRangeTable::loadSettings(QSettings &settings)
{
    int nRange = -1;
    settings.beginGroup("AnalysisRangeTable");
    {
        nRange = settings.value("NAlphaRange", s_AlphaRange.size()).toInt();
        if(nRange>0)
        {
            s_AlphaRange.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_AlphaRange[i].setActive(settings.value(QString::asprintf("RangeAlpha_%d_bActive", i), true).toBool());
                s_AlphaRange[i].m_vMin = settings.value(QString::asprintf("RangeAlpha_%d_min", i), s_AlphaRange.at(i).m_vMin).toDouble();
                s_AlphaRange[i].m_vMax = settings.value(QString::asprintf("RangeAlpha_%d_max", i), s_AlphaRange.at(i).m_vMax).toDouble();
                s_AlphaRange[i].m_vInc = settings.value(QString::asprintf("RangeAlpha_%d_inc", i), s_AlphaRange.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_AlphaRange.resize(1);
            s_AlphaRange.first() = {true, 0.0, 1.0, 0.5};
        }

        nRange = settings.value("NClRange", s_ClRange.size()).toInt();
        if(nRange>0)
        {
            s_ClRange.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_ClRange[i].setActive(settings.value(QString::asprintf("RangeCl_%d_bActive", i), true).toBool());
                s_ClRange[i].m_vMin = settings.value(QString::asprintf("RangeCl_%d_min", i), s_ClRange.at(i).m_vMin).toDouble();
                s_ClRange[i].m_vMax = settings.value(QString::asprintf("RangeCl_%d_max", i), s_ClRange.at(i).m_vMax).toDouble();
                s_ClRange[i].m_vInc = settings.value(QString::asprintf("RangeCl_%d_inc", i), s_ClRange.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_ClRange.resize(1);
            s_ClRange.first() = {true, 0.0, 1.0, 0.5};
        }

        nRange = settings.value("NReynoldsRange", s_ReRange.size()).toInt();
        if(nRange>0)
        {
            s_ReRange.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_ReRange[i].setActive(settings.value(QString::asprintf("RangeReynolds_%d_bActive", i), true).toBool());
                s_ReRange[i].m_vMin = settings.value(QString::asprintf("RangeReynolds_%d_min", i), s_ReRange.at(i).m_vMin).toDouble();
                s_ReRange[i].m_vMax = settings.value(QString::asprintf("RangeReynolds_%d_max", i), s_ReRange.at(i).m_vMax).toDouble();
                s_ReRange[i].m_vInc = settings.value(QString::asprintf("RangeReynolds_%d_inc", i), s_ReRange.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_ReRange.resize(1);
            s_ReRange.first() = {true, 0.0, 1.0, 0.5};
        }

        nRange = settings.value("NThetaRange", s_ThetaRange.size()).toInt();
        if(nRange>0)
        {
            s_ThetaRange.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_ThetaRange[i].setActive(settings.value(QString::asprintf("RangeTheta_%d_bActive", i), true).toBool());
                s_ThetaRange[i].m_vMin = settings.value(QString::asprintf("RangeTheta_%d_min", i), s_ThetaRange.at(i).m_vMin).toDouble();
                s_ThetaRange[i].m_vMax = settings.value(QString::asprintf("RangeTheta_%d_max", i), s_ThetaRange.at(i).m_vMax).toDouble();
                s_ThetaRange[i].m_vInc = settings.value(QString::asprintf("RangeTheta_%d_inc", i), s_ThetaRange.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_ThetaRange.resize(1);
            s_ThetaRange.first() = {true, 0.0, 1.0, 0.5};
        }


        nRange = settings.value("NT12Range", s_T12Range.size()).toInt();
        if(nRange>0)
        {
            s_T12Range.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_T12Range[i].setActive(settings.value(QString::asprintf("RangeT12_%d_bActive", i), true).toBool());
                s_T12Range[i].m_vMin = settings.value(QString::asprintf("RangeT12_%d_min", i), s_T12Range.at(i).m_vMin).toDouble();
                s_T12Range[i].m_vMax = settings.value(QString::asprintf("RangeT12_%d_max", i), s_T12Range.at(i).m_vMax).toDouble();
                s_T12Range[i].m_vInc = settings.value(QString::asprintf("RangeT12_%d_inc", i), s_T12Range.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_T12Range.resize(1);
            s_T12Range.first() = {true, 0.0, 1.0, 0.5};
        }

        nRange = settings.value("NT3Range", s_T3Range.size()).toInt();
        if(nRange>0)
        {
            s_T3Range.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_T3Range[i].setActive(settings.value(QString::asprintf("RangeT3_%d_bActive", i), true).toBool());
                s_T3Range[i].m_vMin = settings.value(QString::asprintf("RangeT3_%d_min", i), s_T3Range.at(i).m_vMin).toDouble();
                s_T3Range[i].m_vMax = settings.value(QString::asprintf("RangeT3_%d_max", i), s_T3Range.at(i).m_vMax).toDouble();
                s_T3Range[i].m_vInc = settings.value(QString::asprintf("RangeT3_%d_inc", i), s_T3Range.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_T3Range.resize(1);
            s_T3Range.first() = {true, 0.0, 1.0, 0.5};
        }

        nRange = settings.value("NT5Range", s_T5Range.size()).toInt();
        if(nRange>0)
        {
            s_T5Range.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_T5Range[i].setActive(settings.value(QString::asprintf("RangeT5_%d_bActive", i), true).toBool());
                s_T5Range[i].m_vMin = settings.value(QString::asprintf("RangeT5_%d_min", i), s_T5Range.at(i).m_vMin).toDouble();
                s_T5Range[i].m_vMax = settings.value(QString::asprintf("RangeT5_%d_max", i), s_T5Range.at(i).m_vMax).toDouble();
                s_T5Range[i].m_vInc = settings.value(QString::asprintf("RangeT5_%d_inc", i), s_T5Range.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_T5Range.resize(1);
            s_T5Range.first() = {true, 0.0, 1.0, 0.5};
        }

        nRange = settings.value("NCtrlRange", s_T6Range.size()).toInt();
        if(nRange>0)
        {
            s_T6Range.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_T6Range[i].setActive(settings.value(QString::asprintf("RangeT6_%d_bActive", i), true).toBool());
                s_T6Range[i].m_vMin = settings.value(QString::asprintf("RangeT6_%d_min", i), s_T6Range.at(i).m_vMin).toDouble();
                s_T6Range[i].m_vMax = settings.value(QString::asprintf("RangeT6_%d_max", i), s_T6Range.at(i).m_vMax).toDouble();
                s_T6Range[i].m_vInc = settings.value(QString::asprintf("RangeT6_%d_inc", i), s_T6Range.at(i).m_vInc).toDouble();
            }
        }
        else
        {
            // force one line
            s_T6Range.resize(1);
            s_T6Range.first() = {true, 0.0, 1.0, 0.25};
        }

/*        nRange = settings.value("NStabRange", s_T7Range.size()).toInt();
        if(nRange>0)
        {
            s_T7Range.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_T7Range[i].setActive(settings.value(QString::asprintf("RangeT7_%d_bActive", i), true).toBool());
                s_T7Range[i].m_vMin = settings.value(QString::asprintf("RangeT7_%d_min", i), s_T7Range.at(i).m_vMin).toDouble();
                s_T7Range[i].m_vMax = settings.value(QString::asprintf("RangeT7_%d_max", i), s_T7Range.at(i).m_vMax).toDouble();
                s_T7Range[i].m_vInc = settings.value(QString::asprintf("RangeT7_%d_inc", i), s_T7Range.at(i).m_vInc).toDouble();
            }
        }
        else*/
        {
            // force one line
            s_T7Range.resize(1);
            s_T7Range.first() = {false, 0,0,0};
        }

        nRange = settings.value("NBoatRange", s_BtRange.size()).toInt();
        if(nRange>0)
        {
            s_BtRange.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_BtRange[i].setActive(settings.value(QString::asprintf("RangeBt_%d_bActive", i), true).toBool());
                s_BtRange[i].m_vMin = settings.value(QString::asprintf("RangeBt_%d_min", i), s_BtRange.at(i).m_vMin).toDouble();
                s_BtRange[i].m_vMax = settings.value(QString::asprintf("RangeBt_%d_max", i), s_BtRange.at(i).m_vMax).toDouble();
                s_BtRange[i].m_vInc = settings.value(QString::asprintf("RangeBt_%d_inc", i), s_BtRange.at(i).m_vInc).toDouble();
//                qDebug()<<"loading"<<s_BtRange.at(i).m_vMin<<s_BtRange.at(i).m_vMax<<s_BtRange.at(i).m_vInc;
            }
        }
        else
        {
            // force one line
            s_BtRange.resize(1);
            s_BtRange.first() = {true, 0.0, 1.0, 0.25};
        }
    }

    settings.endGroup();
}


void AnalysisRangeTable::saveSettings(QSettings &settings)
{
    settings.beginGroup("AnalysisRangeTable");
    {
        settings.setValue("NAlphaRange", s_AlphaRange.size());
        for(int i=0; i<s_AlphaRange.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeAlpha_%d_bActive", i), s_AlphaRange.at(i).isActive());
            settings.setValue(QString::asprintf("RangeAlpha_%d_min", i),     s_AlphaRange.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeAlpha_%d_max", i),     s_AlphaRange.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeAlpha_%d_inc", i),     s_AlphaRange.at(i).m_vInc);
        }

        settings.setValue("NClRange", s_ClRange.size());
        for(int i=0; i<s_ClRange.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeCl_%d_bActive", i), s_ClRange.at(i).isActive());
            settings.setValue(QString::asprintf("RangeCl_%d_min", i),     s_ClRange.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeCl_%d_max", i),     s_ClRange.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeCl_%d_inc", i),     s_ClRange.at(i).m_vInc);
        }

        settings.setValue("NReynoldsRange", s_ReRange.size());
        for(int i=0; i<s_ReRange.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeReynolds_%d_bActive", i), s_ReRange.at(i).isActive());
            settings.setValue(QString::asprintf("RangeReynolds_%d_min", i),     s_ReRange.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeReynolds_%d_max", i),     s_ReRange.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeReynolds_%d_inc", i),     s_ReRange.at(i).m_vInc);
        }

        settings.setValue("NThetaRange", s_ThetaRange.size());
        for(int i=0; i<s_ThetaRange.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeTheta_%d_bActive", i), s_ThetaRange.at(i).isActive());
            settings.setValue(QString::asprintf("RangeTheta_%d_min", i),     s_ThetaRange.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeTheta_%d_max", i),     s_ThetaRange.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeTheta_%d_inc", i),     s_ThetaRange.at(i).m_vInc);
        }

        settings.setValue("NT12Range", s_T12Range.size());
        for(int i=0; i<s_T12Range.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeT12_%d_bActive", i), s_T12Range.at(i).isActive());
            settings.setValue(QString::asprintf("RangeT12_%d_min", i), s_T12Range.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeT12_%d_max", i), s_T12Range.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeT12_%d_inc", i), s_T12Range.at(i).m_vInc);
        }

        settings.setValue("NT3Range", s_T3Range.size());
        for(int i=0; i<s_T3Range.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeT3_%d_bActive", i), s_T3Range.at(i).isActive());
            settings.setValue(QString::asprintf("RangeT3_%d_min", i), s_T3Range.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeT3_%d_max", i), s_T3Range.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeT3_%d_inc", i), s_T3Range.at(i).m_vInc);
        }

        settings.setValue("NT5Range", s_T5Range.size());
        for(int i=0; i<s_T5Range.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeT5_%d_bActive", i), s_T5Range.at(i).isActive());
            settings.setValue(QString::asprintf("RangeT5_%d_min", i), s_T5Range.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeT5_%d_max", i), s_T5Range.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeT5_%d_inc", i), s_T5Range.at(i).m_vInc);
        }

        settings.setValue("NCtrlRange", s_T6Range.size());
        for(int i=0; i<s_T6Range.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeT6_%d_bActive", i), s_T6Range.at(i).isActive());
            settings.setValue(QString::asprintf("RangeT6_%d_min", i), s_T6Range.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeT6_%d_max", i), s_T6Range.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeT6_%d_inc", i), s_T6Range.at(i).m_vInc);
        }

        settings.setValue("NStabRange", s_T7Range.size());
        for(int i=0; i<s_T7Range.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeT7_%d_bActive", i), s_T7Range.at(i).isActive());
            settings.setValue(QString::asprintf("RangeT7_%d_min", i), s_T7Range.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeT7_%d_max", i), s_T7Range.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeT7_%d_inc", i), s_T7Range.at(i).m_vInc);
        }

        settings.setValue("NBoatRange", s_BtRange.size());
        for(int i=0; i<s_BtRange.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeBt_%d_bActive", i), s_BtRange.at(i).isActive());
            settings.setValue(QString::asprintf("RangeBt_%d_min", i), s_BtRange.at(i).m_vMin);
            settings.setValue(QString::asprintf("RangeBt_%d_max", i), s_BtRange.at(i).m_vMax);
            settings.setValue(QString::asprintf("RangeBt_%d_inc", i), s_BtRange.at(i).m_vInc);
//            qDebug()<<"saving"<<s_BtRange.at(i).m_vMin<<s_BtRange.at(i).m_vMax<<s_BtRange.at(i).m_vInc;
        }
    }
    settings.endGroup();
}


QVector<double> AnalysisRangeTable::oppList()
{
    QVector<double> opps;

    QVector<AnalysisRange> ranges;
    readTable(ranges);

    for(int ia=0; ia<ranges.size(); ia++)
    {
        if(ranges.at(ia).isActive())
        {
            opps.append(ranges.at(ia).values());
        }
    }


    //sort and remove duplicates
    QVector<double> sorteduniquevalues;
    std::sort(opps.begin(), opps.end());
    double dlast = -1.e10;
    for(int i=0; i<opps.size(); i++)
    {
        if(fabs(opps.at(i)-dlast)>1.e-6) sorteduniquevalues.append(opps.at(i));
        dlast = opps.at(i);
    }

    return sorteduniquevalues;
}


QVector<AnalysisRange> AnalysisRangeTable::ranges()
{
    QVector<AnalysisRange> ranges;
    readTable(ranges);

    return ranges;
}
