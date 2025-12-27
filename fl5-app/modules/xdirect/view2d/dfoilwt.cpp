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
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>

#include <QToolTip>
#include <QFileDialog>

#include "dfoilwt.h"

#include <modules/xdirect/view2d/dfoillegendwt.h>
#include <modules/xdirect/xdirect.h>
#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>
#include <api/geom_global.h>
#include <api/constants.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <interfaces/widgets/line/legendbtn.h>

XDirect *DFoilWt::s_pXDirect(nullptr);
bool DFoilWt::s_bFillSelected(true);
bool DFoilWt::s_bShowTEHinge(false);
bool DFoilWt::s_bShowLE(false);
bool DFoilWt::s_bLECircle(false);
double DFoilWt::s_LERad = 0.025;

Foil* DFoilWt::s_pHighFoil(nullptr);

DFoilWt::DFoilWt(QWidget *pParent) : Section2dWt(pParent)
{
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    m_ViewName = "Foils";

    m_pFoilLegendWt = new DFoilLegendWt(this);
    m_bResetLegend  = true;
    m_bShowLegend   = true;

    m_Grid.showXMinGrid(true);
    m_Grid.showYMinGrid(0,true);

    m_pHoverTimer = new QTimer;
    m_pHoverTimer->setSingleShot(true);
    connect(m_pHoverTimer, SIGNAL(timeout()), SLOT(onHovered()));

    createActions();
    createBaseContextMenu();
}


DFoilWt::~DFoilWt()
{
    delete m_pHoverTimer;
}


void DFoilWt::updateView()
{
    m_bResetLegend = true;
    update();
}


void DFoilWt::selectFoil(Foil *pFoil)
{
    m_pFoilLegendWt->selectFoil(pFoil);
}



void DFoilWt::paint(QPainter &painter)
{
    if(m_bResetLegend && m_pFoilLegendWt)
    {
        makeLegend();
        if(m_bShowLegend)
        {
            int w = width();
            int wl = m_pFoilLegendWt->width();
            m_pFoilLegendWt->move(w-wl-15, 15);
        }
    }

    drawBackImage(painter);
    drawGrids(painter);
    paintFoils(painter);

    if(s_bLECircle) paintLECircle(painter);

    updateScaleLabel();

    if(m_DebugPts.size()>0) drawDebugPts(painter);
}


void DFoilWt::paintLECircle(QPainter &painter)
{
    float rx = s_LERad * m_fScale;
    float ry = s_LERad * m_fScale * m_fScaleY;
    QRectF rc(m_ptOffset.x(), m_ptOffset.y() - ry,  2*rx, 2*ry);

    QPen CirclePen(QColor(128,128,128));
    CirclePen.setStyle(Qt::DashLine);
    painter.setPen(CirclePen);
    painter.drawEllipse(rc);
}


void DFoilWt::paintFoils(QPainter &painter)
{
    painter.save();
    QPen FoilPen, CenterPen;
    FoilPen.setCosmetic(true);
    CenterPen.setCosmetic(true);
    QBrush FillBrush(DisplayOptions::backgroundColor());
    painter.setBrush(FillBrush);

    // first draw the selected foil
    // so that it may be filled in the background
    if(XDirect::curFoil() && XDirect::curFoil()->isVisible())
    {
        Foil const *pFoil = XDirect::curFoil();
        FoilPen.setStyle(xfl::getStyle(pFoil->lineStipple()));

        if(pFoil==XDirect::curFoil())
        {
            FoilPen.setWidth(pFoil->lineWidth());
            if(pFoil==s_pHighFoil)
            {
                FoilPen.setWidth(s_HighStyle.m_Width);
            }
            if(DisplayOptions::isLightTheme()) FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()).darker());
            else                               FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()).lighter());
        }
        else
        {
            FoilPen.setWidth(pFoil->lineWidth());
            FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
        }

        painter.setPen(FoilPen);

        QColor clr = xfl::fromfl5Clr(pFoil->lineColor());
        clr.setAlpha(75);
        xfl::drawFoil(painter, pFoil, 0.0, 0.0, m_fScale, m_fScale*m_fScaleY, m_ptOffset, s_bFillSelected, clr);

        if(pFoil->isCamberLineVisible())
        {
            if(pFoil==XDirect::curFoil())
            {
                CenterPen.setWidth(pFoil->lineWidth()+3);
                if(DisplayOptions::isLightTheme()) CenterPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()).darker());
                else                               CenterPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()).lighter());
            }
            else
            {
                CenterPen.setWidth(pFoil->lineWidth());
                CenterPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
            }
            CenterPen.setStyle(Qt::DashLine);
            painter.setPen(CenterPen);
            xfl::drawFoilMidLine(painter, pFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
        }

        xfl::drawFoilPoints(painter, pFoil, 0.0, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor(), rect());

        if(s_bShowLE && pFoil==XDirect::curFoil())
        {
            double xa = (pFoil->LE().x-0.5)  + 0.5;
            double ya =  pFoil->LE().y;
            QPoint pt(int(xa*m_fScale + m_ptOffset.x()), int(-ya*m_fScale*m_fScaleY + m_ptOffset.y()));
            xfl::drawSymbol(painter, Line::BIGCROSS, Qt::yellow, Qt::yellow, pt);
            painter.drawText(pt.x()-DisplayOptions::textFontStruct().width("L.E.")-5, pt.y()+DisplayOptions::textFontStruct().height(), "L.E.");
        }

        if(s_bShowTEHinge && pFoil==XDirect::curFoil() && pFoil->hasTEFlap())
        {
            double xa = (pFoil->TEHinge().x-0.5)  + 0.5;
            double ya =  pFoil->TEHinge().y;
            QPoint pt(int(xa*m_fScale + m_ptOffset.x()), int(-ya*m_fScale*m_fScaleY + m_ptOffset.y()));
            xfl::drawSymbol(painter, Line::BIGCIRCLE_F, Qt::cyan, Qt::transparent, pt);
            painter.drawText(pt.x()-DisplayOptions::textFontStruct().width("T.E. hinge")-5, pt.y()+DisplayOptions::textFontStruct().height(), "T.E. hinge");
        }
    }

    // overlay all other foils
    for (int k=0; k<Objects2d::nFoils(); k++)
    {
        Foil const *pFoil = Objects2d::foil(k);
        if (pFoil !=XDirect::curFoil() && pFoil->isVisible())
        {
            if(pFoil==s_pHighFoil)
            {
                FoilPen.setWidth(s_HighStyle.m_Width);
//                FoilPen.setStyle(s_HighStyle.getStipple());
//                FoilPen.setColor(s_HighStyle.m_Color);
                FoilPen.setStyle(xfl::getStyle(pFoil->lineStipple()));
                FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
            }
            else
            {
                FoilPen.setWidth(pFoil->lineWidth());
                FoilPen.setStyle(xfl::getStyle(pFoil->lineStipple()));
                FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
            }

            painter.setPen(FoilPen);

            xfl::drawFoil(painter, pFoil, 0.0, 0.0, m_fScale, m_fScale*m_fScaleY,m_ptOffset);

            if(pFoil->isCamberLineVisible())
            {
                CenterPen.setWidth(pFoil->lineWidth());
                CenterPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
                CenterPen.setStyle(Qt::DashLine);
                painter.setPen(CenterPen);
                xfl::drawFoilMidLine(painter, pFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
            }

            xfl::drawFoilPoints(painter, pFoil, 0.0, m_fScale,m_fScale*m_fScaleY, m_ptOffset,
                           DisplayOptions::backgroundColor(), rect());

            if(s_bShowLE && pFoil==XDirect::curFoil())
            {
                //draw LE;
                double xa = (pFoil->LE().x-0.5)  + 0.5;
                double ya =  pFoil->LE().y;
                QPoint pt(int(xa*m_fScale + m_ptOffset.x()), int(-ya*m_fScale*m_fScaleY + m_ptOffset.y()));
                xfl::drawSymbol(painter, Line::BIGCROSS, Qt::yellow, Qt::yellow, pt);
                painter.drawText(pt.x()-DisplayOptions::textFontStruct().width("L.E.")-5, pt.y()+DisplayOptions::textFontStruct().height(), "L.E.");
            }
        }
    }
    painter.restore();
}


void DFoilWt::showEvent(QShowEvent *pEvent)
{
    Section2dWt::showEvent(pEvent);
    resizeEvent(nullptr);
    resetDefaultScale();
    setAutoUnits();
    update();
}


void DFoilWt::resizeEvent(QResizeEvent *pEvent)
{
    Section2dWt::resizeEvent(pEvent);
    if(m_bShowLegend)
    {
        int w = width();
        int wl = m_pFoilLegendWt->width();
        m_pFoilLegendWt->move(w-wl-15, 5);
    }

    resetDefaultScale();

}


void DFoilWt::showLegend(bool bShow)
{
    m_bShowLegend = bShow;
    m_pFoilLegendWt->setVisible(bShow);
}


void DFoilWt::onHovered()
{
    //selects the foil
    Foil *pFoil = isFoil(mousetoReal(m_LastPoint));
    if(pFoil)
    {
        QToolTip::showText(QCursor::pos(), QString::fromStdString(pFoil->name()), this, QRect(), 15000);
    }
}


void DFoilWt::onFillFoil(bool bFill)
{
    s_bFillSelected=bFill;
    updateView();
}


void DFoilWt::createActions()
{
    createBaseActions();
}


void DFoilWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    Section2dWt::mouseMoveEvent(pEvent);
    m_LastPoint = pEvent->pos();
    m_pHoverTimer->start(150);
    QToolTip::hideText();

    //highlight the frame
    if(!m_DynTimer.isActive())
    {
        QPoint point = pEvent->pos();
        s_pHighFoil = isFoil(mousetoReal(point));
        update();
    }
}


void DFoilWt::mousePressEvent(QMouseEvent *pEvent)
{
    stopDynamicTimer();

    QPoint point = pEvent->pos();

    m_LastPoint = pEvent->pos();

    Foil *pFoil = nullptr;

    // get a reference for mouse movements
    m_PointDown.rx() = point.x();
    m_PointDown.ry() = point.y();

    if(m_bZoomPlus)
    {
        m_ZoomRect.setTopLeft(point);
        m_ZoomRect.setBottomRight(point);
    }
    else if(!m_bZoomPlus && (pEvent->buttons() & Qt::LeftButton))
    {
        pFoil = isFoil(mousetoReal(point));
        if(pFoil)
        {
            emit foilSelected(pFoil);
            m_pFoilLegendWt->selectFoil(pFoil);
        }
        else
        {
            //dragging the view
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            m_bTrans = true;
        }
    }

    m_MoveTime.restart();

//    Section2dView::mousePressEvent(pEvent);
}


void DFoilWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    QPoint point = pEvent->pos();

    if(m_bTrans && (m_PointDown-point).manhattanLength()>5)
    {
        double dist = sqrt((point.x()-m_PointDown.x())*(point.x()-m_PointDown.x())+(point.y()-m_PointDown.y())*(point.y()-m_PointDown.y()));
        if(dist<5) //pixels
        {
            //check foil deselection
            Foil *pFoil = isFoil(mousetoReal(pEvent->pos()));
            if(!pFoil)
            {
                emit foilSelected(pFoil);
                m_pFoilLegendWt->selectFoil(pFoil);
            }
        }
    }
    else
    {
        //selects the foil
        Foil *pFoil = isFoil(mousetoReal(point));
        if(!pFoil)
        {
            s_pXDirect->setCurFoil(nullptr);
            m_pFoilLegendWt->selectFoil(nullptr);
            s_pXDirect->setControls();
            emit foilSelected(nullptr);
            update();
        }
    }
    Section2dWt::mouseReleaseEvent(pEvent);
}


void DFoilWt::makeLegend()
{
    if(m_pFoilLegendWt)
        m_pFoilLegendWt->makeDFoilLegend();

    m_bResetLegend = false;
}


Foil* DFoilWt::isFoil(QPointF pos)
{
    double pixelDist = 5.0;

    for(int i=0; i<Objects2d::nFoils(); i++)
    {
        Foil *pOldFoil = Objects2d::foil(i);
        if(pOldFoil->nNodes()==0 || !pOldFoil->isVisible())
        {
        }
        else
        {
            for(int ip=0; ip<pOldFoil->nNodes()-1; ip++)
            {
                double x1 = pOldFoil->x(ip);
                double x2 = pOldFoil->x(ip+1);
                double y1 = pOldFoil->y(ip);
                double y2 = pOldFoil->y(ip+1);
                double d2  = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                if(d2>PRECISION)
                {
                    double dotproduct = (pos.x()-x1) * (pos.x()-x2) + (pos.y()-y1)*(pos.y()-y2);
                    double crossZ = (pos.x()-x1)*(pos.y()-y2) - (pos.y()-y1)* (pos.x()-x2);

                    if(dotproduct<=0.0 && fabs(crossZ)/d2<pixelDist/m_fScale)
                    {
                        return pOldFoil;
                    }
                }
                else if(sqrt((pos.x()-x1)*(pos.x()-x1)+(pos.y()-y1)*(pos.y()-y1))<pixelDist/m_fScale)
                {
                    return pOldFoil;
                }
            }
        }
    }
    return nullptr;
}


void DFoilWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("DFoilWt");
    {
        s_bShowLE          = settings.value("ShowLE",          s_bShowLE).toBool();
        s_bLECircle        = settings.value("ShowLECircle",    s_bLECircle).toBool();
        s_LERad            = settings.value("LE_Radius",       s_LERad).toDouble();
        s_bFillSelected    = settings.value("FillHighlighted", s_bFillSelected).toBool();
        s_bShowTEHinge     = settings.value("TEHinge",         s_bShowTEHinge).toBool();
        m_Grid.loadSettings(settings);
    }
    settings.endGroup();
}


void DFoilWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("DFoilWt");
    {
        settings.setValue("ShowLE",          s_bShowLE);
        settings.setValue("ShowLECircle",    s_bLECircle);
        settings.setValue("LE_Radius",       s_LERad);
        settings.setValue("FillHighlighted", s_bFillSelected);
        settings.setValue("TEHinge",         s_bShowTEHinge);
        m_Grid.saveSettings(settings);
    }
    settings.endGroup();
}



void DFoilWt::onSaveToSvg()
{
/*    QString FileName, tempfilepath;

    FileName = m_ViewName;

    FileName = QFileDialog::getSaveFileName(this, "Exporqt view to SVG",
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
    painter.setRenderHint(QPainter::Antialiasing,          s_bAntiAliasing);
    painter.setRenderHint(QPainter::TextAntialiasing,      s_bAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, s_bAntiAliasing);
    painter.fillRect(rect(), DisplayOptions::backgroundColor());
    paint(painter);

    // add the legend
    painter.translate(m_pFoilLegendWt->pos());

    painter.fillRect(m_pFoilLegendWt->rect(), DisplayOptions::backgroundColor());
    QList<LegendBtn*> list = m_pFoilLegendWt->legendBtns().keys();
    for(int i=0; i<m_pFoilLegendWt->legendBtns().keys().size(); i++)
    {
        list[i]->paintButton(painter);
        painter.translate(0, list[i]->height());
    }

    painter.end();*/
}

