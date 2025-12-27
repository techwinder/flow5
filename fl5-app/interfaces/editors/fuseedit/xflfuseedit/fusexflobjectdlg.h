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
#include <QList>
#include <QStandardItem>
#include <QSplitter>
#include <QTreeView>

#include <QCheckBox>
#include <QDialogButtonBox>

#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldlg.h>

#include <api/enums_objects.h>
#include <api/vector3d.h>


class gl3dFuseView;
class FuseXfl;

class PointMass;
class EditObjectDelegate;
class ObjectTreeView;

class FuseXflObjectDlg : public FuseXflDlg
{
    Q_OBJECT

    public:
        FuseXflObjectDlg(QWidget *pParent = nullptr);


        bool bChanged() const {return m_bChanged;}

        void initDialog(Fuse*pFuse) override;
        void setActiveFrame(int iFrame);


    private:
        void connectSignals();
        void identifySelection(const QModelIndex &indexSel);
        void insertPointBefore();
        void insertPointAfter();
        void removeSelectedPoint();
        void setBody(FuseXfl *pFuseXfl) override;
        void enableStackBtns() override;
        void readPointMassTree(PointMass &ppm, QModelIndex indexLevel);
        void readBodyFrameTree(Frame &pFrame, QModelIndex indexLevel);
        void setupLayout();
        void fillFuseXflTreeView();
        void readFuseTree(QModelIndex indexLevel);
        void readInertiaTree(double &volumeMass, QVector<PointMass> &pointMasses, QModelIndex indexLevel);
        void readVectorTree(Vector3d &V, QModelIndex indexLevel);
        void selectThing(int iFrame, int iPoint=-1);

        void updateFuseDlg() override;

        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        QList<QStandardItem *> prepareRow(const QString &first, const QString &second="", const QString &third="",  const QString &fourth="");
        QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
        QList<QStandardItem *> prepareIntRow(const QString &first, const QString &second, const int &third);
        QList<QStandardItem *> prepareDoubleRow(const QString &first, const QString &second, const double &third,  const QString &fourth);

    private slots:
        void onFrameClickedIn2dView() override;
        void onPointClickedIn2dView() override;
        void onUpdateFuseDlg();
        void onConvertToFlatFace() override;

        void onRefillBodyTree();
        void onItemClicked(const QModelIndex &index);
        void onResizeColumns();

        void onRedraw();

        void onInsertBefore();
        void onInsertAfter();
        void onDelete();


    public:
        static QByteArray m_HSplitterSizes;


    private:

        ObjectTreeView * m_pStruct;
        EditObjectDelegate *m_pDelegate;
        QStandardItemModel *m_pModel;

        QMenu *m_pContextMenu;
        QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteItem;

        QSplitter *m_pHSplitter;

        bool m_bIsInertiaSelected;
        int m_iActivePointMass;
};

