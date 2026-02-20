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

#include <QMenu>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QAction>
#include <QGroupBox>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QOpenGLContext>
#include <QPushButton>
#include <QRadioButton>
#include <QFontMetrics>
#include <QSplitter>
#include <QSurfaceFormat>
#include <QVBoxLayout>

#include "opengldlg.h"

#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/testgl/gl2dfractal.h>
#include <interfaces/opengl/testgl/gl2dquat.h>
#include <interfaces/opengl/testgl/gl2dnewton.h>
#include <interfaces/opengl/testgl/gl3dtestglview.h>
#include <interfaces/opengl/testgl/gl3dflowvtx.h>
#include <interfaces/opengl/testgl/gl3dlorenz.h>
#include <interfaces/opengl/testgl/gl3dlorenz2.h>
#include <interfaces/opengl/testgl/gl3dattractors.h>
#include <interfaces/opengl/testgl/gl3dboids.h>
#include <interfaces/opengl/testgl/gl3dboids2.h>
#include <interfaces/opengl/testgl/gl3dhydrogen.h>
#include <interfaces/opengl/testgl/gl3dsolarsys.h>
#include <interfaces/opengl/testgl/gl3dsagittarius.h>
#include <interfaces/opengl/testgl/gl3dspace.h>
#include <api/trace.h>

QByteArray OpenGlDlg::s_Geometry;
QByteArray OpenGlDlg::s_HSplitterSizes;

int OpenGlDlg::s_iView = 2;

struct Version {
    const char *str;
    int major;
    int minor;
};


static struct Version versions[] = {
    { "1.0", 1, 0 },
    { "1.1", 1, 1 },
    { "1.2", 1, 2 },
    { "1.3", 1, 3 },
    { "1.4", 1, 4 },
    { "1.5", 1, 5 },
    { "2.0", 2, 0 },
    { "2.1", 2, 1 },
    { "3.0", 3, 0 },
    { "3.1", 3, 1 },
    { "3.2", 3, 2 },
    { "3.3", 3, 3 },
    { "4.0", 4, 0 },
    { "4.1", 4, 1 },
    { "4.2", 4, 2 },
    { "4.3", 4, 3 },
    { "4.4", 4, 4 },
    { "4.5", 4, 5 },
    { "4.6", 4, 6 }
};


OpenGlDlg::OpenGlDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("OpenGL context info"));
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_bChanged = false;

    setupLayout();
}


void OpenGlDlg::reject()
{
    if(m_bChanged)
    {
        int resp = QMessageBox::question(this, tr("Close"), tr("Discard the changes?"),
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                         QMessageBox::No);
        if(resp == QMessageBox::Yes)
        {
            gl3dView::setXflSurfaceFormat(m_SavedFormat);
        }
        else return;
    }
    QDialog::reject();
}


void OpenGlDlg::onApply()
{
    if(applyChanges()==QMessageBox::Yes)
    {
        readFormat(m_SavedFormat);
        gl3dView::setXflSurfaceFormat(m_SavedFormat);
        return;
    }
}


QMessageBox::StandardButton OpenGlDlg::applyChanges()
{
    if(m_bChanged)
    {
        QString strange = tr("Use these settings as the future default?\n(Application restart required)");
        QMessageBox::StandardButton resp = QMessageBox::question(this, tr("Close"), strange,
                                                                 QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                                                 QMessageBox::No);
        if(resp == QMessageBox::Yes)
        {
            QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
            readFormat(fmt);
            gl3dView::setXflSurfaceFormat(fmt);
//qDebug()            << gl3dView::defaultXflSurfaceFormat();
            if(xfl::g_bTrace)
            {
                xfl::trace("OpenGLDlg - new default OpenGL context specified:\n");
                printFormat(fmt, strange);
                xfl::trace(strange+"\n");
            }
            m_bChanged = false;
        }
        return resp;
    }
    return QMessageBox::No;
}


void OpenGlDlg::onMultiSampling()
{
    W3dPrefs::setMultiSample(m_pchMultiSampling->isChecked());
}


void OpenGlDlg::onSettingsChanged()
{
    QPair<int, int> oglversion;
    readVersion(oglversion);
    enableControls(oglversion);
}


void OpenGlDlg::readFormat(QSurfaceFormat &fmt)
{
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    QPair<int, int> oglversion;
    readVersion(oglversion);
    fmt.setVersion(oglversion.first, oglversion.second);
    fmt.setOption(QSurfaceFormat::DeprecatedFunctions, m_pchDeprecatedFcts->isChecked());
    fmt.setSamples(m_pieSamples->value());
    if     (m_prbProfiles[0]->isChecked())  fmt.setProfile(QSurfaceFormat::NoProfile);
    else if(m_prbProfiles[1]->isChecked())  fmt.setProfile(QSurfaceFormat::CoreProfile);
    else if(m_prbProfiles[2]->isChecked())  fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
}


void OpenGlDlg::readVersion(QPair<int, int> &oglversion)
{
    QString vtext = m_pcbVersion->currentText();
    float version = vtext.toFloat();
    oglversion.first  = int(version);
    oglversion.second = int(roundf(version*10.0f - float(oglversion.first)*10.0f));
}


void OpenGlDlg::enableControls(QPair<int, int> oglversion)
{
    if(oglversion.first<3)
    {
        m_prbProfiles[0]->setChecked(true);
        m_prbProfiles[1]->setChecked(false);
        m_prbProfiles[2]->setChecked(false);
        m_pchDeprecatedFcts->setChecked(false);
    }
    m_pchDeprecatedFcts->setEnabled(oglversion.first>=3);

    int version = oglversion.first *10 + oglversion.second;
    m_prbProfiles[0]->setEnabled(version>=32);
    m_prbProfiles[1]->setEnabled(version>=32);
    m_prbProfiles[2]->setEnabled(version>=32);

}


void OpenGlDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_W:
            if(bCtrl) reject();
            break;
        case Qt::Key_Escape:
            reject();
            break;
        default:
            QWidget::keyPressEvent(pEvent);
    }
    pEvent->ignore();

    QWidget::keyPressEvent(pEvent);
}


void OpenGlDlg::setupLayout()
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHSplitter = new QSplitter(Qt::Horizontal);
        {
            QGroupBox *pTestContextBox = new QGroupBox("Test OpenGL context");
            {
                QVBoxLayout *pSettingsLayout = new QVBoxLayout;
                {
                    QLabel *pFlow5Link = new QLabel;
                    pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/UI/OpenGL.html>https://flow5.tech/docs/flow5_doc/UI/OpenGL.html</a>");
                    pFlow5Link->setOpenExternalLinks(true);
                    pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                    pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

                    QString strange;
                    strange = QString::asprintf("Current context version requested by the application: OpenGL %d.%d", gl3dView::oglMajor(), gl3dView::oglMinor());
                    QLabel *pCurrentLab = new QLabel(strange);

                    QFont fnt;
                    QFontMetrics fm(fnt);

                    QPushButton *pBtn = new QPushButton("Test context");
                    int h = fm.height()*2;
                    pBtn->setMinimumHeight(h);
                    connect(pBtn, SIGNAL(clicked()), SLOT(onCreateContext()));

                    QLabel *pLabApply = new QLabel("If successful, Apply to make this version the new default.");
                    pLabApply->setStyleSheet("font: bold");

                    QHBoxLayout *pOptionsLayout = new QHBoxLayout;
                    {
                        QGroupBox *pProfGroupBox = new QGroupBox("Profile");
                        {
                            QVBoxLayout *pVBoxLayout = new QVBoxLayout;
                            {
                                m_prbProfiles[0] = new QRadioButton("none (context<=3.0)");
                                m_prbProfiles[1] = new QRadioButton("core (context>=3.2)");
                                m_prbProfiles[2] = new QRadioButton("compatibility (context>=3.2)");
                                for(int i=0; i<3; i++)
                                {
                                    connect(m_prbProfiles[i], SIGNAL(clicked()), SLOT(onSettingsChanged()));
                                    pVBoxLayout->addWidget(m_prbProfiles[i]);
                                }

                                QLabel *pTipLab = new QLabel("<p><b>Recommendation:</b> core profile</p>");
                                pVBoxLayout->addWidget(pTipLab);
                            }
                            pProfGroupBox->setLayout(pVBoxLayout);
                        }

                        QGroupBox *pOptionsBox = new QGroupBox("Options");
                        {
                            QVBoxLayout *pVBoxLayout = new QVBoxLayout;
                            {
                                m_pchDeprecatedFcts = new QCheckBox("Deprecated functions (context>=3.0)");
                                connect(m_pchDeprecatedFcts, SIGNAL(clicked()), SLOT(onSettingsChanged()));

                                m_pchMultiSampling = new QCheckBox("Multi-sampling");
                                connect(m_pchMultiSampling, SIGNAL(clicked()), SLOT(onMultiSampling()));
                                QHBoxLayout *pSamplesLayout = new QHBoxLayout;
                                {
                                    QLabel *pLabSample = new QLabel("Samples:");
                                    m_pieSamples = new IntEdit(4);
                                    m_pieSamples->setMin(1);
                                    QString tip="Sets the number of samples for antialiasing;\n"
                                                "0 = no antialiasing\n"
                                                "Increasing the sampling may slow down slightly the display.\n"
                                                "Recommended value = 4 to 32.";
                                    m_pieSamples->setToolTip(tip);
                                    pSamplesLayout->addWidget(pLabSample);
                                    pSamplesLayout->addWidget(m_pieSamples);
                                    pSamplesLayout->addStretch();
                                    connect(m_pieSamples, SIGNAL(intChanged(int)), SLOT(onSettingsChanged()));
                                }


                                QLabel *pTipLab = new QLabel("<p><b>Recommendation:</b> no deprecated functions</p>");

                                pVBoxLayout->addWidget(m_pchDeprecatedFcts);
                                pVBoxLayout->addWidget(m_pchMultiSampling);
                                pVBoxLayout->addLayout(pSamplesLayout);
                                pVBoxLayout->addWidget(pTipLab);
                                pVBoxLayout->addStretch();
                             }
                            pOptionsBox->setLayout(pVBoxLayout);
                        }
                        pOptionsLayout->addWidget(pProfGroupBox);
                        pOptionsLayout->addWidget(pOptionsBox);
                    }

                    QGridLayout *pCtxVersionLayout = new QGridLayout;
                    {
                        QLabel *pLabel = new QLabel("Context version: ");
                        m_pcbVersion = new QComboBox;
                        connect(m_pcbVersion, SIGNAL(activated(int)), SLOT(onSettingsChanged()));

                        for (size_t i=0; i<sizeof(versions) / sizeof(Version); ++i)
                        {
                            m_pcbVersion->addItem(QString::fromLatin1(versions[i].str));
                        }

                        QLabel *pTipLab = new QLabel("<p><b>Minimal context version:</b> 3.3+<br>"
                                                     "<b>Recommended context version:</b> 4.3+</p>");
                        pCtxVersionLayout->addWidget(pLabel,1,1);
                        pCtxVersionLayout->addWidget(m_pcbVersion,1,2);
                        pCtxVersionLayout->addWidget(pTipLab,2,1,1,2);
                    }

                    m_pptglOutput = new PlainTextOutput;
                    m_pptglOutput->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
                    m_pptglOutput->setReadOnly(true);

                    m_plabMemStatus  = new QLabel("GPU memory usage (NVIDIA only)");
                    m_plabMemStatus->setStyleSheet("font: bold");

                    pSettingsLayout->addWidget(pFlow5Link);
                    pSettingsLayout->addWidget(pCurrentLab);
                    pSettingsLayout->addLayout(pCtxVersionLayout);

                    pSettingsLayout->addLayout(pOptionsLayout);

                    pSettingsLayout->addWidget(pBtn);
                    pSettingsLayout->addWidget(pLabApply);
                    pSettingsLayout->addWidget(m_pptglOutput);
                    pSettingsLayout->setStretchFactor(m_pptglOutput, 1);
                    pSettingsLayout->addWidget(m_plabMemStatus);
                }

                pTestContextBox->setLayout(pSettingsLayout);
            }

            m_pStackWt = new QStackedWidget;
            {
                m_pglTestView = getView(s_iView);
                m_pStackWt->addWidget(m_pglTestView);
                gl2dView *pgl2dTestView = dynamic_cast<gl2dView*>(m_pglTestView);
                if(pgl2dTestView)
                    connect(pgl2dTestView, SIGNAL(ready2d()), SLOT(onRenderWindowReady()));
                gl3dTestGLView *pgl3dTestView = dynamic_cast<gl3dTestGLView*>(m_pglTestView);
                if(pgl3dTestView)
                    connect(pgl3dTestView, SIGNAL(ready()), SLOT(onRenderWindowReady()));
            }

            m_pHSplitter->addWidget(pTestContextBox);
            m_pHSplitter->addWidget(m_pStackWt);
            m_pHSplitter->setStretchFactor(1,1);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
        {
            m_ppbApply = new QPushButton("Apply");
            m_ppbApply->setEnabled(false); // until something changes
            connect(m_ppbApply, SIGNAL(clicked()), SLOT(onApply()));

            m_ppbTestView = new QPushButton(tr("View type"));

            QMenu *pViewSelMenu = new QMenu;
            {
                QAction *pNewtonAct      = new QAction(tr("Newton fractal"),           this);
                QAction *pMandelbrotAct  = new QAction(tr("Mandelbrot"),               this);
                QAction *pQuatAct        = new QAction(tr("Quaternion Julia fractal"), this);
                QAction *pSphereAct      = new QAction(tr("Spheres"),                  this);
                QAction *pFlowAct        = new QAction(tr("Horsehoe vortex flow"),     this);
                QAction *pLorenzAct      = new QAction(tr("Lorenz (CPU)"),             this);
                QAction *pLorenz2Act     = new QAction(tr("Lorenz (GPU)"),             this);
                QAction *pAttractorsAct  = new QAction(tr("Attractors"),               this);
                QAction *pBoidsAct       = new QAction(tr("Boids (CPU)"),              this);
                QAction *pBoids2Act      = new QAction(tr("Boids (GPU)"),              this);
                QAction *pHydrogenAct    = new QAction(tr("Hydrogen atom"),            this);
                QAction *pSolarSysAct    = new QAction(tr("Solar system"),             this);
                QAction *pSagittariusAct = new QAction(tr("Sagittarius A*"),           this);
                QAction *pSpaceAct       = new QAction(tr("The final frontier"),       this);

#ifdef Q_OS_MAC
                pLorenz2Act->setEnabled(false);
                pBoids2Act->setEnabled(false);
#endif

                pNewtonAct->setData(      0);
                pMandelbrotAct->setData(  1);
                pQuatAct->setData(        2);
                pSphereAct->setData(      3);
                pFlowAct->setData(        4);
                pLorenzAct->setData(      5);
                pLorenz2Act->setData(     6);
                pAttractorsAct->setData(  7);
                pBoidsAct->setData(       8);
                pBoids2Act->setData(      9);
                pHydrogenAct->setData(   10);
                pSolarSysAct->setData(   11);
                pSagittariusAct->setData(12);
                pSpaceAct->setData(      13);

                connect(pNewtonAct,      SIGNAL(triggered()), SLOT(onViewType()));
                connect(pMandelbrotAct,  SIGNAL(triggered()), SLOT(onViewType()));
                connect(pQuatAct,        SIGNAL(triggered()), SLOT(onViewType()));
                connect(pSphereAct,      SIGNAL(triggered()), SLOT(onViewType()));
                connect(pFlowAct,        SIGNAL(triggered()), SLOT(onViewType()));
                connect(pLorenzAct,      SIGNAL(triggered()), SLOT(onViewType()));
                connect(pLorenz2Act,     SIGNAL(triggered()), SLOT(onViewType()));
                connect(pAttractorsAct,  SIGNAL(triggered()), SLOT(onViewType()));
                connect(pBoidsAct,       SIGNAL(triggered()), SLOT(onViewType()));
                connect(pBoids2Act,      SIGNAL(triggered()), SLOT(onViewType()));
                connect(pHydrogenAct,    SIGNAL(triggered()), SLOT(onViewType()));
                connect(pSolarSysAct,    SIGNAL(triggered()), SLOT(onViewType()));
                connect(pSagittariusAct, SIGNAL(triggered()), SLOT(onViewType()));
                connect(pSpaceAct,       SIGNAL(triggered()), SLOT(onViewType()));

                pViewSelMenu->addAction(pNewtonAct);
                pViewSelMenu->addAction(pMandelbrotAct);
                pViewSelMenu->addAction(pQuatAct);
                pViewSelMenu->addAction(pSphereAct);
                pViewSelMenu->addAction(pFlowAct);
                pViewSelMenu->addAction(pLorenzAct);
                pViewSelMenu->addAction(pLorenz2Act);
                pViewSelMenu->addAction(pAttractorsAct);
                pViewSelMenu->addAction(pBoidsAct);
                pViewSelMenu->addAction(pBoids2Act);
                pViewSelMenu->addAction(pHydrogenAct);
                pViewSelMenu->addAction(pSolarSysAct);
                pViewSelMenu->addAction(pSagittariusAct);
                pViewSelMenu->addAction(pSpaceAct);

                m_ppbTestView->setMenu(pViewSelMenu);
            }
            m_pButtonBox->addButton(m_ppbApply,    QDialogButtonBox::ActionRole);
            m_pButtonBox->addButton(m_ppbTestView, QDialogButtonBox::ActionRole);
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addWidget(m_pHSplitter);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void OpenGlDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) reject();
}


void OpenGlDlg::onCreateContext()
{
    m_bChanged = true;
    m_ppbApply->setEnabled(true);

    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    readFormat(fmt);
    gl3dView::setXflSurfaceFormat(fmt);

    QString log = "Context creation request with format:\n";
    printFormat(fmt, log, true);

    xfl::trace(log+"\n\n");

    m_pptglOutput->clear();
    m_pStackWt->removeWidget(m_pglTestView);
    delete m_pglTestView;
    m_pglTestView = getView(s_iView);

    gl2dView *pgl2dTestView = dynamic_cast<gl2dView*>(m_pglTestView);
    if(pgl2dTestView)
        connect(pgl2dTestView, SIGNAL(ready2d()), SLOT(onRenderWindowReady()));
    gl3dTestGLView *pgl3dTestView = dynamic_cast<gl3dTestGLView*>(m_pglTestView);
    if(pgl3dTestView)
        connect(pgl3dTestView, SIGNAL(ready()), SLOT(onRenderWindowReady()));
 //    connect(m_pglTestView, SIGNAL(ready()), SLOT(onRenderWindowReady()));

    m_pStackWt->addWidget(m_pglTestView);
    m_pStackWt->setCurrentWidget(m_pglTestView);

    m_pglTestView->update(); // force context initialization

    if (!m_pglTestView->context())
    {
        xfl::trace("Context creation error.................\n");
        m_pptglOutput->appendPlainText("Failed to create context");
        return;
    }
}


QOpenGLWidget *OpenGlDlg::getView(int iView)
{
    switch(iView)
    {
        default:
        case 0:  return new gl2dNewton;
        case 1:  return new gl2dFractal;
        case 2:  return new gl2dQuat;
        case 3:  return new gl3dTestGLView;
        case 4:  return new gl3dFlowVtx;
        case 5:  return new gl3dLorenz;
        case 6:  return new gl3dLorenz2;
        case 7:  return new gl3dAttractors;
        case 8:  return new gl3dBoids;
        case 9:  return new gl3dBoids2;
        case 10: return new gl3dHydrogen;
        case 11: return new gl3dSolarSys;
        case 12: return new gl3dSagittarius;
        case 13: return new gl3dSpace;
    }
    return nullptr;
}


void OpenGlDlg::printFormat(QSurfaceFormat const &format, QString &log, bool bFull)
{
    QString strange;

    log += QString("   OpenGL version: %1.%2\n").arg(format.majorVersion()).arg(format.minorVersion());

    switch(format.renderableType())
    {
        case QSurfaceFormat::DefaultRenderableType:
            strange = "The default";
            break;
        case QSurfaceFormat::OpenGL:
            strange = "Desktop OpenGL rendering";
            break;
        case QSurfaceFormat::OpenGLES:
            strange = "OpenGL ES 2.0 rendering";
            break;
        case QSurfaceFormat::OpenVG:
            strange = "Open Vector Graphics rendering";
            break;
    }
    log += "   Renderable type: " + strange +"\n";

    switch(format.profile())
    {
        case QSurfaceFormat::NoProfile:
            log += "   Profile:         No profile\n";
            break;
        case QSurfaceFormat::CoreProfile:
            log += "   Profile:         Core profile\n";
            break;
        case QSurfaceFormat::CompatibilityProfile:
            log += "   Profile:         Compatibility profile\n";
            break;
    }

    switch(format.swapBehavior())
    {
        case QSurfaceFormat::DefaultSwapBehavior:
            strange = "The default";
            break;
        case QSurfaceFormat::SingleBuffer:
            strange = "Single buffer";
            break;
        case QSurfaceFormat::DoubleBuffer:
            strange = "Double buffer";
            break;
        case QSurfaceFormat::TripleBuffer:
            strange = "Triple buffer";
            break;
    }
    log += "   Swap behaviour:  "+strange + "\n";

    QString opts = "   Options: ";
    if(format.testOption(QSurfaceFormat::StereoBuffers))       opts += " Stereo buffers / ";
    if(format.testOption(QSurfaceFormat::DebugContext))        opts += " DebugContext / ";
    if(format.testOption(QSurfaceFormat::DeprecatedFunctions)) opts += " Deprecated functions / ";
    if(format.testOption(QSurfaceFormat::ResetNotification))   opts += " Reset notification / ";
    log += opts + "\n";

    if(bFull)
    {
        log += (QString("   Depth buffer size   : %1\n").arg(QString::number(format.depthBufferSize())));
        log += (QString("   Stencil buffer size : %1\n").arg(QString::number(format.stencilBufferSize())));
        log += (QString("   Samples             : %1\n").arg(QString::number(format.samples())));
        log += (QString("   Red buffer size     : %1\n").arg(QString::number(format.redBufferSize())));
        log += (QString("   Green buffer size   : %1\n").arg(QString::number(format.greenBufferSize())));
        log += (QString("   Blue buffer size    : %1\n").arg(QString::number(format.blueBufferSize())));
        log += (QString("   Alpha buffer size   : %1\n").arg(QString::number(format.alphaBufferSize())));
        log += (QString("   Swap interval       : %1\n").arg(QString::number(format.swapInterval())));
    }
}


void OpenGlDlg::initDialog()
{
    m_SavedFormat = gl3dView::defaultXflSurfaceFormat();

    // select the active default format in the CbBox
    if(gl3dView::oglMajor()==1)
    {
        if     (gl3dView::oglMinor()==0) m_pcbVersion->setCurrentIndex(0);
        else if(gl3dView::oglMinor()==1) m_pcbVersion->setCurrentIndex(1);
        else if(gl3dView::oglMinor()==2) m_pcbVersion->setCurrentIndex(2);
        else if(gl3dView::oglMinor()==3) m_pcbVersion->setCurrentIndex(3);
        else if(gl3dView::oglMinor()==4) m_pcbVersion->setCurrentIndex(4);
        else if(gl3dView::oglMinor()==5) m_pcbVersion->setCurrentIndex(5);
    }
    else if(gl3dView::oglMajor()==2)
    {
        if     (gl3dView::oglMinor()==0) m_pcbVersion->setCurrentIndex(6);
        else if(gl3dView::oglMinor()==1) m_pcbVersion->setCurrentIndex(7);
    }
    else if(gl3dView::oglMajor()==3)
    {
        if     (gl3dView::oglMinor()==0) m_pcbVersion->setCurrentIndex(8);
        else if(gl3dView::oglMinor()==1) m_pcbVersion->setCurrentIndex(9);
        else if(gl3dView::oglMinor()==2) m_pcbVersion->setCurrentIndex(10);
        else if(gl3dView::oglMinor()==3) m_pcbVersion->setCurrentIndex(11);
    }
    else if(gl3dView::oglMajor()==4)
    {
        if     (gl3dView::oglMinor()==0) m_pcbVersion->setCurrentIndex(12);
        else if(gl3dView::oglMinor()==1) m_pcbVersion->setCurrentIndex(13);
        else if(gl3dView::oglMinor()==2) m_pcbVersion->setCurrentIndex(14);
        else if(gl3dView::oglMinor()==3) m_pcbVersion->setCurrentIndex(15);
        else if(gl3dView::oglMinor()==4) m_pcbVersion->setCurrentIndex(16);
        else if(gl3dView::oglMinor()==5) m_pcbVersion->setCurrentIndex(17);
        else if(gl3dView::oglMinor()==6) m_pcbVersion->setCurrentIndex(18);
    }

    m_prbProfiles[0]->setChecked(gl3dView::defaultXflSurfaceFormat().profile()==QSurfaceFormat::NoProfile);
    m_prbProfiles[1]->setChecked(gl3dView::defaultXflSurfaceFormat().profile()==QSurfaceFormat::CoreProfile);
    m_prbProfiles[2]->setChecked(gl3dView::defaultXflSurfaceFormat().profile()==QSurfaceFormat::CompatibilityProfile);
    m_pieSamples->setValue(gl3dView::defaultXflSurfaceFormat().samples());
    m_pchDeprecatedFcts->setChecked(gl3dView::defaultXflSurfaceFormat().testOption(QSurfaceFormat::DeprecatedFunctions));

    m_pchMultiSampling->setChecked(W3dPrefs::bMultiSample());

    enableControls({gl3dView::oglMajor(), gl3dView::oglMinor()});
}


void OpenGlDlg::onRenderWindowReady()
{
    xfl::trace("OpenGlDlg::onRenderWindowReady\n");

    QString vendor, renderer, version, glslVersion;
    const GLubyte *p=nullptr;

//    glEnable(GL_MULTISAMPLE);

    if ((p = glGetString(GL_VENDOR)))
        vendor = QString::fromLatin1(reinterpret_cast<const char*>(p));
    if ((p = glGetString(GL_RENDERER)))
        renderer = QString::fromLatin1(reinterpret_cast<const char*>(p));
    if ((p = glGetString(GL_VERSION)))
        version = QString::fromLatin1(reinterpret_cast<const char*>(p));
    if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
        glslVersion = QString::fromLatin1(reinterpret_cast<const char*>(p));

    m_pptglOutput->appendPlainText(QString("*** Context information ***"));
    m_pptglOutput->appendPlainText(QString("   Vendor:         %1").arg(vendor));
    m_pptglOutput->appendPlainText(QString("   Renderer:       %1").arg(renderer));
    m_pptglOutput->appendPlainText(QString("   OpenGL version: %1").arg(version));
    m_pptglOutput->appendPlainText(QString("   GLSL version:   %1").arg(glslVersion));

    m_pptglOutput->appendPlainText(QString("\n*** gl3dView::defaultFormat = requested context ***"));
    QString log;
    printFormat(gl3dView::defaultXflSurfaceFormat(), log, true);
    m_pptglOutput->appendPlainText(log);

    QOpenGLContext *pContext = m_pglTestView->context();
    m_pptglOutput->appendPlainText(QString("\n*** QSurfaceFormat from context = response of the OS ***"));
    log.clear();
    printFormat(pContext->format(), log, true);
    m_pptglOutput->appendPlainText(log );

    m_pptglOutput->appendPlainText("\n*** Shaders ***");
    m_pptglOutput->appendPlainText("   Using glsl 330 style shaders\n");

    m_pptglOutput->appendPlainText(QString("*** Qt build information ***"));
    const char *gltype[] = { "Desktop", "GLES 2", "GLES 1" };
    m_pptglOutput->appendPlainText(QString("   Qt OpenGL configuration  : %1")
                     .arg(QString::fromLatin1(gltype[QOpenGLContext::openGLModuleType()])));

    m_pptglOutput->moveCursor(QTextCursor::Start);

    m_pptglOutput->appendPlainText("\n*** OpenGL support: ***");
    QString strange;
    strange = "   Desktop OpenGL  : ";
    qApp->testAttribute(Qt::AA_UseDesktopOpenGL)? strange += "true" : strange+="false";
    m_pptglOutput->appendPlainText(strange);

    strange = "   OpenGL ES       : ";
    qApp->testAttribute(Qt::AA_UseOpenGLES)? strange += "true" : strange+="false";
    m_pptglOutput->appendPlainText(strange);

    strange = "   Software OpenGL : ";
    qApp->testAttribute(Qt::AA_UseSoftwareOpenGL)? strange += "true" : strange+="false";
    m_pptglOutput->appendPlainText(strange+"\n");

    if(m_pglTestView)
    {
        if(vendor.contains("nvidia", Qt::CaseInsensitive))
        {
            int total=0, available=0; // kb
            gl::getMemoryStatus(total, available);

            QString strange;
            strange = QString::asprintf("GPU memory usage: %g/%g MB", double(total-available)/1024, double(total)/1024);
            m_plabMemStatus->setText(strange);
        }
    }
}


void OpenGlDlg::onRenderWindowError(const QString &msg)
{
    m_pptglOutput->appendPlainText(QString("An error has occurred:\n%1").arg(msg));
}


void OpenGlDlg::onViewType()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;
    s_iView = pSenderAction->data().toInt();

    onCreateContext();
}


void OpenGlDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
//    if(m_pglTestView) m_pglTestView->reset3dScale();
}


void OpenGlDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void OpenGlDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("OpenGlDlg");
    {
        s_iView          = settings.value("iView", s_iView).toInt();
        s_Geometry       = settings.value("WindowGeometry").toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSize").toByteArray();
    }
    settings.endGroup();
}


void OpenGlDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("OpenGlDlg");
    {
        settings.setValue("iView",          s_iView);
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("HSplitterSize",  s_HSplitterSizes);
    }
    settings.endGroup();
}
