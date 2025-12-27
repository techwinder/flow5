/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QWidget>
#include <QComboBox>
#include <QThread>
#include <QTimer>
#include <QPushButton>
#include <QSettings>

#include <api/gmshparams.h>
#include <api/node.h>
#include <api/wingxfl.h>

class Fuse;
class Sail;
class FloatEdit;
class IntEdit;
class PlainTextOutput;
class GMesher;
class Vector3d;
class Node;
class Triangle3d;
class WingXfl;


class GMesherWt : public QFrame
{
    Q_OBJECT
    public:
        GMesherWt(QWidget *pParent);
        ~GMesherWt();

        void initWt(Fuse *pFuse, bool bMakexzSymmetric, bool bThickSurfaces);
        void initWt(Sail *pSail);

        std::vector<Triangle3d> const &triangles() const {return m_Triangles;}

        void setAlgo(int iAlgo) {s_idxAlgo=iAlgo;}

        void setWings(QVector<WingXfl> const &wings) {m_Wings=wings;}
        void clearWings() {m_Wings.clear();}

        QSize sizeHint() const override {return QSize(300,150);}
        QSize minimumSizeHint() const override {return QSize(100,50);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onCheckLogger();
        void onHandleMeshResults(bool bError);
        void onKillMeshThread();
        void onMesh();

    private:
        void setupLayout();

        void meshNURBSSail();
        void meshSplineSail();
        void meshOccSail();
        void meshFuseShellsThinSurfaces();
        void meshFuseShellsThickSurfaces();

        int  meshAlgo();
        bool readMeshSize();
        void convertFromGmsh();

    signals:
        void meshCurrent();
        void updateFuseView();
        void outputMsg(QString);

    private:
        FloatEdit *m_pfeMinSize, *m_pfeMaxSize;
        IntEdit *m_pieFromCurvature;
        QComboBox *m_pcbMeshAlgo;

        QPushButton *m_ppbMesh;

        PlainTextOutput *m_ppto;
        QTimer m_LogTimer;


        QWidget *m_pParent;
        bool m_bIsMeshing;
        bool m_bMakexzSymmetric;
        bool m_bThickSurfaces;

        int m_iLoggerStack;


        std::vector<Triangle3d> m_Triangles; /**< the resulting triangles */
        std::vector<Node> m_Nodes;

        QVector<QVector<Vector3d>> m_Curves;

        QThread m_MeshThread;
        GMesher *m_pWorker;

        Fuse *m_pFuse;
        Sail *m_pSail;

        QVector<WingXfl> m_Wings; /** if not empty, used to embed mid lines in the fuse mesh */

        static int s_idxAlgo;
};

