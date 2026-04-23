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
#include <QSettings>
#include <QProgressDialog>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/opengl/controls/colourlegend.h>
#include <api/boid.h>

class Boat;
class BoatPolar;
class BoatOpp;
class XSailDisplayCtrls;
class Opp3dScalesCtrls;
class Opp3d;
class P3LinAnalysis;
class P3UniAnalysis;
class P4Analysis;
class XSail;
class CrossFlowCtrls;
class Vorton;

class gl3dXSailView : public gl3dXflView
{
    friend class XSailDisplayCtrls;
    friend class Opp3dScalesCtrls;
    friend class XSail;
    friend class CrossFlowCtrls;

    Q_OBJECT

    public:
        gl3dXSailView();
        ~gl3dXSailView() override;

        bool intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I) override;

        double objectReferenceLength() const override;

        void glMake3dObjects() override;
        void glRenderView() override;
        void paintOverlay() override;

        void glRenderGeometry();
        void glRenderPanelBasedBuffers();
        void glRenderOppBuffers();
        void glRenderFlow();
        void cancelFlow();

        void makeVisiblePanel3List(Boat const *pBoat, const BoatPolar *pBtPolar, std::vector<Panel3> &panel3list);

        void paintForce(QOpenGLBuffer &vbo);

        void setBoat(Boat const*pBoat);

        void setResultControls(XSailDisplayCtrls *pResults3dControls, CrossFlowCtrls *pContourCtrls);
        void setCrossFlowCtrls(CrossFlowCtrls *pContourCtrls) {m_pCrossFlowCtrls = pContourCtrls;}

        void setLiveVortons(std::vector<std::vector<Vorton>> const &vortons);

        void startFlow(bool bStart);
        void resetFlow() {m_bResetFlowPanels=m_bResetBoids=true;}
        void restartFlow();


        static void resetglBoat()  {s_bResetglBoat = s_bResetglBtOpp = true;}
        static void resetglSail()  {s_bResetglSail = s_bResetglBtOpp = true;}
        static void resetglHull()  {s_bResetglHull = true;}
        static void resetglMesh()  {s_bResetglMesh = s_bResetglBtOpp = true;}
        static void resetglBtOpp() {s_bResetglBtOpp = true;}
        static void resetglDrag()  {s_bResetglDrag = true;}
        static void resetglStreamLines() {s_bResetglStream = true;}
        static void resetglColorMap() {s_bResetglPanelGamma=s_bResetglPanelCp=s_bResetglPanelForce=true;}

        static void setXSail(XSail *pXSail) {s_pXSail=pXSail;}

        void loadXSailSettings(QSettings &settings);
        void saveXSailSettings(QSettings &settings);

    protected:
        void contextMenuEvent (QContextMenuEvent * pEvent) override;
        void hideEvent(QHideEvent*pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void resizeGL(int width, int height) override;

    protected slots:
        void onWindBack();
        void onWindFront();
        void onCancelThreads();
        void onPartSelClicked() override;


    private:
        void resizeSailBuffers(int nSails);

        bool pickTriUniPanel(QPoint const &point);
        bool pickTriLinPanel(QPoint const &point);

        void glMakeMeshBuffers();
        void glMakeOppBuffers();
        void glMakeWindSpline();
        void glMakeSailForces(BoatOpp const *pBtOpp, float qDyn, QOpenGLBuffer &vbo);

        bool glMakeStreamLines(const std::vector<Panel3> &panel3list, const Boat *pBoat, BoatOpp const *pBtOpp);
        void computeP3VelocityVectors(const Opp3d *pPOpp, QVector<Vector3d> const &points, QVector<Vector3d> &velvectors);

        void makeTriVelocityBlock( int iBlock, QVector<Vector3d> const &C, double const *mu, double const *sigma, Vector3d *VField) const;

        void makeBoids();
        void moveBoids();
        void glMakeFlowBuffers();

        void initializeGL() override;

    private:

        ColourLegend m_LegendOverlay;

        XSailDisplayCtrls *m_pDisplayCtrls;
        Opp3dScalesCtrls *m_pOpp3dScalesCtrls;
        CrossFlowCtrls *m_pCrossFlowCtrls;

        double m_LiveCtrlParam;
        std::vector<std::vector<Vorton>> m_LiveVortons;

        int m_nBlocks;

        double m_PickedVal;

        QVector<QOpenGLBuffer> m_vboSailSurface;
        QVector<QOpenGLBuffer> m_vboSailOutline;
        QVector<QOpenGLBuffer> m_vboSailNormals;
        QVector<QOpenGLBuffer> m_vboSailMesh;
        QVector<QOpenGLBuffer> m_vboFuseTriangulation;
        QVector<QOpenGLBuffer> m_vboFuseOutline;

        QOpenGLBuffer m_vboPanelCp;
        QOpenGLBuffer m_vboPanelForces;
        QOpenGLBuffer m_vboStreamlines;
        QOpenGLBuffer m_vboSurfaceVelocities;
        QOpenGLBuffer m_vboQuadWake;
        QOpenGLBuffer m_vboTriWake;
        QOpenGLBuffer m_vboPanelNormals;
        QOpenGLBuffer m_vboVortons;
        QOpenGLBuffer m_vboContourClrs;
        QOpenGLBuffer m_vboContourLines;
        QOpenGLBuffer m_vboVortonLines;
        QOpenGLBuffer m_vboGridVelocities;
        QOpenGLBuffer m_vboTTWSpline;
        QOpenGLBuffer m_vboBackgroundQuad;
        QOpenGLBuffer m_vboHPlane;
        QOpenGLBuffer m_vboTessHull, m_vboTessSail;

        QVector<Vector3d> m_TrueWindSpline;

        bool m_bWindFront, m_bWindBack;


        P3UniAnalysis *m_pP3UniAnalysis;
        P3LinAnalysis *m_pP3LinAnalysis;

        QOpenGLBuffer m_vboTriMesh, m_vboTriMeshEdges;
        QOpenGLBuffer m_vboNormals;


        //flow
        //flow CS locations
        FlowLocations m_shadFlowLoc;

        float m_FlowYPos;
        bool m_bResetFlowPanels;
        bool m_bResetBoids;
        QVector<Boid> m_Boid;

        QOpenGLShaderProgram m_shadFlow;
        QOpenGLBuffer m_ssboPanels, m_ssboVortons;
        QOpenGLBuffer m_ssboBoids;
        QOpenGLBuffer m_vboTraces;
        QTimer m_FlowTimer;

        QVector<QColor> m_StreamLineColours;

        static double s_GammaCoef;

        static bool s_bResetglBoat;               /**< true if the geometry OpenGL list needs to be re-generated */
        static bool s_bResetglSail;
        static bool s_bResetglHull;
        static bool s_bResetglMesh;               /**< true if the mesh OpenGL list needs to be re-generated */
        static bool s_bResetglWake;               /**< true if the wake OpenGL list needs to be re-generated */
        static bool s_bResetglBtOpp;              /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglLift;               /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglMoments;            /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglDrag;               /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglPanelGamma;         /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglPanelCp;            /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglPanelForce;         /**< true if the OpenGL lists need to be re-generated */
        static bool s_bResetglStream;             /**< true if the streamlines OpenGL list needs to be re-generated */
        static bool s_bResetglBodyMesh;           /**< true if the openGL list for panel mesh needs to be re-generated */

        static XSail *s_pXSail;
};

