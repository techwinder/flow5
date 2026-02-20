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
#include <QPaintEvent>

#include "fuselinewt.h"


#include <api/frame.h>
#include <api/fusesections.h>
#include <api/fusexfl.h>
#include <api/units.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>


FuseLineWt::FuseLineWt(QWidget *pParent, FuseXfl *pBody) : Section2dWt(pParent)
{
    m_ViewName = "Fuse outline";

    m_pFuseXfl = pBody;
    m_Grid.showXAxis(true);
    m_Grid.showYAxis(true);
    m_Grid.setnDecimals(1);
    createBaseActions();
    createContextMenu();
    setCursor(Qt::CrossCursor);
}


double FuseLineWt::defaultScale() const
{
    if(!m_pFuseXfl) return double(rect().width())*3/4;
    else            return double(rect().width())*3/4* 1.0/m_pFuseXfl->length();
}


void FuseLineWt::drawFuseLines()
{
    if(!m_pFuseXfl) return;
    QPainter painter(this);
    painter.save();

    double zpos=0;

    QPolygonF midLine;
    QPolygonF contourline;
    //Middle Line
    if(!m_pFuseXfl->isSectionType())
    {
        for (int k=0; k<m_pFuseXfl->frameCount();k++)
        {
            zpos = (m_pFuseXfl->frame(k).firstControlPoint().z +m_pFuseXfl->frame(k).lastControlPoint().z )/2.0;
            midLine.append(QPointF(m_pFuseXfl->frame(k).position().x*m_fScale + m_ptOffset.x(),
                                   zpos*(-m_fScale*m_fScaleY) + m_ptOffset.y()));
        }
    }
    else
    {
        FuseSections const *pFusePts = dynamic_cast<FuseSections*>(m_pFuseXfl);
        if(pFusePts->hasSections() && pFusePts->hasPoints())
        {
            for (int k=0; k<pFusePts->nSections();k++)
            {
                std::vector<Vector3d> const &section = pFusePts->sectionAt(k);
                zpos = (section.front().z +section.back().z )/2.0;
                midLine.append(QPointF(section.front().x*m_fScale + m_ptOffset.x(),
                                       zpos*(-m_fScale*m_fScaleY) + m_ptOffset.y()));
            }
        }
    }

    if(m_pFuseXfl->isFlatFaceType())
    {
        //Top Line
        for (int k=0; k<m_pFuseXfl->frameCount();k++)
        {
            contourline.append(QPointF(m_pFuseXfl->frame(k).position().x        *m_fScale              + m_ptOffset.x(),
                                   m_pFuseXfl->frame(k).firstControlPoint().z*(-m_fScale*m_fScaleY)+ m_ptOffset.y()));
        }

        //Bottom Line
        for (int k=m_pFuseXfl->frameCount()-1; k>=0;k--)
        {
            contourline.append(QPointF(m_pFuseXfl->frame(k).position().x        *m_fScale              + m_ptOffset.x(),
                                   m_pFuseXfl->frame(k).lastControlPoint().z*(-m_fScale*m_fScaleY) + m_ptOffset.y()));
        }
    }
    else
    {
        Vector3d Point;
        double xinc, u, v;

        int nh = 50;
        xinc = 1./double(nh-1);

        //top line
        u=0.0;
        v = 0.0;
        for (int i=0; i<=nh; i++)
        {
            m_pFuseXfl->getPoint(u,v,true, Point);
            contourline.append(QPointF(Point.x*m_fScale + m_ptOffset.x(),
                                       Point.z*(-m_fScale*m_fScaleY) + m_ptOffset.y()));
            u += xinc;
        }

        //bottom line
        u = 1.0;
        v = 1.0;
        for (int i=nh; i>0; i--)
        {
            m_pFuseXfl->getPoint(u,v,true, Point);
            contourline.append(QPointF(Point.x*m_fScale + m_ptOffset.x(),
                                       Point.z*(-m_fScale*m_fScaleY) + m_ptOffset.y()));
            u -= xinc;
        }
    }

    QPen linePen(xfl::fromfl5Clr(m_pFuseXfl->color()));
    linePen.setWidth(1);
    linePen.setStyle(Qt::DashLine);
    painter.setPen(linePen);
    painter.drawPolyline(midLine);

    linePen.setWidth(2);
    linePen.setStyle(Qt::SolidLine);
    painter.setPen(linePen);
    QColor clr = xfl::fromfl5Clr(m_pFuseXfl->color());
    QColor fillclr(clr);
    fillclr.setAlpha(75);
    QBrush fillbrush(fillclr);
    painter.setBrush(fillbrush);
    painter.drawPolygon(contourline);

    painter.restore();
}


void FuseLineWt::drawFusePoints()
{
    if(!m_pFuseXfl) return;

    QPainter painter(this);
    painter.save();
    QPen pointPen;

    QColor fuseclr = xfl::fromfl5Clr(m_pFuseXfl->color());
    QColor color;
    int w = 1;
    if(DisplayOptions::isLightTheme()) fuseclr = fuseclr.darker();
    else                               fuseclr = fuseclr.lighter();

    if(m_pFuseXfl->isFlatFaceType() || m_pFuseXfl->isSplineType())
    {
        for (int k=0; k<m_pFuseXfl->frameCount();k++)
        {
            if(m_pFuseXfl->activeFrameIndex()==k)
            {
                w = 4;
                color = xfl::fromfl5Clr(s_SelectStyle.m_Color);
            }
            else if(m_pFuseXfl->highlightedFrame()==k)
            {
                w = 4;
                color = xfl::fromfl5Clr(s_HighStyle.m_Color);
            }
            else
            {
                w = 2;
                color = fuseclr;
            }

            pointPen.setWidth(w);
            pointPen.setColor(color);
            painter.setPen(pointPen);
            double x = m_pFuseXfl->frame(k).position().x * m_fScale + m_ptOffset.x();
            double y = (m_pFuseXfl->frame(k).firstControlPoint().z + m_pFuseXfl->frame(k).lastControlPoint().z ) /2.0* (-m_fScale*m_fScaleY)+ m_ptOffset.y();
            xfl::drawSymbol(painter, Line::BIGCIRCLE, DisplayOptions::backgroundColor(), color, x, y );
        }
    }
    else if(m_pFuseXfl->isSectionType())
    {
        FuseSections const *pFusePts = dynamic_cast<FuseSections*>(m_pFuseXfl);
        for (int k=0; k<pFusePts->nSections();k++)
        {
            std::vector<Vector3d> const & section = pFusePts->sectionAt(k);
            if(pFusePts->activeSectionIndex()==k)
            {
                w = 4;
                color = xfl::fromfl5Clr(s_SelectStyle.m_Color);
            }
            else if(pFusePts->highlightedSectionIndex()==k)
            {
                w = 4;
                color = xfl::fromfl5Clr(s_HighStyle.m_Color);
            }
            else
            {
                w = 2;
                color = fuseclr;
            }

            pointPen.setWidth(w);
            pointPen.setColor(color);
            painter.setPen(pointPen);
            double x =  section.front().x * m_fScale + m_ptOffset.x();
            double y = (section.front().z + section.back().z ) /2.0* (-m_fScale*m_fScaleY) + m_ptOffset.y();
            xfl::drawSymbol(painter, Line::BIGCIRCLE, DisplayOptions::backgroundColor(), color, x, y);
        }
    }

    painter.restore();
}


void FuseLineWt::setXflFuse(FuseXfl *pBody)
{
    m_pFuseXfl = pBody;
    resetDefaultScale();
}


void FuseLineWt::paint(QPainter &painter)
{
    painter.save();

    updateScaleLabel();
    drawBackImage(painter);

    drawGrids(painter);

    drawFuseLines();
    drawFusePoints();

    painter.restore();
}


void FuseLineWt::onScaleBody()
{
    if(!m_pFuseXfl) return;

    emit(scaleFuse(false));
}


void FuseLineWt::onTranslateBody()
{
    if(!m_pFuseXfl) return;
    emit (translateFuse());
}


void FuseLineWt::onInsertPt()
{
    QPointF ptf = mousetoReal(m_PointDown);
    emit insertFrame(Vector3d(ptf.x(), 0.0, ptf.y()));
}


void FuseLineWt::onRemovePt()
{
    QPointF ptf = mousetoReal(m_PointDown);
    Vector2d Real(ptf.x(), ptf.y());

    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    int n = -1;
    if(!m_pFuseXfl->isSectionType())
        n = m_pFuseXfl->isFramePos(Vector3d(Real.x, 0.0, Real.y), dtolx, dtoly);
    else
    {
        FuseSections const *pFusePts = dynamic_cast<FuseSections*>(m_pFuseXfl);
        n = pFusePts->isSection(Vector3d(Real.x, 0.0, Real.y), dtolx, dtoly);
    }
    if (n>=0)
    {
        emit removeFrame(n);
    }
}


int FuseLineWt::highlightPoint(QPointF const&real)
{
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    Vector3d pt(real.x(), 0.0, real.y());

    if(!m_pFuseXfl->isSectionType())
    {
        int iframe = m_pFuseXfl->isFramePos(pt, dtolx, dtoly);
        if(iframe>=0)
            m_pFuseXfl->setHighlighted(iframe);
        return m_pFuseXfl->highlightedFrame();
    }
    else
    {
        FuseSections const *pFusePts = dynamic_cast<FuseSections*>(m_pFuseXfl);
        int isec = pFusePts->isSection(pt, dtolx, dtoly);
        pFusePts->setHighlightedSection(isec);
        return isec;
    }
    return -1;
}


int FuseLineWt::selectPoint(const QPointF &real)
{
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    Vector3d pt(real.x(), 0.0, real.y());

    int newindex = -1;
    if(!m_pFuseXfl->isSectionType())
    {
        newindex = m_pFuseXfl->isFramePos(pt, dtolx, dtoly);
        m_pFuseXfl->setActiveFrameIndex(newindex);
    }
    else
    {
        FuseSections const *pFusePts = dynamic_cast<FuseSections*>(m_pFuseXfl);
        newindex = pFusePts->isSection(pt, dtolx, dtoly);
        pFusePts->setActiveSectionIndex(newindex);
    }

    emit selectedChanged(newindex);
    return newindex;
}


void FuseLineWt::dragSelectedPoint(double x, double y)
{
    if(!m_pFuseXfl->isSectionType())
    {
        if(m_pFuseXfl->activeFrameIndex()<0) return;
        m_pFuseXfl->activeFrame().setPosition({x,0,y});
    }
    else
    {
        FuseSections *pFusePts = dynamic_cast<FuseSections*>(m_pFuseXfl);
        if(pFusePts->activeSectionIndex()<0) return;
        pFusePts->setActiveSectionPosition(x,y);
    }
}


void FuseLineWt::createContextMenu()
{
    QAction *pTranslateBody = new QAction("Translate body", this);
    connect(pTranslateBody,  SIGNAL(triggered()), this, SLOT(onTranslateBody()));

    QAction *pScaleBody = new QAction("Scale body", this);
    connect(pScaleBody,  SIGNAL(triggered()), this, SLOT(onScaleBody()));

    QAction *pInsertPt = new QAction("Insert control point\tShift+Click", this);
    connect(pInsertPt, SIGNAL(triggered()), this, SLOT(onInsertPt()));

    QAction *pRemovePt = new QAction("Remove control point \tCtrl+Click", this);
    connect(pRemovePt, SIGNAL(triggered()), this, SLOT(onRemovePt()));

    m_pSection2dContextMenu = new QMenu(this);
    {
        m_pSection2dContextMenu->addAction(pTranslateBody);
        m_pSection2dContextMenu->addAction(pScaleBody);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(pInsertPt);
        m_pSection2dContextMenu->addAction(pRemovePt);
        m_pSection2dContextMenu->addAction(m_pResetXYScaleAct);
        m_pSection2dContextMenu->addAction(m_pGridAct);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(m_pExportToSVG);
        m_pSection2dContextMenu->addSeparator();
        QMenu *pImageMenu = m_pSection2dContextMenu->addMenu(tr("Background image"));
        {
            pImageMenu->addAction(m_pLoadImage);
            pImageMenu->addAction(m_pClearImage);
            pImageMenu->addAction(m_pImageSettings);
        }
    }
}





