/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    
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



#include <api/xmlwpolarwriter.h>

#include <api/objects3d.h>
#include <api/objects_global.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/wpolar.h>

XmlWPolarWriter::XmlWPolarWriter(QFile &XFile) : XflXmlWriter(XFile)
{
}


void XmlWPolarWriter::writeHeader()
{
}


void XmlWPolarWriter::writeXMLWPolar(const WPolar *pWPolar)
{
    if(!pWPolar) return;

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
            writeWPolarData(pWPolar);
        }
        writeEndElement();
    }
    writeEndDocument();
}


void XmlWPolarWriter::writeWPolarData(WPolar const *pWPolar)
{
//    Plane const *pPlane = Objects3d::plane(pWPolar->planeName());
//    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);

    QString strange;
    writeStartElement("Polar");
    {
        writeTextElement("Polar_Name", pWPolar->name());

        writeComment("For scripts and batch analyses: if the plane's name is left blank, the analysis will be associated to all available planes");
        writeTextElement("Plane_Name", pWPolar->planeName());

        writeTheStyle(pWPolar->theStyle());
        writeComment("Available analysis types are: FIXEDSPEEDPOLAR, FIXEDLIFTPOLAR, GLIDEPOLAR, CONTROLPOLAR, STABILITYPOLAR, T8POLAR");
        writeTextElement("Type",                 objects::polarType(pWPolar->type()));
        writeComment("Available methods are: LLT, VLM1, VLM2, QUADS, TRIUNIFORM, TRILINEAR");
        writeTextElement("Method",               Polar3d::analysisMethod(pWPolar->analysisMethod()));
        writeTextElement("Include_Fuse_Moments", pWPolar->bFuseMi()            ? "true" : "false");
        writeTextElement("Thin_Surfaces",        pWPolar->bThinSurfaces()      ? "true" : "false");
        writeTextElement("Ground_Effect",        pWPolar->bGroundEffect()      ? "true" : "false");
        writeTextElement("Free_Surface",         pWPolar->bFreeSurfaceEffect() ? "true" : "false");
        writeTextElement("Ground_Height",        QString::asprintf("%g", pWPolar->groundHeight()*Units::mtoUnit()));

        writeWakeData(*pWPolar);

        writeStartElement("Reference_Dimensions");
        {
            writeComment    ("Available options are PLANFORM, PROJECTED, CUSTOM");
            writeTextElement("Reference_Dimensions",   Polar3d::referenceDimension(pWPolar->referenceDim()));
            writeComment("The following fields are required only if dimensions are set to CUSTOM");
            writeTextElement("Reference_Area",          QString::asprintf("%g", pWPolar->referenceArea()*Units::m2toUnit()));
            writeTextElement("Reference_Span_Length",   QString::asprintf("%g", pWPolar->referenceSpanLength()*Units::mtoUnit()));
            writeTextElement("Reference_Chord_Length",  QString::asprintf("%g", pWPolar->referenceChordLength()*Units::mtoUnit()));
            writeTextElement("Include_Other_Wing_Area", pWPolar->bIncludeOtherWingAreas() ? "true" : "false");
        }
        writeEndElement();

        writeFluidProperties(pWPolar->viscosity(), pWPolar->density());

        writeStartElement("Viscous_Analysis");
        {
            writeTextElement("Is_Viscous_Analysis", pWPolar->isViscous()?        "true" : "false");
            writeTextElement("XFoil_OnTheFly",    pWPolar->isViscOnTheFly()?     "true" : "false");
            writeComment("Set the field From_CL to true if the viscous properties are interpolated using the local lift coefficient, i.e. the xflr5 method; "
                         "and to false if they are interpolated from the local apparent angle, e.g. the case of control polars and the viscous loop.");
            writeTextElement("From_CL",             pWPolar->isViscFromCl() ?       "true" : "false");
            strange = QString::asprintf("%.2f", pWPolar->NCrit());
            writeTextElement("NCrit",   strange);
            strange = QString::asprintf("%.3f", pWPolar->XTrTop());
            writeTextElement("XTrTop",   strange);
            strange = QString::asprintf("%.3f", pWPolar->XTrBot());
            writeTextElement("XTrBot",   strange);
        }
        writeEndElement();

        writeStartElement("Fuselage_Drag");
        {
            writeTextElement("Friction_Drag", pWPolar->hasFuseDrag()? "true" : "false");
            writeComment("Set the field Friction_drag_method to either KARMAN-SCHOENHERR or PRANDTL-SCHLICHTING");
            if(pWPolar->fuseDragMethod()==WPolar::KARMANSCHOENHERR) writeTextElement("Friction_Drag_Method", "Karman-Schoenherr");
            else                                                    writeTextElement("Friction_Drag_Method", "Prandtl-Schlichting");
        }
        writeEndElement();

        if(pWPolar->hasExtraDrag()) writeExtraDrag(pWPolar->extraDragList());

        writeTextElement("Use_plane_inertia",  pWPolar->bAutoInertia() ? "true" : "false");

        writeComment("The inertia fields are used only if Use_plane_inertia is set to false");
        writeWPolarInertia(pWPolar);

        if(pWPolar->isFixedSpeedPolar() || pWPolar->isBetaPolar())
        {
            writeTextElement("Fixed_Velocity",     QString("%1").arg(pWPolar->velocity()*Units::mstoUnit(),11,'f',5));
        }
        if(pWPolar->isBetaPolar())
        {
            writeTextElement("Fixed_AOA",          QString("%1").arg(pWPolar->alphaSpec(),11,'f',5));
        }

        if(pWPolar->isStabilityPolar())
        {
            if(pWPolar->nAVLControls()>=0)
            {
                writeStartElement("AVL_controls");
                {
                    strange = QString("For each AVL-type control, specify the name and the list of gains separatetd by spaces;"
                                      "the gain values should be listed in the same order as they appear in the GUI's editor");
                    writeComment(strange);
                    for(int ie=0; ie<pWPolar->nAVLControls(); ie++)
                    {
                        AngleControl const &ctrl = pWPolar->AVLCtrl(ie);
                        writeStartElement("Control");
                        {
                            writeTextElement("Name", ctrl.name());
                            strange.clear();
                            for(int ic=0; ic<ctrl.nValues(); ic++)
                            {
                                strange += QString::asprintf("  %g", pWPolar->AVLGain(ie, ic));
                            }
                            writeTextElement("gains", strange);
                        }
                        writeEndElement();
                    }
                }
                writeEndElement();
            }
        }


        if(pWPolar->isType12358() || pWPolar->isType7())
        {
            writeStartElement("Flap_settings");
            {
                QString str;
                for(int iwing=0; iwing<pWPolar->nFlapCtrls(); iwing++)
                {
                    AngleControl const &ac = pWPolar->flapCtrls(iwing);
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

        if(pWPolar->isControlPolar())
        {
            writeStartElement("Operating_range");
            {
                QString str;
                writeTextElement("Adjusted_velocity", pWPolar->isAdjustedVelocity() ? "true" : "false");

                str = QString::asprintf("%3.f %3.f", pWPolar->m_OperatingRange.at(0).ctrlMin()*Units::mstoUnit(), pWPolar->m_OperatingRange.at(0).ctrlMax()*Units::mstoUnit());
                writeTextElement("Velocity",  str);
                str = QString::asprintf("%3.f %3.f", pWPolar->m_OperatingRange.at(1).ctrlMin(), pWPolar->m_OperatingRange.at(1).ctrlMax());
                writeTextElement("Alpha",  str);
                str = QString::asprintf("%3.f %3.f", pWPolar->m_OperatingRange.at(2).ctrlMin(), pWPolar->m_OperatingRange.at(2).ctrlMax());
                writeTextElement("Beta",  str);
            }
            writeEndElement();

            writeStartElement("Inertia_range");
            {
                QString str;
                str = QString::asprintf("%3.f %3.f", pWPolar->m_InertiaRange.at(0).ctrlMin()*Units::kgtoUnit(), pWPolar->m_InertiaRange.at(0).ctrlMax()*Units::kgtoUnit());
                writeTextElement("Mass",  str);
                str = QString::asprintf("%3.f %3.f", pWPolar->m_InertiaRange.at(1).ctrlMin()*Units::mtoUnit(), pWPolar->m_InertiaRange.at(1).ctrlMax()*Units::mtoUnit());
                writeTextElement("CoG_x", str);
                str = QString::asprintf("%3.f %3.f", pWPolar->m_InertiaRange.at(2).ctrlMin()*Units::mtoUnit(), pWPolar->m_InertiaRange.at(2).ctrlMax()*Units::mtoUnit());
                writeTextElement("CoG_z", str);
            }
            writeEndElement();

            writeStartElement("Angle_range");
            {
                for(int iw=0; iw<pWPolar->m_AngleRange.size(); iw++)
                {
                    QString strange;
                    strange = QString::asprintf("Wing_%d", iw);
                    writeStartElement(strange);
                    {
                        for(int c=0; c<pWPolar->m_AngleRange.at(iw).size(); c++)
                        {
                            QString str;
                            str = QString::asprintf("%.3f %.3f", pWPolar->angleRange(iw, c).ctrlMin(), pWPolar->angleRange(iw, c).ctrlMax());
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


void XmlWPolarWriter::writeWPolarInertia(WPolar const*pWPolar)
{
    QString strange;
    writeStartElement("Inertia");
    {
        writeTextElement("Mass", QString("%1").arg(pWPolar->mass()*Units::kgtoUnit(),11,'f',5));
        strange = QString::asprintf("%.5g, %.5g, %.5g", pWPolar->CoG().x*Units::mtoUnit(), pWPolar->CoG().y*Units::mtoUnit(), pWPolar->CoG().z*Units::mtoUnit());
        writeTextElement("CoG", strange);
        writeTextElement("CoG_Ixx", QString("%1").arg(pWPolar->Ixx()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Iyy", QString("%1").arg(pWPolar->Iyy()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Izz", QString("%1").arg(pWPolar->Izz()*Units::kgm2toUnit(), 11, 'g', 5));
        writeTextElement("CoG_Ixz", QString("%1").arg(pWPolar->Ixz()*Units::kgm2toUnit(), 11, 'g', 5));
    }
    writeEndElement();
}



