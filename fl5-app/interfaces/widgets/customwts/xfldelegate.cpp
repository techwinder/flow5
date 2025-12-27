/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QPainter>


#include "xfldelegate.h"
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>
#include <api/linestyle.h>
#include <interfaces/widgets/line/linemenu.h>

XflDelegate::XflDelegate(QObject *pParent) : QStyledItemDelegate(pParent)
{
    m_Digits.clear();
    m_iCheckColumn = -1;
}


void XflDelegate::setCheckColumn(int iCol)
{
    m_iCheckColumn=iCol;
    if(iCol>=0)
    {
        if(iCol<m_Digits.size()) m_Digits[iCol] = -1;
        if(iCol<m_ItemType.size()) m_ItemType[iCol] = XflDelegate::CHECKBOX;
    }
}


void XflDelegate::setActionColumn(int iCol)
{
    if(iCol>=0)
    {
        if(iCol<m_Digits.size()) m_Digits[iCol] = -1;
        if(iCol<m_ItemType.size()) m_ItemType[iCol] = XflDelegate::ACTION;
    }
}


void XflDelegate::setNCols(int n, enumItemType type)
{
    m_Digits.resize(n);
    m_Digits.fill(0);
    m_ItemType.resize(n);
    m_ItemType.fill(type);
}



QWidget *XflDelegate::createEditor(QWidget *pParent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
    int col = index.column();
    if(col<m_ItemType.size() && m_ItemType.at(col)==ACTION) return nullptr;

    enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case XflDelegate::STRING:
        {
            QLineEdit *pEditor = new QLineEdit(pParent);
            pEditor->setAlignment(Qt::AlignLeft);
            return pEditor;
        }
        case XflDelegate::INTEGER:
        {
            IntEdit *pEditor = new IntEdit(pParent);
            int value = index.model()->data(index, Qt::EditRole).toInt();
            pEditor->setValue(value);// redundant with setEditorData?
            return pEditor;
        }
        case XflDelegate::DOUBLE:
        {
            FloatEdit *pEditor = new FloatEdit(pParent);
//            pEditor->setDigits(m_Precision[index.column()]);
            double value = index.model()->data(index, Qt::EditRole).toDouble();
            pEditor->setValue(value);// redundant with setEditorData?
            return pEditor;
        }
        case XflDelegate::CHECKBOX:
        {
            break;
        }
        case XflDelegate::LINE:
        {
            break;
        }
        case XflDelegate::ACTION:
        {
            break;
        }
    }
    return nullptr;
}


void XflDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    int col = index.column();
    if(col<m_ItemType.size() && m_ItemType.at(col)==ACTION) return;

    enumItemType type = m_ItemType.at(col);
    switch (type)
    {
        case XflDelegate::STRING:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
            pLineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
            break;
        }
        case XflDelegate::INTEGER:
        {
            int value = index.model()->data(index, Qt::EditRole).toInt();
            IntEdit *pDE = static_cast<IntEdit*>(pEditor);
            pDE->setValueNoFormat(value);
            break;
        }
        case XflDelegate::DOUBLE:
        {
            double value = index.model()->data(index, Qt::EditRole).toDouble();
            FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
            pDE->setValueNoFormat(value);
            break;
        }
        case XflDelegate::CHECKBOX:
        {
            break;
        }
        case XflDelegate::LINE:
        {
            break;
        }
        case XflDelegate::ACTION:
        {
            break;
        }
    }
}


void XflDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex &index) const
{
    int col = index.column();

    if(col==m_iCheckColumn)
    {
        pModel->setData(index, QString(), Qt::EditRole); // to force the dataChanged signal and force repaint
        return;
    }

    if(col<m_ItemType.size() && m_ItemType.at(col)==ACTION) return;

    enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case XflDelegate::STRING:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
//            pModel->setData(index, pLineEdit->text(), Qt::DisplayRole); // not sure
            pModel->setData(index, pLineEdit->text(), Qt::EditRole);
            break;
        }
        case XflDelegate::INTEGER:
        {
            IntEdit *pIE = static_cast<IntEdit*>(pEditor);
            pIE->readValue();
            pModel->setData(index, pIE->value(), Qt::EditRole);
            break;
        }
        case XflDelegate::DOUBLE:
        {
            FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
            pDE->readValue();
            pModel->setData(index, pDE->value(), Qt::EditRole);
            break;
        }
        case XflDelegate::CHECKBOX:
        {
            break;
        }
        case XflDelegate::LINE:
        {
            break;
        }
        case XflDelegate::ACTION:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
            pModel->setData(index, pLineEdit->text(), Qt::EditRole);
            break;
        }
    }
}


void XflDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}


void XflDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    if(index.column()>=m_ItemType.size()) return;

    enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case XflDelegate::ACTION:
            option->displayAlignment = Qt::AlignCenter;
            break;
        case XflDelegate::STRING:
        {
            option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
            break;
        }
        case XflDelegate::INTEGER:
        {
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
        }
        case XflDelegate::DOUBLE:
        {
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
        }
        case XflDelegate::CHECKBOX:
        {
            break;
        }
        case XflDelegate::LINE:
        {
        }
    }
}


void XflDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int col = index.column();

    painter->save();
    enumItemType type(STRING);

    if(col<m_ItemType.size()) type = m_ItemType.at(col);

    QStyleOptionViewItem floatoption(option);

    bool bActive = true;

    if(m_iCheckColumn>=0)
    {
        // get the checked status in the action column
        QModelIndex sibling = index.sibling(index.row(), m_iCheckColumn);
        bActive = index.model()->data(sibling, Qt::UserRole).toBool();
    }

    QColor textcolor = option.palette.color(QPalette::Normal,   QPalette::Text);
    QColor backcolor = option.palette.color(QPalette::Active,   QPalette::Window);
    if(!bActive)
    {
        floatoption.palette.setColor(QPalette::Normal,   QPalette::Text, Qt::gray);
        floatoption.palette.setColor(QPalette::Active,   QPalette::Text, Qt::gray);
        floatoption.palette.setColor(QPalette::Inactive, QPalette::Text, Qt::gray);
        floatoption.palette.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);

        textcolor = Qt::gray;
        floatoption.font.setItalic(true);
    }

    painter->setFont(floatoption.font);

    QPen pen(textcolor);
    painter->setPen(pen);

    switch (type)
    {
        case XflDelegate::ACTION:
        {
            QString strong = index.model()->data(index, Qt::DisplayRole).toString();
            painter->drawText(option.rect, Qt::AlignCenter, strong);
            break;
        }
        case XflDelegate::STRING:
        {
            QString strong = index.model()->data(index, Qt::DisplayRole).toString();
            painter->drawText(option.rect, Qt::AlignLeading | Qt::AlignVCenter, strong);
            break;
        }
        case XflDelegate::DOUBLE:
        {
            QString strange = index.model()->data(index, Qt::DisplayRole).toString();

            if(!strange.isEmpty())
            {
                bool bOK(false);
                double dble = index.model()->data(index, Qt::DisplayRole).toDouble(&bOK);
                if(bOK)
                {
                    if(xfl::isLocalized() && col<m_Digits.size() && m_Digits.at(col)>=0)
                        strange = QString("%L1").arg(dble ,0,'f', m_Digits.at(col));
                    else
                        strange = QString::asprintf("%g", dble);
                }
            }
            painter->drawText(option.rect, Qt::AlignRight | Qt::AlignVCenter, strange);
            break;
        }
        case XflDelegate::INTEGER:
        {
            bool bOK(false);
            int n = index.model()->data(index, Qt::DisplayRole).toInt(&bOK);
            QString strange;
            if(bOK) strange = QString::asprintf("%d", n);

            painter->drawText(option.rect, Qt::AlignRight | Qt::AlignVCenter, strange);
            break;
        }
        case XflDelegate::CHECKBOX:
        {
            QFontMetrics fm(option.font);
            bool bChecked = index.model()->data(index, Qt::UserRole).toBool();
            wt::drawCheckBox(painter, bChecked, option.rect, fm.height(), false, false, textcolor, backcolor);
//            drawCheckBox(painter, bChecked, option.rect, option.rect.height(), false, false, textcolor, backcolor);
            break;
        }
        case XflDelegate::LINE:
        {
            if (index.data(Qt::DisplayRole).canConvert<LineStyle>())
            {
                QColor linecolor;
/** @todo gcc signals a potential out of bounds issue */
//                LineStyle ls = qvariant_cast<LineStyle>(index.data());
                LineStyle ls = index.data().value<LineStyle>();


                QRect r = option.rect;

                QStyledItemDelegate::paint(painter, option, index); // paint the background, using palette colors, including stylesheet mods

                if(ls.m_bIsEnabled)
                {
                    painter->save();
                    QPen LinePen;
                    if(ls.m_bIsEnabled)
                    {
                        linecolor = xfl::fromfl5Clr(ls.m_Color);
                        LinePen.setStyle(xfl::getStyle(ls.m_Stipple));
                        LinePen.setWidth(ls.m_Width);
                    }
                    else
                    {
                        linecolor = Qt::gray;
                        LinePen.setStyle(xfl::getStyle(Line::SOLID));
                        LinePen.setWidth(1);
                    }
                    LinePen.setColor(linecolor);
                    painter->setPen(LinePen);
                    painter->drawLine(r.left()+5, r.center().y(), r.right()-5, r.center().y());

                    LinePen.setStyle(Qt::SolidLine);
                    painter->setPen(LinePen);

                    QColor backcolor;
                    if (option.state & QStyle::State_Selected) backcolor = option.palette.highlight().color();
                    else                                       backcolor = option.palette.base().color();

                    if(ls.m_Symbol>0 && ls.m_bIsEnabled) xfl::drawSymbol(*painter, ls.m_Symbol, backcolor, linecolor, r.center());
                    painter->restore();
                }
            }
            else
            {
                QStyledItemDelegate::paint(painter, option, index);
            }
            break;
        }
    }
    painter->restore();
}



