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
#include <QtConcurrent/QtConcurrent>

#include "gl3dquadfield.h"


#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>


Quaternion gl3dQuadField::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

bool gl3dQuadField::s_bShowVectors = true;
bool gl3dQuadField::s_bShowPlane = true;
double gl3dQuadField::s_VelScale = 0.1;
double gl3dQuadField::s_RefLength= 1.0;
int gl3dQuadField::s_iSource = 1;
int gl3dQuadField::s_iPlaneDir = 0;
double gl3dQuadField::s_PlanePos = 0.0;
int gl3dQuadField::s_iVelDir = 3;
int gl3dQuadField::s_Size_u = 3;
int gl3dQuadField::s_Size_w = 3;

QByteArray gl3dQuadField::s_Geometry;

gl3dQuadField::gl3dQuadField(QWidget *pParent) : gl3dXflView(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    m_bResetQuad = true;
    m_bResetContour = true;

    // quad size
    m_ul = m_wl = 1.0;

    // the color contour plane
    m_nRows = 151;
    m_nCols = 151;

    double s = 0.5;
    Vector3d LA(-s, s,0);
    Vector3d LB( s, s,0);
    Vector3d TA(-s,-s,0);
    Vector3d TB( s,-s,0);
    m_p4.setPanelFrame(LA, LB, TA, TB);

    makeControls();

    onMakeView();
}


void gl3dQuadField::makeControls()
{
    bool bContourPlane = true;

    m_pfrControls = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Text, DisplayOptions::textColor());

        QColor clr = DisplayOptions::backgroundColor();
        if(!DisplayOptions::isLightTheme())
            clr = Qt::white;

        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);

        m_pfrControls->setCursor(Qt::ArrowCursor);
        m_pfrControls->setFrameShape(QFrame::NoFrame);
        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QGroupBox *pSingBox = new QGroupBox("Singularity");
            {
                QVBoxLayout *pSingLayout = new QVBoxLayout;
                {
                    m_prbSource  = new QRadioButton("Source");
                    m_prbDoublet = new QRadioButton("Doublet");
                    m_prbVortex  = new QRadioButton("Vortex");
                    m_prbSource->setChecked( s_iSource==0);
                    m_prbDoublet->setChecked(s_iSource==1);
                    m_prbVortex->setChecked( s_iSource==2);

                    pSingLayout->addWidget(m_prbSource);
                    pSingLayout->addWidget(m_prbDoublet);
                    pSingLayout->addWidget(m_prbVortex);
                }
                pSingBox->setLayout(pSingLayout);
            }

            QGroupBox *pPlaneBox = new QGroupBox("Plane geometry");
            {
                pPlaneBox->setPalette(palette);
                QGridLayout *pPlaneLayout = new QGridLayout;
                {
                    QLabel *pLabDir = new QLabel("Plane normal");
//                    pLabDir->setPalette(palette);
                    m_pcbPlaneDir = new QComboBox;
                    m_pcbPlaneDir->addItems({"X", "Y", "Z"});
                    m_pcbPlaneDir->setCurrentIndex(s_iPlaneDir);
                    m_pslPlanePos = new QSlider(Qt::Horizontal);
                    m_pslPlanePos->setRange(0, 1000);
                    m_pslPlanePos->setTickInterval(100);
                    m_pslPlanePos->setTickPosition(QSlider::TicksBelow);
                    m_pslPlanePos->setValue((s_PlanePos+2.0)*250.0);

                    pPlaneLayout->addWidget(pLabDir,       1,1);
                    pPlaneLayout->addWidget(m_pcbPlaneDir, 1,2);
                    pPlaneLayout->addWidget(m_pslPlanePos, 2,1,1,2);
                }
                pPlaneBox->setLayout(pPlaneLayout);
            }

            QGroupBox *pContourBox = new QGroupBox("Contours");
            {
                pContourBox->setPalette(palette);
                QGridLayout*pParamsLayout = new QGridLayout;
                {
                    m_pchContourPlane = new QCheckBox("Contour plane");
                    m_pchContourPlane->setChecked(s_bShowPlane);
                    m_pcbVelDir = new QComboBox;
                    m_pcbVelDir->addItems({"X", "Y", "Z", "|V|"});
                    m_pcbVelDir->setCurrentIndex(s_iVelDir);

                    m_pchAutoScale = new QCheckBox("Auto scale");
                    m_pdeVMin = new FloatEdit(this);
                    m_pdeVMax = new FloatEdit(this);
                    m_pchAutoScale->setChecked(true);
                    m_pchAutoScale->setEnabled(false);
                    m_pdeVMin->setEnabled(false);
                    m_pdeVMax->setEnabled(false);
                    pParamsLayout->addWidget(m_pchContourPlane,    1, 1, 1, 2);
                    pParamsLayout->addWidget(new QLabel("V dir"),  2, 1, Qt::AlignRight|Qt::AlignVCenter);
                    pParamsLayout->addWidget(m_pcbVelDir,          2, 2);
                    pParamsLayout->addWidget(m_pchAutoScale,       3, 1, 1, 2);
                    pParamsLayout->addWidget(new QLabel("min"),    4, 1, Qt::AlignRight|Qt::AlignVCenter);
                    pParamsLayout->addWidget(m_pdeVMin,            4, 2);
                    pParamsLayout->addWidget(new QLabel("max"),    5, 1, Qt::AlignRight|Qt::AlignVCenter);
                    pParamsLayout->addWidget(m_pdeVMax,            5, 2);
                    pParamsLayout->setColumnStretch(3,1);
                }
                pContourBox->setLayout(pParamsLayout);
            }

            QGroupBox *pViewBox = new QGroupBox("View");
            {
                pViewBox->setPalette(palette);
                QFormLayout*pParamsLayout = new QFormLayout;
                {
                    m_pchVectors = new QCheckBox("Show vectors");
                    m_pchVectors->setChecked(s_bShowVectors);
                    m_pieNu = new IntEdit(s_Size_u);
                    m_pieNw = new IntEdit(s_Size_w);
                    m_pdeVelScale  = new FloatEdit(s_VelScale);
                    m_pdeRefLength = new FloatEdit(s_RefLength);

                    pParamsLayout->addRow(m_pchVectors);
                    pParamsLayout->addRow("N_u",          m_pieNu);
                    pParamsLayout->addRow("N_w",          m_pieNw);
                    pParamsLayout->addRow("Vector scale", m_pdeVelScale);
                    pParamsLayout->addRow("RefLength",    m_pdeRefLength);
                }
                pViewBox->setLayout(pParamsLayout);
            }

            pFrameLayout->addWidget(pSingBox);
            pFrameLayout->addWidget(pPlaneBox);
            pFrameLayout->addWidget(pContourBox);
            pFrameLayout->addWidget(pViewBox);

            m_pfrControls->setLayout(pFrameLayout);
        }
    }


    m_LegendOverlay.setTitle("V");
    m_LegendOverlay.setVisible(bContourPlane);

    connect(m_prbSource,       SIGNAL(clicked(bool)),        SLOT(onMakeView()));
    connect(m_prbDoublet,      SIGNAL(clicked(bool)),        SLOT(onMakeView()));
    connect(m_prbVortex,       SIGNAL(clicked(bool)),        SLOT(onMakeView()));

    connect(m_pieNu,           SIGNAL(intChanged(int)),       SLOT(onMakeView()));
    connect(m_pieNw,           SIGNAL(intChanged(int)),       SLOT(onMakeView()));
    connect(m_pdeVelScale,     SIGNAL(floatChanged(float)),       SLOT(onVelScale()));
    connect(m_pdeRefLength,    SIGNAL(floatChanged(float)),       SLOT(onRefLength()));
    connect(m_pchContourPlane, SIGNAL(toggled(bool)),        SLOT(onContourPlane(bool)));
    connect(m_pcbPlaneDir,     SIGNAL(activated(int)),       SLOT(onMakeView()));
    connect(m_pslPlanePos,     SIGNAL(sliderMoved(int)),     SLOT(onPlanePosition(int)));
    connect(m_pcbVelDir,       SIGNAL(activated(int)),       SLOT(onMakeView()));

    connect(m_pchAutoScale,    SIGNAL(clicked(bool)),        SLOT(onLegendScale()));
    connect(m_pdeVMin,         SIGNAL(floatChanged(float)),       SLOT(onLegendScale()));
    connect(m_pdeVMax,         SIGNAL(floatChanged(float)),       SLOT(onLegendScale()));

    connect(m_pchVectors,      SIGNAL(clicked(bool)),        SLOT(update()));
}


void gl3dQuadField::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dQuadField");
    {
        s_Geometry  = settings.value("Geometry").toByteArray();

        s_bShowVectors = settings.value("ShowVectors", s_bShowVectors).toBool();
        s_bShowPlane = settings.value("ShowPlane",     s_bShowPlane).toBool();
        s_Size_u       = settings.value("Size_u",      s_Size_u).toInt();
        s_Size_w       = settings.value("Size_w",      s_Size_w).toInt();
        s_iPlaneDir    = settings.value("iPlaneDir",   s_iPlaneDir).toInt();
        s_iVelDir      = settings.value("iVelDir",     s_iVelDir).toInt();
        s_iSource      = settings.value("iSource",     s_iSource).toInt()%3;
        s_PlanePos     = settings.value("PlanePos",    s_PlanePos).toDouble();
        s_RefLength    = settings.value("RefLength",   s_RefLength).toDouble();
        s_VelScale     = settings.value("VelScale",    s_VelScale).toDouble();
    }
    settings.endGroup();
}


void gl3dQuadField::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dQuadField");
    {
        settings.setValue("ShowVectors", s_bShowVectors);
        settings.setValue("ShowPlane",   s_bShowPlane);
        settings.setValue("Size_u",      s_Size_u);
        settings.setValue("Size_w",      s_Size_w);
        settings.setValue("Geometry",    s_Geometry);
        settings.setValue("iPlaneDir",   s_iPlaneDir);
        settings.setValue("iVelDir",     s_iVelDir);
        settings.setValue("iSource",     s_iSource);
        settings.setValue("PlanePos",    s_PlanePos);
        settings.setValue("RefLength",   s_RefLength);
        settings.setValue("VelScale",    s_VelScale);
    }
    settings.endGroup();
}


void gl3dQuadField::keyPressEvent(QKeyEvent *pEvent)
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


void gl3dQuadField::glRenderView()
{
    QMatrix4x4 vmMat, pvmMat;
    vmMat = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;
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

    paintQuad(Qt::darkCyan, true, 1.0f, false, true, true, m_vboBackgroundQuad);

    if(m_pchVectors->isChecked())
    {
        paintColorSegments(m_vboForces, W3dPrefs::s_LiftStyle.m_Width);
    }

    //paint the colormap
    if(m_pchContourPlane->isChecked())
    {
        paintColourMap(m_vboClrMap);
        paintSegments(m_vboContours, QColor(125,125,125), 1);
    }
}


void gl3dQuadField::glMake3dObjects()
{
    if(m_bResetQuad)
    {
//        glMakeVectors();
        glMakeForces(m_PointArray, m_VectorArray, s_VelScale, m_vboForces);

        gl::makeQuad(m_p4.vertex(0), m_p4.vertex(1), m_p4.vertex(2), m_p4.vertex(3), m_vboBackgroundQuad);

//        m_pglStdBuffers->glMakeTriangle(m_Triangle);
    }


    if(m_bResetContour)
    {
        bool bAuto = m_pchAutoScale->isChecked();
        double lmin = m_pdeVMin->value();
        double lmax = m_pdeVMax->value();

        gl::makeQuadColorMap(m_vboClrMap,   m_nRows, m_nCols, m_Nodes, m_Values, lmin, lmax, bAuto, xfl::isMultiThreaded());
        gl::makeQuadContoursOnGrid( m_vboContours, m_nRows, m_nCols, m_Nodes, m_Values, xfl::isMultiThreaded());

        m_bResetContour = false;
    }

    m_bResetQuad = false;
}


void gl3dQuadField::showEvent(QShowEvent *pEvent)
{
    gl3dXflView::showEvent(pEvent);
    restoreGeometry(s_Geometry);

//    m_LegendOverlay.setLegendHeight((height()*2)/3);
    m_LegendOverlay.resize(100, (height()*2)/3, devicePixelRatioF());
    m_LegendOverlay.makeLegend();

    QPoint pos1(5,height()-m_pfrControls->height()-5);
    m_pfrControls->move(pos1);

    setReferenceLength(s_RefLength);
    reset3dScale();
    restoreViewPoint(s_ab_quat);
}


void gl3dQuadField::hideEvent(QHideEvent *pEvent)
{
    gl3dXflView::hideEvent(pEvent);
    readData();
    s_Geometry = saveGeometry();
    saveViewPoint(s_ab_quat);
}


void gl3dQuadField::onContourPlane(bool bChecked)
{
    if(bChecked)
        onMakeView();

    m_LegendOverlay.setVisible(bChecked);

    m_pchAutoScale->setEnabled(bChecked);
    if(!bChecked)
    {
        m_pdeVMin->setEnabled(false);
        m_pdeVMax->setEnabled(false);
    }

    update();
}


void gl3dQuadField::onLegendScale()
{
    if(m_pchAutoScale->isChecked())
    {
        double vmin = 1e10;
        double vmax = -1e10;
        for(int iv=0; iv<m_Values.size(); iv++)
        {
            vmin = std::min(vmin, m_Values.at(iv));
            vmax = std::max(vmax, m_Values.at(iv));
        }
        m_LegendOverlay.setRange(vmin, vmax);
        m_pdeVMin->setValue(vmin);
        m_pdeVMax->setValue(vmax);
    }
    else
    {
        m_LegendOverlay.setRange(m_pdeVMin->value(), m_pdeVMax->value());
    }
    m_LegendOverlay.makeLegend();
    m_pdeVMin->setEnabled(!m_pchAutoScale->isChecked());
    m_pdeVMax->setEnabled(!m_pchAutoScale->isChecked());

    m_bResetContour = true;
    update();
}


void gl3dQuadField::readData()
{
    if     (m_prbSource->isChecked())  s_iSource=0;
    else if(m_prbDoublet->isChecked()) s_iSource=1;
    else if(m_prbVortex->isChecked())  s_iSource=2;

    s_bShowVectors = m_pchVectors->isChecked();
    s_bShowPlane = m_pchContourPlane->isChecked();
    s_Size_u = m_pieNu->value();
    s_Size_w = m_pieNw->value();
    s_iVelDir = m_pcbVelDir->currentIndex();
    s_RefLength = m_pdeRefLength->value();
    s_VelScale = m_pdeVelScale->value();
    s_iPlaneDir = m_pcbPlaneDir->currentIndex()%3;
}


void gl3dQuadField::onPlanePosition(int pos)
{
    s_PlanePos = double(pos)/250.0 - 2.0;
    onMakeView();
}


void gl3dQuadField::onMakeView()
{
    readData();
    makeVelocityVectors();
    makeColorValues();
    m_LegendOverlay.makeLegend();
    update();
}


void gl3dQuadField::onRefLength()
{
    readData();
    setReferenceLength(s_RefLength);
    reset3dScale();
}


void gl3dQuadField::onVelScale()
{
    readData();
    makeVelocityVectors();
    m_bResetQuad = true;
    update();
}


void gl3dQuadField::makeColorValues()
{
    m_Nodes.resize(m_nRows*m_nCols);
    m_Values.resize(m_nRows*m_nCols);

    if(xfl::isMultiThreaded())
    {
        QFutureSynchronizer<void> futureSync;
        for(int irow=0; irow<m_nRows; irow++)
        {
           double u = (double(irow)/double(m_nRows-1)-0.5)*m_wl*2;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
           futureSync.addFuture(QtConcurrent::run(this, &gl3dQuadField::makeColorRow,
                                                   u, s_PlanePos, irow, s_iPlaneDir));
#else
           futureSync.addFuture(QtConcurrent::run(&gl3dQuadField::makeColorRow, this,
                                                   u, s_PlanePos, irow, s_iPlaneDir));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int irow=0; irow<m_nRows; irow++)
        {
            double u = (double(irow)/double(m_nRows-1)-0.5)*m_wl*2;
            makeColorRow(u, s_PlanePos, irow, s_iPlaneDir);
        }
    }

    if(m_pchAutoScale->isChecked())
    {
        double vmin = 1e10;
        double vmax = -1e10;
        for(int iv=0; iv<m_Values.size(); iv++)
        {
            vmin = std::min(vmin, m_Values.at(iv));
            vmax = std::max(vmax, m_Values.at(iv));
        }
        m_LegendOverlay.setRange(vmin, vmax);
        m_pdeVMin->setValue(vmin);
        m_pdeVMax->setValue(vmax);
    }
    else
    {
        m_LegendOverlay.setRange(m_pdeVMin->value(), m_pdeVMax->value());
    }
    m_bResetContour = true;
}


void gl3dQuadField::makeColorRow(double u, double w, int ir, int iPlaneDir)
{
    Vector3d Vel, omp;
    for(int ic=0; ic<m_nCols; ic++)
    {
        double y = (double(ic)/double(m_nCols-1)-0.5)*m_wl*2;

        int idx = ir*m_nCols + ic;
        switch(iPlaneDir)
        {
            default:
            case 0:  m_Nodes[idx].set(w, y, u);      break;
            case 1:  m_Nodes[idx].set(y, w, u);      break;
            case 2:  m_Nodes[idx].set(y, u, w);      break;
        }

        Vector3d const &pt = m_Nodes.at(idx);
        Vel = velocity(pt);

        switch (m_pcbVelDir->currentIndex())
        {
            case 0: m_Values[idx] = Vel.x;       break;
            case 1: m_Values[idx] = Vel.y;       break;
            case 2: m_Values[idx] = Vel.z;       break;
            default:
            case 3: m_Values[idx] = Vel.norm();  break;
        }
    }
}


Vector3d gl3dQuadField::velocity(Vector3d const &pt)
{
    Vector3d Vel;
    switch(s_iSource)
    {
        case 0:  m_p4.sourceN4023Velocity(pt, false, Vel, 0.0);     break;
        case 1:  m_p4.doubletN4023Velocity(pt, Vel, 0.0);           break;
        case 2:
        {
//                Panel4::VLMQmnVelocity(m_p4.LB(), m_p4.LA(), m_p4.TB(), m_p4.TA(), pt, Vel);
//                Vel *= 4.0*PI;
            m_p4.doubletVortexVelocity(pt, Vel, 0.0, true);
            break;
        }
        default: Vel.set(0,0,0);                                    break;
    }
    return Vel;
}


void gl3dQuadField::makeVelocityVectors()
{
    setWindowTitle("Equivalent vortex induced velocity");

    m_PointArray.resize( s_Size_u*s_Size_w);
    m_VectorArray.resize(s_Size_u*s_Size_w);

    Vector2d A(-1.0, 0.0);
    Vector2d B(+1.0, 0.0);

    double u=0, w=0;
    double z = s_PlanePos;
    Vector3d pt, Vel;

    for(int i=0; i<s_Size_u; i++)
    {
        if(s_Size_u>1) u = -m_ul + 2.0*m_ul*double(i)/double(s_Size_u-1);
        else           u = m_ul/2.0;

        for(int j=0; j<s_Size_w; j++)
        {
            if(s_Size_w>1) w = -m_wl + 2*m_wl*double(j)/double(s_Size_w-1);
            else           w =  0.0;

            switch(s_iPlaneDir)
            {
                default:
                case 0:  pt.set(z, u, w);      break;
                case 1:  pt.set(u, z, w);      break;
                case 2:  pt.set(u, w, z);      break;
            }

            Vel = velocity(pt);

            Vel *= 4.0*PI;

            m_PointArray[i*(s_Size_w)+j].set(pt);

            m_VectorArray[i*(s_Size_w)+j].set(0,0,0);
            if     (s_iVelDir==0) m_VectorArray[i*(s_Size_w)+j].x = Vel.x*s_VelScale;
            else if(s_iVelDir==1) m_VectorArray[i*(s_Size_w)+j].y = Vel.y*s_VelScale;
            else if(s_iVelDir==2) m_VectorArray[i*(s_Size_w)+j].z = Vel.z*s_VelScale;
            else                  m_VectorArray[i*(s_Size_w)+j].set(Vel.x*s_VelScale, Vel.y*s_VelScale, Vel.z*s_VelScale);
        }
    }

    m_bResetQuad = true;
}


void gl3dQuadField::glMakeForces(QVector<Vector3d> const &nodes, QVector<Vector3d> const &Vectors, float scalef, QOpenGLBuffer &vbo)
{
    int nNodes = nodes.size();
    if(!nNodes)
    {
        vbo.bind();
        vbo.destroy();
        return;
    }

    Quaternion Qt; // Quaternion operator to align the reference arrow with the panel's normal
    Vector3d Omega; // rotation vector to align the reference arrow with the panel's normal
    Vector3d O;
    //The vectors defining the reference arrow
    Vector3d Ra(0.0,0.0,1.0);
    Vector3d R1( 0.05, 0.0, -0.1);
    Vector3d R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    //define the range of values to set the colors in accordance
    float rmin =  1.e10;
    float rmax = -1.e10;

    for (int in=0; in<nNodes; in++)
    {
        if(in<Vectors.size())
        {
            float length = Vectors.at(in).normf();
            rmax = std::max(rmax, length);
            rmin = std::min(rmin, length);
        }
    }

    float range = rmax - rmin;

    // vertices array size:
    //        nNodes x 1 arrow
    //      x3 lines per arrow
    //      +2 (vertices per line)
    //      x(3 vertex components+3 color components)
    //

    int forceVertexSize = nNodes * 3 * 2 * 6;
    QVector<float> forceVertexArray(forceVertexSize);
    QColor clr;
    int iv=0;
    for (int in=0; in<nNodes; in++)
    {
        float force = Vectors.at(in).normf();
        float tau = (force-rmin)/range;
        //scale force for display
        force *= scalef;

        clr = ColourLegend::colour(tau);
        float r = clr.redF();
        float g = clr.greenF();
        float b = clr.blueF();

        O = nodes.at(in);

        Qt.from2UnitVectors(Ra, Vectors.at(in).normalized());

        Qt.conjugate(Ra,  P);
        Qt.conjugate(R1, P1);
        Qt.conjugate(R2, P2);

        // Scale the pressure vector
        P  *= double(force);
        P1 *= double(force);
        P2 *= double(force);


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
        forceVertexArray[iv++] = O.xf()+P.xf()+P1.xf();
        forceVertexArray[iv++] = O.yf()+P.yf()+P1.yf();
        forceVertexArray[iv++] = O.zf()+P.zf()+P1.zf();
        forceVertexArray[iv++] = r;
        forceVertexArray[iv++] = g;
        forceVertexArray[iv++] = b;

        forceVertexArray[iv++] = O.xf()+P.xf();
        forceVertexArray[iv++] = O.yf()+P.yf();
        forceVertexArray[iv++] = O.zf()+P.zf();
        forceVertexArray[iv++] = r;
        forceVertexArray[iv++] = g;
        forceVertexArray[iv++] = b;
        forceVertexArray[iv++] = O.xf()+P.xf()+P2.xf();
        forceVertexArray[iv++] = O.yf()+P.yf()+P2.yf();
        forceVertexArray[iv++] = O.zf()+P.zf()+P2.zf();
        forceVertexArray[iv++] = r;
        forceVertexArray[iv++] = g;
        forceVertexArray[iv++] = b;
    }

    Q_ASSERT(iv==forceVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(forceVertexArray.data(), forceVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


