/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QTabWidget>
#include <QSettings>


#include <xflgeom/geom3d/quaternion.h>
#include <xflwidgets/customdlg/xfldialog.h>

class Boat;
class Fuse;
class Sail;
class gl3dBoatView;
class gl3dGeomControls;
class CPTableView;
class ActionItemModel;
class XflDelegate;
class PlainTextOutput;

class BoatDlg : public XflDialog
{
    Q_OBJECT

    public:
        BoatDlg(QWidget *pParent=nullptr);
        ~BoatDlg();

        void initDialog(Boat *pBoatDef, bool bEnableName);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(1100,900);}

        static void loadSettings(QSettings &pSettings);
        static void saveSettings(QSettings &settings);


    private slots:
        void onAddHull();
        void onAddSail();
        void onDeleteHull();
        void onDeleteSail();
        void onDuplicateHull();
        void onDuplicateSail();
        void onEditHull();
        void onEditJib();
        void onEditMainSail();
        void onEditSail();
        void onGetHull();
        void onHullCellChanged(QWidget *pWidget);
        void onHullItemClicked(const QModelIndex &index);
        void onImportHullCAD();
        void onImportHullSTL();
        void onImportHullXML();
        void onImportSailFromBoat();
        void onImportSailFromCAD();
        void onImportSailFromSTL();
        void onImportSailFromXml();
        void onInsertFuseStl();
        void onInsertFuseXfl();
        void onMoveHullDown();
        void onMoveHullUp();
        void onMoveSailDown();
        void onMoveSailUp();
        void onNodeDistance();
        void onPickedNodePair(QPair<int, int> nodepair);
        void onRenameHull();
        void onRenameSail();
        void onResizeTableColumns();
        void onSailCellChanged(QWidget *pWidget);
        void onSailItemClicked(const QModelIndex &index);

        void accept() override;
        void reject() override;


    private:
        void setupLayout();
        void setControls();
        void connectSignals();
        void fillSailList();
        void fillHullList();
        void readData();
        void setSail(int iSelect);
        void setHull(int iSelect);
        void editHull(int iFuse);

        void editSail(Sail *pSail);
        void editSplineSail(Sail *pSail);
        void editOccSail(Sail *pSail);
        void editStlSail(Sail *pSail);
        void editNurbsSail(Sail *pSail);
        void editWingSail(Sail *pSail);

    private:
        Boat *m_pBoat;

        gl3dBoatView *m_pglBoatView;
        gl3dGeomControls *m_pglBoatCtrls;

        QTabWidget *m_pPartTabWt;
        QLineEdit *m_pleBtName;
        QTextEdit *m_pteBtDescription;

        CPTableView *m_pcptSails, *m_pcptHulls;
        ActionItemModel *m_pSailModel, *m_pHullModel;
        XflDelegate *m_pSailDelegate, *m_pHullDelegate;

        QPushButton *m_ppbAddSail;

        QPushButton *m_ppbAddHull;

        PlainTextOutput *m_pptoOutput;


        QSplitter *m_pViewHSplitter, *m_pVPartSplitter;
        static QByteArray s_HSplitterSizes;
        static QByteArray s_VSplitterSizes;
        static QByteArray s_WindowGeometry;

        static Quaternion s_ab_quat;
};

