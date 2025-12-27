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
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QGroupBox>

#include <interfaces/opengl/fl5views/gl3dfuseview.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class FuseStl;
class FloatEdit;
class IntEdit;
class ColorBtn;
class gl3dGeomControls;

class FuseDlg : public XflDialog
{
    Q_OBJECT
    public:
        FuseDlg(QWidget *pParent);

        void enableName(bool bEnable)  {m_bEnableName=bEnable;}
        bool geometryChanged() const {return m_bChanged;}

        void hideSaveAsNew() {m_ppbSaveAsNew->hide();}

        QSize sizeHint() const override {return QSize(1100,900);}


    protected:
        virtual void accept() override;

        virtual void initDialog(Fuse *pFuse);

        virtual void updateStdOutput(std::string const &) {}
        virtual void updateOutput(QString const &) {}
        virtual void updateProperties(bool bFull=false) = 0;
        virtual void updateView();

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        void makeWidgets();
        void connectBaseSignals();
        void createBaseActions();

    protected slots:
        void onMetaDataChanged();
        void onTessellation();
        void onFuseInertia();
        virtual void onScale();
        virtual void onTranslate();
        virtual void onRotate();
        void onExportMeshToSTLFile();
        void onExportTrianglesToSTLFile();
        void onExportFuseToCADFile();
        void onFuseColor();
        void onNodeDistance();
        void onPickedNodePair(QPair<int, int> nodepair);

        virtual void onButton(QAbstractButton *pButton) override;

    protected:

        gl3dFuseView *m_pglFuseView;
        gl3dGeomControls *m_pglControls;

        ColorBtn *m_pcbColor;
        QLineEdit *m_pleName;
        QTextEdit *m_pteDescription;

        QAction *m_pRotate, *m_pScale, *m_pTranslate;


        QAction *m_pFuseInertia;
        QAction *m_pExportMeshToSTL, *m_pExportTrianglesToSTL, *m_pExportToCADFile;
        QAction *m_pTessSettings;

        QAction *m_pBackImageLoad, *m_pBackImageClear, *m_pBackImageSettings;

        QFrame *m_pMetaFrame;

        QPushButton *m_ppbSaveAsNew;

        Fuse *m_pFuse;

        bool m_bEnableName;

        static bool s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses;
        static Quaternion s_ab_quat;

    public:
        static QVector<Vector3d> s_DebugPts;
};


