/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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





