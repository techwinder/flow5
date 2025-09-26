/****************************************************************************

	flow5 application

	Copyright (C) Andre Deperrois 

	All rights reserved.

*****************************************************************************/

#pragma once


#include <QPainter>
#include <QStyledItemDelegate>

class PlaneXfl;

class PlanePartDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        PlanePartDelegate(QWidget *parent);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        void setPrecision(QVector<int>PrecisionTable){m_Precision = PrecisionTable;}
        void setEditable(QVector<bool> const &editable) {m_bEditable=editable;}

        void setPlane(PlaneXfl const*pPlane) {m_pPlane=pPlane;}

    private:
        PlaneXfl const *m_pPlane;
        QVector<int> m_Precision;
        QVector<bool> m_bEditable;
};

