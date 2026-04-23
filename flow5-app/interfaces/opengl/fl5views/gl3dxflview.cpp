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


#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QVector4D>
#include <QVBoxLayout>

#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>

#include "gl3dxflview.h"


#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <api/foil.h>
#include <api/frame.h>
#include <api/triangle3d.h>
#include <api/vector3d.h>
#include <api/geom_global.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/controls/colourlegend.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <api/planeopp.h>
#include <api/wingopp.h>
#include <api/planepolar.h>
#include <api/fuseocc.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/pointmass.h>
#include <api/objects3d.h>
#include <api/plane.h>
#include <api/planestl.h>
#include <api/planexfl.h>
#include <api/surface.h>
#include <api/wingxfl.h>
#include <api/anglecontrol.h>
#include <api/boat.h>
#include <api/sail.h>
#include <api/panel4.h>
#include <api/units.h>
#include <api/utils.h>


#define ANIMATIONFRAMES 50


bool gl3dXflView::s_bCirculation = false;
bool gl3dXflView::s_bVortonLine = false;
bool gl3dXflView::s_bNegVortices = false;


bool gl3dXflView::s_bResetglVorticity(true);
bool gl3dXflView::s_bResetglGridVelocities(true);

double gl3dXflView::s_VelocityScale = 0.1;

float gl3dXflView::s_NodeRadius = 0.015f;

gl3dXflView::gl3dXflView(QWidget *pParent) : gl3dView(pParent)
{
    m_Measure.reset();

    m_PickType = xfl::NOPICK;

    m_bResetglMesh  = true;
    m_bResetglSegments = true;

    m_bP3Select = false;

    m_bOutline        = true;
    m_bSurfaces       = true;
    m_bMeshPanels     = false;
    m_bShowMasses     = false;
    m_bFoilNames      = false;
    m_bNormals        = false;
    m_bTessellation   = false;
    m_bHighlightPanel = false;
    m_bCtrlPoints     = false;
    m_bSailCornerPts  = false;
    m_bEdgeNodes      = false;

    m_iSelectedPlaneMass = m_iSelectedPartMass  =-1;

    m_bPickedVertex    = false;
    m_PickedNodeIndex  = -1;
    m_HighFace         = -1;
    m_HighEdge         = -1;
    m_PickedPanelIndex = -1;
    m_NodePair         = {-1, -1};
    m_SurfacePick      = xfl::NOSURFACE;

    m_pfrParts = nullptr;
    m_plabBotLeft  = new QLabel(this);
    m_plabTopRight = new QLabel(this);
    m_plabBotRight = new QLabel(this);

    setLabelFonts();
}


gl3dXflView::~gl3dXflView()
{
    if(m_bAutoDeleteBuffers)
    {
        if(m_vboHighlightPanel3.isCreated())   m_vboHighlightPanel3.destroy();
        if(m_vboSegments.isCreated()) m_vboSegments.destroy();
    }
}


void gl3dXflView::setLabelFonts()
{
    QString stylesheet = QString::asprintf("color: %s;", DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str());
    m_plabBotLeft->setFont(DisplayOptions::textFontStruct().font());
    m_plabBotLeft->setStyleSheet(stylesheet);
    m_plabBotLeft->setTextFormat(Qt::PlainText);
    m_plabBotLeft->setAttribute(Qt::WA_NoSystemBackground);

    m_plabTopRight->setFont(DisplayOptions::textFontStruct().font());
    m_plabTopRight->setStyleSheet(stylesheet);
    m_plabTopRight->setTextFormat(Qt::PlainText);
    m_plabTopRight->setAttribute(Qt::WA_NoSystemBackground);

    m_plabBotRight->setFont(DisplayOptions::textFontStruct().font());
    m_plabBotRight->setStyleSheet(stylesheet);
    m_plabBotRight->setTextFormat(Qt::PlainText);
    m_plabBotRight->setAttribute(Qt::WA_NoSystemBackground);
}


void gl3dXflView::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = pEvent->modifiers() & Qt::ControlModifier;
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            m_Measure.reset();
            m_NodePair = {-1,-1};
            update();
            break;
        }
        case Qt::Key_T:
        {
            if(bCtrl)
            {
                m_bTessellation = !m_bTessellation;
                update();
                pEvent->accept();
                return;
            }
            break;
        }
        default:
            break;
    }
    gl3dView::keyPressEvent(pEvent);
}


void gl3dXflView::setSelectedPart(int index)
{
    if(!m_SelectedParts.contains(index)) m_SelectedParts.append(index);
}


void gl3dXflView::onSurfaces(bool bChecked)
{
    m_bSurfaces = bChecked;
    update();
}


void gl3dXflView::onOutline(bool bChecked)
{
    m_bOutline = bChecked;
    update();
}


void gl3dXflView::onPanels(bool bChecked)
{
    m_bMeshPanels = bChecked;
    update();
}


void gl3dXflView::onNormals(bool bChecked)
{
    m_bNormals = bChecked;
    update();
}


void gl3dXflView::onFoilNames(bool bChecked)
{
    m_bFoilNames = bChecked;
    update();
}


void gl3dXflView::onShowMasses(bool bChecked)
{
    m_bShowMasses = bChecked;
    update();
}


void gl3dXflView::onTessellation(bool bChecked)
{
    m_bTessellation = bChecked;
    update();
}


void gl3dXflView::onHighlightPanel(bool bChecked)
{
    m_PickType = bChecked ? xfl::PANEL3 : xfl::NOPICK;
    m_bHighlightPanel = bChecked;  // highlight the panel
    update();
}


void gl3dXflView::onCtrlPoints(bool bChecked)
{
    m_bCtrlPoints = bChecked;
    update();
}


void gl3dXflView::onCornerPts(bool bChecked)
{
    m_bSailCornerPts = bChecked;
    update();
}


void gl3dXflView::paintPartMasses(Vector3d const &pos, double volumeMass, QString const &tag,
                                  std::vector<PointMass> const &ptMasses, int iSelectedMass)
{
    double delta = 0.02/double(m_glScalef);
    QColor tagcolor;
    if(DisplayOptions::isLightTheme()) tagcolor = W3dPrefs::s_MassColor.darker();
    else                               tagcolor = W3dPrefs::s_MassColor.lighter();
    if(fabs(volumeMass)>PRECISION)
    {
        glRenderText(pos.x, pos.y, pos.z + delta,
                     tag + QString(" (%1 ").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::massUnitQLabel()+")",
                     tagcolor);
    }

    for(int im=0; im<int(ptMasses.size()); im++)
    {
        QColor massclr;
        if(im==iSelectedMass) massclr = Qt::red;
        else                  massclr= W3dPrefs::s_MassColor;
        paintSphere(ptMasses.at(im).position() +pos,
                    W3dPrefs::s_MassRadius/double(m_glScalef),
                    massclr,
                    true);
        glRenderText(ptMasses.at(im).position().x + pos.x,
                     ptMasses.at(im).position().y + pos.y,
                     ptMasses.at(im).position().z + pos.z + delta,
                     QString::fromStdString(ptMasses.at(im).tag())+QString(" (%1 ").arg(ptMasses.at(im).mass()*Units::kgtoUnit(), 0,'g',3)+Units::massUnitQLabel()+")",
                     tagcolor);
    }
}


void gl3dXflView::paintFoilNames(WingXfl const *pWing)
{
    if(!pWing || !pWing->nSurfaces() || !pWing->isWingXfl()) return; //surfaces may not have been constructed yet

    int j(0);
    for(j=0; j<pWing->nSurfaces(); j++)
    {
        Surface const &surf = pWing->surfaceAt(j);
        glRenderText(surf.TA(), QString::fromStdString(surf.leftFoilName()), DisplayOptions::textColor());
    }

    j = pWing->nSurfaces()-1;
    Surface const &surf = pWing->surfaceAt(j);
    glRenderText(surf.TB(), QString::fromStdString(surf.rightFoilName()), DisplayOptions::textColor());
}


void gl3dXflView::paintFlaps(PlaneXfl const* pPlaneXfl, PlanePolar const*pWPolar, const PlaneOpp *pPOpp)
{
    if(!pPlaneXfl) return;

    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);

        if(!pWing->isVisible()) continue;

        AngleControl const *pAVLC(nullptr);
        if(pWPolar && iw<pWPolar->nFlapCtrls()) pAVLC = &pWPolar->flapCtrls(iw);

        uint iFlap = 0;
        for(int j=0; j<pWing->nSurfaces(); j++)
        {
            Surface const &surf = pWing->surfaceAt(j);
            if(surf.hasTEFlap())
            {
                Vector3d const pos = (surf.TB()+surf.TA())/2.0;
                QString strange = QString::asprintf("Flap %d ", iFlap+1) + EOLch;
                if(pAVLC)
                    strange += THETAch + QString::asprintf("=%g", pAVLC->value(iFlap)) + DEGch + EOLch;
                if(pPOpp && iw<pPOpp->nWOpps())
                {
                    if(iFlap<pPOpp->WOpp(iw).m_FlapMoment.size())
                        strange += QString::asprintf("Moment=%.3g ", pPOpp->WOpp(iw).m_FlapMoment.at(iFlap)*Units::NmtoUnit()) + Units::momentUnitQLabel();
                }
                glRenderText(pos, strange, DisplayOptions::textColor());
                iFlap++;
            }
        }
    }
}


void gl3dXflView::glDrawMasses(Plane const *pPlane)
{
    if(!pPlane) return;
    double delta = 0.02/double(m_glScalef);

    if(pPlane->isXflType() && pPlane->bAutoInertia())
    {
        PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            if(pPlaneXfl->wingAt(iw))
            {
                paintPartMasses(pPlaneXfl->wingLE(iw), pPlaneXfl->wingAt(iw)->structuralMass(), QString::fromStdString(pPlaneXfl->wingAt(iw)->name()),
                                pPlaneXfl->wingAt(iw)->pointMasses(), -1);
            }
        }

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);

            paintPartMasses(pPlaneXfl->fusePos(ifuse), pFuse->structuralMass(), QString::fromStdString(pFuse->name()), pFuse->pointMasses(), -1);
        }
    }

    paintPartMasses(Vector3d(), 0.0, "", pPlane->pointMassList(), m_iSelectedPlaneMass);

    //plot CG
    Vector3d Place(pPlane->CoG_t());
    paintSphere(Place, W3dPrefs::s_MassRadius*2.0/double(m_glScalef),
                W3dPrefs::s_MassColor.darker());

    glRenderText(Place.x, Place.y, Place.z + delta,
                 "CoG "+QString("%1 ").arg(pPlane->totalMass()*Units::kgtoUnit(), 0,'f',2)
                 +Units::massUnitQLabel(), W3dPrefs::s_MassColor.darker(125), false, true);
}


void gl3dXflView::setFlags(bool bOutline, bool bSurfaces, bool bVLMPanels, bool bAxes,
                           bool bShowMasses, bool bFoilNames, bool bTessellation, bool bNormals, bool bCtrlPoints)
{
    m_bAxes         = bAxes;
    m_bOutline      = bOutline;
    m_bSurfaces     = bSurfaces;
    m_bMeshPanels   = bVLMPanels;
    m_bShowMasses   = bShowMasses;
    m_bFoilNames    = bFoilNames;
    m_bTessellation = bTessellation;
    m_bNormals      = bNormals;
    m_bCtrlPoints   = bCtrlPoints;
}


void gl3dXflView::setPlaneReferenceLength(Plane const *pPlane)
{
    if(!pPlane) return;
    if(pPlane->isSTLType())
    {
        PlaneSTL const *pPlaneSTL = dynamic_cast<PlaneSTL const*>(pPlane);
        m_RefLength =  pPlaneSTL->maxSize()/2.0; // use the half span
    }
    else if(pPlane->isXflType())
    {
        PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

        m_RefLength = 0.0;
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
            m_RefLength = std::max(m_RefLength, pPlaneXfl->wingAt(iw)->projectedSpan()/2.0); // use the half span

        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            m_RefLength = std::max(m_RefLength, pPlaneXfl->fuseAt(ifuse)->length());
            m_RefLength = std::max(m_RefLength, pPlaneXfl->fuseAt(ifuse)->maxWidth());
            m_RefLength = std::max(m_RefLength, pPlaneXfl->fuseAt(ifuse)->maxHeight());
        }
    }
    else m_RefLength = 1.0;
}


/**
 * Set the reference length for the display so that the whole plane fits in the view
 * @param pBoat
 */
void gl3dXflView::setBoatReferenceLength(const Boat *pBoat)
{
    double length = 10.0;
    if(pBoat)
        length = std::max(pBoat->length(), pBoat->height()) *2;

    if(length<0.1) length = 10.0;
    setReferenceLength(length);
}


void gl3dXflView::paintNormals(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(135,105,35));

        m_shadLine.setUniformValue(m_locLine.m_Thickness, 2.0f);

        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));


            int nNormals = vbo.size()/2/3/int(sizeof(float)); //  (two vertices) x (x,y,z) = 6

            //    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_LINES, 0, nNormals*2);

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        vbo.release();
    }
    m_shadLine.release();
}


void gl3dXflView::paintTriPanels(QOpenGLBuffer &vbo, bool bFrontAndBack)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrColor);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_Light,       0);

        vbo.bind();
        {
            int nTriangles = vbo.size()/3/7/int(sizeof(float));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 7* sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrColor,  GL_FLOAT, 3* sizeof(GLfloat), 4, 7* sizeof(GLfloat));

            if(bFrontAndBack)
            {
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            else
            {
                glEnable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT, GL_FILL);
            }
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);

            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_CULL_FACE);

            m_shadSurf.disableAttributeArray(m_locSurf.m_attrColor);
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        }
        vbo.release();
        //leave things as they were
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
        m_shadSurf.setUniformValue(m_locSurf.m_Light,       1);
    }
    m_shadSurf.release();
}


void gl3dXflView::paintStreamLines(QOpenGLBuffer &vbo, QVector<QColor> const &linecolors, int NX)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    const int stride = 3;

    QMatrix4x4 idMatrix;
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*idMatrix);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj * m_matView);
        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_StreamStyle.m_Width));
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(W3dPrefs::s_StreamStyle.m_Stipple));

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, stride * sizeof(GLfloat));

            int pos=0;

            int nStreamLines = vbo.size()/(NX+1) / stride / sizeof(float);

            for(int il=0; il<nStreamLines; il++)
            {
                if(il<linecolors.size())
                    m_shadLine.setUniformValue(m_locLine.m_UniColor, linecolors.at(il));
                else
                    m_shadLine.setUniformValue(m_locLine.m_UniColor, xfl::fromfl5Clr(W3dPrefs::s_StreamStyle.m_Color));
                glDrawArrays(GL_LINE_STRIP, pos, NX+1);
                pos += NX+1;
            }
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        vbo.release();
    }
    m_shadLine.release();
}


void gl3dXflView::setTopRightOutput(QString const &info)
{
    m_plabTopRight->setText(info);
    int w = rect().width();
    m_plabTopRight->adjustSize();
    QPoint pos1(w-m_plabTopRight->width()-5, 5);
    m_plabTopRight->move(pos1);
}


void gl3dXflView::setBotRightOutput(QString const &info)
{
    m_plabBotRight->setText(info);
    m_plabBotRight->adjustSize();
    QPoint pos2(width()-5-m_plabBotRight->width(), height()-m_plabBotRight->height()-5);
    m_plabBotRight->move(pos2);
}


void gl3dXflView::setBotLeftOutput(QString const &info)
{
    m_plabBotLeft->setText(info);
    m_plabBotLeft->adjustSize();
    QPoint pos0(5, height()-m_plabBotLeft->height()-5);
    m_plabBotLeft->move(pos0);
}


void gl3dXflView::resizeLabels()
{
    setLabelFonts();
    int w = rect().width();
    m_plabTopRight->adjustSize();
    QPoint pos1(w-m_plabTopRight->width()-5, 5);
    m_plabTopRight->move(pos1);

    QPoint pos2(width()-5-m_plabBotRight->width(), height()-m_plabBotRight->height()-5);
    m_plabBotRight->move(pos2);

    QPoint pos0(5, height()-m_plabBotLeft->height()-5);
    m_plabBotLeft->move(pos0);
}


void gl3dXflView::resizeGL(int w, int h)
{
    gl3dView::resizeGL(w, h);
    resizeLabels();
}


void gl3dXflView::showEvent(QShowEvent *pEvent)
{
    resizeLabels();
    gl3dView::showEvent(pEvent);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void gl3dXflView::enterEvent(QEvent *pEvent)
#else
void gl3dXflView::enterEvent(QEnterEvent *pEvent)
#endif
{
    setFocus();
    gl3dView::enterEvent(pEvent);
}


void gl3dXflView::leaveEvent(QEvent *pEvent)
{
    m_PickedPanelIndex = -1;
    clearTopRightOutput();
    update();
    gl3dView::leaveEvent(pEvent);
}


void gl3dXflView::highlightPickedPanel3(Panel3 const &p3)
{
    paintTriangle(m_vboTriangle, true, false, Qt::black);
    QString strong;
    strong = QString::asprintf("T%d", m_PickedPanelIndex);
    glRenderText(m_PickedPoint.x+0.03/double(m_glScalef), m_PickedPoint.y+0.03/double(m_glScalef), m_PickedPoint.z+0.03/double(m_glScalef),
                 strong,
                 DisplayOptions::textColor(), true);
#ifdef QT_DEBUG
/*    for(int in=0; in<3; in++)
    {
        Node const &nd = p3.vertexAt(in);
        glRenderText(nd.x+0.015/double(m_glScalef), nd.y+0.015/double(m_glScalef), nd.z+0.015/double(m_glScalef),
                     QString::asprintf("nd%d", in),
                     s_TextColor, true);
    }*/
    (void)p3;
#else
    (void)p3;
#endif
}


bool gl3dXflView::pickPanel3(QPoint const &point, std::vector<Panel3> const&panels, Vector3d &I)
{
    Vector3d Inter, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

    float zmax = +1.e10;

    for(uint i3=0; i3<panels.size(); i3++)
    {
        Panel3 const &p3 = panels.at(i3);
        if(p3.intersect(AA, U, Inter))
        {
            worldToScreen(Inter, v4d);
            if(v4d.z()<zmax)
            {
                zmax = v4d.z();
                m_PickedPoint = p3.CoG();
                m_PickedPanelIndex = p3.index();
                I = Inter;
            }
        }
    }
    return m_PickedPanelIndex>=0;
}


bool gl3dXflView::pickTriangle3d(QPoint const &point, std::vector<Triangle3d> const&panels, Vector3d &I)
{
    Vector3d Inter, AA, BB;

    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);

    Vector3d U = (BB-AA).normalized();

    QVector4D v4d;

    m_PickedPanelIndex = -1;

    float zmax = +1.e10;

    for(uint i3=0; i3<panels.size(); i3++)
    {
        Triangle3d const &p3 = panels.at(i3);
        if(p3.intersectRayInside(AA, U, Inter))
        {
            worldToScreen(Inter, v4d);
            if(v4d.z()<zmax)
            {
                zmax = v4d.z();
                m_PickedPoint = p3.CoG_g();
                m_PickedPanelIndex = i3;
                I = Inter;
            }
        }
    }
    return m_PickedPanelIndex>=0;
}


void gl3dXflView::pickNode(QPoint const &point, QVector<Node> const &nodes, xfl::enumSurfacePosition pos)
{
    Vector3d I, AA, BB;

    screenToWorld(point, -1.0, AA);
    screenToWorld(point,  1.0, BB);

    QVector4D v4d;

    double dist=0.0;
    double dmax = 100.;
    float zmax = +1.e10;
    double dcrit = 0.05/double(m_glScalef);

    m_PickedNodeIndex = -1;

    for(int in=0; in<nodes.size(); in++)
    {
        Node const &node = nodes.at(in);
        if(node.surfacePosition()==pos || pos==xfl::NOSURFACE)
        {
            dist = geom::distanceToLine3d(AA, BB, node);
            // first screening: set a max distance
            if(dist*0.999<dmax && dist<dcrit) //0.999 coeff in case two points are on a lign parallel to U
            {
                worldToScreen(node, v4d);
                // second screening: keep the node closest to the viewer
                if(v4d.z()<zmax)
                {
                    dmax = dist;
                    zmax = v4d.z();
                    m_PickedPoint = node;
                    m_PickedNodeIndex = node.index();
                }
            }
        }
    }
}


bool gl3dXflView::pickShapeVertex(QPoint const &point, TopoDS_ListOfShape const&shapes, Vector3d &I)
{
    Vector3d AA, BB;

    screenToWorld(point, -1.0, AA);
    screenToWorld(point,  1.0, BB);

    QVector4D v4d;

    double dist=0.0;
    double dmax = 100.;
    float zmax = +1.e10;
    double dcrit = 0.05/double(m_glScalef);

    m_bPickedVertex = false;
    m_PickedNodeIndex = -1;

    TopoDS_ListIteratorOfListOfShape iterator;

    gp_Pnt pt0, pt1;


    for (iterator.Initialize(shapes); iterator.More(); iterator.Next()) // for each of the fuse (cut) shells
    {
        TopExp_Explorer shapeExplorer;
        // for each face of the shape
        for (shapeExplorer.Init(iterator.Value(), TopAbs_VERTEX); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Vertex vtx = TopoDS::Vertex(shapeExplorer.Current());
            gp_Pnt P = BRep_Tool::Pnt(vtx);

            Vector3d node(P.X(), P.Y(), P.Z());

            dist = geom::distanceToLine3d(AA, BB, node);
            // first screening: set a max distance
            if(dist*0.999<dmax && dist<dcrit) //0.999 coeff in case two points are on a lign parallel to U
            {
                worldToScreen(node, v4d);
                // second screening: keep the node closest to the viewer
                if(v4d.z()<zmax)
                {
                    dmax = dist;
                    zmax = v4d.z();
                    m_PickedPoint = node;
                    m_bPickedVertex = true;
                    I = node;
                }
            }

        }
    }

    return m_bPickedVertex;
}


void gl3dXflView::pickPanelNode(Panel3 const &p3, Vector3d const &I, xfl::enumSurfacePosition pos)
{
    Vector3d Inter(I); // make a copy because I is a moving reference to m_PickedPoint;
    if(p3.surfacePosition()!=pos && pos!=xfl::NOSURFACE)
    {
        m_PickedNodeIndex = -1;
        return;
    }
    // find and return the vertex to which I is closest

    double dist=0.0;
    double dmax = 1.e10;

    m_PickedNodeIndex = -1;

    for(int in=0; in<3; in++)
    {
        Node const &node = p3.vertexAt(in);
        dist = Inter.distanceTo(node);
        // first screening: set a max distance
        if(dist<dmax) //0.999 coeff in case two points are on a lign parallel to U
        {
            dmax = dist;
            m_PickedPoint = node;
            m_PickedNodeIndex = node.index();
        }
    }
}


void gl3dXflView::pickTriangleNode(Triangle3d const &p3, Vector3d const &I)
{
    Vector3d Inter(I); // make a copy because I is a moving reference to m_PickedPoint;

    // find and return the vertex to which I is closest

    double dist=0.0;
    double dmax = 1.e10;

    m_PickedNodeIndex = -1;

    for(int in=0; in<3; in++)
    {
        Node const &node = p3.vertexAt(in);
        dist = Inter.distanceTo(node);
        // first screening: set a max distance
        if(dist<dmax) //0.999 coeff in case two points are on a lign parallel to U
        {
            dmax = dist;
            m_PickedPoint = node;
            m_PickedNodeIndex = node.index();
        }
    }
}


void gl3dXflView::updatePartFrame(Plane const *pPlane)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());
    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    if(m_pfrParts) delete m_pfrParts;
    m_pfrParts = new QFrame(this);
    m_pfrParts->setCursor(Qt::ArrowCursor);
//    m_pfrParts->setAutoFillBackground(false);
    m_pfrParts->setAttribute(Qt::WA_NoSystemBackground);
    m_pfrParts->setPalette(palette);
    m_pfrParts->setFrameShape(QFrame::NoFrame);
    m_pfrParts->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    qDeleteAll(m_pfrParts->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
    QLayout *pCurrentLayout = m_pfrParts->layout();
    if(pCurrentLayout) delete pCurrentLayout;

    if(!pPlane) return;
    if(!pPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    int w = 0;

    QVBoxLayout *pPartLayout = new QVBoxLayout;
    {
        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
        {
            WingXfl const *pWing = pPlaneXfl->wingAt(iw);
            QCheckBox *pWingCB = new QCheckBox(QString::fromStdString(pWing->name()));
            pWingCB->setStyleSheet(QString::asprintf("color: %s;", DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str()));
            pWingCB->setChecked(pWing->isVisible());
            pWingCB->setProperty("partindex", iw);
            pWingCB->adjustSize();
            w = std::max(w, pWingCB->width());
            connect(pWingCB, SIGNAL(toggled(bool)), SLOT(onPartSelClicked()));
            pPartLayout->addWidget(pWingCB);
        }
        for(int ifuse=0; ifuse<pPlaneXfl->nFuse(); ifuse++)
        {
            Fuse const *pFuse = pPlaneXfl->fuseAt(ifuse);
            QCheckBox *pFuseCB = new QCheckBox(QString::fromStdString(pFuse->name()));
            pFuseCB->setStyleSheet(QString::asprintf("color: %s;", DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str()));
            pFuseCB->setChecked(pFuse->isVisible());
            pFuseCB->setProperty("partindex", pPlaneXfl->nWings() + ifuse);
            pFuseCB->adjustSize();
            w = std::max(w, pFuseCB->width());
            connect(pFuseCB, SIGNAL(clicked(bool)), SLOT(onPartSelClicked()));
            pPartLayout->addWidget(pFuseCB);
        }
    }
    pPartLayout->setSpacing(0);

    m_pfrParts->setLayout(pPartLayout);
    m_pfrParts->show();
    m_pfrParts->adjustSize();
}


void gl3dXflView::updatePartFrame(Boat const *pBoat)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());
    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    if(m_pfrParts) delete m_pfrParts;
    m_pfrParts = new QFrame(this);
    m_pfrParts->setCursor(Qt::ArrowCursor);
    m_pfrParts->setAutoFillBackground(true);
    m_pfrParts->setPalette(palette);
    m_pfrParts->setFrameShape(QFrame::NoFrame);
    m_pfrParts->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_pfrParts->setAttribute(Qt::WA_NoSystemBackground);

    qDeleteAll(m_pfrParts->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
    QLayout *pCurrentLayout = m_pfrParts->layout();
    if(pCurrentLayout)  delete pCurrentLayout;

    if(!pBoat) return;

    int w = 0;

    QVBoxLayout *pPartLayout = new QVBoxLayout;
    {
        for(int iw=0; iw<pBoat->nSails(); iw++)
        {
            Sail const *pSail = pBoat->sailAt(iw);
            QCheckBox *pSailCB = new QCheckBox(QString::fromStdString(pSail->name()));
            pSailCB->setPalette(palette);
            pSailCB->setChecked(pSail->isVisible());
            pSailCB->setProperty("partindex", iw);
            pSailCB->adjustSize();
            w = std::max(w, pSailCB->width());
            connect(pSailCB, SIGNAL(toggled(bool)), SLOT(onPartSelClicked()));
            pPartLayout->addWidget(pSailCB);
        }
        for(int ifuse=0; ifuse<pBoat->nHulls(); ifuse++)
        {
            Fuse const *pFuse = pBoat->hullAt(ifuse);
            QCheckBox *pFuseCB = new QCheckBox(QString::fromStdString(pFuse->name()));
            pFuseCB->setPalette(palette);
            pFuseCB->setChecked(pFuse->isVisible());
            pFuseCB->setProperty("partindex", pBoat->nSails() + ifuse);
            pFuseCB->adjustSize();
            w = std::max(w, pFuseCB->width());
            connect(pFuseCB, SIGNAL(clicked(bool)), SLOT(onPartSelClicked()));
            pPartLayout->addWidget(pFuseCB);
        }
    }
    m_pfrParts->setLayout(pPartLayout);
    m_pfrParts->show();
    m_pfrParts->adjustSize();
}


void gl3dXflView::paintMeasure()
{
    if(m_Measure.length()>LENGTHPRECISION)
    {
        paintThinArrow(m_Measure.vertexAt(0), m_Measure.segment(), W3dPrefs::highlightColor(), 3, Line::SOLID);
        glRenderText(m_Measure.midPoint()+Vector3d(0.0,0.0,0.02/m_glScalef),
                     QString::asprintf("%g", m_Measure.length()*Units::mtoUnit())+Units::lengthUnitQLabel(),
                     DisplayOptions::textColor(), true);
    }
}


void gl3dXflView::paintSailCornerPoints(Sail const*pSail, Vector3d const &pos)
{
    paintIcoSphere(pos+pSail->clew(), 0.007*2.0/double(m_glScalef), Qt::darkCyan, true, true);
    paintIcoSphere(pos+pSail->tack(), 0.007*2.0/double(m_glScalef), Qt::darkCyan, true, true);
    paintIcoSphere(pos+pSail->head(), 0.007*2.0/double(m_glScalef), Qt::darkCyan, true, true);
    paintIcoSphere(pos+pSail->peak(), 0.007*2.0/double(m_glScalef), Qt::darkCyan, true, true);

    double delta = 0.02/double(m_glScalef);
    glRenderText(pos.x+pSail->clew().x, pos.y+pSail->clew().y, pos.z+pSail->clew().z + delta, "clew", DisplayOptions::textColor(), false, true);
    glRenderText(pos.x+pSail->tack().x, pos.y+pSail->tack().y, pos.z+pSail->tack().z + delta, "tack", DisplayOptions::textColor(), false, true);
    glRenderText(pos.x+pSail->head().x, pos.y+pSail->head().y, pos.z+pSail->head().z + delta, "head", DisplayOptions::textColor(), false, true);
    glRenderText(pos.x+pSail->peak().x, pos.y+pSail->peak().y, pos.z+pSail->peak().z + delta, "peak", DisplayOptions::textColor(), false, true);

/*    Vector3d LE = pSail->head()-pSail->tack();
    if(LE.norm()>1.0e-3)
    {
        paintThinArrow(pos+pSail->tack(), LE, 3, W3dPrefs::highlightColor());
        glRenderText(pos+(pSail->head()+pSail->tack())/2.0 + Vector3d(-delta, -delta,0.0), "Luff axis", s_TextColor, false, true);
    } */
}


int gl3dXflView::pickFace(const QPoint &point, TopoDS_Shape const &shape, TopoDS_Face &pickedface)
{
    if(shape.IsNull())
    {
        return -1;
    }

    Vector3d I, AA, BB;
    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);
    Vector3d U = (BB-AA).normalized();
    QVector4D v4d;

    m_PickedPanelIndex = -1;
    float zmax = +1.e10;

    TopExp_Explorer shapeExplorer;

    int iClosest(-1);
    int nFace = 0;
    for (shapeExplorer.Init(shape, TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
    {
        TopoDS_Face const &aFace = TopoDS::Face(shapeExplorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);
        if(hTriangulation.IsNull())
        {
            return -1;
        }
        else
        {
//            const TColgp_Array1OfPnt& nodes = hTriangulation->Nodes();
//            const Poly_Array1OfTriangle& triangles = hTriangulation->Triangles();

            for (int i3=1; i3<=hTriangulation->NbTriangles(); i3++)
            {
                const Poly_Triangle& tri = hTriangulation->Triangle(i3);
                const Vector3d p1 = Vector3d(hTriangulation->Node(tri(1)).X(), hTriangulation->Node(tri(1)).Y(), hTriangulation->Node(tri(1)).Z());
                const Vector3d p2 = Vector3d(hTriangulation->Node(tri(2)).X(), hTriangulation->Node(tri(2)).Y(), hTriangulation->Node(tri(2)).Z());
                const Vector3d p3 = Vector3d(hTriangulation->Node(tri(3)).X(), hTriangulation->Node(tri(3)).Y(), hTriangulation->Node(tri(3)).Z());

                Triangle3d t3d(p1,p2,p3);

                if(t3d.intersectRayInside(AA, U, I))
                {
                    worldToScreen(t3d.CoG_g(), v4d);
                    if(v4d.z()<zmax)
                    {
                        zmax = v4d.z();
                        m_PickedPoint = t3d.CoG_g();
                        m_PickedPanelIndex = nFace;

                        pickedface = aFace;
                        iClosest = nFace;
                        break; // continue with next FACE
                    }
                }
            }
        }
        nFace++;
    }

    return iClosest;
}


QPair<int, int> gl3dXflView::pickFace(const QPoint &point, TopoDS_ListOfShape const &shapes)
{
    if(shapes.Extent()==0)
    {
        return {-1, -1};
    }

    Vector3d I, AA, BB;
    screenToWorld(point, 0.0, AA);
    screenToWorld(point, 1.0, BB);
    Vector3d U = (BB-AA).normalized();
    QVector4D v4d;

    m_PickedPanelIndex = -1;
    float zmax = +1.e10;

    TopExp_Explorer shapeExplorer;

    int iShell(-1);
    int nShell = 0;
    int iFace(-1);
    int nFace = 0;
    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        for (shapeExplorer.Init(shapeit.Value(), TopAbs_FACE); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Face const &aFace = TopoDS::Face(shapeExplorer.Current());
            TopLoc_Location location;
            Handle(Poly_Triangulation) hTriangulation = BRep_Tool::Triangulation(aFace, location);
            if(hTriangulation.IsNull())
            {
                return {-1,-1};
            }
            else
            {
//                const TColgp_Array1OfPnt& nodes = hTriangulation->Nodes();
//                const Poly_Array1OfTriangle& triangles = hTriangulation->Triangles();

                for (int i3=1; i3<=hTriangulation->NbTriangles(); i3++)
                {
                    const Poly_Triangle& tri = hTriangulation->Triangle(i3);
                    const Vector3d p1 = Vector3d(hTriangulation->Node(tri(1)).X(), hTriangulation->Node(tri(1)).Y(), hTriangulation->Node(tri(1)).Z());
                    const Vector3d p2 = Vector3d(hTriangulation->Node(tri(2)).X(), hTriangulation->Node(tri(2)).Y(), hTriangulation->Node(tri(2)).Z());
                    const Vector3d p3 = Vector3d(hTriangulation->Node(tri(3)).X(), hTriangulation->Node(tri(3)).Y(), hTriangulation->Node(tri(3)).Z());

                    Triangle3d t3d(p1,p2,p3);

                    if(t3d.intersectRayInside(AA, U, I))
                    {
                        worldToScreen(t3d.CoG_g(), v4d);
                        if(v4d.z()<zmax)
                        {
                            zmax = v4d.z();
                            m_PickedPoint = t3d.CoG_g();
                            m_PickedPanelIndex = nFace;

                            iShell     = nShell;
                            iFace      = nFace;
                            break; // continue with next FACE
                        }
                    }
                }
            }
            nFace++;
        }
        nShell++;
    }

    return {iShell, iFace};
}
