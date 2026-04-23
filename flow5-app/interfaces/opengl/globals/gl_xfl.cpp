/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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


#include "gl_xfl.h"


#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <interfaces/opengl/controls/colourlegend.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <api/geom_global.h>
#include <api/boatpolar.h>
#include <api/fusesections.h>
#include <api/fusexfl.h>
#include <api/boat.h>
#include <api/sailspline.h>
#include <api/sailwing.h>
#include <api/wingsailsection.h>



void gl::makeQuadPanels(std::vector<Panel4> const & panel4list, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    if(panel4list.size()<=0)
    {
        vbo.destroy();
        return;
    }

    // write as many nodes as there are triangles.
    //
    // vertices array size:
    //        nPanels
    //      x2 triangles per panels
    //      x3 nodes per triangle
    //        x7 = 3 vertex components + 4 color components

    int nodeVertexSize = int(panel4list.size()) * 2 * 3 * (3+4);
    QVector<float> nodeVertexArray(nodeVertexSize);

    QColor panelclr;
    QColor flapClr = W3dPrefs::s_FlapPanelColor;
    QColor wingclr = W3dPrefs::s_WingPanelColor;
    QColor bodyclr = W3dPrefs::s_FusePanelColor;
    Vector3d TA, LA, TB, LB;

    int nTriangles = 0;
    int iv = 0;
    for (uint i4=0; i4<panel4list.size(); i4++)
    {
        Panel4 const &p4 = panel4list.at(i4);

        if(W3dPrefs::s_bUseBackClr)  panelclr = DisplayOptions::backgroundColor();
        else
        {
            if     (p4.isWingPanel())  panelclr = p4.isFlapPanel() ? flapClr : wingclr;
            else if(p4.isFusePanel())  panelclr = bodyclr;
            else                       panelclr = Qt::lightGray; // xfl::NOSURFACE
        }

        LA.copy(p4.m_Node[0]);
        TA.copy(p4.m_Node[1]);
        TB.copy(p4.m_Node[2]);
        LB.copy(p4.m_Node[3]);

        // each quad is two triangles
        // write the first one
        nodeVertexArray[iv++] = TA.xf()+pos.xf();
        nodeVertexArray[iv++] = TA.yf()+pos.yf();
        nodeVertexArray[iv++] = TA.zf()+pos.zf();
        nodeVertexArray[iv++] = float(panelclr.redF());
        nodeVertexArray[iv++] = float(panelclr.greenF());
        nodeVertexArray[iv++] = float(panelclr.blueF());
        nodeVertexArray[iv++] = float(panelclr.alphaF());

        nodeVertexArray[iv++] = LB.xf()+pos.xf();
        nodeVertexArray[iv++] = LB.yf()+pos.yf();
        nodeVertexArray[iv++] = LB.zf()+pos.zf();
        nodeVertexArray[iv++] = float(panelclr.redF());
        nodeVertexArray[iv++] = float(panelclr.greenF());
        nodeVertexArray[iv++] = float(panelclr.blueF());
        nodeVertexArray[iv++] = float(panelclr.alphaF());

        nodeVertexArray[iv++] = LA.xf()+pos.xf();
        nodeVertexArray[iv++] = LA.yf()+pos.yf();
        nodeVertexArray[iv++] = LA.zf()+pos.zf();
        nodeVertexArray[iv++] = float(panelclr.redF());
        nodeVertexArray[iv++] = float(panelclr.greenF());
        nodeVertexArray[iv++] = float(panelclr.blueF());
        nodeVertexArray[iv++] = float(panelclr.alphaF());

        nTriangles++;

        // write the second one
        nodeVertexArray[iv++] = LB.xf()+pos.xf();
        nodeVertexArray[iv++] = LB.yf()+pos.yf();
        nodeVertexArray[iv++] = LB.zf()+pos.zf();
        nodeVertexArray[iv++] = float(panelclr.redF());
        nodeVertexArray[iv++] = float(panelclr.greenF());
        nodeVertexArray[iv++] = float(panelclr.blueF());
        nodeVertexArray[iv++] = float(panelclr.alphaF());


        nodeVertexArray[iv++] = TA.xf()+pos.xf();
        nodeVertexArray[iv++] = TA.yf()+pos.yf();
        nodeVertexArray[iv++] = TA.zf()+pos.zf();
        nodeVertexArray[iv++] = float(panelclr.redF());
        nodeVertexArray[iv++] = float(panelclr.greenF());
        nodeVertexArray[iv++] = float(panelclr.blueF());
        nodeVertexArray[iv++] = float(panelclr.alphaF());

        nodeVertexArray[iv++] = TB.xf()+pos.xf();
        nodeVertexArray[iv++] = TB.yf()+pos.yf();
        nodeVertexArray[iv++] = TB.zf()+pos.zf();
        nodeVertexArray[iv++] = float(panelclr.redF());
        nodeVertexArray[iv++] = float(panelclr.greenF());
        nodeVertexArray[iv++] = float(panelclr.blueF());
        nodeVertexArray[iv++] = float(panelclr.alphaF());

        nTriangles++;
    }

    (void)nTriangles;

    Q_ASSERT(iv==nodeVertexSize);
    Q_ASSERT(nodeVertexArray.size()==int(panel4list.size())*2*3*7);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeQuadEdges(std::vector<Panel4> const & panel4list, Vector3d const &position, QOpenGLBuffer &vbo)
{
    int nPanel4 = int(panel4list.size());
    // vertices array size:
    //        n quads Panels
    //      x4 edges
    //      x2 nodes per edges
    //        x3 vertex components

    int buffersize = nPanel4*4*2*3;

    QVector<float> nodeVertexArray(buffersize);

    int iv = 0;
    for (int i4=0; i4<nPanel4; i4++)
    {
        Panel4 const &p4 = panel4list.at(i4);

        for(int i=0; i<4; i++)
        {
            nodeVertexArray[iv++] = p4.vertex(i).xf() + position.xf();
            nodeVertexArray[iv++] = p4.vertex(i).yf() + position.yf();
            nodeVertexArray[iv++] = p4.vertex(i).zf() + position.zf();

            nodeVertexArray[iv++] = p4.vertex(i+1).xf() + position.xf();
            nodeVertexArray[iv++] = p4.vertex(i+1).yf() + position.yf();
            nodeVertexArray[iv++] = p4.vertex(i+1).zf() + position.zf();
        }
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriEdges(const std::vector<Panel3> &panel3, const Vector3d &position, QOpenGLBuffer &vbo)
{
    int nPanel3 = int(panel3.size());
    // vertices array size:
    //        n triangular Panels
    //      x3 edges
    //      x2 nodes per edges
    //        x3 vertex components

    int buffersize = nPanel3*3*2*3;

    QVector<float> nodeVertexArray(buffersize);

    int iv = 0;
    for (int i3=0; i3<nPanel3; i3++)
    {
        Panel3 const &p3 = panel3.at(i3);

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = p3.vertexAt(i).xf() + position.xf();
            nodeVertexArray[iv++] = p3.vertexAt(i).yf() + position.yf();
            nodeVertexArray[iv++] = p3.vertexAt(i).zf() + position.zf();

            nodeVertexArray[iv++] = p3.vertexAt(i+1).xf() + position.xf();
            nodeVertexArray[iv++] = p3.vertexAt(i+1).yf() + position.yf();
            nodeVertexArray[iv++] = p3.vertexAt(i+1).zf() + position.zf();
        }
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriPanels(std::vector<Panel3> const &panel3, Vector3d const &position, QOpenGLBuffer &vbo)
{
    int nPanel3 = int(panel3.size());
    // vertices array size:
    //        n triangular Panels
    //      x4 nodes per triangle
    //        x7 = 3 vertex components + 4 color components

    int buffersize = nPanel3*3*7;

    QVector<float> nodeVertexArray(buffersize);

    QColor panelcolor;

    int iv = 0;
    for (int i3=0; i3<nPanel3; i3++)
    {
        Panel3 const &p3 = panel3.at(i3);

        if(W3dPrefs::s_bUseBackClr) panelcolor = DisplayOptions::backgroundColor();
        else
        {
            if (p3.isWingPanel())
            {
                if(p3.isTrailing() /*&&p3.bFromSTL()*/)
                {
                    if(p3.isTopPanel())
                        panelcolor = Qt::darkRed;
                    else
                        panelcolor = Qt::darkCyan;
                }
                else if (p3.isFlapPanel()) panelcolor = W3dPrefs::s_FlapPanelColor;
                else panelcolor = W3dPrefs::s_WingPanelColor;
            }
            else if(p3.isFusePanel()) panelcolor = W3dPrefs::s_FusePanelColor;
            else if(p3.isWakePanel()) panelcolor = W3dPrefs::s_WakePanelColor;
            else
            {
                // Xfl::NOSURFACE
                panelcolor = Qt::lightGray;
            }
        }

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = p3.vertexAt(i).xf() + position.xf();
            nodeVertexArray[iv++] = p3.vertexAt(i).yf() + position.yf();
            nodeVertexArray[iv++] = p3.vertexAt(i).zf() + position.zf();

            nodeVertexArray[iv++] = float(panelcolor.redF());
            nodeVertexArray[iv++] = float(panelcolor.greenF());
            nodeVertexArray[iv++] = float(panelcolor.blueF());
            nodeVertexArray[iv++] = float(panelcolor.alphaF());
        }
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makePanelNormals(std::vector<Panel3> const &panel3, float length, QOpenGLBuffer &vbo)
{
    // vertices array size:
    //        n  Panels
    //      x2 nodes per normal
    //        x3 = 3 vertex components
    int buffersize = int(panel3.size()) * 2 * 3;
    QVector<float>NormalVertexArray(buffersize);

    int iv = 0;
    int vec=0;
    for (uint i3=0; i3<panel3.size(); i3++)
    {
        Panel3 const & p3 = panel3.at(i3);

        Vector3d pt = p3.CoG();

        NormalVertexArray[iv++] = pt.xf();
        NormalVertexArray[iv++] = pt.yf();
        NormalVertexArray[iv++] = pt.zf();

        NormalVertexArray[iv++] = pt.xf() + p3.normal().xf() * length;
        NormalVertexArray[iv++] = pt.yf() + p3.normal().yf() * length;
        NormalVertexArray[iv++] = pt.zf() + p3.normal().zf() * length;
        vec++;

    }
    (void)vec;

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(NormalVertexArray.data(), NormalVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makePanelNormals(std::vector<Panel4> const & panel4list, float length, QOpenGLBuffer &vbo)
{
    // vertices array size:
    //        n  Panels
    //      x2 nodes per normal
    //        x3 = 3 vertex components

    int buffersize = int(panel4list.size())*2*3;
    QVector<float>nodeVertexArray(buffersize);

    int iv = 0;
    int vec=0;
    for (uint i4=0; i4<panel4list.size(); i4++)
    {
        Panel4 const & p4 = panel4list.at(i4);
//        if(p3.isTrailing() && p3.isBotPanel())
        {
            Vector3d pt = p4.CoG();

            nodeVertexArray[iv++] = pt.xf();
            nodeVertexArray[iv++] = pt.yf();
            nodeVertexArray[iv++] = pt.zf();

            nodeVertexArray[iv++] = pt.xf() + p4.normal().xf() * length;
            nodeVertexArray[iv++] = pt.yf() + p4.normal().yf() * length;
            nodeVertexArray[iv++] = pt.zf() + p4.normal().zf() * length;
            vec++;
        }
    }

    (void)vec;

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));

    vbo.release();
}


void gl::makeBodySplines_outline(FuseXfl const *pFuseXfl, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    if(!pFuseXfl) return;
    int NX = W3dPrefs::bodyAxialRes();
    int NZ = W3dPrefs::bodyHoopRes();

    NURBSSurface const & nurbs = pFuseXfl->nurbs();
    Vector3d Point, Point1;

    //OUTLINE
    //     frameSize()*(NH+1) : frames
    //     (NX+1) + (NX+1)    : top and bottom lines
    //
    int nsegs =   nurbs.frameCount()*NX*2 // frames
                + NZ                       // top outline
                + NZ;                      // bot outline
    int outlinesize =  nsegs * 2 * 3; // x2vertices x3 vertices components

    QVector<float> OutlineVertexArray(outlinesize);
    std::vector<double> fraclist;
    xfl::getPointDistribution(fraclist, NX, xfl::COSINE);

    double u(0), v(0), v1(0);

    int iv(0);

    for (int iFr=0; iFr<nurbs.frameCount(); iFr++)
    {
        u = nurbs.getu(nurbs.frameAt(iFr).position().x);
        for (uint j=0; j<fraclist.size()-1; j++)
        {
            v  = fraclist.at(j);
            v1 = fraclist.at(j+1);
            nurbs.getPoint(u,v, Point);
            nurbs.getPoint(u,v1,Point1);
            OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
            OutlineVertexArray[iv++] = Point.yf()  + pos.yf();
            OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
            OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
            OutlineVertexArray[iv++] = Point1.yf() + pos.yf();
            OutlineVertexArray[iv++] = Point1.zf() + pos.zf();

            // add xz symmetric segment
            OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
            OutlineVertexArray[iv++] = -(Point.yf()  + pos.yf());
            OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
            OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
            OutlineVertexArray[iv++] = -(Point1.yf() + pos.yf());
            OutlineVertexArray[iv++] = Point1.zf() + pos.zf();
        }
    }

    //top line: NX+1
    v = 0.0;
    for (int iu=0; iu<NZ; iu++)
    {
        nurbs.getPoint(double(iu)  /double(NZ),v, Point);
        nurbs.getPoint(double(iu+1)/double(NZ),v, Point1);
        OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
        OutlineVertexArray[iv++] = Point.yf()  + pos.yf();
        OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
        OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
        OutlineVertexArray[iv++] = Point1.yf() + pos.yf();
        OutlineVertexArray[iv++] = Point1.zf() + pos.zf();
    }

    //bottom line: NX+1
    v = 1.0;
    for (int iu=0; iu<NZ; iu++)
    {
        nurbs.getPoint(double(iu)  /double(NZ),v, Point);
        nurbs.getPoint(double(iu+1)/double(NZ),v, Point1);
        OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
        OutlineVertexArray[iv++] = Point.yf()  + pos.yf();
        OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
        OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
        OutlineVertexArray[iv++] = Point1.yf() + pos.yf();
        OutlineVertexArray[iv++] = Point1.zf() + pos.zf();
    }
    Q_ASSERT(iv==outlinesize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinesize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeSplineSailOutline(SailSpline const *pSSail, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    if(!pSSail) return;

    Vector2d p0, p1;
    Vector3d Pt, Pt1, pt3d, pt3d1;
    double v(0), v1(0);

    int NX = Sail::iXRes();
    int NZ = Sail::iZRes();

    // outline:
    //     frameSize()*(NH+1) : frames
    //     (NX+1) + (NX+1)    : top and bottom lines
    //
    int nsegs = pSSail->sectionCount()*NX  // frames
                + NZ                       // top outline
                + NZ;                      // bot outline

    int outlinesize = nsegs * 2 *3; // x3 vertices components

    QVector<float> OutlineVertexArray(outlinesize);

    std::vector<double> fraclist;
    xfl::getPointDistribution(fraclist, NX, xfl::COSINE);

    int iv=0;
    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pSSail->sectionCount(); iFr++)
    {
        Spline const *pSpline = pSSail->splineAt(iFr);
        Vector3d O(pSpline->firstCtrlPoint().x, pSpline->firstCtrlPoint().y, pSSail->sectionPosition(iFr).z);
        for(uint j=0; j<fraclist.size()-1; j++)
        {
            v  = fraclist.at(j);
            v1 = fraclist.at(j+1);
//            if(v>=1.0) v=0.999999; /** @todo do something */
//            if(v<=0.0) v=0.000001;
            p0 = pSpline->splinePoint(v);
            p1 = pSpline->splinePoint(v1);
            Pt.x  = p0.x;
            Pt.y  = p0.y;
            Pt1.x = p1.x;
            Pt1.y = p1.y;
            pt3d.set( Pt.x,  Pt.y,  pSSail->sectionPosition(iFr).z);
            pt3d1.set(Pt1.x, Pt1.y, pSSail->sectionPosition(iFr).z);

            pt3d.rotateY( O, pSSail->sectionAngle(iFr));
            pt3d1.rotateY(O, pSSail->sectionAngle(iFr));
            OutlineVertexArray[iv++] = pt3d.xf()  + pos.xf();
            OutlineVertexArray[iv++] = pt3d.yf()  + pos.yf();
            OutlineVertexArray[iv++] = pt3d.zf()  + pos.zf();
            OutlineVertexArray[iv++] = pt3d1.xf() + pos.xf();
            OutlineVertexArray[iv++] = pt3d1.yf() + pos.yf();
            OutlineVertexArray[iv++] = pt3d1.zf() + pos.zf();
        }
    }

    //leading line: NZ+1
    for (int iu=0; iu<NZ; iu++)
    {
        Pt  = pSSail->point(0.0, double(iu)/double(NZ));
        Pt1 = pSSail->point(0.0, double(iu+1)/double(NZ));
        OutlineVertexArray[iv++] = Pt.xf()  + pos.xf();
        OutlineVertexArray[iv++] = Pt.yf()  + pos.yf();
        OutlineVertexArray[iv++] = Pt.zf()  + pos.zf();
        OutlineVertexArray[iv++] = Pt1.xf() + pos.xf();
        OutlineVertexArray[iv++] = Pt1.yf() + pos.yf();
        OutlineVertexArray[iv++] = Pt1.zf() + pos.zf();
    }

    //trailing line: NZ+1
    for (int iu=0; iu<NZ; iu++)
    {
        Pt  = pSSail->point(1.0, double(iu)/double(NZ));
        Pt1 = pSSail->point(1.0, double(iu+1)/double(NZ));
        OutlineVertexArray[iv++] = Pt.xf() + pos.xf();
        OutlineVertexArray[iv++] = Pt.yf() + pos.yf();
        OutlineVertexArray[iv++] = Pt.zf() + pos.zf();
        OutlineVertexArray[iv++] = Pt1.xf()+ pos.xf();
        OutlineVertexArray[iv++] = Pt1.yf()+ pos.yf();
        OutlineVertexArray[iv++] = Pt1.zf()+ pos.zf();
    }

    Q_ASSERT(iv==outlinesize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinesize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeWingSailOutline(SailWing const *pWSail, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    if(!pWSail) return;

    int NX = Sail::iXRes();
    int NZ = Sail::iZRes();


    Vector3d Point;
    double v=0;

    int nsegs =  pWSail->sectionCount()*2*NX // frames
                 + NZ                        // top outline
                 + NZ;                       // bot outline

    int outlinesize = nsegs * 2 * 3; // x3 vertices components

    QVector<float> OutlineVertexArray(outlinesize, 0);

    int iv(0);

    Vector3d pt3d, pt3d1;
    std::vector<double> fraclist;
    xfl::getPointDistribution(fraclist, NX, xfl::COSINE);

    double x(0), y(0);
    for (int iSec=0; iSec<pWSail->sectionCount(); iSec++)
    {
        WingSailSection const &section = pWSail->sectionAt(iSec);

        for(uint j=0; j<fraclist.size()-1; j++)
        {
            v = fraclist.at(j);
            section.sectionPoint(v, xfl::TOPSURFACE, x, y);
            pt3d.set(pWSail->sectionPosition(iSec).x+x, y, pWSail->sectionPosition(iSec).z);
            pt3d.rotateZ(pWSail->sectionPosition(iSec), -section.twist());
            pt3d.rotateY(pWSail->sectionPosition(iSec), pWSail->sectionAngle(iSec));

            v = fraclist.at(j+1);
            section.sectionPoint(v, xfl::TOPSURFACE, x, y);
            pt3d1.set(pWSail->sectionPosition(iSec).x+x, y, pWSail->sectionPosition(iSec).z);
            pt3d1.rotateZ(pWSail->sectionPosition(iSec), -section.twist());
            pt3d1.rotateY(pWSail->sectionPosition(iSec), pWSail->sectionAngle(iSec));

            OutlineVertexArray[iv++] = pt3d.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt3d.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt3d.zf()+pos.zf();

            OutlineVertexArray[iv++] = pt3d1.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt3d1.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt3d1.zf()+pos.zf();
        }

        for(uint j=0; j<fraclist.size()-1; j++)
        {
            v = fraclist.at(j);
            section.sectionPoint(v, xfl::BOTSURFACE, x, y);
            pt3d.set(pWSail->sectionPosition(iSec).x+x, y, pWSail->sectionPosition(iSec).z);
            pt3d.rotateZ(pWSail->sectionPosition(iSec), -section.twist());
            pt3d.rotateY(pWSail->sectionPosition(iSec), pWSail->sectionAngle(iSec));

            v = fraclist.at(j+1);
            section.sectionPoint(v, xfl::BOTSURFACE, x, y);
            pt3d1.set(pWSail->sectionPosition(iSec).x+x, y, pWSail->sectionPosition(iSec).z);
            pt3d1.rotateZ(pWSail->sectionPosition(iSec), -section.twist());
            pt3d1.rotateY(pWSail->sectionPosition(iSec), pWSail->sectionAngle(iSec));

            OutlineVertexArray[iv++] = pt3d.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt3d.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt3d.zf()+pos.zf();

            OutlineVertexArray[iv++] = pt3d1.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt3d1.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt3d1.zf()+pos.zf();
        }
    }

    //leading line: NZ+1
    for (int iu=0; iu<NZ; iu++)
    {
        Point.set(pWSail->point(0.0, double(iu)/double(NZ)));
        OutlineVertexArray[iv++] = Point.xf()+pos.xf();
        OutlineVertexArray[iv++] = Point.yf()+pos.yf();
        OutlineVertexArray[iv++] = Point.zf()+pos.zf();

        Point.set(pWSail->point(0.0, double(iu+1)/double(NZ)));
        OutlineVertexArray[iv++] = Point.xf()+pos.xf();
        OutlineVertexArray[iv++] = Point.yf()+pos.yf();
        OutlineVertexArray[iv++] = Point.zf()+pos.zf();
    }

    //trailing line: NZ+1
    for (int iu=0; iu<NZ; iu++)
    {
        Point.set(pWSail->point(1.0, double(iu)/double(NZ)));
        OutlineVertexArray[iv++] = Point.xf()+pos.xf();
        OutlineVertexArray[iv++] = Point.yf()+pos.yf();
        OutlineVertexArray[iv++] = Point.zf()+pos.zf();

        Point.set(pWSail->point(1.0, double(iu+1)/double(NZ)));
        OutlineVertexArray[iv++] = Point.xf()+pos.xf();
        OutlineVertexArray[iv++] = Point.yf()+pos.yf();
        OutlineVertexArray[iv++] = Point.zf()+pos.zf();
    }

    Q_ASSERT(iv==outlinesize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinesize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeSectionHighlight(SailSpline const *pSSail, Vector3d pos, QOpenGLBuffer &vbo)
{
    int iFr = pSSail->activeSection();
    Spline const *pSpline = pSSail->activeSpline();
    if(!pSpline)
    {
        vbo.destroy();
        return;
    }

    int NX = Sail::iXRes();


    int bufferSize = NX+1;
    bufferSize *=3; // x3 vertices components

    QVector<float> HighlightVertexArray(bufferSize);

    double hinc=1./double(NX);

    int iv = 0;
    Vector2d Point;
    for (int j=0; j<=NX; j++)
    {
        double v = double(j)*hinc;
        if(v>=1.0) v=0.999999; /** @todo do something */
        if(v<=0.0) v=0.000001;
        Point = pSpline->splinePoint(v);
        Vector3d pt3d(Point.x, Point.y, pSSail->sectionPosition(iFr).z);
        Vector3d O(pSpline->firstCtrlPoint().x, pSpline->firstCtrlPoint().y, pSSail->sectionPosition(iFr).z);
        pt3d.rotateY(O, pSSail->sectionAngle(iFr));
        HighlightVertexArray[iv++] = pt3d.xf()+pos.xf();
        HighlightVertexArray[iv++] = pt3d.yf()+pos.yf();
        HighlightVertexArray[iv++] = pt3d.zf()+pos.zf();
    }

    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(HighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    vbo.release();
}


void gl::makeSectionHighlight(SailWing const *pWSail, Vector3d pos, QOpenGLBuffer &vbo)
{
    int iSec = pWSail->activeSection();
    if(iSec<0 || iSec>=pWSail->sectionCount())
    {
        vbo.destroy();
        return;
    }
    WingSailSection const &section = pWSail->sectionAt(iSec);

    int NX = Sail::iXRes();

    std::vector<double> fraclist;
    xfl::getPointDistribution(fraclist, NX);

    int bufferSize = int(fraclist.size())*2;
    bufferSize *=3; // x3 vertices components

    QVector<float> HighlightVertexArray(bufferSize);

    int iv = 0;
    Vector2d Point;
    for(uint j=0; j<fraclist.size(); j++)
    {
        double v = fraclist.at(j);
        section.sectionPoint(v, xfl::TOPSURFACE, Point.x, Point.y);
        Vector3d pt3d(pWSail->sectionPosition(iSec).x+Point.x, Point.y, pWSail->sectionPosition(iSec).z);
        pt3d.rotateZ(pWSail->sectionPosition(iSec), -section.twist());
        pt3d.rotateY(pWSail->sectionPosition(iSec), pWSail->sectionAngle(iSec));
        HighlightVertexArray[iv++] = pt3d.xf()+pos.xf();
        HighlightVertexArray[iv++] = pt3d.yf()+pos.yf();
        HighlightVertexArray[iv++] = pt3d.zf()+pos.zf();
    }
    for (int j=int(fraclist.size())-1; j>=0; j--)
    {
        double v = fraclist.at(j);
        section.sectionPoint(v, xfl::BOTSURFACE, Point.x, Point.y);
        Vector3d pt3d(pWSail->sectionPosition(iSec).x+Point.x, Point.y, pWSail->sectionPosition(iSec).z);
        pt3d.rotateZ(pWSail->sectionPosition(iSec), -section.twist());
        pt3d.rotateY(pWSail->sectionPosition(iSec), pWSail->sectionAngle(iSec));
        HighlightVertexArray[iv++] = pt3d.xf()+pos.xf();
        HighlightVertexArray[iv++] = pt3d.yf()+pos.yf();
        HighlightVertexArray[iv++] = pt3d.zf()+pos.zf();
    }

    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(HighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    vbo.release();
}


/** make the wind arrows set at beta=0; will be rotated along the wind at display time */
void gl::makeWind(Boat const*pBoat, BoatPolar const *pBtPolar, QOpenGLBuffer &vbo)
{
    double  maxheight= 30.0;  // 100m
    if(pBoat) maxheight = pBoat->height()*1.5;
    double windstrength = 5.0;
    double distance = 3.*windstrength;
    if(pBtPolar) distance *=std::min(1.0, pBtPolar->windForce(10.0));

    int nArrows = 10;
    int buffersize = nArrows;
    buffersize *=3;  // x3 lines/arrow
    buffersize *=2;  // x2 vertices/lines
    buffersize *=3;  // x3 vertices components

    QVector<float> VertexArray(buffersize);
    int iv=0;
    Vector3d O,P,P1,P2;

    for(int iw=0; iw<nArrows; iw++)
    {
        double h = double(iw)/10.0 * maxheight;
        double windfactor = 1.0;
        if(pBtPolar) windfactor = pBtPolar->windForce(h);
        P.set(1.0,0.0,0.0);
        P1.set( 0.15, 0.0, +0.05);
        P2.set( 0.15, 0.0, -0.05);
        P  *= windfactor * windstrength;
        P1 *= windfactor * windstrength;
        P2 *= windfactor * windstrength;

        O.set(-distance, 0, h);

        VertexArray[iv++] = O.xf();
        VertexArray[iv++] = O.yf();
        VertexArray[iv++] = O.zf();

        VertexArray[iv++] = O.xf()+P.xf();
        VertexArray[iv++] = O.yf()+P.yf();
        VertexArray[iv++] = O.zf()+P.zf();

        VertexArray[iv++] = O.xf()+P.xf();
        VertexArray[iv++] = O.yf()+P.yf();
        VertexArray[iv++] = O.zf()+P.zf();
        VertexArray[iv++] = O.xf()+P.xf()-P1.xf();
        VertexArray[iv++] = O.yf()+P.yf()-P1.yf();
        VertexArray[iv++] = O.zf()+P.zf()-P1.zf();

        VertexArray[iv++] = O.xf()+P.xf();
        VertexArray[iv++] = O.yf()+P.yf();
        VertexArray[iv++] = O.zf()+P.zf();
        VertexArray[iv++] = O.xf()+P.xf()-P2.xf();
        VertexArray[iv++] = O.yf()+P.yf()-P2.yf();
        VertexArray[iv++] = O.zf()+P.zf()-P2.zf();
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(VertexArray.data(), buffersize*int(sizeof(float)));
    vbo.release();

}


/**< used for fuse/hull edition only */
void gl::makeFuseXflQuadPanels(FuseXfl const *pFuseXfl, Vector3d position, QOpenGLBuffer &vbo)
{
    if(!pFuseXfl)
    {
        vbo.bind();
        vbo.destroy();
        vbo.release();
        return;
    }
    if     (pFuseXfl->isFlatFaceType()) gl::makeFuseXflFlatPanels(pFuseXfl, position, vbo);
    else if(pFuseXfl->isSplineType())   gl::makeFuseXflSplinePanels(pFuseXfl, position, vbo);
}


void gl::makeFuseXflFlatPanels(FuseXfl const*pFuseXfl, Vector3d position, QOpenGLBuffer &vbo)
{
    if(!pFuseXfl) return;

    double dpx=position.x;
    double dpy=position.y;
    double dpz=position.z;

    double dj=0, dj1=0.0;
    Vector3d PLA, PLB, PTA, PTB, LA, LB, TA, TB;

    // count the number of quads
    int nx = 0;
    for(int i=0; i<pFuseXfl->frameCount()-1; i++) nx+=pFuseXfl->xPanels(i);
    int nh = 0;
    for(int i=0; i<pFuseXfl->sideLineCount()-1; i++) nh+=pFuseXfl->hPanels(i);

    int buffersize = 0;
    buffersize  = nx*nh*2; // number of quads
    buffersize *= 2; // two triangles/quads
    buffersize *= 3; // three vertices/triangles
    buffersize *= 3; // three components/vertex
    QVector<float> meshVertexArray(buffersize);

    int iv=0;
    for (int i=0; i<pFuseXfl->frameCount()-1; i++)
    {
        for (int j=0; j<pFuseXfl->xPanels(i); j++)
        {
            dj  = double(j)  /double(pFuseXfl->xPanels(i));
            dj1 = double(j+1)/double(pFuseXfl->xPanels(i));

            //body left side
            for (int k=0; k<pFuseXfl->sideLineCount()-1; k++)
            {
                //build the four corner points of the strips
                PLB.x =  (1.0- dj) * pFuseXfl->framePosition(i)             +  dj * pFuseXfl->framePosition(i+1)               +dpx;
                PLB.y = -(1.0- dj) * pFuseXfl->frameAt(i).ctrlPointAt(k).y  -  dj * pFuseXfl->frameAt(i+1).ctrlPointAt(k).y    +dpy;
                PLB.z =  (1.0- dj) * pFuseXfl->frameAt(i).ctrlPointAt(k).z  +  dj * pFuseXfl->frameAt(i+1).ctrlPointAt(k).z    +dpz;

                PTB.x =  (1.0-dj1) * pFuseXfl->framePosition(i)             + dj1 * pFuseXfl->framePosition(i+1)               +dpx;
                PTB.y = -(1.0-dj1) * pFuseXfl->frameAt(i).ctrlPointAt(k).y  - dj1 * pFuseXfl->frameAt(i+1).ctrlPointAt(k).y    +dpy;
                PTB.z =  (1.0-dj1) * pFuseXfl->frameAt(i).ctrlPointAt(k).z  + dj1 * pFuseXfl->frameAt(i+1).ctrlPointAt(k).z    +dpz;

                PLA.x =  (1.0- dj) * pFuseXfl->framePosition(i)              +  dj * pFuseXfl->framePosition(i+1)               +dpx;
                PLA.y = -(1.0- dj) * pFuseXfl->frameAt(i).ctrlPointAt(k+1).y -  dj * pFuseXfl->frameAt(i+1).ctrlPointAt(k+1).y  +dpy;
                PLA.z =  (1.0- dj) * pFuseXfl->frameAt(i).ctrlPointAt(k+1).z +  dj * pFuseXfl->frameAt(i+1).ctrlPointAt(k+1).z  +dpz;

                PTA.x =  (1.0-dj1) * pFuseXfl->framePosition(i)              + dj1 * pFuseXfl->framePosition(i+1)               +dpx;
                PTA.y = -(1.0-dj1) * pFuseXfl->frameAt(i).ctrlPointAt(k+1).y - dj1 * pFuseXfl->frameAt(i+1).ctrlPointAt(k+1).y  +dpy;
                PTA.z =  (1.0-dj1) * pFuseXfl->frameAt(i).ctrlPointAt(k+1).z + dj1 * pFuseXfl->frameAt(i+1).ctrlPointAt(k+1).z  +dpz;

                LB = PLB;
                TB = PTB;

                for (int l=0; l<pFuseXfl->hPanels(k); l++)
                {

                    double dl1  = double(l+1) / double(pFuseXfl->hPanels(k));
                    LA = PLB * (1.0- dl1) + PLA * dl1;
                    TA = PTB * (1.0- dl1) + PTA * dl1;


                    //LEFT SIDE TRIANGLES
                    //first triangle
                    meshVertexArray[iv++] = LA.xf();
                    meshVertexArray[iv++] = LA.yf();
                    meshVertexArray[iv++] = LA.zf();
                    meshVertexArray[iv++] = TA.xf();
                    meshVertexArray[iv++] = TA.yf();
                    meshVertexArray[iv++] = TA.zf();
                    meshVertexArray[iv++] = TB.xf();
                    meshVertexArray[iv++] = TB.yf();
                    meshVertexArray[iv++] = TB.zf();

                    //second triangle
                    meshVertexArray[iv++] = TB.xf();
                    meshVertexArray[iv++] = TB.yf();
                    meshVertexArray[iv++] = TB.zf();
                    meshVertexArray[iv++] = LB.xf();
                    meshVertexArray[iv++] = LB.yf();
                    meshVertexArray[iv++] = LB.zf();
                    meshVertexArray[iv++] = LA.xf();
                    meshVertexArray[iv++] = LA.yf();
                    meshVertexArray[iv++] = LA.zf();

                    //RIGHT SIDE TRIANGLES
                    //first triangle
                    meshVertexArray[iv++] =  LA.xf();
                    meshVertexArray[iv++] = -LA.yf();
                    meshVertexArray[iv++] =  LA.zf();
                    meshVertexArray[iv++] =  TA.xf();
                    meshVertexArray[iv++] = -TA.yf();
                    meshVertexArray[iv++] =  TA.zf();
                    meshVertexArray[iv++] =  TB.xf();
                    meshVertexArray[iv++] = -TB.yf();
                    meshVertexArray[iv++] =  TB.zf();

                    //second triangle
                    meshVertexArray[iv++] =  TB.xf();
                    meshVertexArray[iv++] = -TB.yf();
                    meshVertexArray[iv++] =  TB.zf();
                    meshVertexArray[iv++] =  LB.xf();
                    meshVertexArray[iv++] = -LB.yf();
                    meshVertexArray[iv++] =  LB.zf();
                    meshVertexArray[iv++] =  LA.xf();
                    meshVertexArray[iv++] = -LA.yf();
                    meshVertexArray[iv++] =  LA.zf();

                    LB = LA;
                    TB = TA;
                }
            }
        }
    }
    //    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshVertexArray.data(), meshVertexArray.size()* int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeFuseXflSplinePanels(FuseXfl const *pFuseXfl, Vector3d position, QOpenGLBuffer &vbo)
{
    int nx=0, nh=0;

    Vector3d P1, P2, P3, P4;
    QVector<float> meshVertexArray;

    double dx = position.x;
    double dy = position.y;
    double dz = position.z;


    nx = pFuseXfl->nxNurbsPanels();
    nh = pFuseXfl->nhNurbsPanels();

    int bufferSize = nx*nh *9 *2;
    bufferSize *=2;

    meshVertexArray.resize(bufferSize);

    //x-lines;
    int iv=0;
    for (int k=0; k<nx; k++)
    {
        double u  = double(k)  /double(nx);
        double u1 = double(k+1)/double(nx);
        for (int l=0; l<nh; l++)
        {
            double v  = double(l)  /double(nh);
            double v1 = double(l+1)/double(nh);
            pFuseXfl->getPoint(u,  v,  true, P1);
            pFuseXfl->getPoint(u1, v,  true, P2);
            pFuseXfl->getPoint(u1, v1, true, P3);
            pFuseXfl->getPoint(u,  v1, true, P4);

            P1.x+=dx;   P2.x+=dx;   P3.x+=dx;   P4.x+=dx;
            P1.y+=dy;   P2.y+=dy;   P3.y+=dy;   P4.y+=dy;
            P1.z+=dz;   P2.z+=dz;   P3.z+=dz;   P4.z+=dz;

            //LEFT SIDE TRIANGLES
            //first triangle
            meshVertexArray[iv++] = P1.xf();
            meshVertexArray[iv++] = P1.yf();
            meshVertexArray[iv++] = P1.zf();
            meshVertexArray[iv++] = P2.xf();
            meshVertexArray[iv++] = P2.yf();
            meshVertexArray[iv++] = P2.zf();
            meshVertexArray[iv++] = P3.xf();
            meshVertexArray[iv++] = P3.yf();
            meshVertexArray[iv++] = P3.zf();

            //second triangle
            meshVertexArray[iv++] = P3.xf();
            meshVertexArray[iv++] = P3.yf();
            meshVertexArray[iv++] = P3.zf();
            meshVertexArray[iv++] = P4.xf();
            meshVertexArray[iv++] = P4.yf();
            meshVertexArray[iv++] = P4.zf();
            meshVertexArray[iv++] = P1.xf();
            meshVertexArray[iv++] = P1.yf();
            meshVertexArray[iv++] = P1.zf();

            //RIGHT SIDE TRIANGLES
            //first triangle
            meshVertexArray[iv++] =  P1.xf();
            meshVertexArray[iv++] = -P1.yf();
            meshVertexArray[iv++] =  P1.zf();
            meshVertexArray[iv++] =  P2.xf();
            meshVertexArray[iv++] = -P2.yf();
            meshVertexArray[iv++] =  P2.zf();
            meshVertexArray[iv++] =  P3.xf();
            meshVertexArray[iv++] = -P3.yf();
            meshVertexArray[iv++] =  P3.zf();

            //second triangle
            meshVertexArray[iv++] =  P3.xf();
            meshVertexArray[iv++] = -P3.yf();
            meshVertexArray[iv++] =  P3.zf();
            meshVertexArray[iv++] =  P4.xf();
            meshVertexArray[iv++] = -P4.yf();
            meshVertexArray[iv++] =  P4.zf();
            meshVertexArray[iv++] =  P1.xf();
            meshVertexArray[iv++] = -P1.yf();
            meshVertexArray[iv++] =  P1.zf();
        }
    }
    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshVertexArray.data(), meshVertexArray.size()* int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeFuseTriMesh(Fuse const *pFuse, Vector3d const&pos, QOpenGLBuffer &vboPanels, QOpenGLBuffer &vboEdges)
{
    int bufferSize = pFuse->nPanel3();
    bufferSize *=3;    // 3 vertex for each triangle
    bufferSize *=3;    // 3 components for each node

    QVector<float> meshVertexArray(bufferSize);
    int iv = 0;
    for(int it=0; it<pFuse->nPanel3(); it++)
    {
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            meshVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx).xf() + pos.xf();
            meshVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx).yf() + pos.yf();
            meshVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx).zf() + pos.zf();
        }
    }

    Q_ASSERT(meshVertexArray.size()==bufferSize);

    vboPanels.create();
    vboPanels.bind();
    vboPanels.allocate(meshVertexArray.data(),bufferSize *  int(sizeof(GLfloat)));
    //    int nTriangles = m_vboPartEditTriMesh.size()/4/3/sizeof(float); // three vertices and three components

    vboPanels.release();



    bufferSize = pFuse->nPanel3();
    bufferSize *= 3;    // 3 edges
    bufferSize *= 2;    // 2 nodes/edge
    bufferSize *= 3;    // 3 components for each node

    QVector<float> EdgeVertexArray(bufferSize);
    iv = 0;
    for(int it=0; it<pFuse->nPanel3(); it++)
    {
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            EdgeVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx).xf()   + pos.xf();
            EdgeVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx).yf()   + pos.yf();
            EdgeVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx).zf()   + pos.zf();
            EdgeVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx+1).xf() + pos.xf();
            EdgeVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx+1).yf() + pos.yf();
            EdgeVertexArray[iv++] = pFuse->panel3At(it).vertexAt(ivtx+1).zf() + pos.zf();
        }
    }

    Q_ASSERT(EdgeVertexArray.size()==bufferSize);

    vboEdges.create();
    vboEdges.bind();
    vboEdges.allocate(EdgeVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    vboEdges.release();
}


void gl::makeFuseXflFrames(const FuseXfl *pFuseXfl, Vector3d const &pos, int , int NHOOOP, QOpenGLBuffer &vbo)
{
    if(!pFuseXfl) return;
    Vector3d pt0, pt1;

    //OUTLINE
    // outline:
    //     frameSize(): frames
    //     Nhoop: segs on one side
    //      x2 sides
    //      x2 vertex/segment
    //      x3 coordinates/vertex
    //
    int nsegs = pFuseXfl->frameCount() * NHOOOP * 2 ;
    int outlinesize =   nsegs*2*3; // 6 components

    QVector<float> OutlineVertexArray(outlinesize);

    double hinc=1./double(NHOOOP);
    double u=0.0, v0=0.0, v1=0.0;

    int iv=0;
    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pFuseXfl->frameCount(); iFr++)
    {
        u = pFuseXfl->getu(pFuseXfl->frameAt(iFr).position().x);
        for (int j=0; j<NHOOOP; j++)
        {
            v0 = double(j)*hinc;
            pFuseXfl->getPoint(u,v0,true, pt0);
            v1 = double(j+1)*hinc;
            pFuseXfl->getPoint(u,v1,true, pt1);

            // left segment
            OutlineVertexArray[iv++] =  pt0.xf()+pos.xf();
            OutlineVertexArray[iv++] = -pt0.yf()+pos.yf();
            OutlineVertexArray[iv++] =  pt0.zf()+pos.zf();

            OutlineVertexArray[iv++] =  pt1.xf()+pos.xf();
            OutlineVertexArray[iv++] = -pt1.yf()+pos.yf();
            OutlineVertexArray[iv++] =  pt1.zf()+pos.zf();

            // right segment
            OutlineVertexArray[iv++] = pt0.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt0.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt0.zf()+pos.zf();

            OutlineVertexArray[iv++] = pt1.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt1.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt1.zf()+pos.zf();
        }
    }

    Q_ASSERT(iv==outlinesize);

    pFuseXfl = nullptr;

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinesize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeFuseXflSections(const FuseSections *pFuseSecs, Vector3d const &pos, int , int NHOOOP, QOpenGLBuffer &vbo)
{
    if(!pFuseSecs) return;

//    QVector<Vector3d> T(int(NXXXX+1)*int(NHOOOP+1)); //temporary points to save calculation times for body NURBS surfaces

    Vector3d pt0, pt1;

    //OUTLINE
    // outline:
    //     frameSize(): frames
    //     Nhoop: segs on one side
    //      x2 sides
    //      x2 vertex/segment
    //      x3 coordinates/vertex
    //
    int nsegs = pFuseSecs->nSections() * NHOOOP * 2 ;
    int outlinesize =   nsegs*2*3; // 6 components

    QVector<float> OutlineVertexArray(outlinesize);

    double hinc=1./double(NHOOOP);
    double u=0.0, v0=0.0, v1=0.0;

    int iv=0;
    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pFuseSecs->nSections(); iFr++)
    {
        std::vector<Vector3d> const &section = pFuseSecs->sectionAt(iFr);
        u = pFuseSecs->getu(section.front().x);
        for (int j=0; j<NHOOOP; j++)
        {
            v0 = double(j)*hinc;
            pFuseSecs->getPoint(u,v0,true, pt0);
            v1 = double(j+1)*hinc;
            pFuseSecs->getPoint(u,v1,true, pt1);

            // left segment
            OutlineVertexArray[iv++] =  pt0.xf()+pos.xf();
            OutlineVertexArray[iv++] = -pt0.yf()+pos.yf();
            OutlineVertexArray[iv++] =  pt0.zf()+pos.zf();

            OutlineVertexArray[iv++] =  pt1.xf()+pos.xf();
            OutlineVertexArray[iv++] = -pt1.yf()+pos.yf();
            OutlineVertexArray[iv++] =  pt1.zf()+pos.zf();

            // right segment
            OutlineVertexArray[iv++] = pt0.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt0.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt0.zf()+pos.zf();

            OutlineVertexArray[iv++] = pt1.xf()+pos.xf();
            OutlineVertexArray[iv++] = pt1.yf()+pos.yf();
            OutlineVertexArray[iv++] = pt1.zf()+pos.zf();
        }
    }

    Q_ASSERT(iv==outlinesize);

    pFuseSecs = nullptr;

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinesize * int(sizeof(GLfloat)));
    vbo.release();
}

/**
 * makes a vbo of the body surface using two half triangles for each quad face
 */
void gl::makeBodyFlatFaces_2triangles_outline(FuseXfl const *pBody, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    Vector3d P1, P2, P3, P4, Tj, Tjp1;

    //outline

    int outlinebuffersize = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); // quads
    outlinebuffersize *= 5; // five vertices
    outlinebuffersize *= 3; // 3 components/vertex
    outlinebuffersize *= 2; // x2 sides
    QVector<float> OutlineVertexArray(outlinebuffersize);

    int iv = 0;
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frameAt(j).position().x,     0.0, 0.0);
            Tjp1.set(pBody->frameAt(j+1).position().x, 0.0, 0.0);

            P1 = pBody->frameAt(j).ctrlPointAt(k);       P1.x = pBody->frameAt(j).position().x;
            P2 = pBody->frameAt(j+1).ctrlPointAt(k);     P2.x = pBody->frameAt(j+1).position().x;
            P3 = pBody->frameAt(j+1).ctrlPointAt(k+1);   P3.x = pBody->frameAt(j+1).position().x;
            P4 = pBody->frameAt(j).ctrlPointAt(k+1);     P4.x = pBody->frameAt(j).position().x;

            //right side quad
            OutlineVertexArray[iv++] = P1.xf()+pos.xf();
            OutlineVertexArray[iv++] = P1.yf()+pos.yf();
            OutlineVertexArray[iv++] = P1.zf()+pos.zf();
            OutlineVertexArray[iv++] = P2.xf()+pos.xf();
            OutlineVertexArray[iv++] = P2.yf()+pos.yf();
            OutlineVertexArray[iv++] = P2.zf()+pos.zf();
            OutlineVertexArray[iv++] = P3.xf()+pos.xf();
            OutlineVertexArray[iv++] = P3.yf()+pos.yf();
            OutlineVertexArray[iv++] = P3.zf()+pos.zf();
            OutlineVertexArray[iv++] = P4.xf()+pos.xf();
            OutlineVertexArray[iv++] = P4.yf()+pos.yf();
            OutlineVertexArray[iv++] = P4.zf()+pos.zf();
            OutlineVertexArray[iv++] = P1.xf()+pos.xf();
            OutlineVertexArray[iv++] = P1.yf()+pos.yf();
            OutlineVertexArray[iv++] = P1.zf()+pos.zf();

            //left side quad
            P1.y = -P1.y;
            P2.y = -P2.y;
            P3.y = -P3.y;
            P4.y = -P4.y;
            OutlineVertexArray[iv++] = P1.xf()+pos.xf();
            OutlineVertexArray[iv++] = P1.yf()+pos.yf();
            OutlineVertexArray[iv++] = P1.zf()+pos.zf();
            OutlineVertexArray[iv++] = P2.xf()+pos.xf();
            OutlineVertexArray[iv++] = P2.yf()+pos.yf();
            OutlineVertexArray[iv++] = P2.zf()+pos.zf();
            OutlineVertexArray[iv++] = P3.xf()+pos.xf();
            OutlineVertexArray[iv++] = P3.yf()+pos.yf();
            OutlineVertexArray[iv++] = P3.zf()+pos.zf();
            OutlineVertexArray[iv++] = P4.xf()+pos.xf();
            OutlineVertexArray[iv++] = P4.yf()+pos.yf();
            OutlineVertexArray[iv++] = P4.zf()+pos.zf();
            OutlineVertexArray[iv++] = P1.xf()+pos.xf();
            OutlineVertexArray[iv++] = P1.yf()+pos.yf();
            OutlineVertexArray[iv++] = P1.zf()+pos.zf();
        }
    }
    Q_ASSERT(iv==outlinebuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinebuffersize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeQuadNodeClrMap(std::vector<Panel4> const &panel4list, const std::vector<double> &data,
                            double &lmin, double &lmax, bool bAuto, QOpenGLBuffer &vbo)
{
    if(panel4list.size()<=0 || panel4list.size()>data.size())
    {
        vbo.destroy();
        return;
    }

    float tau=0;

    float coef = 1.0f;

    if(bAuto)
    {
        // find min and max Cp for scale set
        lmin =  1000000.0;
        lmax = -1000000.0;
        for (uint i4=0; i4<panel4list.size(); i4++)
        {
            int index = panel4list.at(i4).index();
            for(int ivtx=0; ivtx<3; ivtx++)
            {
                lmin = std::min(lmin, data.at(index)*coef);
                lmax = std::max(lmax, data.at(index)*coef);
            }
        }
    }


    float range = lmax - lmin;

    // vertices array size:
    //        nPanels
    //      x2 triangles per panels
    //      x3 nodes per triangle
    //        x6 = 3 vertex components + 3 color components

    int nodeVertexSize = int(panel4list.size()) * 2 * 3 * 6;
    QVector<float> nodeVertexArray(nodeVertexSize);

    Vector3d TS[2][3];  // two triangles with three vertices

    QColor clr;
    int iv=0;
    for (uint i4=0; i4<panel4list.size(); i4++)
    {
        int index = panel4list.at(i4).index();
        // each quad is two triangles
        //first triangle
        TS[0][0].copy(panel4list.at(i4).m_Node[1]);
        TS[0][1].copy(panel4list.at(i4).m_Node[2]);
        TS[0][2].copy(panel4list.at(i4).m_Node[0]);

        //second triangle
        TS[1][0].copy(panel4list.at(i4).m_Node[3]);
        TS[1][1].copy(panel4list.at(i4).m_Node[0]);
        TS[1][2].copy(panel4list.at(i4).m_Node[2]);

        for(int it=0; it<2; it++)
        {
            //each triangle is made of three vertices
            for(int i=0; i<3; i++)
            {
                nodeVertexArray[iv++] = TS[it][i].xf();
                nodeVertexArray[iv++] = TS[it][i].yf();
                nodeVertexArray[iv++] = TS[it][i].zf();
                tau = (float(data.at(index))*coef-lmin)/range;
                clr = ColourLegend::colour(tau);

/*                nodeVertexArray[iv++] = ColourLegend::GLGetRed(tau);
                nodeVertexArray[iv++] = ColourLegend::GLGetGreen(tau);
                nodeVertexArray[iv++] = ColourLegend::GLGetBlue(tau);*/
                nodeVertexArray[iv++] = clr.redF();
                nodeVertexArray[iv++] = clr.greenF();
                nodeVertexArray[iv++] = clr.blueF();
            }
        }
    }

    Q_ASSERT(iv==nodeVertexSize);
    Q_ASSERT(iv==int(panel4list.size())*2*3*6);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriangleNodeValues(QVector<Panel3> const &panel3, QVector<Node> const &NodeList, QVector<double> const &tab,
                            QVector<double> &CpInf, QVector<double> &CpSup, QVector<double> &Cp100,
                            double &lmin, double &lmax)
{
    lmin =  1.e10;
    lmax = -1.e10;
    int nPanel3 = panel3.size();
    int nNodes = NodeList.size();
    CpInf.resize(nNodes);
    CpSup.resize(nNodes);
    Cp100.resize(nNodes);

    double valmin=1e10, valmax=-1e10;
    for (int n=0; n<NodeList.size(); n++)
    {
        int averageInf=0, averageSup=0, average100=0;
        CpInf[n] = CpSup[n] = Cp100[n] = 0.0;
        for (int i3=0; i3<nPanel3; i3++)
        {
            Panel3 const &p3 = panel3.at(i3);
            int index = p3.index();
            int iNode=-1;

            if     (p3.nodeIndex(0)==n) iNode=0;
            else if(p3.nodeIndex(1)==n) iNode=1;
            else if(p3.nodeIndex(2)==n) iNode=2;
            if(iNode>=0 && 3*index+iNode<tab.size())
            {
                if(p3.isTopPanel())
                {
                    CpSup[n] += tab[3*index+iNode];
                    averageSup++;
                }
                else if(p3.isBotPanel() || p3.isMidPanel())
                {
                    CpInf[n] += tab[3*index+iNode];
                    averageInf++;
                }
                else if(p3.isFusePanel())
                {
                    Cp100[n] += tab[3*index+iNode];
                    average100++;
                }
                valmin = std::min(valmin, tab[3*index+iNode]);
                valmax = std::max(valmax, tab[3*index+iNode]);
            }
         }
        if(averageSup>0)
        {
            CpSup[n] /= averageSup;
            lmin = std::min(CpSup[n], lmin);
            lmax = std::max(CpSup[n], lmax);
        }
        if(averageInf>0)
        {
            CpInf[n] /= averageInf;
            lmin = std::min(CpInf[n], lmin);
            lmax = std::max(CpInf[n], lmax);
        }
        if(average100>0)
        {
            Cp100[n] /= average100;
            lmin = std::min(Cp100[n], lmin);
            lmax = std::max(Cp100[n], lmax);
        }
    }
//    qDebug("makeTriangleNodeValues:: minval=%17g   maxval=%17g", valmin, valmax);
}


void gl::makeTriUniColorMap(std::vector<Panel3> const &panel3list, std::vector<double> const &tab,
                            double &lmin, double &lmax, bool bAuto,
                            QOpenGLBuffer &vbo)
{
    if(tab.size()<panel3list.size()*3)
    {
        vbo.destroy();
        return;
    }

    int nPanel3 = int(panel3list.size());
    float tau =0;
    float range=0;
    float coef = 1.0;

    if(bAuto)
    {
        lmin =  1000000.0;
        lmax = -1000000.0;
        for (int i3=0; i3<nPanel3; i3++)
        {
            int index = panel3list.at(i3).index();
            for(int ivtx=0; ivtx<3; ivtx++)
            {
                lmin = std::min(lmin, tab[index*3+ivtx]*coef);
                lmax = std::max(lmax, tab[index*3+ivtx]*coef);
            }
        }
    }

    range = lmax - lmin;

    // vertices array size:
    //        n triangular Panels
    //      x3 nodes per triangle
    //        x6 = 3 vertex components + 3 color components

    int nodeVertexSize = nPanel3* 3 * 6;
    QVector<float> nodeVertexArray(nodeVertexSize, 0.0f);
    QColor clr;
    int iv=0;
    for (int i3=0; i3<nPanel3; i3++)
    {
        Panel3 const &p3 = panel3list.at(i3);
        int index = p3.index();
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            nodeVertexArray[iv++] = p3.vertexAt(ivtx).xf();
            nodeVertexArray[iv++] = p3.vertexAt(ivtx).yf();
            nodeVertexArray[iv++] = p3.vertexAt(ivtx).zf();

            tau  = (float(tab[index*3+ivtx])*coef-lmin) / range;

            clr = ColourLegend::colour(tau);
            nodeVertexArray[iv++] = clr.redF();
            nodeVertexArray[iv++] = clr.greenF();
            nodeVertexArray[iv++] = clr.blueF();
        }
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriLinColorMap(std::vector<Panel3> const &panel3, std::vector<Node> const &NodeList,
                            std::vector<double> const &Cp,
                            double lmin, double lmax,
                            QOpenGLBuffer &vbo)
{
    float tau=0;

    int nPanel3 = int(panel3.size());
    int nNodes = int(NodeList.size());

    double range = lmax - lmin;

    // vertices array size:
    //        n triangular Panels
    //      x3 nodes per triangle
    //        x6 = 3 vertex components + 3 color components

    int nodeVertexSize = nPanel3* 3 * 6;
    QVector<float> nodeVertexArray(nodeVertexSize, 0.0f);
    QColor clr;
    int iv=0;
    for(int p=0; p<nPanel3; p++)
    {
        Panel3 const &p3 = panel3.at(p);

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = p3.vertexAt(i).xf();
            nodeVertexArray[iv++] = p3.vertexAt(i).yf();
            nodeVertexArray[iv++] = p3.vertexAt(i).zf();

            if(0<=p3.nodeIndex(i) && p3.nodeIndex(i)<nNodes)
            {
                tau = float((Cp[p3.nodeIndex(i)]-lmin)/range);
            }
            else
            {
                tau = Qt::black;
            }
            clr = ColourLegend::colour(tau);
            nodeVertexArray[iv++] = clr.redF();
            nodeVertexArray[iv++] = clr.greenF();
            nodeVertexArray[iv++] = clr.blueF();
        }
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


/** Implementation of the "marching triangles" algorithm.
 *  https://en.wikipedia.org/wiki/Marching_squares#Contouring_triangle_meshes
*/
void gl::makeTriangleContoursOnMesh(std::vector<Panel3> const &panel3list,
                                    std::vector<double> const &Cp,
                                    double lmin, double lmax,
                                    QOpenGLBuffer &vbo)
{
    if(panel3list.size()<1)     {vbo.destroy();  return;}

    float range = lmax - lmin;

    //define the threshold values for the contours
    int nContours = W3dPrefs::s_NContourLines;

    // nContours+1 to skip useless min and max iso-contours
    QVector<float> contour(nContours);
//    qDebug("      lmin = %17g", lmin);
    for(int ic=0; ic<nContours; ic++)
    {
        contour[ic] = lmin + float(ic+1)/float(nContours+1)*range;
//        qDebug("Contour_%2d = %17g", ic, contour[ic]);
    }
//    qDebug("      lmax = %17g\n", lmax);

    QVector<Segment3d> segs;

    Vector3d I0, I1;// crossover points on edge
    for(int ic=0; ic<nContours; ic++)
    {
        double threshold = contour.at(ic);
        for(uint t3=0; t3<panel3list.size(); t3++)
        {
            Panel3 const &p3 = panel3list.at(t3);

            // check for crossover of contour value
            // use base 2 key as table index

            double tau = 0.0;
            int i0=-1, i1=-1, i2=-1, i3=-1;
            int key = 0;
            int k=1;
            double val[]= {0,0,0};
            for(int iVtx=0; iVtx<3; iVtx++)
            {
                val[iVtx] = Cp[p3.nodeIndex(iVtx)];

                if(val[iVtx]-threshold<0) // true if there is a crossover
                {
                    key += k;
                }
                k *= 2;
            }

            gl::lookUpTriangularKey(key, i0, i1, i2, i3);

            if(i0>=0 && i1>=0 && i2>=0 && i3>=0)
            {
                tau = (threshold - val[i0]) /(val[i1]-val[i0]);
                I0 = p3.vertexAt(i0)*(1-tau) + p3.vertexAt(i1)*tau;

                tau = (threshold - val[i2]) /(val[i3]-val[i2]);
                I1 = p3.vertexAt(i2)*(1-tau) + p3.vertexAt(i3)*tau;

                segs.push_back({I0, I1});
            }
        }
    }

    // vertex array size
    // nsegs
    // x 2 vertices
    // x 3 components
    int nodeVertexSize = segs.size() * 2 * 3;
    QVector<float> nodeVertexArray(nodeVertexSize);

    int iv=0;
    for(int is=0; is<segs.size(); is++)
    {
        Node const & n0 = segs.at(is).vertexAt(0);
        Node const & n1 = segs.at(is).vertexAt(1);
        nodeVertexArray[iv++] = n0.xf();
        nodeVertexArray[iv++] = n0.yf();
        nodeVertexArray[iv++] = n0.zf();
        nodeVertexArray[iv++] = n1.xf();
        nodeVertexArray[iv++] = n1.yf();
        nodeVertexArray[iv++] = n1.zf();
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makePanelForces(std::vector<Panel4> const &panel4list,
                         const std::vector<double> &Cp, float qDyn, bool bVLM,
                         double &rmin, double &rmax, bool bAuto, double scale,
                         QOpenGLBuffer &vbo)
{
    if( panel4list.size()==0 || panel4list.size()>Cp.size())
    {
        vbo.destroy();
        return;
    }

    Quaternion Qt; // Quaternion operator to align the reference arrow to the panel's normal
    Vector3d Omega; // rotation vector to align the reference arrow with the panel's normal
    Vector3d O;
    //The vectors defining the reference arrow
    Vector3d R(0.0,0.0,1.0);
    Vector3d R1( 0.05, 0.0, -0.1);
    Vector3d R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    //define the range of values to set the colors in accordance
    float coef = 0.00001f;

    if(bAuto)
    {
        rmin = 1.e10;
        rmax = -rmin;
        for (uint i4=0; i4<panel4list.size(); i4++)
        {
            int index = panel4list.at(i4).index();
            rmax = std::max(rmax, Cp.at(index));
            rmin = std::min(rmin, Cp.at(index));
        }
        rmin *= qDyn; // *dynamic pressure
        rmax *= qDyn;
    }

    float range = rmax - rmin;

    // vertices array size:
    //        nPanels x 1 arrow
    //      x3 lines per arrow
    //      x2 vertices per line
    //        x6 = 3 vertex components + 3 color components

    int forceVertexSize = int(panel4list.size()) * 3 * 2 * 6;
    QVector<float> forceVertexArray(forceVertexSize);

    int iv=0;
    for(uint p=0; p<panel4list.size(); p++)
    {
        Panel4 const &p4 = panel4list.at(p);
        int index = p4.index();

        float force = qDyn * float(Cp.at(index));
        float tau = (force-rmin)/range;

        //scale force for display
        force *= scale *coef;

        QColor clr = ColourLegend::colour(tau);

        float r = clr.redF();
        float g = clr.greenF();
        float b = clr.blueF();

        O.set(p4.ctrlPt(bVLM));

        // Rotate the reference arrow to align it with the panel normal
        if(R.isSame(P))
        {
            Qt.set(0.0, 0.0,0.0,1.0); //Null quaternion
        }
        else
        {
            float cosa   = float(R.dot(p4.normal()));
            float sina2  = sqrtf((1.0f - cosa)*0.5f);
            float cosa2  = sqrtf((1.0f + cosa)*0.5f);

            Omega = R * p4.normal();//crossproduct
            Omega.normalize();
            Omega *= double(sina2);
            Qt.set(double(cosa2), Omega.x, Omega.y, Omega.z);
        }

        Qt.conjugate(R,  P);
        Qt.conjugate(R1, P1);
        Qt.conjugate(R2, P2);

        // Scale the pressure vector
        P  *= double(force);
        P1 *= double(force);
        P2 *= double(force);
        if(Cp.at(index)>0 && !p4.isMidPanel())
        {
            // compression, point towards the surface
            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()-P1.xf();
            forceVertexArray[iv++] = O.yf()-P1.yf();
            forceVertexArray[iv++] = O.zf()-P1.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()-P2.xf();
            forceVertexArray[iv++] = O.yf()-P2.yf();
            forceVertexArray[iv++] = O.zf()-P2.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
        }
        else
        {
            // depression, point outwards from the surface
            P.set(-P.x, -P.y, -P.z);

            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf()-P1.xf();
            forceVertexArray[iv++] = O.yf()+P.yf()-P1.yf();
            forceVertexArray[iv++] = O.zf()+P.zf()-P1.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf()-P2.xf();
            forceVertexArray[iv++] = O.yf()+P.yf()-P2.yf();
            forceVertexArray[iv++] = O.zf()+P.zf()-P2.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
        }

    }
    Q_ASSERT(iv==forceVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(forceVertexArray.data(), forceVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makePanelForces(std::vector<Panel3> const &panel3list, std::vector<double> const &Cp,
                         float qDyn,
                         double &rmin, double &rmax, bool bAuto, double scale,
                         QOpenGLBuffer &vbo)
{
    int nPanel3 = int(panel3list.size());
    if(nPanel3==0 || nPanel3>=int(Cp.size()))
    {
        vbo.destroy();
        return;
    }

    Quaternion Qt; // Quaternion operator to align the reference arrow with the panel's normal
    Vector3d O;
    //The vectors defining the reference arrow
    Vector3d const Ra(0.0,0.0,1.0);
    Vector3d const R1( 0.05, 0.0, -0.1);
    Vector3d const R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    //define the range of values to set the colors in accordance

    float coef = 0.00001f;

    if(bAuto)
    {
        rmin = 1.e10;
        rmax = -rmin;
        for (int i3=0; i3<nPanel3; i3++)
        {
            int index = panel3list.at(i3).index();
            if(index*3<int(Cp.size()))
            {
                rmax = std::max(rmax, Cp[index*3]);
                rmin = std::min(rmin, Cp[index*3]);
            }
        }
        rmin *= qDyn;
        rmax *= qDyn;
    }

    float range = rmax - rmin;

    // vertices array size:
    //        nPanels x 1 arrow
    //      x3 lines per arrow
    //      x2 vertices per line
    //        x6 = 3 vertex components + 3 color components

    int forceVertexSize = nPanel3 * 3 * 2 * 6;
    QVector<float> forceVertexArray(forceVertexSize);
    QColor clr;
    int iv=0;
    for (int p=0; p<nPanel3; p++)
    {
        Panel3 const &p3 = panel3list.at(p);
        if(p3.index()*3>=int(Cp.size())) continue;

        float force = qDyn * float(Cp[p3.index()*3]);// * p3.area();
        float tau = (force-rmin)/range;
        //scale force for display
        force *= scale *coef;

        clr = ColourLegend::colour(tau);
        float r = clr.redF();
        float g = clr.greenF();
        float b = clr.blueF();

        O = p3.CoG();

        if(fabs(Ra.dot(p3.normal())-1.0)<0.0001)
        {
            P = Ra; P1=R1; P2=R2;
        }
        else if(fabs(Ra.dot(p3.normal())+1.0)<0.0001)
        {
            P = Ra*-1.0; P1=R1*-1.0; P2=R2*-1.0;
        }
        else
        {
            Qt.from2UnitVectors(Ra, p3.normal());
            Qt.conjugate(Ra,  P);
            Qt.conjugate(R1, P1);
            Qt.conjugate(R2, P2);
        }

        // Scale the pressure vector
        P  *= double(force);
        P1 *= double(force);
        P2 *= double(force);

        if(Cp[p3.index()*3]>0 && !p3.isMidPanel())
        {
            // compression, point towards the surface
            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()-P1.xf();
            forceVertexArray[iv++] = O.yf()-P1.yf();
            forceVertexArray[iv++] = O.zf()-P1.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()-P2.xf();
            forceVertexArray[iv++] = O.yf()-P2.yf();
            forceVertexArray[iv++] = O.zf()-P2.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
        }
        else
        {
            // depression, point outwards from the surface
            P.set(-P.x, -P.y, -P.z);

            forceVertexArray[iv++] = O.xf();
            forceVertexArray[iv++] = O.yf();
            forceVertexArray[iv++] = O.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf()-P1.xf();
            forceVertexArray[iv++] = O.yf()+P.yf()-P1.yf();
            forceVertexArray[iv++] = O.zf()+P.zf()-P1.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = O.xf()+P.xf();
            forceVertexArray[iv++] = O.yf()+P.yf();
            forceVertexArray[iv++] = O.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = O.xf()+P.xf()-P2.xf();
            forceVertexArray[iv++] = O.yf()+P.yf()-P2.yf();
            forceVertexArray[iv++] = O.zf()+P.zf()-P2.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
        }
    }

//    Q_ASSERT(iv==forceVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(forceVertexArray.data(), forceVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


/** triangles for the surface shader */
void gl::makeVortons_spheres(std::vector<std::vector<Vorton>> const &Vortons, QOpenGLBuffer &vbo)
{
    if(!Vortons.size())
    {
        vbo.destroy();
        return;
    }

    // count the active vortons
    int nVortons = 0;
    for(uint irow=0; irow<Vortons.size(); irow++)
    {
        std::vector<Vorton> const &vtnrow = Vortons.at(irow);
        for(uint jv=0; jv<vtnrow.size(); jv++)
        {
            if(vtnrow.at(jv).isActive()) nVortons++;
        }
    }

    // make vertices
    std::vector<Triangle3d> icotriangles;
    geom::makeSphere(W3dPrefs::s_VortonRadius, 1, icotriangles);

    //Make the triangulation
    int bufferSize = nVortons * int(icotriangles.size());
    bufferSize *=4;    // 4 vertices for each triangle
    bufferSize *=6;    // (3 coords+3 normal components) for each node

    QVector<float> meshvertexarray(bufferSize);
    bool bFlatNormals = true;
    Vector3d N;

    int iv = 0;

    for(uint irow=0; irow<Vortons.size(); irow++)
    {
        std::vector<Vorton> const &vtnrow = Vortons.at(irow);
        for(uint jv=0; jv<vtnrow.size(); jv++)
        {
            Vorton const &vorton = vtnrow.at(jv);
            if(!vorton.isActive()) continue;

            for(uint it=0; it<icotriangles.size(); it++)
            {
                Triangle3d const &t3d = icotriangles.at(it);
                N.set(t3d.normal());

                meshvertexarray[iv++] = t3d.vertexAt(0).xf() + vorton.position().xf();
                meshvertexarray[iv++] = t3d.vertexAt(0).yf() + vorton.position().yf();
                meshvertexarray[iv++] = t3d.vertexAt(0).zf() + vorton.position().zf();
                if(bFlatNormals)
                {
                    meshvertexarray[iv++] = N.xf();
                    meshvertexarray[iv++] = N.yf();
                    meshvertexarray[iv++] = N.zf();
                }
                else
                {
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().xf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().yf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().zf();
                }

                meshvertexarray[iv++] = t3d.vertexAt(1).xf() + vorton.position().xf();
                meshvertexarray[iv++] = t3d.vertexAt(1).yf() + vorton.position().yf();
                meshvertexarray[iv++] = t3d.vertexAt(1).zf() + vorton.position().zf();
                if(bFlatNormals)
                {
                    meshvertexarray[iv++] = N.xf();
                    meshvertexarray[iv++] = N.yf();
                    meshvertexarray[iv++] = N.zf();
                }
                else
                {
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().xf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().yf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().zf();
                }

                meshvertexarray[iv++] = t3d.vertexAt(2).xf() + vorton.position().xf();
                meshvertexarray[iv++] = t3d.vertexAt(2).yf() + vorton.position().yf();
                meshvertexarray[iv++] = t3d.vertexAt(2).zf() + vorton.position().zf();
                if(bFlatNormals)
                {
                    meshvertexarray[iv++] = N.xf();
                    meshvertexarray[iv++] = N.yf();
                    meshvertexarray[iv++] = N.zf();
                }
                else
                {
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().xf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().yf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().zf();
                }

                //close the triangle
                meshvertexarray[iv++] = t3d.vertexAt(0).xf() + vorton.position().xf();
                meshvertexarray[iv++] = t3d.vertexAt(0).yf() + vorton.position().yf();
                meshvertexarray[iv++] = t3d.vertexAt(0).zf() + vorton.position().zf();
                if(bFlatNormals)
                {
                    meshvertexarray[iv++] = N.xf();
                    meshvertexarray[iv++] = N.yf();
                    meshvertexarray[iv++] = N.zf();
                }
                else
                {
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().xf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().yf();
                    meshvertexarray[iv++] = t3d.vertexAt(0).normal().zf();
                }

            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


/** For instance rendering*/
void gl::makeVortons(std::vector<std::vector<Vorton>> const &vortons, QOpenGLBuffer &vbo)
{
    if(!vortons.size() || !vortons.front().size())
    {
        vbo.destroy();
        return;
    }

    bool bMidRowOnly = false;
    int nVortons = 0;
    if(bMidRowOnly) nVortons = int(vortons.front().size());
    else            nVortons = int(vortons.size()*vortons.front().size());
    int buffersize = nVortons*3;

    QVector<float> pts(buffersize);
    int iv =0;
    for(uint i=0; i<vortons.size(); i++)
    {
        std::vector<Vorton> const& vortonrow = vortons.at(i);
        if(bMidRowOnly)
        {
            if(i!=vortons.size()/2) continue;
        }

        for(uint j=0; j<vortonrow.size(); j++)
        {
            Vorton const &vorton = vortonrow.at(j);
            pts[iv++] = vorton.position().xf();
            pts[iv++] = vorton.position().yf();
            pts[iv++] = vorton.position().zf();
        }
    }

    if(vbo.isCreated()) vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
    vbo.release();
}
