/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "xmlxsailwriter.h"

#include <xflcore/units.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>
#include <xflgeom/geom2d/splines/bspline.h>

XmlXSailWriter::XmlXSailWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlXSailWriter::writeMetaData(Sail const*pSail, Vector3d const &position)
{
    double lengthunit = Units::mtoUnit();
    writeTextElement("Name", pSail->name());
    writeTheStyle(pSail->theStyle());
    writeTextElement("Description", pSail->description());
    writeComment(QString("The following field defines the sail's position relative to the boat"));
    writeTextElement("Position",QString("%1, %2, %3").arg(position.x*lengthunit, 11,'g',5)
                                                     .arg(position.y*lengthunit, 11,'g',5)
                                                     .arg(position.z*lengthunit, 11,'g',5));

    writeTextElement("Reference_area",  QString::asprintf("%g", pSail->refArea()*Units::m2toUnit()));
    writeTextElement("Reference_chord", QString::asprintf("%g", pSail->refChord()*Units::mtoUnit()));
}


void XmlXSailWriter::writeNURBSSail(NURBSSail const *pNSail, Vector3d const &position)
{
    writeStartElement("NURBSSail");
    {
        writeMetaData(pNSail, position);

        writeTextElement("x_panels", QString ("%1").arg(pNSail->nXPanels()));
        writeTextElement("z_panels", QString ("%1").arg(pNSail->nZPanels()));
        writeTextElement("x_panel_distribution", xfl::distributionType(pNSail->xDistType()));
        writeTextElement("z_panel_distribution", xfl::distributionType(pNSail->zDistType()));

        QVector<int> dummyarray(pNSail->frameCount());

        writeNURBS(pNSail->nurbs(), dummyarray, false);
    }
    writeEndElement();
}


void XmlXSailWriter::writeSplineSail(const SplineSail *pSSail, Vector3d const &position)
{
    double lengthunit = Units::mtoUnit();
    writeStartElement("SplineSail");
    {
        writeMetaData(pSSail, position);

        writeComment(QString("Must include the spline type; either BSPLINE, BEZIERSPLINE, CUBICSPLINE or POINTSPLINE"));
        switch(pSSail->splineType())
        {
            case Spline::BSPLINE: writeTextElement("Type", "BSPLINE");       break;
            case Spline::BEZIER:  writeTextElement("Type", "BEZIERSPLINE");  break;
            case Spline::CUBIC:   writeTextElement("Type", "CUBICSPLINE");   break;
            case Spline::POINT:   writeTextElement("Type", "POINTSPLINE");   break;
            default:  return;
        }

        writeComment(QString("The two following fields define the number of mesh panels in the horizontal and vertical directions"));
        writeTextElement("x_panels", QString ("%1").arg(pSSail->nXPanels()));
        writeTextElement("x_panel_distribution", xfl::distributionType(pSSail->xDistType()));

        writeTextElement("z_panels", QString ("%1").arg(pSSail->nZPanels()));
        writeTextElement("z_panel_distribution", xfl::distributionType(pSSail->zDistType()));

        for(int iSection=0; iSection<pSSail->sectionCount(); iSection++)
        {
            Spline const *pSpline = pSSail->splineAt(iSection);
            writeStartElement("Section");
            {
                writeTextElement("Position",QString("%1, %2, %3").arg(pSSail->sectionPosition(iSection).x*lengthunit, 11,'g',5)
                                                                 .arg(pSSail->sectionPosition(iSection).y*lengthunit, 11,'g',5)
                                                                 .arg(pSSail->sectionPosition(iSection).z*lengthunit, 11,'g',5));
                writeTextElement("Ry", QString ("%1").arg(pSSail->sectionAngle(iSection)));
                if(pSpline->isBSpline())
                {
                    BSpline const *pspl = dynamic_cast<const BSpline*>(pSpline);
                    writeComment(QString("degree < number of control points"));
                    writeTextElement("Degree", QString ("%1").arg(pspl->degree()));
                }
                writeComment(QString(" x, y,  weight"));
                for(int iPt=0; iPt<pSpline->ctrlPointCount(); iPt++)
                {
                    Vector2d const& Pt= pSpline->controlPoint(iPt);
                    writeTextElement("point",QString("%1, %2, %3").arg(Pt.x*lengthunit, 11,'g',5)
                                                                  .arg(Pt.y*lengthunit, 11,'g',5)
                                                                  .arg(pSpline->weight(iPt), 11,'g',5));
                }

            }
            writeEndElement();
        }
    }
    writeEndElement();
}


void XmlXSailWriter::writeWingSail(const WingSail *pWSail, Vector3d const &position)
{
    double lengthunit = Units::mtoUnit();
    writeStartElement("WingSail");
    {
        writeMetaData(pWSail, position);

        for(int iSection=0; iSection<pWSail->sectionCount(); iSection++)
        {
            WingSailSection const &sec = pWSail->sectionAt(iSection);
            writeStartElement("Section");
            {
                writeTextElement("Position", QString("%1, %2, %3").arg(pWSail->sectionPosition(iSection).x*lengthunit, 11,'g',5)
                                                                  .arg(pWSail->sectionPosition(iSection).y*lengthunit, 11,'g',5)
                                                                  .arg(pWSail->sectionPosition(iSection).z*lengthunit, 11,'g',5));
                writeTextElement("Ry", QString ("%1").arg(pWSail->sectionAngle(iSection)));

                writeTextElement("FoilName", QString("%1").arg(sec.foilName()));
                writeTextElement("Chord",    QString("%1").arg(sec.chord()));
                writeTextElement("Twist",    QString("%1").arg(sec.twist()));
                writeTextElement("NXpanels", QString("%1").arg(sec.nxPanels()));
                writeTextElement("NZpanels", QString("%1").arg(sec.nzPanels()));
                writeTextElement("x_panel_distribution", xfl::distributionType(sec.xDistType()));
                writeTextElement("z_panel_distribution", xfl::distributionType(sec.zDistType()));
            }
            writeEndElement();
        }
    }
    writeEndElement();
}


