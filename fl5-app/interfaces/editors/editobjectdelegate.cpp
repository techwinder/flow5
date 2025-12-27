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
#include <QComboBox>

#include "editobjectdelegate.h"

#include <core/enums_core.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <api/objects2d.h>
#include <api/foil.h>


EditObjectDelegate::EditObjectDelegate(QWidget *pParent) : QStyledItemDelegate(pParent)
{
}


QWidget *EditObjectDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();

    switch (dataType)
    {
        case xfl::BOOLVALUE:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("true");
            pEditor->addItem("false");

            //change behaviour so that the combobox closes on selection
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));

            return pEditor;
        }
        case xfl::POLARTYPE:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("FIXEDSPEEDPOLAR");
            pEditor->addItem("FIXEDLIFTPOLAR");
            pEditor->addItem("CONTROLPOLAR");
            pEditor->addItem("STABILITYPOLAR");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::ANALYSISMETHOD:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("LLT");
            pEditor->addItem("VLM1");
            pEditor->addItem("VLM2");
            pEditor->addItem("QUADS");
            pEditor->addItem("TRIUNIFORM");
            pEditor->addItem("TRILINEAR");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::PANELDISTRIBUTION:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("INV_EXP");
            pEditor->addItem("EXP");
            pEditor->addItem("TANH");
            pEditor->addItem("INV_SINH");
            pEditor->addItem("INV_SINE");
            pEditor->addItem("COSINE");
            pEditor->addItem("SINE");
            pEditor->addItem("UNIFORM");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::REFDIMENSIONS:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("PLANFORM");
            pEditor->addItem("PROJECTED");
            pEditor->addItem("CUSTOM");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::BODYTYPE:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("FLATPANELS");
            pEditor->addItem("NURBS");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::FUSEDRAG:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("KARMANSCHOENHERR");
            pEditor->addItem("PRANDTLSCHLICHTING");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::FOILNAME:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            for(int i=0; i<Objects2d::nFoils(); i++)
            {
                Foil *pFoil = Objects2d::foil(i);
                pEditor->addItem(QString::fromStdString(pFoil->name()));
            }
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::BOUNDARYCONDITION:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("DIRICHLET");
            pEditor->addItem("NEUMANN");
            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::WINGTYPE:
        {
            QComboBox *pEditor = new QComboBox(parent);
            pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            pEditor->addItem("MAINWING");
            pEditor->addItem("ELEVATOR");
            pEditor->addItem("FIN");
            pEditor->addItem("SECONDWING");
            pEditor->addItem("OTHERWING");

            connect(pEditor, SIGNAL(activated(int)), pEditor, SLOT(close()));
            return pEditor;
        }
        case xfl::DOUBLEVALUE:
        {
            FloatEdit *pEditor = new FloatEdit(parent);
            return pEditor;
        }
        case xfl::INTEGER:
        {
            IntEdit *pEditor = new IntEdit(parent);
            return pEditor;
        }
        default:
        {
            //String case, only edit the first line with the polar name
            if(index.row()==0 && index.column()==2)  return new QLineEdit(parent);
            else                                     return nullptr;
        }
    }
}


void EditObjectDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();
    if(dataType==xfl::INTEGER)
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        IntEdit *pIE = static_cast<IntEdit*>(pEditor);
        pIE->setValue(value);
    }
    else if(dataType==xfl::DOUBLEVALUE)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->setValue(value);
    }
    else if(dataType==xfl::STRING)
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *pLineEdit = dynamic_cast<QLineEdit*>(pEditor);
        pLineEdit->setText(strong);
    }
    //    else if(dataType==BOOL || dataType==PANELDISTRIBUTION || dataType==FOILNAME || dataType==BODYTYPE)
    else
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *pCbBox = static_cast<QComboBox*>(pEditor);
        int pos = pCbBox->findText(strong);
        if (pos>=0) pCbBox->setCurrentIndex(pos);
        else        pCbBox->setCurrentIndex(0);

    }
}


void EditObjectDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex &index) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();
    if(dataType==xfl::INTEGER)
    {
        IntEdit *pIE = static_cast<IntEdit*>(pEditor);
        pIE->readValue();
        pModel->setData(index, pIE->value(), Qt::EditRole);
    }
    else if(dataType==xfl::DOUBLEVALUE)
    {
        FloatEdit *pDE = static_cast<FloatEdit*>(pEditor);
        pDE->readValue();
        pModel->setData(index, pDE->value(), Qt::EditRole);
    }
    else if(dataType==xfl::STRING)
    {
        QLineEdit *pLineEdit = dynamic_cast<QLineEdit*>(pEditor);
        pModel->setData(index, pLineEdit->text(), Qt::EditRole);
    }
    else
    {
        QString strong;
        QComboBox *pCbBox = static_cast<QComboBox*>(pEditor);
        int sel = pCbBox->currentIndex();
        if (sel >=0) strong = pCbBox->itemText(sel);
        pModel->setData(index, strong, Qt::EditRole);
    }
}


void EditObjectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();
    QString strong;
    QStyleOptionViewItem myOption = option;
    if(dataType==xfl::INTEGER)
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        strong = QString("%L1").arg(index.model()->data(index, Qt::DisplayRole).toInt());
    }
    else if(dataType==xfl::DOUBLEVALUE)
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        double dble = index.model()->data(index, Qt::DisplayRole).toDouble();
        if(xfl::isLocalized())
            strong = QString("%L1").arg(dble ,0,'g');
        else
            strong = QString("%1").arg(dble ,0,'g');

    }
    else if(dataType==xfl::STRING)
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }
    else if( dataType==xfl::REFDIMENSIONS)
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }
    //    else if(dataType==BOOL || dataType==PANELDISTRIBUTION || dataType==FOILNAME ||
    //            dataType==BODYTYPE || dataType==POLARTYPE || dataType==ANALYSISMETHOD || dataType==REFDIMENSIONS)
    else
    {
        myOption.displayAlignment = Qt::AlignCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }

    painter->drawText(myOption.rect, myOption.displayAlignment, strong);
}


void EditObjectDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}












