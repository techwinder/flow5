/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QPainter>
#include <QStyledItemDelegate>

class Foil;
class WingSection;
class FloatEdit;

class WingSectionDelegate : public QStyledItemDelegate
{
    public:
        WingSectionDelegate(QObject *parent = nullptr);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
        void setEditorData(QWidget *pEditor, const QModelIndex &index) const override;
        void setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        void setPrecision(QVector<int> PrecisionTable) {m_Precision = PrecisionTable;}
        void setWingSectionArray(QVector<WingSection> &wingsectionarray) {m_pWingSection=&wingsectionarray;}

    private:
        QVector<int> m_Precision; ///table of float precisions for each column
        QVector<WingSection> const *m_pWingSection;
};

