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

#pragma once

#include <QStyledItemDelegate>

#include <interfaces/widgets/globals/wt_globals.h>



class XflDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        enum enumItemType {STRING, INTEGER, DOUBLE, LINE, CHECKBOX, ACTION};


    public:
        XflDelegate(QObject *pParent = nullptr);
        QWidget *createEditor(QWidget *pParent, const QStyleOptionViewItem &, const QModelIndex &index) const override;

        void setEditorData(QWidget *pEditor, const QModelIndex &index) const override;
        void setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;


        void setName(QString const &name) {m_Name=name;}
        void setNCols(int n, enumItemType type);
        void setCheckColumn(int iCol);
        void setActionColumn(int iCol);
        void setDigits(QVector<int> const &DigitList) {m_Digits = DigitList;}
        void setItemTypes(QVector<enumItemType> itemtypes) {m_ItemType=itemtypes;}
        void setItemType(int col, enumItemType itemtype) {if(col>=0 && col<m_ItemType.size()) m_ItemType[col]=itemtype;}

    private:
        QVector<int> m_Digits; ///table of float precisions for each column

        int m_iCheckColumn;

        QVector<enumItemType> m_ItemType;

        QString m_Name; /// debug info

};

