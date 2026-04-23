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


#include <QListWidget>

#include "wingsectiondelegate.h"
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/objects2d.h>
#include <api/foil.h>
#include <api/wingsection.h>

WingSectionDelegate::WingSectionDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_pWingSection = nullptr;
}


QWidget *WingSectionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
    if(index.column()==5 || index.column()==7 || index.column()==9)
    {
        QString strong;
        QListWidget *plwEditor = new QListWidget(parent);
        if(index.column()==5)
        {
            for(int i=0; i<Objects2d::nFoils(); i++)
            {
                Foil *pFoil = Objects2d::foil(i);
                strong = QString::fromStdString(pFoil->name());
                plwEditor->addItem(strong);
            }
        }
        else if(index.column()==7)
        {
            plwEditor->addItem("UNIFORM");
            plwEditor->addItem("COSINE");
            plwEditor->addItem("TANH");
            plwEditor->addItem("SIGMOID");
        }
        else if(index.column()==9)
        {
            plwEditor->addItem("UNIFORM");
            plwEditor->addItem("COSINE");
            plwEditor->addItem("SINE");
            plwEditor->addItem("INV_SINE");
            plwEditor->addItem("TANH");
            plwEditor->addItem("EXP");
            plwEditor->addItem("INV_EXP");
        }

//        pComboBoxEditor->showPopup();
        //change behaviour so that the combobox closes on selection
        connect(plwEditor, SIGNAL(itemClicked(QListWidgetItem*)), plwEditor, SLOT(close()));

        return plwEditor;
    }
    else if(index.column()==10)
    {
        return nullptr;
    }
    else
    {
        FloatEdit *pDoubleEditor = new FloatEdit(parent);
//        pDoubleEditor->setAlignment(Qt::AlignRight);
//        pDoubleEditor->setDigits(m_Precision[index.column()]);

        if(index.column()==6)
        {
            pDoubleEditor->setMin(1);
        }
        if(index.column()==8)
        {
            pDoubleEditor->setMin(1);
        }
        return pDoubleEditor;
    }
}


void WingSectionDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    if(index.column()==5 || index.column()==7 || index.column()==9)
    {
        QString strong = index.model()->data(index, Qt::DisplayRole).toString();
        QListWidget *plwEditor = static_cast<QListWidget*>(pEditor);
        QList<QListWidgetItem*> items = plwEditor->findItems(strong, Qt::MatchExactly);
        if (items.size()) plwEditor->setCurrentItem(items.front());

        QAbstractItemModel *pItemModel = plwEditor->model();
        QItemSelectionModel *pSelModel = plwEditor->selectionModel();
        if(pSelModel && pItemModel)
        {
            for(int row=0; row<pItemModel->rowCount(); row++)
            {
                QModelIndex index = pItemModel->index(row, 0);
                QString name = pItemModel->data(index).toString();
                if(strong==name)
                {
                    pSelModel->select(index, QItemSelectionModel::Select);
                    break;
                }
            }
        }
    }
    else if(index.column()==10)
    {
    }
    else
    {
        double value = index.model()->data(index, Qt::DisplayRole).toDouble();
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->setValue(value);
    }
}


void WingSectionDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, QModelIndex const &index) const
{
    if(index.column()==5 || index.column()==7 || index.column()==9)
    {
/*        pEditor->adjustSize();
        int h = pEditor->height();*/
        pEditor->setMaximumWidth(option.rect.width());
        pEditor->move(option.rect.topLeft());
    }
    else if(index.column()==10)
    {
    }
    else
    {
        pEditor->setGeometry(option.rect);
    }
}


void WingSectionDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex &index) const
{
    if(index.column()==5 || index.column()==7 || index.column()==9)
    {
        QString strong;
        QListWidget *plw = static_cast<QListWidget*>(pEditor);
        QListWidgetItem*pItem = plw->currentItem();
        if (pItem) strong = pItem->text();
        pModel->setData(index, strong, Qt::EditRole);
    }
    else if(index.column()==10)
    {
    }
    else
    {
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->readValue();
        double value = pDE->value();
        pModel->setData(index, value, Qt::EditRole);
    }
}


void WingSectionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int row = index.row();
    int col = index.column();

    if(row==int(m_pWingSection->size())-1 && (col==3 || (col>=6 && col<10)))
    {
        // don't paint anything because there is no next section
    }
    else if(col<=4)
    {
        double dble = index.model()->data(index, Qt::DisplayRole).toDouble();
        QString strange;
        if(xfl::isLocalized())
            strange = QString("%L1").arg(dble ,0,'f',  m_Precision.at(index.column()));
        else
            strange = QString::asprintf("%g", dble);
        painter->drawText(option.rect, Qt::AlignRight | Qt::AlignVCenter, strange);
    }
    else if(col==10) // action column
    {
        QVariant const & var = index.model()->data(index, Qt::DisplayRole);
        painter->drawText(option.rect, Qt::AlignCenter, var.toString());
    }
    else QStyledItemDelegate::paint(painter, option, index);
}


