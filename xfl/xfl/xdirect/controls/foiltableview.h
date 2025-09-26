/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QStandardItemModel>
#include <xflwidgets/customwts/cptableview.h>

class XDirect;
class FoilTableDelegate;
class Foil;

class FoilTableView : public CPTableView
{
    Q_OBJECT

    public:
        FoilTableView(QWidget *pParent);
        FoilTableView();

        void updateTable();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        void setTableFontStruct(FontStruct const &fntstruct);

        QSize sizeHint() const override {return QSize(0, s_Height);}

        static void setXDirect(XDirect*pXDirect) {s_pXDirect=pXDirect;}
        static void setDefaultHeight(int h) {s_Height=h;}

    private:
        void makeFoilTable();
        void connectSignals();
        void fillFoilTable();
        void resizeColumns();
        Foil* setObjectFromIndex(QModelIndex index);

    private slots:
        void onCurrentRowChanged(QModelIndex currentIndex, QModelIndex);
        void onItemClicked(const QModelIndex &index);

    public slots:
        void selectFoil(Foil *pFoil);

    private:
        QStandardItemModel *m_pFoilModel;
        FoilTableDelegate *m_pFoilDelegate;

        static int s_Height;
        static XDirect *s_pXDirect;
};

