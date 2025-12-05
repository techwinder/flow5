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

#include <QToolTip>

#include "oppointwt.h"


#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <globals/mainframe.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphdlg.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/curve.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/view2d/paint2d.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/view/section2dwt.h>
#include <modules/xdirect/menus/xdirectmenus.h>
#include <modules/xdirect/xdirect.h>
#include <api/foil.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <api/constants.h>



MainFrame *OpPointWt::s_pMainFrame = nullptr;
XDirect *OpPointWt::s_pXDirect = nullptr;

LineStyle OpPointWt::s_NeutralStyle  = {true, Line::DASHDOT, 1, fl5Color(95,95,95),  Line::NOSYMBOL};
LineStyle OpPointWt::s_PressureStyle = {true, Line::SOLID,   1, fl5Color(75,205,75), Line::NOSYMBOL};
LineStyle OpPointWt::s_BLStyle       = {true, Line::DASH,    1, fl5Color(205,75,75), Line::NOSYMBOL};

bool OpPointWt::s_bFillFoil(false);


OpPointWt::OpPointWt(QWidget *pParent) : QWidget(pParent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    QSizePolicy sizepol;
    sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
    sizepol.setVerticalPolicy(QSizePolicy::Expanding);
    setSizePolicy(sizepol);

    m_bTransFoil   = false;
    m_bTransGraph  = false;
    m_bAnimate     = false;
    m_bBL          = false;
    m_bPressure    = false;

    m_bXDown = m_bYDown = false;

    m_ZoomFactor = 1.0;
    m_bDynTranslation = m_bDynScaling = m_bDynResetting = false;
    m_iResetDyn = 0;
    m_ScaleInc = m_ScaleYInc = 0.0;
    m_iTimerInc = 0;

    m_fScale = m_fScaleY = 1.0;
    m_pCpGraph = nullptr;

    m_pHoverTimer = new QTimer;
    m_pHoverTimer->setSingleShot(true);
    connect(m_pHoverTimer, SIGNAL(timeout()), SLOT(onHovered()));
    connect(&m_DynTimer,   SIGNAL(timeout()), SLOT(onDynamicIncrement()));
    connect(&m_ResetTimer, SIGNAL(timeout()), SLOT(onResetIncrement()));

    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFont().family().toStdString().c_str(),
                                           DisplayOptions::textFont().pointSize());

    m_plabFoilDataOutput = new QLabel(this);
    m_plabFoilDataOutput->setStyleSheet(stylesheet);
    m_plabFoilDataOutput->setTextFormat(Qt::PlainText);
    m_plabFoilDataOutput->setAttribute(Qt::WA_NoSystemBackground);

    m_plabOppDataOutput = new QLabel(this);
    m_plabOppDataOutput->setStyleSheet(stylesheet);
    m_plabOppDataOutput->setTextFormat(Qt::PlainText);
    m_plabOppDataOutput->setWordWrap(false);
    m_plabOppDataOutput->setAlignment(Qt::AlignLeft);
    m_plabOppDataOutput->setAttribute(Qt::WA_NoSystemBackground);
}


OpPointWt::~OpPointWt()
{
    delete m_pHoverTimer;
}


void OpPointWt::setFoilData(QString const &data)
{
    //reset style, just in case settings have changed
    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFont().family().toStdString().c_str(),
                                           DisplayOptions::textFont().pointSize());
    m_plabFoilDataOutput->setStyleSheet(stylesheet);
    m_plabFoilDataOutput->setText(data);
    showEvent(nullptr);
}


void OpPointWt::setOppData(QString const &data)
{
    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFont().family().toStdString().c_str(),
                                           DisplayOptions::textFont().pointSize());
    m_plabOppDataOutput->setStyleSheet(stylesheet);
    m_plabOppDataOutput->setText(data);
    showEvent(nullptr);
}


void OpPointWt::showEvent(QShowEvent *pEvent)
{
    (void)pEvent;
    m_plabFoilDataOutput->adjustSize();
    QPoint pos1(5, height()-m_plabFoilDataOutput->height()-5);
    m_plabFoilDataOutput->move(pos1);

    m_plabOppDataOutput->adjustSize();
    QPoint pos2(width()-5-m_plabOppDataOutput->width(), height()-m_plabOppDataOutput->height()-5);
    m_plabOppDataOutput->move(pos2);
}


void OpPointWt::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    bool bCtrl  = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;
    switch (pEvent->key())
    {
        case Qt::Key_V:
        {
            if(m_rGraphRect.contains(m_LastPoint))
            {
                GraphDlg::setActivePage(0);
                onGraphSettings();
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_G:
        {
            if(m_rGraphRect.contains(m_LastPoint))
            {
                onGraphSettings();
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_R:
        {
            if(m_rGraphRect.contains(m_LastPoint))
            {
                resetGraphScale();
            }
            else
                setFoilScale(true);
            update();
            break;
        }
        case Qt::Key_X:
        {
            m_bXDown  = true;
            break;
        }
        case Qt::Key_Y:
        {
            m_bYDown = true;
            break;
        }
        case Qt::Key_A:
        {
            if(bCtrl)
            {

            }
            break;
        }
        default:pEvent->ignore();
    }
}


void OpPointWt::keyReleaseEvent(QKeyEvent *)
{
    m_bXDown = m_bYDown = false;
}


void OpPointWt::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint pt(pEvent->pos().x(), pEvent->pos().y()); //client coordinates
    stopDynamicTimer();

    if(pEvent->buttons() & Qt::LeftButton)
    {
        if (m_rGraphRect.contains(pEvent->pos()))
        {
            int ipt = -1;
            Curve *pCurve = m_pCpGraph->getCurve(pt, ipt);
            if(pCurve) emit curveClicked(pCurve);
            else       m_bTransGraph = true;
        }
        else
        {
            m_bTransFoil = true;
        }

        m_LastPoint = pt;
        m_LastPressedPt = pt;
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        //        if(!m_bAnimate) update();
    }
    else if(pEvent->buttons() & Qt::RightButton)
    {
        QPoint point = pEvent->pos();
        int ipt = -1;
        Curve *pCurve = m_pCpGraph->getCurve(point, ipt);
        if(pCurve)
        {
            emit curveDoubleClicked(pCurve);
            pEvent->accept();
        }
        else
        {
            s_pXDirect->m_pMenus->m_pOperFoilCtxMenu->exec(QCursor::pos());
            s_pMainFrame->setSavedState(false);
        }
    }
    pEvent->accept();

    m_MoveTime.restart();

}


void OpPointWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    m_bTransGraph = false;
    m_bTransFoil  = false;

    int movetime = m_MoveTime.elapsed();
    if(movetime<DisplayOptions::moveTimeThreshold() && !m_LastPressedPt.isNull())
    {
//        if(m_rGraphRect.contains(pEvent->pos()) && GraphWt::bSpinAnimation() && pEvent->button()==Qt::LeftButton)
        {
            m_Trans = pEvent->pos() - m_LastPressedPt;

            startDynamicTimer();
            m_bDynTranslation = true;
        }
/*        else
        {
            m_Trans = pEvent->pos() - m_PointDown;
            startDynamicTimer();

            m_bDynTranslation = true;
        }*/
    }
    pEvent->accept();
}


void OpPointWt::onHovered()
{
    if(m_pCpGraph && m_rGraphRect.contains(m_LastPoint))
    {
        int ipt = -1;
        Curve *pCurve = m_pCpGraph->getCurve(m_LastPoint, ipt);
        if(pCurve)
        {
            QToolTip::showText(QCursor::pos(), pCurve->name(), this, QRect(), 15000);
        }
    }
}


void OpPointWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    double scale=1.0;
    double a=0.0;

    QPoint pt = pEvent->pos();//client coordinates
    setFocus();
    //    bool bShift = false;
    //    bool bCtrl  = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    //    if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

    if (pEvent->buttons() & Qt::LeftButton)
    {
        if(m_bTransGraph)
        {
            QPoint point = pEvent->pos();

            // curve translation inside the graph
            if (m_pCpGraph->hasRightAxis() && double(m_LastPoint.x())/double(width())<0.15)
            {
                int iy = 0;
                double y1 = m_pCpGraph->clientToy(iy, m_LastPoint.y());
                double yu = m_pCpGraph->clientToy(iy, point.y());
                double ymin = m_pCpGraph->yMin(iy) - yu+y1;
                double ymax = m_pCpGraph->yMax(iy) - yu+y1;
                m_pCpGraph->setYWindow(iy, ymin, ymax);
            }
            else if(m_pCpGraph->hasRightAxis() && double(m_LastPoint.x())/double(width())>0.85)
            {
                int iy = 1;
                double y1 = m_pCpGraph->clientToy(iy, m_LastPoint.y());
                double yu = m_pCpGraph->clientToy(iy, point.y());
                double ymin = m_pCpGraph->yMin(iy) - yu+y1;
                double ymax = m_pCpGraph->yMax(iy) - yu+y1;
                m_pCpGraph->setYWindow(iy, ymin, ymax);
            }
            else
            {
                m_pCpGraph->setAuto(false);
                double x1 = m_pCpGraph->clientTox(m_LastPoint.x());
                double xu = m_pCpGraph->clientTox(point.x());
                double xmin = m_pCpGraph->xMin() - xu+x1;
                double xmax = m_pCpGraph->xMax() - xu+x1;
                m_pCpGraph->setXWindow(xmin, xmax);

                for(int iy=0; iy<2; iy++)
                {
                    double y1 = m_pCpGraph->clientToy(iy, m_LastPoint.y());
                    double yu = m_pCpGraph->clientToy(iy, point.y());
                    double ymin = m_pCpGraph->yMin(iy) - yu+y1;
                    double ymax = m_pCpGraph->yMax(iy) - yu+y1;
                    m_pCpGraph->setYWindow(iy, ymin, ymax);
                }
            }

            m_pCpGraph->invalidate();

        }
        else if(m_bTransFoil)
        {
            // we translate the airfoil
            m_ptOffset.rx() += pt.x()-m_LastPoint.x();
            m_ptOffset.ry() += pt.y()-m_LastPoint.y();
        }
    }
    else if (((pEvent->buttons() & Qt::MiddleButton) || pEvent->modifiers().testFlag(Qt::AltModifier)))
    {
        if (m_rGraphRect.contains(pEvent->pos()))
        {
            //zoom graph
            double zoomfactor = 1.02;
            if(pt.y()-m_LastPoint.y()<0) zoomfactor = 1.0/1.02;
            scaleAxes(-1, zoomfactor);
        }
        else
        {
            scale = m_fScale;

            if(pt.y()-m_LastPoint.y()<0) m_fScale /= 1.02;
            else                         m_fScale *= 1.02;

            a = rect().center().x();

            m_ptOffset.rx() = a + (m_ptOffset.x()-a)/scale*m_fScale;
        }
    }

    m_LastPoint = pt;
    m_pHoverTimer->start(150);
    QToolTip::hideText();

    update();

    pEvent->accept();
}


void OpPointWt::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    if (pEvent->buttons() & Qt::LeftButton)
    {
        if (m_rGraphRect.contains(pEvent->pos()))
        {
            QPoint point = pEvent->pos();
            int ipt = -1;
            Curve *pCurve = m_pCpGraph->getCurve(point, ipt);
            if(pCurve)
                emit curveDoubleClicked(pCurve);
            else
                onGraphSettings();

            update();
        }
    }
}




/**
 * The user has requested an edition of the settings of the active graph
 */
void OpPointWt::onGraphSettings()
{
    GraphDlg grDlg(this);
    grDlg.setGraph(m_pCpGraph);

    //    QAction *action = qobject_cast<QAction *>(sender());
    //    grDlg.setActivePage(0);

    if(grDlg.exec() == QDialog::Accepted)
    {
        emit graphChanged(m_pCpGraph);
    }
}


void OpPointWt::resizeEvent(QResizeEvent *)
{
    if(m_pCpGraph)
    {
        int h = rect().height();
        int h4 = int(h/3.0);
        m_rGraphRect = QRect(0, 0, + rect().width(), rect().height()-h4);

        m_pCpGraph->setGraphScales(m_rGraphRect);
    }
    m_ptOffset = defaultOffset();
    m_fScale = defaultScale();
    m_fScaleY = 1.0;

    showEvent(nullptr);
}


void OpPointWt::resetGraphScale()
{
    if(!m_pCpGraph) return;
    stopDynamicTimer();

    if(!m_pCpGraph) return;
    if(GraphOptions::bSpinAnimation())
    {
        QRect r(rect());
        Graph graph(*m_pCpGraph);
        graph.setCurveModel(m_pCpGraph->curveModel());
        graph.setAuto(true);
        graph.resetLimits();
        graph.setGraphScales(r);
        m_xAxisStart    = m_pCpGraph->xAxis();
        m_yAxisStart[0] = m_pCpGraph->yAxis(0);
        m_yAxisStart[1] = m_pCpGraph->yAxis(1);

        m_xAxisEnd    = graph.xAxis();
        m_yAxisEnd[0] = graph.yAxis(0);
        m_yAxisEnd[1] = graph.yAxis(1);

        m_iResetDyn = 0;
        m_pCpGraph->setAuto(false);

        m_bDynResetting = true;
        startDynamicTimer();
    }
    else
    {
        m_pCpGraph->setAuto(true);
        m_pCpGraph->resetLimits();
        m_pCpGraph->invalidate();
    }

    update();
}




#define INCFRACTION 1.0/10.0
#define ANIMATIONFRAMES 30

void OpPointWt::onDynamicIncrement()
{
    if(m_pCpGraph && m_rGraphRect.contains(m_LastPoint))
    {
        if(m_bDynResetting)
        {
            if(m_iResetDyn>=ANIMATIONFRAMES)
            {
                stopDynamicTimer();
                m_pCpGraph->setAuto(true);
                m_pCpGraph->resetLimits();
                m_pCpGraph->invalidate();
            }
            else
            {
                m_iResetDyn++;
                double tau = double(m_iResetDyn)/double(ANIMATIONFRAMES);
                m_pCpGraph->setXMin(m_xAxisStart.axmin()*(1.0-tau) + m_xAxisEnd.axmin()*tau);
                m_pCpGraph->setXMax(m_xAxisStart.axmax()*(1.0-tau) + m_xAxisEnd.axmax()*tau);

                for(int iy=0; iy<2; iy++)
                {
                    m_pCpGraph->setYMin(iy, m_yAxisStart[iy].axmin()*(1.0-tau) + m_yAxisEnd[iy].axmin()*tau);
                    m_pCpGraph->setYMax(iy, m_yAxisStart[iy].axmax()*(1.0-tau) + m_yAxisEnd[iy].axmax()*tau);
                }
                m_pCpGraph->invalidate();
            }
        }
        else
        {
            if(m_bDynTranslation)
            {
                if(sqrt(m_Trans.x()*m_Trans.x()+m_Trans.y()*m_Trans.y())<1.0)
                {
                    stopDynamicTimer();
                    update();
                    return;
                }

                m_pCpGraph->setAuto(false);
                double dx = m_Trans.x()/m_pCpGraph->xScale()*INCFRACTION;
                double xmin = m_pCpGraph->xMin() - dx;
                double xmax = m_pCpGraph->xMax() - dx;
                m_pCpGraph->setXWindow(xmin, xmax);

                for(int iy=0; iy<2; iy++)
                {
                    double dy = m_Trans.y()/m_pCpGraph->yScale(iy)*INCFRACTION;
                    double ymin = m_pCpGraph->yMin(iy) - dy;
                    double ymax = m_pCpGraph->yMax(iy) - dy;
                    m_pCpGraph->setYWindow(iy, ymin, ymax);
                }
                m_pCpGraph->invalidate();
                m_Trans *= (1.0-GraphOptions::spinDamping());
            }

            if(m_bDynScaling)
            {
                if(abs(m_ZoomFactor)<10)
                {
                    stopDynamicTimer();
                    update();
                    return;
                }

                double scalefactor(1.0-DisplayOptions::scaleFactor()/3.0 * m_ZoomFactor/120);

                int iy = -1; // both axes
                scaleAxes(iy, scalefactor);

                m_ZoomFactor *= (1.0-GraphOptions::spinDamping());
            }
        }
    }
    else
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

            m_Trans *= (1.0-Section2dWt::spinDamping());
        }

        if(m_bDynScaling)
        {
            if(abs(m_ZoomFactor)<10)
            {
                stopDynamicTimer();
                update();
                return;
            }

            double scalefactor(1.0-DisplayOptions::scaleFactor()/3.0 * m_ZoomFactor/120);

            zoomView(m_LastPoint, scalefactor);

            m_ZoomFactor *= (1.0-Section2dWt::spinDamping());
        }
    }
    update();
}


void OpPointWt::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void OpPointWt::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
    }
    m_bDynTranslation = m_bDynScaling = m_bDynResetting = false;
    setMouseTracking(true);
}


void OpPointWt::wheelEvent(QWheelEvent *pEvent)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QPointF const pos = pEvent->position();
#else
    QPointF const pos = pEvent->posF();
#endif

    m_LastPoint = pos;

    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y();

    if(m_pCpGraph && m_rGraphRect.contains(pos))
    {
        if(GraphOptions::bSpinAnimation() && (abs(dy)>120))
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
                double zoomfactor=1.0;
                QPointF pos;
    #if QT_VERSION >= 0x050F00
                pos = pEvent->position();
    #else
                pos = pEvent->pos();
    #endif
                QPoint pt(pos.x(), pos.y()); //client coordinates
                if(pEvent->angleDelta().y()>0) zoomfactor = 1.0/(1.0+DisplayOptions::scaleFactor());
                else                           zoomfactor = 1.0+DisplayOptions::scaleFactor();

                if(m_pCpGraph && rect().contains(pt))
                {
                    int iy = -1; // both axes
                    if (m_pCpGraph->hasRightAxis() && double(pt.x())/double(width())<0.15)
                    {
                        iy = 0; // left axis
                    }
                    else if(m_pCpGraph->hasRightAxis() && double(pt.x())/double(width())>0.85)
                    {
                        iy = 1; // right axis
                    }
                    scaleAxes(iy, zoomfactor);
                }
            }
        }
    }
    else
    {
        if(Section2dWt::bAnimateTransitions() && abs(dy)>120)
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
                zoomView(pEvent->position(), zoomfactor);
    #else
                zoomView(pEvent->posF(), zoomfactor);
    #endif
            }
        }
    }
    update();
}


void OpPointWt::zoomView(const QPointF &pt, double zoomFactor)
{
    double  scale = m_fScale;

    if (m_bXDown)
    {
        m_fScale  *= zoomFactor;
        m_fScaleY *= 1./zoomFactor;
    }
    else if (m_bYDown) m_fScaleY *= zoomFactor;
    else  m_fScale *= zoomFactor;


//    double a = rect().center().x();
//    double b = rect().center().y();
    double a = pt.x();
    double b = pt.y();
    m_ptOffset.rx() = a + (m_ptOffset.x()-a)*m_fScale/scale;
    m_ptOffset.ry() = b + (m_ptOffset.y()-b)*m_fScale/scale;
}


void OpPointWt::scaleAxes(int iy, double zoomfactor)
{
    if(!m_pCpGraph) return;

    double val[]{0,0,0};
    m_pCpGraph->clientToGraph(m_LastPoint, val);
    double y[] = {val[1], val[2]};

    if(m_bXDown && m_bYDown)
    {
        //zoom both
        m_pCpGraph->setAuto(false);
        m_pCpGraph->scaleAxes(val[0], y, 1./zoomfactor);
    }
    else if (m_bXDown)
    {
        //zoom x scale
        m_pCpGraph->setAutoX(false);
        m_pCpGraph->scaleXAxis(val[0], 1./zoomfactor);
    }
    else if(m_bYDown)
    {
        //zoom y scale
        m_pCpGraph->setAutoY(0, false);
        m_pCpGraph->setAutoY(1, false);
        m_pCpGraph->scaleYAxis(0, y[0], 1./zoomfactor);
        m_pCpGraph->scaleYAxis(1, y[1], 1./zoomfactor);
    }
    else
    {
        if(iy>=0 && iy<2)
        {
            m_pCpGraph->setAutoY(iy, false);
            m_pCpGraph->scaleYAxis(iy, y[iy], 1./zoomfactor);
        }
        else
        {
            m_pCpGraph->scaleAxes(val[0], y, 1./zoomfactor);
        }
    }

    m_pCpGraph->setAutoXUnit();
    m_pCpGraph->setAutoYUnit(0);
    m_pCpGraph->setAutoYUnit(1);
    m_pCpGraph->invalidate();
    update();
}


void OpPointWt::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing,          Section2dWt::bAntiAliasing());
    painter.setRenderHint(QPainter::TextAntialiasing,      Section2dWt::bAntiAliasing());
    painter.setRenderHint(QPainter::SmoothPixmapTransform, Section2dWt::bAntiAliasing());

    painter.fillRect(rect(), DisplayOptions::backgroundColor());
    paintGraph(painter);
    paintOpPoint(painter);

    painter.restore();
}


void OpPointWt::paintGraph(QPainter &painter)
{
    if(!m_pCpGraph) return;

    painter.save();

    QFontMetrics fm(DisplayOptions::textFont());
    int fmheight = fm.height();
    int fmWidth  = fm.averageCharWidth();
    //  draw  the graph
    if(m_rGraphRect.width()>200 && m_rGraphRect.height()>150)
    {
        m_pCpGraph->drawGraph(painter, m_rGraphRect);
        QPoint Place(m_rGraphRect.right()-73*fmWidth, m_rGraphRect.top()+fmheight);
        if(m_pCpGraph->isLegendVisible())
            m_pCpGraph->drawLegend(painter, Place);
    }


    if(m_rGraphRect.contains(m_LastPoint) && Graph::bMousePos())
    {
        QPen textpen(DisplayOptions::textColor());
        textpen.setCosmetic(true);

        painter.setPen(textpen);

        painter.drawText(m_rGraphRect.width()-12*fm.averageCharWidth(),
                         m_rGraphRect.top()+fmheight,   QString::asprintf("x = %7.3f", m_pCpGraph->clientTox(m_LastPoint.x())));
        painter.drawText(m_rGraphRect.width()-12*fm.averageCharWidth(),
                         m_rGraphRect.top()+2*fmheight, QString::asprintf("y = %7.3f",m_pCpGraph->clientToy(0, m_LastPoint.y())));
    }
    painter.restore();
}


void OpPointWt::paintOpPoint(QPainter &painter)
{
    Foil const *pFoil = XDirect::curFoil();
    if (!pFoil) return;

    Polar const *pCurPolar = XDirect::curPolar();
    OpPoint const *pCurOpp = XDirect::curOpp();

    painter.save();

    if(!m_rGraphRect.contains(m_LastPoint) && Graph::bMousePos())
    {
        QPen textpen(DisplayOptions::textColor());
        textpen.setCosmetic(true);
        QFontMetrics fm(DisplayOptions::textFont());
        int fmheight  = fm.height();
        painter.setPen(textpen);

        Vector2d real = mousetoReal(m_LastPoint);
        painter.drawText(m_rGraphRect.width()-12*fm.averageCharWidth(),
                         m_rGraphRect.height() + fmheight, QString::asprintf("x = %7.3f", real.x));
        painter.drawText(m_rGraphRect.width()-12*fm.averageCharWidth(),
                         m_rGraphRect.height() + 2*fmheight, QString::asprintf("y = %7.3f",real.y));
    }

    double Alpha = 0.0;
    if(pCurOpp) Alpha = pCurOpp->aoa();
    else if(pCurPolar)
    {
        if(pCurPolar->isType4() || pCurPolar->isType6()) Alpha = pCurPolar->aoaSpec();
    }

    QPen foilpen;
    foilpen.setCosmetic(true);
    foilpen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
    foilpen.setWidth(pFoil->lineWidth());
    foilpen.setStyle(xfl::getStyle(pFoil->lineStipple()));
    painter.setPen(foilpen);

    QColor clr =  xfl::fromfl5Clr(pFoil->lineColor());
    clr.setAlpha(75);

    xfl::drawFoil(painter, pFoil, -Alpha, 0.0, m_fScale, m_fScale*m_fScaleY, m_ptOffset, s_bFillFoil, clr);

    if(pFoil->pointStyle()>0)
        xfl::drawFoilPoints(painter, pFoil, -Alpha, m_fScale,m_fScale*m_fScaleY, m_ptOffset,
                            DisplayOptions::backgroundColor(), rect());

    if(m_bPressure && pCurOpp) paintPressure(painter, m_fScale, m_fScale*m_fScaleY);
    if(m_bBL && pCurOpp)
    {
        if (pCurOpp->isXFoil()) paintBLXFoil(painter, pCurOpp, m_fScale, m_fScale*m_fScaleY);
    }

    if(s_NeutralStyle.m_bIsVisible)
    {
        QPen neutralpen(xfl::fromfl5Clr(s_NeutralStyle.m_Color));
        neutralpen.setCosmetic(true);
        neutralpen.setStyle(xfl::getStyle(s_NeutralStyle.m_Stipple));
        neutralpen.setWidth(s_NeutralStyle.m_Width);
        painter.setPen(neutralpen);
        painter.drawLine(rect().left(),  int(m_ptOffset.y()),
                         rect().right(), int(m_ptOffset.y()));
    }
    painter.restore();
}


void OpPointWt::paintPressure(QPainter &painter, double scalex, double scaley)
{
    Foil const *pFoil = XDirect::curFoil();
    if(!pFoil) return;

    OpPoint const *pOpp = XDirect::curOpp();
    if(!pOpp || !pOpp->bViscResults()) return;

    float alpha = -pOpp->m_Alpha*PI/180.0;
    float cosa = cos(alpha);
    float sina = sin(alpha);
    float x(0), y(0), xs(0), ys(0), xe(0), ye(0), dx(0), dy(0), x1(0), x2(0), y1(0), y2(0), r2(0);
    float cp(0);
    QPointF offset = m_ptOffset;

    painter.save();

    QPen cpvpen(xfl::fromfl5Clr(s_PressureStyle.m_Color));
    cpvpen.setCosmetic(true);
    cpvpen.setStyle(xfl::getStyle(s_PressureStyle.m_Stipple));
    cpvpen.setWidth(s_PressureStyle.m_Width);
    painter.setPen(cpvpen);


    for(int i=0; i<pFoil->nNodes(); i++)
    {
        if(pOpp->bViscResults()) cp = pOpp->m_Cpv.at(i);
        else                     cp = pOpp->m_Cpi.at(i);
        x = pFoil->node(i).xf();
        y = pFoil->node(i).yf();

        xs = (x-0.5)*cosa - y*sina + 0.5;
        ys = (x-0.5)*sina + y*cosa;

        if(cp>0)
        {           
            x += pFoil->node(i).normal().xf() * cp * 0.05;
            y += pFoil->node(i).normal().yf() * cp * 0.05;

            xe = (x-0.5)*cosa - y*sina + 0.5;
            ye = (x-0.5)*sina + y*cosa;
            painter.drawLine(int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                             int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()));

            dx = xe - xs;
            dy = ye - ys;
            r2 = sqrtf(dx*dx + dy*dy);
            if(r2!=0.0) //you can never be sure
            {
                dx = dx/r2;
                dy = dy/r2;
            }

            x1 = xs + 0.0085*dx + 0.005*dy;
            y1 = ys + 0.0085*dy - 0.005*dx;
            x2 = xs + 0.0085*dx - 0.005*dy;
            y2 = ys + 0.0085*dy + 0.005*dx;

            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(x1*scalex + offset.x()), int(-y1*scaley + offset.y()));

            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(x2*scalex + offset.x()), int(-y2*scaley + offset.y()));
        }
        else
        {
            x += -pFoil->node(i).normal().xf() * cp *0.05;
            y += -pFoil->node(i).normal().yf() * cp *0.05;

            xe = (x-0.5)*cosa - y*sina+ 0.5;
            ye = (x-0.5)*sina + y*cosa;
            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()));

            dx = xe - xs;
            dy = ye - ys;
            r2 = sqrtf(dx*dx + dy*dy);
            if(r2!=0.0) //you can never be sure...
            {
                dx = -dx/r2;
                dy = -dy/r2;
            }

            x1 = xe + 0.0085*dx + 0.005*dy;
            y1 = ye + 0.0085*dy - 0.005*dx;
            x2 = xe + 0.0085*dx - 0.005*dy;
            y2 = ye + 0.0085*dy + 0.005*dx;

            painter.drawLine( int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()),
                              int(x1*scalex + offset.x()), int(-y1*scaley + offset.y()));

            painter.drawLine( int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()),
                              int(x2*scalex + offset.x()), int(-y2*scaley + offset.y()));
        }
    }

    //lastly draw lift at XCP position
    QPen liftpen(xfl::fromfl5Clr(s_PressureStyle.m_Color));
    liftpen.setCosmetic(true);
    liftpen.setStyle(xfl::getStyle(s_PressureStyle.m_Stipple));
    liftpen.setWidth(s_PressureStyle.m_Width+1);
    painter.setPen(liftpen);

    xs =  (pOpp->m_XCP-0.5)*cosa  + 0.5;
    ys = -(pOpp->m_XCP-0.5)*sina ;

    xe = xs;
    ye = ys - pOpp->m_Cl/10.0;

    painter.drawLine( int(xs*scalex + offset.x()), int(ys*scaley + offset.y()),
                      int(xs*scalex + offset.x()), int(ye*scaley + offset.y()));

    dx = xe - xs;
    dy = ye - ys;
    r2 = sqrt(dx*dx + dy*dy);
    dx = -dx/r2;
    dy = -dy/r2;

    x1 = xe + 0.0085*dx + 0.005*dy;
    y1 = ye + 0.0085*dy - 0.005*dx;
    x2 = xe + 0.0085*dx - 0.005*dy;
    y2 = ye + 0.0085*dy + 0.005*dx;

    painter.drawLine( int(xe*scalex + offset.x()), int(ye*scaley + offset.y()),
                      int(x1*scalex + offset.x()), int(y1*scaley + offset.y()));

    painter.drawLine( int(xe*scalex + offset.x()), int(ye*scaley + offset.y()),
                      int(x2*scalex + offset.x()), int(y2*scaley + offset.y()));

    painter.restore();
}


void OpPointWt::paintBLXFoil(QPainter &painter, OpPoint const* pOpPoint, double scalex, double scaley)
{
    if(!pOpPoint) return;

    QPointF offset, From, To;

    double alpha = -pOpPoint->aoa()*PI/180.0;
    double cosa = cos(alpha);
    double sina = sin(alpha);

    if(!pOpPoint->bViscResults() || !pOpPoint->m_bBL) return;

    painter.save();

    offset = m_ptOffset;

    QPen wakepen(xfl::fromfl5Clr(s_BLStyle.m_Color));
    wakepen.setCosmetic(true);
    wakepen.setStyle(xfl::getStyle(s_BLStyle.m_Stipple));
    wakepen.setWidth(s_BLStyle.m_Width);

    painter.setPen(wakepen);

    double x = (pOpPoint->m_BLXFoil.xd1[1]-0.5)*cosa - pOpPoint->m_BLXFoil.yd1[1]*sina + 0.5;
    double y = (pOpPoint->m_BLXFoil.xd1[1]-0.5)*sina + pOpPoint->m_BLXFoil.yd1[1]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=2; i<=pOpPoint->m_BLXFoil.nd1; i++)
    {
        x = (pOpPoint->m_BLXFoil.xd1[i]-0.5)*cosa - pOpPoint->m_BLXFoil.yd1[i]*sina + 0.5;
        y = (pOpPoint->m_BLXFoil.xd1[i]-0.5)*sina + pOpPoint->m_BLXFoil.yd1[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    x = (pOpPoint->m_BLXFoil.xd2[0]-0.5)*cosa - pOpPoint->m_BLXFoil.yd2[0]*sina + 0.5;
    y = (pOpPoint->m_BLXFoil.xd2[0]-0.5)*sina + pOpPoint->m_BLXFoil.yd2[0]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=1; i<pOpPoint->m_BLXFoil.nd2; i++)
    {
        x = (pOpPoint->m_BLXFoil.xd2[i]-0.5)*cosa - pOpPoint->m_BLXFoil.yd2[i]*sina + 0.5;
        y = (pOpPoint->m_BLXFoil.xd2[i]-0.5)*sina + pOpPoint->m_BLXFoil.yd2[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    x = (pOpPoint->m_BLXFoil.xd3[0]-0.5)*cosa - pOpPoint->m_BLXFoil.yd3[0]*sina + 0.5;
    y = (pOpPoint->m_BLXFoil.xd3[0]-0.5)*sina + pOpPoint->m_BLXFoil.yd3[0]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=1; i<pOpPoint->m_BLXFoil.nd3; i++)
    {
        x = (pOpPoint->m_BLXFoil.xd3[i]-0.5)*cosa - pOpPoint->m_BLXFoil.yd3[i]*sina + 0.5;
        y = (pOpPoint->m_BLXFoil.xd3[i]-0.5)*sina + pOpPoint->m_BLXFoil.yd3[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    painter.restore();
}


void OpPointWt::paintNode2d(QPainter &painter, QVector<Node2d> const &node, double alpha)
{
    // paint the wake
    double x=0, y=0;
    double cosa = cos(PI*alpha/180.0);
    double sina = sin(PI*alpha/180.0);
    QPointF from, to;
    QPolygonF polyline;
    QPen wakepen;
    wakepen.setCosmetic(true);
    wakepen.setStyle(Qt::DashLine);
    wakepen.setWidth(1);
    painter.setPen(wakepen);
    for (int i=1; i<node.size(); i++)
    {
        //        if(node[i].isWakeNode())
        {
            Node2d const &pt = node[i];
            x = (pt.x-0.5)*cosa - pt.y*sina + 0.5;
            y = (pt.x-0.5)*sina + pt.y*cosa;
            from.rx() =  x*m_fScale + m_ptOffset.x();
            from.ry() = -y*m_fScale*m_fScaleY + m_ptOffset.y();

            Vector2d ptN =  pt + node[i].normal()*0.05;
            x = (ptN.x-0.5)*cosa - ptN.y*sina + 0.5;
            y = (ptN.x-0.5)*sina + ptN.y*cosa;
            to.rx() =  x*m_fScale + m_ptOffset.x();
            to.ry() = -y*m_fScale*m_fScaleY + m_ptOffset.y();
//            painter.drawLine(from, to);

            polyline.append(from);
        }
    }
    painter.drawPolyline(polyline);

    painter.restore();
}


Vector2d OpPointWt::mousetoReal(QPointF const& point) const
{
    Vector2d Real;
    Real.x =  (point.x() - m_ptOffset.x())/m_fScale;
    Real.y = -(point.y() - m_ptOffset.y())/m_fScale;
    return Real;
}


void OpPointWt::onResetFoilScale()
{
    setFoilScale(true);
    update();
}


void OpPointWt::onResetIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_iTimerInc = 0;
        m_ResetTimer.stop();
        update();
        return;
    }

    m_ptOffset += m_Trans;
    m_fScale += m_ScaleInc;
    m_fScaleY += m_ScaleYInc;

    update();
    m_iTimerInc++;
}


QPointF OpPointWt::defaultOffset() const
{
    QPointF offset;
    if(m_pCpGraph)
    {
        int h =  m_rGraphRect.height();
        offset.setX(m_pCpGraph->xToClient(0));
        offset.setY((rect().height()+h)/2);
    }
    return offset;
}


double OpPointWt::defaultScale() const
{
    if(m_pCpGraph)
    {
        double p0  = m_pCpGraph->xToClient(0.0);
        double p1  = m_pCpGraph->xToClient(1.0);
        return (p1-p0);
    }
    return 1.0;
}


void OpPointWt::setFoilScale(bool )
{
    stopDynamicTimer();

    if(!Section2dWt::bAnimateTransitions())
    {
        m_fScale = defaultScale();
        m_fScaleY = 1.0;

        m_ptOffset = defaultOffset();
        update();
    }
    else
    {
        double inc = double(ANIMATIONFRAMES);
        QPointF trans(defaultOffset()-m_ptOffset);
        m_Trans = trans/inc;

        m_ScaleInc = (defaultScale()-m_fScale)/inc;
        m_ScaleYInc = (1.0-m_fScaleY)/inc;

        m_iTimerInc = 0;
        m_ResetTimer.stop();
        m_ResetTimer.start(5);

        update();
    }

}


void OpPointWt::onShowNeutralLine()
{
    s_NeutralStyle.m_bIsVisible = !s_NeutralStyle.m_bIsVisible;
    update();
}


void OpPointWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("OpPointWt");
    {
        xfl::loadLineSettings(settings, s_PressureStyle, "PressureStyle");
        xfl::loadLineSettings(settings, s_BLStyle,       "BLStyle");
        xfl::loadLineSettings(settings, s_NeutralStyle,  "NeutralStyle");

        s_bFillFoil = settings.value("FillFoil", s_bFillFoil).toBool();
    }
    settings.endGroup();
}


void OpPointWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("OpPointWt");
    {
        xfl::saveLineSettings(settings, s_PressureStyle, "PressureStyle");
        xfl::saveLineSettings(settings, s_BLStyle,       "BLStyle");
        xfl::saveLineSettings(settings, s_NeutralStyle,  "NeutralStyle");
        settings.setValue("FillFoil", s_bFillFoil);
    }
    settings.endGroup();
}



