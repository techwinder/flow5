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


#include <QPainter>

#include "partinertiadelegate.h"
#include <interfaces/widgets/customwts/floatedit.h>
#include <core/xflcore.h>

PartInertiaDelegate::PartInertiaDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_Precision.clear();
}


QWidget *PartInertiaDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
    if(index.column()==1)
    {
        //we have a number
        FloatEdit *pEditor = new FloatEdit(parent);
        pEditor->setDigits(m_Precision[index.column()]);
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        pEditor->setValue(value);

        return pEditor;
    }
    else
    {
        //we have a string
        QLineEdit *pEditor = new QLineEdit(parent);
        pEditor->setAlignment(Qt::AlignLeft);
        return pEditor;
    }
}


void PartInertiaDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(m_Precision[index.column()]>=0)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        FloatEdit *pDE = static_cast<FloatEdit*>(editor);
        pDE->setValueNoFormat(value);
    }
    else
    {
        QLineEdit *pLine = static_cast<QLineEdit*>(editor);
        pLine->setText(index.model()->data(index, Qt::EditRole).toString());
    }
}


void PartInertiaDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    if(index.column()==1)
    {
        FloatEdit *pDE = static_cast<FloatEdit*>(editor);
        pDE->readValue();
        double value = pDE->value();
        model->setData(index, value, Qt::EditRole);
    }
    else
    {
        QLineEdit *pLine = static_cast<QLineEdit*>(editor);
        model->setData(index, pLine->text(), Qt::EditRole);
    }
}


void PartInertiaDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


void PartInertiaDelegate::setPrecision(QVector<int>PrecisionTable)
{
    m_Precision = PrecisionTable;
}


void PartInertiaDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strange;

    Qt::Alignment align;

    if(index.column()==0)
    {
        strange = index.model()->data(index, Qt::DisplayRole).toString();
        align = Qt::AlignLeft | Qt::AlignVCenter;
    }
    else if(index.column()==2)
    {
        strange = index.model()->data(index, Qt::DisplayRole).toString();
        align = Qt::AlignCenter;
    }
    else
    {
        bool bOK(false);
        double dble = index.model()->data(index, Qt::DisplayRole).toDouble(&bOK);
        if(bOK)
        {
            if(xfl::isLocalized())
                strange = QString("%L1").arg(dble ,0,'f', m_Precision.at(index.column()));
            else
                strange = QString::asprintf("%g", dble);
            align = Qt::AlignRight | Qt::AlignVCenter;
        }
/*        else
        {
            strange = index.model()->data(index, Qt::DisplayRole).toString();
            align = Qt::AlignLeft | Qt::AlignVCenter;
        }*/
    }
    painter->drawText(option.rect, align, strange);
}




