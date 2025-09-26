/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QTableView>

#include <xflcore/fontstruct.h>

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

