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

#include "gl3dplanexflview.h"
#include <interfaces/opengl/controls/colourlegend.h>

class PlaneXfl;
class PlanePolar;
class PlaneOpp;


class gl3dOptimXflView : public gl3dPlaneXflView
{
    Q_OBJECT

    public:
        gl3dOptimXflView(QWidget *pParent = nullptr);
        ~gl3dOptimXflView();

        void setSectionEndPoint(QVector<Node> &endpts, QVector<QColor> const&colors);
        void setSectionCp(std::vector<Node> &SectionPts, std::vector<double> &sectioncp, QVector<QColor> const &colors);
        void setPlaneOpp(PlaneOpp*pPOpp) {m_pPOpp=pPOpp; m_bResetCp=true;}

        static int iColorMap() {return s_iColorMap;}
        static void setColorMap(int imap) {s_iColorMap=imap;}

    private:
        void glRenderView() override;
        void glMake3dObjects() override;
        void paintOverlay() override;
        void glMakeNodeArrows();
        void paintSections(WingXfl const*pWing);
        void paintMeshPanels() override;
        void resizeGL(int w, int h) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    private slots:
        void onCp(      bool b);
        void onGamma();
        void onSigma();
        void onContours(bool b);
        void onSections(bool b);

    private:
        bool m_bShowCp, m_bShowContours, m_bShowSections;

        bool m_bResetArrows;
        bool m_bResetCp;

        std::vector<Node> m_CpNodes;
        QVector<QColor> m_NodeColors;

        QVector<QColor> m_SectionColors;
        QVector<Node>m_SectionEndPoint;

        QOpenGLBuffer m_vboSectionArrows;

        PlaneOpp const*m_pPOpp;

        QOpenGLBuffer m_vboCp;
        QOpenGLBuffer m_vboContours;

        ColourLegend m_ColourLegend;


        static int s_iColorMap;
};

