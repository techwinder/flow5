/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <xflwidgets/customwts/intedit.h>
#include <xflcore/xflcore.h>

IntEdit::IntEdit(QWidget *pParent) : NumEdit(pParent)
{
    setParent(pParent);
    initialize(0);
}


IntEdit::IntEdit(int val, QWidget *pParent) : NumEdit(pParent)
{
    setParent(pParent);
    initialize(val);
}


void IntEdit::initialize(int value)
{
    m_Value = value;
    m_pIV = new QIntValidator(this);
    setValidator(m_pIV);
    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));
}


void IntEdit::readValue()
{
    bool bOK=false;
    QString str = text().trimmed();
    int val = 0;
    if(xfl::isLocalized())
        val = locale().toInt(str, &bOK);
    else
        val = str.toDouble(&bOK);

    if(bOK)
    {
        bool bValueChanged = abs(m_Value-val)>0;
        m_Value = val;
        if(bValueChanged)
        {
            emit intChanged(m_Value);
        }
    }
}


void IntEdit::setValue(int val)
{
    m_Value = val;
    formatValue();
}


void IntEdit::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            readValue();
            formatValue();
            break;
        }
        case Qt::Key_Escape:
        {
            formatValue();
            break;
        }
        default:
        {
            break;
        }
    }
    NumEdit::keyPressEvent(pEvent);
}


void IntEdit::formatValue()
{
    QString str;
    if(xfl::isLocalized())
        str = QString("%L1").arg(m_Value);
    else
        str = QString::asprintf("%d",m_Value);
    setText(str);
}


void IntEdit::setValueNoFormat(int val)
{
    m_Value = val;
}

