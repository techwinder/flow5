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

#include "ctrltabledelegate.h"

#include <core/xflcore.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/widgets/customwts/floatedit.h>


CtrlTableDelegate::CtrlTableDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}


QWidget *CtrlTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    if(index.column()==0)
    {
        QLineEdit *pEditor = new QLineEdit(parent);
        pEditor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        return pEditor;
    }
    else if(index.column()==1 || index.column()==2)
    {
        if(index.column()<m_bEditable.size() && m_bEditable.at(index.column()))
        {
            FloatEdit *pEditor = new FloatEdit(parent);
            pEditor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            if(index.column()<m_Precision.size())
                pEditor->setDigits(m_Precision[index.column()]);
            return pEditor;
        }
        else return nullptr;
    }

    return nullptr;
}


void CtrlTableDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *pLineEdit = dynamic_cast<QLineEdit*>(pEditor);
        pLineEdit->setText(strong);
    }
    else if(index.column()==1 || index.column()==2)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->setValue(value);
    }
    else if(index.column()==3)
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = dynamic_cast<QLineEdit*>(pEditor);
        lineEdit->setText(strong);
    }
}


void CtrlTableDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *pAbstractItemModel, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        QString strong;
        QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
        strong = pLineEdit->text();

        pAbstractItemModel->setData(index, strong, Qt::EditRole);
    }
    else if(index.column()==1 || index.column()==2)
    {
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->readValue();
        pAbstractItemModel->setData(index, pDE->value(), Qt::EditRole);
    }
    else if(index.column()==3)
    {
/*        QString strong;
        QLineEdit *pLineEdit = static_cast<QLineEdit*>(editor);
        strong = pLineEdit->text();
        model->setData(index, strong, Qt::EditRole);*/
    }
}


void CtrlTableDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}


bool CtrlTableDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;

    // make sure that we have the right event type
    if (event->type() == QEvent::MouseButtonRelease)
    {
        const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                              option.rect.size(),
                                              QRect(option.rect.x() + textMargin, option.rect.y(),
                                                    option.rect.width() - (2 * textMargin), option.rect.height()));

        if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos())) return false;
    }
    else if (event->type() == QEvent::KeyPress)
    {
        if (   static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space
            && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
            return false;
    }
    else
    {
        return false;
    }

    Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                        ? Qt::Unchecked : Qt::Checked);
    return model->setData(index, state, Qt::CheckStateRole);
}


void CtrlTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strange = index.model()->data(index, Qt::DisplayRole).toString();

    Qt::Alignment align(option.displayAlignment);

    painter->save();

    int col = index.column();

    if(col==0)
    {
        align = Qt::AlignLeft | Qt::AlignVCenter;
    }
    else if(col==1 || col==2)
    {
        align = Qt::AlignRight | Qt::AlignVCenter;

        if(!strange.isEmpty())
        {
            bool bOK(false);
            double dble = index.model()->data(index, Qt::DisplayRole).toDouble(&bOK);
            if(bOK && index.column()<m_Precision.size())
            {
                if(xfl::isLocalized())
                    strange = QString("%L1").arg(dble ,0,'f',  m_Precision.at(index.column()));
                else
                    strange = QString("%1").arg(dble ,0,'f',  m_Precision.at(index.column()));
            }
            else
                strange = QString::asprintf("%g", dble);
        }
    }
    else if(col==3)
    {
        align = Qt::AlignLeft | Qt::AlignVCenter;
    }

    painter->drawText(option.rect, align, strange);
    painter->restore();
}



void CtrlTableDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
//    QStyledItemDelegate::initStyleOption(option, index);
    int col = index.column();

    if(col==0)
    {
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    }
    else if(col==1 || col==2)
    {
        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    }
    else if(col==3)
    {
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    }

    // note: colours are overriden by app stylesheet
    if(!m_bEditable.at(col))
    {
            option->font.setItalic(true);
            option->palette.setColor(QPalette::Text, Qt::gray);
    }
}


