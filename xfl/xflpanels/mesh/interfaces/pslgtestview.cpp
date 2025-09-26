/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QPainter>
#include <QKeyEvent>

#include "pslgtestview.h"
#include <xflcore/displayoptions.h>

#include <xflmath/constants.h>


PSLGTestView::PSLGTestView(QWidget *pParent) : Section2dWt (pParent)
{
    m_ViewName = "PSLG";

    m_RefLength = 1.0;

    m_bPaintPSLGIndexes = m_bPaintTriangleIndexes = false;
    m_iSelected = -1;
}


int PSLGTestView::highlightPoint(const QPointF &pt)
{
    //test first the currently selected triangle which is the most likely candidate
    if(m_iSelected>=0 && m_iSelected<m_Triangles.size())
    {
        if(m_Triangles.at(m_iSelected).isStrictlyInside(pt.x(), pt.y()))
        {
            //don't change anything
            return m_iSelected;
        }
    }

    m_iSelected = -1;
    for(int it=0; it<m_Triangles.size(); it++)
    {
        if(m_Triangles.at(it).isStrictlyInside(pt.x(), pt.y()))
        {
            m_iSelected = it;
            QString strange = QString("Triangle %1:\n").arg(it);
            strange += m_Triangles.at(it).properties(true);
            if(m_LongTriList.contains(it))
            {
                strange+= "  triangle is long\n";
            }
            setOutputInfo(strange);
            break;
        }
    }
    if(m_iSelected<0) setOutputInfo(QString());
    return m_iSelected;
}


int PSLGTestView::selectPoint(const QPointF & )
{
/*    for(int it=0; it<m_TriangleList.size(); it++)
    {
        if(m_TriangleList.at(it).isStrictlyInside(pt.x(), pt.y()))
            return it;
    }*/
    return -1;
}


void PSLGTestView::dragSelectedPoint(double , double )
{
}


void PSLGTestView::onInsertPt()
{
}


void PSLGTestView::onRemovePt()
{
}


void PSLGTestView::showEvent(QShowEvent *pEvent)
{
    Section2dWt::showEvent(pEvent);

    resetDefaultScale();
    setAutoUnits();
}


void PSLGTestView::setTriangles(QVector<QVector<Triangle2d>> const &triangles)
{
    m_Triangles.clear();
    for(int i=0; i<triangles.size(); i++) m_Triangles.append(triangles.at(i));
}


void PSLGTestView::setReferenceLength()
{
    // find the max length
    if(m_PSLG.size())
    {
        double xmin=1e10, ymin=1e10, xmax=-1e10, ymax=-1e10;
        for(int ip=0; ip<m_PSLG.size(); ip++)
        {
            for(int iv=0; iv<2; iv++)
            {
                Vector2d const& vtx = m_PSLG.at(ip).vertexAt(iv);
                xmin = std::min(xmin, vtx.x);
                xmax = std::max(xmax, vtx.x);
                ymin = std::min(ymin, vtx.y);
                ymax = std::max(ymax, vtx.y);
            }
        }
        m_RefLength = std::max(fabs(xmax),  fabs(xmin));
        m_RefLength = std::max(m_RefLength, fabs(ymax));
        m_RefLength = std::max(m_RefLength, fabs(ymin));
    }
    else
    {
//        leave unchanged
    }
}


void PSLGTestView::paint(QPainter &painter)
{
    painter.save();
    painter.fillRect(rect(), DisplayOptions::backgroundColor());
    painter.setRenderHint(QPainter::Antialiasing,          s_bAntiAliasing);
    painter.setRenderHint(QPainter::TextAntialiasing,      s_bAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, s_bAntiAliasing);

    drawGrids(painter);

    paintTriangles(painter);

    paintPoints(painter);

    QColor clr(175,75,75);
    QPen linePen(clr);
    linePen.setWidth(2);
    painter.setPen(linePen);
    QBrush diskbrush(clr.lighter(150));
    painter.setBrush(diskbrush);
    drawPSLG(painter, m_PSLG, true);

    paintIndexes(painter);
    updateScaleLabel();
    painter.restore();
}


void PSLGTestView::paintPoints(QPainter &painter)
{
    painter.save();
    QPen outlinepen(QColor(55,35,45));
    outlinepen.setWidth(1);
    painter.setPen(outlinepen);

    QBrush fillbrush(QColor(95,155,205, 235));
    painter.setBrush(fillbrush);

    for(int p=0; p<m_Points.size(); p++)
    {
        int x = int(m_Points.at(p).x * m_fScale              + m_ptOffset.x());
        int y = int(m_Points.at(p).y * (-m_fScale*m_fScaleY) + m_ptOffset.y());
        painter.drawRect(x-5,y-5,10,10);
    }
    painter.restore();
}


void PSLGTestView::paintTriangles(QPainter &painter)
{
    painter.save();
    QPen outlinepen(QColor(205,150,50));
    outlinepen.setWidth(1);
    painter.setPen(outlinepen);

    int pos = 0;
    for(int p=0; p<m_Triangles.size(); p++)
    {
        paintTriangle(painter, m_Triangles.at(p), false, false);
    }

    outlinepen.setColor(QColor(55,35,45));
    painter.setPen(outlinepen);
    pos = 0;

    QBrush fillbrush(QColor(205,205,205, 135));

    for(int p=0; p<m_Triangles.size(); p++)
    {
        Triangle2d const &t2d = m_Triangles.at(p);
        if(p==m_iSelected)
        {
            fillbrush.setColor(QColor(205,35,75, 135));
//            painter.setBrush(fillbrush);
//            paintTriangle(painter, t2d, false, true);
        }
        else if(m_SkinnyTriList.contains(p))
        {
            fillbrush.setColor(QColor(175,175,35, 135));
//            painter.setBrush(fillbrush);
//            paintTriangle(painter, t2d, false, true);
        }
        else if(m_LongTriList.contains(p))
        {
            fillbrush.setColor(QColor(55,155,193, 135));
//            painter.setBrush(fillbrush);
//            paintTriangle(painter, t2d, false, true);
        }
        else if(m_FlipList.contains(p))
        {
            fillbrush.setColor(QColor(15,35,233, 135));
//            painter.setBrush(fillbrush);
//            paintTriangle(painter, t2d, false, true);
        }
        else
        {
            fillbrush.setColor(QColor(175,175,175, 135));
        }
        painter.setBrush(fillbrush);
        paintTriangle(painter, t2d, false, true);

        if(p==m_iSelected)
        {
            painter.setBrush(Qt::NoBrush);
            paintCircumCircle(painter, t2d);
        }
        pos +=4;
    }
    painter.restore();
}


void PSLGTestView::paintIndexes(QPainter &painter)
{
    painter.save();

    QFont fnt = DisplayOptions::textFontStruct().font();
    fnt.setWeight(QFont::Bold);
    fnt.setItalic(true);
    painter.setFont(fnt);
    if(m_bPaintTriangleIndexes)
    {
        for(int it=0; it<m_Triangles.size(); it++)
        {
            QPen pen(Qt::black);
            painter.setPen(pen);
            Vector2d const &cog = m_Triangles.at(it).CoG();
            QPointF pt;
            pt.rx() = cog.x *   m_fScale            + m_ptOffset.x();
            pt.ry() = cog.y * (-m_fScale*m_fScaleY) + m_ptOffset.y();
            painter.drawText(pt, QString("%1").arg(it));
        }
    }
    if(m_bPaintPSLGIndexes)
    {
        fnt.setPointSize(fnt.pointSize()+2);
        painter.setFont(fnt);
        for(int ip=0; ip<m_PSLG.size(); ip++)
        {
            QPen pen(QColor(51,51,151));
            painter.setPen(pen);
            Vector2d cog = m_PSLG.at(ip).midPoint();
            QPointF pt;
            pt.rx() = cog.x * m_fScale              + m_ptOffset.x();
            pt.ry() = cog.y * (-m_fScale*m_fScaleY) + m_ptOffset.y();
            painter.drawText(pt, QString("%1").arg(ip));
        }
    }
    painter.restore();
}



void PSLGTestView::listNonCompliant(double length)
{
    m_EncroachedSegList.clear();
    m_SkinnyTriList.clear();
    m_LongTriList.clear();

    listEncroachedSegments();

    listSkinnyTriangles();

    listLongTriangles(length);
}


/** skinny triangles with length less than the specified size are not counted as skinny */
int PSLGTestView::listSkinnyTriangles()
{
    m_SkinnyTriList.clear();
    //list skinny triangles
    for(int it=0; it<m_Triangles.size(); it++)
    {
        Triangle2d const &t2d = m_Triangles.at(it);

        if(t2d.isSkinny()) m_SkinnyTriList.push_back(it);
    }
    return m_SkinnyTriList.size();
}


/** list long triangles */
int PSLGTestView::listLongTriangles(double length)
{
    m_LongTriList.clear();
    for(int it=0; it<m_Triangles.size(); it++)
    {
        if(m_Triangles.at(it).isLong(length))m_LongTriList.push_back(it);
    }
    return m_LongTriList.size();
}


/** Lists PSLG segments encroached by skinny triangles */
int PSLGTestView::listEncroachedSegments()
{
    m_EncroachedSegList.clear();
    Vector2d CC;
    double r=0;
    for(int ipslg = m_PSLG.size()-1; ipslg>=0; ipslg--)
    {
        Segment2d seg = m_PSLG.at(ipslg);
        for(int it=0; it<m_Triangles.size(); it++)
        {
            Triangle2d const &t2d = m_Triangles.at(it);
            if(t2d.isSkinny())
            {
                t2d.circumCenter(r, CC);
                if(seg.isEncroachedBy(CC))
                {
                    if(!m_EncroachedSegList.contains(ipslg))
                        m_EncroachedSegList.push_back(ipslg);
                    // the segment is encroached, no need to continue checking other triangles
                    break;
                }
            }
        }
    }
    std::sort(m_EncroachedSegList.begin(), m_EncroachedSegList.end());
    return m_EncroachedSegList.size();
}
