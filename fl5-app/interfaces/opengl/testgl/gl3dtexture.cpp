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
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QDesktopServices>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>

#include "gl3dtexture.h"
#include <api/geom_global.h>
#include <api/triangle3d.h>
#include <api/utils.h>
#include <core/displayoptions.h>
#include <api/trace.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/controls/gllightdlg.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>

QByteArray gl3dTexture::s_Geometry;

float gl3dTexture::s_TimeOut = 5.0;
float gl3dTexture::s_UpdatePeriod = 1.0;
QSize gl3dTexture::s_ImgSize(1920,1080);
int gl3dTexture::s_MaxOccupancy(150);

double gl3dTexture::s_a = -1.7;
double gl3dTexture::s_b =  3.5;
double gl3dTexture::s_c = -0.9;
double gl3dTexture::s_d =  1.7;

float gl3dTexture::s_red(1.0f), gl3dTexture::s_green(1.0f), gl3dTexture::s_blue(1.0f);

gl3dTexture::gl3dTexture(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setWindowTitle("Texture");

    m_pImg = nullptr;
    m_bInitialized  = false;
    m_bResetTexture = true;
    m_bAxes = true;
//    setLightOn(false);

    m_bUpdating = false;

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
            connect(pchAxes, &QCheckBox::clicked, this, &gl3dTexture::onAxes);

            QPushButton *ppbLight = new QPushButton("Setup light");
            connect(ppbLight, &QPushButton::clicked, this, &gl3dTexture::onSetupLight);


            QGroupBox *pgbAttractor = new QGroupBox("Attractor settings");
            {
                QGridLayout*pCliffLayout = new QGridLayout;
                {
                    QLabel *plabTimeOut = new QLabel("Time out:");
                    QLabel *plabSecs = new QLabel("s");
                    QLabel *plabUpdate = new QLabel("Update period:");
                    QLabel *plabSecs2 = new QLabel("s");
                    m_pfeTimeOut = new FloatEdit(s_TimeOut);
                    m_pfeTimeOut->setToolTip("<p>Defines the time during which the attractor will run</p>");
                    m_pfeUpdate = new FloatEdit(s_UpdatePeriod);

//                    connect(m_pfeTimeOut, &FloatEdit::floatChanged, this, &gl3dTexture::onReadParams);
                    connect(m_pfeUpdate,  &FloatEdit::floatChanged, this, &gl3dTexture::onReadParams);

                    QLabel *plaba = new QLabel("a=");
                    QLabel *plabb = new QLabel("b=");
                    QLabel *plabc = new QLabel("c=");
                    QLabel *plabd = new QLabel("d=");
                    m_pfea = new FloatEdit(s_a);
                    m_pfeb = new FloatEdit(s_b);
                    m_pfec = new FloatEdit(s_c);
                    m_pfed = new FloatEdit(s_d);

                    m_ppbClear = new QPushButton("Clear");
                    connect(m_ppbClear, SIGNAL(clicked()), SLOT(onClear()));

                    m_ppbStart = new QPushButton("Start");
                    connect(m_ppbStart, SIGNAL(clicked()), SLOT(onContinue()));



                    pCliffLayout->addWidget(plabTimeOut,     2, 1);
                    pCliffLayout->addWidget(m_pfeTimeOut,    2, 2);
                    pCliffLayout->addWidget(plabSecs,        2, 3);

                    pCliffLayout->addWidget(plabUpdate,      3, 1);
                    pCliffLayout->addWidget(m_pfeUpdate,     3, 2);
                    pCliffLayout->addWidget(plabSecs2,       3, 3);

                    pCliffLayout->addWidget(plaba,           5, 1);
                    pCliffLayout->addWidget(m_pfea,          5, 2);

                    pCliffLayout->addWidget(plabb,           6, 1);
                    pCliffLayout->addWidget(m_pfeb,          6, 2);

                    pCliffLayout->addWidget(plabc,           7, 1);
                    pCliffLayout->addWidget(m_pfec,          7, 2);

                    pCliffLayout->addWidget(plabd,           8, 1);
                    pCliffLayout->addWidget(m_pfed,          8, 2);


                    pCliffLayout->addWidget(m_ppbClear,      16,1,1,2);
                    pCliffLayout->addWidget(m_ppbStart,      17,1,1,2);
                    pCliffLayout->setColumnStretch(4,1);
                    pCliffLayout->setRowStretch(17,1);
                }

                pgbAttractor->setLayout(pCliffLayout);
            }

            QGroupBox *pgbImage = new QGroupBox("Image processing");
            {
                QVBoxLayout *pImageLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pSizeLayout = new QHBoxLayout;
                    {
                        QLabel *plabImgWidth = new QLabel("Image size=");
                        QLabel *plabTimes = new QLabel(TIMESch);
                        QLabel *plabPixel = new QLabel("pixels");
                        m_pieWidth  = new IntEdit(s_ImgSize.width());
                        m_pieHeight = new IntEdit(s_ImgSize.height());
                        connect(m_pieWidth,  &IntEdit::intChanged, this, &gl3dTexture::onResizeImage);
                        connect(m_pieHeight, &IntEdit::intChanged, this, &gl3dTexture::onResizeImage);
                        pSizeLayout->addWidget(plabImgWidth);
                        pSizeLayout->addWidget(m_pieWidth);
                        pSizeLayout->addWidget(plabTimes);
                        pSizeLayout->addWidget(m_pieHeight);
                        pSizeLayout->addWidget(plabPixel);
                        pSizeLayout->addStretch();
                    }

                    QGridLayout*pColorLayout = new QGridLayout;
                    {
                        QLabel *plabMaxOcc = new QLabel("Max. occupancy:");

                        m_plabMaxOcc = new QLabel("0 / ");
                        m_plabMaxOcc->setFont(DisplayOptions::tableFont());

                        m_pieMaxOcc = new IntEdit(s_MaxOccupancy);
                        m_pieMaxOcc->setToolTip("<p>Normalization factor for the occupancies.<br>"
                                                "Recommendation: ...</p>");
                        connect(m_pieMaxOcc, &IntEdit::intChanged, this, &gl3dTexture::updateImg);

                        m_pslRed = new QSlider(Qt::Horizontal);
                        m_pslRed->setRange(0, 100);
                        m_pslRed->setTickInterval(20);
                        m_pslRed->setTickPosition(QSlider::TicksBelow);
                        m_pslRed->setValue(int(s_red*100.0f));

                        m_pslGreen = new QSlider(Qt::Horizontal);
                        m_pslGreen->setRange(0, 100);
                        m_pslGreen->setTickInterval(20);
                        m_pslGreen->setTickPosition(QSlider::TicksBelow);
                        m_pslGreen->setValue(int(s_green*100.0f));

                        m_pslBlue = new QSlider(Qt::Horizontal);
                        m_pslBlue->setRange(0, 100);
                        m_pslBlue->setTickInterval(20);
                        m_pslBlue->setTickPosition(QSlider::TicksBelow);
                        m_pslBlue->setValue(int(s_blue*100.0f));


                        QLabel *plabRed   = new QLabel("Red:");
                        QLabel *plabGreen = new QLabel("Green:");
                        QLabel *plabBlue  = new QLabel("Blue:");

                        connect(m_pslRed,   &QSlider::sliderReleased, this, &gl3dTexture::updateImg);
                        connect(m_pslGreen, &QSlider::sliderReleased, this, &gl3dTexture::updateImg);
                        connect(m_pslBlue,  &QSlider::sliderReleased, this, &gl3dTexture::updateImg);

                        m_ppbSaveImg = new QPushButton(QString::asprintf("Save 2d image %dx%d", s_ImgSize.width(), s_ImgSize.height()));
                        connect(m_ppbSaveImg, &QPushButton::clicked, this, &gl3dTexture::onSaveImg);

                        QPushButton *ppbOpenImg = new QPushButton("Open saved image");
                        connect(ppbOpenImg, &QPushButton::clicked, this, &gl3dTexture::onOpenImg);

                        pColorLayout->addWidget(plabMaxOcc,      5, 1);
                        pColorLayout->addWidget(m_plabMaxOcc,    5, 2, Qt::AlignRight);
                        pColorLayout->addWidget(m_pieMaxOcc,     5, 3);

                        pColorLayout->addWidget(plabRed,         7, 1);
                        pColorLayout->addWidget(m_pslRed,        7, 2,1,2);
                        pColorLayout->addWidget(plabGreen,       8, 1);
                        pColorLayout->addWidget(m_pslGreen,      8, 2,1,2);
                        pColorLayout->addWidget(plabBlue,        9, 1);
                        pColorLayout->addWidget(m_pslBlue,       9, 2,1,2);

                        pColorLayout->addWidget(m_ppbSaveImg,    11,1,1,2);
                        pColorLayout->addWidget(ppbOpenImg,      12,1,1,2);
                        pColorLayout->setColumnStretch(2,1);
                        pColorLayout->setRowStretch(13,1);
                    }
                    pImageLayout->addLayout(pSizeLayout);
                    pImageLayout->addLayout(pColorLayout);
                }
                pgbImage->setLayout(pImageLayout);
            }
            m_plabInfo = new QLabel;
            m_plabInfo->setFont(DisplayOptions::tableFont());
            m_plabInfo->setMinimumHeight(DisplayOptions::tableFontStruct().height()*3);
            m_plabInfo->setWordWrap(true);


            pFrameLayout->addWidget(pchAxes);
            pFrameLayout->addWidget(ppbLight);
            pFrameLayout->addWidget(pgbAttractor);
            pFrameLayout->addWidget(pgbImage);
            pFrameLayout->addWidget(m_plabInfo);
            pFrameLayout->addStretch();
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

//    makeTestTexture();

    onResizeImage();

//    connect(this, &Attractor2d::updateImg,    this, &Attractor2d::onUpdateImg, Qt::DirectConnection); // do not update btns which belong to the app's thread
    connect(this, &gl3dTexture::taskFinished, this, &gl3dTexture::onTaskFinished, Qt::QueuedConnection);
}


void gl3dTexture::onTaskFinished()
{
    QApplication::restoreOverrideCursor();
    m_plabInfo->setText(QString("N steps = %L1").arg(m_NSteps));
    updateBtns(true);
}


void gl3dTexture::updateBtns(bool bStart)
{
    if(bStart)
    {
        m_ppbStart->setText(tr("Start/continue"));

        m_ppbClear->setEnabled(true);
        m_ppbSaveImg->setEnabled(true);
    }
    else
    {
        m_ppbStart->setText(tr("Stop"));
        m_ppbClear->setEnabled(false);
        m_ppbSaveImg->setEnabled(false);
    }
}


void gl3dTexture::onReadParams()
{
    s_MaxOccupancy  = m_pieMaxOcc->value();
    s_TimeOut = m_pfeTimeOut->valuef();
    if(s_TimeOut<0.1f) s_TimeOut = 0.1f;
    s_UpdatePeriod = m_pfeUpdate->valuef();

    s_a = m_pfea->value();
    s_b = m_pfeb->value();
    s_c = m_pfec->value();
    s_d = m_pfed->value();
}


void gl3dTexture::onClear()
{
    m_bCancel = true;

    updateBtns(true);

    onReadParams();
    m_pImg->fill(Qt::black);

    m_Occupancy.fill(0);
    m_NSteps = 0;
    updateImg();
    update();
}


void gl3dTexture::onContinue()
{
    if(!m_bIsRunning)
    {
        QApplication::setOverrideCursor(Qt::BusyCursor);
        onReadParams();
        updateBtns(false);

        m_bIsRunning = true;
        m_bCancel = false;

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        QFuture<void> future = QtConcurrent::run(&gl3dTexture::runAttractor, this, this);
#else
        QtConcurrent::run(this, &gl3dTexture::runAttractor, this);
#endif
    }
    else
    {
        QApplication::restoreOverrideCursor();

        m_bCancel = true;
        updateBtns(true);
    }
}


void gl3dTexture::onResizeImage()
{
    s_ImgSize.setWidth(m_pieWidth->value());
    s_ImgSize.setHeight(m_pieHeight->value());
    m_ppbSaveImg->setText(tr("Save 2d image %1x%2").arg(s_ImgSize.width()).arg(s_ImgSize.height()));

    if(m_pImg) delete m_pImg;
    m_pImg = new QImage(s_ImgSize, QImage::Format_ARGB32);
    m_pImg->fill(Qt::black);


    m_Occupancy.resize(m_pImg->width()*m_pImg->height());
    m_Occupancy.fill(0);

}


void gl3dTexture::onSaveImg()
{
    QStringList loc = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if(!loc.isEmpty()) s_LastFileName = loc.front()+QDir::separator();
    s_LastFileName += QString::asprintf("texture2d_a%g_b%g_c%g_d%g.png", s_a, s_b, s_c, s_d);
    m_pImg->save(s_LastFileName, "PNG");
    m_plabInfo ->setText(tr("image saved to:<br>%1").arg(s_LastFileName));
    m_plabInfo->adjustSize();
    setFocus();
}


void gl3dTexture::onOpenImg()
{
    if(!s_LastFileName.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(s_LastFileName));
}


void gl3dTexture::updateImg()
{
    if(m_bUpdating) return; // don't stack update requests

    m_bUpdating = true;

    s_MaxOccupancy = m_pieMaxOcc->value();
    s_red   = float(m_pslRed->value())/100.0f;
    s_green = float(m_pslGreen->value())/100.0f;
    s_blue  = float(m_pslBlue->value())/100.0f;

    // Reading while we're writing!
    int nBlocks = QThread::idealThreadCount();

    int h = m_pImg->height();
    int rowblock = h/nBlocks;

    QFutureSynchronizer<void> futureSync;

    int firstrow(0), lastrow(0);
    for(int iBlock=0; iBlock<nBlocks; iBlock++)
    {
        firstrow = iBlock*rowblock;
        lastrow = (iBlock+1)*rowblock;
        if(iBlock==nBlocks-1)
            lastrow = h; // correct rounding errors;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        futureSync.addFuture(QtConcurrent::run(this, &gl3dTexture::processImgBlock, firstrow, lastrow));
#else
        futureSync.addFuture(QtConcurrent::run(&gl3dTexture::processImgBlock, this, firstrow, lastrow));
#endif
    }
    futureSync.waitForFinished();

    m_bResetTexture = true;

    m_bUpdating = false;
    update();
}


void gl3dTexture::processImgBlock(int rf, int rl)
{
    uchar b[]{0,0,0};
    uchar alpha = 0xff;

    int w = m_pImg->width();
    int h = m_pImg->height();
    int npixels = w*h;

    ushort maxocc = 0;
    for(int ipixel=0; ipixel<m_Occupancy.size(); ipixel++)
        maxocc = std::max(m_Occupancy.at(ipixel), maxocc);

    m_plabMaxOcc->setText(QString::asprintf("%d / ", maxocc));

    if(s_MaxOccupancy<=0) s_MaxOccupancy=maxocc;

    float red(0), green(0), blue(0);

    float tau(0);

    for(int row=rf; row<rl; row++)
    {
        QRgb *pLine = reinterpret_cast<QRgb*>(m_pImg->scanLine(row));
        for(int col=0; col<w; col++)
        {
            QRgb &rgb = pLine[col];
            int ipixel = row*w+col;
            Q_ASSERT(ipixel<npixels);

            if(m_Occupancy.at(ipixel)==0)
            {
                b[0]=b[1]=b[2]=0;
            }
            else
            {
                tau = float(m_Occupancy.at(ipixel))/float(s_MaxOccupancy);
                tau  = std::min(tau, 1.0f);

                red   = tau*s_red;
                green = tau*s_green;
                blue  = tau*s_blue;

                b[0] = red   * 0xff;
                b[1] = green * 0xff;
                b[2] = blue  * 0xff;

//              b[0] = b[1] = b[2] = 0xaa;
            }


            rgb =  qRgba(b[0], b[1], b[2], alpha);
        }
    }
}


void gl3dTexture::cartesianToSpherical(Vector3d const &pos, float &theta, float &phi)
{
//    float r = pos.norm();
    theta = atan2(pos.y, pos.x);
    if(theta<0) theta = 2.0*PI + theta;

    float rproj = sqrt(pos.x*pos.x+pos.y*pos.y);
    phi =  atan2(pos.z, rproj);
}


gl3dTexture::~gl3dTexture()
{
    if(m_pglTexture) delete m_pglTexture;
    if(m_pImg) delete m_pImg;
}


void gl3dTexture::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dTexture");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
        s_ImgSize      = settings.value("ImgSize",  s_ImgSize).toSize();
        s_TimeOut      = settings.value("TimeOut",  s_TimeOut).toFloat();
        s_UpdatePeriod = settings.value("Update",   s_UpdatePeriod).toFloat();
        s_MaxOccupancy = settings.value("MaxVal",   s_MaxOccupancy).toInt();
        s_a            = settings.value("a",        s_a).toFloat();
        s_b            = settings.value("b",        s_b).toFloat();
        s_c            = settings.value("c",        s_c).toFloat();
        s_d            = settings.value("d",        s_d).toFloat();

        s_red          = settings.value("red",      s_red).toFloat();
        s_green        = settings.value("green",    s_green).toFloat();
        s_blue         = settings.value("blue",     s_blue).toFloat();

    }
    settings.endGroup();
}


void gl3dTexture::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dTexture");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("ImgSize",  s_ImgSize);
        settings.setValue("TimeOut",  s_TimeOut);
        settings.setValue("Update",   s_UpdatePeriod);
        settings.setValue("MaxVal",   s_MaxOccupancy);
        settings.setValue("a",        s_a);
        settings.setValue("b",        s_b);
        settings.setValue("c",        s_c);
        settings.setValue("d",        s_d);
        settings.setValue("red",      s_red);
        settings.setValue("green",    s_green);
        settings.setValue("blue",     s_blue);
    }
    settings.endGroup();
}


void gl3dTexture::glMakeTexSphere(int nLong, int nLat)
{
    //  contruction that creates triangles with edges along meridians and avoids the texture seam

    float start_lon = 0.0f;
    float start_lat = -PIf/2.0f;

    float lon_incr =  2.0* PIf / (nLong-1);
    float lat_incr =       PIf / (nLat-1);

    int nQuads = (nLong-1) * (nLat-1);
    int nTriangles = nQuads*2;
    int bufferSize =  nTriangles * 3 * 8; // 3 vertices *(3 vtx + 3 normal + 2 texture) components
    QVector<GLfloat> sphereVertexArray(bufferSize);

    float phi(0), theta(0), phi1(0), theta1(0);
    float c(0), s(0), c1(0), s1(0), ct(0), st(0), ct1(0), st1(0);

    int iv=0;
    for (int iLong=0; iLong<nLong-1; iLong++)
    {
        phi  = start_lon + float(iLong)   * lon_incr;
        phi1 = start_lon + float(iLong+1) * lon_incr;

        c  = cosf(phi);
        c1 = cosf(phi1);
        s  = sinf(phi);
        s1 = sinf(phi1);

        for (int iLat=0; iLat<nLat-1; iLat++)
        {
            theta  = start_lat + float(iLat)   * lat_incr;
            theta1 = start_lat + float(iLat+1) * lat_incr;

            ct  = cosf(theta);
            ct1 = cosf(theta1);
            st  = sinf(theta);
            st1 = sinf(theta1);

            //first triangle
            sphereVertexArray[iv++] = c * ct;
            sphereVertexArray[iv++] = s * ct;
            sphereVertexArray[iv++] = st;
            sphereVertexArray[iv++] = c * ct;
            sphereVertexArray[iv++] = s * ct;
            sphereVertexArray[iv++] = st;
            sphereVertexArray[iv++] = phi/2.0/PI;            // map longitude to texture coordinates in [0,1]
            sphereVertexArray[iv++] = (theta+PIf/2.0f)/PIf;   // map latitude to texture coordinates in [0,1]


            sphereVertexArray[iv++] = c1 * ct;
            sphereVertexArray[iv++] = s1 * ct;
            sphereVertexArray[iv++] = st;
            sphereVertexArray[iv++] = c1 * ct;
            sphereVertexArray[iv++] = s1 * ct;
            sphereVertexArray[iv++] = st;
            sphereVertexArray[iv++] = phi1/2.0/PI; // map longitude to texture coordinates in [0,1]
            sphereVertexArray[iv++] = (theta+PIf/2.0f)/PIf;   // map latitude to texture coordinates in [0,1]

            sphereVertexArray[iv++] = c * ct1;
            sphereVertexArray[iv++] = s * ct1;
            sphereVertexArray[iv++] = st1;
            sphereVertexArray[iv++] = c * ct1;
            sphereVertexArray[iv++] = s * ct1;
            sphereVertexArray[iv++] = st1;
            sphereVertexArray[iv++] = phi/2.0/PI; // map longitude to texture coordinates in [0,1]
            sphereVertexArray[iv++] = (theta1+PIf/2.0f)/PIf;   // map latitude to texture coordinates in [0,1]

            //second triangle
            sphereVertexArray[iv++] = c1 * ct;
            sphereVertexArray[iv++] = s1 * ct;
            sphereVertexArray[iv++] = st;
            sphereVertexArray[iv++] = c1 * ct;
            sphereVertexArray[iv++] = s1 * ct;
            sphereVertexArray[iv++] = st;
            sphereVertexArray[iv++] = phi1/2.0/PI;   // map longitude to texture coordinates in [0,1]
            sphereVertexArray[iv++] = (theta+PIf/2.0f)/PIf;   // map latitude to texture coordinates in [0,1]

            sphereVertexArray[iv++] = c1 * ct1;
            sphereVertexArray[iv++] = s1 * ct1;
            sphereVertexArray[iv++] = st1;
            sphereVertexArray[iv++] = c1 * ct1;
            sphereVertexArray[iv++] = s1 * ct1;
            sphereVertexArray[iv++] = st1;
            sphereVertexArray[iv++] = phi1/2.0/PI;  // map longitude to texture coordinates in [0,1]
            sphereVertexArray[iv++] = (theta1+PIf/2.0f)/PI;   // map latitude to texture coordinates in [0,1]

            sphereVertexArray[iv++] = c * ct1;
            sphereVertexArray[iv++] = s * ct1;
            sphereVertexArray[iv++] = st1;
            sphereVertexArray[iv++] = c * ct1;
            sphereVertexArray[iv++] = s * ct1;
            sphereVertexArray[iv++] = st1;
            sphereVertexArray[iv++] = phi/2.0/PI;  // map longitude to texture coordinates in [0,1]
            sphereVertexArray[iv++] = (theta1+PIf/2.0f)/PI;   // map latitude to texture coordinates in [0,1]
        }
    }

    Q_ASSERT(iv==bufferSize);

    m_vboTexSphere.create();
    m_vboTexSphere.bind();
    m_vboTexSphere.allocate(sphereVertexArray.constData(), sphereVertexArray.size() * int(sizeof(GLfloat)));
    m_vboTexSphere.release();
}


void gl3dTexture::glMakeTexSphere(int nSplits)
{
    double radius = 1.0;
    // make vertices
    std::vector<Triangle3d> icotriangles;
    geom::makeSphere(radius, nSplits, icotriangles);

    int bufferSize = int(icotriangles.size());
    bufferSize *= 3;    // 3 vertices for each triangle
    bufferSize *= 8;    // (3 coords + 3 normal components + 2 texcoords) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N;

    float longitude(0), latitude(0);
    float U(0), V(0);
    int iv = 0;
    for(uint it=0; it<icotriangles.size(); it++)
    {
        Triangle3d const &t3d = icotriangles.at(it);
        N.set(t3d.normal());

        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Node const &vtx = t3d.vertexAt(ivtx);
            meshvertexarray[iv++] = vtx.xf();
            meshvertexarray[iv++] = vtx.yf();
            meshvertexarray[iv++] = vtx.zf();

            meshvertexarray[iv++] = vtx.normal().xf();
            meshvertexarray[iv++] = vtx.normal().yf();
            meshvertexarray[iv++] = vtx.normal().zf();

            cartesianToSpherical(vtx, longitude, latitude);
            //inverse equirectangular projection

            // map to texture coordinates in [0,1]
            U = longitude/2.0/PI;
            V = (latitude + PI/2.0)/PI;
            V = 1.0-V;

            meshvertexarray[iv++] = U;
            meshvertexarray[iv++] = V;
        }
    }

    Q_ASSERT(iv==bufferSize);

    if(m_vboTexSphere.isCreated()) m_vboTexSphere.destroy();
    m_vboTexSphere.create();
    m_vboTexSphere.bind();
    m_vboTexSphere.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboTexSphere.release();
}




void gl3dTexture::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void gl3dTexture::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void gl3dTexture::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == OCCUPANCY_EVENT)
    {
        OccupancyEvent const *pOccEvent = dynamic_cast<OccupancyEvent*>(pEvent);
        m_Occupancy = pOccEvent->newStates();

        QString strange = QString::asprintf("Elapsed = %.1f s\n", pOccEvent->elapsed());
        strange += QString("N steps = %L1").arg(m_NSteps);
        m_plabInfo->setText(strange);

        updateImg();

        if(pOccEvent->isFinished())
        {
            updateBtns(true);
        }

        update();
    }
    else
        QWidget::customEvent(pEvent);
}


void gl3dTexture::initializeGL()
{
    gl3dTestGLView::initializeGL();

//        glMakeTexSphere(4);

    glMakeTexSphere(47,51);
//        glMakeIcoSphere(4);

}

void gl3dTexture::glMake3dObjects()
{
    if(m_bResetTexture)
    {
        if(m_pglTexture) delete m_pglTexture;
        m_pglTexture = new QOpenGLTexture(*m_pImg);
        m_bResetTexture = false;
    }
}


void gl3dTexture::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    int stride = 8;

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);

        m_shadSurf.setUniformValue(m_locSurf.m_Light, isLightOn());

        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // doesn't matter, textures are one-sided in OpenGL

        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 1);

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrUV);

        m_vboTexSphere.bind();
        {
            int nTriangles = m_vboTexSphere.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrUV,     GL_FLOAT, 6*sizeof(GLfloat), 2, stride*sizeof(GLfloat));

            m_pglTexture->bind();
            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
        }
        m_vboTexSphere.release();
        glDisable(GL_POLYGON_OFFSET_FILL);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrUV);
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // leave things as they were
        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();

/*
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
        m_vboTexSphere.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(GLfloat));

            int nSegs = m_vboTexSphere.size()/2/stride/int(sizeof(float)); // 2 vertices and (3 position components)

            m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
            m_shadLine.setUniformValue(m_locLine.m_UniColor, Qt::white);
            m_shadLine.setUniformValue(m_locLine.m_Thickness, 1.0f);

            glDrawArrays(GL_LINES, 0, nSegs*2);
            glDisable(GL_LINE_STIPPLE);
        }
        m_vboTexSphere.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();*/


    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dTexture::runAttractor(QWidget *pParent)
{
    QElapsedTimer t;
    t.start();
    int lasttime = 0;

    QVector<ushort> tmpOccupancy = m_Occupancy; // continue existing

    double x(0), y(0), x1(0), y1(0);
    double xmin(0), xmax(2.0*PI);
    double ymin(0), ymax(PI);

    int w = m_pImg->width();
    int h = m_pImg->height();

    double xrange = (xmax-xmin);
    double yrange = (ymax-ymin);

    x = QRandomGenerator::global()->bounded(1.0) * 2.0* PI;
    y = QRandomGenerator::global()->bounded(1.0) * PI;

    bool bFinished(false);
    int m(0), n(0);
    do
    {
        m =  std::round((x-xmin)/xrange*double(w));
        n =  std::round((y-ymin)/yrange*double(h));


        int ipix = n*w+m;

        if(ipix>=0 && ipix<tmpOccupancy.size())
        {
            tmpOccupancy[ipix]++;
        }
        else
        {
//            qDebug()<<"past image borders";
        }

        x1 = fx(x, y);
        y1 = fy(x, y);
/*if(m_NSteps<50) qDebug(" %4d  %4d  %13g  %13g  %13g  %13g", m, n, x, y, x1, y1);*/
        x = x1;
        y = y1;

        m_NSteps++;


        bFinished = t.hasExpired(int(s_TimeOut*1000.0f)) || m_bCancel;
        // post an event every second, but may not give enough time to main thread to process image
        // so that events will stack
        if((t.elapsed()-lasttime)>int(s_UpdatePeriod*1000.0) || bFinished)
        {
            //renew seed - overkill
            x = QRandomGenerator::global()->bounded(1.0) * 2.0* PI;
            y = QRandomGenerator::global()->bounded(1.0) * PI;

            lasttime = t.elapsed();
            OccupancyEvent *pOccupEvent = new OccupancyEvent();

            pOccupEvent->setElapsed(float(t.elapsed())/1000.0f);
            pOccupEvent->setTau(tmpOccupancy);

            if(bFinished)
                pOccupEvent->setFinished();

            qApp->postEvent(pParent, pOccupEvent);
        }
    }
    while (!bFinished);

    m_bIsRunning = false;
    emit taskFinished();
}


// Clifford type functions for latitude and longitude
double gl3dTexture::fx(double x, double y) const
{
//    float x1 = fmod(1.5* (s_a*cos(x*y)+s_b*y*sin(x-y)), 2.0*PI);

//    float x1 = sin(s_a*y) + s_c * cos(s_a*x);
    float x1 = s_a*cos(x+y)+s_b*sin(x-y);

    while(x1<0) {x1 += 2.0f*PIf;}
    return x1;
}


double gl3dTexture::fy(double x, double y) const
{
        float y1 =  sin(s_c*x*y) + cos(s_d*y);
    //float y1 =   cos(s_b*(y+x)) + sin(s_d*y);
    y1 = (tanh(y1) +1.0)/2.0 * PI; // project latitude onto [0,PI]
    return y1;
}












