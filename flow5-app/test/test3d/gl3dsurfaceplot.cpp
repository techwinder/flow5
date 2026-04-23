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
#include <QVBoxLayout>

#include "gl3dsurfaceplot.h"



#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <api/stream2d.h>
#include <api/panel2d.h>
#include <api/panel3.h>

gl3dSurfacePlot::gl3dSurfacePlot(QWidget *pParent) : gl3dSurface(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_Vertices.resize(3);
    m_Vertices[0] = {-0.5, 0.0, 0.0};
    m_Vertices[1] = { 0.0, 0.0, 0.0};
    m_Vertices[2] = { 0.5, 0.0, 0.0};


    m_ValMin = LARGEVALUE;
    m_ValMax = -LARGEVALUE;

    m_Size_x = 23;
    m_Size_y = 23;
    m_PointArray.resize(m_Size_x*m_Size_y);

    m_bResetSurface = true;

    m_bDisplaySurface = false;


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

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            m_pchGrid    = new QCheckBox("Grid");
            connect(m_pchGrid, SIGNAL(clicked(bool)), SLOT(update()));
            m_pchGrid->setChecked(true);
            m_pchContour = new QCheckBox("Contour lines");
            connect(m_pchContour, SIGNAL(clicked(bool)), SLOT(update()));


            pMainLayout->addWidget(m_pchGrid);
            pMainLayout->addWidget(m_pchContour);
        }

        pFrame->setLayout(pMainLayout);
    }
}


void gl3dSurfacePlot::setTriangle(Vector3d* vertices)
{
    for(int i=0; i<3; i++) m_TriangleVertex[i] = vertices[i];
}


gl3dSurfacePlot::~gl3dSurfacePlot()
{
}


void gl3dSurfacePlot::on3dTop()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qtTop(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
    Quaternion qt1(  sqrt(2.0)/2.0, 0.0, 0.0, sqrt(2.0)/2.0);
    m_QuatEnd = qt1*qtTop;

    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dSurfacePlot::on3dBot()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qtTop(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
    Quaternion qt1(  sqrt(2.0)/2.0, 0.0, 0.0, sqrt(2.0)/2.0);
    Quaternion qtflip(180.0, Vector3d(0.0,1.0,0.0));
    m_QuatEnd = qt1*qtflip*qtTop;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}

void gl3dSurfacePlot::glRenderView()
{
    if(m_bDisplaySurface)
    {
        paintColourMap(m_vboSurface);
        if(m_bGrid) paintGrid();
        if(m_pchContour->isChecked())
            paintSegments(m_vboContourLines, W3dPrefs::s_ContourLineStyle);
    }

    paintTriangle(m_vboTriangle, false, true, Qt::darkRed);

    paintPolygon();

//    for(int i=0; i<3; i++)   paintSphere(m_Vertices[i], 0.01f, Qt::red);
}


void gl3dSurfacePlot::glMake3dObjects()
{
    if(m_bResetSurface)
    {
        glMakeSurface();
        QVector<double> nodevalues(m_PointArray.size());
        for(int i=0; i<m_PointArray.size(); i++) nodevalues[i] = m_PointArray.at(i).z;
        gl::makeQuadContoursOnGrid( m_vboContourLines, m_Size_x, m_Size_y, m_PointArray, nodevalues, true);

        //        m_pglStdBuffers->glMakeTriangle(m_TriangleVertex[0], m_TriangleVertex[1], m_TriangleVertex[2]);
        glMakePolygon();
    }
    m_bResetSurface = false;
}


void gl3dSurfacePlot::glMakePolygon()
{
    int polyArraySize = (m_Vertices.size()+1) * 3; // three vertex components
    QVector<float> pPolyVertexArray(polyArraySize);

    int iVtx=0;
    for(int i=0; i<m_Vertices.size(); i++)
    {
        pPolyVertexArray[iVtx++] = m_Vertices[i].x;
        pPolyVertexArray[iVtx++] = m_Vertices[i].y;
        pPolyVertexArray[iVtx++] = m_Vertices[i].z;
    }
    // close it
    pPolyVertexArray[iVtx++] = m_Vertices[0].x;
    pPolyVertexArray[iVtx++] = m_Vertices[0].y;
    pPolyVertexArray[iVtx++] = m_Vertices[0].z;
    Q_ASSERT(iVtx==polyArraySize);

    m_vboPolygon.destroy();
    m_vboPolygon.create();
    m_vboPolygon.bind();
    m_vboPolygon.allocate(pPolyVertexArray.data(), pPolyVertexArray.size()* int(sizeof(float)));

    m_vboPolygon.release();
}


void gl3dSurfacePlot::make3dPotentialSurface()
{
    m_TriangleVertex[0] = Vector3d(-1.0, 0.0, 0.0);
    m_TriangleVertex[1] = Vector3d( 1.0, 0.0, 0.0);
    m_TriangleVertex[2] = Vector3d( 0.0, 1.0, 0.0);
    setTriangle(m_TriangleVertex);

    Panel3 panel3(m_TriangleVertex[0], m_TriangleVertex[1], m_TriangleVertex[2]);

    double mu0=0.0, mu1=0.0, mu2=1.0;
    panel3.setBarycentricDoubletDensity(mu0,mu1,mu2);

    double x=0.0,y=0.0,z=0.0;
    double fx=0.0;
    double phi[] = {0.0,0.0,0.0};
    Vector3d V, Pt;
    double G1[10], G3[10], G5[10];   //  actual sizes are 3/6/3
    memset(G1, 0, sizeof(G1));
    memset(G3, 0, sizeof(G3));
    memset(G5, 0, sizeof(G5));

    double dG[18], GG[18];
    memset(dG, 0, sizeof(dG));
    memset(GG, 0, sizeof(GG));

    double range = 2.5;
    m_RefLength = range/2.0;

    z = 0.0;

    Vector3d A( 0.0, 0.0, -0.25);
    Vector3d B( 0.0, 0.0,  0.25);

    m_ValMin = 1e10;
    m_ValMax = -1e10;
    for(int i=0; i<m_Size_x; i++)
    {
        x = -range/2.0+ range*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -range/2.0+ range*double(j)/double(m_Size_y-1);
            Pt.set(x, y, z);

            //            panel3.doubleLayerQuadrature(Pt, fx, V);
            //            panel3.doubleLayerMC(Pt, fx, V);
            panel3.doubletBasisPotential(Pt, false, phi, true);        fx=(mu0*phi[0]+mu1*phi[1]+mu2*phi[2]);

            //qDebug("%13.7f, %13.7f, %13.7f", x,y,fx);

            m_ValMin = std::min(m_ValMin, fx);
            m_ValMax = std::max(m_ValMax, fx);

            m_RefLength = std::max(m_RefLength, fx);
            m_PointArray[i*m_Size_y+j].set(x,y,fx);
        }
    }
    setReferenceLength(m_RefLength);
    reset3dScale();
    m_bResetSurface = true;
    m_bDisplaySurface = true;
}


void gl3dSurfacePlot::make3dVelocitySurface()
{
    m_TriangleVertex[0] = Vector3d(-1.5, -1.0, 0.0);
    m_TriangleVertex[1] = Vector3d( 1.5, -1.0, 0.0);
    m_TriangleVertex[2] = Vector3d( 0.0,  1.5, 0.0);

    setTriangle(m_TriangleVertex);

    Panel3 panel3(m_TriangleVertex[0], m_TriangleVertex[1], m_TriangleVertex[2]);

    double mu0=1.0/3.0, mu1=1.0/3.0, mu2=1.0/3.0;
    panel3.setBarycentricDoubletDensity(mu0,mu1,mu2);

    double x=0.0,y=0.0,z=0.0;
    double fx=0.0;

    Vector3d Vb[3];
    Vector3d Pt;
    double G1[10], G3[10], G5[10];   //  actual sizes are 3/6/3
    memset(G1, 0, sizeof(G1));
    memset(G3, 0, sizeof(G3));
    memset(G5, 0, sizeof(G5));

    double dG[18], GG[18];
    memset(dG, 0, sizeof(dG));
    memset(GG, 0, sizeof(GG));

    double range = 5;
    m_RefLength = range/2.0;

    z = 0.0;
    m_ValMin = 1e10;
    m_ValMax = -1e10;

    for(int i=0; i<m_Size_x; i++)
    {
        x = -range/2.0+ range*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -range/2.0+ range*double(j)/double(m_Size_y-1);
            Pt.set(x, y, z);

            //            panel3.doubleLayerQuadrature(Pt, fx, V);
            //            panel3.doubleLayerMC(Pt, fx, V);
            m_ValMin = std::min(m_ValMin, fx);
            m_ValMax = std::max(m_ValMax, fx);

            panel3.doubletBasisVelocity(Pt, Vb, true);
            fx=(mu0*Vb[0].z+mu1*Vb[1].z+mu2*Vb[2].z);



            //qDebug("%13.7f, %13.7f, %13.7f", x,y,fx);

            m_RefLength = std::max(m_RefLength, fx);
            m_PointArray[i*m_Size_y+j].set(x,y,fx);
        }
    }
    setReferenceLength(m_RefLength);
    reset3dScale();
    m_bResetSurface = true;
    m_bDisplaySurface = true;
}


void gl3dSurfacePlot::make2dVortexStreamFct()
{
    m_Size_x = 23;
    m_Size_y = 30;

    m_PointArray.resize(m_Size_x*m_Size_y);


    Panel2d p2d0(Vector2d(m_Vertices[0].x,m_Vertices[0].y), Vector2d(m_Vertices[1].x,m_Vertices[1].y));
    Panel2d p2d1(Vector2d(m_Vertices[1].x,m_Vertices[1].y), Vector2d(m_Vertices[2].x,m_Vertices[2].y));

    Vector2d pt;
    double x=0,y=0;
    double psi=0;
    double psi_p=0, psi_m=0;
    double gamma1=30.0, gamma2=30.0, gamma3=30.0;
    double side = 3.0;
    double mx = side; // used to scale the 3d plot
    m_ValMin = 1e10;
    m_ValMax = -1e10;

    for(int i=0; i<m_Size_x; i++)
    {
        x = -side/2.0+ side*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -side/2+ side*double(j)/double(m_Size_y-1);

            pt.set(x,y);

            p2d0.linearVortex(pt, psi_p, psi_m);
            psi = ((gamma2+gamma1)*psi_p + (gamma2-gamma1) *psi_m)/4.0/PI;
            p2d1.linearVortex(pt, psi_p, psi_m);
            psi += ((gamma3+gamma2)*psi_p + (gamma3-gamma2) *psi_m)/4.0/PI;


            //            p2d.linearSource(pt, &psi1, &psi2, nullptr, nullptr);
            //            psi = psi1*sigma1+psi2*sigma2;
            m_ValMin = std::min(m_ValMin, psi);
            m_ValMax = std::max(m_ValMax, psi);

            m_PointArray[i*m_Size_y+j].set(x,y,psi);

            mx = std::max(mx, fabs(m_PointArray[i*m_Size_y+j].z));
        }
    }

    setReferenceLength(mx);
    reset3dScale();
    m_bResetSurface = true;
    m_bDisplaySurface = true;

}


void gl3dSurfacePlot::make2dSourceStreamFct()
{
    m_Size_x = 59;
    m_Size_y = 57;

    m_PointArray.resize(m_Size_x*m_Size_y);


    Panel2d p2d(Vector2d(m_Vertices[0].x,m_Vertices[0].y), Vector2d(m_Vertices[2].x,m_Vertices[2].y));

//    double sigma1 = 1.0;
//    double sigma2 = 1.0;


    Vector2d pt;
    double x(0),y(0);
    double psi(0);
    double phi(0);
//    double psi1(0), psi2(0);
    double side = 2.0;
    double mx = side; // used to scale the 3d plot
    m_ValMin = 1e10;
    m_ValMax = -1e10;

    for(int i=0; i<m_Size_x; i++)
    {
        x = -side/2.0+ side*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -side/2+ side*double(j)/double(m_Size_y-1);

            pt.set(x,y);
//            p2d.linearSource(pt, &psi1, &psi2, nullptr, nullptr);
//            psi  = sigma1*psi1 + sigma2*psi2;
            p2d.uniformSource(pt, &phi, &psi, nullptr);


            m_PointArray[i*m_Size_y+j].set(x,y,psi);
            m_ValMin = std::min(m_ValMin, psi);
            m_ValMax = std::max(m_ValMax, psi);

            mx = std::max(mx, fabs(m_PointArray[i*m_Size_y+j].z));
        }
    }

    int NPTS = 100;
    for(int i=0; i<NPTS; i++)
    {
        x = -side + 2.0*side*double(i)/double(NPTS-1);
        y = +0.00001;
        pt.set(x,y);
        p2d.uniformSource(pt, &phi, &psi, nullptr);
//        psi  = sigma1*psi1 + sigma2*psi2;
        qDebug(" %13g   %13g", x, psi);
    }


    setReferenceLength(mx);
    reset3dScale();
    m_bResetSurface = true;
    m_bDisplaySurface = true;

}


void gl3dSurfacePlot::makeVertices(QVector<Vector3d> const & vertices)
{
    for(int i=0; i<3; i++)
        m_Vertices[i] = vertices.at(i);
    m_RefLength = 0.0;
    for(int i=0; i<vertices.size(); i++)
    {
        m_RefLength = std::max(m_RefLength, fabs(vertices.at(i).x));
        m_RefLength = std::max(m_RefLength, fabs(vertices.at(i).y));
        m_RefLength = std::max(m_RefLength, fabs(vertices.at(i).z));
    }

    m_RefLength *= 2.0;
    setReferenceLength(m_RefLength);
    reset3dScale();

    m_bDisplaySurface = false;
}


void gl3dSurfacePlot::paintPolygon()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 idMatrix;
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);


        m_vboPolygon.bind();
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(255,0,0));

        m_shadLine.setUniformValue(m_locLine.m_Thickness, 5.0f);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));

        if(m_vboPolygon.size()>=0)
        {
            glDrawArrays(GL_LINE_STRIP, 0, 3);
        }

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        m_vboPolygon.release();
    }
    m_shadLine.release();
}


void gl3dSurfacePlot::make2dStreamFct(Stream2d const *pStream2d, float alpha, float qinf)
{
    m_Size_x = 97;
    m_Size_y = 97;

    m_PointArray.resize(m_Size_x*m_Size_y);

    //    double sigma1 = 1.0;
    //    double sigma2 = 1.0;
    //    double l = m_panel.B.x - m_panel.A.x;

    Vector2d pt;
    double x=0,y=0;
    double psi=0;
    //    double psi1, psi2;

    double sidex = 1.5;
    double sidey = 0.3;
    double mx = sidex; // used to scale the 3d plot
    m_ValMin = 1e10;
    m_ValMax = -1e10;

    for(int i=0; i<m_Size_x; i++)
    {
        x = -sidex/5.0+ sidex*double(i)/double(m_Size_x-1);
        for(int j=0; j<m_Size_y; j++)
        {
            y = -sidey/2.5+ sidey*double(j)/double(m_Size_y-1);

            pt.set(x,y);

            //            m_panel.linearVortexStream(pt, psi_p, psi_m);
            //            psi = ((gamma2+gamma1)*psi_p + (gamma2-gamma1) *psi_m)/4.0/PI;   // Drela eq. (3)

            psi = pStream2d->streamValue(alpha, qinf, pt);

            //            m_panel.linearSource(pt, &psi1, &psi2, nullptr, nullptr);
            //            psi = psi1*sigma1+psi2*sigma2;

            m_PointArray[i*m_Size_y+j].set(x,y,psi*100.0);
            m_ValMin = std::min(m_ValMin, psi);
            m_ValMax = std::max(m_ValMax, psi);

            mx = std::max(mx, fabs(m_PointArray[i*m_Size_y+j].z));
        }
    }

    setReferenceLength(mx);
    reset3dScale();
    m_bResetSurface = true;
    m_bDisplaySurface = true;

    // make the foil
    clearVertices();
    for(int i=0; i<pStream2d->nNodes(); i++)
    {
        Node2d const &n2d = pStream2d->node2d(i);
        addVertex(n2d.x, n2d.y, pStream2d->psi());
    }
}
