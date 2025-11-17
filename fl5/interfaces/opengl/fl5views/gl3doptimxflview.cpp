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

#define _MATH_DEFINES_DEFINED



#include <QOpenGLPaintDevice>
#include <QKeyEvent>
#include <QPainter>
#include <QQuaternion>

#include "gl3doptimxflview.h"
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <api/planeopp.h>
#include <api/planexfl.h>
#include <api/trimesh.h>


int gl3dOptimXflView::s_iColorMap = 0;

gl3dOptimXflView::gl3dOptimXflView(QWidget *pParent) : gl3dPlaneXflView(pParent)
{
    m_pPlaneXfl = nullptr;
    m_pPOpp     = nullptr;

    m_bResetCp     = true;
    m_bResetArrows = true;

    m_bSurfaces     = false;
    m_bMeshPanels   = true;
    m_bShowSections = true;
    m_bShowCp       = false;
    m_bShowContours = false;
}


gl3dOptimXflView::~gl3dOptimXflView()
{
    if(m_vboContours.isCreated())      m_vboContours.destroy();
    if(m_vboCp.isCreated())            m_vboCp.destroy();
    if(m_vboSectionArrows.isCreated()) m_vboSectionArrows.destroy();
}


void gl3dOptimXflView::resizeGL(int w, int h)
{
    gl3dXflView::resizeGL(w, h);

//    m_LegendOverlay.setLegendHeight((height()*2)/3);
    m_ColourLegend.resize(100, (height()*2)/3, devicePixelRatioF());
    m_ColourLegend.makeLegend();
}


void gl3dOptimXflView::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = pEvent->modifiers() & Qt::ControlModifier;

    switch (pEvent->key())
    {
        case Qt::Key_G:
        {
            if(bCtrl) onGamma();
            break;
        }
        case Qt::Key_S:
        {
            if(bCtrl) onSigma();
            break;
        }
        [[fallthrough]]; default: // shut clang up
            gl3dView::keyPressEvent(pEvent);
            break;
    }
}


void gl3dOptimXflView::onCp(bool b)
{
    s_iColorMap = 0;
    m_bResetCp = true;
    m_bShowCp=b;
    m_ColourLegend.setVisible(m_bShowCp);
    update();
}


void gl3dOptimXflView::onGamma()
{
    s_iColorMap = 1;
    m_bResetCp = true;
    m_bShowCp = true;
    m_ColourLegend.setVisible(m_bShowCp);
    update();
}


void gl3dOptimXflView::onSigma()
{
    s_iColorMap = 2;
    m_bResetCp = true;
    m_bShowCp = true;
    m_ColourLegend.setVisible(m_bShowCp);
    update();
}


void gl3dOptimXflView::onContours(bool b)
{
    m_bShowContours=b;
    update();
}


void gl3dOptimXflView::onSections(bool b)
{
    m_bShowSections=b;
    update();
}


void gl3dOptimXflView::setSectionEndPoint(QVector<Node> &endpts, QVector<QColor> const&colors)
{
    m_SectionColors = colors;
    m_SectionEndPoint = endpts;
}


void gl3dOptimXflView::setSectionCp(std::vector<Node> &SectionPts, std::vector<double> &sectioncp, QVector<QColor> const&colors)
{
    gl::makeCpSection(SectionPts, sectioncp, 0.3, m_CpNodes);

    m_NodeColors.resize(m_CpNodes.size());

    if(colors.size()>0)
    {
        int nsecpts = int(m_CpNodes.size()/colors.size());

        for(uint i=0; i<m_CpNodes.size(); i++)
        {
            int icolor = i/nsecpts;
            m_NodeColors[i] = colors.at(icolor);
        }
    }

    m_bResetArrows = true;
}


void gl3dOptimXflView::glMake3dObjects()
{
    gl3dPlaneXflView::glMake3dObjects();

    if(m_bResetArrows)
    {
        glMakeNodeArrows();
        m_bResetArrows = false;
    }

    if(m_bResetCp && m_pPOpp)
    {
        std::vector<double> val(m_pPOpp->nPanel3()*3);
        std::vector<double> m_NodeValue(m_pPOpp->m_NodeValue.size());
        double lmin=0, lmax=0;
        switch(s_iColorMap)
        {
            default:
            case 0:
                val = m_pPOpp->Cp();
                m_ColourLegend.setTitle("Cp");
                if(Opp3dScalesCtrls::isAutoCpScale())
                {
                    Opp3dScalesCtrls::setCpRange(lmin,lmax);
                }
                else
                {
                    lmin = Opp3dScalesCtrls::CpMin();
                    lmax = Opp3dScalesCtrls::CpMax();
                }
                break;
            case 1:
                for(uint i=0; i<m_pPOpp->gamma().size(); i++) val[i] = m_pPOpp->gamma(i)*1000.0;
                m_ColourLegend.setTitle("gamma*1000");
                break;
            case 2:
                for(uint i=0; i<m_pPOpp->sigma().size(); i++) val[3*i] = val[3*i+1] =val[3*i+2] = m_pPOpp->sigma(i)*1000.0;
                m_ColourLegend.setTitle("sigma*1000");
                break;
        }

        TriMesh::makeNodeValues(m_pPlaneXfl->triMesh().nodes(), m_pPlaneXfl->triMesh().panels(),
                                val, m_NodeValue, lmin, lmax, 1.0);

        gl::makeTriLinColorMap(m_pPlaneXfl->triMesh().panels(), m_pPlaneXfl->triMesh().nodes(), m_NodeValue, lmin, lmax, m_vboCp);
        gl::makeTriangleContoursOnMesh(m_pPlaneXfl->triMesh().panels(), m_NodeValue, lmin, lmax, m_vboContours);

        m_ColourLegend.setRange(lmin, lmax);
        m_ColourLegend.makeLegend();
        m_bResetCp = false;
    }
}


void gl3dOptimXflView::glRenderView()
{
    gl3dPlaneXflView::glRenderView();

    if(m_bShowSections)
    {
        paintColorSegments(m_vboSectionArrows,3);
        paintSections(m_pPlaneXfl->mainWing());
    }

    if(m_pPOpp)
    {
        if(m_bShowCp)       paintColourMap(m_vboCp);
        if(m_bShowContours) paintSegments(m_vboContours, W3dPrefs::s_ContourLineStyle);
    }
}


void gl3dOptimXflView::paintMeshPanels()
{
    if(m_bMeshPanels)
    {
        for(int iw=0; iw<m_pPlaneXfl->nWings(); iw++)
        {
            WingXfl const *pWing = m_pPlaneXfl->wingAt(iw);
            if(pWing->isVisible() && iw<m_vboTriMesh.size())
            {
                if(!m_bSurfaces && !m_bShowCp) paintTriPanels(m_vboTriMesh[iw], false);
                paintSegments(m_vboTriEdges[iw], W3dPrefs::s_PanelStyle);
            }
        }
        for(int ifuse=0; ifuse<m_pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = m_pPlaneXfl->fuseAt(ifuse);
            int ivbo = m_pPlaneXfl->nWings()+ifuse;
            if(pFuse->isVisible() && ivbo<m_vboTriMesh.size())
            {
                if(!m_bSurfaces && !m_bShowCp) paintTriPanels(m_vboTriMesh[ivbo], false);
                paintSegments(m_vboTriEdges[ivbo], W3dPrefs::s_PanelStyle);
            }
        }

        if(m_Segments.size())
            paintSegments(m_vboSegments, W3dPrefs::highlightColor(), 1, Line::SOLID, true);
    }
}


void gl3dOptimXflView::paintSections(WingXfl const *)
{
    for(int isec=0; isec<m_SectionEndPoint.size(); isec++)
    {
        Node const &nd = m_SectionEndPoint.at(isec);
        glRenderText(nd.x+0.03/double(m_glScalef), nd.y, nd.z,
                     QString::asprintf("Section_%d", isec),
                     m_SectionColors.at(isec));
    }
}


void gl3dOptimXflView::glMakeNodeArrows()
{
    int nNodes = int(m_CpNodes.size());
    if(nNodes==0)
    {
        if(m_vboSectionArrows.isCreated()) m_vboSectionArrows.destroy();
        return;
    }
    Quaternion Qt; // Quaternion operator to align the reference arrow to the panel's normal
    Vector3d Omega; // rotation vector to align the reference arrow with the panel's normal
    Vector3d O;
    //The vectors defining the reference arrow
    QVector3D R(  0.0f,  0.0f,  1.00f);
    QVector3D R1( 0.05f, 0.05f, -0.15f);
    QVector3D R2(-0.05f, -0.05f, -0.15f);
    //The three vectors defining the arrow on the panel
    QVector3D P, P1, P2;

    QVector<float> ArrowVertexArray(m_CpNodes.size()*3*12);
    QVector3D N(0,0,1);// this is the vector used to define m_vboArrow

    float r=0, g=0, b=0;
    int iv = 0;
    for(uint ipt=0; ipt<m_CpNodes.size(); ipt++)
    {
        Vector3d const &arrow = m_CpNodes.at(ipt).normal();
        QVector3D A(arrow.xf(), arrow.yf(), arrow.zf());
        A.normalize();
        QQuaternion qqt = QQuaternion::rotationTo(N, A);

        r = m_NodeColors.at(ipt).redF();
        g = m_NodeColors.at(ipt).greenF();
        b = m_NodeColors.at(ipt).blueF();

        O.set(m_CpNodes.at(ipt));

        P  = qqt.rotatedVector(R);
        P1 = qqt.rotatedVector(R1);
        P2 = qqt.rotatedVector(R2);

        // Scale the pressure vector
        P  *= arrow.normf();
        P1 *= arrow.normf();
        P2 *= arrow.normf();

        ArrowVertexArray[iv++] = O.xf();
        ArrowVertexArray[iv++] = O.yf();
        ArrowVertexArray[iv++] = O.zf();
        ArrowVertexArray[iv++] = r;
        ArrowVertexArray[iv++] = g;
        ArrowVertexArray[iv++] = b;
        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = r;
        ArrowVertexArray[iv++] = g;
        ArrowVertexArray[iv++] = b;

        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = r;
        ArrowVertexArray[iv++] = g;
        ArrowVertexArray[iv++] = b;
        ArrowVertexArray[iv++] = O.xf()+P.x()+P1.x();
        ArrowVertexArray[iv++] = O.yf()+P.y()+P1.y();
        ArrowVertexArray[iv++] = O.zf()+P.z()+P1.z();
        ArrowVertexArray[iv++] = r;
        ArrowVertexArray[iv++] = g;
        ArrowVertexArray[iv++] = b;

        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = r;
        ArrowVertexArray[iv++] = g;
        ArrowVertexArray[iv++] = b;
        ArrowVertexArray[iv++] = O.xf()+P.x()+P2.x();
        ArrowVertexArray[iv++] = O.yf()+P.y()+P2.y();
        ArrowVertexArray[iv++] = O.zf()+P.z()+P2.z();
        ArrowVertexArray[iv++] = r;
        ArrowVertexArray[iv++] = g;
        ArrowVertexArray[iv++] = b;
    }
    Q_ASSERT(iv==ArrowVertexArray.size());

    m_vboSectionArrows.destroy();
    m_vboSectionArrows.create();
    m_vboSectionArrows.bind();
    m_vboSectionArrows.allocate(ArrowVertexArray.data(), ArrowVertexArray.size() * int(sizeof(GLfloat)));
    m_vboSectionArrows.release();
}


void gl3dOptimXflView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);


    glDisable(GL_CULL_FACE);

    if(m_ColourLegend.isVisible())
    {
        int wc = m_ColourLegend.m_pix.width();
        QPoint pos3(width()-5-wc, 50);
        painter.drawPixmap(pos3, m_ColourLegend.m_pix);
    }

    gl3dView::paintOverlay();
}


