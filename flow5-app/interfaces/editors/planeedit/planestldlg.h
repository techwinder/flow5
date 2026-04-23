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

#include <QLabel>
#include <QDialog>
#include <QStack>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSplitter>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTextEdit>
#include <QSettings>
#include <QLineEdit>

#include <interfaces/editors/planeedit/planedlg.h>
#include <api/triangle3d.h>
#include <api/linestyle.h>
#include <api/trimesh.h>

class Plane;
class PlaneSTL;

class gl3dPlaneSTLView;
class gl3dGeomControls;

class PlaneStl;
class FloatEdit;
class IntEdit;
class ColorBtn;
class PlainTextOutput;

class PlaneSTLDlg : public PlaneDlg
{
    Q_OBJECT

    public:
        PlaneSTLDlg(QWidget *pParent);

        void initDialog(Plane *pPlane, bool bAcceptName) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onBotTEPanels();
        void onCheckTEPanels();
        void onFlipNormals() override;
        void onGuessTEPanels();
        void onInitializePlane();
        void onOK(int iExitCode=QDialog::Accepted) override;
        void onPanelSelected(int);
        void onPlaneInertia() override;
        void onReferenceDims();
        void onRotate();
        void onScale();
        void onSetColor();
        void onTopTEPanels();
        void onTranslate();
        void onMakeMeshFromTriangles();
        void onPickedNodePair(QPair<int, int> nodepair);
        void onUpdatePlane() override;

    private:
        void connectSignals();

        void setupLayout();
        void setControls() override;

        void makeTESegments();

        bool deselectButtons();


    private:
        PlaneSTL *m_pPlaneSTL;

        QTabWidget *m_pLeftTabWt;

        QAction *m_pConvertTriangles;

        FloatEdit *m_pfeRefArea, *m_pfeRefChord, *m_pfeRefSpan;

        FloatEdit *m_pfeRotate, *m_pfeTranslate, *m_pfeScale;
        QPushButton *m_ppbTranslateX, *m_ppbTranslateY, *m_ppbTranslateZ;
        QPushButton *m_ppbRotateX, *m_ppbRotateY, *m_ppbRotateZ;
        QPushButton *m_ppbScale;

        QPushButton *m_ppbTopTEPanels, *m_ppbBotTEPanels, *m_ppbGuessTE;
        QPushButton *m_ppbCheckTE;
        QCheckBox *m_pchGuessOpposite;
        FloatEdit *m_pfeTEAngle;

        ColorBtn *m_pcbColor;

        QStack<TriMesh> m_MeshStack;
        QSplitter *m_pVSplitter;

        static bool s_bGuessOpposite;
        static QByteArray s_VSplitterSizes;
        static QByteArray s_STLGeometry;
};



