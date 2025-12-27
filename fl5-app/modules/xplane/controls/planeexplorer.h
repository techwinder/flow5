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

#include <QSettings>
#include <QSplitter>
#include <QModelIndex>

#include <core/enums_core.h>

class Plane;
class PlanePolar;
class PlaneOpp;
class ExpandableTreeView;
class ObjectTreeModel;
class ObjectTreeItem;
class ObjectTreeDelegate;
class PlainTextOutput;
class MainFrame;
class XPlane;

class PlaneExplorer : public QWidget
{
    Q_OBJECT
    friend class BatchModeDlg;

    public:
    enum enumSelectionType {NOOBJECT, PLANE, WPOLAR, PLANEOPP, STABILITYMODE};

    public:
        PlaneExplorer(QWidget *pParent = nullptr);
        ~PlaneExplorer();

        void insertPlane(Plane *pPlane);
        void insertWPolar(const PlanePolar *pWPolar);

        QString removePlane(const QString &planeName);
        QString removePlane(Plane *pPlane);
        QString removeWPolar(PlanePolar *pWPolar);
        void removeWPolars(const Plane *pPlane);
        void removePlaneOpp(PlaneOpp *pPOpp);
        void removeWPolarPOpps(const PlanePolar *pWPolar);

        void selectPlane(Plane* pPlane);
        void selectPlPolar(PlanePolar *pWPolar, bool bSelectPOpp);
        void selectPlaneOpp(PlaneOpp *pPOpp=nullptr);

        void selectCurrentObject();
        void setObjectFromIndex(QModelIndex filteredindex);

        void addPOpps(const PlanePolar *pWPolar=nullptr);
        void fillModelView();
        void fillWPolars(ObjectTreeItem *pPlaneItem, const Plane *pPlane);
        void updatePOpps();

        void selectObjects();
        void setCurveParams();
        void updateLineStyles();
        void updateVisibilityBoxes();

        void updatePlane(Plane const *pPlane);

        QByteArray const &splitterSize() {return s_SplitterSizes;}
        void setSplitterSize(QByteArray size) {s_SplitterSizes = size;}

        void setObjectProperties();
//        void updateObjectView();
        void setPropertiesFont(QFont const &fnt);
        void setTreeFont(const QFont &fnt);

        enumSelectionType selectedType() const {return m_Selection;}
        bool isPlaneSelected()  const {return m_Selection==PLANE;}
        bool isWPolarSelected() const {return m_Selection==WPOLAR;}
        bool isPlaneOpp()       const {return m_Selection==PLANEOPP;}

        void setOverallCheckStatus();

        Qt::CheckState planeState(const Plane *pPlane) const;
        Qt::CheckState polarState(const PlanePolar *pWPolar) const;

        static void setDefaultWidth(int width) {s_Width=width;}
        static void setMainFrame(MainFrame*pMainFrame) {s_pMainFrame = pMainFrame;}
        static void setXPlane(XPlane*pXPlane) {s_pXPlane = pXPlane;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(s_Width, 0);}

    private slots:
        void onItemClicked(const QModelIndex &filteredindex);
        void onItemDoubleClicked(const QModelIndex &index);
        void onCurrentRowChanged(QModelIndex currentfilteredidx);
        void onSetFilter();

    public slots:
        void onSwitchAll(bool bChecked);

    private:
        void setupLayout();

    private:
        ExpandableTreeView *m_pTreeView;
        ObjectTreeModel *m_pModel;

        ObjectTreeDelegate *m_pDelegate;

        QSplitter *m_pMainSplitter;

        enumSelectionType m_Selection;

        PlainTextOutput *m_pptObjectProps;

        static QByteArray s_SplitterSizes;
        static int s_Width;
        static MainFrame *s_pMainFrame;
        static XPlane *s_pXPlane;
};

