/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QSplitter>
#include <QModelIndex>

#include <xflcore/fontstruct.h>
#include <xflcore/enums_core.h>

class Plane;
class WPolar;
class PlaneOpp;
class ExpandableTreeView;
class ObjectTreeModel;
class ObjectTreeItem;
class ObjectTreeDelegate;
class PlainTextOutput;
class MainFrame;
class XPlane;

class PlaneTreeView : public QWidget
{
    Q_OBJECT

    public:
        enum enumSelectionType {NOOBJECT, PLANE, WPOLAR, PLANEOPP};

    public:
        PlaneTreeView(QWidget *pParent = nullptr);

        void insertPlane(Plane *pPlane);
        void insertWPolar(const WPolar *pWPolar);

        QString removePlane(const QString &planeName);
        QString removePlane(Plane *pPlane);
        QString removeWPolar(WPolar *pWPolar);
        void removeWPolars(const Plane *pPlane);
        void removePlaneOpp(PlaneOpp *pPOpp);
        void removeWPolarPOpps(const WPolar *pWPolar);

        void selectPlane(Plane* pPlane);
        void selectWPolar(WPolar *pWPolar, bool bSelectPOpp);
        void selectPlaneOpp(PlaneOpp *pPOpp=nullptr);

        void selectCurrentObject();
        void setObjectFromIndex(QModelIndex filteredindex);

        void addPOpps(WPolar const *pWPolar=nullptr);
        void updatePOpps();
        void fillModelView();
        void fillWPolars(ObjectTreeItem *pPlaneItem, const Plane *pPlane);
        void selectObjects();
        void setCurveParams();

        void updatePlane(Plane const *pPlane);

        QByteArray const &splitterSize() {return s_SplitterSizes;}
        void setSplitterSize(QByteArray size) {s_SplitterSizes = size;}

        void setObjectProperties();
        void setPropertiesFont(QFont const &fnt);
        void setTreeFontStruct(const FontStruct &fntstruct);

        enumSelectionType selectedType() const {return m_Selection;}
        bool isPlaneSelected()  const {return m_Selection==PLANE;}
        bool isWPolarSelected() const {return m_Selection==WPOLAR;}
        bool isPlaneOpp()       const {return m_Selection==PLANEOPP;}

        void setOverallCheckStatus();

        Qt::CheckState planeState(const Plane *pPlane) const;
        Qt::CheckState wPolarState(const WPolar *pWPolar) const;

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

