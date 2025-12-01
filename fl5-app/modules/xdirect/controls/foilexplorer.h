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


#include <QTreeView>
#include <QToolBar>
#include <QSplitter>
#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>

#include <core/enums_core.h>
#include <api/linestyle.h>

class MainFrame;
class XDirect;

class Foil;
class Polar;
class OpPoint;
class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;
class ObjectTreeItem;
class PlainTextOutput;

class FoilExplorer : public QWidget
{
    Q_OBJECT

    public:
        /** @enum The different objects selectable in the FoilExplorer */
        enum enumSelectionType {NONE, FOIL, POLAR, OPPOINT};

    public:
        FoilExplorer(QWidget *pParent = nullptr);
        ~FoilExplorer() override;

        void contextMenuEvent(QContextMenuEvent *event) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        QSize sizeHint() const override {return QSize(s_Width, 0);}
        void resizeEvent(QResizeEvent *pEvent) override;

        void setupLayout();

        void setObjectFromIndex(const QModelIndex &filteredindex);

        void updateObjectView();

        void insertPolar(const Polar *pPolar);
        void insertFoil(Foil* pFoil);

        void fillModelView();
        void fillPolars(ObjectTreeItem *pFoilItem, Foil const*pFoil);
        void addOpps(Polar *pPolar);

        void removeOpPoint(OpPoint *pOpp);
        void removePolarOpps(Polar const*pPolar);
        void removeFoilPolars(Foil*pFoil);

        QString removePolar(Polar *pPolar);
        QString removeFoil(Foil* pFoil);
        QString removeFoil(const QString &foilName);

        void selectObjects();
        void setObjectProperties();

        void setOverallCheckStatus();

        void setCurveParams();
        void updateLineStyles();
        void updateVisibilityBoxes();

        void setPropertiesFont(QFont const &fnt);
        void setTreeFont(const QFont&fnt);

        QByteArray const &splitterSize() const {return m_SplitterSizes;}
        void setSplitterSize(QByteArray size) {m_SplitterSizes = size;}

        static void setXDirect(XDirect *pXDirect) {s_pXDirect = pXDirect;}
        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame = pMainFrame;}
        static void setDefaultWidth(int width) {s_Width=width;}

    private:
        Qt::CheckState foilState(const Foil *pFoil) const;
        Qt::CheckState polarState(const Polar *pPolar) const;


    public slots:
        void onItemClicked(const QModelIndex &index);
        void onCurrentRowChanged(QModelIndex currentIndex, QModelIndex previousIndex);
        void onItemDoubleClicked(const QModelIndex &index);

        void selectFoil(Foil*pFoil);
        void selectPolar(Polar*pPolar);
        void selectOpPoint(OpPoint *pOpp=nullptr);

        void onSwitchAll(bool bChecked);
        void onSetFilter();

    protected:
        static MainFrame *s_pMainFrame;
        static XDirect *s_pXDirect;

        enumSelectionType m_Selection;

        PlainTextOutput *m_ppto;

        static int s_Width;

    private:
        ExpandableTreeView *m_pTreeView;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QByteArray m_SplitterSizes;
        QSplitter *m_pMainSplitter;

};


