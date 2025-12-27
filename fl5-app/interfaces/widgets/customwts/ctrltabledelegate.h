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

#include <QList>
#include <QStyledItemDelegate>
#include <QStandardItemModel>


class FloatEdit;

class CtrlTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        CtrlTableDelegate(QObject *parent = nullptr);

        void setPrecision(QVector<int> const &precision) {m_Precision=precision;}
        void setEditable(QVector<bool> const &editable) {m_bEditable=editable;}

    private:
        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget *pEditor, const QModelIndex &index) const override;
        void setModelData(QWidget *pEditor, QAbstractItemModel *pAbstractItemModel, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
        void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    private:
        QVector<int> m_Precision; ///table of float precisions for each column
        QVector<bool> m_bEditable;

};





