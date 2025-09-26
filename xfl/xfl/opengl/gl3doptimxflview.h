/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include "gl3dplanexflview.h"
#include <xfl3d/controls/colourlegend.h>

class PlaneXfl;
class WPolar;
class PlaneOpp;


class gl3dOptimXflView : public gl3dPlaneXflView
{
    Q_OBJECT

    public:
        gl3dOptimXflView(QWidget *pParent = nullptr);
        ~gl3dOptimXflView();

        void setSectionEndPoint(QVector<Node> &endpts, QVector<QColor> const&colors);
        void setSectionCp(QVector<Node> &SectionPts, QVector<double> &sectioncp, QVector<QColor> const &colors);
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

        QVector<Node> m_CpNodes;
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

