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


#include <QWidget>
#include <QGroupBox>
#include <QTreeView>
#include <QCheckBox>
#include <QSplitter>
#include <QSettings>

#include <core/enums_core.h>
#include <core/fontstruct.h>

class Boat;
class BoatPolar;
class BoatOpp;
class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;
class ObjectTreeItem;
class PlainTextOutput;
class XSail;

class BoatExplorer : public QWidget
{
	Q_OBJECT

    public:
        /** @enum The different objects selectable in the BoatExplorer */
        enum enumSelectionType {NOBOAT, BOAT, BTPOLAR, BOATOPP};

    public:
        BoatExplorer(QWidget *pParent = nullptr);
        ~BoatExplorer();

        void insertBoat(Boat* pBoat);
        void insertBtPolar(BoatPolar *pBtPolar);

        QString removeBoat(const QString &BoatName);
        QString removeBoat(Boat *pBoat);
        QString removeBtPolar(const BoatPolar *pBtPolar);
        void removeBoatOpp(BoatOpp *pBtOpp);
        void removeBtPolarBtOpps(BoatPolar *pBtPolar);
        void removeBtOpps(Boat* pBoat);

        void selectBoat(Boat* pBoat);
        void selectBtPolar(BoatPolar *pBtPolar);
        void selectBtOpp(BoatOpp*pPOpp=nullptr);

        void selectCurrentObject();
        void setObjectFromIndex(QModelIndex index);

        void addBtOpps(BoatPolar *pBPolar=nullptr);
        void fillModelView();
        void fillBtPolars(ObjectTreeItem *pBoatItem, const Boat *pBoat);
        void selectObjects();
        void setCurveParams();
        void updateLineStyles();
        void updateVisibilityBoxes();

        QByteArray const &splitterSize() {return s_SplitterSizes;}
        void setSplitterSize(QByteArray size) {s_SplitterSizes = size;}

        void setObjectProperties();
        void updateObjectView();

        void setTreeFontStruct(FontStruct const &fntstruct);
        void setPropertiesFont(QFont const &fnt);

        Qt::CheckState boatState(const Boat *pBoat) const;
        Qt::CheckState btPolarState(const BoatPolar *pBPolar) const;


        static void setDefaultWidth(int width) {s_Width=width;}
        static void setXSail(XSail*pXSail) {s_pXSail = pXSail;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    protected:
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(s_Width, 0);}

    public slots:
        void onItemClicked(const QModelIndex &filteredindex);
        void onItemDoubleClicked(const QModelIndex &filteredindex);
        void onCurrentRowChanged(QModelIndex curidx, QModelIndex);
        void onSwitchAll(bool bChecked);
        void onSetFilter();

    private:
        void setupLayout();
        void setOverallCheckStatus();

    private:
        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QSplitter *m_pMainSplitter;

        enumSelectionType m_Selection;

        PlainTextOutput *m_ppto;

        static int s_Width;

        static XSail *s_pXSail;
        static QByteArray s_SplitterSizes;
};

