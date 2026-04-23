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


#include <QMouseEvent>
#include <QPainter>

#include <core/xflcore.h>
#include <api/geom_global.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/fl5views/gl3dwingview.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/globals/gl_xfl.h>
#include <api/surface.h>
#include <api/wingxfl.h>
#include <api/panel3.h>
#include <api/units.h>


gl3dWingView::gl3dWingView(QWidget *pParent) : gl3dXflView(pParent)
{
    m_pWing = nullptr;
    m_iSection = -1;
    m_bRightSide               = true;
    m_bHighlightSection        = true;
    m_bResetglSectionHighlight = true;
    m_bResetglWing             = true;

    m_nHighlightLines = m_HighlightLineSize=0;
}


gl3dWingView::~gl3dWingView()
{
    if(m_vboSurface.isCreated())          m_vboSurface.destroy();
    if(m_vboOutline.isCreated())          m_vboOutline.destroy();
    if(m_vboTessellation.isCreated())     m_vboTessellation.destroy();
    if(m_vboTessNormals.isCreated())      m_vboTessNormals.destroy();
    if(m_vboSectionHighlight.isCreated()) m_vboSectionHighlight.destroy();
    if(m_vboTessNormals.isCreated())      m_vboTessNormals.destroy();
}


std::vector<Node> const &gl3dWingView::nodes() const {return m_pWing->nodes();}


void gl3dWingView::glRenderView()
{
    if(!m_pWing) return;

    WingXfl *pWingXfl = dynamic_cast<WingXfl*>(m_pWing);

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();

    if(m_bSurfaces)   paintTriangles3Vtx(m_vboSurface, xfl::fromfl5Clr(m_pWing->color()), false, isLightOn());
    if(m_bOutline)    paintSegments(m_vboOutline, W3dPrefs::s_OutlineStyle);

    if(pWingXfl)
    {
        if(m_bFoilNames && pWingXfl)  paintFoilNames(pWingXfl);
    }

    if(m_bTessellation)
    {
        paintSegments(m_vboTessellation, W3dPrefs::s_OutlineStyle);
        paintSegments(m_vboTessNormals, Qt::yellow, 1);
    }

    if(m_bNormals)
    {
        paintNormals(m_vboNormals);
    }

    if(m_bMeshPanels && !m_bSurfaces)
    {
        paintTriPanels(m_vboTriMesh, false);
        paintSegments(m_vboTriEdges, W3dPrefs::s_PanelStyle);
    }
    if(m_bShowMasses)
    {
        paintPartMasses(Vector3d(0.0,0.0,0.0), 0.0, "Structural mass", m_pWing->pointMasses(), m_iSelectedPartMass);

        //plot CG
        paintSphere(m_pWing->CoG_t(), W3dPrefs::s_MassRadius*2.0/double(m_glScalef), W3dPrefs::s_MassColor.darker());

        double delta = 0.02/double(m_glScalef);
        glRenderText(m_pWing->CoG_t().x, m_pWing->CoG_t().y, m_pWing->CoG_t().z + delta,
                     "CoG "+QString("%1").arg(m_pWing->totalMass()*Units::kgtoUnit(), 0, 'f', 2)
                     +Units::massUnitQLabel(), W3dPrefs::s_MassColor.darker(), false, true);
    }
    if(m_iSection>=0 && m_bHighlightSection)
    {
        paintSegments(m_vboSectionHighlight, W3dPrefs::s_HighStyle, false);
//        paintWingSectionHighlight();
    }

    if(m_bHighlightPanel && isPicking() && m_PickedPanelIndex>=0 && m_bMeshPanels)
        highlightPickedPanel3(m_pWing->triMesh().panelAt(m_PickedPanelIndex));

    if(bPickNode() && m_bMeshPanels)
    {
        if(m_NodePair.first>=0 && m_NodePair.first<int(m_Nodes.size()))
        {
            Node const &nd = m_Nodes.at(m_NodePair.first);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::green, true);
        }
        if(m_NodePair.second>=0 && m_NodePair.second<int(m_Nodes.size()))
        {
            //unnecessary, pair is cleared immediately
            Node const &nd = m_Nodes.at(m_NodePair.second);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::cyan, true);
        }

        if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<int(m_Nodes.size()))
        {
            Node const &nd = m_Nodes.at(m_PickedNodeIndex);
            paintSphere(nd, s_NodeRadius/double(m_glScalef), Qt::red, true);
        }
    }
    paintMeasure();
}


void gl3dWingView::glMake3dObjects()
{
    if(!m_pWing) return;
    WingXfl *pWingXfl = dynamic_cast<WingXfl*>(m_pWing);

    if(m_bResetglSectionHighlight || m_bResetglWing)
    {
        if(pWingXfl && m_iSection>=0)
        {
            glMakeWingSectionHighlight_seg(pWingXfl, m_iSection);
        }
        m_bResetglSectionHighlight = false;
    }

    if(m_bResetglWing)
    {
        m_bResetglWing = false;

        if(pWingXfl)
        {
            pWingXfl->makeTriangulation(nullptr, W3dPrefs::s_iChordwiseRes);
            gl::makeTriangles3Vtx(m_pWing->triangulation().triangles(), false, m_vboSurface);
            gl::makeTrianglesOutline(m_pWing->triangulation().triangles(), Vector3d(), m_vboTessellation);
            gl::makeSegments(pWingXfl->outline(), Vector3d(), m_vboOutline);
        }

        gl::makeTriangleNodeNormals(m_pWing->triangulation().triangles(), 0.05f, m_vboTessNormals);
//        m_pglStdBuffers->glMakeTriangleNormals(triangles, 0.1f, m_vboTessNormals);

        // wing triangular panels are not stored in the wing object, need to rebuild them on the fly
        // and store them in this class to pick them

        if(pWingXfl)
        {
            for (int j=0; j<pWingXfl->nSurfaces(); j++)
            {
                pWingXfl->surface(j).makeSideNodes(nullptr);
            }
        }

        m_pWing->makeTriPanels(0, 0, true);
        gl::makeTriPanels(pWingXfl->panels(), Vector3d(), m_vboTriMesh);
        gl::makeTriEdges(pWingXfl->panels(), Vector3d(), m_vboTriEdges);
        gl::makePanelNormals(pWingXfl->panels(), 0.1f, m_vboNormals);
    }
}


void gl3dWingView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(!m_bHasMouseMoved && m_bP3Select)
    {
        Vector3d I;
        if(pickPanel3(pEvent->pos(), m_pWing->panels(), I))
        {
            emit panelSelected(m_PickedPanelIndex);
        }
    }
    else
    {
        int draggeddistance = (m_PressedPoint-pEvent->pos()).manhattanLength();

        if(draggeddistance<5 && (pEvent->button()==Qt::LeftButton) && bPickNode() && m_PickedNodeIndex>=0)
        {
            if(m_PickedNodeIndex<0)
            {
                // clear and start again node pair picking
                m_NodePair = {-1,-1};
                return;
            }

            if(m_NodePair.first<0) m_NodePair.first = m_PickedNodeIndex;
            else if(m_NodePair.second<0)
            {
                m_NodePair.second = m_PickedNodeIndex;
                //two valid node indexes, merge them
                emit pickedNodePair(m_NodePair);
                m_NodePair = {-1,-1};
            }
            else
            {
                //start again
                m_NodePair = {m_PickedNodeIndex, -1};
            }
        }
    }


    gl3dXflView::mouseReleaseEvent(pEvent);

    update();
}


void gl3dWingView::mouseMoveEvent(QMouseEvent *pEvent)
{
    int lastpickedindex = m_PickedPanelIndex;
    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
//        m_PickedIndex = -1;
//        clearOutputInfo();
        gl3dXflView::mouseMoveEvent(pEvent);
        return;
    }

    if((!m_bHighlightPanel && !bPickNode())  || !m_bMeshPanels)
    {
        clearTopRightOutput();
        m_PickedPanelIndex = -1;
        return;
    }

    Vector3d I;
    pickPanel3(pEvent->pos(), m_pWing->panels(), I);

    if(m_PickedPanelIndex<0 || m_PickedPanelIndex>int(m_pWing->nPanel3()))
    {
        m_PickedPanelIndex = -1;
        m_PickedNodeIndex = -1;
        clearTopRightOutput();
        update();
    }
    else if(m_PickedPanelIndex!=lastpickedindex)
    {
        if(m_bHighlightPanel)
        {
            if(m_PickedPanelIndex>=0 && m_PickedPanelIndex<int(m_pWing->nPanel3()))
            {
                Panel3 const &p3 = m_pWing->panel3At(m_PickedPanelIndex);
                gl::makeTriangle(p3.vertexAt(0), p3.vertexAt(1), p3.vertexAt(2), m_vboTriangle);

                setTopRightOutput(p3.properties(false));
            }
        }
        else if(bPickNode())
        {
            Panel3 const& p3 = m_pWing->panel3At(m_PickedPanelIndex);
            int lastpickedindex = m_PickedNodeIndex;
            pickPanelNode(p3, I, xfl::NOSURFACE);
            if(m_PickedNodeIndex<0 || m_PickedNodeIndex>=int(m_Nodes.size()))
            {
                m_PickedNodeIndex = -1;
                clearTopRightOutput();
            }
            else if(m_PickedNodeIndex!=lastpickedindex)
            {
                if(m_PickedNodeIndex>=0 && m_PickedNodeIndex<int(m_Nodes.size()))
                {
                    Node const &nd = m_Nodes.at(m_PickedNodeIndex);
                    setTopRightOutput(QString::fromStdString(nd.properties()));
                 }
            }
        }
        update();
    }
}


bool gl3dWingView::intersectTheObject(const Vector3d &AA,  const Vector3d &BB, Vector3d &I)
{
    Vector3d U = (BB-AA).normalized();
    Vector3d N;
    if (m_pWing->triangulation().intersect(AA, BB, I, N)) return true;
    return false;
}


/** used only in WingDlg*/
void gl3dWingView::paintWingSectionHighlight()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_vboSectionHighlight.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

            m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::highlightWidth()));
            m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::highlightColor());
            m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));

            int pos = 0;
            for(int iLines=0; iLines<m_nHighlightLines; iLines++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, m_HighlightLineSize);
                pos += m_HighlightLineSize;
            }

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        m_vboSectionHighlight.release();
    }
    m_shadLine.release();
}


void gl3dWingView::glMakeWingSectionHighlight(WingXfl const *pWing, int iSectionHighLight, bool bRightSide)
{
    Node node;

    if(iSectionHighLight<0 || iSectionHighLight>pWing->nSections())
    {
        m_vboSectionHighlight.destroy();
        return;
    }

    int CHORDPOINTS = W3dPrefs::s_iChordwiseRes;
    int iSection = 0;
    int jSurf = 0;
    for(int jSection=0; jSection<pWing->nSections(); jSection++)
    {
        if(jSection==iSectionHighLight) break;
        if((jSection<pWing->nSections()-1) &&fabs(pWing->yPosition(jSection+1)-pWing->yPosition(jSection)) > WingXfl::minSurfaceLength())
            iSection++;
    }

    m_HighlightLineSize = CHORDPOINTS * 2;
    int bufferSize = m_HighlightLineSize *2 *3 ;
    QVector<float> pHighlightVertexArray(bufferSize);

    m_nHighlightLines = 0;
    int iv=0;
    if(iSection==0)
    {
        m_nHighlightLines++;
        //define the inner left side surface
        if(pWing->isTwoSided()) jSurf = pWing->nSurfaces()/2 - 1;
        else                    jSurf = pWing->nSurfaces()   - 1;
        if(jSurf>=0 && jSurf<pWing->nSurfaces())
        {
            for (int lx=0; lx<CHORDPOINTS; lx++)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pWing->surfaceAt(jSurf).getSideNode(xRel, true, xfl::TOPSURFACE, node);
                pHighlightVertexArray[iv++] = node.xf();
                pHighlightVertexArray[iv++] = node.yf();
                pHighlightVertexArray[iv++] = node.zf();
            }
            for (int lx=CHORDPOINTS-1; lx>=0; lx--)
            {
                double xRel = double(lx)/double(CHORDPOINTS-1);
                pWing->surfaceAt(jSurf).getSideNode(xRel, true, xfl::BOTSURFACE, node);
                pHighlightVertexArray[iv++] = node.xf();
                pHighlightVertexArray[iv++] = node.yf();
                pHighlightVertexArray[iv++] = node.zf();
            }
        }
    }
    else
    {
        if((pWing->isSymmetric() || bRightSide) && pWing->isTwoSided())
        {
            m_nHighlightLines++;
            jSurf = pWing->nSurfaces()/2 + iSection -1;
            if(jSurf>=0 && jSurf<pWing->nSurfaces())
            {
                for (int lx=0; lx<CHORDPOINTS; lx++)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pWing->surfaceAt(jSurf).getSideNode(xRel, true, xfl::TOPSURFACE, node);
                    pHighlightVertexArray[iv++] = node.xf();
                    pHighlightVertexArray[iv++] = node.yf();
                    pHighlightVertexArray[iv++] = node.zf();
                }
                for (int lx=CHORDPOINTS-1; lx>=0; lx--)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pWing->surfaceAt(jSurf).getSideNode(xRel, true, xfl::BOTSURFACE, node);
                    pHighlightVertexArray[iv++] = node.xf();
                    pHighlightVertexArray[iv++] = node.yf();
                    pHighlightVertexArray[iv++] = node.zf();
                }
            }
        }

        if(pWing->isSymmetric() || !bRightSide)
        {
            m_nHighlightLines++;
            if(!pWing->isFin())  jSurf = pWing->nSurfaces()/2 - iSection;
            else                 jSurf = pWing->nSurfaces()   - iSection;
            if(jSurf>=0 && jSurf<pWing->nSurfaces())
            {
                //plot A side outline
                for (int lx=0; lx<CHORDPOINTS; lx++)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pWing->surfaceAt(jSurf).getSideNode(xRel, false, xfl::TOPSURFACE, node);
                    pHighlightVertexArray[iv++] = node.xf();
                    pHighlightVertexArray[iv++] = node.yf();
                    pHighlightVertexArray[iv++] = node.zf();
                }

                for (int lx=CHORDPOINTS-1; lx>=0; lx--)
                {
                    double xRel = double(lx)/double(CHORDPOINTS-1);
                    pWing->surfaceAt(jSurf).getSideNode(xRel, false, xfl::BOTSURFACE, node);
                    pHighlightVertexArray[iv++] = node.xf();
                    pHighlightVertexArray[iv++] = node.yf();
                    pHighlightVertexArray[iv++] = node.zf();
                }
            }
        }
    }

    m_vboSectionHighlight.destroy();
    m_vboSectionHighlight.create();
    m_vboSectionHighlight.bind();
    m_vboSectionHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboSectionHighlight.release();
}


void gl3dWingView::glMakeWingSectionHighlight_seg(WingXfl const *pWing, int iSectionHighLight)
{
    if(iSectionHighLight<0 || iSectionHighLight>pWing->nSections())
    {
        m_vboSectionHighlight.destroy();
        return;
    }

    int CHORDPOINTS = W3dPrefs::s_iChordwiseRes;
    CHORDPOINTS = std::max(CHORDPOINTS,2);

    int iSection = 0;
    for(int jSection=0; jSection<pWing->nSections(); jSection++)
    {
        if(jSection==iSectionHighLight) break;
        if((jSection<pWing->nSections()-1) &&fabs(pWing->yPosition(jSection+1)-pWing->yPosition(jSection)) > WingXfl::minSurfaceLength())
            iSection++;
    }

    std::vector<Node> PtTopLeft, PtTopRight, PtBotLeft, PtBotRight;
    std::vector<float> HighlightVertexArray;

    int stride = 6; // 2 vertices/segment x3 space coordinates

    int iv=0;
    int buffersize = (CHORDPOINTS-1)   // number of segments
                     *2                // top and bottom
                     *stride;
    if(pWing->isTwoSided()) buffersize *= 2;

    HighlightVertexArray.resize(buffersize);

    if(pWing->isTwoSided())
    {
        // left side
        int jSurf = pWing->nSurfaces()/2 - iSection;

        Surface const &lsurf = pWing->surfaceAt(jSurf);
        lsurf.getSidePoints_1(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, CHORDPOINTS, xfl::COSINE);
        lsurf.getSidePoints_1(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, CHORDPOINTS, xfl::COSINE);

        for(int i=0; i<CHORDPOINTS-1; i++)
        {
            // top segment
            HighlightVertexArray[iv++] = PtTopLeft.at(i).xf();
            HighlightVertexArray[iv++] = PtTopLeft.at(i).yf();
            HighlightVertexArray[iv++] = PtTopLeft.at(i).zf();

            HighlightVertexArray[iv++] = PtTopLeft.at(i+1).xf();
            HighlightVertexArray[iv++] = PtTopLeft.at(i+1).yf();
            HighlightVertexArray[iv++] = PtTopLeft.at(i+1).zf();

            // bot segment
            HighlightVertexArray[iv++] = PtBotLeft.at(i).xf();
            HighlightVertexArray[iv++] = PtBotLeft.at(i).yf();
            HighlightVertexArray[iv++] = PtBotLeft.at(i).zf();

            HighlightVertexArray[iv++] = PtBotLeft.at(i+1).xf();
            HighlightVertexArray[iv++] = PtBotLeft.at(i+1).yf();
            HighlightVertexArray[iv++] = PtBotLeft.at(i+1).zf();
        }

        // right side
        jSurf = pWing->nSurfaces()/2 + iSection -1;

        Surface const &rsurf = pWing->surfaceAt(jSurf);
        rsurf.getSidePoints_1(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, CHORDPOINTS, xfl::COSINE);
        rsurf.getSidePoints_1(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, CHORDPOINTS, xfl::COSINE);

        for(int i=0; i<CHORDPOINTS-1; i++)
        {
            // top segment
            HighlightVertexArray[iv++] = PtTopRight.at(i).xf();
            HighlightVertexArray[iv++] = PtTopRight.at(i).yf();
            HighlightVertexArray[iv++] = PtTopRight.at(i).zf();

            HighlightVertexArray[iv++] = PtTopRight.at(i+1).xf();
            HighlightVertexArray[iv++] = PtTopRight.at(i+1).yf();
            HighlightVertexArray[iv++] = PtTopRight.at(i+1).zf();

            // bot segment
            HighlightVertexArray[iv++] = PtBotRight.at(i).xf();
            HighlightVertexArray[iv++] = PtBotRight.at(i).yf();
            HighlightVertexArray[iv++] = PtBotRight.at(i).zf();

            HighlightVertexArray[iv++] = PtBotRight.at(i+1).xf();
            HighlightVertexArray[iv++] = PtBotRight.at(i+1).yf();
            HighlightVertexArray[iv++] = PtBotRight.at(i+1).zf();
        }
    }
    else
    {
        // one-sided left wing

        if(iSection==0)
        {
            int jSurf = pWing->nSurfaces()-1 - iSection;

            Surface const &lsurf = pWing->surfaceAt(jSurf);
            lsurf.getSidePoints_1(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, CHORDPOINTS, xfl::COSINE);
            lsurf.getSidePoints_1(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, CHORDPOINTS, xfl::COSINE);

            for(int i=0; i<CHORDPOINTS-1; i++)
            {
                // top segment
                HighlightVertexArray[iv++] = PtTopRight.at(i).xf();
                HighlightVertexArray[iv++] = PtTopRight.at(i).yf();
                HighlightVertexArray[iv++] = PtTopRight.at(i).zf();

                HighlightVertexArray[iv++] = PtTopRight.at(i+1).xf();
                HighlightVertexArray[iv++] = PtTopRight.at(i+1).yf();
                HighlightVertexArray[iv++] = PtTopRight.at(i+1).zf();

                // bot segment
                HighlightVertexArray[iv++] = PtBotRight.at(i).xf();
                HighlightVertexArray[iv++] = PtBotRight.at(i).yf();
                HighlightVertexArray[iv++] = PtBotRight.at(i).zf();

                HighlightVertexArray[iv++] = PtBotRight.at(i+1).xf();
                HighlightVertexArray[iv++] = PtBotRight.at(i+1).yf();
                HighlightVertexArray[iv++] = PtBotRight.at(i+1).zf();
            }
        }
        else
        {
            int jSurf = pWing->nSurfaces() - iSection;

            Surface const &lsurf = pWing->surfaceAt(jSurf);
            lsurf.getSidePoints_1(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, CHORDPOINTS, xfl::COSINE);
            lsurf.getSidePoints_1(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, CHORDPOINTS, xfl::COSINE);

            for(int i=0; i<CHORDPOINTS-1; i++)
            {
                // top segment
                HighlightVertexArray[iv++] = PtTopLeft.at(i).xf();
                HighlightVertexArray[iv++] = PtTopLeft.at(i).yf();
                HighlightVertexArray[iv++] = PtTopLeft.at(i).zf();

                HighlightVertexArray[iv++] = PtTopLeft.at(i+1).xf();
                HighlightVertexArray[iv++] = PtTopLeft.at(i+1).yf();
                HighlightVertexArray[iv++] = PtTopLeft.at(i+1).zf();

                // bot segment
                HighlightVertexArray[iv++] = PtBotLeft.at(i).xf();
                HighlightVertexArray[iv++] = PtBotLeft.at(i).yf();
                HighlightVertexArray[iv++] = PtBotLeft.at(i).zf();

                HighlightVertexArray[iv++] = PtBotLeft.at(i+1).xf();
                HighlightVertexArray[iv++] = PtBotLeft.at(i+1).yf();
                HighlightVertexArray[iv++] = PtBotLeft.at(i+1).zf();
            }
        }
    }

    Q_ASSERT(iv==buffersize);

    m_vboSectionHighlight.destroy();
    m_vboSectionHighlight.create();
    m_vboSectionHighlight.bind();
    m_vboSectionHighlight.allocate(HighlightVertexArray.data(), int(HighlightVertexArray.size()*sizeof(float)));
    m_vboSectionHighlight.release();
}




