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

#include "numedit.h"


NumEdit::NumEdit(QWidget *pWidget) : QLineEdit(pWidget)
{
    QPalette palette;
    QColor clr = palette.color(QPalette::Active, QPalette::Window);
    setStyleSheet("QLineEdit{background-color: " + clr.name() + ";}"); // overrides background issue in Windows
}


QSize NumEdit::sizeHint() const
{
    QFont fnt;
    QFontMetrics fm(fnt);
    int w = 9 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void NumEdit::showEvent(QShowEvent*pEvent)
{
    formatValue();
    QLineEdit::showEvent(pEvent);
}


void NumEdit::focusInEvent(QFocusEvent *pEvent)
{
    selectAll();
    QLineEdit::focusInEvent(pEvent);
}

void NumEdit::focusOutEvent(QFocusEvent *pEvent)
{
    readValue();
    formatValue();

    QLineEdit::focusOutEvent(pEvent);
}


/** Hides the base function */
void NumEdit::paste()
{
    QClipboard const *pClip = QApplication::clipboard();
    if (!pClip->mimeData()->hasText()) return; // can only paste text;

    QString eol = "\n";
    QStringList lines = pClip->text().split(eol);
    if(!lines.size()) return;

    clear();
    setText(lines.front());
}
