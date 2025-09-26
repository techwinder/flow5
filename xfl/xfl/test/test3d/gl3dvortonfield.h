/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QComboBox>
#include <QCheckBox>

#include <xfl/opengl/gl3dxflview.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/vorton.h>
#include <xfl3d/controls/colourlegend.h>


class IntEdit;
class FloatEdit;

class gl3dVortonField : public gl3dXflView
{
    Q_OBJECT

    public:
        gl3dVortonField(QWidget *pParent = nullptr);

        void setVortons(QVector<Vorton> const& vortons) {m_pVortons=&vortons;}
        double vortonCoreSize()   const {return m_CoreSize;}
        void setVortonCoreSize(double l)   {m_CoreSize=l;}

        void setTriangle(Node* vertices) {m_Triangle.setVertices(vertices);}

        void setPanels(QVector<Panel3> const &panels) {m_pPanels=&panels;}
        void setPlotLine(Segment3d const &seg) {m_PlotLine=seg; m_bResetPlotLine=true;}

        void resetVectors() {m_bResetVectors=true;}
        void resetPanels()  {m_bResetPanels=true;}


        void makePlotLineVelocities(int iSource, int nPts);
        void makeVorticityColorMap();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void glRenderView() override;
        void glMake3dObjects() override;

        void glMakeVectors();

        bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) override;

        void makeControls();

        void paintVectors();

        void makeVorticityColorRow(double z, double t, int ir, int iPlaneDir);

        void makePlotLine();


        QSize sizeHint() const override {return QSize(1100,1000);}

        void showEvent(QShowEvent *pEvent) override;
        void resizeGL(int w, int h) override;



    private slots:
        void onRefLength();
        void onVelScale();
        void onMakeColorMap();
        void onContourPlane(bool bChecked);

    private:
        Triangle3d m_Triangle;

        double m_xl;
        double m_yl;

        int m_nPts;
        QVector<Vector3d> m_PointArray;
        QVector<Vector3d> m_VectorArray;
        QVector<Panel3> const *m_pPanels;
        QVector<Vorton> const *m_pVortons;

        //colormap and iso contours
        int m_nRows, m_nCols;
        QVector<Vector3d> m_Nodes;
        QVector<double> m_Values;

        QOpenGLBuffer m_vboPlotLine;
        QOpenGLBuffer m_vboVectors;
        QOpenGLBuffer m_vboTriangles, m_vboTriangleEdges;

        QOpenGLBuffer m_vboClrMap;
        QOpenGLBuffer m_vboContours;

        bool m_bResetVectors;
        bool m_bResetPanels;
        bool m_bResetPlotLine;
        bool m_bResetContour;


        Segment3d m_PlotLine;

        QFrame *m_pfrControls;

        QLabel *m_plabTitle;
        FloatEdit *m_pdeVelScale;
        FloatEdit *m_pdeRefLength;

        QCheckBox *m_pchContourPlane;
        QSlider *m_pslPlanePos;
        QComboBox *m_pcbPlaneDir;

        QComboBox *m_pcbOmegaDir;

        ColourLegend m_LegendOverlay;

        double m_CoreSize;

        static int s_iSource;
        static double s_VelScale;
        static double s_RefLength;
};



