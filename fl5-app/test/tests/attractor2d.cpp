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


#include <QtConcurrent/QtConcurrent>
#include <QStandardPaths>
#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QGroupBox>
#include <QColorDialog>

#include "attractor2d.h"
#include <api/utils.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>


QByteArray Attractor2d::s_Geometry;

QString Attractor2d::s_LastFileName=QString();

QSize Attractor2d::s_ImgSize(1920,1080);

ushort Attractor2d::s_MaxOccupancy(500);
float Attractor2d::s_TimeOut(5);

bool Attractor2d::s_bDark(true);

double Attractor2d::s_a = -1.7;
double Attractor2d::s_b =  1.7;
double Attractor2d::s_c = -2.5;
double Attractor2d::s_d =  0.7;

float Attractor2d::s_red(1.0f), Attractor2d::s_green(1.0f), Attractor2d::s_blue(1.0f);

Attractor2d::Attractor2d(QWidget *parent) : QWidget{parent}
{
    setWindowTitle("Clifford 2d attractor");
    m_bCancel = true; // start in cancelled state
    m_bIsRunning = false;

    m_NSteps = 0;

    m_pImg = nullptr;

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());

    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(255);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    m_pFrame = new QFrame(this);
    {
        m_pFrame->setPalette(palette);
        m_pFrame->setFrameShape(QFrame::NoFrame);
        m_pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout * pFrameLayout = new QVBoxLayout;
        {
            QLabel *plabTitle = new QLabel("Clifford 2d attractor");

            QGroupBox *pgbAttractor = new QGroupBox("Attractor settings");
            {
                QVBoxLayout *pBoxLayout = new QVBoxLayout;
                {
                    QGridLayout*pCliffLayout = new QGridLayout;
                    {
                        QLabel *plabNIter = new QLabel("Time out:");
                        QLabel *plabmSecs = new QLabel("s");
                        m_pfeTimeOut = new FloatEdit(s_TimeOut);
                        m_pfeTimeOut->setToolTip("<p>Defines the time during which the attractor will run</p>");

                        QLabel *plaba = new QLabel("a=");
                        QLabel *plabb = new QLabel("b=");
                        QLabel *plabc = new QLabel("c=");
                        QLabel *plabd = new QLabel("d=");
                        m_pfea = new FloatEdit(s_a);
                        m_pfeb = new FloatEdit(s_b);
                        m_pfec = new FloatEdit(s_c);
                        m_pfed = new FloatEdit(s_d);

                        QString tip = "<p>"
                                      "Suggested sets:<br>"
                                      "<tt>"
                                      "a=-1.70, b= 1.70, c=-2.5,  d= 0.7<br>"
                                      "a=-1.70, b= 1.80, c=-1.90, d= 0.4<br>"
                                      "a= 0.70, b= 1.30, c=-1.40, d= 1.15<br>"
                                      "a= 2.70, b=-1.15, c= 1.75, d= 0.75<br>"
                                      "a= 1.45, b=-1.35, c= 2.30, d= 1.10<br>"
                                      "a= 1.77, b= 1.70, c= 1.20, d= 0.67<br>"
                                      "a=-1.40, b= 1.60, c= 1.00, d= 0.70<br>"
                                      "a=-1.40, b= 1.60, c= 2.00, d=-0.67<br>"
                                      "a=-2.00, b=-1.33, c= 0.95, d= 0.45"
                                      "</tt>"
                                      "</p>";
                        m_pfea->setToolTip(tip);
                        m_pfeb->setToolTip(tip);
                        m_pfec->setToolTip(tip);
                        m_pfed->setToolTip(tip);


                        m_ppbClear = new QPushButton("Clear");
                        connect(m_ppbClear, SIGNAL(clicked()), SLOT(onClear()));

                        m_ppbStart = new QPushButton("Start");
                        connect(m_ppbStart, SIGNAL(clicked()), SLOT(onContinue()));



                        pCliffLayout->addWidget(plabNIter,       3, 1);
                        pCliffLayout->addWidget(m_pfeTimeOut,    3, 2);
                        pCliffLayout->addWidget(plabmSecs,       3, 3);

                        pCliffLayout->addWidget(plaba,           5, 1);
                        pCliffLayout->addWidget(m_pfea,          5, 2);

                        pCliffLayout->addWidget(plabb,           6, 1);
                        pCliffLayout->addWidget(m_pfeb,          6, 2);

                        pCliffLayout->addWidget(plabc,           7, 1);
                        pCliffLayout->addWidget(m_pfec,          7, 2);

                        pCliffLayout->addWidget(plabd,           8, 1);
                        pCliffLayout->addWidget(m_pfed,          8, 2);


                        pCliffLayout->addWidget(m_ppbClear,      16,1,1,3);
                        pCliffLayout->addWidget(m_ppbStart,      17,1,1,3);
                        pCliffLayout->setColumnStretch(2,1);
                        pCliffLayout->setRowStretch(17,1);
                    }

                    pBoxLayout->addLayout(pCliffLayout);
                }
                pgbAttractor->setLayout(pBoxLayout);
            }

            QGroupBox *pgbImage = new QGroupBox("Image processing");
            {
                QVBoxLayout *pImageLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pWidthLayout = new QHBoxLayout;
                    {
                        QLabel *plabImgWidth = new QLabel("Image size=");
                        QLabel *plabTimes = new QLabel(TIMESch);
                        QLabel *plabPixel = new QLabel("pixels");
                        m_pieWidth  = new IntEdit(s_ImgSize.width());
                        m_pieHeight = new IntEdit(s_ImgSize.height());
                        pWidthLayout->addWidget(plabImgWidth);
                        pWidthLayout->addWidget(m_pieWidth);
                        pWidthLayout->addWidget(plabTimes);
                        pWidthLayout->addWidget(m_pieHeight);
                        pWidthLayout->addWidget(plabPixel);
                        pWidthLayout->addStretch();
                    }

                    QGridLayout*pColorLayout = new QGridLayout;
                    {
                        connect(m_pieWidth,  &IntEdit::intChanged, this, &Attractor2d::onResizeImage);
                        connect(m_pieHeight, &IntEdit::intChanged, this, &Attractor2d::onResizeImage);

                        QLabel *plabMaxOcc = new QLabel("Max. occupancy:");

                        m_plabMaxOcc = new QLabel("0 / ");
                        m_plabMaxOcc->setFont(DisplayOptions::tableFont());

                        m_pieMaxOcc = new IntEdit(s_MaxOccupancy);
                        m_pieMaxOcc->setToolTip("<p>Normalization factor for the occupancies.<br>"
                                                "Recommendation: ...</p>");
                        connect(m_pieMaxOcc, &IntEdit::intChanged, this, &Attractor2d::updateImg);

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

                        connect(m_pslRed,   &QSlider::sliderReleased, this, &Attractor2d::updateImg);
                        connect(m_pslGreen, &QSlider::sliderReleased, this, &Attractor2d::updateImg);
                        connect(m_pslBlue,  &QSlider::sliderReleased, this, &Attractor2d::updateImg);

                        m_pchDark = new QCheckBox("Dark background");
                        m_pchDark->setChecked(s_bDark);
                        connect(m_pchDark,  &QCheckBox::clicked, this, &Attractor2d::onBackground);
                        connect(m_pchDark,  &QCheckBox::clicked, this, &Attractor2d::updateImg);

                        m_ppbSaveImg = new QPushButton(QString::asprintf("Save 2d image %dx%d", s_ImgSize.width(), s_ImgSize.height()));
                        connect(m_ppbSaveImg, &QPushButton::clicked, this, &Attractor2d::onSaveImg);

                        QPushButton *ppbOpenImg = new QPushButton("Open saved image");
                        connect(ppbOpenImg, SIGNAL(clicked()), SLOT(onOpenImg()));

                        pColorLayout->addWidget(plabMaxOcc,      5, 1);
                        pColorLayout->addWidget(m_plabMaxOcc,    5, 2, Qt::AlignRight);
                        pColorLayout->addWidget(m_pieMaxOcc,     5, 3);

                        pColorLayout->addWidget(plabRed,         7, 1);
                        pColorLayout->addWidget(m_pslRed,        7, 2, 1, 2);
                        pColorLayout->addWidget(plabGreen,       8, 1);
                        pColorLayout->addWidget(m_pslGreen,      8, 2, 1, 2);
                        pColorLayout->addWidget(plabBlue,        9, 1);
                        pColorLayout->addWidget(m_pslBlue,       9, 2, 1, 2);

                        pColorLayout->addWidget(m_pchDark,       10,1, 1, 3);
                        pColorLayout->addWidget(m_ppbSaveImg,    11,1, 1, 3);
                        pColorLayout->addWidget(ppbOpenImg,      12,1, 1, 3);
                        pColorLayout->setColumnStretch(3,1);
                        pColorLayout->setRowStretch(13,1);
                    }

                    pImageLayout->addLayout(pWidthLayout);
                    pImageLayout->addLayout(pColorLayout);
                }
                pgbImage->setLayout(pImageLayout);
            }

            m_plabInfo = new QLabel;
            m_plabInfo->setFont(DisplayOptions::tableFont());
            m_plabInfo->setMinimumHeight(DisplayOptions::tableFontStruct().height()*3);
            m_plabInfo->setWordWrap(true);

            pFrameLayout->addWidget(plabTitle);
            pFrameLayout->addWidget(pgbAttractor);
            pFrameLayout->addWidget(pgbImage);
            pFrameLayout->addWidget(m_plabInfo);
        }

        m_pFrame->setLayout(pFrameLayout);
//        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(m_pFrame, palette);
    }

    onResizeImage();

//    connect(this, &Attractor2d::updateImg,    this, &Attractor2d::onUpdateImg, Qt::DirectConnection); // do not update btns which belong to the app's thread
    connect(this, &Attractor2d::taskFinished, this, &Attractor2d::onTaskFinished, Qt::QueuedConnection);
}


Attractor2d::~Attractor2d()
{
    if(m_pImg) delete m_pImg;
}


void Attractor2d::onSaveImg()
{
    QStringList loc = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if(!loc.isEmpty()) s_LastFileName = loc.front()+QDir::separator();
    s_LastFileName += QString::asprintf("attr2d_a%g_b%g_c%g_d%g.png", s_a, s_b, s_c, s_d);
    m_pImg->save(s_LastFileName, "PNG");
    m_plabInfo ->setText("image saved to:<br>"+s_LastFileName);
    m_plabInfo->adjustSize();
    setFocus();
}


void Attractor2d::onOpenImg()
{
    if(!s_LastFileName.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(s_LastFileName));
}


void Attractor2d::readParams()
{
    s_MaxOccupancy = m_pieMaxOcc->value();
    s_TimeOut = m_pfeTimeOut->value();

    s_a = m_pfea->value();
    s_b = m_pfeb->value();
    s_c = m_pfec->value();
    s_d = m_pfed->value();
}


void Attractor2d::onResizeImage()
{
    s_ImgSize.setWidth(m_pieWidth->value());
    s_ImgSize.setHeight(m_pieHeight->value());
    m_ppbSaveImg->setText(QString::asprintf("Save 2d image %dx%d", s_ImgSize.width(), s_ImgSize.height()));

    if(m_pImg) delete m_pImg;
    m_pImg = new QImage(s_ImgSize, QImage::Format_ARGB32);
    s_bDark = m_pchDark->isChecked();
    m_pImg->fill(s_bDark ? Qt::black : Qt::white);
    m_Occupancy.resize(m_pImg->width()*m_pImg->height());
    m_Occupancy.fill(0);
}


void Attractor2d::onClear()
{
    m_bCancel = true;

    updateBtns(true);

    readParams();

    s_bDark = m_pchDark->isChecked();
    m_pImg->fill(s_bDark ? Qt::black : Qt::white);

    m_Occupancy.fill(0);
    m_NSteps = 0;
    update();
}


void Attractor2d::onBackground()
{
    s_bDark = m_pchDark->isChecked();

    QPalette palette;

    QColor clr = s_bDark ? Qt::black : Qt::white;
    clr.setAlpha(0);
    palette.setColor(QPalette::Window,     clr);
    palette.setColor(QPalette::Base,       clr);

    palette.setColor(QPalette::WindowText, s_bDark ? Qt::white : Qt::black);
    palette.setColor(QPalette::Text,       s_bDark ? Qt::white : Qt::black);

    wt::setWidgetStyle(m_pFrame, palette);
}



void Attractor2d::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void Attractor2d::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void Attractor2d::loadSettings(QSettings &settings)
{
    settings.beginGroup("Attractor2d");
    {
        s_Geometry     = settings.value("WindowGeometry").toByteArray();
        s_ImgSize      = settings.value("ImgSize",  s_ImgSize).toSize();
        s_TimeOut      = settings.value("TimeOut",  s_TimeOut).toFloat();
        s_MaxOccupancy = settings.value("MaxVal",   s_MaxOccupancy).toInt();
        s_a            = settings.value("a",        s_a).toFloat();
        s_b            = settings.value("b",        s_b).toFloat();
        s_c            = settings.value("c",        s_c).toFloat();
        s_d            = settings.value("d",        s_d).toFloat();
        s_red          = settings.value("red",      s_red).toFloat();
        s_green        = settings.value("green",    s_green).toFloat();
        s_blue         = settings.value("blue",     s_blue).toFloat();
        s_bDark        = settings.value("dark",     s_bDark).toBool();
    }
    settings.endGroup();
}


void Attractor2d::saveSettings(QSettings &settings)
{
    settings.beginGroup("Attractor2d");
    {
        settings.setValue("WindowGeometry", s_Geometry);
        settings.setValue("ImgSize",        s_ImgSize);
        settings.setValue("TimeOut",        s_TimeOut);
        settings.setValue("MaxVal",         s_MaxOccupancy);
        settings.setValue("a",              s_a);
        settings.setValue("b",              s_b);
        settings.setValue("c",              s_c);
        settings.setValue("d",              s_d);
        settings.setValue("red",            s_red);
        settings.setValue("green",          s_green);
        settings.setValue("blue",           s_blue);
        settings.setValue("dark",     s_bDark);
    }
    settings.endGroup();
}


void Attractor2d::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.save();
    painter.drawImage(rect(), *m_pImg);
    painter.restore();
}


void Attractor2d::updateBtns(bool bStart)
{
    if(bStart)
    {
        m_ppbStart->setText("Start/continue");

        m_ppbClear->setEnabled(true);
        m_ppbSaveImg->setEnabled(true);
    }
    else
    {
        m_ppbStart->setText("Stop");
        m_ppbClear->setEnabled(false);
        m_ppbSaveImg->setEnabled(false);
    }
}


void Attractor2d::onContinue()
{
    if(!m_bIsRunning)
    {
        QApplication::setOverrideCursor(Qt::BusyCursor);
        readParams();
        updateBtns(false);

        m_bIsRunning = true;
        m_bCancel = false;

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        QFuture<void> future = QtConcurrent::run(&Attractor2d::runAttractor, this, this);
#else
        QtConcurrent::run(this, &Attractor2d::runAttractor, this);
#endif
    }
    else
    {
        QApplication::restoreOverrideCursor();

        m_bCancel = true;
        updateBtns(true);
    }
}


void Attractor2d::runAttractor(QWidget *pParent)
{
    QElapsedTimer t;
    t.start();
    int lasttime = 0;

    QVector<ushort> tmpOccupancy = m_Occupancy; // continue existing

    double x(0), y(0);
    double xmin( 1.e10), ymin( 1.e10);
    double xmax(-1.e10), ymax(-1.e10);

    double x1(0), y1(0);

    x = QRandomGenerator::global()->bounded(1.0);
    y = QRandomGenerator::global()->bounded(1.0);


    // determine approx. width and height of attractor
    for(int i=0; i<5000; i++)
    {
        x1 = fx(x, y);
        y1 = fy(x, y);

        if(fabs(x-x1)<PRECISION && fabs(y-y1)<PRECISION)
        {
//            m_plabInfo->setText("Sequence is stationary"); // m_plabinfo belongs to other thread
            m_bCancel = true;
            m_bIsRunning = false;
            emit taskFinished();
            return;
        }

        x = x1;
        y = y1;

        xmin = std::min(x, xmin);
        ymin = std::min(y, ymin);
        xmax = std::max(x, xmax);
        ymax = std::max(y, ymax);
    }

    int w = m_pImg->width();
    int h = m_pImg->height();

    double marginfactor = 1.025;

    double xrange = (xmax-xmin)*marginfactor;
    double yrange = (ymax-ymin)*marginfactor;
    double xscale = xrange/double(w);
    double yscale = yrange/double(h);

    // center attractor in image
    int xoffset(0), yoffset(0);
    if(xscale>yscale)
    {
        yoffset = (h - int(yrange / xscale))/2;
    }
    else
    {
        xoffset = (w - int(xrange / yscale))/2;
    }

    x = QRandomGenerator::global()->bounded(1.0);
    y = QRandomGenerator::global()->bounded(1.0);

    bool bFinished(false);
    int m(0), n(0);
    do
    {
        if(xscale>yscale)
        {
            m =  std::round((x-xmin*marginfactor)/xrange*double(w));
            n =  std::round((y-ymin*marginfactor)/xrange*double(w));
        }
        else
        {
            m =  std::round((x-xmin*marginfactor)/yrange*double(h));
            n =  std::round((y-ymin*marginfactor)/yrange*double(h));
        }

        m += xoffset;
        n += yoffset;

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

        x = x1;
        y = y1;

        m_NSteps++;


        bFinished = t.hasExpired(int(s_TimeOut*1000.0f)) || m_bCancel;
        // post an event every second, but may not give enough time to main thread to process image
        // so that events will stack
        if((t.elapsed()-lasttime)>1000 || bFinished)
        {
            //renew seed - overkill
//            x = 2.0*QRandomGenerator::global()->bounded(1.0)-1.0;
//            y = 2.0*QRandomGenerator::global()->bounded(1.0)-1.0;

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


void Attractor2d::customEvent(QEvent *pEvent)
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


void Attractor2d::onTaskFinished()
{
    QApplication::restoreOverrideCursor();
    m_plabInfo->setText(QString("N steps = %L1").arg(m_NSteps));
    updateBtns(true);
}


double Attractor2d::fx(double x, double y) const
{
    return sin(s_a*y) + s_c * cos(s_a*x);
}


double Attractor2d::fy(double x, double y) const
{
    return sin(s_b*x) + s_d * cos(s_b*y);
}



void Attractor2d::updateImg()
{
    s_MaxOccupancy = m_pieMaxOcc->value();
    s_red   = float(m_pslRed->value())/100.0f;
    s_green = float(m_pslGreen->value())/100.0f;
    s_blue  = float(m_pslBlue->value())/100.0f;
    s_bDark = m_pchDark->isChecked();

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
        futureSync.addFuture(QtConcurrent::run(this, &Attractor2d::processImgBlock, firstrow, lastrow));
#else
        futureSync.addFuture(QtConcurrent::run(&Attractor2d::processImgBlock, this, firstrow, lastrow));
#endif
    }
    futureSync.waitForFinished();

    update();
}

/*
for (int y = 0; y < image.height(); ++y) {
    QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));
    for (int x = 0; x < image.width(); ++x) {
        QRgb &rgb = line[x];
        rgb = qRgba(qRed(rgb), qGreen(0), qBlue(rgb), qAlpha(rgb));
    }
}*/


void Attractor2d::processImgBlock(int rf, int rl)
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
                if(s_bDark)
                {
                    b[0] = 0x00;
                    b[1] = 0x00;
                    b[2] = 0x00;
                }
                else
                {
                    b[0] = 0xff;
                    b[1] = 0xff;
                    b[2] = 0xff;
                }

            }
            else
            {
                float occ = std::min(s_MaxOccupancy, m_Occupancy.at(ipixel));
                tau = float(occ)/float(s_MaxOccupancy);
                tau  = std::min(tau, 1.0f);

                if(s_bDark)
                {
                    red   = tau*s_red;
                    green = tau*s_green;
                    blue  = tau*s_blue;
                }
                else
                {
                    red   = 1.0f - (1.0f-s_red)  *tau;
                    green = 1.0f - (1.0f-s_green)*tau;
                    blue  = 1.0f - (1.0f-s_blue) *tau;
                }
                b[0] = red   * 0xff;
                b[1] = green * 0xff;
                b[2] = blue  * 0xff;

            }
            rgb =  qRgba(b[0], b[1], b[2], alpha);

        }
    }
}





