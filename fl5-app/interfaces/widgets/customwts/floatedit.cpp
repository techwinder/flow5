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

#include <cmath>

#include <QFontMetrics>

#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <core/xflcore.h>

FloatEdit::FloatEdit(QWidget *pParent)  : NumEdit(pParent)
{
    setParent(pParent);
    initialize(0.0, -1);
}


FloatEdit::FloatEdit(float val, int decimals, QWidget *pParent) : NumEdit(pParent)
{
    setParent(pParent);
    initialize(val, decimals);
}


void FloatEdit::initialize(float value, int decimals)
{
    m_Value = value;
    m_Notation = QDoubleValidator::StandardNotation;
    setRange(-1.e10, 1.e10);
    m_Digits = decimals;

    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));
}


void FloatEdit::readValue()
{
    QString strange=text().trimmed();

    bool bOK=true;
    double val = strange.toDouble(&bOK);
    if(xfl::isLocalized())
        val = locale().toDouble(strange, &bOK);


    if(bOK)
    {
        bool bValueChanged = fabs(m_Value-val)>1.e-10;
        m_Value = val;

        if(bValueChanged) emit floatChanged(m_Value);
    }
}


void FloatEdit::setValue(float val)
{
    m_Value = val;
    formatValue();
}


void FloatEdit::setValuef(float val)
{
    m_Value = val;
    formatValue();
}


void FloatEdit::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            readValue();
            formatValue();

            QLineEdit::keyPressEvent(pEvent);
            break;
        }
        case Qt::Key_Escape:
        {
            formatValue();
            QLineEdit::keyPressEvent(pEvent);
            break;
        }
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_E:
        case Qt::Key_Comma:
        case Qt::Key_Period:
        case Qt::Key_Minus:
        case Qt::Key_Plus:
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_End:
        case Qt::Key_Home:
        case Qt::Key_Tab:
        {
            NumEdit::keyPressEvent(pEvent);
//            readValue();
            break;
        }
        default:
        {
            NumEdit::keyPressEvent(pEvent);
            pEvent->ignore();
            break;
        }
    }
}

/**
 * Checks that the value is within bounds and formats it
 */
void FloatEdit::formatValue()
{
    if(m_Value<m_MinValue) m_Value=m_MinValue;
    if(m_Value>m_MaxValue) m_Value=m_MaxValue;

    QString str = wt::formatDouble(m_Value, m_Digits, xfl::isLocalized());
    setText(str);
}


void FloatEdit::setValueNoFormat(float val)
{
    m_Value = val;
}


