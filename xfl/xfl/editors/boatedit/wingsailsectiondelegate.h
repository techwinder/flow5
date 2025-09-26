/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QStyledItemDelegate>


class WingSail;

class WingSailSectionDelegate : public QStyledItemDelegate
{
    public:
        WingSailSectionDelegate(QObject *parent = nullptr);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *pModel, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        void setPrecision(QVector<int> PrecisionTable);
        void setWingSail(WingSail *pWingSail) {m_pWingSail=pWingSail;}

    private:

        QVector<int> m_Precision; ///table of float precisions for each column
        WingSail *m_pWingSail;
};
