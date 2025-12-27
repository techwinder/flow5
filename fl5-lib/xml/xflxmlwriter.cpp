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


#include <QFileInfo>

#include <QDir>




#include <extradrag.h>
#include <fuseocc.h>
#include <fusestl.h>
#include <fusexfl.h>
#include <inertia.h>
#include <mathelem.h>
#include <nurbssurface.h>
#include <objects2d.h>
#include <pointmass.h>
#include <polar3d.h>
#include <units.h>
#include <utils.h>
#include <vector3d.h>
#include <wingxfl.h>
#include <xflxmlwriter.h>
#include <xml_globals.h>



XflXmlWriter::XflXmlWriter(QFile &XFile) : QXmlStreamWriter()
{
    setDevice(&XFile);
    QFileInfo fi(XFile);
    m_FileName = fi.fileName();
    m_Path = fi.absolutePath();

    setAutoFormatting(true);
}


XflXmlWriter::~XflXmlWriter() {}


void XflXmlWriter::writeTheStyle(LineStyle const &theStyle)
{
    writeStartElement("The_Style");
    {
        writeComment("Available style options are: SOLID, DASH, DOT, DASHDOT, DASHDOTDOT, NOLINE");
        switch (theStyle.m_Stipple)
        {
            case Line::SOLID:       writeTextElement("Stipple", "SOLID");        break;
            case Line::DASH:        writeTextElement("Stipple", "DASH");         break;
            case Line::DOT:         writeTextElement("Stipple", "DOT");          break;
            case Line::DASHDOT:     writeTextElement("Stipple", "DASHDOT");      break;
            case Line::DASHDOTDOT:  writeTextElement("Stipple", "DASHDOTDOT");   break;
            case Line::NOLINE:      writeTextElement("Stipple", "NOLINE");       break;
        }

        writeComment("Available symbol options are "
                     "NOSYMBOL, LITTLECIRCLE, LITTLECIRCLEFILLED, BIGCIRCLE, BIGCIRCLEFILLED, "
                     "LITTLESQUARE, LITTLESQUAREFILLED, BIGSQUARE, BIGSQUAREFILLED, "
                     "TRIANGLE, TRIANGLE_INV, TRIANGLEFILLED, TRIANGLEFILLED_INV, LITTLECROSS, BIGCROSS");
        switch (theStyle.m_Symbol)
        {
            case Line::NOSYMBOL:        writeTextElement("PointStyle", "NOSYMBOL");            break;
            case Line::LITTLECIRCLE:    writeTextElement("PointStyle", "LITTLECIRCLE");        break;
            case Line::BIGCIRCLE:       writeTextElement("PointStyle", "BIGCIRCLE");           break;
            case Line::LITTLESQUARE:    writeTextElement("PointStyle", "LITTLESQUARE");        break;
            case Line::BIGSQUARE:       writeTextElement("PointStyle", "BIGSQUARE");           break;
            case Line::TRIANGLE:        writeTextElement("PointStyle", "TRIANGLE");            break;
            case Line::TRIANGLE_INV:    writeTextElement("PointStyle", "TRIANGLE_INV");        break;
            case Line::LITTLECROSS:     writeTextElement("PointStyle", "LITTLECROSS");         break;
            case Line::BIGCROSS:        writeTextElement("PointStyle", "BIGCROSS");            break;
            case Line::LITTLECIRCLE_F:  writeTextElement("PointStyle", "LITTLECIRCLEFILLED");  break;
            case Line::BIGCIRCLE_F:     writeTextElement("PointStyle", "BIGCIRCLEFILLED");     break;
            case Line::LITTLESQUARE_F:  writeTextElement("PointStyle", "LITTLESQUAREFILLED");  break;
            case Line::BIGSQUARE_F:     writeTextElement("PointStyle", "BIGSQUAREFILLED");     break;
            case Line::TRIANGLE_F:      writeTextElement("PointStyle", "TRIANGLEFILLED");      break;
            case Line::TRIANGLE_INV_F:  writeTextElement("PointStyle", "TRIANGLEFILLED_INV");  break;
        }
        writeTextElement("Width", QString("%1").arg(theStyle.m_Width));
        writeColor(theStyle.m_Color);
    }
    writeEndElement();
}


void XflXmlWriter::writeColor(fl5Color const &color)
{
    writeStartElement("Color");
    {
        writeTextElement("red",   QString::asprintf("%d", color.red()));
        writeTextElement("green", QString::asprintf("%d", color.green()));
        writeTextElement("blue",  QString::asprintf("%d", color.blue()));
        writeTextElement("alpha", QString::asprintf("%d", color.alpha()));
    }
    writeEndElement();
}


void XflXmlWriter::writeUnits()
{
    writeStartElement("Units");
    {
        QString strange = QString("Unit conversion factors: "
                                  "The values in this file will be multiplied by the following factors to obtain SI units."
                                  "Labels changed in v7.50 for consistency, legacy ones have been kept active");
        writeComment(strange);

        writeTextElement("meter_to_length_unit", QString::asprintf("%g", 1./Units::mtoUnit()));
        writeTextElement("m2_to_area_unit",      QString::asprintf("%g", 1./Units::m2toUnit()));
        writeTextElement("kg_to_mass_unit",      QString::asprintf("%g", 1./Units::kgtoUnit()));
        writeTextElement("ms_to_speed_unit",     QString::asprintf("%g", 1./Units::mstoUnit()));
        writeTextElement("kgm2_to_inertia_unit", QString::asprintf("%g", 1./Units::kgm2toUnit()));
    }
    writeEndElement();
}


void XflXmlWriter::writePointMass(const PointMass &pm)
{
    writeStartElement("Point_Mass");
    {
        writeTextElement("Tag", QString::fromStdString(pm.tag()));
        writeTextElement("Mass", QString("%1").arg(pm.mass()*Units::kgtoUnit(),7,'f',3));
        writeTextElement("coordinates",QString("%1, %2, %3").arg(pm.position().x*Units::mtoUnit(), 11,'g',5)
                                                            .arg(pm.position().y*Units::mtoUnit(), 11,'g',5)
                                                            .arg(pm.position().z*Units::mtoUnit(), 11,'g',5));
    }
    writeEndElement();
}


void XflXmlWriter::writeXMLBody(FuseOcc const &)
{

}

void XflXmlWriter::writeXMLBody(FuseStl const &)
{

}

void XflXmlWriter::writeXMLBody(FuseXfl const &bodyxfl)
{
    writeHeader();

    Vector3d V;
    writeXflFuse(bodyxfl, V);

    writeEndDocument();
}


void XflXmlWriter::writeXflFuse(const FuseXfl &xflfuse, const Vector3d &position)
{
    NURBSSurface const &nurbs = xflfuse.nurbs();
    writeStartElement("body");
    {
        writeTextElement("Name", QString::fromStdString(xflfuse.name()));
        writeColor(xflfuse.color());

        if(xflfuse.description().length())
        {
            writeTextElement("Description", QString::fromStdString(xflfuse.description()));
        }

        writeTextElement("Position",QString("%1, %2, %3").arg(position.x*Units::mtoUnit(), 11,'g',5)
                                                         .arg(position.y*Units::mtoUnit(), 11,'g',5)
                                                         .arg(position.z*Units::mtoUnit(), 11,'g',5));

        writeTextElement("Type", xflfuse.isFlatFaceType()? "FLATPANELS" : "NURBS");

        if(xflfuse.isFlatFaceType())
        {
            writeStartElement("Panel_Stripes");
            {
                for(int isl=0; isl<xflfuse.sideLineCount(); isl++)
                {
                    writeTextElement(QString("stripe_%1").arg(isl),  QString("%1").arg(xflfuse.hPanels(isl)));
                }
            }
            writeEndElement();
        }
        else
        {
            writeTextElement("x_panels", QString ("%1").arg(xflfuse.nxNurbsPanels()));
            writeTextElement("hoop_panels", QString ("%1").arg(xflfuse.nhNurbsPanels()));
        }

        writeStartElement("Inertia");
        {
            writeTextElement("Volume_Mass", QString("%1").arg(xflfuse.structuralMass(),7,'f',3));
            for(int ipm=0; ipm<xflfuse.pointMassCount(); ipm++)
            {
                writePointMass(xflfuse.inertia().pointMassAt(ipm));
            }
        }
        writeEndElement();

        writeNURBS(nurbs, xflfuse.xPanels(), xflfuse.isFlatFaceType());
    }
    writeEndElement();
}


void XflXmlWriter::writeNURBS(NURBSSurface const &nurbs, std::vector<int> const &xPanels, bool bFramesOnly)
{
    double lengthUnit = Units::mtoUnit();
    writeStartElement("NURBS");
    {
        if(!bFramesOnly)
        {
            writeTextElement("u_degree",           QString ("%1").arg(nurbs.uDegree()));
            writeTextElement("v_degree",           QString ("%1").arg(nurbs.vDegree()));
            writeTextElement("uAxis",              QString ("%1").arg(nurbs.uAxis()));
            writeTextElement("vAxis",              QString ("%1").arg(nurbs.VAxis()));
            writeTextElement("uEdgeWeight",        QString ("%1").arg(nurbs.uEdgeWeight()));
            writeTextElement("vEdgeWeight",        QString ("%1").arg(nurbs.vEdgeWeight()));
            writeTextElement("Bunch_amplitude",    QString ("%1").arg(nurbs.bunchAmplitude()));
            writeTextElement("Bunch_distribution", QString ("%1").arg(nurbs.bunchDist()));
        }

        for(int iFrame=0; iFrame<nurbs.frameCount(); iFrame++)
        {
            Frame const &frame = nurbs.frameAt(iFrame);
            writeStartElement("frame");
            {
                writeTextElement("Angle", QString::asprintf("%g", frame.angle()));
                if(iFrame<int(xPanels.size()))
                    writeTextElement("x_panels", QString::asprintf("%d",xPanels.at(iFrame)));
                else
                    writeTextElement("x_panels", "1");

                writeTextElement("Position", QString("%1, %2, %3").arg(frame.position().x*lengthUnit, 11,'g',5)
                                                                 .arg(frame.position().y*lengthUnit, 11,'g',5)
                                                                 .arg(frame.position().z*lengthUnit, 11,'g',5));

                for(int iPt=0; iPt<frame.nCtrlPoints(); iPt++)
                {
                    Vector3d Pt(frame.pointAt(iPt));
                    writeTextElement("point",QString("%1, %2, %3").arg(Pt.x*lengthUnit, 11,'g',5)
                                                                  .arg(Pt.y*lengthUnit, 11,'g',5)
                                                                  .arg(Pt.z*lengthUnit, 11,'g',5));
                }
            }
            writeEndElement();
        }
    }
    writeEndElement();
}


void XflXmlWriter::writeExtraDrag(std::vector<ExtraDrag> const &XDrag)
{
    writeStartElement("ExtraDrag");
    {
        writeComment("Tag,  Area,  coef");
        for(uint iex=0; iex<XDrag.size(); iex++)
        {
            if(fabs(XDrag.at(iex).area())>PRECISION && fabs(XDrag.at(iex).coef())>PRECISION)
            {
                QString strong;
                strong = QString::asprintf(", %g, %g", XDrag.at(iex).area(), XDrag.at(iex).coef());
                strong  = QString::fromStdString(XDrag.at(iex).tag()) + strong;
                writeTextElement("Drag", strong);
            }
        }
    }
    writeEndElement();
}


void XflXmlWriter::writeInertia(Inertia const &inertia)
{
    QString strange;
    writeStartElement("Inertia");
    {
        writeTextElement("Mass", QString("%1").arg(inertia.structuralMass()*Units::kgtoUnit(),11,'f',5));
        strange = QString::asprintf("%.5g, %.5g, %.5g", inertia.CoG_s().x*Units::mtoUnit(), inertia.CoG_s().y*Units::mtoUnit(), inertia.CoG_s().z*Units::mtoUnit());
        writeTextElement("CoG", strange);
        writeTextElement("CoG_Ixx", QString("%1").arg(inertia.Ixx_s()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Iyy", QString("%1").arg(inertia.Iyy_s()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Izz", QString("%1").arg(inertia.Izz_s()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Ixz", QString("%1").arg(inertia.Ixz_s()*Units::kgm2toUnit(), 11, 'g', 5));

        for(int ipm=0; ipm<inertia.pointMassCount(); ipm++)
        {
            PointMass const &pm = inertia.pointMassAt(ipm);
            writePointMass(pm);
        }
    }
    writeEndElement();
}


void XflXmlWriter::writeFluidProperties(double viscosity, double density)
{
    writeStartElement("Fluid");
    {
        writeTextElement("Viscosity", QString("%1").arg(viscosity,11,'g',5));
        writeTextElement("Density",   QString("%1").arg(density,11,'g',5));
    }
    writeEndElement();
}


void XflXmlWriter::writeWakeData(Polar3d const &polar3d)
{
    writeStartElement("Wake");
    {
        writeComment("Set the following field to true for a conventional flat panel wake, and to false for a vorton wake. "
                     "The default is true. All lengths are in reference chord units.");
        writeTextElement("FlatPanelWake", xfl::boolToString(!polar3d.bVortonWake()));
        if(!polar3d.bVortonWake())
        {
            writeComment("These parameters define the wake panels;\n"
                         "                NX is the number of panels in one column,\n"
                         "                ProgressionFactor is the geometric progression factor between rows n+1 and n\n"
                         "                LengthFactor is the total wake length in M.A.C. units");
            writeComment("Recommended values: NX=5, ProgressionFactor=1.1, LengthFactor=30");
            writeTextElement("NX", QString("%1").arg(polar3d.NXWakePanel4()));
            writeTextElement("ProgressionFactor", QString("%1").arg(polar3d.wakePanelFactor(),       7, 'f', 3));
            writeTextElement("LengthFactor",      QString("%1").arg(polar3d.totalWakeLengthFactor(), 7, 'f', 3));
        }
        else
        {
            writeTextElement("VPW_BufferWakeLength",  QString::asprintf("%g", polar3d.bufferWakeFactor()));
            writeTextElement("VPW_FirstStep",         QString::asprintf("%g", polar3d.vortonL0()));
//            writeTextElement("VPW_GeomFactor",        QString::asprintf("%g", polar3d.vortonXFactor()));
            writeTextElement("VPW_MaxLength",         QString::asprintf("%g", polar3d.VPWMaxLength()));
            writeTextElement("Vorton_Core_Size",      QString::asprintf("%g", polar3d.vortonCoreSize()));
            writeTextElement("VPW_Iterations",        QString::asprintf("%d", polar3d.VPWIterations()));
        }
    }
    writeEndElement();
}


void XflXmlWriter::writeWing(WingXfl const &wing, Vector3d WingLE, double Rx, double Ry, bool bWriteFoils)
{
    writeStartElement("wing");
    {
        writeTextElement("Name", QString::fromStdString(wing.name()));
        writeComment("Available options are MAINWING, SECONDWING, ELEVATOR, FIN, OTHERWING");
        writeTextElement("Type", xml::wingType(wing.wingType()));
        writeColor(wing.color());
        writeTextElement("Description", QString::fromStdString(wing.description()));
        writeTextElement("Position",QString("%1, %2, %3").arg(WingLE.x*Units::mtoUnit(), 11,'g',5)
                                                         .arg(WingLE.y*Units::mtoUnit(), 11,'g',5)
                                                         .arg(WingLE.z*Units::mtoUnit(), 11,'g',5));
        writeComment("The field Tip_strips specifies the number of horizontal panel strips at the tips.");
        writeTextElement("Tip_Strips",        QString::asprintf("%d", wing.nTipStrips()));
        writeTextElement("Rx_angle",          QString("%1").arg(Rx,7,'f',3));
        writeTextElement("Ry_angle",          QString("%1").arg(Ry,7,'f',3));
        writeTextElement("symmetric",         xfl::boolToString(wing.isSymmetric()));
        writeTextElement("Two_Sided",         xfl::boolToString(wing.isTwoSided()));
        writeTextElement("Closed_Inner_Side", xfl::boolToString(wing.isClosedInnerSide()));

        writeComment("If the field AUTOINERTIA is set to TRUE, the fields COG and COG_I** will be ignored");
        writeTextElement("AutoInertia", xfl::boolToString(wing.bAutoInertia()));
        writeInertia(wing.inertia());

        writeStartElement("Sections");
        {
            for(int ips=0; ips<wing.nSections(); ips++)
            {
                WingSection const &ws = wing.section(ips);
                writeStartElement("Section");
                {
                    writeTextElement("y_position",           QString("%1").arg(ws.m_YPosition*Units::mtoUnit(), 7, 'f', 3));
                    writeTextElement("Chord",                QString("%1").arg(ws.m_Chord*Units::mtoUnit(), 7, 'f', 3));
                    writeTextElement("xOffset",              QString("%1").arg(ws.m_Offset*Units::mtoUnit(), 7, 'f', 3));
                    writeTextElement("Dihedral",             QString("%1").arg(ws.m_Dihedral, 7, 'f', 3));
                    writeTextElement("Twist",                QString("%1").arg(ws.m_Twist, 7, 'f', 3));
                    writeTextElement("x_number_of_panels",   QString("%1").arg(ws.m_NXPanels));
                    writeTextElement("x_panel_distribution", QString::fromStdString(xfl::distributionType(ws.m_XPanelDist)));
                    writeTextElement("y_number_of_panels",   QString("%1").arg(ws.m_NYPanels));
                    writeTextElement("y_panel_distribution", QString::fromStdString(xfl::distributionType(ws.m_YPanelDist)));
                    if(bWriteFoils)
                    {
                        writeTextElement("Left_Side_Foil_File",  QString::fromStdString(ws.m_LeftFoilName) +".dat");
                        writeTextElement("Right_Side_Foil_File", QString::fromStdString(ws.m_RightFoilName)+".dat");
                    }
                    else
                    {
                        writeTextElement("Left_Side_FoilName",  QString::fromStdString(ws.m_LeftFoilName));
                        writeTextElement("Right_Side_FoilName", QString::fromStdString(ws.m_RightFoilName));
                    }

                }
                writeEndElement();
            }
        }
        writeEndElement();
    }
    writeEndElement();

    if(bWriteFoils) return;

    //make a list of unique foil names;
    QStringList foilnames;
    for(int is=0; is<wing.nSections(); is++)
    {
        WingSection const &ws = wing.section(is);
        if(!foilnames.contains(QString::fromStdString(ws.leftFoilName())))  foilnames.append(QString::fromStdString(ws.leftFoilName()));
        if(!foilnames.contains(QString::fromStdString(ws.rightFoilName()))) foilnames.append(QString::fromStdString(ws.rightFoilName()));
    }

    for(int is=0; is<foilnames.size(); is++)
    {
        Foil const*m_pFoil = Objects2d::foil(foilnames.at(is).toStdString());
        if(!m_pFoil) continue;

        QString filename = m_Path + QDir::separator() + foilnames.at(is)+".dat";

        QFile XFile(filename);
        if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

        std::string outstring;
        m_pFoil->exportFoilToDat(outstring);
        QTextStream out(&XFile);
        out << QString::fromStdString(outstring);
        XFile.close();
    }
}




