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

#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShowEvent>
#include <QTabWidget>

#include "gllightdlg.h"
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/exponentialslider.h>

#include <api/units.h>

double GLLightDlg::s_VerticalAngle = 60.0;
double GLLightDlg::s_ViewDistance = 15.0;
bool GLLightDlg::s_bOrtho = true;

#define REFLENGTH  10.0 //meters

QByteArray GLLightDlg::s_Geometry;

GLLightDlg::GLLightDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("OpenGL Light Options");
//    setModal(false);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    setupLayout();

    m_pglView = nullptr;

    connect(m_pchLight,                SIGNAL(clicked()), SLOT(onLight()));

    connect(m_pslRed,                  SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_pslGreen,                SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_pslBlue,                 SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_peslLightAmbient,        SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_peslLightDiffuse,        SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_peslLightSpecular,       SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_peslXLight,              SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_peslYLight,              SIGNAL(sliderMoved(int)), SLOT(onChanged()));
    connect(m_peslZLight,              SIGNAL(sliderMoved(int)), SLOT(onChanged()));

    connect(m_peslEyePos,              SIGNAL(sliderMoved(int)), SLOT(onChanged()));

    connect(m_pslMatShininess,         SIGNAL(sliderMoved(int)), SLOT(onChanged()));

    connect(m_pdeConstantAttenuation,  SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pdeLinearAttenuation,    SIGNAL(floatChanged(float)), SLOT(onChanged()));
    connect(m_pdeQuadAttenuation,      SIGNAL(floatChanged(float)), SLOT(onChanged()));
}


void GLLightDlg::setupLayout()
{
    QTabWidget *pMainTabWt = new QTabWidget;
    {
        QFrame *pLightTab = new QFrame;
        {
            QVBoxLayout *pLightLayout = new QVBoxLayout;
            {
                m_pchLight = new QCheckBox("Light");
                QGroupBox *pLightIntensityBox = new QGroupBox("Intensity");
                {
                    QVBoxLayout *pLightIntensities = new QVBoxLayout;
                    {
                        QGridLayout *pLightIntensityLayout = new QGridLayout;
                        {
                            QLabel *lab1 = new QLabel("Diffuse");
                            QLabel *lab2 = new QLabel("Ambient");
                            QLabel *lab3 = new QLabel("Specular");

                            m_peslLightAmbient      = new ExponentialSlider(false, 2.0, Qt::Horizontal);
                            m_peslLightAmbient->setToolTip("<b>Ambient:</b><br>"
                                                            "Bounced light which has been scattered so much that it "
                                                            "is impossible to tell the direction to its source. "
                                                            "It is not attenuated by distance, and disappears if "
                                                            "the light is turned off.");
                            m_peslLightDiffuse      = new ExponentialSlider(false, 2.0, Qt::Horizontal);
                            m_peslLightDiffuse->setToolTip("<b>Diffuse:</b><br>"
                                                            "Directional light which is brighter on perpendicular "
                                                            "surfaces. Its reflection is scattered evenly.");
                            m_peslLightSpecular     = new ExponentialSlider(false, 2.0, Qt::Horizontal);
                            m_peslLightSpecular->setToolTip("<b>Specular:</b><br>"
                                                             "Directional light which tends to reflect in a preferred "
                                                             "direction. It is associated with shininess.");

                            m_peslLightAmbient->setMinimum(0);
                            m_peslLightAmbient->setMaximum(100);
                            m_peslLightAmbient->setTickInterval(10);
                            m_peslLightDiffuse->setMinimum(0);
                            m_peslLightDiffuse->setMaximum(100);
                            m_peslLightDiffuse->setTickInterval(10);
                            m_peslLightSpecular->setMinimum(0);
                            m_peslLightSpecular->setMaximum(100);
                            m_peslLightSpecular->setTickInterval(10);
                            m_peslLightDiffuse->setTickPosition(QSlider::TicksBelow);
                            m_peslLightAmbient->setTickPosition(QSlider::TicksBelow);
                            m_peslLightSpecular->setTickPosition(QSlider::TicksBelow);

                            m_plabLightAmbient = new QLabel;
                            m_plabLightDiffuse = new QLabel;
                            m_plabLightSpecular = new QLabel;
                            pLightIntensityLayout->addWidget(lab2,1,1);
                            pLightIntensityLayout->addWidget(lab1,2,1);
                            pLightIntensityLayout->addWidget(lab3,3,1);
                            pLightIntensityLayout->addWidget(m_peslLightAmbient,1,2);
                            pLightIntensityLayout->addWidget(m_peslLightDiffuse,2,2);
                            pLightIntensityLayout->addWidget(m_peslLightSpecular,3,2);
                            pLightIntensityLayout->addWidget(m_plabLightAmbient,1,3);
                            pLightIntensityLayout->addWidget(m_plabLightDiffuse,2,3);
                            pLightIntensityLayout->addWidget(m_plabLightSpecular,3,3);
                        }

                        QHBoxLayout *pAttenuationLayout = new QHBoxLayout;
                        {
                            QLabel *pAtt = new QLabel("Attenuation factor=1/(");
                            QLabel *pConstant = new QLabel("+");
                            QLabel *pLinear = new QLabel(".d +");
                            QLabel *pQuadratic = new QLabel(QString::fromUtf8(".d²)"));
                            pConstant->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            pLinear->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            pQuadratic->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            m_pdeConstantAttenuation = new FloatEdit(0.0);
                            m_pdeLinearAttenuation = new FloatEdit(0.0);
                            m_pdeQuadAttenuation = new FloatEdit(0.0);

                            pAttenuationLayout->addWidget(pAtt);
                            pAttenuationLayout->addWidget(m_pdeConstantAttenuation);
                            pAttenuationLayout->addWidget(pConstant);
                            pAttenuationLayout->addWidget(m_pdeLinearAttenuation);
                            pAttenuationLayout->addWidget(pLinear);
                            pAttenuationLayout->addWidget(m_pdeQuadAttenuation);
                            pAttenuationLayout->addWidget(pQuadratic);
                        }
                        pLightIntensities->addLayout(pLightIntensityLayout);
                        pLightIntensities->addLayout(pAttenuationLayout);
                    }
                    pLightIntensityBox->setLayout(pLightIntensities);
                }

                QGroupBox *pLightColorBox = new QGroupBox("Colour");
                {
                    QGridLayout *pLightColor = new QGridLayout;
                    {
                        QLabel *lab11 = new QLabel("Red");
                        QLabel *lab12 = new QLabel("Green");
                        QLabel *lab13 = new QLabel("Blue");
                        m_pslRed    = new QSlider(Qt::Horizontal);
                        m_pslGreen  = new QSlider(Qt::Horizontal);
                        m_pslBlue   = new QSlider(Qt::Horizontal);
                        m_pslRed->setMinimum(0);
                        m_pslRed->setMaximum(100);
                        m_pslRed->setTickInterval(10);
                        m_pslGreen->setMinimum(0);
                        m_pslGreen->setMaximum(100);
                        m_pslGreen->setTickInterval(10);
                        m_pslBlue->setMinimum(0);
                        m_pslBlue->setMaximum(100);
                        m_pslBlue->setTickInterval(10);
                        m_pslRed->setTickPosition(QSlider::TicksBelow);
                        m_pslGreen->setTickPosition(QSlider::TicksBelow);
                        m_pslBlue->setTickPosition(QSlider::TicksBelow);

                        m_plabLightRed   = new QLabel;
                        m_plabLightGreen = new QLabel;
                        m_plabLightBlue  = new QLabel;

                        pLightColor->addWidget(lab11,1,1);
                        pLightColor->addWidget(lab12,2,1);
                        pLightColor->addWidget(lab13,3,1);
                        pLightColor->addWidget(m_pslRed,1,2);
                        pLightColor->addWidget(m_pslGreen,2,2);
                        pLightColor->addWidget(m_pslBlue,3,2);
                        pLightColor->addWidget(m_plabLightRed,1,3);
                        pLightColor->addWidget(m_plabLightGreen,2,3);
                        pLightColor->addWidget(m_plabLightBlue,3,3);
                        pLightColorBox->setLayout(pLightColor);
                    }
                }

                QHBoxLayout *pMaterialDataLayout = new QHBoxLayout;
                {
                    m_pslMatShininess = new QSlider(Qt::Horizontal);
                    m_pslMatShininess->setRange(4, 64);
                    m_pslMatShininess->setTickInterval(2);
                    m_pslMatShininess->setTickPosition(QSlider::TicksBelow);

                    QLabel *lab35 = new QLabel("Material shininess");
                    m_plabMatShininess = new QLabel("1");

                    pMaterialDataLayout->addWidget(lab35);
                    pMaterialDataLayout->addWidget(m_pslMatShininess);
                    pMaterialDataLayout->addWidget(m_plabMatShininess);
                }

                QGroupBox *pLightPositionBox = new QGroupBox("Position, view-space");
                {
                    QString tip("This set of sliders define the light's position in view-space coordidates.");
                    pLightPositionBox->setToolTip(tip);
                    QGridLayout *pLightPosition = new QGridLayout;
                    {
                        QLabel *pLab21 = new QLabel("x");
                        QLabel *pLab22 = new QLabel("y");
                        QLabel *pLab23 = new QLabel("z");

                        m_peslXLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
                        m_peslYLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
                        m_peslZLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
                        m_peslXLight->setTickPosition(QSlider::TicksBelow);
                        m_peslYLight->setTickPosition(QSlider::TicksBelow);
                        m_peslZLight->setTickPosition(QSlider::TicksBelow);
                        m_peslXLight->setToolTip(tip);
                        m_peslYLight->setToolTip(tip);
                        m_peslZLight->setToolTip(tip);

                        m_plabposXValue = new QLabel(Units::lengthUnitQLabel());
                        m_plabposYValue = new QLabel(Units::lengthUnitQLabel());
                        m_plabposZValue = new QLabel(Units::lengthUnitQLabel());

                        pLightPosition->addWidget(pLab21,           1,1);
                        pLightPosition->addWidget(pLab22,           2,1);
                        pLightPosition->addWidget(pLab23,           3,1);
                        pLightPosition->addWidget(m_peslXLight,    1,2);
                        pLightPosition->addWidget(m_peslYLight,    2,2);
                        pLightPosition->addWidget(m_peslZLight,    3,2);
                        pLightPosition->addWidget(m_plabposXValue, 1,3);
                        pLightPosition->addWidget(m_plabposYValue, 2,3);
                        pLightPosition->addWidget(m_plabposZValue, 3,3);
                        pLightPositionBox->setLayout(pLightPosition);
                    }
                }

                QHBoxLayout *pEyeDistLayout = new QHBoxLayout;
                {
                    QLabel *pLabEyePos = new QLabel("View distance:");
                    m_peslEyePos = new ExponentialSlider(false, 2.0, Qt::Horizontal);
                    m_peslEyePos->setTickPosition(QSlider::TicksBelow);
                    m_peslEyePos->setToolTip("This value defines how far way the eye is from the scene.<br>"
                                             "In the default orthographic projection, this only changes the position "
                                             "of the point of reflection on surfaces and does not affect the view's proportions.<br>"
                                             "The position is expressed in view-space coordinates.<br>"
                                             "Recommendation: set the position to the max. distance if using orthographic projection.");
                    m_plabEyeDist = new QLabel(Units::lengthUnitQLabel());
                    pEyeDistLayout->addWidget(pLabEyePos);
                    pEyeDistLayout->addWidget(m_peslEyePos);
                    pEyeDistLayout->addWidget(m_plabEyeDist);
                }

                pLightLayout->addWidget(m_pchLight);
                pLightLayout->addWidget(pLightIntensityBox);
                pLightLayout->addWidget(pLightColorBox);
                pLightLayout->addLayout(pMaterialDataLayout);
                pLightLayout->addWidget(pLightPositionBox);
                pLightLayout->addLayout(pEyeDistLayout);
            }
            pLightTab->setLayout(pLightLayout);
        }

        QFrame *pProjectionTab = new QFrame;
        {
            QGridLayout *pProjectionLayout = new QGridLayout;
            {
                m_prbOrtho = new QRadioButton("Ortho (recommended)");
                m_prbPerspective = new QRadioButton("Perspective");

                m_pdeVerticalAngle = new FloatEdit(75);
                m_pdeViewDistance = new FloatEdit(1);
                pProjectionLayout->addWidget(m_prbOrtho, 1, 1);
                pProjectionLayout->addWidget(m_prbPerspective, 1, 2);
                pProjectionLayout->addWidget(new QLabel("Vertical angle:"), 2, 1, Qt::AlignRight | Qt::AlignVCenter);
                pProjectionLayout->addWidget(m_pdeVerticalAngle, 2, 2);
                pProjectionLayout->addWidget(new QLabel("<p>&deg;</p>"), 2, 3, Qt::AlignLeft | Qt::AlignVCenter);
                pProjectionLayout->addWidget(new QLabel("View distance:"), 3, 1, Qt::AlignRight | Qt::AlignVCenter);
                pProjectionLayout->addWidget(m_pdeViewDistance,3,2);
                pProjectionLayout->addWidget(new QLabel(Units::lengthUnitQLabel()), 3, 3, Qt::AlignLeft | Qt::AlignVCenter);
                pProjectionLayout->setRowStretch(4,1);
                connect(m_prbOrtho,         SIGNAL(clicked(bool)),  SLOT(onViewProjection()));
                connect(m_prbPerspective,   SIGNAL(clicked(bool)),  SLOT(onViewProjection()));
                connect(m_pdeVerticalAngle, SIGNAL(floatChanged(float)), SLOT(onViewProjection()));
                connect(m_pdeViewDistance,  SIGNAL(floatChanged(float)), SLOT(onViewProjection()));
            }
            pProjectionTab->setLayout(pProjectionLayout);
        }

        pMainTabWt->addTab(pLightTab, "Light");
        pMainTabWt->addTab(pProjectionTab, "Projection (experimental)");
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::RestoreDefaults);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pMainTabWt);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void GLLightDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)                 close();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton)  onDefaults();
}


void GLLightDlg::apply()
{
    readParams();
    setLabels();

    if(m_pglView)
    {
        m_pglView->glSetupLight();
        m_pglView->setLightVisible(gl3dView::isLightOn());
        m_pglView->update();
    }
}


void GLLightDlg::onChanged()
{
    apply();
}


void GLLightDlg::onDefaults()
{
    setDefaults();
    setParams();
    setEnabled();

    if(m_pglView)
    {
        m_pglView->glSetupLight();
        m_pglView->update();
    }
}


void GLLightDlg::readParams(void)
{
    Light light;

    light.m_bIsLightOn = m_pchLight->isChecked();

    light.m_Red     = float(m_pslRed->value())    /100.0f;
    light.m_Green   = float(m_pslGreen->value())  /100.0f;
    light.m_Blue    = float(m_pslBlue->value())   /100.0f;

    light.m_X  = m_peslXLight->expValuef()/100.0f;
    light.m_Y  = m_peslYLight->expValuef()/100.0f;
    light.m_Z  = m_peslZLight->expValuef()/100.0f;

    light.m_EyeDist = m_peslEyePos->expValuef()/100.0f;

    light.m_Ambient     = m_peslLightAmbient->expValuef()  / 20.0f;
    light.m_Diffuse     = m_peslLightDiffuse->expValuef()  / 20.0f;
    light.m_Specular    = m_peslLightSpecular->expValuef() / 20.0f;

    light.m_iShininess   = m_pslMatShininess->value();

    light.m_Attenuation.m_Constant  = m_pdeConstantAttenuation->valuef();
    light.m_Attenuation.m_Linear    = m_pdeLinearAttenuation->valuef();
    light.m_Attenuation.m_Quadratic = m_pdeQuadAttenuation->valuef();

    gl3dView::setLight(light);
}


void GLLightDlg::setParams(void)
{
    Light const &light = gl3dView::light();

    m_peslXLight->setRange(-int(REFLENGTH*100), int(REFLENGTH*100));
    m_peslYLight->setRange(-int(REFLENGTH*100), int(REFLENGTH*100));
    m_peslZLight->setRange(-int(REFLENGTH*100), int(REFLENGTH*100));
    m_peslXLight->setTickInterval(int(REFLENGTH*10.0));
    m_peslYLight->setTickInterval(int(REFLENGTH*10.0));
    m_peslZLight->setTickInterval(int(REFLENGTH*10.0));
    m_peslXLight->setExpValuef(light.m_X*100.0f);
    m_peslYLight->setExpValuef(light.m_Y*100.0f);
    m_peslZLight->setExpValuef(light.m_Z*100.0f);

    m_peslEyePos->setRange(0, int(REFLENGTH*100));
    m_peslEyePos->setTickInterval(int(double(REFLENGTH*10.0)));
    m_peslEyePos->setExpValuef(light.m_EyeDist*100.0f);

    m_pchLight->setChecked(light.m_bIsLightOn);

    m_peslLightAmbient->setExpValuef( light.m_Ambient  *20.0f);
    m_peslLightDiffuse->setExpValuef( light.m_Diffuse  *20.0f);
    m_peslLightSpecular->setExpValuef(light.m_Specular *20.0f);

    m_peslEyePos->setExpValuef(light.m_EyeDist*100.0f);

    m_pslRed->setValue(  int(light.m_Red  *100.0f));
    m_pslGreen->setValue(int(light.m_Green*100.0f));
    m_pslBlue->setValue( int(light.m_Blue *100.0f));

    m_pslMatShininess->setValue(light.m_iShininess);

    m_pdeConstantAttenuation->setValuef(light.m_Attenuation.m_Constant);
    m_pdeLinearAttenuation->setValuef(light.m_Attenuation.m_Linear);
    m_pdeQuadAttenuation->setValuef(light.m_Attenuation.m_Quadratic);

    m_prbOrtho->setChecked(s_bOrtho);
    m_prbPerspective->setChecked(!s_bOrtho);
    m_pdeVerticalAngle->setValue(s_VerticalAngle);
    m_pdeViewDistance->setValue(s_ViewDistance);
    m_pdeViewDistance->setEnabled(!s_bOrtho);
    m_pdeVerticalAngle->setEnabled(!s_bOrtho);

    setLabels();
}


void GLLightDlg::setLabels()
{
    QString strong;
    Light const &light = gl3dView::light();

    strong = QString::asprintf("%7.1f", double(light.m_Ambient));
    m_plabLightAmbient->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Diffuse));
    m_plabLightDiffuse->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Specular));
    m_plabLightSpecular->setText(strong);    strong = QString::asprintf("%7.1f", double(light.m_X)*Units::mtoUnit());

    m_plabposXValue->setText(strong + Units::lengthUnitQLabel());
    strong = QString::asprintf("%7.1f", double(light.m_Y)*Units::mtoUnit());
    m_plabposYValue->setText(strong + Units::lengthUnitQLabel());
    strong = QString::asprintf("%7.1f", double(light.m_Z)*Units::mtoUnit());
    m_plabposZValue->setText(strong + Units::lengthUnitQLabel());

    strong = QString::asprintf("%7.1f", double(light.m_EyeDist)*Units::mtoUnit());
    m_plabEyeDist->setText(strong + Units::lengthUnitQLabel());

    strong = QString::asprintf("%7.1f", double(light.m_Red));
    m_plabLightRed->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Green));
    m_plabLightGreen->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Blue));
    m_plabLightBlue->setText(strong);

    strong = QString::asprintf("%d", light.m_iShininess);
    m_plabMatShininess->setText(strong);
}


bool GLLightDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("GLLightDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("bOrtho", s_bOrtho);
        settings.setValue("ViewDistance", s_ViewDistance);
        settings.setValue("VerticalAngle", s_VerticalAngle);

    }
    settings.endGroup();

    return true;
}


bool GLLightDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("GLLightDlg");
    {
        s_bOrtho        = settings.value("bOrtho", s_bOrtho).toBool();
        s_ViewDistance  = settings.value("ViewDistance", s_ViewDistance).toDouble();
        s_VerticalAngle = settings.value("VerticalAngle", s_VerticalAngle).toDouble();
        s_Geometry      = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
    return true;
}


void GLLightDlg::setDefaults()
{
    gl3dView::setDefaultLength(REFLENGTH);

    s_bOrtho = true;
    s_ViewDistance = 15.0;
    s_VerticalAngle = 60.0;
}


void GLLightDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            close();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void GLLightDlg::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    m_pglView->setLightVisible(gl3dView::isLightOn());
    setParams();
    setEnabled();
}


void GLLightDlg::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    m_pglView->setLightVisible(false);
    s_Geometry = saveGeometry();
}


void GLLightDlg::onLight()
{
    m_pglView->setLightOn(m_pchLight->isChecked());
    setEnabled();
    apply();
}


void GLLightDlg::onViewProjection()
{
    s_bOrtho = m_prbOrtho->isChecked();
    s_VerticalAngle = m_pdeVerticalAngle->value();
    s_ViewDistance   = m_pdeViewDistance->value();

    m_pdeViewDistance->setEnabled(!s_bOrtho);
    m_pdeVerticalAngle->setEnabled(!s_bOrtho);

    apply();
}


void GLLightDlg::setEnabled()
{
    Light const &light = gl3dView::light();

    m_pslRed->setEnabled(light.m_bIsLightOn);
    m_pslGreen->setEnabled(light.m_bIsLightOn);
    m_pslBlue->setEnabled(light.m_bIsLightOn);

    m_peslLightAmbient->setEnabled(light.m_bIsLightOn);
    m_peslLightDiffuse->setEnabled(light.m_bIsLightOn);
    m_peslLightSpecular->setEnabled(light.m_bIsLightOn);

    m_peslXLight->setEnabled(light.m_bIsLightOn);
    m_peslYLight->setEnabled(light.m_bIsLightOn);
    m_peslZLight->setEnabled(light.m_bIsLightOn);

    m_peslEyePos->setEnabled(light.m_bIsLightOn);

    m_pslMatShininess->setEnabled(light.m_bIsLightOn);
}



void GLLightDlg::setgl3dView(gl3dView*pglView)
{
    m_pglView = pglView;
    //    s_RefLength = m_pglView->referenceLength();
}






