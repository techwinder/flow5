/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <xflobjects/objects3d/analysis/boatpolar.h>
#include <xflobjects/objects3d/analysis/boatopp.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/sailobjects/sails/sail.h>
#include <xflobjects/sailobjects/sailobjects.h>
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xflmath/constants.h>

QStringList BoatPolar::s_BtPolarVariableNames={"Ctrl",
                                               "TWS", "TWA ("+DEGCHAR+")", "AWS", "AWA ("+DEGCHAR+")",
                                               PHICHAR,
                                               "CD", "CDi", "CDv", "CL",
                                               "CX", "CY", "CX_sum", "CY_sum",
                                               "FFFx (N)", "FFFy (N)", "FFFz (N)",
                                               "Fx (N)", "Fy (N)", "Fz (N)",
                                               "L (N.m)", "M (N.m)", "N (N.m)"};


BoatPolar::BoatPolar() : Polar3d()
{
    m_Name.clear();
    m_BoatName.clear();
    setDefaultSpec(nullptr);
    clearData();
}


void BoatPolar::setDefaultSpec(Boat const *pBoat)
{
    m_AnalysisMethod = Polar3d::QUADS;
    m_Type      = xfl::BOATPOLAR;

    m_bIgnoreBodyPanels = true;

    m_bGround       = true;
    m_GroundHeight = 0.0;

    m_ReferenceDim = Polar3d::AUTODIMS;
    m_ReferenceArea = 1.0;
    m_ReferenceChord = 1.0;

    m_VBtMin = m_VBtMax = 0.0;
    m_TWSMin = m_TWSMax = 10.0; //wind speed
    m_TWAMin = m_TWAMax = 0.0;  //sideslip = wind angle
    m_PhiMin = m_PhiMax = 0.0;  //heeling angle
    m_RyMin  = m_RyMax = 0.0;   //Rotation around Y (°), special for windsurfs

    m_WindSpline.clearControlPoints();
    m_WindSpline.setDegree(2);
    m_WindSpline.appendControlPoint(0.0,  0.0);
    m_WindSpline.appendControlPoint(0.5,  1.0);
    m_WindSpline.appendControlPoint(1.0, 10.0);
    m_WindSpline.appendControlPoint(1.0, 15.0);
    m_WindSpline.updateSpline();
    m_WindSpline.makeCurve();

    m_bTrefftz  = true ;
    m_AnalysisMethod = Polar3d::TRIUNIFORM;
    m_bViscous = true;
    m_NCrit = 9;
    m_XTrTop = m_XTrBot = 1.0;
    m_bAutoInertia = true;

    m_Density = 1.225;
    m_Viscosity = 1.5e-5;

    m_nXWakePanel4 = 5;
    m_TotalWakeLengthFactor = 30.0;
    m_WakePanelFactor = 1.1;

    m_CoG.set(0,0,0);

    if(pBoat)
    {
        resizeSailAngles(pBoat->nSails());
        m_SailAngleMin.fill(0);
        m_SailAngleMax.fill(0);
    }
    else
    {
        m_SailAngleMin.clear();
        m_SailAngleMax.clear();
    }

    m_ExtraDrag.clear();
}


void BoatPolar::duplicateSpec(const Polar3d *pPolar3d)
{
    Polar3d::duplicateSpec(pPolar3d);
    if(!pPolar3d->isBoatPolar()) return;

    BoatPolar const *pBtPolar = dynamic_cast<BoatPolar const*>(pPolar3d);

    m_VBtMin = pBtPolar->m_VBtMin;
    m_VBtMax = pBtPolar->m_VBtMax;
    m_TWSMax     = pBtPolar->m_TWSMax;
    m_TWSMin     = pBtPolar->m_TWSMin;
    m_TWSMax     = pBtPolar->m_TWSMax;
    m_TWAMin     = pBtPolar->m_TWAMin;
    m_TWAMax     = pBtPolar->m_TWAMax;
    m_PhiMin     = pBtPolar->m_PhiMin;
    m_PhiMax     = pBtPolar->m_PhiMax;
    m_RyMin      = pBtPolar->m_RyMin;
    m_RyMax      = pBtPolar->m_RyMax;

    m_SailAngleMin = pBtPolar->m_SailAngleMin;
    m_SailAngleMax = pBtPolar->m_SailAngleMax;

    m_ReferenceArea  = pBtPolar->m_ReferenceArea;
    m_ReferenceChord = pBtPolar->m_ReferenceChord;

    m_WindSpline = pBtPolar->m_WindSpline;
}


void BoatPolar::addPoint(BoatOpp const *pBtOpp)
{
    bool bInserted = false;
    int i=0;
    int size = m_Ctrl.size();
    if(size)
    {
        for (i=0; i<size; i++)
        {
            if (fabs(pBtOpp->ctrl() - m_Ctrl.at(i)) < 0.001)
            {
                // then erase former result
                m_Ctrl[i]    =  pBtOpp->ctrl();
                m_VInf[i]    =  pBtOpp->QInf();
                m_Beta[i]    =  pBtOpp->beta();
                m_Phi[i]     =  pBtOpp->phi();
                m_AC[i]      =  pBtOpp->aeroForces();
                bInserted = true;
                break;
            }
            else if (pBtOpp->ctrl() < m_Ctrl.at(i))
            {
                // sort by crescending control values
                m_Ctrl.insert(i, pBtOpp->ctrl());
                m_VInf.insert(i, pBtOpp->QInf());
                m_Beta.insert(i, pBtOpp->beta());
                m_Phi.insert( i, pBtOpp->phi());
                m_AC.insert(  i, pBtOpp->aeroForces());
                bInserted = true;
                break;
            }
        }
    }
    if(!bInserted)
    {
        // data is appended at the end
        m_Ctrl.append(pBtOpp->ctrl());
        m_VInf.append(pBtOpp->QInf());
        m_Beta.append(pBtOpp->beta());
        m_Phi.append(pBtOpp->phi());
        m_AC.append(pBtOpp->aeroForces());
    }
}


bool BoatPolar::serializeFl5v726(QDataStream &ar, bool bIsStoring)
{
    Polar3d::serializeFl5v726(ar, bIsStoring);

    bool boolean(false);
    int n(0);
    double dble(0),d0(0),d1(0),d2(0);
    int i(0);
    int nIntSpares(0);
    int nDbleSpares(0);

    if(bIsStoring)
    {
        Q_ASSERT(false);
    }
    else
    {
        //read variables
        ar >> m_PolarFormat;
        if(m_PolarFormat<500001 || m_PolarFormat>500100) return false;

        if(m_PolarFormat<500028) m_bIgnoreBodyPanels = true;

        ar >> m_BoatName;

        if(m_PolarFormat>=500023)
        {
            ar >> boolean;
            m_ReferenceDim = boolean ? Polar3d::AUTODIMS : Polar3d::CUSTOM;
            ar >> m_ReferenceArea >> m_ReferenceChord;
        }

        if(m_PolarFormat<500024)
        {
            ar >> dble >> dble >> dble >> dble; // formerly wind gradient
        }
        else
        {
            m_WindSpline.serializeFl5(ar, bIsStoring);
        }

        if(m_PolarFormat>=500026) ar >> m_VBtMin >> m_VBtMax;
        ar >> m_TWSMin>>m_TWSMax>>m_TWAMin>>m_TWAMax; // AWS up to format 500026

        ar >> m_PhiMin>>m_PhiMax;
        if(m_PolarFormat>=500025) ar >> m_RyMin >> m_RyMax;

        ar >> n;
        m_SailAngleMin.resize(n);
        m_SailAngleMax.resize(n);
        for (int is=0; is<n; is++)
        {
            ar >> m_SailAngleMin[is] >> m_SailAngleMax[is];
        }

        int datasize=0;
        ar >> datasize;
        ar >> nDbleSpares;
        m_AC.resize(datasize);
        for (i=0; i<datasize; i++)
        {
            ar >>d0;              m_Ctrl.append(d0);
            ar >>d0>>d1>>d2;      m_VInf.append(d0);   m_Beta.append(d1);   m_Phi.append(d2);
            if(m_PolarFormat<500022)  m_AC[i].serializeFl5_b17(ar, bIsStoring);
            else
            {
                if(!m_AC[i].serializeFl5(ar, bIsStoring)) return false;
            }

            for(int l=0; l<nDbleSpares; l++) ar >> dble;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++)  ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;


        // clean-up legacy formats
        m_Type = xfl::BOATPOLAR;
        m_bGround      = true;
        m_GroundHeight = 0.0;
    }
    return true;
}


bool BoatPolar::serializeFl5v750(QDataStream &ar, bool bIsStoring)
{
    Polar3d::serializeFl5v750(ar, bIsStoring);

    bool boolean(false);
    int integer(0);
    double dble(0);

    int n(0);
    double d0(0),d1(0),d2(0);
    int i(0);

    int nDbleSpares(0);

    if(bIsStoring)
    {
        //write variables
        ar << m_PolarFormat; // identifies the format of the file
        ar << m_BoatName;

        boolean = m_ReferenceDim==Polar3d::AUTODIMS ? true : false;
        ar << boolean;
        ar << m_ReferenceArea << m_ReferenceChord;

        m_WindSpline.serializeFl5(ar, bIsStoring);

        ar << m_VBtMin << m_VBtMax;
        ar << m_TWSMin << m_TWSMax << m_TWAMin << m_TWAMax;
        ar << m_PhiMin << m_PhiMax;
        ar << m_RyMin << m_RyMax;

        ar << int(m_SailAngleMin.size());
        for(int is=0; is<m_SailAngleMin.size();is++)
        {
            ar << m_SailAngleMin[is] << m_SailAngleMax[is];
        }

        ar <<int(m_Ctrl.size());

        nDbleSpares = 0;
        ar << nDbleSpares;
        for (i=0; i<m_Ctrl.size(); i++)
        {
            ar << m_Ctrl[i];
            ar << m_VInf[i] <<m_Beta[i] << m_Phi[i];
            m_AC[i].serializeFl5(ar, bIsStoring);

            dble=0.0;
            for(int i=0; i<nDbleSpares; i++) ar << dble;
        }

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar <<boolean;
        for(int i=0; i<20; i++) ar <<integer;
        for(int i=0; i<20; i++) ar <<dble;

        return true;
    }
    else
    {
        //read variables
        ar >> m_PolarFormat;
        if(m_PolarFormat<500750 || m_PolarFormat>501000) return false;


        ar >> m_BoatName;

        ar >> boolean;
        m_ReferenceDim = boolean ? Polar3d::AUTODIMS : Polar3d::CUSTOM;
        ar >> m_ReferenceArea >> m_ReferenceChord;

        m_WindSpline.serializeFl5(ar, bIsStoring);


        ar >> m_VBtMin >> m_VBtMax;
        ar >> m_TWSMin>>m_TWSMax>>m_TWAMin>>m_TWAMax; // AWS up to format 500026

        ar >> m_PhiMin>>m_PhiMax;
        ar >> m_RyMin >> m_RyMax;

        ar >> n;
        m_SailAngleMin.resize(n);
        m_SailAngleMax.resize(n);
        for (int is=0; is<n; is++)
        {
            ar >> m_SailAngleMin[is] >> m_SailAngleMax[is];
        }

        int datasize=0;
        ar >> datasize;
        ar >> nDbleSpares;
        m_AC.resize(datasize);
        for (i=0; i<datasize; i++)
        {
            ar >>d0;              m_Ctrl.append(d0);
            ar >>d0>>d1>>d2;      m_VInf.append(d0);   m_Beta.append(d1);   m_Phi.append(d2);
            if(!m_AC[i].serializeFl5(ar, bIsStoring)) return false;

            for(int l=0; l<nDbleSpares; l++) ar >> dble;
        }

        // provisions for future variable saves
        for(int i=0; i<10; i++) ar >>boolean;
        for(int i=0; i<20; i++) ar >>integer;
        for(int i=0; i<20; i++) ar >>dble;

    }
    return true;
} 


double BoatPolar::variable(int iVariable, int index) const
{
    if(iVariable<0 || iVariable>variableCount()) return 0.0;
    if(index<0 || index>dataSize())  return 0.0;
    return getVariable(iVariable, index);
}


double BoatPolar::getVariable(int iVar, int iPoint) const
{
    double q = qDyn(m_Ctrl.at(iPoint));
    switch (iVar)
    {
        default:
        case 0:  return m_Ctrl.at(iPoint);
        case 1:
        {
            double ctrl = m_Ctrl.at(iPoint);
            return TWSInf(ctrl);
        }
        case 2:
        {
            double ctrl = m_Ctrl.at(iPoint);
            return TWAInf(ctrl);
        }
        case 3:   return m_VInf.at(iPoint);
        case 4:   return m_Beta.at(iPoint);
        case 5:   return m_Phi.at(iPoint);
        case 6:   return m_AC.at(iPoint).CD();
        case 7:   return m_AC.at(iPoint).CDi();
        case 8:   return m_AC.at(iPoint).CDv();
        case 9:   return m_AC.at(iPoint).CSide();
        case 10:  return m_AC.at(iPoint).Cx();
        case 11:  return m_AC.at(iPoint).Cy();
        case 12:  return m_AC.at(iPoint).Cx_sum();
        case 13:  return m_AC.at(iPoint).Cy_sum();
        case 14:  return m_AC.at(iPoint).fffx()  * q * Units::NtoUnit();
        case 15:  return m_AC.at(iPoint).fffy()  * q * Units::NtoUnit();
        case 16:  return m_AC.at(iPoint).fffz()  * q * Units::NtoUnit();
        case 17:  return m_AC.at(iPoint).fsumx() * q * Units::NtoUnit();
        case 18:  return m_AC.at(iPoint).fsumy() * q * Units::NtoUnit();
        case 19:  return m_AC.at(iPoint).fsumz() * q * Units::NtoUnit();
        case 20:  return (m_AC.at(iPoint).Mi()+m_AC.at(iPoint).Mv()).x * q * Units::NmtoUnit();
        case 21:  return (m_AC.at(iPoint).Mi()+m_AC.at(iPoint).Mv()).y * q * Units::NmtoUnit();
        case 22:  return (m_AC.at(iPoint).Mi()+m_AC.at(iPoint).Mv()).z * q * Units::NmtoUnit();
    }
    return m_Ctrl.at(iPoint);
}


void BoatPolar::getProperties(QString &PolarProps, xfl::enumTextFileType filetype, bool bData) const
{
//    QString deg = DEGCHAR;
    QString deg = DEGCHAR;
    QString frontspacer("  ");
    QString strong, lenlab, masslab, speedlab, arealab;
    QString fmt  = xfl::isLocalized() ? "%L1" : "%1";
    QString fmt2 = xfl::isLocalized() ? "%L1 / %L2" : "%1 / %2";

    Units::getLengthUnitLabel(lenlab);    lenlab = " "+lenlab;
    Units::getMassUnitLabel(masslab);     masslab = " "+masslab;
    Units::getSpeedUnitLabel(speedlab);   speedlab = " "+speedlab;
    Units::getAreaUnitLabel(arealab);     arealab = " "+arealab;
    PolarProps.clear();

//    if     (isPanel4Method() && bThickSurfaces()) PolarProps += "Quad Panels/thick surfaces";
//    if     (isPanel4Method() && bThinSurfaces())  PolarProps += "Quad Panels/thin surfaces";
    if     (isPanel4Method())             PolarProps += "Quad Panels/thin surfaces";
    else if(isVLM() && isVLM1())    PolarProps += "Quad Panels/VLM1";
    else if(isVLM() && !isVLM1())   PolarProps += "Quad Panels/VLM2";
    else if(isTriUniformMethod())         PolarProps += "Triangular Panels\nUniform doublet densities";
    else if(isTriLinearMethod())          PolarProps += "Triangular Panels\nLinear doublet densities";
    PolarProps +="\n";

//    if(bThinSurfaces()) PolarProps +="Wings as thin surfaces\n";
//    else                PolarProps +="Wings as thick surfaces\n";

    if(boundaryCondition()==Polar3d::DIRICHLET)  PolarProps += "B.C. = Dirichlet\n";
    else                                     PolarProps += "B.C. = Neumann\n";

    PolarProps += "Reference dimensions:\n";
    PolarProps += QString("  area  = " + fmt).arg(m_ReferenceArea*Units::m2toUnit()) + arealab+ "\n";
    PolarProps += QString("  chord = " + fmt).arg(m_ReferenceChord*Units::mtoUnit()) + lenlab+ "\n";

    if(isViscous()) PolarProps +="Viscous analysis\n";
    else            PolarProps += "Inviscid analysis\n";
    PolarProps +="\n";

    strong = "Analysis variables:";
    PolarProps += strong + "\n";

    strong = QString("V_boat:  "+fmt2).arg(m_VBtMin*Units::mstoUnit(), 5, 'f', 1).arg(m_VBtMax*Units::mstoUnit(), 5, 'f', 1);
    PolarProps += frontspacer + strong + speedlab+ "\n";

    strong = QString("TWS_inf: "+fmt2).arg(m_TWSMin*Units::mstoUnit(), 5, 'f', 1).arg(m_TWSMax*Units::mstoUnit(), 5, 'f', 1);
    PolarProps += frontspacer + strong + speedlab+ "\n";

    strong = QString("TWA:     "+fmt2).arg(m_TWAMin, 5, 'f', 1).arg(m_TWAMax, 5, 'f', 1);
    PolarProps += frontspacer + strong + deg+"\n";

    strong = QString("Phi:     "+fmt2).arg(m_PhiMin, 5, 'f', 1).arg(m_PhiMax, 5, 'f', 1);
    PolarProps += frontspacer + strong + deg+"\n";

    strong = QString("Ry:      "+fmt2).arg(m_RyMin, 5, 'f', 1).arg(m_RyMax, 5, 'f', 1);
    PolarProps += frontspacer + strong + deg+"\n";

    for(int is=0; is<m_SailAngleMin.size();is++)
    {
        Boat *pBoat = SailObjects::boat(boatName());
        if(!pBoat) break;
        Sail *pSail = pBoat->sail(is);
        if(!pSail) break;
        strong = QString(" = "+fmt2).arg(m_SailAngleMin[is], 5, 'f', 1).arg(m_SailAngleMax[is], 5, 'f', 1);
        PolarProps += frontspacer+ pSail->name() + strong + " "+deg+"\n";
    }

    PolarProps +="\n";

    strong  = QString("CoG = ("+fmt).arg(m_CoG.x*Units::mtoUnit(),6,'f',2);
    PolarProps += strong + lenlab;

    strong  = QString("; "+fmt).arg(m_CoG.z*Units::mtoUnit(),6,'f',2);
    PolarProps += strong + lenlab + ")\n";


    PolarProps += "Fluid properties:\n";

    strong  = frontspacer + RHOCHAR + QString(" = "+fmt+ " ").arg(density()*Units::densitytoUnit(),9,'g', 5);
    strong += Units::densityUnitLabel() + "\n";
    PolarProps += strong;

    strong  = frontspacer + NUCHAR  + QString(" = "+fmt + " ").arg(viscosity()*Units::viscositytoUnit(),9,'g', 5);
    strong += Units::viscosityUnitLabel() + "\n";
    PolarProps += strong;

    if(extraDragCount())
    {
        strong = "Extra drag:\n";
        PolarProps += strong;
        for(int ix=0; ix<extraDragCount(); ix++)
        {
            if(fabs(m_ExtraDrag[ix].area())>PRECISION && fabs(m_ExtraDrag[ix].coef())>PRECISION)
            {
                PolarProps += frontspacer+m_ExtraDrag.at(ix).tag()+":";
                strong = QString(" area=" + fmt).arg(m_ExtraDrag.at(ix).area()*Units::m2toUnit());
                strong += arealab + ",  ";
                PolarProps += strong;
                strong = QString("coeff.=" + fmt).arg(m_ExtraDrag.at(ix).coef());
                PolarProps += strong + "\n";
            }
        }
        PolarProps += "\n";
    }

    if(!isVLM())
    {
        if(!bVortonWake())
        {
            strong = "Flat panel wake:\n";
            PolarProps += strong;
            strong = QString("Nb. of wake panels = %1\n").arg(NXWakePanel4());
            PolarProps += frontspacer + strong;
            strong = QString("Length             = "+fmt).arg(totalWakeLengthFactor());
            PolarProps += frontspacer + strong + " x MAC\n";
            strong = QString("Progression factor = "+fmt+"\n").arg(wakePanelFactor());
            PolarProps += frontspacer + strong;
        }
        else
        {
            strong = "Vorton wake:\n";
            double refchord = referenceChordLength();
            PolarProps += strong;
            strong = QString("Buffer wake length = " + fmt).arg(m_BufferWakeFactor*refchord*Units::mtoUnit(), 9, 'g');
            PolarProps += frontspacer + strong + Units::lengthUnitLabel() +"\n";
            strong = QString("Streamwise step    = " + fmt).arg(m_VortonL0*refchord*Units::mtoUnit(), 9, 'g');
            PolarProps += frontspacer + strong + Units::lengthUnitLabel() +"\n";
            strong = QString("Discard distance   = " + fmt).arg(m_VPWMaxLength*refchord*Units::mtoUnit(), 9, 'g');
            PolarProps += frontspacer + strong + Units::lengthUnitLabel() +"\n";
            strong = QString("Vorton core size   = " + fmt).arg(m_VortonCoreSize*refchord*Units::mtoUnit(), 9, 'g');
            PolarProps += frontspacer + strong + Units::lengthUnitLabel() +"\n";
            strong = QString("VPW iterations     = " + fmt).arg(m_VPWIterations);
            PolarProps += frontspacer + strong;
        }
    }

    strong = QString("Data points = %1\n").arg(m_Ctrl.size());
    PolarProps += "\n"+strong;

    if(!bData) return;
    QTextStream out;
    strong.clear();
    out.setString(&strong);
    exportBtPlr(out, filetype, true);
    PolarProps += "\n"+strong;
}


void BoatPolar::exportBtPlr(QTextStream &out, xfl::enumTextFileType filetype, bool bDataOnly) const
{
    QString Header, strong, str;

    if (filetype==xfl::TXT)
    {
        if(!bDataOnly)
        {
            strong ="sail7 v.xxxx\n\n";
            out << strong;

            out << m_BoatName<<"\n";
            out << m_Name<<"\n";

            Units::getSpeedUnitLabel(str);

            out << "\n";
        }


        Header = "     Ctrl        Fx         Fy         Fz         Mx         My         Mz    \n";
        out << Header;
        for (int j=0; j<m_Ctrl.size(); j++)
        {
            Vector3d drag = windDirection(0, m_Beta.at(j)) * m_AC.at(j).viscousDrag();
            strong = QString::asprintf(" %9.3f  %9.3f  %9.3f  %9.3f  %9.3f  %9.3f  %9.3f \n",
                                       m_Ctrl.at(j),
                                       m_AC.at(j).fffx()+drag.x,
                                       m_AC.at(j).fffy()+drag.y,
                                       m_AC.at(j).fffz()+drag.z,
                                       (m_AC.at(j).Mi()+m_AC.at(j).Mi()).x,
                                       (m_AC.at(j).Mi()+m_AC.at(j).Mi()).y,
                                       (m_AC.at(j).Mi()+m_AC.at(j).Mi()).z);

            out << strong;
        }
    }
    else if(filetype==xfl::CSV)
    {
        if(!bDataOnly)
        {
            strong ="version xxxxxxxxx\n\n";
            out << strong;
            out << m_BoatName<<"\n";
            out << m_Name<<"\n";
        }

        Header = "Ctrl, FFFx, FFFy, FFFz, Fx, Fy, Fz, Mx, My, Mz\n";
        out << Header;
        for (int j=0; j<m_Ctrl.size(); j++)
        {
            Vector3d drag = windDirection(0, m_Beta.at(j)) * m_AC.at(j).viscousDrag();

            strong = QString::asprintf(" %9.3f,  %9.3f,  %9.3f,  %9.3f,  %9.3f,  %9.3f,  %9.3f\n",
                                       m_Ctrl.at(j),
                                       m_AC.at(j).fffx()+drag.x,
                                       m_AC.at(j).fffy()+drag.y,
                                       m_AC.at(j).fffz()+drag.z,
                                       (m_AC.at(j).Mi()+m_AC.at(j).Mi()).x,
                                       (m_AC.at(j).Mi()+m_AC.at(j).Mi()).y,
                                       (m_AC.at(j).Mi()+m_AC.at(j).Mi()).z);

            out << strong;

        }
    }
    out << "\n\n";
}


bool BoatPolar::hasBtOpp(BoatOpp const*pBOpp) const
{
    return pBOpp->polarName()==m_Name;
}


void BoatPolar::clearData()
{
    m_Ctrl.clear();
    m_VInf.clear();
    m_Beta.clear();
    m_Phi.clear();
    m_AC.clear();
}


void BoatPolar::setVariableNames(QString const &strSpeed, QString const &strForce, QString const &strMoment)
{
    s_BtPolarVariableNames.clear();
    s_BtPolarVariableNames = QStringList({"Ctrl", "TWS (" +strSpeed+ ")", "TWA (" +DEGCHAR+ ")", "AWS (" +strSpeed+ ")", "AWA (" +DEGCHAR+ ")", PHICHAR,
                                          "CD", "CDi", "CDv", "CL",
                                          "CX", "CY", "CX_sum", "CY_sum",
                                          "FFFx ("+strForce+")", "FFFy ("+strForce+")", "FFFz ("+strForce+")",
                                          "Fx ("+strForce+")", "Fy ("+strForce+")", "Fz ("+strForce+")",
                                          "L ("+ strMoment+")", "M ("+ strMoment+")", "N ("+ strMoment+")"});
}


void BoatPolar::setSailAngleRange(int isail, double thetamin, double thetamax)
{
    if(isail>=0 && isail<sailAngleSize())
    {
        m_SailAngleMin[isail] = thetamin;
        m_SailAngleMax[isail] = thetamax;
    }
}


void BoatPolar::makeDefaultArrays() //debug only
{
    int N=0;
    m_Ctrl.resize(N);
    m_VInf.resize(N);
    m_Beta.resize(N);
    m_Phi.resize(N);
    m_AC.resize(N);

    double phi = double(rand())/double(RAND_MAX) *2.0*PI;

    for(int i=0; i<N; i++)
    {
        double di = double(i)/double(N-1)/3.0;

        m_Ctrl[i] = 100.0*cos(di*1.0*PI+phi);
        m_VInf[i] = 1.0*di+phi;
        m_Beta[i] = 32.0*cos(di*1.0*PI+phi);
        m_Phi[i]  = 25.0*cos(di*1.0*PI+phi);
    }
}


double BoatPolar::sailAngle(int iSail, double ctrl) const
{
    if(iSail<0 || iSail>=m_SailAngleMin.size()) return 0.0;
    return m_SailAngleMin.at(iSail) + ctrl * (m_SailAngleMax.at(iSail)-m_SailAngleMin.at(iSail));
}


void BoatPolar::resizeSailAngles(int newsize)
{
    if(m_SailAngleMax.size()!= newsize) m_SailAngleMax.resize(newsize);
    if(m_SailAngleMin.size()!= newsize) m_SailAngleMin.resize(newsize);
}


void BoatPolar::getBtPolarData(QString &polardata, QString const & sep) const
{
    QString strong, strange;

    strong = boatName() + "\n";
    polardata += strong;

    strong = name()+ "\n";
    polardata += strong;

    for(int in=0; in<BoatPolar::variableCount(); in++)
    {
        strange =  BoatPolar::variableName(in).replace(" ","_");
        if(in==0) strange = "     "+strange;// start with a blank space for consistency with polar data
        for(int il=strange.length(); il<11; il++) strange+=" ";
        polardata += strange+sep;
    }
    polardata += "\n";


    for(int i=0; i<m_Ctrl.count(); i++)
    {
        for(int iVar=0; iVar<BoatPolar::variableCount(); iVar++)
        {
            double pX = getVariable(iVar, i);
            strange = QString::asprintf("%11.5g", pX);
            polardata += strange+sep;
        }
        polardata += "\n";
    }
}


void BoatPolar::copy(const BoatPolar *pBoatPolar)
{
    duplicateSpec(pBoatPolar);
    m_BoatName = pBoatPolar->boatName();
    m_Name = pBoatPolar->name();

    clearData();

    m_Ctrl = pBoatPolar->m_Ctrl;
    m_Beta = pBoatPolar->m_Beta;
    m_VInf = pBoatPolar->m_VInf;
    m_Phi  = pBoatPolar->m_Phi;
    m_AC   = pBoatPolar->m_AC;
}


/**
 * Removes the data at index i of the data arrays
 * @param i the index at which the data is to be deleted
 **/
void BoatPolar::remove(int i)
{
    m_Ctrl.removeAt(i);
    m_VInf.removeAt(i);
    m_Beta.removeAt(i);
    m_Phi.removeAt(i);
    m_AC.removeAt(i);
}


void BoatPolar::insertDataPointAt(int index, bool bAfter)
{
    if(bAfter) index++;
    m_Ctrl.insert(index,0.0);
    m_VInf.insert(index,0.0);
    m_Beta.insert(index,0.0);
    m_Phi.insert(index,0.0);
    m_AC.insert(index,AeroForces());
}


double BoatPolar::windForce(double z) const
{
    if(m_WindSpline.ctrlPointCount()<1) return 1.0;

    if(z>m_WindSpline.lastCtrlPoint().y) return 1.0;
    if(z<0.0) return 0.0;

    QVector<Vector2d> pts = m_WindSpline.outputPts();

    for (int k=0; k<pts.size()-1; k++)
    {
        if (pts.at(k).y<pts.at(k+1).y  && pts.at(k).y <= z && z<=pts.at(k+1).y )
        {
            double x = (pts.at(k).x + (pts.at(k+1).x-pts.at(k).x)
                                     /(pts.at(k+1).y-pts.at(k).y)*(z-pts.at(k).y));
            return x;
        }
    }
    return 0.0;
}


void BoatPolar::trueWindSpeed(double ctrl, double z, Vector3d &VT) const
{
    if(m_WindSpline.ctrlPointCount()<1) return;
    z = std::min(m_WindSpline.lastCtrlPoint().y, z);
    z = std::max(z, 0.0);

    double tws_inf = m_TWSMin*(1-ctrl) + m_TWSMax*ctrl;
    double twa = m_TWAMin*(1-ctrl) + m_TWAMax*ctrl;;// true wind angle at high altitude in degrees

    QVector<Vector2d> const &pts = m_WindSpline.outputPts();

    for (int k=0; k<pts.size()-1; k++)
    {
        if (pts.at(k).y<pts.at(k+1).y  && pts.at(k).y <= z && z<=pts.at(k+1).y )
        {
            double amp = (pts.at(k).x + (pts.at(k+1).x-pts.at(k).x) /(pts.at(k+1).y-pts.at(k).y)*(z-pts.at(k).y));
            double Vh = amp*tws_inf;
            VT.set(Vh*cos(twa*PI/180.0), Vh*sin(twa*PI/180.0), 0.0);
            return;
        }
    }
}


/** Returns the wind angle at high altitude */
double BoatPolar::AWSInf(double ctrl) const
{
    double Vb      = m_VBtMin*(1-ctrl)+ m_VBtMax*ctrl;
    double tws_inf = m_TWSMin*(1-ctrl) + m_TWSMax*ctrl;
    double twa     = m_TWAMin*(1-ctrl) + m_TWAMax*ctrl;// true wind angle at high altitude in degrees
    double Vx = tws_inf * cos(twa*PI/180.0)+Vb;
    double Vy = tws_inf * sin(twa*PI/180.0);
    return sqrt(Vx*Vx+Vy*Vy);
}


/** Returns the wind angle at high altitude */
double BoatPolar::AWAInf(double ctrl) const
{
    double Vb      = m_VBtMin*(1-ctrl)+ m_VBtMax*ctrl;
    double tws_inf = m_TWSMin*(1-ctrl) + m_TWSMax*ctrl;
    double twa     = m_TWAMin*(1-ctrl) + m_TWAMax*ctrl;// true wind angle at high altitude in degrees
    double Vx = tws_inf * cos(twa*PI/180.0)+Vb;
    double Vy = tws_inf * sin(twa*PI/180.0);
    double beta = atan2(Vy, Vx)*180.0/PI;
    return beta;
}


/**
 * @param the control parameter
 * @param z altitude
 * @param AWS the apparent wind speed at altitude z
 */
void BoatPolar::apparentWind(double ctrl, double z, Vector3d &AWS) const
{
    if(m_WindSpline.ctrlPointCount()<1) return;
    z = std::min(m_WindSpline.lastCtrlPoint().y, z);
    z = std::max(z, 0.0);

    double Vb = boatSpeed(ctrl);

    double tws_inf = TWSInf(ctrl); // true wind speed at high altitude
    double twa_inf = TWAInf(ctrl); // true wind angle at high altitude


    Vector3d TWS(tws_inf*cos(twa_inf*PI/180.0), tws_inf*sin(twa_inf*PI/180.0), 0.0); // true wind speed

    QVector<Vector2d> pts = m_WindSpline.outputPts();

    for (int k=0; k<pts.size()-1; k++)
    {
        if (pts.at(k).y<pts.at(k+1).y  && pts.at(k).y<=z && z<=pts.at(k+1).y )
        {
            double amp = (pts.at(k).x + (pts.at(k+1).x-pts.at(k).x) /(pts.at(k+1).y-pts.at(k).y)*(z-pts.at(k).y));

            // determine TWA
            Vector3d TWS_h = TWS * amp;
            AWS.set(TWS_h.x+Vb, TWS_h.y, 0.0);
            return;
        }
    }
    // execution should never get here
    AWS.set(TWS.x+Vb, TWS.y, 0.0);
    Q_ASSERT(false);
}



