/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QStyledItemDelegate>

#include <xflwidgets/wt_globals.h>





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

