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

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/optim/particle.h>


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


