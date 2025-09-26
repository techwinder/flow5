/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QListWidget>


#include <xfl3d/testgl/gl3dtestglview.h>
#include <xflgeom/geom3d/vector3d.h>
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


