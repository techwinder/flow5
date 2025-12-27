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

#pragma once

#include <QTableView>

#include <core/fontstruct.h>

/**
 * @class A QTableView with copy-paste capability
 */
class CPTableView : public QTableView
{
    Q_OBJECT
    public:
        CPTableView(QWidget *pParent=nullptr);

        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void contextMenuEvent(QContextMenuEvent *pEvent) override;
        virtual void mouseDoubleClickEvent(QMouseEvent *pEvent) override;
        virtual QSize sizeHint() const override;
        virtual QSize minimumSizeHint() const override;
        virtual void resizeEvent(QResizeEvent *pEvent) override;

        void copySelection() const;
        void pasteClipboard();

        void setEditable(bool bEditable);
        bool isEditable() const {return m_bIsEditable;}

        void setCharSize(int nHChar, int nVChar) {m_nHorizontalChars=nHChar;  m_nVerticalChars=nVChar;}

        int fontHeight();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    signals:
        void tableResized();
        void dataPasted();
        void doubleClick(QModelIndex);

    protected slots:
        void onCopySelection() const;
        void onPaste();

    protected:
        bool m_bIsEditable;
        int m_nVerticalChars, m_nHorizontalChars;

};

