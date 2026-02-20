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

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPainter>

#include "foilwt.h"

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/view2d/paint2d.h>
#include <interfaces/widgets/customdlg/selectiondlg.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/view/section2doptions.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/splinefoil.h>
#include <api/spline.h>
#include <api/geom_global.h>
#include <api/constants.h>

bool FoilWt::s_bCamberLines = false;
bool FoilWt::s_bFillBufferFoil=false;
LineStyle FoilWt::s_BufferStyle = LineStyle(true, Line::DASH, 2, fl5Color(200,200,200), Line::NOSYMBOL, "buffer_foil_style");



FoilWt::FoilWt(QWidget *pParent) : Section2dWt(pParent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

    // override section2dWt
    setMouseTracking(false);
    setFocusPolicy(Qt::NoFocus);

    m_ViewName = "Foil";

    m_oaFoil.clear();
    m_oaSpline.clear();

    m_pSF = nullptr;

    m_bShowHinges = false;
    m_LECircleRadius = 0.0;

    m_bIn01Interval = false;
    m_bFrozenSpline = false;

    m_pBufferFoil = nullptr;

    m_Grid.showXMinGrid(true);
    m_Grid.showYMinGrid(0, true);

    createBaseActions();
    createContextMenu();
}


void FoilWt::createContextMenu()
{
    m_pSection2dContextMenu = new QMenu(this);

    m_pBufferLineMenu = new LineMenu(m_pSection2dContextMenu);
    m_pBufferLineMenu->setParentMenu(m_pSection2dContextMenu);
    m_pBufferLineMenu->setTitle(tr("Style"));

    m_pOverlayFoil = new QAction(tr("Overlay foil"), this);
    m_pShowCamberLines = new QAction(tr("Show camber lines"), this);
    m_pShowCamberLines->setCheckable(true);
    m_pShowCamberLines->setChecked(s_bCamberLines);

    m_pBufferMenu = m_pSection2dContextMenu->addMenu(tr("Buffer foil"));
    {
        m_pShowBufferFoil = new QAction(tr("Show"), this);
        m_pShowBufferFoil->setCheckable(true);
        m_pFillBufferFoil = new QAction(tr("Fill"), this);
        m_pFillBufferFoil->setCheckable(true);
        m_pBufferMenu->addAction(m_pShowBufferFoil);
        m_pBufferMenu->addAction(m_pFillBufferFoil);
        m_pBufferMenu->addMenu(m_pBufferLineMenu);
    }
    m_pSection2dContextMenu->addSeparator();
    m_pSection2dContextMenu->addAction(m_pOverlayFoil);
    m_pSection2dContextMenu->addAction(m_pShowCamberLines);
    m_pSection2dContextMenu->addSeparator();

    connect(m_pShowBufferFoil,   SIGNAL(triggered()),     SLOT(onBufferShow()));
    connect(m_pFillBufferFoil,   SIGNAL(triggered()),     SLOT(onBufferShow()));
    connect(m_pOverlayFoil,      SIGNAL(triggered()),     SLOT(onOverlayFoil()));
    connect(m_pShowCamberLines,  SIGNAL(triggered(bool)), SLOT(onShowCamberLines(bool)));

    for(int iAc=0; iAc<m_ActionList.count(); iAc++)
        m_pSection2dContextMenu->addAction(m_ActionList.at(iAc));

    m_pSection2dContextMenu->addSeparator();
    QMenu *pImageMenu = m_pSection2dContextMenu->addMenu(tr("Background image"));
    {
        pImageMenu->addAction(m_pLoadImage);
        pImageMenu->addAction(m_pClearImage);
        pImageMenu->addAction(m_pImageSettings);
    }
    m_pSection2dContextMenu->addSeparator();
    m_pSection2dContextMenu->addAction(m_pExportToSVG);
}


void FoilWt::onOverlayFoil()
{
    SelectionDlg dlg(this);
    QStringList foillist;

    for(int i=0; i<Objects2d::nFoils(); i++)
    {
        foillist.append(QString::fromStdString(Objects2d::foil(i)->name()));
    }

    QStringList selectedlist;
    for(int i=0; i<m_oaFoil.size(); i++)
        selectedlist.append(QString::fromStdString(m_oaFoil.at(i)->name()));

    dlg.initDialog("Select the foils to overlay:", foillist, selectedlist, false);
    if(dlg.exec()==QDialog::Accepted)
    {
        for(int i=0; i<Objects2d::nFoils(); i++)
        {
            Foil *pFoil = Objects2d::foil(i);
            if(dlg.selectedList().contains(QString::fromStdString(pFoil->name())))
            {
                // add to list if not yet present
                bool bFound = false;
                for(int j=0; j<m_oaFoil.size(); j++)
                {
                    Foil const *pOldFoil = m_oaFoil.at(j);
                    if(pOldFoil->name()==pFoil->name())
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound)
                {
                    pFoil->setVisible(true);
                    m_oaFoil.append(pFoil);
                }
            }
            else
            {
                // remove from list if present
                for(int j=0; j<m_oaFoil.size(); j++)
                {
                    Foil const *pOldFoil = m_oaFoil.at(j);
                    if(pOldFoil->name()==pFoil->name())
                    {
                        m_oaFoil.removeAt(j);
                        break;
                    }
                }
            }
        }
    }
    update();
}


void FoilWt::onShowCamberLines(bool bShow)
{
    s_bCamberLines = bShow;
    update();
}


void FoilWt::contextMenuEvent(QContextMenuEvent *pEvent)
{
    m_pShowCamberLines->setChecked(s_bCamberLines);

    if(!m_pSection2dContextMenu) return;

    if(!m_pBufferFoil)
    {
        LineStyle ls;
        m_pBufferLineMenu->initMenu(ls);
/*        m_pShowBufferFoil->setEnabled(false);
        m_pFillBufferFoil->setEnabled(false);
        m_pBufferLineMenu->setEnabled(false);*/
        m_pBufferMenu->setEnabled(false);
    }
    else
    {
        m_pBufferMenu->setEnabled(true);
        m_pShowBufferFoil->setChecked(m_pBufferFoil->isVisible());
        m_pShowBufferFoil->setEnabled(true);
        m_pFillBufferFoil->setEnabled(true);
        m_pBufferLineMenu->initMenu(m_pBufferFoil->theStyle());
    }

    m_pFillBufferFoil->setChecked(s_bFillBufferFoil);

    m_pSection2dContextMenu->exec(pEvent->globalPos());

    if(m_pBufferFoil)
    {
        s_BufferStyle = m_pBufferLineMenu->theStyle();
        m_pBufferFoil->setTheStyle(s_BufferStyle);
        m_pBufferFoil->setVisible(m_pShowBufferFoil->isChecked());
    }
    update();
    pEvent->accept();

}


void FoilWt::onBufferShow()
{
    if(!m_pBufferFoil) return; // you never know

    bool bVisible = m_pShowBufferFoil->isChecked();
    s_bFillBufferFoil = m_pFillBufferFoil->isChecked();
    m_pBufferFoil->setVisible(bVisible);
    m_pBufferFoil->setFilled(FoilWt::isFilledBufferFoil());
    update();
}


void FoilWt::showEvent(QShowEvent *pEvent)
{
    Section2dWt::showEvent(pEvent);
    setAutoUnits();
}


void FoilWt::resizeEvent(QResizeEvent *)
{
    Section2dWt::resizeEvent(nullptr);
    resetDefaultScale();

    int w = width() - DisplayOptions::textFontStruct().averageCharWidth()*30;
    int h = DisplayOptions::textFontStruct().height();
    m_LegendPos = QPointF(w,h);
}


void FoilWt::paint(QPainter &painter)
{
    drawBackImage(painter);

    drawGrids(painter);

    paintFoils(painter);
    paintSplines(painter);
    if(m_pSF && m_pSF->isVisible()) paintSplineFoil(painter);
    if(m_LECircleRadius>PRECISION) paintLECircle(painter);
    updateScaleLabel();

    if(m_bShowLegend) paintFoilLegend(painter);
}


void FoilWt::paintFoilLegend(QPainter &painter)
{
    painter.save();

    QString strong;

    int LegendSize = 30;

    int fmh = DisplayOptions::textFontStruct().height();
    fmh *= 11/10;

    painter.setFont(DisplayOptions::textFontStruct().font());

    QColor linecolor;
    QPen TextPen(DisplayOptions::textColor());
    QPen LegendPen(Qt::gray);
    TextPen.setCosmetic(true);
    LegendPen.setCosmetic(true);
    QBrush LegendBrush(DisplayOptions::backgroundColor());
    painter.setBrush(LegendBrush);

    int npos = 0;
    for (int nc=0; nc<m_oaFoil.size(); nc++)
    {
        Foil const*pCurve = m_oaFoil.at(nc);
        if(pCurve->isVisible())
        {
            strong = QString::fromStdString(pCurve->name());


            LegendPen.setStyle(xfl::getStyle(pCurve->lineStipple()));

/*            if(pFoil->isSelected())
            {
                if(s_bIsLightTheme)  linecolor = pCurve->color().darker();
                else                 linecolor = pCurve->color().lighter();
                LegendPen.setWidth(pCurve->width()+3);
            }
            else*/
            {
                linecolor = xfl::fromfl5Clr(pCurve->lineColor());
                LegendPen.setWidth(pCurve->lineWidth());
            }
            LegendPen.setColor(linecolor);
            painter.setPen(LegendPen);

            painter.drawLine(int(m_LegendPos.x()),              int(m_LegendPos.y() + fmh*npos + fmh/3),
                             int(m_LegendPos.x() + LegendSize), int(m_LegendPos.y() + fmh*npos + fmh/3));

            if(pCurve->pointStyle())
            {
                int x1 = int(m_LegendPos.x() + 0.5*LegendSize);
                int y1 = int(m_LegendPos.y() + 1.*fmh*npos+ fmh/3);

                LegendPen.setStyle(Qt::SolidLine);
                painter.setPen(LegendPen);
                xfl::drawSymbol(painter, pCurve->pointStyle(), DisplayOptions::backgroundColor(), linecolor, QPoint(x1, y1));
            }

            painter.setPen(TextPen);
            painter.drawText(int(m_LegendPos.x() + 1.5*LegendSize), int(m_LegendPos.y() + fmh*npos+fmh/2),
                             strong);

            npos++;

        }
    }

    painter.restore();
}


void FoilWt::paintSplineFoil(QPainter &painter)
{
    if(!m_pSF) return;

    painter.save();

    QPen CtrlPen(xfl::fromfl5Clr(m_pSF->color()));
    CtrlPen.setStyle(xfl::getStyle(m_pSF->stipple()));
    CtrlPen.setWidth(m_pSF->width());
    CtrlPen.setCosmetic(true);
    painter.setPen(CtrlPen);

    QBrush FillBrush(DisplayOptions::backgroundColor());
    painter.setBrush(FillBrush);

    xfl::drawSpline(&m_pSF->extrados(), painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
    xfl::drawSpline(&m_pSF->intrados(), painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);

    CtrlPen.setStyle(Qt::SolidLine);
    painter.setPen(CtrlPen);
    if(m_pSF->pointStyle()!=Line::NOSYMBOL)
    {
        xfl::drawOutputPoints(&m_pSF->extrados(), painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor(), rect());
        xfl::drawOutputPoints(&m_pSF->intrados(), painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor(), rect());
    }

    CtrlPen.setColor(Qt::lightGray);
    xfl::drawCtrlPoints(&m_pSF->extrados(), painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor());
    xfl::drawCtrlPoints(&m_pSF->intrados(), painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor());

    if(s_bCamberLines)
    {
        QPen MidPen(xfl::fromfl5Clr(m_pSF->color()));
        MidPen.setCosmetic(true);
        MidPen.setStyle(Qt::DashLine);
        MidPen.setWidth(1);
        painter.setPen(MidPen);

        double scaley = m_fScale*m_fScaleY;

        QPolygonF poly;

        for (uint k=0; k<m_pSF->midPoints().size(); k++)
        {
            poly.append({m_pSF->midPoints().at(k).x*m_fScale + m_ptOffset.x(), -m_pSF->midPoints().at(k).y*scaley + m_ptOffset.y()});
        }
        painter.drawPolyline(poly);
     }

    painter.restore();
}


void FoilWt::paintSpline(QPainter &painter, Spline const *pSpline)
{
    if(!pSpline) return;
    painter.save();

    QPen ctrlPen(xfl::fromfl5Clr(pSpline->color()));
    ctrlPen.setCosmetic(true);
    ctrlPen.setStyle(xfl::getStyle(pSpline->stipple()));
    ctrlPen.setWidth(pSpline->width());
    painter.setPen(ctrlPen);

    QBrush FillBrush(DisplayOptions::backgroundColor());
    painter.setBrush(FillBrush);

    xfl::drawSpline(pSpline, painter, m_fScale, m_fScale*m_fScaleY, m_ptOffset);

    ctrlPen.setStyle(Qt::SolidLine);
    painter.setPen(ctrlPen);
    if(pSpline->pointStyle()!=Line::NOSYMBOL)
        xfl::drawOutputPoints(pSpline, painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor(), rect());

    ctrlPen.setColor(Qt::lightGray);
    xfl::drawCtrlPoints(pSpline, painter, m_fScale,m_fScale*m_fScaleY, m_ptOffset, DisplayOptions::backgroundColor());

    if(pSpline->bShowNormals())
    {
        ctrlPen.setStyle(Qt::DashDotDotLine);
        ctrlPen.setWidth(1);
        painter.setPen(ctrlPen);
        ctrlPen.setColor(xfl::fromfl5Clr(pSpline->color()));
        xfl::drawNormals(pSpline, painter, m_fScale, m_fScaleY, m_ptOffset);
    }

    painter.restore();
}


void FoilWt::paintFoils(QPainter &painter)
{
     for(int i=0; i<m_oaFoil.size(); i++)
    {
        if(m_oaFoil.at(i)!=m_pBufferFoil)
        {
            if(m_oaFoil.at(i)->isVisible())
                paintFoil(painter, m_oaFoil.at(i));
        }
    }
    // paint only if visible
     if(m_pBufferFoil && m_pBufferFoil->isVisible())
     {
         paintFoil(painter, m_pBufferFoil);
         if(m_bShowHinges)
             drawHinges(painter, m_pBufferFoil);
     }
}


void FoilWt::paintSplines(QPainter &painter)
{
    for(int i=0; i<m_oaSpline.size(); i++)
    {
        Spline const *pSpl = m_oaSpline.at(i);
        if(pSpl->isVisible())
            paintSpline(painter, pSpl);
    }
}


void FoilWt::paintFoil(QPainter &painter, Foil const *pFoil)
{
    painter.save();

    QPen FoilPen;
    FoilPen.setCosmetic(true);
    QBrush FillBrush(DisplayOptions::backgroundColor());
    painter.setBrush(FillBrush);

    FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
    FoilPen.setWidth(pFoil->lineWidth());
    FoilPen.setStyle(xfl::getStyle(pFoil->lineStipple()));
    painter.setPen(FoilPen);
    QColor clr = xfl::fromfl5Clr(pFoil->lineColor());
    clr.setAlpha(75);
    xfl::drawFoil(painter, pFoil, 0.0, 0.0, m_fScale, m_fScale*m_fScaleY,m_ptOffset, pFoil->isFilled(), clr);
    xfl::drawFoilPoints(painter,  pFoil, 0.0, m_fScale, m_fScale*m_fScaleY, m_ptOffset,DisplayOptions::backgroundColor(), rect());

    if (s_bCamberLines)
    {
        FoilPen.setStyle(Qt::DashLine);
        painter.setPen(FoilPen);
        xfl::drawFoilMidLine(painter, pFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
    }
    painter.restore();
}


int FoilWt::highlightPoint(const QPointF &real)
{
    if(m_bFrozenSpline) return-1;
    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    for(int is=0; is<m_oaSpline.size(); is++)
    {
        Spline *m_pSpline = nullptr;
        if(m_oaSpline.size()) m_pSpline = m_oaSpline.at(is);

        if(m_pSpline && m_pSpline->isVisible())
        {
            int n = m_pSpline->isCtrlPoint(real.x(), real.y(), dtolx, dtoly);

            if (n>=0 && n<int(m_pSpline->ctrlPointCount()))
            {
                m_pSpline->setHighlighted(n);
                return n;
            }
            else
            {
                if(m_pSpline->highlightedPoint()>=0)
                {
                    m_pSpline->setHighlighted(-10);
                }
            }
        }
    }


    if(m_pSF && m_pSF->isVisible())
    {
        for(int is=0; is<2; is++)
        {
            int n = m_pSF->spline(is)->isCtrlPoint(real.x(), real.y(), dtolx, dtoly);

            if (n>=0 && n<int(m_pSF->spline(is)->ctrlPointCount()))
            {
                m_pSF->spline(is)->setHighlighted(n);
                return n;
            }
            else
            {
                if(m_pSF->spline(is)->highlightedPoint()>=0)
                {
                    m_pSF->spline(is)->setHighlighted(-10);
                }
            }
        }
    }
    return  -1;
}


int FoilWt::selectPoint(const QPointF &real)
{
    if(m_bFrozenSpline) return-1;

    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    for(int is=0; is<m_oaSpline.size(); is++)
    {
        Spline *m_pSpline = nullptr;
        if(m_oaSpline.size()) m_pSpline = m_oaSpline.at(is);

        if(m_pSpline && m_pSpline->isVisible() && m_pSpline->bShowCtrlPts())
        {
            m_pSpline->setSelectedPoint(m_pSpline->isCtrlPoint(real.x(), real.y(), dtolx, dtoly));
            if(m_pSpline->selectedPoint()>=0) return m_pSpline->selectedPoint();
        }
    }

    if(m_pSF && m_pSF->isVisible())
    {
        for(int is=0; is<2; is++)
        {
            m_pSF->spline(is)->setSelectedPoint(m_pSF->spline(is)->isCtrlPoint(real.x(), real.y(), dtolx, dtoly));
            if(m_pSF->spline(is)->selectedPoint()>=0) return m_pSF->spline(is)->selectedPoint();
        }
    }

    return  -1;
}


void FoilWt::dragSelectedPoint(double , double )
{
    if(m_bFrozenSpline) return;

    for(int is=0; is<m_oaSpline.size(); is++)
    {
        Spline *m_pSpline = nullptr;
        if(m_oaSpline.size()) m_pSpline = m_oaSpline.at(is);

        if (m_pSpline && m_pSpline->isVisible())
        {
            int n = m_pSpline->selectedPoint();
            if (n>=0 && n<int(m_pSpline->ctrlPointCount()))
            {
                double x = m_CursorPos.x();
                if(m_bIn01Interval)
                {
                    x = std::max(0.0, x);
                    x = std::min(1.0, x);
                }
                m_pSpline->setCtrlPoint(n, x, m_CursorPos.y());

                if(n==0 && m_pSpline->issymmetric())
                {
                    if(m_pSpline->firstCtrlPoint().y<0)
                    {
                        m_pSpline->setFirstCtrlPoint({m_pSpline->firstCtrlPoint().x, 0});
                    }
                }
                if(n==m_pSpline->ctrlPointCount()-1 && m_pSpline->issymmetric())
                {
                    m_pSpline->setLastCtrlPoint({m_pSpline->lastCtrlPoint().x, 0});
                }
                if(n==0 && m_pSpline->isClosed())
                {
                    m_pSpline->setLastCtrlPoint(m_pSpline->firstCtrlPoint());
                }
                m_pSpline->updateSpline();
                m_pSpline->makeCurve();
                break;
            }
        }
    }

    if(m_pSF && m_pSF->isVisible())
    {
        for(int is=0; is<2; is++)
        {
            int n = m_pSF->spline(is)->selectedPoint();
            if (n>=0 && n<int(m_pSF->spline(is)->ctrlPointCount()))
            {
                m_pSF->spline(is)->setCtrlPoint(n, m_CursorPos.x(), m_CursorPos.y());

                if(n==0 && m_pSF->spline(is)->issymmetric())
                {
                    if(m_pSF->spline(is)->firstCtrlPoint().y<0)
                    {
                        m_pSF->spline(is)->setFirstCtrlPoint({m_pSF->spline(is)->firstCtrlPoint().x, 0});
                    }
                }
                if(n==m_pSF->spline(is)->ctrlPointCount()-1 && m_pSF->spline(is)->issymmetric())
                {
                    m_pSF->spline(is)->setLastCtrlPoint({m_pSF->spline(is)->lastCtrlPoint().x, 0});
                }
                if(n==0 && m_pSF->spline(is)->isClosed())
                {
                    m_pSF->spline(is)->setLastCtrlPoint(m_pSF->spline(is)->firstCtrlPoint());
                }
//                m_pSF->spline(is)->updateSpline();
//                m_pSF->spline(is)->makeCurve();

                if(is==0)
                {
                    if(m_pSF->bClosedTE())
                    {
                        if(n==m_pSF->extrados().ctrlPointCount()-1)
                        {
                            m_pSF->extrados().setLastCtrlPoint(m_pSF->intrados().lastCtrlPoint());
//                            m_pSF->extrados().makeCurve();
                        }
                    }
                    if(m_pSF->bClosedLE())
                    {
                        if(n==0)
                        {
                            m_pSF->extrados().setFirstCtrlPoint(m_pSF->intrados().firstCtrlPoint());
//                            m_pSF->extrados().makeCurve();
                        }
                    }
                }

                if(m_pSF->isSymmetric())
                {
                    if(m_pSF->spline(1-is)->ctrlPointCount() != m_pSF->spline(is)->ctrlPointCount())
                    {
                        m_pSF->spline(1-is)->resizeControlPoints(m_pSF->spline(is)->ctrlPointCount());
                    }

                    for(int ic=0; ic<m_pSF->spline(is)->ctrlPointCount(); ic++)
                    {
                        m_pSF->spline(1-is)->setCtrlPoint(n, m_pSF->spline(is)->controlPoint(n).x, -m_pSF->spline(is)->controlPoint(n).y);
                    }
//                    m_pSF->spline(1-is)->makeCurve();
                }

                m_pSF->makeSplineFoil();
                break;
            }
        }
    }
}


void FoilWt::onInsertPt()
{
    if(m_bFrozenSpline) return;

    QPointF ptf = mousetoReal(m_PointDown);
    Vector2d Real(ptf.x(), ptf.y());

    for(int is=0; is<m_oaSpline.size(); is++)
    {
        Spline *m_pSpline = nullptr;
        if(m_oaSpline.size()) m_pSpline = m_oaSpline.at(is);

        if(m_pSpline && m_pSpline->isVisible())
        {
            m_pSpline->insertCtrlPoint(Real.x,Real.y);
            m_pSpline->updateSpline();
            m_pSpline->makeCurve();
            emit objectModified();
            return;
        }
    }

    if(m_pSF && m_pSF->isVisible())
    {
        if(Real.y>0)
        {
            m_pSF->extrados().insertCtrlPoint(Real.x,Real.y);

            if(m_pSF->isSymmetric())
            {
                m_pSF->intrados().copySymmetric(m_pSF->extrados());
            }
            emit objectModified();
        }
        else
        {
            m_pSF->intrados().insertCtrlPoint(Real.x,Real.y);

            if(m_pSF->isSymmetric())
            {
                m_pSF->extrados().copySymmetric(m_pSF->intrados());

            }
            emit objectModified();
        }
        m_pSF->makeSplineFoil();
    }
}


void FoilWt::onRemovePt()
{
    if(m_bFrozenSpline) return;

    double dtolx = double(s_nPixelSelection)/m_fScale;
    double dtoly = double(s_nPixelSelection)/m_fScale/m_fScaleY;

    QPointF ptf = mousetoReal(m_PointDown);

    for(int is=0; is<m_oaSpline.size(); is++)
    {
        Spline *m_pSpline = nullptr;
        if(m_oaSpline.size()) m_pSpline = m_oaSpline.at(is);

        if(m_pSpline && m_pSpline->isVisible())
        {
            int n =  m_pSpline->isCtrlPoint(ptf.x(), ptf.y(), dtolx, dtoly);
            if (n>=0)
            {
                if(!m_pSpline->removeCtrlPoint(n))
                {
                    return;
                }
                m_pSpline->updateSpline();
                m_pSpline->makeCurve();
                emit objectModified();
                return;
            }
        }
    }

    if(m_pSF && m_pSF->isVisible())
    {
        for(int is=0; is<2; is++)
        {
            int n =  m_pSF->spline(is)->isCtrlPoint(ptf.x(), ptf.y(), dtolx, dtoly);
            if (n>=0)
            {
                m_pSF->spline(is)->removeCtrlPoint(n);
                if(m_pSF->isSymmetric())
                {
                    m_pSF->spline(1-is)->copySymmetric(*m_pSF->spline(is));
                }
                m_pSF->makeSplineFoil();
                emit objectModified();

                return;
            }
        }
    }
}


void FoilWt::paintLECircle(QPainter &painter)
{
    int rx = int(m_LECircleRadius * m_fScale);
    int ry = int(m_LECircleRadius * m_fScale * m_fScaleY);
    QRect rc(int(m_ptOffset.x()), int(m_ptOffset.y()) - ry,  2*rx, 2*ry);

    QPen CirclePen(QColor(128,128,128));
    CirclePen.setCosmetic(true);
    CirclePen.setStyle(Qt::DashLine);
    painter.setPen(CirclePen);
    painter.drawEllipse(rc);
}


void FoilWt::drawHinges(QPainter &painter, Foil const*pFoil)
{
    if(pFoil->hasLEFlap())
    {
        double xa = pFoil->LEHinge().x;
        double ya = pFoil->LEHinge().y;
        QPoint pt(int(xa*m_fScale + m_ptOffset.x()), int(-ya*m_fScale*m_fScaleY + m_ptOffset.y()));
        xfl::drawSymbol(painter, Line::BIGCIRCLE_F, Qt::cyan, Qt::transparent, pt);
        painter.drawText(pt.x()-DisplayOptions::textFontStruct().width("T.E. hinge")-5, pt.y()+DisplayOptions::textFontStruct().height(), "L.E. hinge");
    }

    if(pFoil->hasTEFlap())
    {
        double xa = pFoil->TEHinge().x;
        double ya = pFoil->TEHinge().y;
        QPoint pt(int(xa*m_fScale + m_ptOffset.x()), int(-ya*m_fScale*m_fScaleY + m_ptOffset.y()));
        xfl::drawSymbol(painter, Line::BIGCIRCLE_F, Qt::cyan, Qt::transparent, pt);
        painter.drawText(pt.x()-DisplayOptions::textFontStruct().width("T.E. hinge")-5, pt.y()+DisplayOptions::textFontStruct().height(), "T.E. hinge");
    }
}



