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

#define _MATH_DEFINES_DEFINED



#include <QComboBox>

#include "planepartdelegate.h"

#include <core/xflcore.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/planexfl.h>


PlanePartDelegate::PlanePartDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{
    m_bEditable.resize(8);
    m_Precision.resize(8);
    m_bEditable.fill(true);
    m_Precision.fill(3);
}


QWidget *PlanePartDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
    if(!m_pPlane) return nullptr;
    if(index.column()==0)
    {
        if(m_pPlane->hasFuse() && index.row()>=m_pPlane->nWings())
        {
            // no editor for the body
            return nullptr;
        }
        else
        {
            QComboBox *pComboBoxEditor = new QComboBox(parent);
            pComboBoxEditor->addItem("MAINWING");
            pComboBoxEditor->addItem("SECONDWING");
            pComboBoxEditor->addItem("ELEVATOR");
            pComboBoxEditor->addItem("FIN");
            pComboBoxEditor->addItem("OTHERWING");
            //change behaviour so that the combobox closes on selection
            connect(pComboBoxEditor, SIGNAL(activated(int)), pComboBoxEditor, SLOT(close()));
            return pComboBoxEditor;
        }
    }
    else if(index.column()==1)
    {
        QLineEdit *pLineEditor = new QLineEdit(parent);
        pLineEditor->setAlignment(Qt::AlignLeft);
        return pLineEditor;
    }
    else if(index.column()>1 && index.column()<7)
    {
        FloatEdit *pDoubleEditor = new FloatEdit(parent);
        pDoubleEditor->setAlignment(Qt::AlignRight);
        return pDoubleEditor;
    }
    return nullptr;
}


void PlanePartDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        QString strong = index.model()->data(index, Qt::DisplayRole).toString();
        QComboBox *pCbBox = static_cast<QComboBox*>(pEditor);
        int pos = pCbBox->findText(strong);
        if (pos>=0) pCbBox->setCurrentIndex(pos);
        else        pCbBox->setCurrentIndex(0);
    }
    else if(index.column()==1)
    {
        QString name = index.model()->data(index, Qt::DisplayRole).toString();
        QLineEdit *pLE = static_cast<QLineEdit*>(pEditor);
        pLE->setText(name);
    }
    else if(index.column()>1 && index.column()<7)
    {
        double value = index.model()->data(index, Qt::DisplayRole).toDouble();
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->setValue(value);
    }
    else QStyledItemDelegate::setEditorData(pEditor, index);
}


void PlanePartDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}


/** Custom display of model data */
void PlanePartDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strange;

    if(index.column()<=1)
    {
        strange = index.model()->data(index, Qt::DisplayRole).toString();    
        painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, strange);
    }
    else if(index.column()>=2 && index.column()<=6)
    {
        double dble = index.model()->data(index, Qt::DisplayRole).toDouble();

        if(xfl::isLocalized())
            strange = QString("%L1").arg(dble ,0,'f',  m_Precision.at(index.column()));
        else
            strange = QString("%1").arg(dble, 0, 'f', m_Precision.at(index.column()));

        strange = QString::asprintf("%g", dble);
        painter->drawText(option.rect, Qt::AlignRight | Qt::AlignVCenter,strange);
    }
    else
    {
        //action column
        QVariant const & var = index.model()->data(index, Qt::DisplayRole);
        painter->drawText(option.rect, Qt::AlignCenter, var.toString());
    }
}


void PlanePartDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        QString strong;
        QComboBox *pCbBox = static_cast<QComboBox*>(editor);
        int sel = pCbBox->currentIndex();
        if (sel >=0) strong = pCbBox->itemText(sel);
        model->setData(index, strong, Qt::EditRole);
    }
    else if(index.column()==1)
    {
        QLineEdit *pLE = static_cast<FloatEdit*>(editor);
        model->setData(index, pLE->text(), Qt::EditRole);
    }
    else if(index.column()>0 && index.column()<7)
    {
        FloatEdit *pDE = static_cast<FloatEdit*>(editor);
        pDE->readValue();
        model->setData(index, pDE->value(), Qt::EditRole);
    }
}







