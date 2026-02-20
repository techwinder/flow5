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



#include <QApplication>
#include <QContextMenuEvent>
#include <QPainter>
#include <QFileDialog>

#include <QAction>
#include <QTimer>


#include "section2dwt.h"

#include <api/pslg2d.h>
#include <api/triangle2d.h>
#include <interfaces/widgets/customdlg/gridsettingsdlg.h>
#include <interfaces/widgets/customdlg/imagedlg.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>
#include <core/displayoptions.h>
#include <core/saveoptions.h>

#define ANIMATIONFRAMES 30



LineStyle Section2dWt::s_HighStyle   = {true, Line::SOLID, 5, fl5Color(0,0,255), Line::NOSYMBOL};
LineStyle Section2dWt::s_SelectStyle = {true, Line::SOLID, 5, fl5Color(255,0,0),  Line::NOSYMBOL};


int Section2dWt::s_nPixelSelection = 11;
bool Section2dWt::s_bAntiAliasing = true;

bool Section2dWt::s_bAnimateTransitions = true;
double Section2dWt::s_SpinDamping = 0.01;

Section2dWt::Section2dWt(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
    setCursor(Qt::CrossCursor);

    m_ViewName = "2d view";

    m_pZoomInAct = m_pResetXScaleAct = m_pResetYScaleAct = m_pResetXYScaleAct = nullptr;
    m_pZoomYAct = m_pZoomLessAct = m_pGridAct = nullptr;
    m_pLoadImage = m_pClearImage = m_pImageSettings = nullptr;

    m_bScaleImageWithView = false;
    m_bFlipH = m_bFlipV = false;

    m_ImageScaleX = m_ImageScaleY = 1.0;

    m_CursorPos.setX(0.0);
    m_CursorPos.setY(0.0);

    m_bZoomPlus      = false;
    m_bZoomYOnly     = false;
    m_bTrans         = false;
    m_bDrag          = false;

    m_bShowLegend    = true;
    m_bIsImageLoaded = false;
    m_bXDown = m_bYDown = false;

    m_pTransitionTimer = nullptr;
    m_iTimerInc = 0;
    m_ScaleInc = m_ScaleYInc = 0.0;

    m_bDynTranslation = m_bDynScaling = false;
    m_ZoomFactor = 1.0;

    m_UnitFactor = 1.0;

    m_fScale    = 1.0;
    m_fRefScale = 1.0;

    m_fScaleY    = 1.0;
    m_ptOffset.rx() = 0;
    m_ptOffset.ry() = 0;

    m_pSection2dContextMenu = nullptr;

    m_plabScaleOutput = new QLabel(this);
    m_plabScaleOutput->setAttribute(Qt::WA_NoSystemBackground);
    m_plabInfoOutput = new QLabel(this);
    m_plabInfoOutput->setAttribute(Qt::WA_NoSystemBackground);

    connect(&m_ResetTimer, SIGNAL(timeout()), SLOT(onResetIncrement()));
    connect(&m_DynTimer,   SIGNAL(timeout()), SLOT(onDynamicIncrement()));
}


void Section2dWt::createBaseActions()
{
    m_ActionList.clear();

    QAction *pInsertPt = new QAction(tr("Insert control point\tShift+Click"), this);
    connect(pInsertPt, SIGNAL(triggered()), this, SLOT(onInsertPt()));
    m_ActionList.append(pInsertPt);

    QAction *pRemovePt = new QAction(tr("Remove control point\tCtrl+Click"), this);
    connect(pRemovePt, SIGNAL(triggered()), this, SLOT(onRemovePt()));
    m_ActionList.append(pRemovePt);

    QAction *pSeparator0 = new QAction(this);
    pSeparator0->setSeparator(true);
    m_ActionList.append(pSeparator0);

    QAction *pSeparator1 = new QAction(this);
    pSeparator1->setSeparator(true);
    m_ActionList.append(pSeparator1);

    m_pGridAct= new QAction(QIcon(":/icons/OnGrid.png"), tr("Grid settings"), this);
    m_pGridAct->setStatusTip(tr("Define the grid settings for the view"));
    m_ActionList.append(m_pGridAct);
    connect(m_pGridAct, SIGNAL(triggered()), this, SLOT(onGridSettings()));

    m_pResetXScaleAct= new QAction(QIcon(":/icons/OnResetXScale.png"), tr("Reset X-scale"), this);
    m_pResetXScaleAct->setStatusTip(tr("Resets the scale to fit the current screen width"));
    connect(m_pResetXScaleAct, SIGNAL(triggered()), this, SLOT(onResetXScale()));

    m_pResetXYScaleAct= new QAction(QIcon(":/icons/OnResetXYScale.png"), tr("Reset scales\tR"), this);
    m_pResetXYScaleAct->setStatusTip(tr("Resets the x and y scales to screen size"));
    m_ActionList.append(m_pResetXYScaleAct);
    connect(m_pResetXYScaleAct, SIGNAL(triggered()), this, SLOT(onResetScales()));

    m_pLoadImage = new QAction(tr("Load"), this);
    connect(m_pLoadImage, SIGNAL(triggered()), this, SLOT(onLoadBackImage()));

    m_pClearImage = new QAction(tr("Clear"), this);
    connect(m_pClearImage, SIGNAL(triggered()), this, SLOT(onClearBackImage()));

    m_pImageSettings = new QAction(tr("Settings"), this);
    connect(m_pImageSettings, SIGNAL(triggered()), this, SLOT(onBackImageSettings()));

    m_pExportToSVG = new QAction(tr("to SVG"), this);
    connect(m_pExportToSVG, SIGNAL(triggered()), this, SLOT(onSaveToSvg()));

    m_pResetYScaleAct= new QAction(tr("Reset Y-scale"), this);
    connect(m_pResetYScaleAct, SIGNAL(triggered()), this, SLOT(onResetYScale()));

    m_pZoomInAct= new QAction(QIcon(":/icons/OnZoomIn.png"), tr("Zoom in"), this);
    m_pZoomInAct->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
    m_pZoomInAct->setCheckable(true);
    connect(m_pZoomInAct, SIGNAL(triggered()), this, SLOT(onZoomIn()));

    m_pZoomLessAct= new QAction(QIcon(":/icons/OnZoomOut.png"), tr("Zoom out"), this);
    m_pZoomLessAct->setStatusTip(tr("Zoom Less"));
    connect(m_pZoomLessAct, SIGNAL(triggered()), this, SLOT(onZoomLess()));

    m_pZoomYAct= new QAction(QIcon(":/icons/OnZoomYScale.png"), tr("Zoom Y-scale only (Y+mouse wheel)"), this);
    m_pZoomYAct->setStatusTip(tr("Zoom Y scale Only"));
    m_pZoomYAct->setCheckable(true);
    connect(m_pZoomYAct, SIGNAL(triggered()), this, SLOT(onZoomYOnly()));
}


void Section2dWt::createBaseContextMenu()
{
    m_pSection2dContextMenu = new QMenu(this);
    for(int iAc=0; iAc<m_ActionList.count(); iAc++)
        m_pSection2dContextMenu->addAction(m_ActionList.at(iAc));
}


void Section2dWt::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,          s_bAntiAliasing);
    painter.setRenderHint(QPainter::TextAntialiasing,      s_bAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, s_bAntiAliasing);

    painter.fillRect(rect(), DisplayOptions::backgroundColor());
    paint(painter);
    painter.save();
    if(m_bZoomPlus&& !m_ZoomRect.isEmpty())
    {
        QRect ZRect = m_ZoomRect.normalized();
        QPen ZoomPen(QColor(100,100,100));
        ZoomPen.setStyle(Qt::DashLine);
        painter.setPen(ZoomPen);
        painter.drawRect(ZRect);
    }
    painter.restore();
}


void Section2dWt::contextMenuEvent(QContextMenuEvent *pEvent)
{
    if(m_pSection2dContextMenu)
    {
        m_pSection2dContextMenu->exec(pEvent->globalPos());
        pEvent->accept();
    }
}


void Section2dWt::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_bZoomPlus)
            {
                releaseZoom();
                pEvent->accept();
            }
            else if(m_bZoomYOnly)
            {
                m_bZoomYOnly = false;
                m_pZoomYAct->setChecked(false);
                pEvent->accept();
            }
            m_bXDown = m_bYDown = false;
            break;
        }
        case Qt::Key_D:
            if(pEvent->modifiers() & Qt::MetaModifier) clearDebugPts();
            update();
            break;
        case Qt::Key_R:
            onResetScales();
            break;
        case Qt::Key_X:
            m_bXDown = true;
            break;
        case Qt::Key_Y:
            m_bYDown = true;
            break;
        case Qt::Key_I:
            if (pEvent->modifiers().testFlag(Qt::ControlModifier) && pEvent->modifiers().testFlag(Qt::ShiftModifier))
            {
                if(!m_bIsImageLoaded)
                {
                    onLoadBackImage();
                }
                else
                {
                    onClearBackImage();
                }
            }
            break;
        default:
            pEvent->ignore();
    }
    pEvent->ignore();
}


void Section2dWt::onBackImageSettings()
{
    QVector<double> values;
    values << m_ImageOffset.x() << m_ImageOffset.y() << m_ImageScaleX << m_ImageScaleY;
    ImageDlg dlg(this, values, m_bScaleImageWithView, m_bFlipH, m_bFlipV);

    connect(&dlg, SIGNAL(imageChanged(bool,bool,bool,QPointF,double,double)), SLOT(onUpdateImageSettings(bool,bool,bool,QPointF,double,double)));

    dlg.exec();

    onUpdateImageSettings(dlg.bScaleWithView(), dlg.bFlipH(), dlg.bFlipV(),
                     dlg.offset(), dlg.xScale(), dlg.yScale());
}


void Section2dWt::onUpdateImageSettings(bool bScaleWithView, bool bFlipH, bool bFlipV, QPointF const& offset, double xscale, double yscale)
{
    m_bScaleImageWithView = bScaleWithView;
    m_bFlipH              = bFlipH;
    m_bFlipV              = bFlipV;
    m_ImageOffset         = offset;
    m_ImageScaleX         = xscale;
    m_ImageScaleY         = yscale;

    QImage img(m_ImagePath);

#if (QT_VERSION < QT_VERSION_CHECK(6, 9, 0))
    if(m_bFlipH||m_bFlipV)
    img = img.mirrored(m_bFlipH, m_bFlipV);
#else
    if(m_bFlipV)
        img = img.flipped(Qt::Vertical);

    if(m_bFlipH)
        img = img.flipped(Qt::Horizontal);
#endif

    m_bIsImageLoaded = m_BackImage.convertFromImage(img);

    update();
}


void Section2dWt::onLoadBackImage()
{
    m_ImagePath = QFileDialog::getOpenFileName(this, "Open Image File",
                                               SaveOptions::lastDirName(),
                                               "Image files (*.png *.jpg *.bmp)");

    QFileInfo fi(m_ImagePath);
    if(fi.exists())
        SaveOptions::setLastDirName(fi.canonicalPath());
    else return;

    QImage img(m_ImagePath);

#if (QT_VERSION < QT_VERSION_CHECK(6, 9, 0))
    if(m_bFlipH||m_bFlipV)
    img = img.mirrored(m_bFlipH, m_bFlipV);
#else
    if(m_bFlipV)
        img = img.flipped(Qt::Vertical);

    if(m_bFlipH)
        img = img.flipped(Qt::Horizontal);
#endif
    m_bIsImageLoaded = m_BackImage.convertFromImage(img);

    onBackImageSettings();
    update();
}


void Section2dWt::onClearBackImage()
{
    m_bIsImageLoaded = false;
    update();
}


void Section2dWt::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_X:
            m_bXDown = false;
            break;
        case Qt::Key_Y:
            m_bYDown = false;
            break;
    }
    pEvent->ignore();
}


void Section2dWt::mouseMoveEvent(QMouseEvent *pEvent)
{
//    if(!hasFocus()) setFocus();

    QPoint point = pEvent->pos();

    m_CursorPos = mousetoReal(point);

    if(m_bZoomPlus && (pEvent->buttons() & Qt::LeftButton))
    {
        // we're zooming in using the rectangle method
        m_ZoomRect.setBottomRight(point);
        update();
        return;
    }
    else if((pEvent->buttons() & Qt::LeftButton) && m_bTrans)
    {
        //translate the view
        m_ptOffset.rx() += point.x() - m_LastPoint.x();
        m_ptOffset.ry() += point.y() - m_LastPoint.y();
        m_LastPoint.rx() = point.x();
        m_LastPoint.ry() = point.y();

        update();
        return;
    }
    else if (pEvent->buttons() & Qt::LeftButton && !m_bZoomPlus)
    {
        // user is dragging the point
        dragSelectedPoint(m_CursorPos.x(), m_CursorPos.y());
//        emit objectModified();
    }
    else if ((pEvent->buttons() & Qt::MiddleButton))
    {
        // user is zooming with mouse button down rather than with wheel
        double scale = m_fScale;

        if(!m_bZoomYOnly)
        {
            if (m_bXDown)
            {
                if(point.y()-m_LastPoint.y()>0)
                {
                    m_fScale  *= 1.02;
                    m_fScaleY /= 1.02;
                }
                else
                {
                    m_fScale  /= 1.02;
                    m_fScaleY *= 1.02;
                }
            }
            else if (m_bYDown)
            {
                if(point.y()-m_LastPoint.y()>0) m_fScaleY *= 1.02;
                else                            m_fScaleY /= 1.02;
            }
            else
            {
                if(point.y()-m_LastPoint.y()>0) m_fScale *= 1.02;
                else                            m_fScale /= 1.02;
            }
        }
        else
        {
            if(point.y()-m_LastPoint.y()>0) m_fScaleY *= 1.02;
            else                            m_fScaleY /= 1.02;
        }

        m_LastPoint = point;

        int a = rect().center().x();
        m_ptOffset.rx() = a + int((m_ptOffset.x()-a)*m_fScale/scale);
    }
    else if(pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        double zoomFactor=1.0;

        if(point.y()-m_LastPoint.y()<0) zoomFactor = 1./1.02;
        else                            zoomFactor = 1.02;

        m_LastPoint = point;
        scaleView(m_LastPoint, zoomFactor);
        setAutoUnits();
    }
    else if(!m_bZoomPlus)
    {
        //not zooming, check if mouse passes over control point and highlight
        if(!m_DynTimer.isActive())
        {
            highlightPoint(m_CursorPos);
        }
    }

    if(!m_DynTimer.isActive()) update();

    pEvent->accept();
}


void Section2dWt::mousePressEvent(QMouseEvent *pEvent)
{
    setFocus();
    if(m_iTimerInc>0) return; //do not interfere with timer
    QPoint point = pEvent->pos();

    stopDynamicTimer();

    // get a reference for mouse movements
    m_PointDown = point;
    m_LastPoint = m_PointDown;

    if(m_bZoomPlus)
    {
        m_ZoomRect.setTopLeft(point);
        m_ZoomRect.setBottomRight(point);
    }
    else if(!m_bZoomPlus && (pEvent->buttons() & Qt::LeftButton))
    {
        if (pEvent->modifiers() & Qt::ShiftModifier)
        {
            onInsertPt();
        }
        else if (pEvent->modifiers() & Qt::ControlModifier)
        {
            onRemovePt();
        }
        else
        {
            //Selects the point
            if(selectPoint(mousetoReal(point))>=0)
            {
                //dragging a point
                QApplication::setOverrideCursor(Qt::ClosedHandCursor);
                m_bDrag = true;
            }
            else
            {
                //dragging the view
                QApplication::setOverrideCursor(Qt::ClosedHandCursor);
                m_bTrans = true;
            }
        }
    }
    else if(!m_bZoomPlus && (pEvent->buttons() & Qt::RightButton))
    {
        selectPoint(mousetoReal(point));
    }

    m_MoveTime.restart();

    pEvent->accept();
}


void Section2dWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    QApplication::restoreOverrideCursor(); // in case a link was clicked
    QPoint point = pEvent->pos();

    if(m_bZoomPlus)
    {
        if(!rect().contains(point))
        {
            releaseZoom();
        }
        else
        {
            m_ZoomRect.setBottomRight(point);
            QRect ZRect = m_ZoomRect.normalized();

            if(!ZRect.isEmpty())
            {
                m_ZoomRect = ZRect;

                double ZoomFactor = std::min(double(rect().width())  / double(m_ZoomRect.width()),
                                             double(rect().height()) / double(m_ZoomRect.height()));

                double newScale = std::min(ZoomFactor*m_fScale, 32.0*m_fRefScale);

                ZoomFactor = std::min(ZoomFactor, newScale/m_fScale);

                double newscale = ZoomFactor*m_fScale;
                int a = rect().center().x();
                int b = rect().center().y();

                int aZoom = int((m_ZoomRect.right() + m_ZoomRect.left())/2);
                int bZoom = int((m_ZoomRect.top()   + m_ZoomRect.bottom())/2);

                //translate view
                QPointF newoffset(m_ptOffset);
                newoffset.rx() += (a - aZoom);
                newoffset.ry() += (b - bZoom);
                //scale view
                newoffset.rx() = int(ZoomFactor * (newoffset.x()-a)+a);
                newoffset.ry() = int(ZoomFactor * (newoffset.y()-b)+b);

                m_ZoomRect.setRight(m_ZoomRect.left()-1);

                if(s_bAnimateTransitions)
                {
                    double inc = double(ANIMATIONFRAMES);
                    QPointF trans(newoffset-m_ptOffset);
                    m_TransIncrement = trans/inc;

                    m_ScaleInc = (newscale-m_fScale)/inc;
                    m_ScaleYInc = (1.0-m_fScaleY)/inc;

                    m_iTimerInc = 0;
                    m_ResetTimer.stop();
                    m_ResetTimer.start(5);
                }
                else
                {
                    m_ptOffset = newoffset;
                    m_fScale = newScale;
                }
            }
            else
            {
                m_ZoomRect.setBottomRight(m_ZoomRect.topLeft());
                releaseZoom();
            }
        }
    }
    else if(m_bDrag)
    {
        // finished dragging a point
        int dx = int(m_PointDown.x())-pEvent->pos().x();
        int dy = int(m_PointDown.y())-pEvent->pos().y();
        if(abs(dx)>0 || abs(dy)>0)
        {
            emit mouseDragReleased();           
        }
    }
    else if(s_bAnimateTransitions && pEvent->button()==Qt::LeftButton)
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<DisplayOptions::moveTimeThreshold() && !m_PointDown.isNull())
        {
            m_Trans = pEvent->pos() - m_PointDown;
            startDynamicTimer();

            m_bDynTranslation = true;
        }
    }

    m_bTrans = false;
    m_bDrag = false;
    update();
    pEvent->accept();
}


void Section2dWt::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    if(!hasFocus()) setFocus();

    if(!s_bAnimateTransitions)
    {
        QPoint center = rect().center();
        m_ptOffset.rx() += -pEvent->pos().x() + center.x();
        m_ptOffset.ry() += -pEvent->pos().y() + center.y();
    }
    else
    {
        QPointF trans(pEvent->pos()- rect().center());
        double inc = double(ANIMATIONFRAMES);
        m_TransIncrement.rx() = -(trans.x())/inc;
        m_TransIncrement.ry() = -(trans.y())/inc;

        m_iTimerInc = 0;
        if(m_pTransitionTimer) delete m_pTransitionTimer;
        m_pTransitionTimer = new QTimer(this);
        connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onTranslationIncrement()));
        m_pTransitionTimer->start(5);//10 ms x 30 times is 0.3 seconds

        update();
    }
}


void Section2dWt::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void Section2dWt::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
    }
    m_bDynTranslation = m_bDynScaling = false;

    setMouseTracking(true);
}

#define INCFRACTION 1.0/10.0

void Section2dWt::onDynamicIncrement()
{
    if(m_bDynTranslation)
    {
        if(sqrt(m_Trans.x()*m_Trans.x()+m_Trans.y()*m_Trans.y())<1.0)
        {
            stopDynamicTimer();
            update();
            return;
        }

        //translate the view
        m_ptOffset.rx() += m_Trans.x()*INCFRACTION;
        m_ptOffset.ry() += m_Trans.y()*INCFRACTION;

        m_Trans *= (1.0-s_SpinDamping);
    }

    if(m_bDynScaling)
    {
        if(fabs(m_ZoomFactor)<10.0)
        {
            stopDynamicTimer();
            update();
            return;
        }

        double scalefactor(1.0-DisplayOptions::scaleFactor()/3.0 * m_ZoomFactor/120.0);

        scaleView(m_LastPoint, scalefactor);
        setAutoUnits();

        m_ZoomFactor *= (1.0-s_SpinDamping);
    }
    update();
}


void Section2dWt::onTranslationIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_iTimerInc = 0;
        m_pTransitionTimer->stop();
        delete m_pTransitionTimer;
        m_pTransitionTimer = nullptr;
        return;
    }

    m_ptOffset += m_TransIncrement;

    update();
    m_iTimerInc++;
}


void Section2dWt::updateScaleLabel()
{
    QString strange;
    if(xfl::isLocalized())
        strange = QString("X-scale = %L1\nY-scale = %L2")
                         .arg(m_fScale/m_fRefScale, 4, 'f', 1)
                         .arg(m_fScaleY*m_fScale/m_fRefScale, 4, 'f', 1);
    else
        strange = QString::asprintf("X-scale = %4.1f\nY-scale = %4.1f", m_fScale/m_fRefScale, m_fScaleY*m_fScale/m_fRefScale);

//    if(hasFocus())
    {
        if(xfl::isLocalized())
            strange += QString("\nx = %L1\ny = %L2")
                             .arg(m_CursorPos.x(), 7, 'g', 4)
                             .arg(m_CursorPos.y(), 7, 'g', 4);
        else
            strange += QString::asprintf("\nx = %7.4g\ny = %7.4g", m_CursorPos.x(), m_CursorPos.y());
    }

    m_plabScaleOutput->setText(strange);
    m_plabScaleOutput->adjustSize();
}


void Section2dWt::setOutputInfo(QString const &info)
{
    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                           DisplayOptions::textFontStruct().pointSize());
    m_plabInfoOutput->setStyleSheet(stylesheet);

    m_plabInfoOutput->setText(info);
    m_plabInfoOutput->adjustSize();
    QPoint pos3(width()-m_plabInfoOutput->width(), 10);
    m_plabInfoOutput->move(pos3);
}


void Section2dWt::resizeEvent(QResizeEvent *pEvent)
{
    QWidget::resizeEvent(pEvent);
    m_plabInfoOutput->adjustSize();
    QPoint pos3(width()-m_plabInfoOutput->width(), 10);
    m_plabInfoOutput->move(pos3);

    m_plabScaleOutput->move(5,5);
}


void Section2dWt::showEvent(QShowEvent *pEvent)
{
    (void)pEvent;
    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                           DisplayOptions::textFontStruct().pointSize());
    m_plabInfoOutput->setStyleSheet(stylesheet);
    m_plabScaleOutput->setStyleSheet(stylesheet);

    resizeEvent(nullptr);
}


void Section2dWt::hideEvent(QHideEvent *pEvent)
{
    stopDynamicTimer();
    QWidget::hideEvent(pEvent);
}


void Section2dWt::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usabel on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(s_bAnimateTransitions && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {
        if(m_bDynScaling && m_ZoomFactor*dy<=0)
        {
            //user has changed his mind
            m_bDynScaling=false;
            m_DynTimer.stop();
        }
        else
        {
            double zoomfactor(1.0);
            if(pEvent->angleDelta().y()>0) zoomfactor = 1.0/(1.0+DisplayOptions::scaleFactor());
            else                           zoomfactor = 1.0+DisplayOptions::scaleFactor();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
            scaleView(pEvent->position(), zoomfactor);
#else
            scaleView(pEvent->posF(), zoomfactor);
#endif
            setAutoUnits();
        }
    }

    update();

    pEvent->accept();
}


void Section2dWt::scaleView(const QPointF &pt, double zoomFactor)
{
    m_ZoomRect.setBottomRight(m_ZoomRect.topLeft());
    releaseZoom();

    double  scale = m_fScale;

    if(!m_bZoomYOnly)
    {
        if (m_bXDown)
        {
            m_fScale  *= zoomFactor;
            m_fScaleY *= 1./zoomFactor;
        }
        else if (m_bYDown) m_fScaleY *= zoomFactor;
        else  m_fScale *= zoomFactor;
    }
    else m_fScaleY *= zoomFactor;

    double a = pt.x();
    double b = pt.y();
    m_ptOffset.rx() = a + (m_ptOffset.x()-a)*m_fScale/scale;
    m_ptOffset.ry() = b + (m_ptOffset.y()-b)*m_fScale/scale;
}


/**
 * Sets the default scale for the display.
 */
void Section2dWt::resetDefaultScale()
{
    //scale is set by user zooming
    m_fRefScale = defaultScale();
    m_fScale = m_fRefScale;
    m_fScaleY = 1.0;

    m_ptOffset = defaultOffset();
}


void Section2dWt::drawGrids(QPainter &painter)
{
    painter.save();

    if(m_Grid.bXAxis())   drawXScale(painter);
    if(m_Grid.bYAxis())   drawYScale(painter);

    //draw grids
    if(m_Grid.bXMajGrid())  drawXGrid(painter, m_ptOffset);
    if(m_Grid.bYMajGrid(0)) drawYGrid(painter, m_ptOffset);
    if(m_Grid.bXMinGrid())  drawXMinGrid(painter);
    if(m_Grid.bYMinGrid(0)) drawYMinGrid(painter);

    painter.restore();
}


void Section2dWt::drawDebugPts(QPainter &painter)
{
    painter.save();

    QPointF pt;
    QPen OutPen;
    OutPen.setColor(Qt::cyan);
    OutPen.setStyle(Qt::SolidLine);
    OutPen.setWidth(1);
    painter.setPen(OutPen);

    for (int i=0; i<m_DebugPts.size(); i++)
    {
        pt.rx() =  m_DebugPts[i].x()*m_fScale           + m_ptOffset.x();
        pt.ry() = -m_DebugPts[i].y()*m_fScale*m_fScaleY + m_ptOffset.y();
        if(rect().contains(int(pt.x()),int(pt.y())))
            xfl::drawSymbol(painter, Line::BIGCROSS, DisplayOptions::backgroundColor(), Qt::cyan, pt);
    }

    painter.restore();
}


void Section2dWt::drawXGrid(QPainter &painter, const QPointF &Offset)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    painter.save();
    QPen GridPen(m_Grid.xMajColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    GridPen.setWidth(m_Grid.xMajWidth());
    painter.setPen(GridPen);

    int YMin = rect().top();
    int YMax = rect().bottom();

    double xunit = m_Grid.xMajUnit()/m_UnitFactor;
    double xt = 0;
    if(m_Grid.bXAxis()) xt -= xunit;

    int iter = 0;
    while(int(xt*m_fScale) + Offset.x()>rect().left() && iter++<500)
    {
        if(int(xt*m_fScale) + Offset.x()<rect().right())
        {
            painter.drawLine(int(xt*m_fScale + Offset.x()), YMin, int(xt*m_fScale + Offset.x()), YMax);

        }
        xt -= xunit;
    }

    xt = xunit;
    iter = 0;
    while(int(xt*m_fScale) + Offset.x()<rect().right() && iter++<500)
    {
        if(int(xt*m_fScale) + Offset.x()>rect().left())
        {
            painter.drawLine(int(xt*m_fScale + Offset.x()), YMin, int(xt*m_fScale + Offset.x()), YMax);
        }

        xt += xunit;
    }

    painter.restore();
}


void Section2dWt::drawYGrid(QPainter &painter, const QPointF &Offset)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;

    double scaley = m_fScale * m_fScaleY;

    painter.save();
    QPen GridPen(m_Grid.yMajColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMajStipple(0)));
    GridPen.setWidth(m_Grid.yMajWidth(0));
    painter.setPen(GridPen);

    int XMin = rect().left();
    int XMax = rect().right();

    double yunit = m_Grid.yMajUnit(0)/m_UnitFactor;
    double yt = 0;
    if(m_Grid.bXAxis()) yt -= yunit;

    int iter = 0;
    while(int(yt*scaley) + Offset.y()>rect().top() && iter++<500)
    {
        if(int(yt*scaley) + Offset.y()<rect().bottom())
        {
            painter.drawLine(XMin, int(yt*scaley + Offset.y()), XMax, int(yt*scaley + Offset.y()));
        }
        yt -= yunit;
    }

    iter = 0;
    yt = yunit;
    while(int(yt*scaley) + Offset.y()<rect().bottom() && iter++<500)
    {
        if(int(yt*scaley) + Offset.y()>rect().top())
        {
            painter.drawLine(XMin, int(yt*scaley + Offset.y()), XMax, int(yt*scaley + Offset.y()));
        }
        yt += yunit;
    }

    painter.restore();
}


void Section2dWt::drawXMinGrid(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    painter.save();
    double scalex = m_fScale;
    QPen GridPen(m_Grid.xMinColor());
    GridPen.setWidth(m_Grid.xMinWidth());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMinStipple()));

    painter.setPen(GridPen);

    int YMin = rect().top();
    int YMax = rect().bottom();

    double xunit = m_Grid.xMinUnit()/m_UnitFactor;
    double xt = -xunit;

    int iter = 0;
    while(int(xt*scalex) + m_ptOffset.x()>rect().left() && iter++<500)
    {
        if(int(xt*scalex) + m_ptOffset.x()<rect().right())
        {
            painter.drawLine(int(xt*scalex + m_ptOffset.x()), YMin, int(xt*scalex + m_ptOffset.x()), YMax);
        }
        xt -= xunit;
    }


    xt = xunit;
    iter = 0;
    while(int(xt*scalex) + m_ptOffset.x()<rect().right() && iter++<500)
    {
        if(rect().left()< int(xt*scalex) + m_ptOffset.x())
        {
            painter.drawLine(int(xt*scalex + m_ptOffset.x()), YMin, int(xt*scalex + m_ptOffset.x()), YMax);
        }
        xt += xunit;
    }

    painter.restore();
}


void Section2dWt::drawYMinGrid(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    painter.save();
    double scaley = m_fScale * m_fScaleY;
    QPen GridPen(m_Grid.yMinColor(0));
    GridPen.setWidth(m_Grid.yMinWidth(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMinStipple(0)));

    painter.setPen(GridPen);

    int XMin = rect().left();
    int XMax = rect().right();

    double yunit = m_Grid.yMinUnit(0)/m_UnitFactor;
    double yt = -yunit;

    int iter = 0;

    while(int(yt*scaley) + m_ptOffset.y()>rect().top() && iter++<500)
    {
        if(int(yt*scaley) + m_ptOffset.y()<rect().bottom())
        {
            painter.drawLine(XMin, int(yt*scaley + m_ptOffset.y()), XMax, int(yt*scaley + m_ptOffset.y()));
        }
        yt -= yunit;
    }

    yt = yunit;
    iter = 0;
    while(int(yt*scaley) + m_ptOffset.y()<rect().bottom() && iter++<500)
    {
        if(int(yt*scaley) + m_ptOffset.y()>rect().top())
        {
            painter.drawLine(XMin, int(yt*scaley + m_ptOffset.y()), XMax, int(yt*scaley + m_ptOffset.y()));
        }
        yt += yunit;
    }

    painter.restore();
}


void Section2dWt::drawXScale(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;

    QString format = xfl::isLocalized() ? "%L1" : "%1";

    double scalex = m_fScale;
    painter.save();

    painter.setFont(DisplayOptions::textFontStruct().font());
    int dD = DisplayOptions::textFontStruct().height();
    QString strLabel = QString(format).arg(0.25,0,'f', m_Grid.nDecimals());

    int dW = int((DisplayOptions::textFontStruct().width(strLabel)*4)/16);
    int TickSize = int(dD*3/4);
    int offy = int(m_ptOffset.y());
    offy = std::max(offy,0); //pixels
    offy = std::min(offy, rect().height()-(dD*7/4)-2); //pixels

//    painter.translate(m_ptOffset.x(), offy);

    QPen AxisPen(m_Grid.xAxisColor());
    AxisPen.setStyle(xfl::getStyle(m_Grid.xAxisStipple()));
    AxisPen.setWidth(m_Grid.xAxisWidth());
    painter.setPen(AxisPen);

    QPen LabelPen(DisplayOptions::textColor());

    int XMin = rect().left();
    int XMax = rect().right();

    double xunit = m_Grid.xMajUnit()/m_UnitFactor;
    double xt = 0;

    painter.drawLine(XMin, offy, XMax, offy);

    // ticks and labels for x<0

    int iter = 0;
    int xu = int(xt*scalex + m_ptOffset.x());
    while(xu>XMin && iter++<500)
    {
        xu = int(xt*scalex + m_ptOffset.x());
        if(xu<XMax)
        {
            painter.setPen(AxisPen);
            painter.drawLine(xu, offy, xu, offy+TickSize);
            painter.setPen(LabelPen);
            xfl::drawLabel(painter, xu-dW, offy+(dD*7)/4, xt*m_UnitFactor, Qt::AlignHCenter);
        }
        xt -= xunit;
    }

    // ticks and labels for x>0
    if(m_ptOffset.x()<XMin)
    {
        // skip tickmarks left of the view rectangle
        int nt = int((double(XMin)-m_ptOffset.x())/scalex/xunit);
        xt = nt * xunit;
    }
    else xt = xunit; // start at origin +1 unit
    xu = int(xt*scalex + m_ptOffset.x());
    iter = 0;
    while(xu<XMax && iter++<500)
    {
        xu = int(xt*scalex + m_ptOffset.x());
        if(xu>XMin)
        {
            painter.setPen(AxisPen);
            painter.drawLine(xu, offy, xu, offy+TickSize);
            painter.setPen(LabelPen);
            xfl::drawLabel(painter, xu-dW, offy+(dD*7)/4, xt*m_UnitFactor, Qt::AlignHCenter);
        }
        xt += xunit;
    }

    painter.restore();
}


void Section2dWt::drawYScale(QPainter &painter)
{
    if(qIsNaN(m_fScale) || qIsInf(m_fScale)) return;
    double scaley = m_fScale * m_fScaleY;
    painter.save();
    painter.setFont(DisplayOptions::textFontStruct().font());

    int dD = DisplayOptions::textFontStruct().height();

    int height3  = int(dD/3);

    int TickSize = int(dD*3/4);
    int offx = int(m_ptOffset.x());
    offx = std::max(offx, 0);
    offx = std::min(offx, rect().width()-(TickSize*12)/8-DisplayOptions::textFontStruct().width("-1.0 10-4"));

    QPen AxisPen(m_Grid.yAxisColor());
    AxisPen.setStyle(xfl::getStyle(m_Grid.yAxisStipple()));
    AxisPen.setWidth(m_Grid.yAxisWidth());
    QPen LabelPen(DisplayOptions::textColor());

    int YMin  = rect().top();
    int YMax = rect().bottom();


    double yunit = m_Grid.yMajUnit(0)/m_UnitFactor;

    double yt = -yunit;

    painter.setPen(AxisPen);
    painter.drawLine(offx, YMin, offx, YMax);

    int iter = 0;

    while(int(yt*scaley) + m_ptOffset.y()>YMin && iter<500)
    {
        int yu = int(yt*scaley + m_ptOffset.y());
        if(yu<YMax)
        {
            painter.setPen(AxisPen);
            painter.drawLine(offx, yu, offx+TickSize, yu);
            yu += height3;
            painter.setPen(LabelPen);
            xfl::drawLabel(painter, offx+(TickSize*12)/8, yu, -yt*m_UnitFactor, Qt::AlignLeft);
        }
        yt -= yunit;
        iter++;
    }

    yt = yunit;
    iter = 0;
    while(int(yt*scaley) + m_ptOffset.y()<YMax  && iter<500)
    {
        int yu = int(yt*scaley + m_ptOffset.y());
        if(yu>YMin)
        {
            painter.setPen(AxisPen);
            painter.drawLine(offx, yu, offx+TickSize, yu);
            yu += height3;

            painter.setPen(LabelPen);
            xfl::drawLabel(painter, offx+(TickSize*12)/8, yu, -yt*m_UnitFactor, Qt::AlignLeft);
        }
        yt += yunit;
        iter++;
    }

    painter.restore();
}


void Section2dWt::drawTriangles(QPainter &painter, QVector<Triangle2d> const &t2d,
                                  int linewidth, QColor lineclr, QColor fillclr)
{
    painter.save();
    QPen pen(lineclr);
    pen.setWidth(linewidth);
    painter.setPen(pen);
    QBrush br(fillclr);
    painter.setBrush(br);
    QPolygon poly;
    for(int it=0; it<t2d.size(); it++)
    {
        poly.clear();
        for(int iv=0; iv<3; iv++)
        {
            int x = int(t2d.at(it).vertex(iv).x * m_fScale              + m_ptOffset.x());
            int y = int(t2d.at(it).vertex(iv).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());

            poly.append({x,y});
        }
        painter.drawPolygon(poly);
    }
    painter.restore();
}


void Section2dWt::drawPSLG(QPainter &painter, PSLG2d const &s2d, bool bVertices)
{
    for(int is=0; is<int(s2d.size()); is++)
    {
        int x0 = int(s2d.at(is).vertexAt(0).x * m_fScale              + m_ptOffset.x());
        int y0 = int(s2d.at(is).vertexAt(0).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());
        int x1 = int(s2d.at(is).vertexAt(1).x * m_fScale              + m_ptOffset.x());
        int y1 = int(s2d.at(is).vertexAt(1).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());
        painter.drawLine(x0,y0,x1,y1);
        if(bVertices)
        {
            painter.drawEllipse(QPointF(x0,y0),4,4);
            painter.drawEllipse(QPointF(x1,y1),4,4);
        }
    }
}


void Section2dWt::drawTriangle(QPainter &painter, const Triangle2d &t2d,
                                 int linewidth, QColor lineclr, QColor fillclr)
{
    painter.save();
    QPen pen(lineclr);
    pen.setWidth(linewidth);
    painter.setPen(pen);
    QBrush br(fillclr);
    painter.setBrush(br);
    QPolygon poly;
    for(int iv=0; iv<3; iv++)
    {
        int x = int(t2d.vertex(iv).x * m_fScale              + m_ptOffset.x());
        int y = int(t2d.vertex(iv).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());

        poly.append({x,y});
    }
    painter.drawPolygon(poly);
    painter.restore();
}


void Section2dWt::drawSegment(QPainter &painter, const Segment2d &seg2d,
                                int linewidth, QColor lineclr)
{
    painter.save();
    QPen pen(lineclr);
    pen.setWidth(linewidth);
    painter.setPen(pen);
    int x0 = int(seg2d.vertexAt(0).x * m_fScale              + m_ptOffset.x());
    int y0 = int(seg2d.vertexAt(0).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());
    int x1 = int(seg2d.vertexAt(1).x * m_fScale              + m_ptOffset.x());
    int y1 = int(seg2d.vertexAt(1).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());
    painter.drawLine(x0,y0,x1,y1);
    painter.restore();
}


void Section2dWt::setAutoUnits()
{
    double unit=0;

    setAutoUnit(rect(), m_fScale, m_ptOffset.x(), m_UnitFactor, unit);
    m_Grid.setXMajUnit(unit);

    setAutoUnit(rect(), m_fScale*m_fScaleY, m_ptOffset.x(), m_UnitFactor, unit);
    m_Grid.setYMajUnit(0,unit);

    m_Grid.setXMinUnit(m_Grid.xMajUnit()/5.0);
    m_Grid.setYMinUnit(0, m_Grid.yMajUnit(0)/5.0);
}


void Section2dWt::setAutoUnit(QRect const &r, double scale, double offsetX, double unitfactor, double &unit) const
{
    int nTicks = r.width()/DisplayOptions::textFontStruct().averageCharWidth()/10;

/*    if     (nTicks>=10) nTicks = 10; // number of ticks in the view
    else*/
         if(nTicks>=5)  nTicks = 5;
    else if(nTicks>=2)  nTicks = 2;
    else                nTicks = 1;

    int XMin = r.left();
    int XMax = r.right();
    double xmin = (XMin-offsetX)/scale;
    double xmax = (XMax-offsetX)/scale;

    unit = (xmax-xmin)/double(nTicks) * unitfactor;

    int exponent = 0;

    if (unit <1.0)
    {
        exponent = int(log10(unit *1.00001)-1);
//        exponent = std::max(-4, exponent);
    }
    else exponent = int(log10(unit *1.00001));
    int main = int(unit /pow(10.0, exponent)*1.000001);


    if(main<2)
        unit  = pow(10.0, exponent);
    else if (main<5)
        unit  = 2.0*pow(10.0, exponent);
    else
        unit  = 5.0*pow(10.0, exponent);
}


void Section2dWt::releaseZoom()
{
    m_bZoomPlus = false;

    m_ZoomRect.setRight(m_ZoomRect.left()-1);
    m_ZoomRect.setTop(m_ZoomRect.bottom()+1);
    if(m_pZoomInAct) m_pZoomInAct->setChecked(false);
}


QPointF Section2dWt::mousetoReal(QPointF const &point)
{
    double x =  (point.x() - m_ptOffset.x())/m_fScale;
    double y = -(point.y() - m_ptOffset.y())/m_fScale/m_fScaleY;
    return QPointF(x,y);
}


void Section2dWt::onResetXScale()
{
    stopDynamicTimer();

    resetDefaultScale();
    setAutoUnits();

    releaseZoom();
    update();
}


void Section2dWt::onResetYScale()
{
    stopDynamicTimer();

    m_fScaleY = 1.0;
    setAutoUnits();
    update();
}


void Section2dWt::onResetScales()
{
    releaseZoom();
    stopDynamicTimer();

    if(!s_bAnimateTransitions)
    {
        resetDefaultScale();
        setAutoUnits();
        update();
    }
    else
    {
        double inc = double(ANIMATIONFRAMES);
        QPointF trans(defaultOffset()-m_ptOffset);
        m_TransIncrement = trans/inc;

        m_ScaleInc = (defaultScale()-m_fScale)/inc;
        m_ScaleYInc = (1.0-m_fScaleY)/inc;

        m_iTimerInc = 0;
        m_ResetTimer.stop();
        m_ResetTimer.start(5);

        update();
    }
}


void Section2dWt::onResetIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_iTimerInc = 0;
        m_ResetTimer.stop();
        setAutoUnits();
        update();
        return;
    }

    m_ptOffset += m_TransIncrement;
    m_fScale += m_ScaleInc;
    m_fScaleY += m_ScaleYInc;

    setAutoUnits();
    update();
    m_iTimerInc++;
}


void Section2dWt::onGridSettings()
{
    GridSettingsDlg dlg;
    dlg.initDialog(m_Grid, true);

    if(dlg.exec() == QDialog::Accepted)
    {
        m_Grid = dlg.grid();
        setAutoUnits();
    }
    update();
}


void Section2dWt::drawBackImage(QPainter &painter)
{
    if(!m_bIsImageLoaded || m_BackImage.isNull()) return;

    painter.save();

    double xscale = m_ImageScaleX;
    double yscale = m_ImageScaleY;
    if(m_bScaleImageWithView)
    {
        xscale *= m_fScale          /m_fRefScale;
        yscale *= m_fScale*m_fScaleY/m_fRefScale;
    }

    int w = int(double(m_BackImage.width())* xscale);
    int h = int(double(m_BackImage.height())* yscale);

    double xtop = 0;
    double ytop = 0;

    if(m_bScaleImageWithView)
    {
        xtop = m_ptOffset.x()- double(m_BackImage.width())  /2.*xscale + m_ImageOffset.x()*xscale;
        ytop = m_ptOffset.y()- double(m_BackImage.height()) /2.*yscale + m_ImageOffset.y()*yscale;
    }

    painter.drawPixmap(int(xtop), int(ytop), w, h, m_BackImage);

    painter.restore();
}


void Section2dWt::onZoomIn()
{
    if(!m_bZoomPlus)
    {
        if(m_fScale/m_fRefScale <32.0)
        {
            m_bZoomPlus = true;
        }
        else
        {
            releaseZoom();
        }
    }
    else
    {
        releaseZoom();
    }
}


void Section2dWt::onZoomYOnly()
{
    m_bZoomYOnly = !m_bZoomYOnly;
}


void Section2dWt::onZoomLess()
{
    releaseZoom();

    double ZoomFactor = 0.41;
    double newScale = std::max(ZoomFactor*m_fScale, m_fRefScale);

    ZoomFactor = std::max(ZoomFactor, newScale/m_fScale);

    double newscale = ZoomFactor*m_fScale;
    int a = rect().center().x();
    int b = rect().center().y();

    QPointF newoffset;
    newoffset.rx() = int(ZoomFactor*(m_ptOffset.x()-a)+a);
    newoffset.ry() = int(ZoomFactor*(m_ptOffset.y()-b)+b);

    if(s_bAnimateTransitions)
    {
        double inc = double(ANIMATIONFRAMES);
        QPointF trans(newoffset-m_ptOffset);
        m_TransIncrement = trans/inc;

        m_ScaleInc = (newscale-m_fScale)/inc;
        m_ScaleYInc = (1.0-m_fScaleY)/inc;

        m_iTimerInc = 0;
        m_ResetTimer.stop();
        m_ResetTimer.start(5);
    }
    else
    {
        m_ptOffset = newoffset;
        m_fScale = newScale;
    }

    update();
}


void Section2dWt::paintTriangle(QPainter &painter, Triangle2d const &t2d, bool bVertices, bool bFill)
{
    painter.save();

    QPolygon poly;
    for(int iv=0; iv<3; iv++)
    {
        int x = int(t2d.vertex(iv).x * m_fScale              + m_ptOffset.x());
        int y = int(t2d.vertex(iv).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());

        poly.append({x,y});
    }
    if(bFill)
    {
        painter.drawPolygon(poly);
    }
    else
    {
        painter.drawPolyline(poly);
    }

    if(bVertices)
    {
        for (int k=0; k<3; k++)
        {
            painter.drawEllipse(poly.point(k),7,7);
        }
    }
    painter.restore();
}


void Section2dWt::paintCircumCircle(QPainter &painter, Triangle2d const &t2d)
{
    double r=0.0;
    Vector2d CC;
    t2d.circumCenter(r, CC);
    if(r>LENGTHPRECISION)
    {
        painter.save();
        QPointF pt;
        pt.rx() = CC.x * m_fScale              + m_ptOffset.x();
        pt.ry() = CC.y * (-m_fScale*m_fScaleY) + m_ptOffset.y();
        QPen pen(Qt::darkRed);
        pen.setStyle(Qt::DashLine);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(pt, int(r*m_fScale), int(r*m_fScale*m_fScaleY));
        painter.restore();

        pen.setStyle(Qt::SolidLine);
        painter.setBrush(Qt::black);
        painter.drawEllipse(pt, 4,4);
    }
}


void Section2dWt::onSaveToSvg()
{
 /*   QString FileName, tempfilepath;

    FileName = m_ViewName;

    FileName = QFileDialog::getSaveFileName(this, "Export view to SVG",
                                            SaveOptions::lastExportDirName()+"/"+FileName+".svg",
                                            "SVG file (*.svg)");

    if(!FileName.length()) return;

    QFileInfo fi(FileName);
    if(fi.suffix().isEmpty() || fi.suffix().compare("svg", Qt::CaseInsensitive)!=0)
        FileName+=".svg";

    SaveOptions::setLastExportDirName(fi.path());
    tempfilepath = QDir::toNativeSeparators(FileName);

    QSvgGenerator generator;
    generator.setFileName(FileName);
    generator.setSize(size());
    generator.setViewBox(QRect(0,0,size().width(), size().height()));
    generator.setTitle(m_ViewName);
    QPainter painter;
    painter.begin(&generator);
    paint(painter);

    // add the legend
    QString strange = m_plabInfoOutput->text();
    if(!strange.isEmpty())
    {
        painter.translate(m_plabInfoOutput->pos());
        painter.fillRect(m_plabInfoOutput->rect(), DisplayOptions::backgroundColor());
        painter.setFont(DisplayOptions::textFontStruct().font());
        QStringList lines = strange.split("\n");
        foreach (QString line, lines)
        {
            painter.translate(0, DisplayOptions::textFontStruct().height());
            painter.drawText(0,0,line);
        }
    }
    painter.end();*/
}
