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



#include <QPainter>
#include <QKeyEvent>
#include <QFormLayout>
#include <QtConcurrent/QtConcurrent>

#include "gl3dvortonfield.h"



#include <interfaces/opengl/globals/gl_xfl.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/panel2d.h>
#include <api/panel3.h>
#include <api/panel4.h>
#include <api/vorton.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

int gl3dVortonField::s_iSource = 1;
double gl3dVortonField::s_RefLength = 3.0;
double gl3dVortonField::s_VelScale  = 1.0;



gl3dVortonField::gl3dVortonField(QWidget *pParent) : gl3dXflView(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_pPanels  = nullptr;
    m_pVortons = nullptr;

    m_CoreSize = 0.1;

    // the panels
    m_xl = 5.0;
    m_yl = 2.0;

    // the vectors
    m_nPts = 5;

    // the color contour plane
    m_nRows = 151;
    m_nCols = 151;

    m_bResetPanels   = true;
    m_bResetVectors  = true;
    m_bResetPlotLine = true;
    m_bResetContour  = true;

    makeControls();

    setReferenceLength(s_RefLength);
}


void gl3dVortonField::makeControls()
{
    bool bContourPlane = false;

    m_pfrControls = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Text, DisplayOptions::textColor());

        QColor clr = DisplayOptions::backgroundColor();
        if(!DisplayOptions::isLightTheme())
            clr = Qt::white;

        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);

        m_pfrControls->setCursor(Qt::ArrowCursor);
        m_pfrControls->setFrameShape(QFrame::NoFrame);
        QHBoxLayout *pFrameLayout = new QHBoxLayout;
        {
            QVBoxLayout *pLeftLayout = new QVBoxLayout;
            {
                QGroupBox *pVectorBox = new QGroupBox("Velocity vectors");
                {
                    QFormLayout*pParamsLayout = new QFormLayout;
                    {
                        m_plabTitle    = new QLabel;
                        m_plabTitle->setStyleSheet("font: bold;");
                        m_pdeVelScale  = new FloatEdit(s_VelScale);
                        pParamsLayout->addRow(m_plabTitle);
                        pParamsLayout->addRow("Vector scale", m_pdeVelScale);

                    }
                    pVectorBox->setLayout(pParamsLayout);
                }

                QHBoxLayout *pViewLayout = new QHBoxLayout;
                {
                    m_pdeRefLength = new FloatEdit(s_RefLength);
                    pViewLayout->addWidget(new QLabel("Ref. length"));
                    pViewLayout->addWidget(m_pdeRefLength);
                    pViewLayout->addStretch();
                }
                pLeftLayout->addWidget(pVectorBox);
                pLeftLayout->addStretch();
                pLeftLayout->addLayout(pViewLayout);
            }
            QGroupBox  *pVorticityBox = new QGroupBox("Vorticity");
            {
                QGridLayout*pParamsLayout = new QGridLayout;
                {
                    m_pchContourPlane = new QCheckBox("Vorticity");
                    m_pchContourPlane->setChecked(bContourPlane);
                    m_pcbPlaneDir = new QComboBox;
                    m_pcbPlaneDir->addItems({"X", "Y", "Z"});
                     m_pslPlanePos = new QSlider(Qt::Horizontal);
                    m_pslPlanePos->setMinimumWidth(350);
                    m_pslPlanePos->setRange(0, 500);
                    m_pslPlanePos->setTickInterval(50);
                    m_pslPlanePos->setTickPosition(QSlider::TicksBelow);
                    m_pslPlanePos->setValue(0);

                    m_pcbPlaneDir->setEnabled(bContourPlane);
                    m_pslPlanePos->setEnabled(bContourPlane);

                    m_pcbOmegaDir = new QComboBox;
                    m_pcbOmegaDir->addItems({"X", "Y", "Z", "Norm"});
                    m_pcbOmegaDir->setCurrentIndex(3);

                    pParamsLayout->addWidget(m_pchContourPlane,            1, 1);
                    pParamsLayout->addWidget(new QLabel("Plane normal"),   2, 1);
                    pParamsLayout->addWidget(m_pcbPlaneDir,                2, 2);
                    pParamsLayout->addWidget(new QLabel("Omega_dir"),      3, 1);
                    pParamsLayout->addWidget(m_pcbOmegaDir,                3, 2);
                    pParamsLayout->addWidget(new QLabel("Plane Position"), 4, 1);
                    pParamsLayout->addWidget(m_pslPlanePos,                4, 2, 1, 2);
                    pParamsLayout->setColumnStretch(3,1);
                }
                pVorticityBox->setLayout(pParamsLayout);
            }

            pFrameLayout->addLayout(pLeftLayout);
            pFrameLayout->addWidget(pVorticityBox);
            m_pfrControls->setLayout(pFrameLayout);
        }
    }

    m_LegendOverlay.setTitle("Omega");
    m_LegendOverlay.setVisible(bContourPlane);

    connect(m_pdeVelScale,     SIGNAL(floatChanged(float)), SLOT(onVelScale()));
    connect(m_pdeRefLength,    SIGNAL(floatChanged(float)), SLOT(onRefLength()));
    connect(m_pchContourPlane, SIGNAL(toggled(bool))    , SLOT(onContourPlane(bool)));
    connect(m_pcbPlaneDir,     SIGNAL(activated(int)),    SLOT(onMakeColorMap()));
    connect(m_pslPlanePos,     SIGNAL(sliderMoved(int)),  SLOT(onMakeColorMap()));
    connect(m_pcbOmegaDir,     SIGNAL(activated(int)),    SLOT(onMakeColorMap()));
}


void gl3dVortonField::onRefLength()
{
    s_RefLength = m_pdeRefLength->value();
    setReferenceLength(s_RefLength);
    reset3dScale();
}


void gl3dVortonField::onVelScale()
{
    makePlotLineVelocities(s_iSource, m_nPts);
    m_bResetVectors = true;
    update();
}


bool gl3dVortonField::intersectTheObject(const Vector3d &AA, const Vector3d &BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    for(uint p=0; p<m_pPanels->size(); p++)
    {
        if(m_pPanels->at(p).intersect(AA, U, I)) return true;
    }
    return false;
}


void gl3dVortonField::glRenderView()
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }

    paintSegments(m_vboPlotLine, QColor(73,113,173), 5);

    paintVectors();

    //paint the colormap
    if(m_pchContourPlane->isChecked())
    {
        paintColourMap(m_vboClrMap);
        paintSegments(m_vboContours, QColor(125,125,125), 1);
    }

    paintTriangles3Vtx(m_vboTriangles, QColor(55, 177, 177, 95), true, true);
    paintSegments(m_vboTriangleEdges, W3dPrefs::s_PanelStyle);

    for(int i=0; i<m_PointArray.size(); i++)
        paintCube(m_PointArray.at(i).x, m_PointArray.at(i).y, m_PointArray.at(i).z, 0.005/double(m_glScalef), QColor(55,175,135), true);

    if(m_pVortons)
    {
        for(uint ip=0; ip<m_pVortons->size(); ip++)
        {
            Vorton const &vtn = m_pVortons->at(ip);
            Vector3d vortex = vtn.vortex();
            paintCube(vtn.position().x, vtn.position().y, vtn.position().z, 0.015/m_glScalef, QColor(155, 105, 35), true);
            paintThinArrow(vtn.position(), vtn.vortex()/50, QColor(125, 75, 35), 5, Line::SOLID);
        }
    }
}


void gl3dVortonField::makePlotLine()
{
    QVector<GLfloat>axisVertexArray(6);

    axisVertexArray[0] = m_PlotLine.vertexAt(0).xf();
    axisVertexArray[1] = m_PlotLine.vertexAt(0).yf();
    axisVertexArray[2] = m_PlotLine.vertexAt(0).zf();
    axisVertexArray[3] = m_PlotLine.vertexAt(1).xf();
    axisVertexArray[4] = m_PlotLine.vertexAt(1).yf();
    axisVertexArray[5] = m_PlotLine.vertexAt(1).zf();

    m_vboPlotLine.destroy();
    m_vboPlotLine.create();
    m_vboPlotLine.bind();
    m_vboPlotLine.allocate(axisVertexArray.constData(), axisVertexArray.size() * int(sizeof(GLfloat)));
    m_vboPlotLine.release();

    m_bResetPlotLine = true;
}


void gl3dVortonField::glMake3dObjects()
{
    if(m_bResetVectors)
    {
        glMakeVectors();
        gl::makeTriangle(m_Triangle, m_vboTriangle);
    }
    if(m_bResetPanels)
    {
        if(m_pPanels)
        {
            gl::makeTriPanels(*m_pPanels, Vector3d(), m_vboTriangles);
            gl::makeTriEdges(*m_pPanels, Vector3d(), m_vboTriangleEdges);
        }
    }

    if(m_bResetPlotLine)
        makePlotLine();

    if(m_bResetContour)
    {
        double lmin = -10;
        double lmax  = 10;

        gl::makeQuadColorMap(m_vboClrMap,   m_nRows, m_nCols, m_Nodes, m_Values, lmin, lmax, true, xfl::isMultiThreaded());
        gl::makeQuadContoursOnGrid( m_vboContours, m_nRows, m_nCols, m_Nodes, m_Values, xfl::isMultiThreaded());

        m_bResetContour = false;
    }

    m_bResetPanels   = false;
    m_bResetVectors  = false;
    m_bResetPlotLine = false;
}


void gl3dVortonField::glMakeVectors()
{
    // Size x Size points
    // x3 coordinates/vertex
    // x3 normal vector coordinates/vertex
    // x2 array is written a second time in x-direction with y main parameter
    int bufferSize =m_nPts*6;
    QVector<float> pVertexArray(bufferSize);

    if(m_PointArray.size() != m_nPts || m_VectorArray.size() != m_nPts)
    {
        m_vboVectors.destroy();
        return;
    }

    int iv = 0;
    int ia=0;
    for (int i=0; i<m_nPts; i++)
    {
        pVertexArray[iv++] = m_PointArray[ia].xf();
        pVertexArray[iv++] = m_PointArray[ia].yf();
        pVertexArray[iv++] = m_PointArray[ia].zf();
        pVertexArray[iv++] = m_PointArray[ia].xf() + m_VectorArray[ia].xf();
        pVertexArray[iv++] = m_PointArray[ia].yf() + m_VectorArray[ia].yf();
        pVertexArray[iv++] = m_PointArray[ia].zf() + m_VectorArray[ia].zf();
        ia++;
    }

    Q_ASSERT(iv==bufferSize);

    m_vboVectors.destroy();
    m_vboVectors.create();
    m_vboVectors.bind();
    m_vboVectors.allocate(pVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboVectors.release();
}


void gl3dVortonField::paintVectors()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    if(s_iSource==0) m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(113,173,255));
    else             m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(255,50,25));
    m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
    m_shadLine.setUniformValue(m_locLine.m_Thickness, 2.0f);

    m_vboVectors.bind();
    m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
    m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

    int nLines = m_vboVectors.size()/3/2/int(sizeof(float));
    int nVertices = 2*nLines;

    glDrawArrays(GL_LINES, 0, nVertices);

    m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    m_vboVectors.release();
    m_shadLine.release();
}


void gl3dVortonField::resizeGL(int w, int h)
{
    gl3dXflView::resizeGL(w, h);

    showEvent(nullptr);
}


void gl3dVortonField::showEvent(QShowEvent *pEvent)
{
    gl3dXflView::showEvent(pEvent);
    reset3dScale();

//    m_LegendOverlay.setLegendHeight((height()*2)/3);
    m_LegendOverlay.resize(100, (height()*2)/3, devicePixelRatioF());
    m_LegendOverlay.makeLegend();

    QPoint pos1(5,height()-m_pfrControls->height()-5);
    m_pfrControls->move(pos1);
}


/** compute the velocity vectors from the equivalent vortex ringh of a wake column */
void gl3dVortonField::makePlotLineVelocities(int iSource, int nPts)
{
    s_iSource = iSource; // keep track in case of a request for scale reset

    m_nPts = nPts;

    switch(iSource)
    {
        case 0:     m_plabTitle->setText("Panel induced velocity");     break;
        case 1:     m_plabTitle->setText("Vorton induced velocity");    break;
        default:    m_plabTitle->setText("Unknown velocity");           break;
    }

    s_VelScale = m_pdeVelScale->value();

    m_PointArray.resize( nPts);
    m_VectorArray.resize(nPts);

    Vector3d pt, vel, Vp;
    Vector3d V0[3], V1[3];

    for(int k=0; k<nPts; k++)
    {
        double dk = double(k)/double(nPts-1);
        m_PlotLine.getPoint(dk, pt);
        vel.reset();

        if(iSource==0)
        {
            for(uint i3=0; i3<m_pPanels->size(); i3++)
            {
                Panel3 const &p30 = m_pPanels->at(i3);
                p30.doubletBasisVelocity(pt, V0, false);
                vel += V0[0]*p30.mu(0) + V0[1]*p30.mu(1) + V0[2]*p30.mu(2);
            }
        }
        else if(iSource==1)
        {
            for(uint ip=0; ip<m_pVortons->size(); ip++)
            {
                m_pVortons->at(ip).inducedVelocity(pt, m_CoreSize, Vp);
                vel += Vp;
            }
        }

        m_PointArray[ k].set(pt);
        m_VectorArray[k].set(vel.x*s_VelScale, vel.y*s_VelScale, vel.z*s_VelScale); //plot in the x-y plane
    }

    m_bResetVectors = true;
    m_bResetPanels = true;
}


void gl3dVortonField::makeVorticityColorMap()
{
    int iPlaneDir = m_pcbPlaneDir->currentIndex()%3;
    double t = double(m_pslPlanePos->value())/500.0 * (m_xl+2) -1;

    m_Nodes.resize(m_nRows*m_nCols);
    m_Values.resize(m_nRows*m_nCols);

    if(xfl::isMultiThreaded())
    {
        QFutureSynchronizer<void> futureSync;
        for(int irow=0; irow<m_nRows; irow++)
        {
            double z = (double(irow)/double(m_nRows-1)-0.5)*m_yl*2;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &gl3dVortonField::makeVorticityColorRow,
                                                   z, t, irow, iPlaneDir));
#else
            futureSync.addFuture(QtConcurrent::run(&gl3dVortonField::makeVorticityColorRow, this,
                                                   z, t, irow, iPlaneDir));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int irow=0; irow<m_nRows; irow++)
        {
            double z = (double(irow)/double(m_nRows-1)-0.5)*m_yl*2;
            makeVorticityColorRow(z, t, irow, iPlaneDir);
        }
    }

    m_bResetContour = true;

    double vmin = 1e10;
    double vmax = -1e10;
    for(int iv=0; iv<m_Values.size(); iv++)
    {
        vmin = std::min(vmin, m_Values.at(iv));
        vmax = std::max(vmax, m_Values.at(iv));
    }
    m_LegendOverlay.setRange(vmin, vmax);
}


void gl3dVortonField::makeVorticityColorRow(double z, double t, int ir, int iPlaneDir)
{
    if(!m_pVortons) return;
    Vector3d omega, omp;
    for(int ic=0; ic<m_nCols; ic++)
    {
        double y = (double(ic)/double(m_nCols-1)-0.5)*m_yl*2;

        int idx = ir*m_nCols + ic;
        switch(iPlaneDir)
        {
            default:
            case 0:  m_Nodes[idx].set(t, y, z);      break;
            case 1:  m_Nodes[idx].set(y, t, z);      break;
            case 2:  m_Nodes[idx].set(y, z, t);      break;
        }

        Vector3d const &pt = m_Nodes.at(idx);

        // from the vortons
        omega.set(0,0,0);
        for(uint ip=0; ip<m_pVortons->size(); ip++)
        {
            omp = m_pVortons->at(ip).vorticity(pt);
            omega += omp;
        }

        switch (m_pcbOmegaDir->currentIndex())
        {
            case 0: m_Values[idx] = omega.x;       break;
            case 1: m_Values[idx] = omega.y;       break;
            case 2: m_Values[idx] = omega.z;       break;
            default:
            case 3: m_Values[idx] = omega.norm();  break;
        }
    }
}


void gl3dVortonField::onContourPlane(bool bChecked)
{
    m_pcbPlaneDir->setEnabled(bChecked);
    m_pslPlanePos->setEnabled(bChecked);
    m_LegendOverlay.setVisible(bChecked);
    update();
}


void gl3dVortonField::onMakeColorMap()
{
    makeVorticityColorMap();

    m_LegendOverlay.makeLegend();
    update();
}


void gl3dVortonField::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dVectorField");
    {
        s_RefLength = settings.value("RefLength", s_RefLength).toDouble();
        s_VelScale  = settings.value("VelScale",  s_VelScale).toDouble();
    }
    settings.endGroup();
}


void gl3dVortonField::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dVectorField");
    {
        settings.setValue("RefLength", s_RefLength);
        settings.setValue("VelScale",  s_VelScale);
    }
    settings.endGroup();
}



