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

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSlider>
#include <QRadioButton>

#include <QItemSelectionModel>
#include <QPushButton>
#include <QToolButton>
#include <QSplitter>
#include <QSettings>
#include <QPixmap>

#include <interfaces/editors/fuseedit/fusedlg.h>
#include <interfaces/widgets/view/grid.h>


class FuseXfl;
class FuseLineWt;
class FuseFrameWt;
class Frame;

class FloatEdit;
class PlainTextOutput;

class FuseXflDlg : public FuseDlg
{
    Q_OBJECT
    public:
        FuseXflDlg(QWidget *pParent=nullptr);
        virtual ~FuseXflDlg();

        virtual void initDialog(Fuse *pFuse) override;

    protected slots:
        virtual void onFrameClickedIn2dView() = 0;
        virtual void onPointClickedIn2dView() = 0;

        virtual void setPicture() {}

        virtual void onScaleFuse();
        virtual void onScaleFuse(bool bFrameOnly);
        virtual void onTranslateFuse();

        virtual void onConvertToFlatFace();

        void onRemoveFrame(int iFrame);
        void onInsertFrame(Vector3d const &pos);
        void onRemovePoint(int iPt);
        void onInsertPoint(const Vector3d &pos);
        void onExportFuseToXML();
        void onResetFuse();
        void accept() override;
        void onResetScales();
        void onUndo();
        void onRedo();

    protected:
        void connectFuseXflSignals();
        void createActions();
        void makeCommonWts();
        virtual void updateFuseDlg() = 0;
        void updateFuseXfl();
        void updateView() override;
        void updateProperties(bool bFull=false) override;
        virtual void setBody(FuseXfl *pFuseXfl);

        virtual void enableStackBtns() = 0;

        void clearStack(int pos=0);
        void takePicture();

        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

    protected:
        FuseXfl *m_pFuseXfl;
        int m_StackPos;                /**< the current position on the Undo stack */
        QList<Fuse const*> m_UndoStack;      /**< the stack of incremental modifications to the SplineFoil;
                                     we can't use the QStack though, because we need to access
                                     any point in the case of multiple undo operations */
        FuseLineWt *m_pFuseLineView;
        FuseFrameWt *m_pFrameView;

        QAction *m_pResetFuse;
        QAction *m_pExportBodyXML;
        QAction *m_pScaleBody, *m_pTranslateBody;
        QAction *m_pToFlatFace;

        QPushButton *m_ppblRedraw, *m_ppbMenuButton;

        QSplitter *m_pViewHSplitter, *m_pViewVSplitter;

        static bool s_bShowCtrlPoints;
        static QByteArray s_VViewSplitterSizes, s_HViewSplitterSizes;
        static QByteArray s_Geometry;
};


