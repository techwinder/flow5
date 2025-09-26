/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include "plaintextoutput.h"

#include <xflcore/displayoptions.h>


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


void PlainTextOutput::showEvent(QShowEvent *pEvent)
{
    QPlainTextEdit::showEvent(pEvent);
//    updateColors(true);
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


void PlainTextOutput::onAppendThisPlainText(QString const &sometext)
{
    moveCursor(QTextCursor::End);
    insertPlainText(sometext);
    moveCursor(QTextCursor::End);
    ensureCursorVisible();
}


