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




#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>
#include <QFormLayout>
#include <QTime>


#include "gl3dcontourview.h"


#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <api/geom_global.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>


int gl3dContourView::s_NRows(73);



gl3dContourView::gl3dContourView(QWidget *pParent) : gl3dXflView (pParent)
{
    setWindowTitle("3d Contour view");
    setAttribute(Qt::WA_DeleteOnClose);

    setReferenceLength(2.0);

    m_bResetView   = true;
    m_bShowSegment = true;
    m_bShowPoints  = true;

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Text, DisplayOptions::textColor());

        QColor clr = DisplayOptions::backgroundColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);
        pFrame->setCursor(Qt::ArrowCursor);
//        pFrame->setAutoFillBackground(true);
        pFrame->setPalette(palette);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        pFrame->setMinimumWidth(500);
        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QFormLayout*pParamsLayout = new QFormLayout;
            {
                m_pieNRows =  new IntEdit(s_NRows);
                connect(m_pieNRows, SIGNAL(intChanged(int)), SLOT(onChanged()));


                m_pcbPlaneDir = new QComboBox;
                m_pcbPlaneDir->addItems({"X", "Y", "Z"});
                connect(m_pcbPlaneDir, SIGNAL(activated(int)), SLOT(onChanged()));
                m_psPosition = new QSlider(Qt::Horizontal);
                m_psPosition->setRange(-100, 100);
                m_psPosition->setTickInterval(20);
                m_psPosition->setTickPosition(QSlider::TicksBelow);
                m_psPosition->setValue(0);
                connect(m_psPosition,  SIGNAL(sliderMoved(int)),  SLOT(onChanged()));

                pParamsLayout->addRow("NRows",          m_pieNRows);
                pParamsLayout->addRow("Plane normal",   m_pcbPlaneDir);
                pParamsLayout->addRow("Plane Position", m_psPosition);
            }

            pMainLayout->addLayout(pParamsLayout);
        }

        pFrame->setLayout(pMainLayout);
    }
}


void gl3dContourView::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_F2:
            onSetupLight();
            update();
            break;


        case Qt::Key_Escape:
        case Qt::Key_W:
            close();
            break;
        default:
            gl3dXflView::keyPressEvent(pEvent);
            break;
    }
}


void gl3dContourView::glRenderView()
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
    m_shadSurf.release();

    paintColourMap(m_vboClrMap);
    paintSegments(m_vboContours, W3dPrefs::s_ContourLineStyle);
}



void gl3dContourView::glMake3dObjects()
{
    if(m_bResetView)
    {
        s_NRows = m_pieNRows->value();
        int iDir = m_pcbPlaneDir->currentIndex()%3;
        double t = double(m_psPosition->value())/100.0;
        double pos = 2*t;

        int nrows = s_NRows;
        int ncols = s_NRows;
        double h = 2.5;
        double w = 2.5;
        double x(0),y(0),z(0);

        QVector<Vector3d> nodes(nrows*ncols);
        QVector<double> values(nrows*ncols);

        for(int ir=0; ir<nrows; ir++)
        {
            double r = (double(ir)/double(nrows-1)-0.5)*h;
            for(int ic=0; ic<ncols; ic++)
            {
                double c = (double(ic)/double(ncols-1)-0.5)*w;
                int idx = ir*ncols + ic;
                switch(iDir)
                {
                    default:
                    case 0:  x=pos; y=r;    z=c;        break;
                    case 1:  x=r;   y=pos;  z=c;        break;
                    case 2:  x=r;   y=c;    z=pos;      break;
                }

                nodes[idx] = Vector3d(x,y,z);
                values[idx] = 3.0* cos(1.5*(y+1))*(2.5*(z-0.5)) * sin((2.0*y-1.5*(z-1)-3.0*(x+1.0))) + 2.0*sin(2.5*z-y+(x+0.5));
            }
        }

        double lmin =  1.e10;
        double lmax = -1.e10;
        for(int i=0; i<values.size(); i++)
        {
            lmin = std::min(lmin, values.at(i));
            lmax = std::max(lmax, values.at(i));
        }

        gl::makeQuadColorMap(m_vboClrMap, nrows, ncols, nodes, values, lmin, lmax, false, true);
        glMakeTriangleContoursOnGrid(m_vboContours, nrows, ncols, nodes, values);

        m_bResetView = false;
    }
}

void gl3dContourView::onChanged()
{
    m_bResetView = true;
    update();
}



bool gl3dContourView::intersectTheObject(Vector3d const &, Vector3d const &, Vector3d &)
{
    return false;
}


void gl3dContourView::hideEvent(QHideEvent *)
{
    resetView();
}


void gl3dContourView::showEvent(QShowEvent *pEvent)
{

    gl3dXflView::showEvent(pEvent);
    reset3dScale();
}


void gl3dContourView::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dTestView");
    {
        s_NRows     = settings.value("NRows",     s_NRows).toInt();
    }
    settings.endGroup();
}


void gl3dContourView::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dTestView");
    {
        settings.setValue("NRows",     s_NRows);
    }
    settings.endGroup();
}



/** Implementation of the "marching triangles" algorithm.
 *  https://en.wikipedia.org/wiki/Marching_squares#Contouring_triangle_meshes
*/
void gl3dContourView::glMakeTriangleContoursOnGrid(QOpenGLBuffer &vbo, int nrows, int ncols,
                                                   QVector<Vector3d> const&node, QVector<double> const &value) const
{
    if(node.count()<4)            {vbo.destroy();  return;}
    if(node.size()!=nrows*ncols)  {vbo.destroy();  return;}
    if(value.size()!=nrows*ncols) {vbo.destroy();  return;}

    // find min and max Cp for scale set
    float lmin =  1000000.0f;
    float lmax = -1000000.0f;

    float coef = 1.0f;

    for(int p=0; p<value.count(); p++)
    {
        lmin = std::min(lmin, float(value.at(p))*coef);
        lmax = std::max(lmax, float(value.at(p))*coef);
    }

    float range = lmax - lmin;

    //define the threshold values for the contours
    int nContours = W3dPrefs::s_NContourLines;
    QVector<float> contour(nContours);
    for(int ic=0; ic<nContours; ic++) contour[ic] = lmin + float(ic)/float(nContours-1)*range;

    QVector<Segment3d> segs;
    int idx[] = {0,0,0};

    Vector3d I0, I1;// crossover points on edge
    for(int i=0; i<nContours; i++)
    {
        double threshold = contour.at(i);
        for(int r=0; r<nrows-1; r++)
        {
            for(int c=0; c<ncols-1; c++)
            {
                //
                //    N3------N2
                //    |     / |
                //    |   /   |
                //    | /     |
                //    N0------N1
                //
                // check for crossover of contour value
                // use base 2 key as table index

                double tau = 0.0;
                int i0=-1, i1=-1, i2=-1, i3=-1;

                // for each cell triangle
                for(int it=0; it<2; it++)
                {
                    if(it==0)
                    {
                        // triangle N0-N1-N2
                        idx[0] =  r   *ncols+c;
                        idx[1] =  r   *ncols+c+1;
                        idx[2] = (r+1)*ncols+c+1;
                    }
                    else
                    {
                        // triangle N0-N2-N3
                        idx[0] =  r   *ncols+c;
                        idx[1] = (r+1)*ncols+c+1;
                        idx[2] = (r+1)*ncols+c;
                    }

                    int key = 0;
                    int k=1;
                    for(int i=0; i<3; i++)
                    {
                        if(value[idx[i]]-threshold<0) // true if there is a crossover
                        {
                            key += k;
                        }
                        k *= 2;
                    }

                    gl::lookUpTriangularKey(key, i0, i1, i2, i3);

                    if(i0>=0 && i1>=0 && i2>=0 && i3>=0)
                    {
                        tau = (threshold - value[idx[i0]]) /(value[idx[i1]]-value[idx[i0]]);
                        I0 = node[idx[i0]]*(1-tau) + node[idx[i1]]*tau;

                        tau = (threshold - value[idx[i2]]) /(value[idx[i3]]-value[idx[i2]]);
                        I1 = node[idx[i2]]*(1-tau) + node[idx[i3]]*tau;

                        segs.push_back({I0, I1});
                    }
                }
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


