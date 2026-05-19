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

#include <QKeyEvent>

#include "gl3dtestpoints.h"
#include <api/utils.h>

#include <interfaces/controls/w3dprefs.h>
#include <core/trace.h>

gl3dTestPoints::gl3dTestPoints(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setReferenceLength(3);
    reset3dScale();

    m_bInitialized = false;
    m_bResetPoints = true;
    m_iRenderer = 0;
    m_iShape = 2;
}


void gl3dTestPoints::glMake3dObjects()
{
    if(m_bResetPoints)
    {
        float radius(1.0f);
        int stride = 8;

        int nPoints = 11;

        int buffersize = nPoints*stride;
        QVector<float> pts(buffersize);
        int iv =0;
        for(int i=0; i<nPoints; i++)
        {
            float theta = float(i)/float(nPoints);
            pts[iv++] = radius * cos(theta * 2.0 * PI);
            pts[iv++] = radius * sin(theta * 2.0 * PI);
            pts[iv++] = 0.5f * cos(theta*2.0*PI);
            pts[iv++] = theta;

            pts[iv++] = xfl::getRed(  theta); // for the fragment shader
            pts[iv++] = xfl::getGreen(theta);
            pts[iv++] = xfl::getBlue( theta);
            pts[iv++] = 1.0f; //alpha
        }

        Q_ASSERT(iv==buffersize);

        if(m_vboPoints.isCreated()) m_vboPoints.destroy();
        m_vboPoints.create();
        m_vboPoints.bind();
        m_vboPoints.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
        m_vboPoints.release();

        m_bResetPoints = false;
    }
}


void gl3dTestPoints::glRenderView()
{
    QMatrix4x4 modelmat;
//    modelmat.scale(0.25f, 0.25f, 0.25f);
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    paintIcoSphere(Vector3d(), 0.25, QColor(205, 155, 133), true, true);


    int PointSize = 7;

    if(m_iRenderer==0)
    {

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#else
        m_shadPoint.bind();
        {
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix,  vmMat);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
        }
        m_shadPoint.release();
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

        glDisable(GL_CULL_FACE);
        paintPoints(m_vboPoints, float(PointSize)/1.0f, m_iShape, true, Qt::black, 8);
#endif
    }
    else if(m_iRenderer==1)
    {
        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();
        paintPointInstances(m_vboPoints, float(PointSize)/1000.f/m_glScalef, Qt::cyan, false, true);
    }
    else
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

        m_shadPoint2.bind();
        {
            int stride = 8;
            m_shadPoint2.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
            m_shadPoint2.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);
            m_shadPoint2.setUniformValue(m_locPt2.m_Shape, float(PointSize)*5.0f);
            m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, m_ClipPlanePos);

            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

            m_vboPoints.bind();
            {
                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

                int nPoints = m_vboPoints.size()/stride/int(sizeof(float));

                glEnable (GL_POINT_SPRITE);
                glEnable(GL_PROGRAM_POINT_SIZE); // To set the point size from a shader, enable the glEnable with argument (GL_PROGRAM_POINT_SIZE)
                glDrawArrays(GL_POINTS, 0, nPoints);
            }
            m_vboPoints.release();

            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
        }
        m_shadPoint2.release();
    }

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dTestPoints::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    bool bAlt  = (pEvent->modifiers() & Qt::AltModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(windowFlags()&Qt::FramelessWindowHint)
            {
                setWindowFlags(Qt::Window);
                show(); //Note: This function calls setParent() when changing the flags for a window, causing the widget to be hidden. You must call show() to make the widget visible again..
//                update();
                return;
            }
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                close();
                return;
            }
            break;
        }
        case Qt::Key_0:
        {
            if(bCtrl)
            {
                m_iRenderer = 0;
                update();
            }
            else if(bAlt)
            {
                m_iShape = 0;
                update();
            }
            break;
        }
        case Qt::Key_1:
        {
            if(bCtrl)
            {
                m_iRenderer = 1;
                update();
            }
            else if(bAlt)
            {
                m_iShape = 1;
                update();
            }
            break;
        }
        case Qt::Key_2:
        {
            if(bCtrl)
            {
                m_iRenderer = 2;
                update();
            }
            else if(bAlt)
            {
                m_iShape = 2;
                update();
            }
            break;
        }
    }

    gl3dView::keyPressEvent(pEvent);
}


void gl3dTestPoints::showEvent(QShowEvent *pEvent)
{
    reset3dScale();
    setFocus();
    gl3dView::showEvent(pEvent);
}
