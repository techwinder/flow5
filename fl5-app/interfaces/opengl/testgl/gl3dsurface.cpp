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

#include "gl3dsurface.h"


#include <interfaces/opengl/controls/colourlegend.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>



double c0 = 0.29;
double c1 = 0.22;
double c2 = -0.43;

gl3dSurface::gl3dSurface(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_bGrid = true;
    m_bContour = false;

    m_ValMin = 1e10;
    m_ValMax = -1e10;
    m_HalfSide = 5.0;
    m_Size_x = 23;
    m_Size_y = 23;
    m_PointArray.resize(m_Size_x*m_Size_y);

    m_bResetSurface = true;
    m_bDoubleDipSurface = true;
    m_bDisplaySurface = false;

}


void gl3dSurface::glRenderView()
{
    if(m_bDisplaySurface)
    {
        paintColourMap(m_vboSurface);
        if(m_bGrid) paintGrid();
        if(m_bContour)
            paintSegments(m_vboContourLines, W3dPrefs::s_ContourLineStyle);
    }
}


void gl3dSurface::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_F2:
            m_bDisplaySurface = !m_bDisplaySurface;
            update();
            break;
/*        case Qt::Key_Escape:
            QTimer::singleShot(0, this, SLOT(close()));
            break;*/
        case Qt::Key_W:
            if(bCtrl)
                QTimer::singleShot(0, this, SLOT(close()));
            break;
        default:
            gl3dTestGLView::keyPressEvent(pEvent);
            break;
    }
}


void gl3dSurface::glMake3dObjects()
{
    if(m_bResetSurface)
    {
        glMakeSurface();
        QVector<double> nodevalues(m_PointArray.size());
        for(int i=0; i<m_PointArray.size(); i++) nodevalues[i] = m_PointArray.at(i).z;
        gl::makeQuadContoursOnGrid(m_vboContourLines, m_Size_x, m_Size_y, m_PointArray, nodevalues, true);
    }
    m_bResetSurface = false;
}


void gl3dSurface::glMakeSurface()
{
    // (Sizex-1) x (Sizey-1) quads x2 triangles
    // x3 vertex
    // x (3 vtx + 3 normal)/vertex
    int bufferSize = (m_Size_x-1)*(m_Size_y-1)* 2 * 3 * 6;

    QVector<float>surfvertexarray(bufferSize);

    float zmin= float(LARGEVALUE);
    float zmax=-float(LARGEVALUE);
    for(int i=0; i<m_Size_x*m_Size_y; i++)
    {
        zmin = std::min(zmin, m_PointArray[i].zf());
        zmax = std::max(zmax, m_PointArray[i].zf());
    }
    float range = zmax - zmin;
    range = std::max(range, 0.001f);
    QColor clr;
    int iv = 0;

    int iLA{0},iTA{0},iLB{0},iTB{0};
    float tau{0};
    for (int i=0; i<m_Size_x-1; i++)
    {
        for(int j=0; j<m_Size_y-1; j++)
        {
            iTA =  i   *m_Size_y + j;
            iLA = (i+1)*m_Size_y + j;
            iTB =  i   *m_Size_y + j+1;
            iLB = (i+1)*m_Size_y + j+1;
            Vector3d const &TA =  m_PointArray[iTA];
            Vector3d const &LA =  m_PointArray[iLA];
            Vector3d const &TB =  m_PointArray[iTB];
            Vector3d const &LB =  m_PointArray[iLB];

            // first triangle
            surfvertexarray[iv++] = TB.xf();
            surfvertexarray[iv++] = TB.yf();
            surfvertexarray[iv++] = TB.zf();
            tau = (TB.zf()-zmin)/range;
            clr = ColourLegend::colour(tau);
            surfvertexarray[iv++] = clr.redF();
            surfvertexarray[iv++] = clr.greenF();
            surfvertexarray[iv++] = clr.blueF();

            surfvertexarray[iv++] = TA.xf();
            surfvertexarray[iv++] = TA.yf();
            surfvertexarray[iv++] = TA.zf();
            tau = (TA.zf()-zmin)/range;
            clr = ColourLegend::colour(tau);
            surfvertexarray[iv++] = clr.redF();
            surfvertexarray[iv++] = clr.greenF();
            surfvertexarray[iv++] = clr.blueF();

            surfvertexarray[iv++] = LA.xf();
            surfvertexarray[iv++] = LA.yf();
            surfvertexarray[iv++] = LA.zf();
            tau = (LA.zf()-zmin)/range;
            clr = ColourLegend::colour(tau);
            surfvertexarray[iv++] = clr.redF();
            surfvertexarray[iv++] = clr.greenF();
            surfvertexarray[iv++] = clr.blueF();


            //second triangle
            surfvertexarray[iv++] = LA.xf();
            surfvertexarray[iv++] = LA.yf();
            surfvertexarray[iv++] = LA.zf();
            tau = (LA.zf()-zmin)/range;
            clr = ColourLegend::colour(tau);
            surfvertexarray[iv++] = clr.redF();
            surfvertexarray[iv++] = clr.greenF();
            surfvertexarray[iv++] = clr.blueF();

            surfvertexarray[iv++] = LB.xf();
            surfvertexarray[iv++] = LB.yf();
            surfvertexarray[iv++] = LB.zf();
            tau = (LB.zf()-zmin)/range;
            clr = ColourLegend::colour(tau);
            surfvertexarray[iv++] = clr.redF();
            surfvertexarray[iv++] = clr.greenF();
            surfvertexarray[iv++] = clr.blueF();

            surfvertexarray[iv++] = TB.xf();
            surfvertexarray[iv++] = TB.yf();
            surfvertexarray[iv++] = TB.zf();
            tau = (TB.zf()-zmin)/range;
            clr = ColourLegend::colour(tau);
            surfvertexarray[iv++] = clr.redF();
            surfvertexarray[iv++] = clr.greenF();
            surfvertexarray[iv++] = clr.blueF();
        }
    }

    Q_ASSERT(iv==bufferSize);


    if(m_vboSurface.isCreated()) m_vboSurface.destroy();
    m_vboSurface.create();
    m_vboSurface.bind();
    m_vboSurface.allocate(surfvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboSurface.release();

    //Make the grid
    int nsegs = m_Size_x * (m_Size_y-1) + m_Size_y * (m_Size_x-1);
    bufferSize = nsegs * 2 * 3; // 3 vertex components

    QVector<float>gridvertexarray(bufferSize);
    iv=0;
    for (int i=0; i<m_Size_x; i++)
    {
        for(int j=0; j<m_Size_y-1; j++)
        {
            iTA = i*m_Size_y + j;
            iLA = i*m_Size_y + j+1;
            Vector3d const &TA =  m_PointArray[iTA];
            Vector3d const &LA =  m_PointArray[iLA];
            gridvertexarray[iv++] = TA.xf();
            gridvertexarray[iv++] = TA.yf();
            gridvertexarray[iv++] = TA.zf();

            gridvertexarray[iv++] = LA.xf();
            gridvertexarray[iv++] = LA.yf();
            gridvertexarray[iv++] = LA.zf();
        }
    }

    for(int j=0; j<m_Size_y; j++)
    {
        for (int i=0; i<m_Size_x-1; i++)
        {
            iTA =   i    *m_Size_y + j;
            iTB =  (i+1) *m_Size_y + j;
            Vector3d const &TA =  m_PointArray[iTA];
            Vector3d const &TB =  m_PointArray[iTB];
            gridvertexarray[iv++] = TA.xf();
            gridvertexarray[iv++] = TA.yf();
            gridvertexarray[iv++] = TA.zf();

            gridvertexarray[iv++] = TB.xf();
            gridvertexarray[iv++] = TB.yf();
            gridvertexarray[iv++] = TB.zf();
        }
    }
    Q_ASSERT(iv==bufferSize);

    if(m_vboGrid.isCreated()) m_vboGrid.destroy();
    m_vboGrid.create();
    m_vboGrid.bind();
    m_vboGrid.allocate(gridvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboGrid.release();

}


void gl3dSurface::paintGrid()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();

    LineStyle ls{true, Line::SOLID, 1, fl5Color(200,200,200), Line::NOSYMBOL};
    paintSegments(m_vboGrid, ls);
}


/**
 * z = ax + by + c
 */
void gl3dSurface::make3dPlane(double a, double b, double c)
{
    double x=0.0,y=0.0,z=0.0;
    double range = m_RefLength/2.0;
    //    m_RefLength = range/2.0;

    for(int i=0; i<m_Size_x; i++)
    {
        x = -range/2.0+ range*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -range/2.0+ range*double(j)/double(m_Size_y-1);

            z = a*x + b*y + c;

            m_RefLength = std::max(m_RefLength, z);
            m_PointArray[i*m_Size_y+j].set(x,y,z);
        }
    }
    setReferenceLength(m_RefLength);
    reset3dScale();
    m_bResetSurface = true;
    m_bDisplaySurface = true;
}


double gl3dSurface::makeTestSurface()
{
    m_Size_x = 73;
    m_Size_y = 73;

    m_PointArray.resize(m_Size_x*m_Size_y);

    Vector2d pt;
    double x=0,y=0;
    double z=0;
    double SideLength = 2*m_HalfSide;
    double mx = SideLength; // used to scale the 3d plot
    m_ValMin = 1e10;
    m_ValMax = -1e10;

    for(int i=0; i<m_Size_x; i++)
    {
        x = -SideLength/2.0+ SideLength*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -SideLength/2+ SideLength*double(j)/double(m_Size_y-1);

            z = function(x, y);

            m_PointArray[i*m_Size_y+j].set(x,y,z);
            m_ValMin = std::min(m_ValMin, z);
            m_ValMax = std::max(m_ValMax, z);

            mx = std::max(mx, fabs(m_PointArray[i*m_Size_y+j].z));
        }
    }

    m_bResetSurface = true;
    m_bDisplaySurface = true;
    return mx;
}


double gl3dSurface::error(Vector2d const &pos, bool bMinimum) const
{
    double z = function(pos.x, pos.y);
    if(bMinimum) return (z-m_ValMin) * (z-m_ValMin);
    else         return (z-m_ValMax) * (z-m_ValMax);
}


double gl3dSurface::function(double x, double y) const
{
    if(m_bDoubleDipSurface)
        return x * exp(-(x*x/m_HalfSide + y*y/m_HalfSide))*m_HalfSide/2.0;
    else
        return 1.0/ sqrt((0.5+x*x + y*y)/5.0) * (sin(c0*(0.5*x+y)+3.517) + cos((y+x-c1)*c2-1.57)*sin((y-2.0*x-c1)*c2));
}





