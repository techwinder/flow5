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

#define _MATH_DEFINES_DEFINED

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

#include "fuseframewt.h"
#include <api/frame.h>
#include <api/fusesections.h>
#include <api/fusexfl.h>
#include <api/units.h>

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>


bool FuseFrameWt::s_bCurFrameOnly = false;


FuseFrameWt::FuseFrameWt(QWidget *pParent, FuseXfl *pBody) : Section2dWt(pParent)
{
    setCursor(Qt::CrossCursor);
    m_ViewName = "Fuse frame";

    m_pFuseXfl = pBody;

    m_Grid.showXAxis(true);
    m_Grid.showYAxis(true);
    m_Grid.setnDecimals(2);

    createBaseActions();
    m_pShowCurFrameOnly = nullptr;
    m_pShowCurFrameOnly = new QAction("Show active frame only", this);
    m_pShowCurFrameOnly->setCheckable(true);
    m_pShowCurFrameOnly->setChecked(s_bCurFrameOnly);
    connect(m_pShowCurFrameOnly, SIGNAL(triggered()), SLOT(onShowCurFrameOnly()));

    createContextMenu();
}


double FuseFrameWt::defaultScale() const
{
    if(!m_pFuseXfl) return double(rect().width())*6.0/8.0;
    else return double(rect().width())/(m_pFuseXfl->height());
}


void FuseFrameWt::paint(QPainter &painter)
{
    painter.save();

    drawScaleLegend(painter);
    drawBackImage(painter);

    drawGrids(painter);

    drawFrameLines();
    drawFramePoints();

    painter.restore();
}


void FuseFrameWt::drawFrameLines()
{
    if(!m_pFuseXfl) return;

    Vector3d Point;
    double u=0, v=0;

    QPainter painter(this);
    painter.save();
    int nh = 100;
    //    xinc = 0.1;
    double hinc = 1.0/double(nh-1);

    QColor clr = xfl::fromfl5Clr(m_pFuseXfl->color());
    QPen framepen(clr);
    framepen.setWidth(2);
//    if(DisplayOptions::isLightTheme()) framepen.setColor(clr.darker());
//    else                               framepen.setColor(clr.lighter());

    QColor fillclr(clr);
    fillclr.setAlpha(75);
    QBrush fillbrush(fillclr);

    if(!m_pFuseXfl->isSectionType())
    {
        m_FramePolyline.resize(m_pFuseXfl->frameCount());
        for(int j=0; j<m_pFuseXfl->frameCount(); j++)
        {
            if(s_bCurFrameOnly && j!=m_pFuseXfl->activeFrameIndex()) continue;

            Frame const &frame = m_pFuseXfl->frameAt(j);

            m_FramePolyline[j].clear();
            QPolygonF leftpoly;
            if(m_pFuseXfl->isSplineType())
            {
                u = m_pFuseXfl->getu(frame.position().x);
                v = 0.0;
                for (int k=0; k<nh; k++)
                {
                    m_pFuseXfl->getPoint(u, v, true, Point);
                    m_FramePolyline[j].append({ Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()});
                    leftpoly.append(          {-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()});
                    v += hinc;
                }
            }
            else if(m_pFuseXfl->isFlatFaceType())
            {
                for (int k=0; k<m_pFuseXfl->sideLineCount();k++)
                {
                    m_FramePolyline[j].append({ frame.ctrlPointAt(k).y*m_fScale+m_ptOffset.x(), frame.ctrlPointAt(k).z* -m_fScale + m_ptOffset.y()});
                    leftpoly.append(          {-frame.ctrlPointAt(k).y*m_fScale+m_ptOffset.x(), frame.ctrlPointAt(k).z* -m_fScale + m_ptOffset.y()});
                }
            }
            for(int i=leftpoly.size()-2; i>=0; i--)
            {
                m_FramePolyline[j].append(leftpoly.at(i));
            }

            if(j==m_pFuseXfl->activeFrameIndex())
            {
                framepen.setStyle(Qt::SolidLine);
                painter.setPen(framepen);
                painter.setBrush(fillbrush);
                painter.drawPolygon(m_FramePolyline.at(j));
            }
            else if(j!=m_pFuseXfl->highlightedFrame())
            {
                framepen.setStyle(Qt::DashLine);
                painter.setPen(framepen);
                painter.setBrush(Qt::NoBrush);
                painter.drawPolyline(m_FramePolyline.at(j));
            }

            if(j==m_pFuseXfl->highlightedFrame())
            {
                QColor clr = xfl::fromfl5Clr(Section2dWt::highStyle().m_Color);
                QPen highpen(clr);
                highpen.setWidth(5);
                highpen.setStyle(Qt::SolidLine);
                painter.setPen(highpen);
                painter.setBrush(Qt::NoBrush);
                painter.drawPolygon(m_FramePolyline.at(j));
            }
        }
    }
    else
    {
        FuseSections const *pFuseFromPts = dynamic_cast<FuseSections const*>(m_pFuseXfl);
        m_FramePolyline.resize(pFuseFromPts->nSections());
        for(int j=0; j<pFuseFromPts->nSections(); j++)
        {
            if(s_bCurFrameOnly && j!=pFuseFromPts->activeSectionIndex()) continue;

            m_FramePolyline[j].clear();
            QPolygonF leftpoly;

            u = pFuseFromPts->getu(pFuseFromPts->sectionAt(j).front().x);
            v = 0.0;
            for (int k=0; k<nh; k++)
            {
                pFuseFromPts->getPoint(u,v,true, Point);
                m_FramePolyline[j].append({ Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()});
                leftpoly.append(          {-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()});
                v += hinc;
            }

            for(int i=leftpoly.size()-2; i>=0; i--)
            {
                m_FramePolyline[j].append(leftpoly.at(i));
            }

            if(j==pFuseFromPts->activeSectionIndex())
            {
                framepen.setStyle(Qt::SolidLine);
                painter.setPen(framepen);
                painter.setBrush(fillbrush);
                painter.drawPolygon(m_FramePolyline.at(j));
            }
            else
            {
                framepen.setStyle(Qt::DashLine);
                painter.setPen(framepen);
                painter.setBrush(Qt::NoBrush);
                painter.drawPolyline(m_FramePolyline.at(j));
            }
        }
    }

    painter.restore();
}


void FuseFrameWt::drawFramePoints()
{
    QPainter painter(this);

    QColor fuseclr = xfl::fromfl5Clr(m_pFuseXfl->color());
    QColor color;
    if(DisplayOptions::isLightTheme()) fuseclr = fuseclr.darker();
    else              fuseclr = fuseclr.lighter();

    if(m_pFuseXfl->isSplineType() || m_pFuseXfl->isFlatFaceType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0 || m_pFuseXfl->activeFrameIndex()>=m_pFuseXfl->frameCount()) return;
        painter.save();
        QPen pointpen;
        Frame const &frame = m_pFuseXfl->activeFrame();

        for (int k=0; k<frame.nCtrlPoints();k++)
        {
            if(Frame::selectedIndex()==k)
            {
                pointpen.setWidth(4);
                color = xfl::fromfl5Clr(Section2dWt::selectStyle().m_Color);
            }
            else if(Frame::highlighted()==k)
            {
                pointpen.setWidth(4);
                color = xfl::fromfl5Clr(Section2dWt::highStyle().m_Color);
            }
            else
            {
                pointpen.setWidth(2);
                color = fuseclr;
            }

            pointpen.setColor(color);
            painter.setPen(pointpen);

            xfl::drawSymbol(painter, Line::BIGCIRCLE, DisplayOptions::backgroundColor(), color,
                      frame.ctrlPointAt(k).y *  m_fScale +m_ptOffset.x(),
                      frame.ctrlPointAt(k).z * -m_fScale +m_ptOffset.y());
        }
        painter.restore();
    }
    else if(m_pFuseXfl->isSectionType())
    {
        FuseSections const *pFuseFromPts = dynamic_cast<FuseSections const*>(m_pFuseXfl);
        if(pFuseFromPts->activeSectionIndex()<0 || pFuseFromPts->activeSectionIndex()>=pFuseFromPts->nSections()) return;
        painter.save();
        QPen pointpen;
        std::vector<Vector3d> const &section = pFuseFromPts->activeSection();

        for(int k=0; k<int(section.size()); k++)
        {
            if(pFuseFromPts->activePointIndex()==k)
            {
                pointpen.setWidth(4);
                color = xfl::fromfl5Clr(Section2dWt::selectStyle().m_Color);
            }
            else if(pFuseFromPts->highlightedPointIndex()==k)
            {
                pointpen.setWidth(4);
                color = xfl::fromfl5Clr(Section2dWt::highStyle().m_Color);
            }
            else
            {
                pointpen.setWidth(2);
                color = fuseclr;
            }

            pointpen.setColor(color);
            painter.setPen(pointpen);

            xfl::drawSymbol(painter, Line::BIGCIRCLE, DisplayOptions::backgroundColor(), color,
                      section.at(k).y *  m_fScale +m_ptOffset.x(),
                      section.at(k).z * -m_fScale +m_ptOffset.y());
        }
        painter.restore();
    }
}


void FuseFrameWt::setBody(FuseXfl *pBody)
{
    m_pFuseXfl = pBody;
    resetDefaultScale();
}


void FuseFrameWt::onInsertPt()
{
    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0) return;
        Frame const &pFrame = m_pFuseXfl->activeFrame();

        QPointF ptf = mousetoReal(m_PointDown);
        Vector2d real(ptf.x(), ptf.y());

        Vector3d pt;
        pt.z = real.y;
        pt.y = real.x;
        pt.x = pFrame.position().x;

        emit insertPoint(pt);
    }
    else
    {
        FuseSections const *pFuseSecs = dynamic_cast<FuseSections const*>(m_pFuseXfl);
        if(pFuseSecs->activeSectionIndex()<0) return;
        std::vector<Vector3d> const &sec = pFuseSecs->activeSection();

        QPointF ptf = mousetoReal(m_PointDown);
        Vector2d real(ptf.x(), ptf.y());

        Vector3d pt;
        pt.z = real.y;
        pt.y = real.x;
        pt.x = sec.front().x;

        emit insertPoint(pt);
    }
}


void FuseFrameWt::onRemovePt()
{
    QPointF ptf = mousetoReal(m_PointDown);
    Vector2d real(ptf.x(), ptf.y());

    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0) return;
        Frame const &pFrame = m_pFuseXfl->activeFrame();

        Vector3d pt;
        pt.z = real.y;
        pt.y = real.x;
        pt.x = pFrame.position().x;

        double dtolx = double(s_nPixelSelection)/m_fScale;
        double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

        int n =   pFrame.isPoint(pt, dtolx, 0.005, dtoly);
        if (n>=0)  emit removePoint(n);
    }
    else
    {
        FuseSections const *pFuseSecs = dynamic_cast<FuseSections const*>(m_pFuseXfl);
        if(pFuseSecs->activeSectionIndex()<0) return;
        std::vector<Vector3d> const &section = pFuseSecs->activeSection();
        if(section.size()<=0) return;

        Vector3d pt;
        pt.z = real.y;
        pt.y = real.x;
        pt.x = section.front().x;

        double dtolx = double(s_nPixelSelection)/m_fScale;
        double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

        int n =   pFuseSecs->isPoint(pt, dtolx, 0.005, dtoly);
        if (n>=0)  emit removePoint(n);
    }
}


void FuseFrameWt::onScaleFrame()
{
    if(!m_pFuseXfl) return;
    emit scaleBody(true);
}


void FuseFrameWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!m_bZoomPlus && !pEvent->buttons())
    {
        //highlight the frame
        QPoint point = pEvent->pos();
        int iFrame  = isFrame(point);
        m_pFuseXfl->setHighlighted(iFrame);
        update();
    }

    Section2dWt::mouseMoveEvent(pEvent);
}


void FuseFrameWt::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint point = pEvent->pos();

    stopDynamicTimer();
    m_MoveTime.restart();

    // get a reference for mouse movements
    m_PointDown.rx() = point.x();
    m_PointDown.ry() = point.y();
    m_LastPoint = m_PointDown;

    if(m_bZoomPlus)
    {
        m_ZoomRect.setTopLeft(point);
        m_ZoomRect.setBottomRight(point);
    }
    else if(!m_bZoomPlus && (pEvent->buttons() & Qt::LeftButton))
    {
        if(pEvent->modifiers() & Qt::MetaModifier)
        {
            // insert debug point
        }
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
                //select the frame
                int iFrame  = isFrame(point);

                if(iFrame>=0)
                {
                    emit frameSelected(iFrame);
                }
                else
                {
                    //dragging the view
                    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
                    m_bTrans = true;
                }
            }
        }
    }
    pEvent->accept();
}


int FuseFrameWt::highlightPoint(const QPointF &real)
{
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;
    Vector3d pt;
    pt.z = real.y();
    pt.y = real.x();

    if(!m_pFuseXfl->isSectionType())
    {
        int ifr = m_pFuseXfl->activeFrameIndex();
        if(ifr<0 || ifr>=m_pFuseXfl->frameCount())
        {
            Frame::setHighlighted(-1);
            return -1;
        }

        Frame const &pFrame = m_pFuseXfl->activeFrame();
        pt.x = pFrame.position().x;

        Frame::setHighlighted(pFrame.isPoint(pt, 0.005, dtolx, dtoly));

        return Frame::highlighted();
    }
    else
    {
        FuseSections const *pFuseFromPts = dynamic_cast<FuseSections const*>(m_pFuseXfl);

        int ifr = pFuseFromPts->activeSectionIndex();
        if(ifr<0 || ifr>=pFuseFromPts->nSections())
        {
            pFuseFromPts->setHighlightedPoint(-1);
            return -1;
        }

        std::vector<Vector3d> const &section = pFuseFromPts->activeSection();
        if(!section.size())
        {
            pFuseFromPts->setHighlightedPoint(-1);
            return -1;
        }

        pt.x = section.front().x;
        int ipt = pFuseFromPts->isPoint(pt, 0.005, dtolx, dtoly);
        pFuseFromPts->setHighlightedPoint(ipt);

        return ipt;
    }
}


int FuseFrameWt::isFrame(QPoint pointer)
{
    // test the proximity to the drawn polyline
    for(int is=0; is<m_FramePolyline.size(); is++)
    {
        QPolygonF const &poly = m_FramePolyline.at(is);
        for(int ip=0; ip<poly.size()-1; ip++)
        {
            double x1 = poly.at(ip).x();
            double x2 = poly.at(ip+1).x();
            double y1 = poly.at(ip).y();
            double y2 = poly.at(ip+1).y();
            double d2  = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
            if(d2>PRECISION)
            {
                double dotproduct = (pointer.x()-x1)*(pointer.x()-x2) + (pointer.y()-y1)*(pointer.y()-y2);

                if(dotproduct<=0.0)
                {
                    // mouse point is between two output points
                    // check distance to line
                    double h = (pointer.x()-x1)*(y2-y1) - (x2-x1)*(pointer.y()-y1);
                    h *= 1.0/sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
                    if(fabs(h)<s_nPixelSelection)
                        return is;
                }
            }
            if(sqrt((pointer.x()-x1)*(pointer.x()-x1)+(pointer.y()-y1)*(pointer.y()-y1))<s_nPixelSelection)
            {
                return is;
            }
        }
    }

    return -1;
}


int FuseFrameWt::selectPoint(const QPointF &real)
{
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0)
        {
            Frame::setSelected(-1);
            return -1;
        }
        Frame const &pFrame = m_pFuseXfl->activeFrame();

        Vector3d pt;
        pt.z = real.y();
        pt.y = real.x();
        pt.x = pFrame.position().x;

        Frame::setSelected(pFrame.isPoint(pt, 0.005, dtolx, dtoly));

        emit selectedChanged(Frame::selectedIndex());

        return Frame::selectedIndex();
    }
    else
    {
        FuseSections const *pFuseFromPts = dynamic_cast<FuseSections const*>(m_pFuseXfl);

        if(pFuseFromPts->activeSectionIndex()<0 || pFuseFromPts->activeSectionIndex()>=pFuseFromPts->nSections())
        {
            pFuseFromPts->setHighlightedPoint(-1);
            pFuseFromPts->setActivePoint(-1);
            return -1;
        }

        std::vector<Vector3d >const &section = pFuseFromPts->activeSection();

        Vector3d pt;
        pt.z = real.y();
        pt.y = real.x();
        pt.x = section.front().x;

        int ipt = pFuseFromPts->isPoint(pt, 0.005, dtolx, dtoly);
        pFuseFromPts->setActivePoint(ipt);

        emit selectedChanged(ipt);

        return ipt;
    }
    return -1;
}


void FuseFrameWt::dragSelectedPoint(double x, double y)
{
    if(!m_pFuseXfl->isSectionType())
    {
        if (m_pFuseXfl->activeFrameIndex()<0 || (Frame::selectedIndex()<0) ||
            (Frame::selectedIndex() > m_pFuseXfl->activeFrame().nCtrlPoints())) return;

        if(Frame::selectedIndex()==0 || Frame::selectedIndex()==m_pFuseXfl->activeFrame().nCtrlPoints()-1) x=0.0;
        x = std::max(x,0.0);
        m_pFuseXfl->activeFrame().setSelectedPoint(Vector3d(m_pFuseXfl->activeFrame().position().x, x, y));
    }
    else
    {
        FuseSections *pFuseFromPts = dynamic_cast<FuseSections *>(m_pFuseXfl);
        if(pFuseFromPts->activeSectionIndex()<0 || pFuseFromPts->activeSectionIndex()>=pFuseFromPts->nSections())
            return;
        if(pFuseFromPts->activePointIndex()==0 || pFuseFromPts->activePointIndex()==pFuseFromPts->pointCount()-1) x=0.0;
        x = std::max(x,0.0);
        Vector3d pt(pFuseFromPts->activeSection().front().x, x, y);
        pFuseFromPts->setSelectedPoint(pt);
    }
}


void FuseFrameWt::drawScaleLegend(QPainter &painter)
{
    painter.save();
    painter.setFont(DisplayOptions::textFontStruct().font());

    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);
    painter.drawText(5,1*DisplayOptions::textFontStruct().height(), QString("X-Scale = %1").arg(m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,2*DisplayOptions::textFontStruct().height(), QString("Y-Scale = %1").arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,3*DisplayOptions::textFontStruct().height(), QString("x  = %1").arg(m_CursorPos.x() * Units::mtoUnit(),7,'f',2) + Units::lengthUnitQLabel());
    painter.drawText(5,4*DisplayOptions::textFontStruct().height(), QString("y  = %1").arg(m_CursorPos.y() * Units::mtoUnit(),7,'f',2) + Units::lengthUnitQLabel());
    painter.restore();
}



void FuseFrameWt::createContextMenu()
{
    QAction *pScaleFrame = new QAction("Scale frame", this);
    connect(pScaleFrame,  SIGNAL(triggered()), SLOT(onScaleFrame()));

    QAction *pInsertPt = new QAction("Insert control point\tShift+Click", this);
    connect(pInsertPt, SIGNAL(triggered()), SLOT(onInsertPt()));

    QAction *pRemovePt = new QAction("Remove control point\tCtrl+Click", this);
    connect(pRemovePt, SIGNAL(triggered()), SLOT(onRemovePt()));

    m_pSection2dContextMenu = new QMenu(this);
    {
        m_pSection2dContextMenu->addAction(m_pShowCurFrameOnly);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(pScaleFrame);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(pInsertPt);
        m_pSection2dContextMenu->addAction(pRemovePt);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(m_pResetXYScaleAct);
        m_pSection2dContextMenu->addAction(m_pGridAct);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(m_pExportToSVG);
        m_pSection2dContextMenu->addSeparator();
        QMenu *pImageMenu = m_pSection2dContextMenu->addMenu("Background image");
        {
            pImageMenu->addAction(m_pLoadImage);
            pImageMenu->addAction(m_pClearImage);
            pImageMenu->addAction(m_pImageSettings);
        }
    }
}


void FuseFrameWt::onShowCurFrameOnly()
{
    s_bCurFrameOnly = !s_bCurFrameOnly;
    m_pShowCurFrameOnly->setChecked(s_bCurFrameOnly);
    update();
}


void FuseFrameWt::setFrameProperties(QString const &data)
{
    //reset style, just in case settings have changed
/*    m_plabFrameProps->setFont(DisplayOptions::textFontStruct().font());

    QPalette palette;
    palette.setColor(QPalette::WindowText, s_TextColor);
    m_plabFrameProps->setPalette(palette);

    m_plabFrameProps->setText(data);
  */
    m_plabInfoOutput->setText(data);
}




