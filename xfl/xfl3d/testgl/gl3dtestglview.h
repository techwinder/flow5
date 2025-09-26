/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xfl3d/views/gl3dview.h>
#include <xflgeom/geom3d/vector3d.h>


class gl3dTestGLView : public gl3dView
{
    Q_OBJECT

    public:
        gl3dTestGLView(QWidget *pParent = nullptr);
        void glRenderView() override;
        bool intersectTheObject(Vector3d const&, Vector3d const &, Vector3d &) override {return false;}
        QSize sizeHint() const override {return QSize(1500, 900);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    signals:
        void ready();

    protected:
        bool m_bInitialized;
};

