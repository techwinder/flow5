/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QGridLayout>


#include "sectiongraphwt.h"
#include <xflcore/xflcore.h>
#include <xflcore/displayoptions.h>
#include <xflgraph/graph/graph.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflwidgets/view/section2dwt.h>
#include <xflwidgets/line/linebtn.h>

SectionGraphWt::SectionGraphWt() : GraphWt()
{
    m_pFrame = nullptr;
    m_bOnCurve = true;
    m_bEnableSpline = true;

    m_iActiveSpline = -1;
}


void SectionGraphWt::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    if(pGraph) setWindowTitle(pGraph->name());
}


void SectionGraphWt::initialize()
{
    resetGraphScales();
}


void SectionGraphWt::resetGraphScales()
{
    if(!m_pGraph) return;
    m_pGraph->invalidate();
}


void SectionGraphWt::paintEvent(QPaintEvent* )
{
    if(!m_pGraph)return;
    QPainter painter(this);
    painter.save();

    QBrush BackBrush(m_pGraph->backgroundColor());
    painter.setBrush(BackBrush);

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBackground(BackBrush);

/*    if(!isEnabled())
    {
        painter.fillRect(pEvent->rect(), BackBrush);
        painter.restore();
        return;
    }*/

    m_pGraph->drawGraph(painter, rect());

    if(m_Spline.size() && m_iActiveSpline>=0 && m_iActiveSpline<m_Spline.count())
    {
        BSpline const *pSpline = &m_Spline.at(m_iActiveSpline);
        QPen splinepen(pSpline->color());
        splinepen.setWidth(pSpline->width());
        splinepen.setStyle(xfl::getStyle(pSpline->stipple()));
        painter.setPen(splinepen);

        if(pSpline->isVisible())
        {
            drawSpline(    pSpline, painter, m_pGraph->xScale(), -m_pGraph->yScale(0), m_pGraph->offset(0));
            drawCtrlPoints(pSpline, painter, m_pGraph->xScale(), -m_pGraph->yScale(0), m_pGraph->offset(0), DisplayOptions::backgroundColor());
        }
    }
    //two options for the legend: in graph, or in a separate widget - or both

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
        painter.drawText(width()-14*fm.averageCharWidth(),fmheight,
                         QString("x = %1").arg(m_pGraph->clientTox(m_LastPoint.x()),9,'f',3));
        painter.drawText(width()-14*fm.averageCharWidth(),2*fmheight,
                         QString("y = %1").arg(m_pGraph->clientToy(0, m_LastPoint.y()),9,'f',3));
    }
    painter.restore();
}


void SectionGraphWt::mousePressEvent(QMouseEvent *pEvent)
{
    stopDynamicTimer();

    if(!m_bEnableSpline || m_iActiveSpline<0 || m_iActiveSpline>=m_Spline.count())
    {
        GraphWt::mousePressEvent(pEvent);
        return;
    }

    bool bCtrl=false, bShift=false;

    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl  = true;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift = true;

    int CtrlPt=-1;
    QPoint pttmp;
    QPoint point = pEvent->pos();

    BSpline &spline = m_Spline[m_iActiveSpline];

    if((pEvent->buttons() & Qt::LeftButton))
    {
        m_PointDown.rx() = point.x();
        m_PointDown.ry() = point.y();
        pttmp = QPoint(point.x(), point.y());

        m_bTransGraph = true;

        double xd = m_pGraph->clientTox(point.x());
        double yd = m_pGraph->clientToy(0, point.y());
        double dtolx = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->xScale());
        double dtoly = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->yScale(0));
        CtrlPt = spline.isCtrlPoint(xd, yd, dtolx, dtoly);
        if(!isEnabled() || CtrlPt<0)
        {
            spline.setSelectedPoint(-1);
            GraphWt::mousePressEvent(pEvent);
            return;
        }

        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        spline.setSelectedPoint(CtrlPt);

        if (bCtrl)
        {
            if(CtrlPt>=0)
            {
                if (spline.selectedPoint()>=0)
                {
                    if(!spline.removeCtrlPoint(spline.selectedPoint()))
                    {
                        // The minimum number of control points has been reached for this spline degree
                        return;
                    }
                    spline.updateSpline();
                    spline.makeCurve();
                }
            }
        }
        else if (bShift)
        {
            spline.insertCtrlPoint(xd,yd);
            spline.updateSpline();
            spline.makeCurve();
        }
    }

    m_MoveTime.restart();

    update();
}


void SectionGraphWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    QPoint point;
    point = pEvent->pos();
    if(!m_bEnableSpline || m_iActiveSpline<0 || m_iActiveSpline>=m_Spline.count())
    {
        GraphWt::mouseMoveEvent(pEvent);
        return;
    }

    BSpline &spline = m_Spline[m_iActiveSpline];
    if ((pEvent->buttons() & Qt::LeftButton)  && spline.selectedPoint()>=0)
    {
        // user is dragging the point
        double x1 =  m_pGraph->clientTox(point.x());
        double y1 =  m_pGraph->clientToy(0, point.y());
        int ns = spline.selectedPoint();

        if(ns>=0)
        {
            spline.setCtrlPoint(ns, x1, y1);
        }

        if(ns==0 || ns==spline.ctrlPointCount()-1)
        {
            if(m_bOnCurve && m_pGraph->hasCurve())
            {
                Curve const *pCurve = m_pGraph->curve(m_iActiveSpline);
                if(pCurve)
                {
                    if(ns==0)
                    {
                        if(m_CurveIdxMin.at(m_iActiveSpline)<0)
                        {
                            emit splineConnected();
                        }
                        m_CurveIdxMin[m_iActiveSpline] = pCurve->closestPoint(x1, y1, m_pGraph->xScale(), m_pGraph->yScale(0));
                        spline.setCtrlPoint(ns, pCurve->x(m_CurveIdxMin[m_iActiveSpline]), pCurve->y(m_CurveIdxMin[m_iActiveSpline]));
                    }
                    else if(ns==spline.ctrlPointCount()-1)// n=last
                    {
                        if(m_CurveIdxMax.at(m_iActiveSpline)<0)
                        {
                            emit splineConnected();
                        }
                        m_CurveIdxMax[m_iActiveSpline] = pCurve->closestPoint(x1, y1, m_pGraph->xScale(), m_pGraph->yScale(0));
                        spline.setCtrlPoint(ns, pCurve->x(m_CurveIdxMax[m_iActiveSpline]), pCurve->y(m_CurveIdxMax[m_iActiveSpline]));
                    }
                }
            }
        }

        spline.makeCurve();
//        convertSpline();
        update();
    }
    else
    {
        // highlight if mouse passes over a point
        double x1 =  m_pGraph->clientTox(point.x());
        double y1 =  m_pGraph->clientToy(0, point.y());

        double dtolx = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->xScale());
        double dtoly = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->yScale(0));

        int n = spline.isCtrlPoint(x1, y1, dtolx, dtoly);
        if (n>=0 && n<int(spline.ctrlPointCount()))
        {
            spline.setHighlighted(n);
            update();
        }
        else
        {
            spline.setHighlighted(-1);
        }
        GraphWt::mouseMoveEvent(pEvent);
    }
}


void SectionGraphWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(m_iActiveSpline>=0 && m_iActiveSpline<m_Spline.count())
    {
        BSpline const &spline = m_Spline.at(m_iActiveSpline);
        if(spline.selectedPoint()>=0)  emit splineChanged(); // the user has been dragging a spline point
    }
    update();

    GraphWt::mouseReleaseEvent(pEvent);
}


void SectionGraphWt::resizeEvent(QResizeEvent *pEvent)
{
    GraphWt::resizeEvent(pEvent);
    resizeControls();
}


void SectionGraphWt::resizeControls()
{
    if(!m_pFrame) return;
    m_pFrame->adjustSize();

    QPoint pos0(width()-m_pFrame->width()-5, height()-m_pFrame->height()-5);
    m_pFrame->move(pos0);
    m_pFrame->setVisible(true);
}


void SectionGraphWt::makeSplines()
{
    if(m_Spline.size()==m_pGraph->curveCount()) return; // leave the splines as they are

    m_Spline.resize(m_pGraph->curveCount());
    m_CurveIdxMin.resize(m_pGraph->curveCount());
    m_CurveIdxMax.resize(m_pGraph->curveCount());
    m_CurveIdxMin.fill(-1);
    m_CurveIdxMax.fill(-1);

    for(int ic=0;ic<m_pGraph->curveCount(); ic++)
    {
        makeDefaultSpline(ic);
    }
}


void SectionGraphWt::makeDefaultSpline(int ispline)
{
    Curve *pCurve = m_pGraph->curve(ispline);
    if(!pCurve) return;
    BSpline &spline = m_Spline[ispline];
    QColor clr(pCurve->color());
//        clr.setHsv(clr.hslHue(), clr.saturation()*0.9, clr.value()*0.9);
    spline.setColor(clr.darker(150));
    spline.setStipple(Line::DASHDOT);
    spline.setWidth(3);

    int imin = pCurve->size()/2;
    int imax = pCurve->size()-1;

    Vector2d pt0(pCurve->x(imin), 0.0);
    Vector2d pt1(pCurve->x(imax), 0.0);
    spline.clearControlPoints();
    for(int ic=0; ic<5; ic++)
    {
        double tau = double(ic)/4.0;
        spline.appendControlPoint(pt0.x*(1.0-tau)+pt1.x*tau, -tau*(1.0-tau));
    }
    spline.updateSpline();
    spline.makeCurve();
    m_CurveIdxMin[ispline]= -1;
    m_CurveIdxMax[ispline]= -1;
}


void SectionGraphWt::makeControls()
{
    if(m_pFrame) qDeleteAll(m_pFrame->children());
    else  m_pFrame = new QFrame(this);
    m_pchCurve.clear();

    m_pFrame->setAttribute(Qt::WA_NoSystemBackground);
    m_pFrame->setAutoFillBackground(false);
    m_pFrame->setCursor(Qt::ArrowCursor);

    QGridLayout *pFrameLayout = new QGridLayout;
    {
        for(int ic=0; ic<m_pGraph->curveCount(); ic++)
        {
            Curve *pCurve = m_pGraph->curve(ic);
            QCheckBox *pchBox = new QCheckBox(m_pGraph->curve(ic)->name());
            m_pchCurve.append(pchBox);
            pchBox->setChecked(pCurve->isVisible());
            pchBox->setAttribute(Qt::WA_NoSystemBackground);
            pchBox->setAutoFillBackground(false);

            LineBtn *pBtn = new LineBtn(pCurve->theStyle());
            pBtn->setBackground(false);
            pFrameLayout->addWidget(pchBox, ic, 1);
            pFrameLayout->addWidget(pBtn,   ic, 2);
        }
    }
    m_pFrame->setLayout(pFrameLayout);

    resizeControls();
}


void SectionGraphWt::contextMenuEvent(QContextMenuEvent *pEvent)
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
        {
            QAction *m_pShowAll = new QAction("Show all", this);
            connect(m_pShowAll, SIGNAL(triggered(bool)), SLOT(onShowAllCurves()));
            QAction *m_pHideAll = new QAction("Hide all", this);
            connect(m_pHideAll, SIGNAL(triggered(bool)), SLOT(onHideAllCurves()));
            pContextMenu->addAction(m_pShowAll);
            pContextMenu->addAction(m_pHideAll);
            pContextMenu->addSeparator();
            QMenu *pResetSplineMenu = pContextMenu->addMenu("Reset spline");
            {
                for(int i=0; i<m_Spline.size(); i++)
                {
                    QAction *pResetSpline = new QAction(QString::asprintf("Spline %d", i));
                    pResetSpline->setData(i);
                    connect(pResetSpline, SIGNAL(triggered(bool)), SLOT(onResetSpline()));
                    pResetSplineMenu->addAction(pResetSpline);
                }
            }
            pContextMenu->addSeparator();
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
        }

        pContextMenu->exec(QCursor::pos());
        update();
    }
}


void SectionGraphWt::onResetSpline()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if(!pAction) return;
    bool bOK=false;
    int ispline = pAction->data().toInt(&bOK);
    if(!bOK || ispline<0 || ispline>m_Spline.size()) return;

    makeDefaultSpline(ispline);

    m_iActiveSpline = ispline;

    emit splineConnected();
    update();
}


void SectionGraphWt::onShowAllCurves()
{
    for(int ic=0; ic<m_pGraph->curveCount(); ic++)
        m_pGraph->curve(ic)->setVisible(true);
    for(int ich=0; ich<m_pchCurve.size(); ich++)
        m_pchCurve[ich]->setChecked(true);
    emit curvesChanged();
    update();
}


void SectionGraphWt::onHideAllCurves()
{
    for(int ic=0; ic<m_pGraph->curveCount(); ic++)
        m_pGraph->curve(ic)->setVisible(false);
    for(int ich=0; ich<m_pchCurve.size(); ich++)
        m_pchCurve[ich]->setChecked(false);

    m_iActiveSpline = -1;
    emit curvesChanged();
   update();
}
