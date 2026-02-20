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

#include <cmath>


#include <QApplication>
#include <QPen>
#include <QPainter>
#include <QPaintEvent>
#include <QMenu>
#include <QTimer>
#include <QToolTip>
#include <QMouseEvent>

#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphdlg.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/graphs/globals/graph_globals.h>
#include <interfaces/graphs/controls/graphoptions.h>

#define ANIMATIONFRAMES 30



GraphWt::GraphWt(QWidget *pParent) : QWidget(pParent)
{
    setCursor(Qt::CrossCursor);
    setMouseTracking(GraphOptions::bMouseTrack());
    setFocusPolicy(Qt::WheelFocus);

    m_DefaultSize = QSize(500,500);

    m_bOverlayRectangle = false;

    m_bTransGraph = false;
    m_bXPressed = m_bYPressed = false;

    m_bDynTranslation = m_bDynScaling = m_bDynResetting = false;
    m_iResetDyn = 0;
    m_ZoomFactor = 1.0;

    m_pGraph = nullptr;
    m_bEnableContextMenu = false;
    m_bCurveStylePage = false;

    m_HoverTimer.setSingleShot(true);

    m_pResetScales     = new QAction(tr("Reset scales\tR"),     this);
    m_pShowGraphLegend = new QAction(tr("Show legend"),         this);
    m_pShowGraphLegend->setCheckable(true);
    m_pGraphSettings   = new QAction(tr("Settings\tG"),         this);
    m_pToClipboard     = new QAction(tr("to clipboard"),        this);
    m_pToFile          = new QAction(tr("to text file"),        this);
    m_pToSVG           = new QAction(tr("to SVG"),              this);
    m_pCloseGraph      = new QAction(tr("Close window"),        this);

    setLegendPosition(Qt::AlignTop | Qt::AlignHCenter);

    m_plabInfoOutput = new QLabel(this);
    m_plabInfoOutput->setAttribute(Qt::WA_NoSystemBackground);

    connectSignals();
}


void GraphWt::connectSignals()
{
    connect(&m_HoverTimer,          SIGNAL(timeout()),       SLOT(onHovered()));
    connect(m_pShowGraphLegend,     SIGNAL(triggered(bool)), SLOT(onShowGraphLegend()));
    connect(m_pResetScales,         SIGNAL(triggered(bool)), SLOT(onResetGraphScales()));
    connect(m_pGraphSettings,       SIGNAL(triggered(bool)), SLOT(onGraphSettings()));
    connect(m_pToClipboard,         SIGNAL(triggered(bool)), SLOT(onCopyData()));
    connect(m_pToFile,              SIGNAL(triggered(bool)), SLOT(onExportGraphDataToFile()));
    connect(m_pToSVG,               SIGNAL(triggered(bool)), SLOT(onExportGraphDataToSvg()));
    connect(m_pCloseGraph,          SIGNAL(triggered(bool)), SLOT(onCloseWindow()));

    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));
}


GraphWt::~GraphWt()
{
    m_pGraph = nullptr;
}


void GraphWt::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    if(pGraph)
    {
        setWindowTitle(pGraph->name());
    }
}


void GraphWt::paintEvent(QPaintEvent* )
{
    if(!m_pGraph)return;
    QString strong;
    QPainter painter(this);
    painter.save();

    QBrush BackBrush(m_pGraph->backgroundColor());
    painter.setBrush(BackBrush);
    //    painter.fillRect(event->rect(), BackBrush);

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBackground(BackBrush);

    m_pGraph->drawGraph(painter, rect());

    if(m_bOverlayRectangle)
    {
        painter.save();
        QBrush overlaybrush(QColor(0,175,225,105));
        painter.setBrush(overlaybrush);
        QPointF topleft(m_pGraph->toClient(m_TopLeft.x(), m_TopLeft.y()));
        QPointF botright(m_pGraph->toClient(m_BotRight.x(), m_BotRight.y()));
        QRectF rect(topleft, botright);
        painter.drawRect(rect);
        painter.restore();
    }

    //two options for the legend: in graph, or in a separate widget - or both
    //
    if(m_pGraph->isLegendVisible())
    {
        repositionLegend();
        m_pGraph->drawLegend(painter, m_LegendOrigin);
    }

    if(hasFocus() && Graph::bMousePos())
    {
        QPen textPen(DisplayOptions::textColor());
        QFontMetrics fm(DisplayOptions::textFontStruct().font());

        int fmheight  = fm.height();

        painter.setFont(DisplayOptions::textFontStruct().font());
        painter.setPen(textPen);

        double xm = m_pGraph->clientTox(m_LastPoint.x());
        if(m_pGraph->xAxis().bLogScale()) xm = pow(10.0, xm);
//        if(fabs(xm)<1.e-3) strong = QString::asprintf("x = %9.5g", xm);
//        else               strong = QString::asprintf("x = %9.5f", xm);
        if(xfl::isLocalized()) strong = QString("x = %L1").arg(xm, 9, 'g', 5);
        else                   strong = QString("x = %1" ).arg(xm, 9, 'g', 5);

        painter.drawText(width()-14*fm.averageCharWidth(),fmheight, strong);

        double ym = m_pGraph->clientToy(0, m_LastPoint.y());
        if(m_pGraph->yAxis(0).bLogScale()) ym = pow(10.0, ym);
//        if(fabs(ym)<1.e-3) strong = QString::asprintf("y = %9.5g", ym);
//        else               strong = QString::asprintf("y = %9.5f", ym);
        if(xfl::isLocalized()) strong = QString("y = %L1").arg(ym, 9, 'g', 5);
        else                   strong = QString("y = %1" ).arg(ym, 9, 'g', 5);
        painter.drawText(width()-14*fm.averageCharWidth(),2*fmheight, strong);
    }
    painter.restore();
}


void GraphWt::resizeEvent (QResizeEvent * pEvent)
{
    repositionLegend();

    m_plabInfoOutput->adjustSize();
    QPoint pos3(width()-m_plabInfoOutput->width()-10, height()-m_plabInfoOutput->height()-10);
    m_plabInfoOutput->move(pos3);

    if(m_pGraph)
    {
        m_pGraph->invalidate();
        emit graphResized(m_pGraph);
    }
    pEvent->accept();
}


void GraphWt::showEvent(QShowEvent *pEvent)
{
    setMouseTracking(GraphOptions::bMouseTrack());

    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                           DisplayOptions::textFontStruct().pointSize());
    m_plabInfoOutput->setStyleSheet(stylesheet);

    QWidget::showEvent(pEvent);
}


void GraphWt::hideEvent(QHideEvent *pEvent)
{
    stopDynamicTimer();
    QWidget::hideEvent(pEvent);
}


void GraphWt::closeEvent(QCloseEvent *)
{
    emit widgetClosed(this);
}


void GraphWt::setLegendPosition(Qt::Alignment pos)
{
    if(m_pGraph)
    {
        m_pGraph->setLegendPosition(pos);
    }

    repositionLegend();
}


void GraphWt::repositionLegend()
{
    if(!m_pGraph) return;
    QFontMetrics fm(m_pGraph->legendFont());
    int fmheight  = fm.height();
    int fmWidth   = fm.averageCharWidth();

    if(m_pGraph->legendPosition() & Qt::AlignHCenter)
    {
        m_LegendOrigin.rx() = rect().center().x() - 5*fmWidth;
    }
    else if(m_pGraph->legendPosition() & Qt::AlignRight)
    {
        int w =0;
        for(int ic=0; ic<m_pGraph->curveCount(); ic++)
        {
            w = qMax(w, fm.horizontalAdvance(m_pGraph->curve(ic)->name()));
        }
        w += fm.averageCharWidth()*8; // to account for the line length
        m_LegendOrigin.rx() = rect().right() - w;
    }
    else if(m_pGraph->legendPosition() & Qt::AlignLeft)
    {
        m_LegendOrigin.rx() = rect().left()+fmWidth;
    }

    if(m_pGraph->legendPosition() & Qt::AlignVCenter)
    {
        m_LegendOrigin.ry() = rect().center().y();
    }
    else if(m_pGraph->legendPosition() & Qt::AlignTop)
    {
        m_LegendOrigin.ry() = rect().top() + fmheight;
    }
    else if(m_pGraph->legendPosition() & Qt::AlignBottom)
    {
        m_LegendOrigin.ry() = rect().bottom() - (m_pGraph->curveCount()+1)*fmheight;
    }
}


void GraphWt::contextMenuEvent(QContextMenuEvent *pEvent)
{
    setFocus();
    if(!m_bEnableContextMenu)
    {
        pEvent->ignore();
        return;
    }
    else if(m_pGraph)
    {
        QMenu *pContextMenu = new QMenu("GraphMenu");
        pContextMenu->addAction(m_pResetScales);
        pContextMenu->addAction(m_pShowGraphLegend);
        m_pShowGraphLegend->setChecked(m_pGraph->isLegendVisible());
        pContextMenu->addAction(m_pGraphSettings);
        pContextMenu->addSeparator();
        QMenu *pExportMenu =  pContextMenu->addMenu(tr("Export"));
        {
            pExportMenu->addAction(m_pToFile);
            pExportMenu->addAction(m_pToClipboard);
            pExportMenu->addAction(m_pToSVG);
        }
        pContextMenu->addSeparator();
        pContextMenu->addAction(m_pCloseGraph);

        pContextMenu->exec(QCursor::pos());
        update();
    }
}


void GraphWt::keyPressEvent(QKeyEvent *pEvent)
{
    stopDynamicTimer(); // stop whatever the user was doing

    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_R:
        {
            if(!bCtrl && !bShift)
            {
                onResetGraphScales();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_V:
        {
            if(!bCtrl && !bShift)
            {
                GraphDlg::setActivePage(0);
                onGraphSettings();
                pEvent->accept();
            }
            break;
        }
        case Qt::Key_G:
        {
            if(!bCtrl && !bShift)
            {
                onGraphSettings();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_W:
        {
            if(bShift && bCtrl)
            {
                emit graphWindow(m_pGraph);
                pEvent->ignore();
            }
            else if(!parent() && bCtrl)
            {
                close();
            }

            break;
        }
        case Qt::Key_X:
        {
            m_bXPressed = true;
            pEvent->accept();
            return;
        }
        case Qt::Key_Y:
        {
            m_bYPressed = true;
            pEvent->accept();
            return;
        }

        default:break;;
    }

    QWidget::keyPressEvent(pEvent);
}


void GraphWt::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_X:
        {
            m_bXPressed = false;
            break;
        }
        case Qt::Key_Y:
        {
            m_bYPressed = false;
            break;
        }

        default:pEvent->ignore();
    }
}


void GraphWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!m_pGraph) return;

    if(GraphOptions::bMouseTrack()) setFocus();

    QPointF point = pEvent->pos();

    bool bCtrl= false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    if(!rect().contains(pEvent->pos()))
    {
        m_bTransGraph = false;
        return;
    }

    if ((pEvent->buttons() & Qt::LeftButton) && m_bTransGraph)
    {
        // curve translation inside the graph
        if (m_pGraph->hasRightAxis() && double(m_LastPoint.x())/double(width())<0.15)
        {
            int iy = 0;
            double y1 = m_pGraph->clientToy(iy, m_LastPoint.y());
            double yu = m_pGraph->clientToy(iy, point.y());
            double ymin = m_pGraph->yMin(iy) - yu+y1;
            double ymax = m_pGraph->yMax(iy) - yu+y1;
            m_pGraph->setYWindow(iy, ymin, ymax);
        }
        else if(m_pGraph->hasRightAxis() && double(m_LastPoint.x())/double(width())>0.85)
        {
            int iy = 1;
            double y1 = m_pGraph->clientToy(iy, m_LastPoint.y());
            double yu = m_pGraph->clientToy(iy, point.y());
            double ymin = m_pGraph->yMin(iy) - yu+y1;
            double ymax = m_pGraph->yMax(iy) - yu+y1;
            m_pGraph->setYWindow(iy, ymin, ymax);
        }
        else
        {
            m_pGraph->setAuto(false);
            double x1 = m_pGraph->clientTox(m_LastPoint.x());
            double xu = m_pGraph->clientTox(point.x());
            double xmin = m_pGraph->xMin() - xu+x1;
            double xmax = m_pGraph->xMax() - xu+x1;
            m_pGraph->setXWindow(xmin, xmax);

            for(int iy=0; iy<2; iy++)
            {
                double y1 = m_pGraph->clientToy(iy, m_LastPoint.y());
                double yu = m_pGraph->clientToy(iy, point.y());
                double ymin = m_pGraph->yMin(iy) - yu+y1;
                double ymax = m_pGraph->yMax(iy) - yu+y1;
                m_pGraph->setYWindow(iy, ymin, ymax);
            }
        }

        m_pGraph->invalidate();
    }
    else if ((pEvent->buttons() & Qt::MiddleButton) && !bCtrl)
    {
        //scaling
        double val[]{0,0,0};
        m_pGraph->clientToGraph(m_LastPoint, val);
        double y[] = {val[1], val[2]};

        m_pGraph->setAuto(false);
        double dx = double(point.x()-m_LastPoint.x());
        m_pGraph->scaleXAxis(val[0], 1.+dx/double(width()));
        double dy = double(point.y()-m_LastPoint.y());
        m_pGraph->scaleYAxis(0, y[0], 1.+dy/double(width()));
        m_pGraph->invalidate();
    }
    // we zoom the graph or the foil
    else if ((pEvent->buttons() & Qt::MiddleButton) || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        //zoom graph
        double zoomfactor = 1.02;
        if(point.y()-m_LastPoint.y()<0) zoomfactor = 1.0/1.02;
        scaleAxes(-1, zoomfactor);
    }
    else
    {
        //        update();
    }
    update();

    m_LastPoint = point;
    m_HoverTimer.start(150);
    QToolTip::hideText();
}


void GraphWt::onHovered()
{
    int ipt = -1;
    Curve *pCurve = m_pGraph->getCurve(m_LastPoint, ipt);
    if(pCurve)
    {
        if(pCurve->isLeftAxis())
            QToolTip::showText(QCursor::pos(), pCurve->name(), this, QRect(), 15000);
        else
            QToolTip::showText(QCursor::pos(), pCurve->name()+ "_Right", this, QRect(), 15000);
    }
}


void GraphWt::mousePressEvent(QMouseEvent *pEvent)
{
    setFocus();
    stopDynamicTimer();

    if (pEvent->buttons() & Qt::LeftButton)
    {
        QPoint point = pEvent->pos();
        int ipt = -1;
        Curve *pCurve = m_pGraph->getCurve(point, ipt);
        if(pCurve)
            emit curveClicked(pCurve,ipt);
        else
        {
            m_bTransGraph = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        }

        m_LastPoint = point;
        m_LastPressedPt = point;
    }
    else pEvent->ignore();

    m_MoveTime.restart();
}


void GraphWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QApplication::restoreOverrideCursor();
    m_bTransGraph = false;
    if(m_LastPressedPt==pEvent->pos())
    {
        //the mouse has not moved, the user probably wants to un-highlight the curves
        int ipt = -1;
        Curve *pCurve = m_pGraph->getCurve(pEvent->pos(), ipt);
        if(!pCurve)
        {
            emit curveClicked(nullptr,ipt);
            update();
        }
    }

    if(GraphOptions::bSpinAnimation() && pEvent->button()==Qt::LeftButton)
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<DisplayOptions::moveTimeThreshold() && !m_LastPressedPt.isNull())
        {
            m_Trans = pEvent->pos() - m_LastPressedPt;
            startDynamicTimer();
            m_bDynTranslation = true;
        }
    }
    pEvent->accept();
}


void GraphWt::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    if (pEvent->buttons() & Qt::LeftButton)
    {
        QPoint point = pEvent->pos();
        int ipt = -1;
        Curve *pCurve = m_pGraph->getCurve(point, ipt);
        if(pCurve)    emit curveDoubleClicked(pCurve);
        else onGraphSettings();
    }
}


void GraphWt::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y();

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

            if(m_pGraph && rect().contains(pt))
            {
                int iy = -1; // both axes
                if (m_pGraph->hasRightAxis() && double(pt.x())/double(width())<0.15)
                {
                    iy = 0; // left axis
                }
                else if(m_pGraph->hasRightAxis() && double(pt.x())/double(width())>0.85)
                {
                    iy = 1; // right axis
                }
                scaleAxes(iy, zoomfactor);
            }
        }
    }
}


void GraphWt::onResetGraphScales()
{
    if(!m_pGraph) return;
    stopDynamicTimer();

    if(!m_pGraph) return;
    if(GraphOptions::bSpinAnimation())
    {
        QRect r(rect());
        Graph graph(*m_pGraph);
        graph.setCurveModel(m_pGraph->curveModel());
        graph.setAuto(true);
        graph.resetLimits();
        graph.setGraphScales(r);
        m_xAxisStart    = m_pGraph->xAxis();
        m_yAxisStart[0] = m_pGraph->yAxis(0);
        m_yAxisStart[1] = m_pGraph->yAxis(1);

        m_xAxisEnd    = graph.xAxis();
        m_yAxisEnd[0] = graph.yAxis(0);
        m_yAxisEnd[1] = graph.yAxis(1);

        m_iResetDyn = 0;
        m_pGraph->setAuto(false);

        m_bDynResetting = true;
        startDynamicTimer();
    }
    else
    {
        m_pGraph->setAuto(true);
        m_pGraph->resetLimits();
        m_pGraph->invalidate();
    }

    update();
}


#define INCFRACTION 10

void GraphWt::onDynamicIncrement()
{
    if(!m_pGraph) return;

    if(m_bDynResetting)
    {
        if(m_iResetDyn>=ANIMATIONFRAMES)
        {
            stopDynamicTimer();
            m_pGraph->setAuto(true);
            m_pGraph->resetLimits();
            m_pGraph->invalidate();
        }
        else
        {
            m_iResetDyn++;
//            double tau = double(m_iResetDyn)/double(ANIMATIONFRAMES);
            double frac = double(m_iResetDyn)/double(ANIMATIONFRAMES) * 3.14592654/2.0;
            double tau = sin(frac);
            m_pGraph->setXMin(m_xAxisStart.axmin()*(1.0-tau) + m_xAxisEnd.axmin()*tau);
            m_pGraph->setXMax(m_xAxisStart.axmax()*(1.0-tau) + m_xAxisEnd.axmax()*tau);

            for(int iy=0; iy<2; iy++)
            {
                m_pGraph->setYMin(iy, m_yAxisStart[iy].axmin()*(1.0-tau) + m_yAxisEnd[iy].axmin()*tau);
                m_pGraph->setYMax(iy, m_yAxisStart[iy].axmax()*(1.0-tau) + m_yAxisEnd[iy].axmax()*tau);
            }
            m_pGraph->invalidate();
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

            m_pGraph->setAuto(false);
            double dx = m_Trans.x()/m_pGraph->xScale()/INCFRACTION;
            double xmin = m_pGraph->xMin() - dx;
            double xmax = m_pGraph->xMax() - dx;
            m_pGraph->setXWindow(xmin, xmax);

            for(int iy=0; iy<2; iy++)
            {
                double dy = m_Trans.y()/m_pGraph->yScale(iy)/INCFRACTION;
                double ymin = m_pGraph->yMin(iy) - dy;
                double ymax = m_pGraph->yMax(iy) - dy;
                m_pGraph->setYWindow(iy, ymin, ymax);
            }
            m_pGraph->invalidate();
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

    update();
}


void GraphWt::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void GraphWt::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
    }
    m_bDynTranslation = m_bDynScaling = m_bDynResetting = false;
    setMouseTracking(GraphOptions::bMouseTrack());
}


void GraphWt::scaleAxes(int iy, double zoomfactor)
{
    if(!m_pGraph) return;

    double val[]{0,0,0};
    m_pGraph->clientToGraph(m_LastPoint, val);
    double y[]{val[1], val[2]};

    if(m_bXPressed && m_bYPressed)
    {
        //zoom both
        m_pGraph->setAuto(false);
        m_pGraph->scaleAxes(val[0], y, 1./zoomfactor);
    }
    else if(m_bXPressed)
    {
        //zoom x scale
        m_pGraph->setAutoX(false);
        m_pGraph->scaleXAxis(val[0], 1./zoomfactor);
    }
    else if(m_bYPressed)
    {
        //zoom y scale
        m_pGraph->setAutoY(0, false);
        m_pGraph->setAutoY(1, false);
        m_pGraph->scaleYAxis(0, y[0], 1./zoomfactor);
        m_pGraph->scaleYAxis(1, y[1], 1./zoomfactor);
    }
    else
    {      
        if(iy>=0 && iy<2)
        {
            m_pGraph->setAutoY(iy, false);
            m_pGraph->scaleYAxis(iy, y[iy], 1./zoomfactor);
        }
        else
        {
            m_pGraph->scaleAxes(val[0], y, 1./zoomfactor);
        }
    }

    m_pGraph->setAutoXUnit();
    m_pGraph->setAutoYUnit(0);
    m_pGraph->setAutoYUnit(1);
    m_pGraph->invalidate();
    update();
}


void GraphWt::onGraphSettings()
{
    GraphDlg grDlg(this);
    grDlg.showCurvePage(m_bCurveStylePage);
    grDlg.setGraph(m_pGraph);

    if(grDlg.exec() == QDialog::Accepted)
    {
        if(grDlg.bVariableChanged())
        {
            m_pGraph->setAuto(true);
            m_pGraph->setAutoYMinUnit(true);
            m_pGraph->resetLimits();
        }
    }

    emit graphChanged(m_pGraph);
    m_pGraph->invalidate();
    update();
}


void GraphWt::onShowGraphLegend()
{
    showLegend(m_pShowGraphLegend->isChecked());
}


void GraphWt::showLegend(bool bShow)
{
    if(m_pGraph)
        m_pGraph->setLegendVisible(bShow);
}


/** slot used only when graphwt is detached */
void GraphWt::onCopyData()
{
    if(!m_pGraph) return;
    m_pGraph->toClipboard();
}


/** slot used only when graphwt is detached */
void GraphWt::onExportGraphDataToFile()
{
    if(!m_pGraph) return;
    exportGraphDataToFile(m_pGraph);
}


/** slot used only when graphwt is detached */
void GraphWt::onExportGraphDataToSvg()
{
    if(!m_pGraph) return;
    QString tempfilepath;
    exportGraphToSvg(this, m_pGraph, tempfilepath);
}


void GraphWt::onCloseWindow()
{
    close();
}


void GraphWt::setOverlayedRect(bool bShow, double tlx, double tly, double brx, double bry)
{
    m_bOverlayRectangle = bShow;
    m_TopLeft  = QPointF(tlx, tly);
    m_BotRight = QPointF(brx, bry);
}


void GraphWt::setOutputInfo(QString const &info)
{
    QString stylesheet = QString::asprintf("color: %s; font-family: %s; font-size: %dpt",
                                           DisplayOptions::textColor().name(QColor::HexRgb).toStdString().c_str(),
                                           DisplayOptions::textFontStruct().family().toStdString().c_str(),
                                           DisplayOptions::textFontStruct().pointSize());
    m_plabInfoOutput->setStyleSheet(stylesheet);
//    m_plabInfoOutput->setTextFormat(Qt::MarkdownText);

    m_plabInfoOutput->setText(info);
    m_plabInfoOutput->adjustSize();

    QPoint pos3(width()-m_plabInfoOutput->width()-10, height()-m_plabInfoOutput->height()-10);
    m_plabInfoOutput->move(pos3);
}


