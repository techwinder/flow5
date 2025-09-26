/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xfl/opengl/gl3dxflview.h>
#include <xfloptim/tasks/particle.h>


class gl3dParetoView : public gl3dXflView
{
    public:
        gl3dParetoView(QWidget *pParent = nullptr);

        void setBox(double x, double y, double z) {m_X=x; m_Y=y; m_Z=z; m_bResetBox=true;}
        void setSwarm(QVector<Particle> const &swarm) {m_Swarm=swarm;}
        void setPareto(QVector<Particle> const &pareto) {m_Pareto=pareto;}
        void setBest(int ibest) {m_iBest=ibest;}

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

    private:
        bool intersectTheObject(Vector3d const &AA, Vector3d const &B, Vector3d &I) override;


    private:
        bool m_bResetBox;
        double m_X, m_Y, m_Z; // the side lengths of the target box:
        int m_iBest;

        QVector<Particle> m_Swarm;
        QVector<Particle> m_Pareto;

        QOpenGLBuffer m_vboBox, m_vboBoxEdges;
};


