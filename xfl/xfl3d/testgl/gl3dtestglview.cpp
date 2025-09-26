/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QKeyEvent>

#include "gl3dtestglview.h"


#include <xfl/controls/w3dprefs.h>
#include <xflcore/trace.h>

gl3dTestGLView::gl3dTestGLView(QWidget *pParent) : gl3dView (pParent)
{
    m_bInitialized = false;
    setReferenceLength(3);
    reset3dScale();
}


void gl3dTestGLView::glRenderView()
{
    float dist = 1.0f;
    float satrad = 0.15f;

    QMatrix4x4 modelmat;
//    modelmat.scale(0.25f, 0.25f, 0.25f);
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

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

    paintIcosahedron({dist*cos(1*2*PI/3+3.0*PI/7.0), dist*sin(1*2*PI/3+3.0*PI/7.0), 0.0f}, satrad,
                     Qt::darkCyan,   W3dPrefs::s_OutlineStyle, true, true);
    paintIcosahedron({dist*cos(2*2*PI/3+3.0*PI/7.0), dist*sin(2*2*PI/3+3.0*PI/7.0), 0.0f}, satrad,
                     Qt::darkYellow, W3dPrefs::s_OutlineStyle, true, true);
    paintIcosahedron({dist*cos(3*2*PI/3+3.0*PI/7.0), dist*sin(3*2*PI/3+3.0*PI/7.0), 0.0f}, satrad,
                     Qt::darkGray,   W3dPrefs::s_OutlineStyle, true, true);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dTestGLView::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
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
        }
    }

    gl3dView::keyPressEvent(pEvent);
}


void gl3dTestGLView::showEvent(QShowEvent *pEvent)
{
    reset3dScale();
    setFocus();
    gl3dView::showEvent(pEvent);
}
