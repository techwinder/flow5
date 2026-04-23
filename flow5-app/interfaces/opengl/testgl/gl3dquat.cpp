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

#include <QApplication>
#include <QVBoxLayout>
#include <QOpenGLShaderProgram>
#include <QGroupBox>

#include "gl3dquat.h"

#include <core/xflcore.h>
#include <api/utils.h>
#include <core/displayoptions.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

QByteArray gl3dQuat::s_Geometry;
Quaternion gl3dQuat::s_Quat(0.0f, 0.0f, 0.0f, 0.0f);


gl3dQuat::gl3dQuat(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setWindowTitle("Quaternions");

    m_bInitialized  = false;
    m_bResetSegs = true;
    m_bAxes = true;

    setReferenceLength(3);
    reset3dScale();


    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());

    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QCheckBox *pchAxes = new QCheckBox("Axes");
            pchAxes->setChecked(m_bAxes);
            connect(pchAxes, &QCheckBox::clicked, this, &gl3dQuat::onAxes);

            m_pchTriangles = new QCheckBox("Triangles");
            m_pchTriangles->setChecked(false);
            connect(m_pchTriangles, SIGNAL(clicked(bool)), this, SLOT(update()));

            m_pchOutline = new QCheckBox("Outline");
            m_pchOutline->setChecked(false);
            connect(m_pchOutline, SIGNAL(clicked(bool)), this, SLOT(update()));

            QGroupBox *pgbInput = new QGroupBox("Input");
            {
                QGridLayout *pParamsLayout = new QGridLayout;
                {
                    QLabel *plabSeedX = new QLabel(THETAch +"=");
                    QLabel *plabSeedY = new QLabel("x=");
                    QLabel *plabSeedZ = new QLabel("y=");
                    QLabel *plabSeedW = new QLabel("z=");
                    pParamsLayout->addWidget(plabSeedX, 1, 1);
                    pParamsLayout->addWidget(plabSeedY, 2, 1);
                    pParamsLayout->addWidget(plabSeedZ, 3, 1);
                    pParamsLayout->addWidget(plabSeedW, 4, 1);

                    for(int i=0; i<4; i++)
                    {
                        m_pslQuat[i] = new QSlider(Qt::Horizontal);
                        m_pslQuat[i]->setMinimum(0);
                        m_pslQuat[i]->setMaximum(1000);
                        m_pslQuat[i]->setSliderPosition(500);
                        m_pslQuat[i]->setTickInterval(50);
                        m_pslQuat[i]->setTickPosition(QSlider::TicksBelow);
                        connect(m_pslQuat[i], SIGNAL(sliderMoved(int)), SLOT(onUpdateInput()));
                        pParamsLayout->addWidget(m_pslQuat[i], i+1, 2);
                    }

                    Vector3d axis = s_Quat.axis();
                    m_pslQuat[0]->setValue(int((s_Quat.angle()/180.0 +1.0)*500.0f));
                    m_pslQuat[1]->setValue(int((axis.x+1.0)*500.0f));
                    m_pslQuat[2]->setValue(int((axis.y+1.0)*500.0f));
                    m_pslQuat[3]->setValue(int((axis.z+1.0)*500.0f));
                }
                pgbInput->setLayout(pParamsLayout);
            }

            m_ppto = new PlainTextOutput;
            m_ppto->setCharDimensions(50, 35);

            pFrameLayout->addWidget(pchAxes);
            pFrameLayout->addWidget(m_pchTriangles);
            pFrameLayout->addWidget(m_pchOutline);
            pFrameLayout->addWidget(pgbInput);
            pFrameLayout->addWidget(m_ppto);
            pFrameLayout->addStretch();
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    m_Qt[0].set( 45.0, Vector3d(1.0,0.0,0.0));
    m_Qt[1].set( 60.0, Vector3d(0.0,1.0,0.0));
    m_Qt[2].set(180.0, Vector3d(0.0,0.0,1.0));

    for(int i=0; i<3; i++)
        m_ppto->onAppendStdText(m_Qt[i].listQuaternion()+ EOLstr);

    m_ppto->onAppendStdText((m_Qt[0]*m_Qt[0]).listQuaternion()+ EOLstr);


    double phi(PI/3), theta(PI/6);
    phi = PI/4; theta = 0.0;
    m_Vector[0].set(cos(phi) * cos(theta), sin(phi)*cos(theta), sin(theta));

    phi = PI/3; theta = PI/6;
    m_Vector[1].set(cos(phi) * cos(theta), sin(phi)*cos(theta), sin(theta));

    phi = PI/3; theta = PI/6;
    m_Vector[2].set(cos(phi) * cos(theta), sin(phi)*cos(theta), sin(theta));

    onUpdateInput();
}


void gl3dQuat::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dQuat");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_Quat.a   = settings.value("Seeda",  s_Quat.a).toFloat();
        s_Quat.qx  = settings.value("Seedx",  s_Quat.qx).toFloat();
        s_Quat.qy  = settings.value("Seedy",  s_Quat.qy).toFloat();
        s_Quat.qz  = settings.value("Seedz",  s_Quat.qz).toFloat();
    }
    settings.endGroup();
}


void gl3dQuat::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dQuat");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("Seeda",  s_Quat.a);
        settings.setValue("Seedx",  s_Quat.qx);
        settings.setValue("Seedy",  s_Quat.qy);
        settings.setValue("Seedz",  s_Quat.qz);
    }
    settings.endGroup();
}


void gl3dQuat::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void gl3dQuat::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void gl3dQuat::initializeGL()
{
    gl3dTestGLView::initializeGL();

    glMakeIcoSphere(3);
}


void gl3dQuat::glMake3dObjects()
{

    if(m_bResetSegs)
    {
        int buffersize = m_LineStrip.size()  // number of points
                         *3;                 // three components/vertex
        QVector<float> pHighlightVertexArray;
        pHighlightVertexArray.resize(buffersize);
        int iv=0;
        for (int k=0; k<m_LineStrip.size(); k++)
        {
            pHighlightVertexArray[iv++] = m_LineStrip.at(k).xf();
            pHighlightVertexArray[iv++] = m_LineStrip.at(k).yf();
            pHighlightVertexArray[iv++] = m_LineStrip.at(k).zf();
        }

        if(m_vboVertices.isCreated()) m_vboVertices.destroy();
        m_vboVertices.create();
        m_vboVertices.bind();
        m_vboVertices.allocate(pHighlightVertexArray.data(), buffersize * int(sizeof(GLfloat)));
        m_vboVertices.release();
        m_bResetSegs = false;
    }

}


void gl3dQuat::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

//    QMatrix4x4 vmMat(m_matView*m_matModel);
//    QMatrix4x4 pvmMat(m_matProj*vmMat);

    paintIcoSphere(Vector3d(), 1.0f, Qt::darkCyan, m_pchTriangles->isChecked(), m_pchOutline->isChecked());

/*    paintThickArrow(Vector3d(), m_Qt[0].axis(), xfl::Orchid,      m_matModel);
    paintThickArrow(Vector3d(), m_Qt[1].axis(), xfl::GreenYellow, m_matModel);
    paintThickArrow(Vector3d(), m_Qt[2].axis(), xfl::IndianRed,   m_matModel); */


//    paintThickArrow(Vector3d(), s_Quat.axis() * s_Quat.norm(), xfl::LightCoral);


/*    paintThinArrow(Vector3d(), m_Vector[0], Qt::white, 1.0, Line::SOLID);
    s_Quat.conjugate(m_Vector[0], m_Vector[1]);
    paintThinArrow(Vector3d(), m_Vector[1], xfl::CornFlowerBlue, 1.0, Line::SOLID);*/

    paintLineStrip(m_vboVertices, Qt::darkYellow, 1.0f);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dQuat::onUpdateInput()
{
    float angle = float(m_pslQuat[0]->value()-500)/500.0f * 180.0;
    float x     = float(m_pslQuat[1]->value()-500)/500.0f;
    float y     = float(m_pslQuat[2]->value()-500)/500.0f;
    float z     = float(m_pslQuat[3]->value()-500)/500.0f;

    s_Quat.set(angle, Vector3d(x,y,z), false);


    m_ppto->onAppendStdText(s_Quat.listQuaternion()+ EOLstr);

    m_LineStrip.clear();
    m_LineStrip.append(s_Quat.axis() * s_Quat.norm());

    Quaternion Q = s_Quat;
    Vector3d pt;
    for(int i=0; i<10; i++)
    {
        Q = s_Quat * Q;
        pt = Q.axis() * Q.norm();
        m_LineStrip.append(pt);

//        qDebug("%13g  %13g  %13g  %13g  %13g  %13g", sqrt(x*x+y*y+z*z), Q.axis().norm(), Q.norm(), pt.x, pt.y, pt.z);
    }

//    m_ppto->onAppendQText(EOLCHAR);
    m_bResetSegs = true;

    update();
}




