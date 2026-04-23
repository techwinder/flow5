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

#include <QStandardItemModel>
#include <interfaces/widgets/customwts/cptableview.h>

class XDirect;
class FoilTableDelegate;
class Foil;

class FoilTable : public CPTableView
{
    Q_OBJECT

    public:
        FoilTable(QWidget *pParent);
        FoilTable();

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

