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
#include <QClipboard>
#include <QMimeData>
#include <QKeyEvent>
#include <QHeaderView>
#include <QMenu>

#include "cptableview.h"
#include <interfaces/widgets/customdlg/separatorsdlg.h>
#include <core/displayoptions.h>


CPTableView::CPTableView(QWidget *pParent) : QTableView(pParent)
{
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setEditable(false);
    setFont(DisplayOptions::tableFontStruct().font());
    horizontalHeader()->setFont(DisplayOptions::tableFontStruct().font());
    verticalHeader()->setFont(DisplayOptions::tableFontStruct().font());

    m_nHorizontalChars = 30;
    m_nVerticalChars = 10;
}


int CPTableView::fontHeight()
{
    return DisplayOptions::tableFontStruct().height();
}


void CPTableView::setEditable(bool bEditable)
{
    m_bIsEditable=bEditable;
    if(m_bIsEditable)
    {
        setEditTriggers(QAbstractItemView::DoubleClicked |
                        QAbstractItemView::SelectedClicked |
                        QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::AnyKeyPressed);
    }
    else setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void CPTableView::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    bool bCtrl  = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch(pEvent->key())
    {
        case Qt::Key_C:
        {
            if(bCtrl)
            {
                copySelection();
                pEvent->accept();
            }
            return;
        }
        case Qt::Key_V:
        {
            if(bCtrl)
            {
                pasteClipboard();
                pEvent->accept();
            }
            return;
        }
        case Qt::Key_Delete:
        {
            if(!m_bIsEditable) return;
            model()->blockSignals(true);
            QModelIndexList selected = selectionModel()->selectedIndexes();
            for(QModelIndex const&ind : selected)
            {
                if(ind.isValid())
                    model()->setData(ind, QVariant());
            }
            model()->blockSignals(false);
            emit model()->dataChanged(selectionModel()->selectedIndexes().front(), selectionModel()->selectedIndexes().back());
            break;
        }
        case Qt::Key_Copy:
        {
            copySelection();
            pEvent->accept();
            return;
        }
        case Qt::Key_Paste:
        {
            pasteClipboard();
            pEvent->accept();
            return;
        }
        default:
            break;
    }
    QTableView::keyPressEvent(pEvent);
}


void CPTableView::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    QModelIndex refindex = currentIndex();
    if(refindex.isValid())
    {
        emit doubleClick(refindex);
    }

    QTableView::mouseDoubleClickEvent(pEvent);
}


void CPTableView::copySelection() const
{
    QModelIndexList cells = selectionModel()->selectedIndexes();
    //    qSort(cells);
    std::sort(cells.begin(), cells.end());

    if(cells.count()<1) return;
    QString copySel;

    QModelIndex previous = cells.front();
    copySel.append(model()->data(previous).toString());

    for(int i=1; i<cells.count(); i++)
    {
        QModelIndex index = cells.at(i);
        if(!isColumnHidden(index.column()))
        {
            if(index.row()==previous.row()) copySel.append('\t');
            else                            copySel.append('\n');
            copySel.append(model()->data(index).toString());
        }
        previous = index;
    }

    copySel.append('\n');

    QClipboard *pClipBoard = QApplication::clipboard();
    pClipBoard->setText(copySel);
}


/**
 * Note: no option to change the row and column count of the associated QAbstractItemModel
 * It it up to the parent widget to resize the model prior to the call to this function
 */
void CPTableView::pasteClipboard()
{
    if(!m_bIsEditable) return;

    SeparatorsDlg dlg;
    if(dlg.exec()!=QDialog::Accepted) return;

    QString regexp = "[";
    if(dlg.bWhiteSpace()) regexp += "\\s+";
    if(dlg.bTab())        regexp += "\\\\t+";
    if(dlg.bComma())      regexp += "\\\\,+";
    if(dlg.bSemiColon())  regexp += "\\\\;+";
    regexp += "]";

    QString eol = "\n";

    const QClipboard *pClip = QApplication::clipboard();
    if (!pClip->mimeData()->hasText()) return; // can only paste text;

    QModelIndex refindex = currentIndex();
    QStringList lines = pClip->text().split(eol);
    //if last line is empty, remove it;
    if(lines.back().isEmpty()) lines.removeLast();

    int row = refindex.row();
    int col0 = refindex.column();
    int rowcount = model()->rowCount();
    if(rowcount<row+lines.size())
        model()->insertRows(rowcount, row+lines.size()-rowcount);

    model()->blockSignals(true); // avoid sending one dataChanged() signal for each pasted cell
    int maxColumns = 0;
    for(int iLine=0; iLine<lines.size(); iLine++)
    {
        QString strange = lines[iLine].trimmed();
#if QT_VERSION >= 0x050F00
        QStringList items = strange.split(QRegularExpression(regexp), Qt::SkipEmptyParts);
#else
        QStringList items = strange.split(QRegExp(regexp), QString::SkipEmptyParts);
#endif
        maxColumns = qMax(maxColumns, items.size());
        int col = col0;
        for(int p=0; p<items.size(); p++)
        {
            while(horizontalHeader()->isSectionHidden(col+p))
            {
                col++; // don't paste in hidden columns
            }

            QModelIndex ind = model()->index(row+iLine, col+p, QModelIndex());
            if(ind.isValid())
            {
                model()->setData(ind, items.at(p));
            }
        }
    }

    QModelIndex bottomRightIndex = model()->index(row+lines.size()-1, col0+maxColumns-1);
    selectionModel()->clearSelection();
    QItemSelection selection(refindex, bottomRightIndex);
    selectionModel()->select(selection, QItemSelectionModel::Select);
    model()->blockSignals(false);

    emit dataPasted();
}


QSize CPTableView::sizeHint() const
{
    int w = DisplayOptions::tableFontStruct().m_PtSize;
    int h =  DisplayOptions::tableFontStruct().m_PtSize;
    if(!DisplayOptions::tableFontStruct().m_Family.isEmpty())
    {
        w = DisplayOptions::tableFontStruct().averageCharWidth();
        h = DisplayOptions::tableFontStruct().height();
    }

    return QSize(m_nHorizontalChars*w, m_nVerticalChars  *h);
}


QSize CPTableView::minimumSizeHint() const
{
    return QSize(100,100);
}


void CPTableView::resizeEvent(QResizeEvent *pEvent)
{
    emit tableResized();//signal to perform column resize
    QTableView::resizeEvent(pEvent);
}


void CPTableView::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QAction *pCopyAction = new QAction("Copy", this);
    pCopyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));

    QAction *pPasteAction = new QAction("Paste", this);
    pPasteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    pPasteAction->setEnabled(m_bIsEditable);

    connect(pCopyAction,  SIGNAL(triggered(bool)), this, SLOT(onCopySelection()));
    connect(pPasteAction, SIGNAL(triggered(bool)), this, SLOT(onPaste()));

    QMenu *pCPTableMenu = new QMenu("context menu", this);
    {
        pCPTableMenu->addAction(pCopyAction);
        pCPTableMenu->addAction(pPasteAction);
    }
    pCPTableMenu->exec(pEvent->globalPos());
}


void CPTableView::onCopySelection() const
{
    copySelection();
}


void CPTableView::onPaste()
{
    pasteClipboard();
}


void CPTableView::loadSettings(QSettings &settings)
{
    settings.beginGroup("CPTableView");
    {
    }
    settings.endGroup();
}


void CPTableView::saveSettings(QSettings &settings)
{
    settings.beginGroup("CPTableView");
    {
    }
    settings.endGroup();
}

