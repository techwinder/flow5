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




#include <QFormLayout>

#include "gl3dpanelfield.h"

#include <interfaces/opengl/globals/gl_xfl.h>

#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <api/panel2d.h>
#include <api/panel3.h>
#include <api/panel4.h>
#include <api/testpanels.h>
#include <api/vortex.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

Quaternion gl3dPanelField::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);
int gl3dPanelField::s_nPoints = 50;
double gl3dPanelField::s_VelScale = 0.03;
double gl3dPanelField::s_RefLength= 1.0;

gl3dPanelField::gl3dPanelField(QWidget *pParent) : gl3dXflView(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_bSource = true;
    m_Method = NOPANELMETHOD;
    m_bResetView = true;
    m_CoreRadius = Vortex::coreRadius();

    makeControls();

    makeVectorPlotLine();
}

void gl3dPanelField::makeControls()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());

    m_pFrame = new QFrame(this);
    {
        m_pFrame->setCursor(Qt::ArrowCursor);
        m_pFrame->setFrameShape(QFrame::NoFrame);
        m_pFrame->setPalette(palette);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            m_pLabInfo = new QLabel;
            m_pLabInfo->setStyleSheet("QLabel { font-weight: bold;}");
            QFormLayout*pParamsLayout = new QFormLayout;
            {

                m_pieNw = new IntEdit(s_nPoints);
                m_pdeVelScale  = new FloatEdit(s_VelScale);
                m_pdeRefLength = new FloatEdit(s_RefLength);

                pParamsLayout->addRow("Npoints",      m_pieNw);
                pParamsLayout->addRow("Vector scale", m_pdeVelScale);
                pParamsLayout->addRow("RefLength",    m_pdeRefLength);
            }

            pFrameLayout->addWidget(m_pLabInfo);
            pFrameLayout->addLayout(pParamsLayout);

        }
        m_pFrame->setLayout(pFrameLayout);
    }

    connect(m_pieNw,           SIGNAL(intChanged(int)),       SLOT(onMakeView()));
    connect(m_pdeVelScale,     SIGNAL(floatChanged(float)),       SLOT(onMakeView()));
    connect(m_pdeRefLength,    SIGNAL(floatChanged(float)),       SLOT(onRefLength()));
}


void gl3dPanelField::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dPanel3Field");
    {
        s_nPoints      = settings.value("NPoints",     s_nPoints).toInt();
        s_RefLength    = settings.value("RefLength",   s_RefLength).toDouble();
        s_VelScale     = settings.value("VelScale",    s_VelScale).toDouble();
    }
    settings.endGroup();
}


void gl3dPanelField::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dPanel3Field");
    {
        settings.setValue("NPoints",     s_nPoints);
        settings.setValue("RefLength",   s_RefLength);
        settings.setValue("VelScale",    s_VelScale);
    }
    settings.endGroup();
}


void gl3dPanelField::onRefLength()
{
    readData();
    setReferenceLength(s_RefLength);
    reset3dScale();
}


void gl3dPanelField::onMakeView()
{
    readData();
    switch(m_Method)
    {
        case BASIS:    m_pLabInfo->setText("Velocities from basis");   break;
        case NASA4023: m_pLabInfo->setText("Velocities from N4023");   break;
        case VORTEX:   m_pLabInfo->setText("Velocities from vortex");  break;
        default: m_pLabInfo->clear(); break;
    }

    makeVectorPlotLine();
    m_bResetView = true;
    update();
}


void gl3dPanelField::readData()
{
    s_nPoints = std::max(m_pieNw->value(),2);
    s_RefLength = m_pdeRefLength->value();
    s_VelScale = m_pdeVelScale->value();
}


void gl3dPanelField::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_W:
            close();
            break;
        default:
            gl3dXflView::keyPressEvent(pEvent);
            break;
    }
}


void gl3dPanelField::glRenderView()
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
    paintSegments(m_vboPlotLine, QColor(75, 105, 205), 5);

    paintVectors();

    paintTriPanels(m_vboPanels, true);
    paintSegments(m_vboEdges, W3dPrefs::s_PanelStyle);

    for(int i=0; i<m_PointArray.size(); i++)
        paintIcosahedron(m_PointArray.at(i), 0.005/double(m_glScalef), Qt::darkGreen, W3dPrefs::s_OutlineStyle, true, true);
}


bool gl3dPanelField::intersectTheObject(Vector3d const&AA, Vector3d const&BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    for(uint i3=0; i3<m_Panel3.size(); i3++)
    {
        Panel3 const &p3 = m_Panel3.at(i3);
        if(p3.intersect(AA, U, I)) return true;
    }
    return false;
}


void gl3dPanelField::glMake3dObjects()
{
    if(m_bResetView)
    {
        glMakePlotLine();
        glMakeVectors();
        if(m_Panel3.size())
        {
            gl::makeTriPanels(m_Panel3, Vector3d(), m_vboPanels);
            gl::makeTriEdges(m_Panel3, Vector3d(), m_vboEdges);
        }
        else
        {
            gl::makeQuadPanels(m_Panel4, Vector3d(), m_vboPanels);
            gl::makeQuadEdges(m_Panel4, Vector3d(), m_vboEdges);

        }
    }
    m_bResetView = false;
}


void gl3dPanelField::glMakeVectors()
{
    // Size x Size points
    // x3 coordiantes/vertex
    // x3 normal vector coordinates/vertex
    // x2 array is written a second time in x-direction with y main parameter
    int bufferSize = m_PointArray.size()*6;
    QVector<float> pVertexArray(bufferSize);

    int iv = 0;
    int ia=0;
    for (int i=0; i<m_PointArray.size(); i++)
    {
        pVertexArray[iv++] = m_PointArray.at(ia).xf();
        pVertexArray[iv++] = m_PointArray.at(ia).yf();
        pVertexArray[iv++] = m_PointArray.at(ia).zf();
        pVertexArray[iv++] = m_PointArray.at(ia).xf() + m_VectorArray.at(ia).xf()*s_VelScale;
        pVertexArray[iv++] = m_PointArray.at(ia).yf() + m_VectorArray.at(ia).yf()*s_VelScale;
        pVertexArray[iv++] = m_PointArray.at(ia).zf() + m_VectorArray.at(ia).zf()*s_VelScale;
        ia++;
    }

    Q_ASSERT(iv==bufferSize);

    m_vboVectors.destroy();
    m_vboVectors.create();
    m_vboVectors.bind();
    m_vboVectors.allocate(pVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboVectors.release();
}


void gl3dPanelField::glMakePlotLine()
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
}


void gl3dPanelField::paintVectors()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(0,137,51));
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, 2.0f);

        m_vboVectors.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            int nLines = m_vboVectors.size()/3/2/int(sizeof(float));
            int nVertices = 2*nLines;

            glDrawArrays(GL_LINES, 0, nVertices);

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        m_vboVectors.release();
    }
    m_shadLine.release();
}


void gl3dPanelField::resizeGL(int width, int height)
{
    QPoint pos1(5,height-m_pFrame->height()-5);
    m_pFrame->move(pos1);
    gl3dXflView::resizeGL(width, height);
}


void gl3dPanelField::showEvent(QShowEvent *pEvent)
{
    restoreViewPoint(s_ab_quat);
    setReferenceLength(s_RefLength);
    reset3dScale();
    pEvent->ignore();
}


void gl3dPanelField::hideEvent(QHideEvent *)
{
    saveViewPoint(s_ab_quat);
}


/** compute the velocity vectors from the equivalent vortex ringh of a wake column */
void gl3dPanelField::makeVectorPlotLine()
{
    m_PointArray.resize( s_nPoints);
    m_VectorArray.resize(s_nPoints);
    Vector3d pt, vel;

    for(int i=0; i<s_nPoints; i++)
    {
        pt.set(m_PlotLine.vertexAt(0) + m_PlotLine.segment() * double(i)/double(s_nPoints-1));
        if(m_Panel3.size())
            vel = velocityP3(pt, m_Panel3, m_Method, m_bSource, m_CoreRadius, m_VortexModel);
        else
            vel = velocityP4(pt, m_Panel4, m_Method, m_bSource, m_CoreRadius);
        vel *= 4.0*PI;

        m_PointArray[i].set(pt);
        m_VectorArray[i].set(vel.x, vel.y, vel.z); //plot in the x-y plane
    }

    double w = 0;
    for(uint i3=0; i3<m_Panel3.size(); i3++)
    {
        w = std::max(w,m_Panel3.at(i3).width());
    }

//    setReferenceLength(std::max(m_PlotLine.length()*2.0, w*2.0));

    m_bResetView = true;
}


