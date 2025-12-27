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


#include <QStack>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/opengl/controls/colourlegend.h>
#include <api/panel4.h>
#include <api/panel3.h>
#include <api/boid.h>


class CrossFlowCtrls;
class PlaneXfl;
class Panel3;
class Panel4;
class POpp3dCtrls;
class TriMesh;
class glXPlaneBuffers;
class P4Analysis;
class P3UniAnalysis;
class P3LinAnalysis;
class Vorton;
class Opp3d;

class gl3dXPlaneView : public gl3dXflView
{
    Q_OBJECT

    friend class XPlane;
    friend class CrossFlowCtrls;
    friend class POpp3dCtrls;

    public:
        gl3dXPlaneView(QWidget *parent = nullptr);
        ~gl3dXPlaneView() override;

        double objectReferenceLength() const override;

        void resetWakeControls();

        void setPlane(const Plane *pPlane);

        void setVisiblePanels(std::vector<Panel4> const &p4visible) {m_Panel4Visible=p4visible;}
        void setVisiblePanels(std::vector<Panel3> const &p3visible) {m_Panel3Visible=p3visible;}
        void setVisibleNodes(std::vector<Node> const &visiblenodes) {m_NodeVisible = visiblenodes;}

        glXPlaneBuffers *viewBuffers() {return m_pglXPlaneBuffers;}
        void setXPlaneBuffers(glXPlaneBuffers *pXPlaneBuffers) {m_pglXPlaneBuffers=pXPlaneBuffers;}
        void setSharedBuffers(gl3dXPlaneView *pgl3dXPlaneView);

        void setResultControls(POpp3dCtrls *pResults3DControls);
        void setCrossFlowCtrls(CrossFlowCtrls *pContourCtrls) {m_pCrossFlowCtrls = pContourCtrls;}


        void startFlow(bool bStart);
        void resetFlow() {m_bResetFlowPanels=m_bResetBoids=true;}


        void resetglPOpp();
        void resetglGeom()        {s_bResetglGeom=true;}
        void resetglStreamlines() {s_bResetglStream=true;}
        void resetglMesh()        {s_bResetglMesh=s_bResetglWake=true;}
        void resetglColorMap()    {s_bResetglPanelGamma=s_bResetglPanelCp=s_bResetglPanelForce=true;}
        void resetglVortons()     {s_bResetglVortons=true;}

        void showColourLegend(bool bShow) {m_ColourLegend.setVisible(bShow);}

        void restartFlow();

        static void setXPlane(XPlane *pXPlane) {s_pXPlane=pXPlane;}

    private:
        void glRenderView() override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void contextMenuEvent (QContextMenuEvent * pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        bool intersectTheObject(Vector3d const &AA, Vector3d const &U, Vector3d &I) override;

        void initializeGL() override;
        void resizeGL(int width, int h) override;
        void hideEvent(QHideEvent *) override;

        void glMakeLLTTransitions(PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp, QOpenGLBuffer &vbo) const;
        void glMakeTransitions(PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp, QOpenGLBuffer &vbo) const;
        void glMakeDragStrip(     PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp, bool bICd, bool bVCd);
        void glMakeLLTDragStrip(  PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp, bool bICd, bool bVCd);
        void glMakeMoments(double planformspan, PlanePolar const*pWPolar, PlaneOpp const*pPOpp, float reflength);
        void glMakeLiftStrip(     PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp);
        void glMakeLLTLiftStrip(  PlaneXfl const*pPlane, PlanePolar const*pWPolar, PlaneOpp const*pPOpp);

        void glRenderPanelBasedBuffers();
        void glRenderGeometryBasedBuffers();
        void glRenderPOppBasedBuffers();
        void glRenderFlow();
        void cancelFlow();

        bool pickTriUniPanel(QPoint const &point);
        bool pickQuadPanel(QPoint const &point);

        void paintMoments();

        bool glMakeStreamLines(const std::vector<Panel4> &panel4list, const PlaneOpp *pPOpp);
        bool glMakeStreamLines(const std::vector<Panel3> &panel3list, const std::vector<Node> &nodelist, const PlaneOpp *pPOpp);

        void makeQuadVelocityBlock(int iBlock, const QVector<Vector3d> &C, const double *mu, const double *sigma, Vector3d *VField) const;
        void makeTriVelocityBlock( int iBlock, const QVector<Vector3d> &C, const double *mu, const double *sigma, Vector3d *VField) const;

        void makeLLTDownwash(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp,
                             QVector<Vector3d> &points,QVector<Vector3d> &arrows) const;
        void makeDownwash(const PlaneXfl *pPlane, const PlanePolar *pWPolar, const PlaneOpp *pPOpp,
                          QVector<Vector3d> &points, QVector<Vector3d> &arrows) const;


        void glMake3dObjects() override;
        void glMakeOppBuffers();
        void glMakeFlowBuffers();

        void computeP4VelocityVectors(const Opp3d *pPOpp, QVector<Vector3d> const &points, QVector<Vector3d> &velvectors, bool bMultithread);
        void computeP3VelocityVectors(const Opp3d *pPOpp, const QVector<Vector3d> &points, QVector<Vector3d> &velvectors, bool bMultithread);

        void paintOverlay() override;

        void moveBoids();

    public slots:
        void onCancelThreads();
        void onPartSelClicked() override;
        void onUpdate3dScales();
        void onUpdate3dStreamlines();

    private:
        static XPlane *s_pXPlane;

        double m_PickedVal;

        bool m_bPickCenter;
        bool m_bShowCpScale;
        bool m_bPanelNormals;
        bool m_bNodeNormals;
        bool m_bVortices;

        QPixmap m_PixLegend;
        ColourLegend m_ColourLegend;

        QVector<QColor> m_StreamLineColours;

        int m_nBlocks;

        QOpenGLBuffer m_vboPickedQuad;
        QOpenGLBuffer m_vboHPlane;

        glXPlaneBuffers *m_pglXPlaneBuffers;
        POpp3dCtrls *m_pPOpp3dControls;
        CrossFlowCtrls *m_pCrossFlowCtrls;

        P4Analysis *m_pP4Analysis;
        P3UniAnalysis *m_pP3UniAnalysis;
        P3LinAnalysis *m_pP3LinAnalysis;

        std::vector<Panel4> m_Panel4Visible;
        std::vector<Panel3> m_Panel3Visible;
        std::vector<Node> m_NodeVisible;

        std::vector<Node> m_CpSections; /** Note: Node normal is not a unit vector, has Cp amplitude */

        QVector<Segment3d> m_Vortices;

        //flow CS locations
        FlowLocations m_shadFlowLoc;
        bool m_bResetFlowPanels;
        bool m_bResetBoids;
        float m_FlowYPos;

        QVector<Boid> m_Boid;

        QOpenGLShaderProgram m_shadFlow;
        QOpenGLBuffer m_ssboPanels, m_ssboVortons;
        QOpenGLBuffer m_ssboBoids;
        QOpenGLBuffer m_vboTraces;
        QTimer m_FlowTimer;
        QStack<int>m_stackInterval;


    public:
        static bool s_bResetglGeom;               /**< true if the geometry OpenGL list needs to be re-generated */
        static bool s_bResetglMesh;               /**< true if the mesh OpenGL list needs to be re-generated */
        static bool s_bResetglWake;               /**< true if the wake OpenGL list needs to be re-generated */
        static bool s_bResetglOpp;                /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglLift;               /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglMoments;            /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglDrag;               /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglDownwash;           /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglPanelGamma;         /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglPanelCp;            /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglPanelForce;         /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglStream;             /**< true if the streamlines OpenGL list needs to be re-generated */
        static bool s_bResetglVortons;            /**< true if the vortons need to be updated */
};


