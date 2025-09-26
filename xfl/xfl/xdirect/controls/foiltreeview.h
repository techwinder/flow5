/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QTreeView>
#include <QToolBar>
#include <QSplitter>
#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>

#include <xflcore/enums_core.h>
#include <xflcore/linestyle.h>

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

class FoilTreeView : public QWidget
{
    Q_OBJECT

    public:
        /** @enum The different objects selectable in the FoilTreeView */
        enum enumSelectionType {NONE, FOIL, POLAR, OPPOINT};

    public:
        FoilTreeView(QWidget *pParent = nullptr);
        ~FoilTreeView() override;

        void contextMenuEvent(QContextMenuEvent *event) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        QSize sizeHint() const override {return QSize(s_Width, 0);}
        void resizeEvent(QResizeEvent *pEvent) override;

        void setupLayout();

        void setObjectFromIndex(const QModelIndex &filteredindex);

        void updateObjectView();

        void insertPolar(Polar *pPolar);
        void insertFoil(Foil* pFoil);

        void fillModelView();
        void fillPolars(ObjectTreeItem *pFoilItem, Foil const*pFoil);
        void addOpps(Polar *pPolar);

        void removeOpPoint(OpPoint *pOpp);
        QString removePolar(Polar *pPolar);
        QString removeFoil(Foil* pFoil);
        QString removeFoil(QString foilName);

        void setObjectProperties();

        void setOverallCheckStatus();

        void setCurveParams();
        void setPropertiesFont(QFont const &fnt);

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

        PlainTextOutput *m_pptoObjectProps;

        static int s_Width;

    private:
        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QByteArray m_SplitterSizes;
        QSplitter *m_pMainSplitter;

};


