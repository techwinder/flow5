/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/




#include <QDataStream>

#include <xflocc/occmeshparams.h>

#include <xflcore/units.h>
#include <xflcore/xflcore.h>

OccMeshParams::OccMeshParams()
{
    setDefaults();
}

void OccMeshParams::setDefaults()
{
    m_bLinDefAbs       = false;
    m_LinDeflectionAbs = 0.005;// absolute, meters
    m_LinDeflectionRel = 0.03;
    m_AngularDeviation = 15.0; // in degrees
    m_MaxElementSize   = 0.05;
}


QString OccMeshParams::listParams(QString const &prefix)
{
    QString list;
    QString strange;
    if(m_bLinDefAbs)
    {
        strange = QString::asprintf("Absolute lin. defl. = %.3f", m_LinDeflectionAbs*Units::mtoUnit());
        strange += Units::lengthUnitLabel() + "\n";
    }
    else
    {
        strange = QString::asprintf("Relative lin. defl. = %.1f", m_LinDeflectionRel*100.0);
        strange += DEGCHAR + "\n";
    }
    list += prefix + strange;

    strange = QString::asprintf("Angular deviation=%.1f", m_AngularDeviation);
    strange+= DEGCHAR+"\n";
    list += prefix + strange;

    return list;
}

void OccMeshParams::duplicate(OccMeshParams const &params)
{
    m_bLinDefAbs       = params.m_bLinDefAbs;
    m_LinDeflectionAbs = params.m_LinDeflectionAbs;// absolute, meters
    m_LinDeflectionRel = params.m_LinDeflectionRel;
    m_AngularDeviation = params.m_AngularDeviation; // in degrees
    m_MaxElementSize   = params.m_MaxElementSize;
}


void OccMeshParams::serializeParams(QDataStream &ar, bool bIsStoring)
{
    int k=0;
    double dble=0;
    int ArchiveFormat = 500001;
    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << m_bLinDefAbs;
        ar << m_LinDeflectionAbs;
        ar << m_LinDeflectionRel;
        ar << m_AngularDeviation;
        ar << dble; //m_MinElementSize;
        ar << dble; //m_MaxElementSize;
        ar << dble; //m_AutoSize;
        ar << k; //m_TreeMin;
        ar << k; //m_TreeMax;

        k=0;
        ar << k;
        ar << k;
    }
    else
    {
        ar >> ArchiveFormat;
        ar >> m_bLinDefAbs;
        ar >> m_LinDeflectionAbs;
        ar >> m_LinDeflectionRel;
        ar >> m_AngularDeviation;
        ar >> dble; //m_MinElementSize;
        ar >> m_MaxElementSize;
        ar >> dble; //m_AutoSize;
        ar >> k; //m_TreeMin;
        ar >> k; //m_TreeMax;

        ar >> k;
        ar >> k;
    }
}


void OccMeshParams::loadSettings(QSettings &settings)
{
    settings.beginGroup("OccMeshParams");
    {
        m_LinDeflectionAbs = settings.value("LinDefAbs",        m_LinDeflectionAbs).toDouble();
        m_LinDeflectionRel = settings.value("LinDefRel",        m_LinDeflectionRel).toDouble();
        m_AngularDeviation = settings.value("AngularDeviation", m_AngularDeviation).toDouble();
        m_bLinDefAbs       = settings.value("bLinDefAbs",       m_bLinDefAbs).toBool();
    }
    settings.endGroup();
}


void OccMeshParams::saveSettings(QSettings &settings)
{
    settings.beginGroup("OccMeshParams");
    {
        settings.setValue("LinDefAbs",        m_LinDeflectionAbs);
        settings.setValue("LinDefRel",        m_LinDeflectionRel);
        settings.setValue("AngularDeviation", m_AngularDeviation);
        settings.setValue("bLinDefAbs",       m_bLinDefAbs);
    }
    settings.endGroup();
}



