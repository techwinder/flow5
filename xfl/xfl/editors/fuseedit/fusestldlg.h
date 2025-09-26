/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

#include <xfl/editors/fuseedit/fusedlg.h>

#include <xfl/opengl/gl3dfuseview.h>
#include <xfl/opengl/gl3dgeomcontrols.h>

class FuseStl;
class FloatEdit;
class IntEdit;
class PlainTextOutput;

class FuseStlDlg : public FuseDlg
{
    Q_OBJECT

    public:
        FuseStlDlg(QWidget *pParent);
        void initDialog(Fuse *pFuse) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();

        void updateOutput(QString const &msg) override;

        void enableControls(bool bEnable);
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void updateProperties(bool bFull=false) override;

    private slots:
        void onSplitMeshPanels();
        void onRestoreDefaultMesh();

    private:
        QPushButton *m_ppbSplitTriangles, *m_ppbRestoreDefaultMesh;

        FloatEdit *m_pdeMaxEdgeLength;
        IntEdit *m_pieMaxPanelCount;

        PlainTextOutput *m_pptoOutput;

        FuseStl *m_pFuseStl;

        static QByteArray s_Geometry;
};


