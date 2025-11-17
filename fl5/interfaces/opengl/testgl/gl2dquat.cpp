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
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

#include "gl2dquat.h"

#include <core/xflcore.h>
#include <api/utils.h>
#include <core/displayoptions.h>
#include <core/trace.h>
#include <interfaces/opengl/views/gl3dview.h> // for the static variables
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/globals/wt_globals.h>

int gl2dQuat::s_Hue(1000);
int gl2dQuat::s_MaxIter(128);
int gl2dQuat::s_iSlice(0);
float gl2dQuat::s_MaxLength(10.0f);
QVector2D gl2dQuat::s_Slicer(0.0f, 0.0f);
QVector4D gl2dQuat::s_Seed(0.0f, 0.0f, 0.0f, 0.0f);

gl2dQuat::gl2dQuat(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Quaternion Julia set");
    setMouseTracking(false);

    m_fScale = 0.5f;

    m_bAxes = false;

    m_locJulia = m_locSeed = m_locSlicer = m_locSlice = -1;
    m_locIters = m_locLength = -1;

    m_pCmdFrame = new QFrame(this);
    {
        m_pCmdFrame->setCursor(Qt::ArrowCursor);

        m_pCmdFrame->setFrameShape(QFrame::NoFrame);
        m_pCmdFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *plabTitle = new QLabel("<p>Using OpenGL's fragment shader to compute<br>"
                                           "and plot 2d slices of 4d quaternion Julia sets</p>");

            QGroupBox *pgbInput = new QGroupBox("Input");
            {
                QGridLayout *pParamsLayout = new QGridLayout;
                {
                    QLabel *plabSeedX = new QLabel("Seed.x:");
                    QLabel *plabSeedY = new QLabel("Seed.y:");
                    QLabel *plabSeedZ = new QLabel("Seed.z:");
                    QLabel *plabSeedW = new QLabel("Seed.w:");
                    pParamsLayout->addWidget(plabSeedX, 1, 1);
                    pParamsLayout->addWidget(plabSeedY, 2, 1);
                    pParamsLayout->addWidget(plabSeedZ, 3, 1);
                    pParamsLayout->addWidget(plabSeedW, 4, 1);

                    for(int i=0; i<4; i++)
                    {
                        m_pslSeed[i] = new QSlider(Qt::Horizontal);
                        m_pslSeed[i]->setMinimum(0);
                        m_pslSeed[i]->setMaximum(1000);
                        m_pslSeed[i]->setSliderPosition(500);
                        m_pslSeed[i]->setTickInterval(50);
                        m_pslSeed[i]->setTickPosition(QSlider::TicksBelow);
                        m_pslSeed[i]->setValue(int((s_Seed[i]+1.0)*500.0f));
                        connect(m_pslSeed[i], SIGNAL(sliderMoved(int)), SLOT(update()));
                        pParamsLayout->addWidget(m_pslSeed[i], i+1, 2);
                    }

                    QLabel *plabMaxIter = new QLabel("Max. iter.=");
                    m_pieMaxIter = new IntEdit(s_MaxIter);
                    m_pieMaxIter->setToolTip("<p>The maximum number of iterations before bailing out.</p>");

                    QLabel *plabMaxLength = new QLabel("Max. length=");
                    m_pfeMaxLength = new FloatEdit(s_MaxLength);
                    m_pfeMaxLength->setToolTip("<P>The escape amplitude of z.</p>");

                    connect(m_pieMaxIter,   SIGNAL(intChanged(int)),     this, SLOT(update()));
                    connect(m_pfeMaxLength, SIGNAL(floatChanged(float)), this, SLOT(update()));

                    pParamsLayout->addWidget(plabMaxIter,    6, 1);
                    pParamsLayout->addWidget(m_pieMaxIter,   6, 2);
                    pParamsLayout->addWidget(plabMaxLength,  7, 1);
                    pParamsLayout->addWidget(m_pfeMaxLength, 7, 2);
                }
                pgbInput->setLayout(pParamsLayout);
            }

            QGroupBox *pgbSlice = new QGroupBox("Slice");
            {
                QGridLayout *pSliceLayout = new QGridLayout;
                {
                    m_prbSlice[0] = new QRadioButton("X-Y");
                    m_prbSlice[1] = new QRadioButton("X-Z");
                    m_prbSlice[2] = new QRadioButton("Y-Z");
                    m_prbSlice[3] = new QRadioButton("X-W");
                    m_prbSlice[4] = new QRadioButton("Y-W");
                    m_prbSlice[5] = new QRadioButton("Z-W");
                    for(int i=0; i<6; i++)
                    {
                        connect(m_prbSlice[i], &QRadioButton::clicked, this, &gl2dQuat::onSlice);
                        if(s_iSlice==i) m_prbSlice[i]->setChecked(true);
                    }

                    pSliceLayout->addWidget(m_prbSlice[0],  1, 1);
                    pSliceLayout->addWidget(m_prbSlice[1],  1, 2);
                    pSliceLayout->addWidget(m_prbSlice[2],  1, 3);
                    pSliceLayout->addWidget(m_prbSlice[3],  2, 1);
                    pSliceLayout->addWidget(m_prbSlice[4],  2, 2);
                    pSliceLayout->addWidget(m_prbSlice[5],  2, 3);

                    for(int i=0; i<2; i++)
                    {
                        m_plabSlice[i] = new QLabel(QString::asprintf("Slice %d =:", i+1));
                        m_pslSlice[i] = new QSlider(Qt::Horizontal);
                        m_pslSlice[i] ->setMinimum(0);
                        m_pslSlice[i] ->setMaximum(1000);
                        m_pslSlice[i] ->setSliderPosition(500);
                        m_pslSlice[i] ->setTickInterval(50);
                        m_pslSlice[i] ->setTickPosition(QSlider::TicksBelow);
                        m_pslSlice[i] ->setValue(int((s_Slicer[i]+1.0)*500.0f));
                        connect(m_pslSlice[i] , SIGNAL(sliderMoved(int)), SLOT(update()));
                        pSliceLayout->addWidget(m_plabSlice[i],  i+5, 1);
                        pSliceLayout->addWidget(m_pslSlice[i],   i+5, 2,1,2);
                    }

                    pSliceLayout->setColumnStretch(3,1);
                }
                pgbSlice->setLayout(pSliceLayout);
            }


            QGroupBox *pgbOutput = new QGroupBox("Output");
            {
                QVBoxLayout *pOutputLayout = new QVBoxLayout;
                {
                    QGridLayout *pDisplayLayout = new QGridLayout;
                    {
                        QLabel *plabHue = new QLabel("Hue:");
                        m_pslTau = new QSlider(Qt::Horizontal);
                        m_pslTau->setMinimum(0);
                        m_pslTau->setMaximum(1000);
                        m_pslTau->setSliderPosition(500);
                        m_pslTau->setTickInterval(50);
                        m_pslTau->setTickPosition(QSlider::TicksBelow);
                        m_pslTau->setValue(s_Hue);
                        connect(m_pslTau, SIGNAL(sliderMoved(int)), SLOT(update()));

                        pDisplayLayout->addWidget(plabHue,         7, 1);
                        pDisplayLayout->addWidget(m_pslTau,        7, 2);
                        pDisplayLayout->setRowStretch(1,1);
                        pDisplayLayout->setColumnStretch(2,1);
                    }

                    QGridLayout *pImageLayout = new QGridLayout;
                    {
                        QLabel *plabImgWidth = new QLabel("Image size=");
                        QLabel *plabTimes = new QLabel(TIMESch);
                        QLabel *plabPixel = new QLabel("pixels");
                        pImageLayout->addWidget(plabImgWidth, 1, 1);
                        pImageLayout->addWidget(m_pieWidth,   1, 2);
                        pImageLayout->addWidget(plabTimes,    1, 3);
                        pImageLayout->addWidget(m_pieHeight,  1, 4);
                        pImageLayout->addWidget(plabPixel,    1, 5);

                        pImageLayout->addWidget(m_ppbSaveImg, 2, 1, 1, 5);
                    }
                    pOutputLayout->addLayout(pDisplayLayout);
                    pOutputLayout->addLayout(pImageLayout);
                }
                pgbOutput->setLayout(pOutputLayout);
            }



            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());
            m_plabScale->setWordWrap(true);
            m_plabScale->setMinimumHeight(DisplayOptions::tableFontStruct().height()*3);

            pFrameLayout->addWidget(plabTitle);
            pFrameLayout->addWidget(pgbInput);
            pFrameLayout->addWidget(pgbSlice);
            pFrameLayout->addWidget(pgbOutput);
            pFrameLayout->addWidget(m_plabScale);
        }

        m_pCmdFrame->setLayout(pFrameLayout);

        m_pCmdFrame->setStyleSheet("QFrame{background-color: transparent; color: white}"
                                   "QRadioButton{background-color: transparent; color: white}"
                                   "QCheckBox{background-color: transparent; color: white}");
    }

    onSlice();
}


void gl2dQuat::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dQuat");
    {
        s_Hue        = settings.value("Hue",        s_Hue).toInt();
        s_iSlice     = settings.value("iSlice",     s_iSlice).toInt();
        s_Slicer     = settings.value("Slicer",     s_Slicer).value<QVector2D>();
        s_MaxIter    = settings.value("MaxIters",   s_MaxIter).toInt();
        s_MaxLength  = settings.value("MaxLength",  s_MaxLength).toFloat();
        s_Seed       = settings.value("Seed",       s_Seed).value<QVector4D>();
    }
    settings.endGroup();
}


void gl2dQuat::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dQuat");
    {
        settings.setValue("Hue",        s_Hue);
        settings.setValue("iSlice",     s_iSlice);
        settings.setValue("Slicer",     s_Slicer);
        settings.setValue("MaxIters",   s_MaxIter);
        settings.setValue("MaxLength",  s_MaxLength);
        settings.setValue("Seed",       s_Seed);
    }
    settings.endGroup();
}


void gl2dQuat::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/shaders/shaders2d/fractal_VS.glsl";
    m_shadQuat.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadQuat.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac. vertex shader log:"+m_shadQuat.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/shaders2d/quat_FS.glsl";
    m_shadQuat.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadQuat.log().length())
    {
        strange = QString::asprintf("%s", QString("Quat. fragment shader log:"+m_shadQuat.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadQuat.link();
    m_shadQuat.bind();
    {
        m_attrVertexPosition = m_shadQuat.attributeLocation("VertexPosition");

        m_locJulia      = m_shadQuat.uniformLocation("julia");
        m_locSeed       = m_shadQuat.uniformLocation("seed");
        m_locSlice      = m_shadQuat.uniformLocation("slice");
        m_locSlicer     = m_shadQuat.uniformLocation("slicer");
        m_locLength     = m_shadQuat.uniformLocation("maxlength");
        m_locIters      = m_shadQuat.uniformLocation("maxiters");
        m_locColor      = m_shadQuat.uniformLocation("color");
        m_locViewTrans  = m_shadQuat.uniformLocation("ViewTrans");
        m_locViewScale  = m_shadQuat.uniformLocation("ViewScale");
        m_locViewRatio  = m_shadQuat.uniformLocation("ViewRatio");
    }
    m_shadQuat.release();

    gl2dView::initializeGL();
}


void gl2dQuat::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

//    glDisable(GL_BLEND);
//    glDisable(GL_DEPTH_TEST);

    double w = m_rectView.width();
    QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);

    if(m_shadQuat.bind())
    {
        int stride = 2;

        m_shadQuat.setUniformValue(m_locJulia,  1);

        s_MaxIter   = m_pieMaxIter->value();
        s_MaxLength = m_pfeMaxLength->value();
        m_shadQuat.setUniformValue(m_locIters,     s_MaxIter);
        m_shadQuat.setUniformValue(m_locLength,    s_MaxLength);


        for(int i=0; i<4; i++)
            s_Seed[i] = float(m_pslSeed[i]->value()-500)/500.0f;
        m_shadQuat.setUniformValue(m_locSeed,     s_Seed);

        for(int i=0; i<2; i++)
            s_Slicer[i] = float(m_pslSlice[i]->value()-500)/500.0f;
        m_shadQuat.setUniformValue(m_locSlicer, s_Slicer);

        for(int i=0; i<6; i++)
        {
            if(m_prbSlice[i]->isChecked())
            {
                s_iSlice = i;
                break;
            }
        }

        m_shadQuat.setUniformValue(m_locSlice, s_iSlice);


        s_Hue = m_pslTau->value();
        float tau = float(s_Hue)/1000.0f;
        QVector4D clr = QVector4D(xfl::getRed(tau), xfl::getGreen(tau), xfl::getBlue(tau), 1.0f);
        m_shadQuat.setUniformValue(m_locColor, clr);

        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(0,137,51));

        m_shadQuat.setUniformValue(m_locViewTrans,  off);
        m_shadQuat.setUniformValue(m_locViewRatio, float(width())/float(height()));
        m_shadQuat.setUniformValue(m_locViewScale,  m_fScale);

        m_vboQuad.bind();
        {
            m_shadQuat.enableAttributeArray(m_attrVertexPosition);
            m_shadQuat.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, stride, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadQuat.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
        m_shadQuat.release();
    }


    QString strange = QString::asprintf("Scale = %g\n", m_fScale);
    strange += QString::asprintf("Seed = (%.3f, %.3f, %.3f, %.3f)\n", s_Seed[0], s_Seed[1], s_Seed[2], s_Seed[3]);

    switch(s_iSlice)
    {
        case 0:
            strange += QString::asprintf("Slice.x=%.3f, Slice.y=%.3f)", s_Slicer[0], s_Slicer[1]);
            break;
        case 1:
            strange += QString::asprintf("Slice.x=%.3f, Slice.z=%.3f)", s_Slicer[0], s_Slicer[1]);
            break;
        case 2:
            strange += QString::asprintf("Slice.y=%.3f, Slice.z=%.3f)", s_Slicer[0], s_Slicer[1]);
            break;
        case 3:
            strange += QString::asprintf("Slice.x=%.3f, Slice.w=%.3f)", s_Slicer[0], s_Slicer[1]);
            break;
        case 4:
            strange += QString::asprintf("Slice.y=%.3f, Slice.w=%.3f)", s_Slicer[0], s_Slicer[1]);
            break;
        case 5:
            strange += QString::asprintf("Slice.z=%.3f, Slice.w=%.3f)", s_Slicer[0], s_Slicer[1]);
            break;
    }
    m_plabScale->setText(strange);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}


void gl2dQuat::onSlice()
{
    for(int i=0; i<6; i++)
    {
        if(m_prbSlice[i]->isChecked())
        {
            s_iSlice = i;
            break;
        }
    }

    QString str0, str1;
    switch(s_iSlice)
    {
        case 0:
            str0 = "Slice.x =";
            str1 = "Slice.y =";
            break;
        case 1:
            str0 = "Slice.x =";
            str1 = "Slice.z =";
            break;
        case 2:
            str0 = "Slice.y =";
            str1 = "Slice.z =";
            break;
        case 3:
            str0 = "Slice.x =";
            str1 = "Slice.w =:";
            break;
        case 4:
            str0 = "Slice.y =";
            str1 = "Slice.w =";
            break;
        case 5:
            str0 = "Slice.z =";
            str1 = "Slice.w =";
            break;
        case 6:
            str0 = "Slice." + THETAch +" =";
            str1 = "Slice.length =";
            break;
    }
    m_plabSlice[0]->setText(str0);
    m_plabSlice[1]->setText(str1);
    update();
}


void gl2dQuat::onSaveImage()
{
    QString filename = "Quat Julia.png";
    QString description = QString::asprintf("Made with flow5\n");
    description += QString::asprintf("Seed = x=%.3f, y=%.3f, z=%.3f, w=%.3f\n", s_Seed.x(), s_Seed.y(), s_Seed.z(), s_Seed.w());

    switch(s_iSlice)
    {
        case 0:
            description += QString::asprintf("Slice x=%.3f, y=%.3f", s_Slicer.x(), s_Slicer.y());
            break;
        case 1:
            description += QString::asprintf("Slice x=%.3f, z=%.3f", s_Slicer.x(), s_Slicer.y());
            break;
        case 2:
            description += QString::asprintf("Slice y=%.3f, z=%.3f", s_Slicer.x(), s_Slicer.y());
            break;
        case 3:
            description += QString::asprintf("Slice x=%.3f, w=%.3f", s_Slicer.x(), s_Slicer.y());
            break;
        case 4:
            description += QString::asprintf("Slice y=%.3f, w=%.3f", s_Slicer.x(), s_Slicer.y());
            break;
        case 5:
            description += QString::asprintf("Slice z=%.3f, w=%.3f", s_Slicer.x(), s_Slicer.y());
            break;
        case 6:
            break;
    }
    saveImage(filename, description);
}









