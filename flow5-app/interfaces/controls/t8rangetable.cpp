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

#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>


#include "t8rangetable.h"
#include <modules/xplane/analysis/analysis3dsettings.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/units.h>
#include <api/utils.h>


std::vector<T8Opp> T8RangeTable::s_T8Range;


T8RangeTable::T8RangeTable(QWidget *pParent) : CPTableView(pParent)
{
    QString tip(
            "<p>Use this table to define one or more ranges for &alpha; or the control parameter.<br>"
            "Click on the first column to activate/deactivate a range.<br>"
            "Use the context menu to add or remove ranges.<br>"
            "Duplicate requests will be removed before running the analysis.</p<");
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
    m_pRangeModel->setHeaderData(1, Qt::Horizontal, ALPHAch);
    m_pRangeModel->setHeaderData(2, Qt::Horizontal, BETAch);
    m_pRangeModel->setHeaderData(3, Qt::Horizontal, QString("V")+INFch + " ("+Units::speedUnitQLabel()+")");
    setModel(m_pRangeModel);

    m_pRangeDelegate = new XflDelegate(this);
    m_pRangeDelegate->setName("T8 ranges");
    m_pRangeDelegate->setCheckColumn(0);
    m_pRangeDelegate->setActionColumn(-1);
    m_pRangeDelegate->setDigits({0, 3,3,3});
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


T8RangeTable::~T8RangeTable()
{
    if(m_pRangeModel) delete m_pRangeModel;
    m_pRangeModel = nullptr;
}


void T8RangeTable::connectSignals()
{
    connect(m_pRangeDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onRangeModelChanged()));
    connect(this,             SIGNAL(pressed(QModelIndex)),  SLOT(onRangeTableClicked(QModelIndex)));
}


/** debug use only */
void T8RangeTable::setName(QString const &name)
{
    m_Name=name;
    m_pRangeDelegate->setName(name);
}


void T8RangeTable::keyPressEvent(QKeyEvent *pEvent)
{
    if(pEvent->key()==Qt::Key_Space)
    {
        int row = currentIndex().row();

        if(row>=0 && row<int(s_T8Range.size()))
        {
            QModelIndex ind = m_pRangeModel->index(row, 0);
            bool bChecked = m_pRangeModel->data(ind, Qt::UserRole).toBool();
            bChecked = !bChecked;
            m_pRangeModel->setData(ind, bChecked, Qt::UserRole);
            s_T8Range[row].m_bActive = bChecked;
            emit pressed(QModelIndex());
        }

        return;
    }

    CPTableView::keyPressEvent(pEvent);
}


void T8RangeTable::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QAction *pCopyAction = new QAction("Copy", this);
    pCopyAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_C));

    QAction *pPasteAction = new QAction("Paste", this);
    pPasteAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_V));
    pPasteAction->setEnabled(m_bIsEditable);

    QAction *pActivate     = new QAction("Activate/de-activate", this);
    pActivate->setShortcut(Qt::Key_Space);
    QAction *pMoveUp       = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),   "Move Up",   this);
    QAction *pMoveDown     = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Move Down", this);
    QAction *pDuplicate    = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Duplicate", this);
    QAction *pDelete       = new QAction("Delete", this);
    QAction *pInsertBefore = new QAction("Insert before", this);
    QAction *pInsertAfter  = new QAction("Insert after", this);

    connect(pActivate,     SIGNAL(triggered(bool)), SLOT(onActivate()));
    connect(pMoveUp,       SIGNAL(triggered(bool)), SLOT(onMoveUp()));
    connect(pMoveDown,     SIGNAL(triggered(bool)), SLOT(onMoveDown()));
    connect(pDelete,       SIGNAL(triggered(bool)), SLOT(onDeleteRow()));
    connect(pDuplicate,    SIGNAL(triggered(bool)), SLOT(onDuplicateRow()));
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


void T8RangeTable::showEvent(QShowEvent *pEvent)
{
    QFont fnt(DisplayOptions::tableFont());
    fnt.setPointSize(DisplayOptions::tableFont().pointSize());
    setFont(fnt);
    horizontalHeader()->setFont(fnt);
    verticalHeader()->setFont(fnt);
    onResizeColumns();
    if(pEvent) pEvent->ignore();
}


void T8RangeTable::resizeEvent(QResizeEvent *pEvent)
{
    onResizeColumns();
    pEvent->ignore();
}


void T8RangeTable::onResizeColumns()
{
    int nCols=m_pRangeModel->columnCount()-1;

    QHeaderView *pHHeader = horizontalHeader();
    //pHHeader->setDefaultSectionSize(1);
    pHHeader->setSectionResizeMode(nCols, QHeaderView::Stretch);
    pHHeader->resizeSection(nCols, 1); // 1 pixel to be resized automatically

    double w = double(width());
    int w0 = int(w/10.0);
    int w3 = int((w-w0)/double(3)*0.85);

    setColumnWidth(0, w0);
    for(int i=1; i<nCols; i++)
        setColumnWidth(i,w3);
}


void T8RangeTable::fillRangeTable()
{

    if(s_T8Range.size()==0) s_T8Range.resize(1);
    m_pRangeModel->setRowCount(int(s_T8Range.size()));
    for(uint row=0; row<s_T8Range.size(); row++)
    {
        T8Opp const & range = s_T8Range.at(row);
        QModelIndex ind;
        ind = m_pRangeModel->index(row, 0, QModelIndex());
        m_pRangeModel->setData(ind, range.m_bActive, Qt::UserRole);

        ind = m_pRangeModel->index(row, 1, QModelIndex());
        m_pRangeModel->setData(ind, range.m_Alpha);

        ind = m_pRangeModel->index(row, 2, QModelIndex());
        m_pRangeModel->setData(ind, range.m_Beta);

        ind = m_pRangeModel->index(row, 3, QModelIndex());
        m_pRangeModel->setData(ind, range.m_Vinf*Units::mstoUnit());
     }

}


void T8RangeTable::setRowEnabled(int row, bool bEnabled)
{
    QModelIndex ind;
    for(int col=0; col<m_pRangeModel->columnCount(); col++)
    {
        ind = m_pRangeModel->index(row, col, QModelIndex());
        m_pRangeModel->setData(ind, bEnabled, Qt::UserRole); // used to display the row as enabled or disables
    }
}


int T8RangeTable::readRangeTable(std::vector<T8Opp> &Range, bool bActiveOnly)
{
    s_T8Range.clear();

    if(m_pRangeModel->rowCount()<=0) return 0;

    bool bOk0(false), bOk1(false), bOk2(false);

    QString strange;
    bool bChecked(false);
    double alpha(0), beta(0), vinf(0);

    for(int sel=0; sel<m_pRangeModel->rowCount(); sel++)
    {
        QModelIndex ind = m_pRangeModel->index(sel, 0);
        bChecked = m_pRangeModel->data(ind, Qt::UserRole).toBool();

        ind = m_pRangeModel->index(sel, 1);
        strange = m_pRangeModel->data(ind).toString();
        strange.replace(" ","");
        alpha = strange.toDouble(&bOk0);

        ind = m_pRangeModel->index(sel, 2);
        strange = m_pRangeModel->data(ind).toString();
        strange.replace(" ","");
        beta = strange.toDouble(&bOk1);

        ind = m_pRangeModel->index(sel, 3);
        strange = m_pRangeModel->data(ind).toString();
        strange.replace(" ","");
        vinf = strange.toDouble(&bOk2)/Units::mstoUnit();
        if(bOk0 && bOk1 && bOk2)
        {
            s_T8Range.push_back({bChecked, alpha, beta, vinf}); // append even if inconsistent data - will be filtered later
        }
    }

    Range.clear();
    if(bActiveOnly)
    {
        for(uint i=0; i<s_T8Range.size(); i++)
            if(s_T8Range.at(i).m_bActive) Range.push_back(s_T8Range.at(i));
    }
    else Range = s_T8Range;

    return int(Range.size());
}


void T8RangeTable::onRangeModelChanged()
{
    std::vector<T8Opp> ranges;

    readRangeTable(ranges, false); // to update XRange

    update();

    emit xRangeChanged();
}


void T8RangeTable::onRangeTableClicked(QModelIndex index)
{
    if(index.column()!=0) return;

    bool bActive(false);


    if(index.row()>=int(s_T8Range.size())) return;
    bActive = m_pRangeModel->data(index, Qt::UserRole).toBool(); // use a QVariant with the EditRole rather than the Qt::CheckStateRole - not interested in Qt::PartiallyChecked
    bActive = !bActive; // toggle
    // update the range
    s_T8Range[index.row()].m_bActive = bActive;


    setControls(); // deactivate lines
    update();
}


bool T8RangeTable::hasActiveAnalysis() const
{
    for(uint row=0; row<s_T8Range.size(); row++)
    {
        if(s_T8Range.at(row).m_bActive) return true;
    }

    return false;
}


void T8RangeTable::setControls()
{
    setEnabled(true);

    for(uint row=0; row<s_T8Range.size(); row++)
        setRowEnabled(row, s_T8Range.at(row).m_bActive);
}


void T8RangeTable::onActivate()
{
    int row = currentIndex().row();


    if(row<0 || row>=int(s_T8Range.size())) return;
    s_T8Range[row].m_bActive = !s_T8Range.at(row).m_bActive;
    emit pressed(QModelIndex());


    fillRangeTable();
}


void T8RangeTable::onMoveUp()
{
    int row = currentIndex().row();

    if(row<=0 || row>=int(s_T8Range.size())) return;
    T8Opp range = s_T8Range.at(row);
    s_T8Range.erase(s_T8Range.begin()+row);
    s_T8Range.insert(s_T8Range.begin()+row-1, range);
    emit pressed(QModelIndex());

     fillRangeTable();
}


void T8RangeTable::onMoveDown()
{
    int row = currentIndex().row();

    if(row<0 || row>=int(s_T8Range.size())-1) return;
    T8Opp range = s_T8Range.at(row);
    s_T8Range.erase(s_T8Range.begin()+row);
    s_T8Range.insert(s_T8Range.begin()+row+1, range);
    emit pressed(QModelIndex());

    fillRangeTable();
}


void T8RangeTable::onDeleteRow()
{
    int row = currentIndex().row();


    if(row<0 || row>=int(s_T8Range.size())) return;
    if(s_T8Range.size()<=1) return; // leave at least one row
    s_T8Range.erase(s_T8Range.begin()+row);
    emit pressed(QModelIndex());


    fillRangeTable();
}


void T8RangeTable::onInsertBefore()
{
    int row = currentIndex().row();

    if(row<0)
        s_T8Range.push_back(T8Opp(true, 0.0, 0.0, 1.0));
    else
        s_T8Range.insert(s_T8Range.begin()+row, s_T8Range.at(row));
    emit pressed(QModelIndex());


    fillRangeTable();
}


void T8RangeTable::onDuplicateRow()
{
    int row = currentIndex().row();

    if(row<0) return;
    else
    {
        s_T8Range.insert(s_T8Range.begin()+row+1, s_T8Range.at(row));
    }
    emit pressed(QModelIndex());

    fillRangeTable();
}


void T8RangeTable::onInsertAfter()
{
    int row = currentIndex().row();

    if(row<0)
        s_T8Range.push_back(T8Opp(true, 0, 0, 1));
    else
        s_T8Range.insert(s_T8Range.begin()+row+1, s_T8Range.at(row));
    emit pressed(QModelIndex());

    fillRangeTable();
}


void T8RangeTable::loadSettings(QSettings &settings)
{
    settings.beginGroup("T8RangeTable");
    {
        int nRange = settings.value("NT8Range", int(s_T8Range.size())).toInt();
        if(nRange>0)
        {
            s_T8Range.resize(nRange);
            for(int i=0; i<nRange; i++)
            {
                s_T8Range[i].m_bActive = settings.value(QString::asprintf("RangeT8_%d_bActive", i), s_T8Range.at(i).m_bActive).toBool();
                s_T8Range[i].m_Alpha   = settings.value(QString::asprintf("RangeT8_%d_alpha",   i), s_T8Range.at(i).m_Alpha).toDouble();
                s_T8Range[i].m_Beta    = settings.value(QString::asprintf("RangeT8_%d_beta",    i), s_T8Range.at(i).m_Beta).toDouble();
                s_T8Range[i].m_Vinf    = settings.value(QString::asprintf("RangeT8_%d_vinf",    i), s_T8Range.at(i).m_Vinf).toDouble();
            }
        }
        else
        {
            // force one line
            s_T8Range.resize(1);
            s_T8Range.front() = {true, 0.0, 0.0, 1.0};
        }
    }

    settings.endGroup();
}


void T8RangeTable::saveSettings(QSettings &settings)
{
    settings.beginGroup("T8RangeTable");
    {
        settings.setValue("NT8Range", int(s_T8Range.size()));
        for(uint i=0; i<s_T8Range.size(); i++)
        {
            settings.setValue(QString::asprintf("RangeT8_%d_bActive", i), s_T8Range.at(i).m_bActive);
            settings.setValue(QString::asprintf("RangeT8_%d_alpha",   i), s_T8Range.at(i).m_Alpha);
            settings.setValue(QString::asprintf("RangeT8_%d_beta",    i), s_T8Range.at(i).m_Beta);
            settings.setValue(QString::asprintf("RangeT8_%d_vinf",    i), s_T8Range.at(i).m_Vinf);
        }
    }
    settings.endGroup();
}



