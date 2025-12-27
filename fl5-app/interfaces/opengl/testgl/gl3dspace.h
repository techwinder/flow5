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

#include <QListWidget>


#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <api/vector3d.h>
#include "spaceobject.h"

class IntEdit;
class FloatEdit;


class gl3dSpace : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dSpace(QWidget *pParent = nullptr);
        ~gl3dSpace() override = default;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:

        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        void glRenderView() override;
        void glMake3dObjects() override;
        void selectListItem(int index);
        void makeGalaxies();

        QString degTo24(double Ra) const;

        int lwToGalaxyIndex(int idx) const;
        int galaxyToLwIndex(int iGal) const;


    private slots:

        void onNGalaxies();
        void onObjectRadius(int size);
        void onSelGalaxy(QListWidgetItem *);

    private:
//        QOpenGLVertexArrayObject m_vaoSpace;
        QOpenGLBuffer m_vboTetra, m_vboTetraEdges;
        QOpenGLBuffer m_vboInstPositions;

        QOpenGLBuffer m_vboArcSegments;
        QOpenGLBuffer m_vboRadius;

        QVector<Star> m_Galaxies;

        IntEdit *m_pieNGalaxies;
        QListWidget *m_plwGalaxies;

        bool m_bResetInstances;
        bool m_bResetArcs;
        int m_iSelIndex;
        int m_iCloseIndex;
        Vector3d m_RaLoc, m_DaLoc;

        Qt::MouseButton m_BtnPressed;

        static int s_NObjects;
        static double s_SphereRadius;
};


