/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QPainter>
#include <QTime>
#include <QMouseEvent>
#include <QToolTip>
#include <QApplication>

#include "sailsectionview.h"



#include <xflcore/xflcore.h>
#include <xflcore/displayoptions.h>
#include <xflfoil/globals/objects2d_globals.h>
#include <xflgeom/geom3d/frame.h>
#include <xflgeom/geom3d/nurbssurface.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflgeom/geom_globals/geom_params.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflobjects/objects_globals/objects_global.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>
#include <xflwidgets/view/section2doptions.h>

LineStyle SailSectionView::s_SectionStyle = LineStyle(true, Line::SOLID, 2, Qt::gray, Line::NOSYMBOL);
bool SailSectionView::s_bFill = true;
int SailSectionView::s_iSectionHighlight = -1;

SailSectionView::SailSectionView() : Section2dWt()
{
    m_ViewName = "Sail section";

    m_pSail = nullptr;
    m_pNurbs = nullptr;

    createBaseActions();
    m_pShowNormals = new QAction("Show normals", this);
    m_pShowNormals->setShortcut(QKeySequence(Qt::ALT | Qt::Key_N));
    m_pShowNormals->setCheckable(true);
    connect(m_pShowNormals, SIGNAL(triggered(bool)), SLOT(update()));
    createContextMenu();
}

/*
void SailSectionView::createActions()
{
    Section2dView::createActions();
} */


void SailSectionView::onFillFoil(bool bFill)
{
    s_bFill = bFill;
    update();
}


void SailSectionView::onShowNormals()
{
    update();
}


void SailSectionView::setSail(Sail *pSail)
{
    m_pSail  = pSail;
    if(m_pSail->isNURBSSail())
    {
        NURBSSail *pNS = dynamic_cast<NURBSSail*>(pSail);
        m_pNurbs = pNS->pNurbs();
    }
}


int SailSectionView::highlightPoint(const QPointF &real)
{
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    if(m_pSail->isSplineSail())
    {
        SplineSail *pSS = dynamic_cast<SplineSail*>(m_pSail);
        Spline *pActiveSpline = pSS->activeSpline();
        if(!pActiveSpline)
        {
            pSS->setActiveSection(-1);
            return -1;
        }
        else
        {
            int iCtrlPt = pActiveSpline->isCtrlPoint(real.x(), real.y(), dtolx, dtoly);
            pActiveSpline->setHighlighted(iCtrlPt);
            return pActiveSpline->highlightedPoint();
        }
    }
    else if(m_pSail->isNURBSSail())
    {
        NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
        if(pNS->activeFrameIndex()<0)
        {
            Frame::setSelected(-1);
            return -1;
        }
        Frame const &pActiveframe = pNS->activeFrame();
        Vector3d pt;
        pt.x = real.x();
        pt.y = real.y();
        pt.z = pActiveframe.position().z;
        Frame::setHighlighted(pActiveframe.isPoint(pt, dtolx, dtoly, 0.005));
        return Frame::highlighted();
    }
    return -1;
}


int SailSectionView::isSection(const QPointF &pointer)
{
    if(m_pSail->isSplineSail())
    {
        QPointF  realpt = mousetoReal(pointer);
        SplineSail *pSS = dynamic_cast<SplineSail*>(m_pSail);

        for(int is=0; is<pSS->sectionCount(); is++)
        {
            Spline const *pSpline = pSS->splineAt(is);

            for(int ip=0; ip<pSpline->outputSize()-1; ip++)
            {
                double x1 = pSpline->outputPt(ip).x;
                double x2 = pSpline->outputPt(ip+1).x;
                double y1 = pSpline->outputPt(ip).y;
                double y2 = pSpline->outputPt(ip+1).y;
                double d2  = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                if(d2>PRECISION)
                {
                    double dotproduct = (realpt.x()-x1)*(realpt.x()-x2) + (realpt.y()-y1)*(realpt.y()-y2);

                    if(dotproduct<=0.0)
                    {
                        // mouse point is between two output points
                        // check distance to line
                        double h = (realpt.x()-x1)*(y2-y1) - (x2-x1)*(realpt.y()-y1);
                        h *= 1.0/sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
                        if(fabs(h)<s_nPixelSelection/m_fScale)
                            return is;
                    }
                }
                if(sqrt((realpt.x()-x1)*(realpt.x()-x1)+(realpt.y()-y1)*(realpt.y()-y1))<s_nPixelSelection/m_fScale)
                {
                    return is;
                }
            }
        }
    }
    else if(m_pSail->isNURBSSail())
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
    }
    else if(m_pSail->isWingSail())
    {
        WingSail const *pWS = dynamic_cast<WingSail*>(m_pSail);
        QPointF  realpt = mousetoReal(pointer);

        double pixelDist = 5.0, d2=0;
        double x0=0;
        double xf1=0,xf2=0,yf1=0,yf2=0;
        double x1=0,x2=0,y1=0,y2=0;
        for(int is=0; is<pWS->sectionCount(); is++)
        {
            WingSailSection const &sec = pWS->sectionAt(is);
            double cost = cos(sec.twist()*PI/180.0);
            double sint = sin(sec.twist()*PI/180.0);
            QString FoilName = sec.foilName();
            Foil *pOldFoil = Objects2d::foil(FoilName);
            if(!pOldFoil || pOldFoil->nNodes()==0 || !pOldFoil->isVisible())
            {
            }
            else
            {
                for(int ip=0; ip<pOldFoil->nNodes()-1; ip++)
                {
                    x0 = pWS->xPosition(is);
                    xf1 = pOldFoil->x(ip)   * sec.chord();
                    xf2 = pOldFoil->x(ip+1) * sec.chord();
                    yf1 = pOldFoil->y(ip)   * sec.chord();
                    yf2 = pOldFoil->y(ip+1) * sec.chord();
                    x1 =xf1*cost - yf1*sint;
                    y1 =xf1*sint + yf1*cost;
                    x2 =xf2*cost - yf2*sint;
                    y2 =xf2*sint + yf2*cost;

                    x1 += x0;
                    x2 += x0;
                    d2  = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                    if(d2>PRECISION)
                    {
                        double dotproduct = (realpt.x()-x1)*(realpt.x()-x2) + (realpt.y()-y1)*(realpt.y()-y2);

                        if(dotproduct<=0.0)
                        {
                            // mouse point is between two output points
                            // check distance to line
                            double h = (realpt.x()-x1)*(y2-y1) - (x2-x1)*(realpt.y()-y1);
                            h *= 1.0/sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
                            if(fabs(h)<s_nPixelSelection/m_fScale)
                                return is;
                        }
                    }
                    if(sqrt((realpt.x()-x1)*(realpt.x()-x1)+(realpt.y()-y1)*(realpt.y()-y1))<pixelDist/m_fScale)
                    {
                        return is;
                    }
                }
            }
        }
    }

    return -1;
}


int SailSectionView::selectPoint(const QPointF &real)
{
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    if(m_pSail->isSplineSail())
    {
        SplineSail *pSS = dynamic_cast<SplineSail*>(m_pSail);
        Spline *pActiveSpline = pSS->activeSpline();
        if(!pActiveSpline)
        {
            pSS->setActiveSection(-1);
            emit selectedChanged(-1); // notify the world
        }
        else
        {
            int iCtrlPt = pActiveSpline->isCtrlPoint(real.x(), real.y(), dtolx, dtoly);
            pActiveSpline->setSelectedPoint(iCtrlPt);
            emit selectedChanged(pActiveSpline->selectedPoint()); // notify the world
        }
        return pActiveSpline->selectedPoint();
    }
    else if(m_pSail->isNURBSSail())
    {
        NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);

        if(pNS->activeFrameIndex()<0)
        {
            Frame::setSelected(-1);
            return -1;
        }

        Frame const &activeframe = pNS->activeFrame();
        Vector3d pt;
        pt.x = real.x();
        pt.y = real.y();
        pt.z = activeframe.position().z;
        Frame::setSelected(activeframe.isPoint(pt, dtolx, dtoly, 0.005));

        emit selectedChanged(Frame::selectedIndex()); // notify the world
        return Frame::selectedIndex();
    }
    return -1;
}


void SailSectionView::dragSelectedPoint(double x, double y)
{
    if(m_pSail->isSplineSail())
    {
        SplineSail *pSS = dynamic_cast<SplineSail*>(m_pSail);
        Spline *pActiveSpline = pSS->activeSpline();
        if (!pActiveSpline ) return;
        if(pActiveSpline->selectedPoint()==0) y=0.0;
        pActiveSpline->setSelectedPoint(Vector2d(x,y));
        pActiveSpline->updateSpline();
        pActiveSpline->makeCurve();
    }
    else if(m_pSail->isNURBSSail())
    {
        NURBSSail *pNS = dynamic_cast<NURBSSail*>(m_pSail);
        if(pNS->activeFrameIndex()<0) return;
        Frame &activeframe = pNS->activeFrame();

        if(activeframe.selectedIndex()==0) y=0.0;
        activeframe.setSelectedPoint(Vector3d(x,y,activeframe.position().z));
    }
}


void SailSectionView::onInsertPt()
{

}

void SailSectionView::onRemovePt()
{

}


void SailSectionView::paint(QPainter &painter)
{
    updateScaleLabel();
    drawBackImage(painter);
    drawGrids(painter);
    if(m_pSail->isSplineSail())
    {
        drawSplineLines(painter);
        drawSplineCtrlPoints(painter);
        drawSplineTangents(painter);
        if(m_pShowNormals->isChecked())
        {
            drawSplineNormals(painter);
        }
    }
    else if(m_pSail->isNURBSSail())
    {
        drawFrameLines(painter);
        drawFramePoints(painter);
        drawFrameTangents(painter);
    }
    else if(m_pSail->isWingSail())
    {
        drawFoils(painter);
    }
}


double SailSectionView::defaultScale() const
{
    if(!m_pSail) return double(rect().width())*3/4;
    else
    {
        double L = std:: max(1.0, m_pSail->length());
        return double(rect().width())/L*0.75;
    }
}


void SailSectionView::drawSplineCtrlPoints(QPainter &painter)
{
    if(!m_pSail || !m_pSail->isSplineSail()) return;
    SplineSail *pSSail = dynamic_cast<SplineSail*>(m_pSail);
    Spline const *pActiveSpline = pSSail->activeSpline();
    if(!pActiveSpline) return;

    painter.save();

    QPen pointPen(s_SectionStyle.m_Color);
    pointPen.setCosmetic(true);
    QColor linecolor;

    for (int k=0; k<int(pActiveSpline->ctrlPointCount());k++)
    {
        if(pActiveSpline->selectedPoint()==k)
        {
            pointPen.setWidth(4);
            pointPen.setColor(Qt::red);
        }
        else if(pActiveSpline->highlightedPoint()==k)
        {
            pointPen.setWidth(4);
            pointPen.setColor(pSSail->color().lighter());
        }
        else
        {
            pointPen.setWidth(2);
            pointPen.setColor(pSSail->color());
        }

        painter.setPen(pointPen);

        double x = pActiveSpline->controlPoint(k).x * m_fScale   +m_ptOffset.x();
        double y = pActiveSpline->controlPoint(k).y * (-m_fScale*m_fScaleY)+m_ptOffset.y();
        xfl::drawSymbol(painter, Line::BIGSQUARE, DisplayOptions::backgroundColor(), linecolor, x, y);
    }
    painter.restore();
}


void SailSectionView::createContextMenu()
{
    QAction *pResetScaleAction = new QAction("Reset scales", this);
    connect(pResetScaleAction, SIGNAL(triggered()), SLOT(onResetScales()));

    QAction *pGridSettingsAction = new QAction("Grid settings", this);
    connect(pGridSettingsAction, SIGNAL(triggered()), SLOT(onGridSettings()));

    m_pSection2dContextMenu = new QMenu(this);
    {
        m_pSection2dContextMenu->addAction(pResetScaleAction);
        m_pSection2dContextMenu->addAction(pGridSettingsAction);
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(m_pExportToSVG);
        m_pSection2dContextMenu->addSeparator();
        QMenu *pImageMenu = m_pSection2dContextMenu->addMenu("Background image");
        {
            pImageMenu->addAction(m_pLoadImage);
            pImageMenu->addAction(m_pClearImage);
            pImageMenu->addAction(m_pImageSettings);
        }
        m_pSection2dContextMenu->addSeparator();
        m_pSection2dContextMenu->addAction(m_pShowNormals);
    }
}


/** draws the first and last tangents linking the control points */
void SailSectionView::drawSplineNormals(QPainter &painter)
{
    if(!m_pSail || !m_pSail->isSplineSail()) return;
    SplineSail *pSSail = dynamic_cast<SplineSail*>(m_pSail);
    Spline const *pActiveSpline = pSSail->activeSpline();
    if(!pActiveSpline) return;

    painter.save();

    QColor tgtclr = Qt::red;
    if(DisplayOptions::isLightTheme()) tgtclr = pActiveSpline->color().darker();
    else              tgtclr = pActiveSpline->color().lighter();
    QPen tgpen(tgtclr);
    tgpen.setCosmetic(true);
    tgpen.setStyle(Qt::DashDotLine);
    tgpen.setWidth(1);
    painter.setPen(tgpen);

    drawNormals(pActiveSpline, painter, m_fScale,m_fScaleY, m_ptOffset);

    painter.restore();
}


/** draws the first and last tangents linking the control points */
void SailSectionView::drawSplineTangents(QPainter &painter)
{
    if(!m_pSail || !m_pSail->isSplineSail()) return;
    SplineSail *pSSail = dynamic_cast<SplineSail*>(m_pSail);
    Spline const *pActiveSpline = pSSail->activeSpline();
    if(!pActiveSpline) return;

    int npts = pActiveSpline->ctrlPointCount();
    if(npts<2) return;

    painter.save();

    QColor tgtclr = Qt::lightGray;
    if(DisplayOptions::isLightTheme()) tgtclr = Qt::darkGray;
    QPen tgpen(tgtclr);
    tgpen.setCosmetic(true);
    tgpen.setStyle(Qt::DashDotLine);
    tgpen.setWidth(0);
    painter.setPen(tgpen);

    int x=0, y=0, x1=0, y1=0;
    x  = int(pActiveSpline->controlPoint(0).x * m_fScale              + m_ptOffset.x());
    y  = int(pActiveSpline->controlPoint(0).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());
    x1 = int(pActiveSpline->controlPoint(1).x * m_fScale              + m_ptOffset.x());
    y1 = int(pActiveSpline->controlPoint(1).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());
    painter.drawLine(x,y,x1,y1);

    x  = int(pActiveSpline->controlPoint(npts-1).x * m_fScale              + m_ptOffset.x());
    y  = int(pActiveSpline->controlPoint(npts-1).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());
    x1 = int(pActiveSpline->controlPoint(npts-2).x * m_fScale              + m_ptOffset.x());
    y1 = int(pActiveSpline->controlPoint(npts-2).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());

    painter.drawLine(x,y,x1,y1);
    painter.restore();
}


/** WingSail case only*/
void SailSectionView::drawFoils(QPainter &painter)
{
    if(!m_pSail) return;
    if(!m_pSail->isWingSail()) return;

    painter.save();

    WingSail *pWS = dynamic_cast<WingSail*>(m_pSail);
    for(int is=0; is<pWS->sectionCount(); is++)
    {
        WingSailSection const &section = pWS->section(is);
        Foil *pFoil = Objects2d::foil(section.foilName());
        if(pFoil)
        {
            QPen FoilPen, CenterPen;
            FoilPen.setCosmetic(true);
            CenterPen.setCosmetic(true);

            QBrush FillBrush(DisplayOptions::backgroundColor());
            painter.setBrush(FillBrush);

            bool bActive = is==pWS->activeSection();
            bool bFill = s_bFill && bActive;
            if(is==s_iSectionHighlight)
            {
                FoilPen.setWidth(s_HighStyle.m_Width);
                FoilPen.setColor(s_HighStyle.m_Color);
                FoilPen.setStyle(xfl::getStyle(s_HighStyle.m_Stipple));
            }
            else
            {
                if(bActive)
                {
                    FoilPen.setWidth(s_SectionStyle.m_Width+2);
                    FoilPen.setColor(s_SectionStyle.m_Color);
                    FoilPen.setStyle(xfl::getStyle(s_SectionStyle.m_Stipple));
                }
                else
                {
                    FoilPen.setWidth(s_SectionStyle.m_Width);
                    FoilPen.setColor(s_SectionStyle.m_Color);
                    FoilPen.setStyle(xfl::getStyle(s_SectionStyle.m_Stipple));
                }
            }
            painter.setPen(FoilPen);

            QColor clr = s_SectionStyle.m_Color;
            clr.setAlpha(75);
            double scalex = m_fScale*section.chord();
            double scaley = m_fScale*m_fScaleY*section.chord();
            QPointF off(m_ptOffset.x()+pWS->xPosition(is)*m_fScale, m_ptOffset.y());
            drawFoil(painter, pFoil, 0.0, -section.twist(), scalex, scaley, off, bFill, clr);
//            drawFoilPoints(painter,  pFoil, section.twist(), scalex, scaley, off, DisplayOptions::backgroundColor(), rect());

            if (pFoil->isCamberLineVisible())
            {
                FoilPen.setStyle(Qt::DashLine);
                painter.setPen(CenterPen);
                drawFoilMidLine(painter, pFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
            }
        }
    }
    painter.restore();
}


void SailSectionView::drawSplineLines(QPainter &painter)
{
    if(!m_pSail) return;
    if(!m_pSail->isSplineSail()) return;
    SplineSail *pSSail = dynamic_cast<SplineSail*>(m_pSail);
    Spline const *pActiveSpline = pSSail->activeSpline();

    painter.save();

    QPen framepen(s_SectionStyle.m_Color);
    framepen.setCosmetic(true);
    for(int is=0; is<pSSail->sectionCount(); is++)
    {
        Spline const *pSpline = pSSail->spline(is);
        if(pSpline)
        {
            if(is==s_iSectionHighlight)
            {
                framepen.setWidth(s_HighStyle.m_Width);
                framepen.setColor(s_HighStyle.m_Color);
                framepen.setStyle(xfl::getStyle(s_HighStyle.m_Stipple));
            }
            else
            {
                framepen.setColor(s_SectionStyle.m_Color);
                if(pActiveSpline==pSpline)
                {
                    framepen.setStyle(Qt::SolidLine);
                    framepen.setWidth(3);
                }
                else
                {
                    framepen.setStyle(Qt::DashLine);
                    framepen.setWidth(2);
                }
            }

            painter.setPen(framepen);
            drawSpline(pSpline, painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
        }
    }

    painter.restore();
}


void SailSectionView::drawFrameLines(QPainter &painter)
{
    if(!m_pNurbs) return;

    Vector3d pt;
    double u=0, v=0;

    int nh= 29;

    painter.save();

    double vInc = 1.0/double(nh-1);

    QPen framepen(s_SectionStyle.m_Color);
    framepen.setCosmetic(true);

    m_FramePolyline.resize(m_pNurbs->frameCount());

    for(int j=0; j<m_pNurbs->frameCount(); j++)
    {
        Frame const &pfr = m_pNurbs->frame(j);
        if(j==s_iSectionHighlight)
        {
            framepen.setWidth(s_HighStyle.m_Width);
            framepen.setColor(s_HighStyle.m_Color);
            framepen.setStyle(xfl::getStyle(s_HighStyle.m_Stipple));
        }
        else
        {
            framepen.setColor(s_SectionStyle.m_Color);
            if(j==m_pNurbs->activeFrameIndex())
            {
                framepen.setStyle(Qt::SolidLine);
                framepen.setWidth(3);
            }
            else
            {
                framepen.setStyle(Qt::DashLine);
                framepen.setWidth(2);
            }
        }
        painter.setPen(framepen);
        m_FramePolyline[j].clear();
        v = 0.0;
        u = m_pNurbs->getu(pfr.position().z, v);
        for (int k=0; k<nh; k++)
        {

            m_pNurbs->getPoint(u, v, pt);
            double x = pt.x*m_fScale+m_ptOffset.x();
            double y = pt.y* (-m_fScale)*m_fScaleY + m_ptOffset.y();
            m_FramePolyline[j].append(QPointF(x,y));
            v += vInc;
        }

        painter.drawPolyline(m_FramePolyline[j]);
    }

    painter.restore();
}


void SailSectionView::drawFramePoints(QPainter &painter)
{
    if(!m_pNurbs || m_pNurbs->activeFrameIndex()<0 || m_pNurbs->activeFrameIndex()>=m_pNurbs->nFrames()) return;

    Frame &frame = m_pNurbs->activeFrame();

    painter.save();

    QPen pointpen(s_SectionStyle.m_Color);
    pointpen.setCosmetic(true);
    QColor linecolor;

    for (int k=0; k<frame.nCtrlPoints();k++)
    {
        if(Frame::selectedIndex()==k)
        {
            pointpen.setWidth(4);
            linecolor = Qt::red;
        }
        else if(Frame::highlighted()==k)
        {
            pointpen.setWidth(4);
            linecolor = s_HighStyle.m_Color;
        }
        else
        {
            pointpen.setWidth(2);
            linecolor = s_SectionStyle.m_Color;
        }
        pointpen.setColor(linecolor);

        painter.setPen(pointpen);

        double x = frame.ctrlPointAt(k).x * m_fScale              + m_ptOffset.x();
        double y = frame.ctrlPointAt(k).y * (-m_fScale)*m_fScaleY + m_ptOffset.y();
        xfl::drawSymbol(painter, Line::BIGSQUARE, DisplayOptions::backgroundColor(), linecolor, x, y);
    }
    painter.restore();
}


/** draws the first and last tangents linking the control points */
void SailSectionView::drawFrameTangents(QPainter &painter)
{
    if(!m_pNurbs || m_pNurbs->activeFrameIndex()<0 || m_pNurbs->activeFrameIndex()>=m_pNurbs->nFrames()) return;
    Frame &frame = m_pNurbs->activeFrame();
    int npts = frame.nCtrlPoints();
    if(npts<2) return;

    painter.save();

    QColor tgtclr = Qt::lightGray;
    if(DisplayOptions::isLightTheme()) tgtclr = Qt::darkGray;
    QPen tgpen(tgtclr);
    tgpen.setCosmetic(true);
    tgpen.setStyle(Qt::DashDotLine);
    tgpen.setWidth(0);
    painter.setPen(tgpen);

    int x=0, y=0, x1=0, y1=0;
    x  = int(frame.ctrlPointAt(0).x * m_fScale              + m_ptOffset.x());
    y  = int(frame.ctrlPointAt(0).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());
    x1 = int(frame.ctrlPointAt(1).x * m_fScale              + m_ptOffset.x());
    y1 = int(frame.ctrlPointAt(1).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());
    painter.drawLine(x,y,x1,y1);

    x  = int(frame.ctrlPointAt(npts-1).x * m_fScale              + m_ptOffset.x());
    y  = int(frame.ctrlPointAt(npts-1).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());
    x1 = int(frame.ctrlPointAt(npts-2).x * m_fScale              + m_ptOffset.x());
    y1 = int(frame.ctrlPointAt(npts-2).y * (-m_fScale)*m_fScaleY + m_ptOffset.y());

    painter.drawLine(x,y,x1,y1);
    painter.restore();
}


void SailSectionView::mouseMoveEvent(QMouseEvent *pEvent)
{
    Section2dWt::mouseMoveEvent(pEvent);
    m_LastPoint = pEvent->pos();

    QToolTip::hideText();

    s_iSectionHighlight = isSection(m_LastPoint);
    update();
}


void SailSectionView::mousePressEvent(QMouseEvent *pEvent)
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
                //selects the section
                int iSection = isSection(point);

                if(iSection>=0)
                {
                    emit sectionSelected(iSection);
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


