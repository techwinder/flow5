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



#include <xmlplanepolarwriter.h>


#include <objects3d.h>
#include <objects_global.h>
#include <planexfl.h>
#include <planepolar.h>
#include <units.h>
#include <xml_globals.h>


XmlPlanePolarWriter::XmlPlanePolarWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlPlanePolarWriter::writeHeader()
{
}


void XmlPlanePolarWriter::writeXMLWPolar(const PlanePolar *pPlPolar)
{
    if(!pPlPolar) return;

    setAutoFormatting(true);

    writeStartDocument();
    {
        //    writeHeader();
        writeDTD("<!DOCTYPE flow5>");
        writeStartElement("xflPlanePolar");
        {
            writeAttribute("version", "1.0");
            writeComment("For convenience, all field names are case-insensitive, as an exception to the standard xml specification");
            writeComment("Where applicable, default values will be used for all undefined fields");
            writeUnits();
            writeWPolarData(pPlPolar);
        }
        writeEndElement();
    }
    writeEndDocument();
}


void XmlPlanePolarWriter::writeWPolarData(PlanePolar const *pPlPolar)
{
//    Plane const *pPlane = Objects3d::plane(pWPolar->planeName());
//    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    QString strange;
    writeStartElement("Polar");
    {
        writeTextElement("Polar_Name", QString::fromStdString(pPlPolar->name()));

        writeComment("For scripts and batch analyses: if the plane's name is left blank, the analysis will be associated to all available planes");
        writeTextElement("Plane_Name", QString::fromStdString(pPlPolar->planeName()));

        writeTheStyle(pPlPolar->theStyle());
        writeComment("Available analysis types are: FIXEDSPEEDPOLAR, FIXEDLIFTPOLAR, GLIDEPOLAR, CONTROLPOLAR, STABILITYPOLAR, T8POLAR");
        writeTextElement("Type",                 xml::polarType(pPlPolar->type()));
        writeComment("Available methods are: LLT, VLM1, VLM2, QUADS, TRIUNIFORM, TRILINEAR");
        writeTextElement("Method",               xml::analysisMethod(pPlPolar->analysisMethod()));
        writeTextElement("Include_Fuse_Moments", pPlPolar->bFuseMi()            ? "true" : "false");
        writeTextElement("Thin_Surfaces",        pPlPolar->bThinSurfaces()      ? "true" : "false");
        writeTextElement("Ground_Effect",        pPlPolar->bGroundEffect()      ? "true" : "false");
        writeTextElement("Free_Surface",         pPlPolar->bFreeSurfaceEffect() ? "true" : "false");
        writeTextElement("Ground_Height",        QString::asprintf("%g", pPlPolar->groundHeight()*Units::mtoUnit()));

        writeWakeData(*pPlPolar);

        writeStartElement("Reference_Dimensions");
        {
            writeComment    ("Available options are PLANFORM, PROJECTED, CUSTOM");
            writeTextElement("Reference_Dimensions",    xml::referenceDimension(pPlPolar->referenceDim()));
            writeComment("The following fields are required only if dimensions are set to CUSTOM");
            writeTextElement("Reference_Area",          QString::asprintf("%g", pPlPolar->referenceArea()*Units::m2toUnit()));
            writeTextElement("Reference_Span_Length",   QString::asprintf("%g", pPlPolar->referenceSpanLength()*Units::mtoUnit()));
            writeTextElement("Reference_Chord_Length",  QString::asprintf("%g", pPlPolar->referenceChordLength()*Units::mtoUnit()));
            writeTextElement("Include_Other_Wing_Area", pPlPolar->bIncludeOtherWingAreas() ? "true" : "false");
        }
        writeEndElement();

        writeFluidProperties(pPlPolar->viscosity(), pPlPolar->density());

        writeStartElement("Viscous_Analysis");
        {
            writeTextElement("Is_Viscous_Analysis", pPlPolar->isViscous()?        "true" : "false");
            writeTextElement("XFoil_OnTheFly",    pPlPolar->isViscOnTheFly()?     "true" : "false");
            writeComment("Set the field From_CL to true if the viscous properties are interpolated using the local lift coefficient, i.e. the xflr5 method; "
                         "and to false if they are interpolated from the local apparent angle, e.g. the case of control polars and the viscous loop.");
            writeTextElement("From_CL",             pPlPolar->isViscFromCl() ?       "true" : "false");
            strange = QString::asprintf("%.2f", pPlPolar->NCrit());
            writeTextElement("NCrit",   strange);
            strange = QString::asprintf("%.3f", pPlPolar->XTrTop());
            writeTextElement("XTrTop",   strange);
            strange = QString::asprintf("%.3f", pPlPolar->XTrBot());
            writeTextElement("XTrBot",   strange);
            writeTextElement("TransAtHinge", pPlPolar->bTransAtHinge() ? "true" : "false");
        }
        writeEndElement();

        writeStartElement("Fuselage_Drag");
        {
            writeTextElement("Friction_Drag", pPlPolar->hasFuseDrag()? "true" : "false");
            writeComment("Set the field Friction_drag_method to either KARMAN-SCHOENHERR or PRANDTL-SCHLICHTING");
            if(pPlPolar->fuseDragMethod()==PlanePolar::KARMANSCHOENHERR) writeTextElement("Friction_Drag_Method", "Karman-Schoenherr");
            else                                                    writeTextElement("Friction_Drag_Method", "Prandtl-Schlichting");
        }
        writeEndElement();

        if(pPlPolar->hasExtraDrag()) writeExtraDrag(pPlPolar->extraDragList());

        writeTextElement("Use_plane_inertia",  pPlPolar->bAutoInertia() ? "true" : "false");

        writeComment("The inertia fields are used only if Use_plane_inertia is set to false");
        writeWPolarInertia(pPlPolar);

        if(pPlPolar->isFixedSpeedPolar() || pPlPolar->isBetaPolar())
        {
            writeTextElement("Fixed_Velocity",     QString("%1").arg(pPlPolar->velocity()*Units::mstoUnit(),11,'f',5));
        }
        if(pPlPolar->isBetaPolar())
        {
            writeTextElement("Fixed_AOA",          QString("%1").arg(pPlPolar->alphaSpec(),11,'f',5));
        }

        if(pPlPolar->isStabilityPolar())
        {
            if(pPlPolar->nAVLCtrls()>=0)
            {
                writeStartElement("AVL_controls");
                {
                    strange = QString("For each AVL-type control, specify the name and the list of gains separatetd by spaces;"
                                      "the gain values should be listed in the same order as they appear in the GUI's editor");
                    writeComment(strange);
                    for(int ie=0; ie<pPlPolar->nAVLCtrls(); ie++)
                    {
                        AngleControl const &ctrl = pPlPolar->AVLCtrl(ie);
                        writeStartElement("Control");
                        {
                            writeTextElement("Name", QString::fromStdString(ctrl.name()));
                            strange.clear();
                            for(int ic=0; ic<ctrl.nValues(); ic++)
                            {
                                strange += QString::asprintf("  %g", pPlPolar->AVLGain(ie, ic));
                            }
                            writeTextElement("gains", strange);
                        }
                        writeEndElement();
                    }
                }
                writeEndElement();
            }
        }


        if(pPlPolar->isType123458() || pPlPolar->isType7())
        {
            writeStartElement("Flap_settings");
            {
                QString str;
                for(int iwing=0; iwing<pPlPolar->nFlapCtrls(); iwing++)
                {
                    AngleControl const &ac = pPlPolar->flapCtrls(iwing);
                    writeStartElement(QString::asprintf("Wing_%d", iwing+1));
                    {
                        for(int k=0; k<ac.nValues(); k++)
                        {
                            str = QString::asprintf("%.3f", ac.value(k));
                            writeTextElement(QString::asprintf("flap_%d", k+1), str);
                        }
                    }
                    writeEndElement();
                }
            }
            writeEndElement();
        }

        if(pPlPolar->isControlPolar())
        {
            writeStartElement("Operating_range");
            {
                QString str;
                writeTextElement("Adjusted_velocity", pPlPolar->isAdjustedVelocity() ? "true" : "false");

                str = QString::asprintf("%3.f %3.f", pPlPolar->m_OperatingRange.at(0).ctrlMin()*Units::mstoUnit(), pPlPolar->m_OperatingRange.at(0).ctrlMax()*Units::mstoUnit());
                writeTextElement("Velocity",  str);
                str = QString::asprintf("%3.f %3.f", pPlPolar->m_OperatingRange.at(1).ctrlMin(), pPlPolar->m_OperatingRange.at(1).ctrlMax());
                writeTextElement("Alpha",  str);
                str = QString::asprintf("%3.f %3.f", pPlPolar->m_OperatingRange.at(2).ctrlMin(), pPlPolar->m_OperatingRange.at(2).ctrlMax());
                writeTextElement("Beta",  str);
            }
            writeEndElement();

            writeStartElement("Inertia_range");
            {
                QString str;
                str = QString::asprintf("%3.f %3.f", pPlPolar->m_InertiaRange.at(0).ctrlMin()*Units::kgtoUnit(), pPlPolar->m_InertiaRange.at(0).ctrlMax()*Units::kgtoUnit());
                writeTextElement("Mass",  str);
                str = QString::asprintf("%3.f %3.f", pPlPolar->m_InertiaRange.at(1).ctrlMin()*Units::mtoUnit(), pPlPolar->m_InertiaRange.at(1).ctrlMax()*Units::mtoUnit());
                writeTextElement("CoG_x", str);
                str = QString::asprintf("%3.f %3.f", pPlPolar->m_InertiaRange.at(2).ctrlMin()*Units::mtoUnit(), pPlPolar->m_InertiaRange.at(2).ctrlMax()*Units::mtoUnit());
                writeTextElement("CoG_z", str);
            }
            writeEndElement();

            writeStartElement("Angle_range");
            {
                for(int iw=0; iw<int(pPlPolar->m_AngleRange.size()); iw++)
                {
                    QString strange;
                    strange = QString::asprintf("Wing_%d", iw);
                    writeStartElement(strange);
                    {
                        for(uint c=0; c<pPlPolar->m_AngleRange.at(iw).size(); c++)
                        {
                            QString str;
                            str = QString::asprintf("%.3f %.3f", pPlPolar->angleRange(iw, c).ctrlMin(), pPlPolar->angleRange(iw, c).ctrlMax());
                            writeTextElement(QString::fromUtf8("Range_%1").arg(c), str);
                        }
                    }
                    writeEndElement();
                }
            }
            writeEndElement();
        }
    }
    writeEndElement();
}


void XmlPlanePolarWriter::writeWPolarInertia(PlanePolar const*pPlPolar)
{
    QString strange;
    writeStartElement("Inertia");
    {
        writeTextElement("Mass", QString("%1").arg(pPlPolar->mass()*Units::kgtoUnit(),11,'f',5));
        strange = QString::asprintf("%.5g, %.5g, %.5g", pPlPolar->CoG().x*Units::mtoUnit(), pPlPolar->CoG().y*Units::mtoUnit(), pPlPolar->CoG().z*Units::mtoUnit());
        writeTextElement("CoG", strange);
        writeTextElement("CoG_Ixx", QString("%1").arg(pPlPolar->Ixx()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Iyy", QString("%1").arg(pPlPolar->Iyy()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Izz", QString("%1").arg(pPlPolar->Izz()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Ixz", QString("%1").arg(pPlPolar->Ixz()*Units::kgm2toUnit(), 11, 'g', 5));
    }
    writeEndElement();
}



