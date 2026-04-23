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

#include <QMenu>
#include <QStyle>

#include "plaintextoutput.h"

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/utils.h>

PlainTextOutput::PlainTextOutput(QWidget *pWidget) : QPlainTextEdit(pWidget)
{
    m_nHChar = 15;
    m_nVChar = 5;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);

    updateColors(false);
//    setMinimumHeight(2*DisplayOptions::tableFontStruct().height());
}


void PlainTextOutput::updateColors(bool bOpaque)
{
    QPalette palette;
//    palette.setColor(QPalette::WindowText, s_TableColor);
    if(bOpaque)
    {
        palette.setColor(QPalette::Window, QColor(225,125,125));
        palette.setColor(QPalette::Base, QColor(125,155,225));
    }
    else
    {
        palette.setColor(QPalette::Window, QColor(225,125,125,0));
        palette.setColor(QPalette::Base, QColor(125,155,225,25));
    }

    setBackgroundVisible(false);
    setPalette(palette);
    viewport()->setPalette(palette);
    viewport()->setAutoFillBackground(true);
}


void PlainTextOutput::keyPressEvent(QKeyEvent *pEvent)
{
    bool bAlt = false;
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::AltModifier)      bAlt =true;
    if(pEvent->modifiers() & Qt::ControlModifier)  bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_L:
        {
            if(bAlt)
            {
                clear();
            }
            break;
        }
        case Qt::Key_0:
        {
            if(bCtrl)
            {
                setFont(DisplayOptions::tableFontStruct().font());
                update();
                pEvent->accept();
            }
            break;
        }
        case Qt::Key_Minus:
        {
            if(bCtrl)
            {
                zoomOut();
            }
            break;
        }
        case Qt::Key_Plus:
        {
            if(bCtrl)
            {
                zoomIn();
            }
            break;
        }
    }
    QPlainTextEdit::keyPressEvent(pEvent);
}


void PlainTextOutput::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pContextMenu = new QMenu(tr("PlainTextOutput"));
    {
        QAction *pSelectAll = new QAction(tr("Select all"), this);
        connect(pSelectAll, SIGNAL(triggered(bool)), SLOT(selectAll()));

        QAction *pClear = new QAction(tr("Clear"), this);
        connect(pClear, SIGNAL(triggered(bool)), SLOT(clear()));

        QAction *pCopy = new QAction(tr("Copy selection"), this);
        connect(pCopy, SIGNAL(triggered(bool)), SLOT(copy()));

        QAction *pCopyAll = new QAction(tr("Copy all"), this);
        connect(pCopyAll, SIGNAL(triggered(bool)), SLOT(onCopyAll()));

        pContextMenu->addAction(pSelectAll);
        pContextMenu->addSeparator();
        pContextMenu->addAction(pCopy);
        pContextMenu->addAction(pCopyAll);
        pContextMenu->addSeparator();
        pContextMenu->addAction(pClear);
    }
    pContextMenu->exec(QCursor::pos());
    update();
    pEvent->accept();
}


void PlainTextOutput::showEvent(QShowEvent *pEvent)
{
    QPlainTextEdit::showEvent(pEvent);
    setFont(DisplayOptions::tableFontStruct().font());
}


void PlainTextOutput::setCharDimensions(int nHChar, int nVChar)
{
    m_nHChar = nHChar;
    m_nVChar = nVChar;
}


QSize PlainTextOutput::sizeHint() const
{
    return QSize(DisplayOptions::tableFontStruct().averageCharWidth()*m_nHChar, DisplayOptions::tableFontStruct().height()*m_nVChar);
}


void PlainTextOutput::onCopyAll()
{
    selectAll();
    copy();
}


void PlainTextOutput::onAppendQText(QString const &sometext)
{
    moveCursor(QTextCursor::End);
    insertPlainText(sometext);
    moveCursor(QTextCursor::End);
    ensureCursorVisible();
}


void PlainTextOutput::appendEOL(int n)
{
    moveCursor(QTextCursor::End);
    for(int k=0; k<n; k++)  insertPlainText(EOLch);
    moveCursor(QTextCursor::End);
    ensureCursorVisible();

}

