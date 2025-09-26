/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#define _MATH_DEFINES_DEFINED

#include <QApplication>
#include <QFile>

#include "fileio.h"

#include <xfl/editors/analysis2ddef/foilpolardlg.h>
#include <xfl/editors/analysis3ddef/planepolardlg.h>
#include <xfl/globals/mainframe.h>
#include <xfl/xdirect/xdirect.h>
#include <xflcore/flow5events.h>
#include <xflcore/saveoptions.h>
#include <xflcore/trace.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflfoil/editors/foil1splinedlg.h>
#include <xflfoil/editors/foil2splinedlg.h>
#include <xflfoil/editors/foilcamberdlg.h>
#include <xflfoil/globals/objects2d_globals.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/oppoint.h>
#include <xflfoil/objects2d/polar.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/analysis/wpolarext.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane/planestl.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/sailobjects/sailobjects.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/objects3d/analysis/boatpolar.h>
#include <xflobjects/objects3d/analysis/boatopp.h>
#include <xflobjects/sailobjects/boat.h>


// https://doc.qt.io/qt-6/qthread.html

FileIO::FileIO()
{

}


void FileIO::onLoadProject(const QString &filename)
{
    QString log;

    bool bError = false;

    QString pathname = filename;
    QFile XFile(pathname);

    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange("Could not open the file "+ filename);
        outputMessage(strange);
        emit fileLoaded(xfl::NOAPP, true);
        return;
    }

    QFileInfo fi(pathname);

    QString end = pathname.right(4).toLower();

    pathname.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

    if(end==".plr")
    {
        QVector<Foil*>foilList;
        QVector<Polar*> polarList;
        readPolarFile(XFile, foilList, polarList);
        XFile.close();

        for (int i=0; i<foilList.size(); i++)
        {
            Foil *pFoil=foilList.at(i);
            if(pFoil)
            {
                pFoil->setVisible(true); // clean up former mess
                Objects2d::insertThisFoil(pFoil);
            }
        }

        for (int i=0; i<polarList.size(); i++)
        {
            Polar*pPolar=polarList.at(i);
            Objects2d::insertPolar(pPolar);
        }

        XDirect::setCurPolar(nullptr);
        XDirect::setCurOpp(nullptr);

        outputMessage("Successfully loaded polar file\n");
        emit fileLoaded(xfl::XDIRECT, false);
    }
    else if(end==".dat")
    {
        Foil *pFoil = new Foil();
        readFoilFile(XFile, pFoil);
        XFile.close();

        if(pFoil)
        {
            pFoil->setLineWidth(Curve::defaultLineWidth());
            Objects2d::insertThisFoil(pFoil);

            outputMessage("Successfully loaded " + pFoil->name()+"\n");

            emit fileLoaded(xfl::NOAPP, false);
            return;
        }
    }
    else if(end==".xfl" || end==".fl5")
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        QDataStream ar(&XFile);

        outputMessage("Loading project file " + pathname+ "\n");

        bool bRead =  false;
        if     (end==".xfl")
        {
            bRead = serializeProjectXfl(ar, false, &PlanePolarDlg::staticWPolar());
        }
        else if(end==".fl5")
        {
            bRead = serializeProjectFl5(ar, false);
            outputMessage("\n");
        }

        XFile.close();

        QApplication::restoreOverrideCursor();

        if(!bRead)
        {
            log += "Error reading the file: "+pathname+"\n\n";
            bError = true;
        }
        else
        {
             log += "The file "+pathname+" has been read successfully\n\n";
             bError = false;
        }

        xfl::enumApp iApp(xfl::NOAPP);
        if     (SailObjects::boatCount()) iApp = xfl::XSAIL;
        else if(Objects3d::nPlanes())     iApp = xfl::XPLANE;
        else                              iApp = xfl::XDIRECT;
        outputMessage(log);

        emit fileLoaded(iApp, bError);
    }
}


bool FileIO::serializeProjectFl5(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat(0);

    if (bIsStoring)
    {
        // storing code
        ArchiveFormat = serializeProjectMetaDataFl5(ar, bIsStoring);
        if(ArchiveFormat<0) return false;

        serialize2dObjectsFl5(ar, bIsStoring, ArchiveFormat);

        serialize3dObjectsFl5(ar, bIsStoring, ArchiveFormat);

        serializeBtObjectsFl5(ar, bIsStoring);
    }
    else
    {
        // Loading code
        bool bLoad = false;
        outputMessage("   Reading project meta data... ");

        ArchiveFormat = serializeProjectMetaDataFl5(ar, bIsStoring);
        if(ArchiveFormat<0)
        {
            outputMessage(" error reading meta data\n");
            return false;
        }
        else
            outputMessage("done\n");

        outputMessage("   Reading 2d objects...\n");
        bLoad = serialize2dObjectsFl5(ar, bIsStoring, ArchiveFormat);

        if(!bLoad)
        {
            outputMessage("   error reading 2d objects... aborting\n");
            return false;
        }

        outputMessage("   Reading plane objects...\n");

        bLoad = serialize3dObjectsFl5(ar, bIsStoring, ArchiveFormat);
        if(!bLoad)
        {
            outputMessage("   error reading plane objects... aborting\n");
            return false;
        }


        outputMessage("   Reading boat objects...\n");
        serializeBtObjectsFl5(ar, false);

        if(!bLoad)
        {
            outputMessage("   error reading boat objects... aborting\n");
            return false;
        }

        if(ArchiveFormat<500750)
        {
/*            QString strange;
            Objects3d::cleanObjects(strange);
            if(strange.length())
            {
                strange = "Cleaning results:" + EOLCHAR + strange + EOLCHAR;
                outputMessage(strange);
            }*/

            Objects3d::updateWPolarstoV750();
        }

    }
    return true;
}


void FileIO::outputMessage(QString const &msg)
{
    emit displayMessage(msg);
}


int FileIO::serializeProjectMetaDataFl5(QDataStream &ar, bool bIsStoring)
{
    int nIntSpares(0), nDoubleSpares(0);
    int k(0);
    double dble(0);

    int ArchiveFormat(-1);

    if (bIsStoring)
    {
        // storing code
        // 500001: First instance of new .fl5 format
        // 500002: Added sync project file name serialization
        // 500003: Added foil design splines
        // 500004: Added STL planes
        // 500005: Added External WPolar
        // 500006: Added splines of FoilSplineDlg
        // 500750: added wpolar flap angles

        outputMessage("   Saving project meta-data\n");

        ArchiveFormat = 500750;
        ar << ArchiveFormat;

        // Save default Polar data.
        // change in v7.50: only interested in density and viscosity since the fluid is
        // assumed to be the same across all polars of this project
        FoilPolarDlg::thePolar().serializePolarFl5(ar, bIsStoring);
        PlanePolarDlg::staticWPolar().serializeFl5v750(ar, bIsStoring);

        ar << QString(); // formerly the sync project name

        Foil2SplineDlg::s_SF.serializeFl5(ar, bIsStoring);
        FoilCamberDlg::CSpline().serializeFl5(ar, bIsStoring);
        FoilCamberDlg::TSpline().serializeFl5(ar, bIsStoring);
        Foil1SplineDlg::Bspline().serializeFl5(ar, bIsStoring);
        Foil1SplineDlg::C3Spline().serializeFl5(ar, bIsStoring);

        // space allocation for the future storage of more data, without need to change the format
        ar << nIntSpares;
        k=0;
        for (int i=0; i<nIntSpares; i++) ar << k;

        ar<< nDoubleSpares;
        for (int i=0; i<nDoubleSpares; i++) ar << dble;
    }
    else
    {
        // LOADING CODE
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>501000) return -1; // failsafe

        //Load the default Polar data. Not in the Settings, since this is Project dependant
        Polar  apolar;
        WPolar aplanepolar;

        if(!apolar.serializePolarFl5(ar, bIsStoring)) return -1;

        if(ArchiveFormat<=500006)
        {
            if(!aplanepolar.serializeFl5v726(ar, bIsStoring))
                return -1;
        }
        else
        {
            if(!aplanepolar.serializeFl5v750(ar, bIsStoring))
                return -1;
        }

        // change in v7.50: only interested in density and viscosity
        PlanePolarDlg::staticWPolar().setDensity(aplanepolar.density());
        PlanePolarDlg::staticWPolar().setViscosity(aplanepolar.viscosity());

        QString strange;
        if(ArchiveFormat>=500002) ar >> strange; // formerly the sync project name

        if(ArchiveFormat>=500003)
        {
            Foil2SplineDlg::s_SF.serializeFl5(ar, bIsStoring);
            FoilCamberDlg::CSpline().serializeFl5(ar, bIsStoring);
            FoilCamberDlg::TSpline().serializeFl5(ar, bIsStoring);
        }
        if(ArchiveFormat>=500006)
        {
            Foil1SplineDlg::Bspline().serializeFl5(ar, bIsStoring);
            Foil1SplineDlg::C3Spline().serializeFl5(ar, bIsStoring);
        }

        // space allocation for the future storage of more data, without need to change the format
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> k;

        if(ArchiveFormat>=500002)
            ar >> nDoubleSpares;
        double dble=0.0;
        for (int i=0; i<nDoubleSpares; i++) ar >> dble;
    }

    return ArchiveFormat;
}


bool FileIO::serializeProjectXfl(QDataStream &ar, bool bIsStoring, WPolar *pMetaWPolar)
{
    QString strong;

    WPolar *pWPolar(nullptr);
    PlaneOpp *pPOpp(nullptr);
    PlaneXfl *pPlaneXfl(nullptr);
    Polar *pPolar(nullptr);
    OpPoint *pOpp(nullptr);

    int n=0;

    if (bIsStoring)
    {
        // used to sync airfoil data
        // storing code
        int ArchiveFormat = 200002;
        ar << ArchiveFormat;
        // 200001 : First instance of new ".xfl" format

        //Save unit data
        ar << Units::lengthUnitIndex();
        ar << Units::areaUnitIndex();
        ar << Units::weightUnitIndex();
        ar << Units::speedUnitIndex();
        ar << Units::forceUnitIndex();
        ar << Units::momentUnitIndex();

        // format 200002
        // saving WPolar full data including extra drag
        pMetaWPolar->serializeWPlrXFL(ar, true);

        // save the planes...
        ar << 0;

        // save the WPolars
        ar << 0;

        // save the Plane Opps
        ar << 0;

        // then the foils
        ar << Objects2d::nFoils();
        for(int i=0; i<Objects2d::nFoils(); i++)
        {
            Foil *pFoil = Objects2d::foil(i);
            pFoil->serializeXfl(ar, bIsStoring);
        }

        //the foil polars
        ar << Objects2d::nPolars();
        for (int i=0; i<Objects2d::nPolars();i++)
        {
            pPolar = Objects2d::polarAt(i);
            pPolar->serializePolarXFL(ar, bIsStoring);
        }

        //the oppoints
        ar << 0;

        // and the spline foil whilst we're at it
        Foil2SplineDlg::s_SF.serializeXfl(ar, bIsStoring);

        ar << Units::pressureUnitIndex();
        ar << Units::inertiaUnitIndex();
        //add provisions
        // space allocation for the future storage of more data, without need to change the format
        for (int i=2; i<20; i++) ar << 0;
        double dble=0;
        for (int i=0; i<50; i++) ar << dble;
    }
    else
    {
        // LOADING CODE

        int ArchiveFormat(0);
        ar >> ArchiveFormat;
        if(ArchiveFormat<200001 || ArchiveFormat>210000) return false;

        //Load unit data
        ar >> n; //Units::setLengthUnitIndex(n);
        ar >> n; //Units::setAreaUnitIndex(n);
        ar >> n; //Units::setWeightUnitIndex(n);
        ar >> n; //Units::setSpeedUnitIndex(n);
        ar >> n; //Units::setForceUnitIndex(n);
        ar >> n; //Units::setMomentUnitIndex(n);
        //pressure and inertia units are added later on in the provisions.

        //        Units::setUnitConversionFactors();

        //Load the default Polar data. Not in the Settings, since this is Project dependant
        if(ArchiveFormat==200001)
        {
            ar >> n;
            if(n==1)      pMetaWPolar->setType(xfl::T1POLAR);
            else if(n==2) pMetaWPolar->setType(xfl::T2POLAR);
            else if(n==4) pMetaWPolar->setType(xfl::T4POLAR);
            else if(n==5) pMetaWPolar->setType(xfl::T5POLAR);
            else if(n==7) pMetaWPolar->setType(xfl::T7POLAR);

            ar >> n;
            if     (n==1) pMetaWPolar->setAnalysisMethod(Polar3d::LLT);
            else if(n==2) pMetaWPolar->setAnalysisMethod(Polar3d::VLM2);
            else if(n==3) pMetaWPolar->setAnalysisMethod(Polar3d::QUADS);
            else if(n==4) pMetaWPolar->setAnalysisMethod(Polar3d::TRIUNIFORM);
            else if(n==5) pMetaWPolar->setAnalysisMethod(Polar3d::TRILINEAR);

            double m=0, x=0, y=0, z=0;
            ar >> m;   pMetaWPolar->setMass(m);
            ar >> pMetaWPolar->m_QInfSpec;
            ar >> x>>y>>z;
            pMetaWPolar->setCoG({x,y,z});  /** @todo wrong */

            double d=0;
            ar >> d;    pMetaWPolar->setDensity(d);
            ar >> d;    pMetaWPolar->setViscosity(d);
            ar >> d;    pMetaWPolar->setAlphaSpec(d);
            ar >> d;    pMetaWPolar->setBeta(d);

            bool b=false;
            ar >> b;  // pMetaWPolar->setTilted(b);
            ar >> b;  pMetaWPolar->setVortonWake(b);
        }
        else if(ArchiveFormat==200002) pMetaWPolar->serializeWPlrXFL(ar, false);


        // load the planes...
        // assumes all object have been deleted and the array cleared.
        ar >> n;

        QVector<PlaneXfl*> planelist;
        for(int i=0; i<n; i++)
        {
            pPlaneXfl = new PlaneXfl();
            if(pPlaneXfl->serializePlaneXFL(ar, bIsStoring))
            {
                bool bInserted = false;
                for(int k=0; k<Objects3d::nPlanes(); k++)
                {
                    Plane* pOldPlane = Objects3d::planeAt(k);
                    if(pOldPlane->name().compare(pPlaneXfl->name(), Qt::CaseInsensitive)>0)
                    {
                        Objects3d::insertPlane(k, pPlaneXfl);
                        planelist.push_back(pPlaneXfl);
                        bInserted = true;
                        break;
                    }
                }
                if(!bInserted)
                {
                    Objects3d::appendPlane(pPlaneXfl);
                    planelist.push_back(pPlaneXfl);
                }

                // force uniform x number of panel on all wings;
                QString log;
                for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                {
                    int nx = pPlaneXfl->wing(iw)->uniformizeXPanelNumber();
                    if(nx>0)
                    {
                        QString strong;
                        strong = "   Plane: " + pPlaneXfl->name() + "\n";
                        log = strong;

                        strong = QString::asprintf("      Forced Nx=%d at all sections of wing: ", nx);
                        strong += pPlaneXfl->wing(iw)->name() + "\n";
                        log = strong;
                    }
                }

                strong = "   Loaded plane: " + pPlaneXfl->name() + "\n";
            }
            else
            {
                delete pPlaneXfl;
                strong = QString::asprintf("   Error while reading the plane %d\n",i+1);
                return false;
            }
        }


        // load the WPolars
        ar >> n;
        strong = QString::asprintf("Loading %d plane polars\n", n);

        for(int i=0; i<n; i++)
        {
            pWPolar = new WPolar();
            if(pWPolar->serializeWPlrXFL(ar, bIsStoring))
            {
                Plane *pPlane = Objects3d::plane(pWPolar->planeName());
                if(!pPlane || !pPlane->isXflType()) continue;
                pPlaneXfl = dynamic_cast<PlaneXfl *>(pPlane);
                double refarea=0;
                Objects3d::appendWPolar(pWPolar);
                if(pWPolar->referenceDim()==Polar3d::PLANFORM)
                {
                    refarea = pPlaneXfl->planformArea(pWPolar->bIncludeOtherWingAreas());
                    pWPolar->setReferenceSpanLength(pPlaneXfl->planformSpan());
                    if(pPlaneXfl->hasOtherWing() && pWPolar->bIncludeOtherWingAreas())
                    {
                        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                        {
                            if(pPlaneXfl->wing(iw) && pPlaneXfl->wing(iw)->isOtherWing())
                            {
                                refarea += pPlaneXfl->wing(iw)->planformArea();
                                break;
                            }
                        }
                    }
                    pWPolar->setReferenceChordLength(pPlaneXfl->mac());
                }
                else if(pWPolar->referenceDim()==Polar3d::PROJECTED)
                {
                    refarea = pPlaneXfl->projectedArea(pWPolar->bIncludeOtherWingAreas());
                    pWPolar->setReferenceSpanLength(pPlaneXfl->projectedSpan());
                    if(pPlaneXfl->hasOtherWing() && pWPolar->bIncludeOtherWingAreas())
                    {
                        for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
                        {
                            if(pPlaneXfl->wing(iw) && pPlaneXfl->wing(iw)->isOtherWing())
                            {
                                refarea += pPlaneXfl->wing(iw)->projectedArea();
                                break;
                            }
                        }
                    }
                    pWPolar->setReferenceChordLength(pPlaneXfl->mac());
                }
                else
                {
                    refarea = pWPolar->referenceArea();
                }
                pWPolar->setReferenceArea(refarea);
                strong = "   Loaded plane polar:" + pWPolar->planeName() + " / " +pWPolar->name() + "\n";
                if(pWPolar->polarFormat()<=200013 && pWPolar->isStabilityPolar())
                {
                    strong = "      discarding stability angle gains not compatible with flow5\n";
                }
            }
            else
            {
                delete pWPolar;
                strong = QString::asprintf("   Error while reading plane polar %d\n", i+1);
                return false;
            }
        }


        // the PlaneOpps
        ar >> n;
        strong = QString::asprintf("Loading %d plane operating points\n", n);

        for(int i=0; i<n; i++)
        {
            pPOpp = new PlaneOpp();
            if(pPOpp && pPOpp->serializePOppXFL(ar, bIsStoring))
            {
                // do not keep v6 POpps: fixed four WOpps not consistent with adaptive wing count of v4;
                delete pPOpp;
                strong = QString::asprintf("   Discarding plane operating point %d not compatible with flow5\n", i+1);
            }
            else
            {
                strong = QString::asprintf("   Error while reading plane operating point %d\n",i+1);
                return false;
            }
        }


        // load the Foils
        ar >> n;
        strong = QString::asprintf("Loading %d foils\n", n);

        for(int i=0; i<n; i++)
        {
            Foil *pFoil = new Foil();
            if(pFoil->serializeXfl(ar, bIsStoring))
            {
                // delete any former foil with that name - necessary in the case of project insertion to avoid duplication
                // there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
                Foil *pOldFoil = Objects2d::foil(pFoil->name());
                if(pOldFoil) Objects2d::deleteFoil(pOldFoil);
                Objects2d::insertThisFoil(pFoil);
                strong = "   Loaded foil: " + pFoil->name() + "\n";
            }
            else
            {
                delete pFoil;
                strong = QString::asprintf("   Error while reading foil %d\n",i+1);
                return false;
            }
        }


        // load the Polars
        ar >> n;
        strong = QString::asprintf("Loading %d foil polars\n", n);

        for(int i=0; i<n; i++)
        {
            pPolar = new Polar();
            if(pPolar->serializePolarXFL(ar, bIsStoring))
            {
                Objects2d::appendPolar(pPolar);
                strong = "   Loaded foil polar " + pPolar->foilName() + " / " + pPolar->name() + "\n";
            }
            else
            {
                delete pPolar;
                strong = QString::asprintf("   Error while reading foil polar %d\n",i+1);
                return false;
            }
        }


        // OpPoints
        ar >> n;
        strong = QString::asprintf("Loading %d foil operating points\n", n);

        for(int i=0; i<n; i++)
        {
            pOpp = new OpPoint();
            if(pOpp->serializeOppXFL(ar, bIsStoring))
            {
                Objects2d::appendOpp(pOpp);
                strong = QString::asprintf("   Loaded %d foil operating points\n", i+1);
            }
            else
            {
                delete pOpp;
                strong = QString::asprintf("   Error while reading foil operating point %d\n",i+1);
                return false;
            }
        }

        // and the spline foil whilst we're at it
        Foil2SplineDlg::s_SF.serializeXfl(ar, bIsStoring);

        // space allocation
        int k=0;
        double dble=0;
        ar >> n; // Units::setPressureUnitIndex(n);
        ar >> n; // Units::setInertiaUnitIndex(n);
        ar >> n;        pMetaWPolar->setNXWakePanel4(n);
        for (int i=3; i<20; i++) ar >> k;
        ar >>dble;      pMetaWPolar->setWakePanelFactor(dble);
        ar >>dble;      pMetaWPolar->setTotalWakeLengthFactor(dble);
        for (int i=2; i<50; i++) ar >> dble;

        if(pMetaWPolar->NXWakePanel4()<1) pMetaWPolar->setNXWakePanel4(1);
        if(fabs(pMetaWPolar->wakePanelFactor())<PRECISION)
            pMetaWPolar->setWakePanelFactor(1.1);
        if(fabs(pMetaWPolar->totalWakeLengthFactor())<1.0)
            pMetaWPolar->setTotalWakeLengthFactor(100.0);
    }

    return true;
}



void FileIO::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {

//        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
//        outputMessage(pMsgEvent->msg());
    }
    else
        QObject::customEvent(pEvent);
}


void FileIO::readFoilFile(QFile &xFoilFile, Foil *pFoil)
{
    QString strong;
    QString tempStr;
    QString FoilName;

    int pos(0);
    double x(0), y(0), z(0);
    double xp(0), yp(0);
    bool bRead(false);

    QTextStream inStream(&xFoilFile);

    QFileInfo fileInfo(xFoilFile);

    QString fileName = fileInfo.fileName();
    int suffixLength = fileInfo.suffix().length()+1;
    fileName = fileName.left(fileName.size()-suffixLength);

    while(tempStr.length()==0 && !inStream.atEnd())
    {
        strong = inStream.readLine();
        pos = strong.indexOf("#",0);
        // ignore everything after # (including #)
        if(pos>0)strong.truncate(pos);
        tempStr = strong;
        tempStr.remove(" ");
        FoilName = strong;
    }

    if(!inStream.atEnd())
    {
        // FoilName contains the last comment

        if(xfl::readValues(strong,x,y,z)==2)
        {
            //there isn't a name on the first line, use the file's name
            FoilName = fileName;
            {
                pFoil->m_BaseNode.push_back({x,y});
//                pFoil->nb=1;
//                xp = x;
//                yp = y;
            }
        }
        else FoilName = strong;
        // remove fore and aft spaces
        FoilName = FoilName.trimmed();
    }

    bRead = true;
    xp=-9999.0;
    yp=-9999.0;
    do
    {
        strong = inStream.readLine();
        pos = strong.indexOf("#",0);
        // ignore everything after # (including #)
        if(pos>0)strong.truncate(pos);
        tempStr = strong;
        tempStr.remove(" ");
        if (!strong.isNull() && bRead && tempStr.length())
        {
            if(xfl::readValues(strong, x,y,z)==2)
            {
                //add values only if the point is not coincident with the previous one
                double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
                if(dist>0.000001)
                {
                    pFoil->m_BaseNode.push_back({x,y});
//                    pFoil->nb++;

                    xp = x;
                    yp = y;
                }
            }
            else bRead = false;
        }
    }while (bRead && !strong.isNull());

    pFoil->setName(FoilName);

    // Check if the foil was written clockwise or counter-clockwise
    int ip = 0;
    double area = 0.0;
    for (int i=0; i<pFoil->nBaseNodes(); i++)
    {
        if(i==pFoil->nBaseNodes()-1) ip = 0;
        else                 ip = i+1;
        area +=  0.5*(pFoil->yb(i)+pFoil->yb(ip))*(pFoil->xb(i)-pFoil->xb(ip));
    }

    if(area < 0.0)
    {
        //reverse the points order
        double xtmp, ytmp;
        for (int i=0; i<pFoil->nBaseNodes()/2; i++)
        {
            xtmp         = pFoil->xb(i);
            ytmp         = pFoil->yb(i);
            pFoil->m_BaseNode[i].x = pFoil->xb(pFoil->nBaseNodes()-i-1);
            pFoil->m_BaseNode[i].y = pFoil->yb(pFoil->nBaseNodes()-i-1);
            pFoil->m_BaseNode[pFoil->nBaseNodes()-i-1].x = xtmp;
            pFoil->m_BaseNode[pFoil->nBaseNodes()-i-1].y = ytmp;
        }
    }

    pFoil->initGeometry();
}


bool FileIO::readPolarFile(QFile &plrFile, QVector<Foil*> &foilList, QVector<Polar*> &polarList)
{
    Foil* pFoil(nullptr);
    Polar *pPolar(nullptr);
    Polar *pOldPolar(nullptr);
    int n(0), l(0);

    QDataStream ar(&plrFile);
    ar.setVersion(QDataStream::Qt_4_5);
    ar.setByteOrder(QDataStream::LittleEndian);

    ar >> n;

    if(n<100000)
    {
        // deprecated format
        return false;
    }
    else if (n >=100000 && n<200000)
    {
        //new format XFLR5 v1.99+
        //first read all available foils
        ar>>n;
        for (int i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!serializeFoil(pFoil, ar, false))
            {
                delete pFoil;
                return false;
            }
            foilList.append(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0; i<n; i++)
        {
            pPolar = new Polar();

            if (!serializePolarv6(pPolar, ar, false))
            {
                delete pPolar;
                return false;
            }
            for (l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);
                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.removeAt(l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.append(pPolar);
        }
    }
    else if (n >=500000 && n<600000)
    {
        // v7 format
        // number of foils to read
        ar>>n;
        for (int i=0;i<n; i++)
        {
            pFoil = new Foil();
            if (!pFoil->serializeFl5(ar, false))
            {
                delete pFoil;
                return false;
            }
            foilList.append(pFoil);
        }

        //next read all available polars

        ar>>n;
        for (int i=0;i<n; i++)
        {
            pPolar = new Polar();

            if (!pPolar->serializePolarFl5(ar, false))
            {
                delete pPolar;
                return false;
            }
            for (l=0; l<polarList.size(); l++)
            {
                pOldPolar = polarList.at(l);

                if (pOldPolar->foilName()  == pPolar->foilName() &&
                    pOldPolar->name() == pPolar->name())
                {
                    //just overwrite...
                    polarList.removeAt(l);
                    delete pOldPolar;
                    //... and continue to add
                }
            }
            polarList.append(pPolar);
        }
    }
    return true;
}


bool FileIO::serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
    int n(0), l(0), k(0);
    int ArchiveFormat(0);// identifies the format of the file
    float f(0);
//    double dble(0);


    if(bIsStoring)
    {
        //write variables
        n = pPolar->m_Alpha.size();

        ar << 1005; // identifies the format of the file
        // 1005: added Trim Polar parameters
        // 1004: added XCp
        // 1003: re-instated NCrit, XTopTr and XBotTr with polar
        xfl::writeString(ar, pPolar->m_FoilName);
        xfl::writeString(ar, pPolar->name());

        if     (pPolar->isFixedSpeedPolar())  ar<<1;
        else if(pPolar->isFixedLiftPolar())   ar<<2;
        else if(pPolar->isRubberChordPolar()) ar<<3;
        else if(pPolar->isFixedaoaPolar())    ar<<4;
        else if(pPolar->isControlPolar())     ar<<5;
        else                                  ar<<1;

        ar << pPolar->m_MaType << pPolar->m_ReType;
        ar << int(pPolar->Reynolds()) << float(pPolar->m_Mach);
        ar << float(pPolar->m_aoaSpec);
        ar << n << float(pPolar->m_ACrit);
        ar << float(pPolar->m_XTripTop) << float(pPolar->m_XTripBot);
        xfl::writeColor(ar, pPolar->lineColor().red(), pPolar->lineColor().green(), pPolar->lineColor().blue());

        ar << pPolar->theStyle().m_Stipple << pPolar->theStyle().m_Width;
        if (pPolar->isVisible())  ar<<1; else ar<<0;
        ar<<pPolar->pointStyle();

        for (int i=0; i< pPolar->m_Alpha.size(); i++){
            ar << float(pPolar->m_Alpha.at(i)) << float(pPolar->m_Cd.at(i)) ;
            ar << float(pPolar->m_Cdp.at(i))   << float(pPolar->m_Cl.at(i)) << float(pPolar->m_Cm.at(i));
            ar << float(pPolar->m_XTrTop.at(i))  << float(pPolar->m_XTrBot.at(i));
            ar << float(pPolar->m_HMom.at(i))  << float(pPolar->m_Cpmn.at(i));
            ar << float(pPolar->m_Re.at(i));
            ar << float(pPolar->m_XCp.at(i));
            ar << float(pPolar->m_Control.at(i));
        }

        ar << pPolar->m_ACrit << pPolar->m_XTripTop << pPolar->m_XTripBot;

/*        for(int i=0; i<pPolar->nCtrls(); i++)
        {
            ar<<dble<<dble;
        }*/

        return true;
    }
    else
    {
        //read variables
        QString strange;
        float Alpha=0, Cd(0), Cdp(0), Cl(0), Cm(0), XTr1(0), XTr2(0), HMom(0), Cpmn(0), Re(0), XCp(0);
        int iRe(0);

        ar >> ArchiveFormat;
        if (ArchiveFormat <1001 || ArchiveFormat>1100)
        {
            return false;
        }

        xfl::readString(ar, pPolar->m_FoilName);
        xfl::readString(ar, strange); pPolar->setName(strange);

        if(pPolar->m_FoilName.isEmpty() || pPolar->name().isEmpty())
        {
            return false;
        }

        ar >>k;
        if     (k==1) pPolar->m_Type = xfl::T1POLAR;
        else if(k==2) pPolar->m_Type = xfl::T2POLAR;
        else if(k==3) pPolar->m_Type = xfl::T3POLAR;
        else if(k==4) pPolar->m_Type = xfl::T4POLAR;
        else          pPolar->m_Type = xfl::T1POLAR;


        ar >> pPolar->m_MaType >> pPolar->m_ReType;

        if(pPolar->m_MaType!=1 && pPolar->m_MaType!=2 && pPolar->m_MaType!=3)
        {
            return false;
        }
        if(pPolar->m_ReType!=1 && pPolar->m_ReType!=2 && pPolar->m_ReType!=3)
        {
            return false;
        }

        ar >> iRe;
        pPolar->setReynolds(double(iRe));
        ar >> f; pPolar->m_Mach = double(f);

        ar >> f; pPolar->m_aoaSpec= double(f);

        ar >> n;
        ar >> f; pPolar->m_ACrit    = double(f);
        ar >> f; pPolar->m_XTripTop = double(f);
        ar >> f; pPolar->m_XTripBot = double(f);

        if(ArchiveFormat<1005)
        {
            int r,g,b;
            xfl::readColor(ar, r,g,b);
            pPolar->setLineColor({r,g,b});
            ar >>n;
            pPolar->setLineStipple(LineStyle::convertLineStyle(n));
            ar >> n; pPolar->setLineWidth(n);
            if(ArchiveFormat>=1002)
            {
                ar >> l;
                if(l!=0 && l!=1 )
                {
                    return false;
                }
                if (l) pPolar->setVisible(true); else pPolar->setVisible(false);
            }
            ar >> l;  pPolar->setPointStyle(LineStyle::convertSymbol(l));
        }
        else pPolar->theStyle().serializeXfl(ar, bIsStoring);

        bool bExists=false;
        for (int i=0; i< n; i++)
        {
            ar >> Alpha >> Cd >> Cdp >> Cl >> Cm;
            ar >> XTr1 >> XTr2;
            ar >> HMom >> Cpmn;

            if(ArchiveFormat >=4) ar >> Re;
            else                  Re = float(pPolar->Reynolds());

            if(ArchiveFormat>=1004) ar>> XCp;
            else                    XCp = 0.0;

            bExists = false;
            if(pPolar->m_Type!=xfl::T4POLAR)
            {
                for (int j=0; j<pPolar->m_Alpha.size(); j++)
                {
                    if(fabs(double(Alpha)-pPolar->m_Alpha.at(j))<0.001)
                    {
                        bExists = true;
                        break;
                    }
                }
            }
            else
            {
                for (int j=0; j<pPolar->m_Re.size(); j++)
                {
                    if(fabs(double(Re)-pPolar->m_Re.at(j))<0.1)
                    {
                        bExists = true;
                        break;
                    }
                }
            }
            if(!bExists)
            {
                pPolar->addPoint(double(Alpha), double(Cd), double(Cdp), double(Cl), double(Cm), double(HMom),
                                 double(Cpmn), double(Re), double(XCp), 0.0, double(XTr1), double(XTr2), 0,0,0,0);
            }
        }
        if(ArchiveFormat>=1003)
            ar >>pPolar->m_ACrit >> pPolar->m_XTripTop >> pPolar->m_XTripBot;
    }
    return true;
}


/**
 * Loads or Saves the data of this foil to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool FileIO::serializeFoil(Foil *pFoil, QDataStream &ar, bool bIsStoring)
{
    // saves or loads the foil to the archive ar

    int ArchiveFormat = 1007;
    // 1007 : saved hinge positions is absolute values rather than %
    // 1006 : QFLR5 v0.02 : added Foil description
    // 1005 : added LE Flap data
    // 1004 : added Points and Centerline property
    // 1003 : added Visible property
    // 1002 : added color and style save
    // 1001 : initial format
    int p=0, j=0;
    float f=0, ff=0;

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        xfl::writeString(ar, pFoil->name());
        xfl::writeString(ar, pFoil->description());
        ar << pFoil->theStyle().m_Stipple << pFoil->theStyle().m_Width;
        xfl::writeColor(ar, pFoil->lineColor().red(), pFoil->lineColor().green(), pFoil->lineColor().blue());

        if (pFoil->theStyle().m_bIsVisible) ar << 1; else ar << 0;
        if (pFoil->theStyle().m_Symbol>0)   ar << 1; else ar << 0;//1004
        if (pFoil->m_bCamberLine)    ar << 1; else ar << 0;//1004
        if (pFoil->m_bLEFlap)        ar << 1; else ar << 0;
        ar << float(pFoil->m_LEFlapAngle) << float(pFoil->m_LEXHinge) << float(pFoil->m_LEYHinge);
        if (pFoil->m_bTEFlap)        ar << 1; else ar << 0;
        ar << float(pFoil->m_TEFlapAngle) << float(pFoil->m_TEXHinge) << float(pFoil->m_TEYHinge);

        ar << 1.f << 1.f << 9.f;//formerly transition parameters
        ar << pFoil->nBaseNodes();
        for (int jl=0; jl<pFoil->nBaseNodes(); jl++)
        {
            ar << float(pFoil->xb(jl)) << float(pFoil->yb(jl));
        }
        ar << pFoil->nNodes();
        for (int jl=0; jl<pFoil->nNodes(); jl++)
        {
            ar << float(pFoil->x(jl)) << float(pFoil->y(jl));
        }
        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000||ArchiveFormat>1010)
            return false;

        QString strange;
        xfl::readString(ar, strange);
        pFoil->setName(strange);
        if(ArchiveFormat>=1006)
        {
            xfl::readString(ar, strange);
            pFoil->setDescription(strange);
        }
        if(ArchiveFormat>=1002)
        {
            ar >> p;
            pFoil->setLineStipple(LineStyle::convertLineStyle(p));
            ar >> p; pFoil->setLineWidth(p);
            int r=0,g=0,b=0;
            xfl::readColor(ar, r, g, b);
            pFoil->setLineColor(QColor(r,g,b));
        }
        if(ArchiveFormat>=1003)
        {
            ar >> p;
            if(p) pFoil->setVisible(true); else pFoil->setVisible(false);
        }
        if(ArchiveFormat>=1004)
        {
            ar >> p;
            pFoil->setPointStyle(LineStyle::convertSymbol(p));
            ar >> p;
            if(p) pFoil->m_bCamberLine = true; else pFoil->m_bCamberLine = false;
        }

        if(ArchiveFormat>=1005)
        {
            ar >> p;
            if (p) pFoil->m_bLEFlap = true; else pFoil->m_bLEFlap = false;
            ar >> f; pFoil->m_LEFlapAngle = double(f);
            ar >> f; pFoil->m_LEXHinge = double(f);
            ar >> f; pFoil->m_LEYHinge = double(f);
        }
        ar >> p;
        if (p) pFoil->m_bTEFlap = true; else pFoil->m_bTEFlap = false;
        ar >> f; pFoil->m_TEFlapAngle =double(f);
        ar >> f; pFoil->m_TEXHinge = double(f);
        ar >> f; pFoil->m_TEYHinge = double(f);

        if(ArchiveFormat<1007)
        {
            pFoil->m_LEXHinge /= 100.0;
            pFoil->m_LEYHinge /= 100.0;
            pFoil->m_TEXHinge /= 100.0;
            pFoil->m_TEYHinge /= 100.0;
        }

        ar >> f >> f >> f; //formerly transition parameters
        ar >> p;
//        if(pFoil->nb()>IBX) return false;

        pFoil->resizePointArrays(p);
        for (j=0; j<p; j++)
        {
            ar >> f >> ff;
            pFoil->m_BaseNode[j].x = double(f);
            pFoil->m_BaseNode[j].y = double(ff);
        }
//for(int i=0; i<pFoil->xb.size(); i++)    qDebug("  %2d  %13.7f  %13.7f", i, pFoil->xb[i], pFoil->yb[i]);

        /** @todo remove. We don't need to save/load the current foil geom
         *  since we recreate it using base geometry and flap data */
        if(ArchiveFormat>=1001)
        {
            ar >> p; //pFoil->n;
//            if(pFoil->n>IBX) return false;

            if(p>pFoil->nNodes())
            {
                pFoil->m_Node.resize(p);
//                pFoil->resizeArrays(p);
            }
            for (j=0; j<p; j++)
            {
                ar >> f >> ff;
//                pFoil->x[j]=f; pFoil->y[j]=ff;
            }
            if(pFoil->nBaseNodes()==0 && pFoil->nNodes()!=0)
            {
//                pFoil->nb = pFoil->n();
//                pFoil->xb= pFoil->x;/** @todo is this an array copy?*/
//                pFoil->yb= pFoil->y;
            }
        }
        else
        {
//            pFoil->x= pFoil->xb; /** @todo is this an array copy?*/
//            pFoil->y= pFoil->yb;
//            pFoil->n=pFoil->nb;
        }


        pFoil->initGeometry();

        return true;
    }
}



Polar *FileIO::importXFoilPolar(QFile & txtFile, QString &logmsg)
{
    double Re=0, alpha=0, CL=0, CD=0, CDp=0, CM=0, Xt=0, Xb=0,Cpmn=0, HMom=0;
    QString FoilName, strong, strange, str;
    bool bRead = false;

    if (!txtFile.open(QIODevice::ReadOnly))
    {
        strange = "Could not open the file "+txtFile.fileName();
        logmsg += strange;
        return nullptr;
    }
    Polar *pPolar = new Polar;

    QTextStream in(&txtFile);
    int Line = 0;
    bool bOK=false, bOK2=false;

    xfl::readAVLString(in, Line, strong);    // XFoil or XFLR5 version
    xfl::readAVLString(in, Line, strong);    ;// Foil Name

    FoilName = strong.right(strong.length()-22);
    FoilName = FoilName.trimmed();

    pPolar->setFoilName(FoilName);

    xfl::readAVLString(in, Line, strong);// analysis type

    int retype = strong.mid(0,2).toInt(&bOK);
    if(bOK) pPolar->setReType(retype);
    int matype = strong.mid(2,2).toInt(&bOK2);
    if(bOK) pPolar->setMaType(matype);

    if(!bOK || !bOK2)
    {
        str = QString::asprintf("Error reading line %d: Unrecognized Mach and Reynolds type.\nThe polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";

        return nullptr;
    }
    if     (pPolar->ReType() ==1 && pPolar->MaType() ==1) pPolar->setType(xfl::T1POLAR);
    else if(pPolar->ReType() ==2 && pPolar->MaType() ==2) pPolar->setType(xfl::T2POLAR);
    else if(pPolar->ReType() ==3 && pPolar->MaType() ==1) pPolar->setType(xfl::T3POLAR);
    else                                                  pPolar->setType(xfl::T1POLAR);

    bRead = xfl::readAVLString(in, Line, strong);
    if(!bRead || strong.length() < 34)
    {
        str = QString::asprintf("Error reading line %d. The polar(s) will not be stored.",Line);
        delete pPolar;

        logmsg += str+"\n";
        return nullptr;
    }

    double xtr = strong.mid(9,6).toDouble(&bOK);
    if(bOK) pPolar->setXTripBot(xtr);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Bottom Transition value at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    xtr = strong.mid(28,6).toDouble(&bOK);
    if(bOK) pPolar->setXTripTop(xtr);

    if(!bOK)
    {
        str = QString::asprintf("Error reading Top Transition value at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;

        logmsg += str+"\n";
        return nullptr;
    }

    // Mach     Re     NCrit
    bRead = xfl::readAVLString(in, Line, strong);// blank line
    if(!bRead || strong.length() < 50)
    {
        str = QString::asprintf("Error reading line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    double Ma = strong.mid(8,6).toDouble(&bOK);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Mach Number at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }
    else
        pPolar->setMach(Ma);

    Re = strong.mid(24,10).toDouble(&bOK);
    if(!bOK)
    {
        str = QString::asprintf("Error reading Reynolds Number at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }
    Re *=1000000.0;
    pPolar->setReynolds(Re);

    double ncrit = strong.mid(52,8).toDouble(&bOK);
    if(bOK) pPolar->setNCrit(ncrit);
    if(!bOK)
    {
        str = QString::asprintf("Error reading NCrit at line %d. The polar(s) will not be stored.",Line);
        delete pPolar;
        logmsg += str+"\n";
        return nullptr;
    }

    xfl::readAVLString(in, Line, strong);// column titles
    bRead = xfl::readAVLString(in, Line, strong);// underscores


    while(bRead && !in.atEnd())
    {
        bRead = xfl::readAVLString(in, Line, strong);// polar data
        if(strong.length())
        {
            if(strong.length())
            {
                //                textline = strong.toLatin1();
                //                text = textline.constData();
                //                res = sscanf(text, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &alpha, &CL, &CD, &CDp, &CM, &Xt, &Xb, &Cpmn, &HMom);

                //Do this the C++ way
                QStringList values;
#if QT_VERSION >= 0x050F00
                values = strong.split(" ", Qt::SkipEmptyParts);
#else
                values = strong.split(" ", QString::SkipEmptyParts);
#endif

                if(values.length()>=7)
                {
                    alpha  = values.at(0).toDouble();
                    CL     = values.at(1).toDouble();
                    CD     = values.at(2).toDouble();
                    CDp    = values.at(3).toDouble();
                    CM     = values.at(4).toDouble();
                    Xt     = values.at(5).toDouble();
                    Xb     = values.at(6).toDouble();

                    if(values.length() >= 9)
                    {
                        Cpmn    = values.at(7).toDouble();
                        HMom    = values.at(8).toDouble();
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, Cpmn, HMom, Re, 0, 0, Xt, Xb, 0, 0, 0, 0);
                    }
                    else
                    {
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, 0.0, 0.0,Re,0.0,0.0, Xt, Xb, 0, 0, 0, 0);

                    }
                }
            }
        }
    }
    txtFile.close();

    Re = pPolar->Reynolds()/1000000.0;
    QString name = QString("T%1_Re%2_M%3")
            .arg(pPolar->type()+1)
            .arg(Re,0,'f',2)
            .arg(pPolar->Mach(),0,'f',2);
    str = QString("_N%1").arg(pPolar->NCrit(),0,'f',1);
    name += str;
    pPolar->setName(name);


    return pPolar;
}


bool FileIO::serialize2dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    int n(0), k(0);

    int Archive2dFormat = 500750; // v750: added archive2dformat to serialization

    if (bIsStoring)
    {
        outputMessage("   Saving 2d objects\n");
        storeFoilsFl5(QVector<Foil*>(), ar, true);
    }
    else
    {
        if(ArchiveFormat != 600001 && ArchiveFormat>=500750)
            ar >> Archive2dFormat;

        // load the Foils
        QString strange;
        ar >> n;

        strange = QString::asprintf("   Reading %d foils\n", n);
        outputMessage(strange);
        trace(strange);

        for(int i=0; i<n; i++)
        {
            Foil *pFoil = new Foil();
            if(pFoil->serializeFl5(ar, bIsStoring))
            {
                // delete any former foil with that name - necessary in the case of project insertion to avoid duplication
                // there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
                Foil *pOldFoil = Objects2d::foil(pFoil->name());
                if(pOldFoil) Objects2d::deleteFoil(pOldFoil);
                Objects2d::appendFoil(pFoil);
                strange = QString::asprintf("      foil %s... loaded\n", pFoil->name().toStdString().c_str());
                outputMessage(strange);
            }
            else
            {
                strange = QString::asprintf("      error reading the foil %s... aborting\n", pFoil->name().toStdString().c_str());
                outputMessage(strange);
                trace(strange);


                delete pFoil;
                return false;
            }
        }

        // load the Polars
        ar >> n;
        strange = QString::asprintf("   Reading %d foil polars\n", n);
        outputMessage(strange);
        trace(strange);

        for(int i=0; i<n; i++)
        {
            Polar *pPolar = new Polar();
            if(pPolar->serializePolarFl5(ar, bIsStoring))
            {
                Objects2d::insertPolar(pPolar);
            }
            else
            {
                strange = QString::asprintf("      error reading the polar %s... aborting\n", pPolar->name().toStdString().c_str());
                outputMessage(strange);
                trace(strange);

                return false;
            }
        }


        // OpPoints
        ar >> n;
        strange = QString::asprintf("   Reading %d foil operating points\n", n);
        outputMessage(strange);
        trace(strange);

        for(int i=0; i<n; i++)
        {
            OpPoint *pOpp = new OpPoint();

            ar >> k;
            if(pOpp->serializeOppFl5(ar, bIsStoring))
            {
                if(pOpp->isXFoil())
                {
//                    Foil const *pFoil = Objects2d::foil(pOpp->foilName());
//                    if(pFoil) pOpp->setSurfaceNodes(pFoil->nodes());
                }

                if(!pOpp->foilName().isEmpty() && !pOpp->polarName().isEmpty()) // cleaning past errors.
                    Objects2d::appendOpp(pOpp);
                else
                    delete pOpp;
            }
            else
            {
                strange = QString::asprintf("      error reading the operating point %s... aborting\n", pOpp->name().toStdString().c_str());
                outputMessage(strange);
                trace(strange);

                delete pOpp;
                return false;
            }
        }
    }

    return true;
}


bool FileIO::storeFoilsFl5(QVector<Foil*>const &FoilSelection, QDataStream &ar, bool bAll)
{
    bool bIsStoring = true;

    int nPlr0=0;

    int Archive2dFormat = 500750; // v750: added archive2dformat to serialization

    ar <<Archive2dFormat;

    // storing code
    // the foils
    QVector<Foil*> FoilList(FoilSelection);
    if(bAll)
    {
        FoilList = Objects2d::s_oaFoil;
    }

    int nFoils = FoilList.size();
    ar << nFoils;

    outputMessage(QString::asprintf("      Saving %d foils\n", nFoils));

    for(int iFoil=0; iFoil<FoilList.size(); iFoil++)
    {
        Foil *pFoil = FoilList.at(iFoil);
        if(pFoil)
        {
            pFoil->serializeFl5(ar, bIsStoring);
            // count the associated polars
            for (int iplr=0; iplr<Objects2d::nPolars(); iplr++)
            {
                if(pFoil->name()==Objects2d::polarAt(iplr)->foilName()) nPlr0++;
            }
        }
    }

    //the foil's polars
    ar << nPlr0;
    outputMessage(QString::asprintf("      Saving %d foil polars\n", nPlr0));

    int nPlr1=0;
    for (int i=0; i<Objects2d::nPolars(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        if(pPolar)
        {
            for(int ifoil=0; ifoil<FoilList.size(); ifoil++)
            {
                Foil *pFoil = FoilList.at(ifoil);
                if(pFoil->name()==pPolar->foilName())
                {
                    pPolar->serializePolarFl5(ar, bIsStoring);
                    nPlr1++;
                    break; // next polar
                }
            }
        }
    }
    Q_ASSERT(nPlr0==nPlr1);


    //the operating points
    if(SaveOptions::bSaveOpps())
    {
        QVector<OpPoint*> opplist;
        for(int iopp=0; iopp<Objects2d::nOpPoints(); iopp++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(iopp);
            QString foilname = pOpp->foilName();
            for(int ifoil=0; ifoil<FoilList.size(); ifoil++)
            {
                if(FoilList.at(ifoil)->name()==foilname) opplist.append(pOpp);
            }
        }

        int k(0);
        ar << int(opplist.size());
        outputMessage(QString::asprintf("      Saving %d foil operating points\n", Objects2d::nOpPoints()));
        for (int iopp=0; iopp<opplist.size(); iopp++)
        {
            OpPoint *pOpp = opplist.at(iopp);
            if      (pOpp->isXFoil()) k=0;
            ar <<k;

            if(pOpp) pOpp->serializeOppFl5(ar, bIsStoring);
        }
    }
    else
    {
        outputMessage("      Preference option: Not saving the foil operating points\n");
        ar << 0;
    }

    return true;
}


bool FileIO::serialize3dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    Plane *pPlane(nullptr);
    WPolar *pWPolar(nullptr);
    PlaneOpp *pPOpp(nullptr);

    int intg=-1;

    // ArchiveFormat =500004 --> added the STL planes
    // ArchiveFormat =500005 --> added the external polars
    if(bIsStoring)
    {
        // save the planes
        outputMessage("   Saving 3d plane objects\n");

        ar << Objects3d::nPlanes();
        outputMessage(QString::asprintf("      Saving %d planes\n", Objects3d::nPlanes()));

        for (int i=0; i<Objects3d::nPlanes();i++)
        {
            pPlane = Objects3d::planeAt(i);
            if     (pPlane->isXflType()) ar << 0;
            else if(pPlane->isSTLType()) ar << 1;
            else                         ar << intg;
            if(pPlane) pPlane->serializePlaneFl5(ar, bIsStoring);
        }

        // save the WPolars
        // count the WPolars
        int nWPlr=0;
        int nWPlrExt = 0;
        for(int iplr=0; iplr<Objects3d::nPolars(); iplr++)
        {
            WPolar const *pWPolar = Objects3d::wPolarAt(iplr);
            if(pWPolar)
            {
                if(pWPolar->isExternalPolar()) nWPlrExt++;
                else                           nWPlr++;
            }
        }
        // save first the xfl type polars
        ar << nWPlr;
        outputMessage(QString::asprintf("      Saving %d plane polars\n", nWPlr));
        for (int i=0; i<Objects3d::nPolars();i++)
        {
            pWPolar = Objects3d::wPolarAt(i);
            if(pWPolar && !pWPolar->isExternalPolar()) pWPolar->serializeFl5v750(ar, bIsStoring);
        }
        // save next the xfl type polars
        ar << nWPlrExt;
        outputMessage(QString::asprintf("      Saving %d external plane polars\n", nWPlrExt));
        for (int i=0; i<Objects3d::nPolars();i++)
        {
            pWPolar = Objects3d::wPolarAt(i);
            if(pWPolar && pWPolar->isExternalPolar()) pWPolar->serializeFl5v750(ar, bIsStoring);
        }

        // the PlaneOpps
        if(SaveOptions::bSavePOpps())
        {
            outputMessage(QString::asprintf("      Saving %d plane operating points\n",  Objects3d::nPOpps()));
            ar << Objects3d::nPOpps();
            for (int i=0; i<Objects3d::nPOpps();i++)
            {
                pPOpp = Objects3d::POppAt(i);
                if(pPOpp) pPOpp->serializeFl5(ar, bIsStoring);
            }
        }
        else
        {
            outputMessage("      Preference option: Not saving the plane operating points\n");
            ar <<0;
        }
    }
    else // is loading
    {
        QString strange;
        int n(0);

        // load the planes...
        ar >> n;
        QVector<Plane*> planelist;
        if(n<=1) strange = QString::asprintf("   Reading %d plane\n", n);
        else     strange = QString::asprintf("   Reading %d planes\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            if(ArchiveFormat>=500004)
            {
                ar >> intg;
                if     (intg==0) pPlane = new PlaneXfl;
                else if(intg==1) pPlane = new PlaneSTL;
                else return false;
            }
            else pPlane = new PlaneXfl;

            if(pPlane->serializePlaneFl5(ar, bIsStoring))
            {
                strange = QString::asprintf("      finished reading plane %s\n", pPlane->name().toStdString().c_str());
                outputMessage(strange);

                bool bInserted = false;
                for(int k=0; k<Objects3d::nPlanes(); k++)
                {
                    Plane* pOldPlane = Objects3d::planeAt(k);
                    if(pOldPlane->name().compare(pPlane->name(), Qt::CaseInsensitive)>0)
                    {
                        Objects3d::insertPlane(k, pPlane);
                        planelist.push_back(pPlane);
                        bInserted = true;
                        break;
                    }
                }
                if(!bInserted)
                {
                    Objects3d::appendPlane(pPlane);
                    planelist.push_back(pPlane);
                }
            }
            else
            {
                strange = QString::asprintf("      error reading the plane %s... aborting\n", pPlane->name().toStdString().c_str());
                outputMessage(strange);

                delete pPlane;
                return false;
            }
        }

        // load the WPolars
        ar >> n;
        strange = QString::asprintf("   Reading %d plane polars\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pWPolar = new WPolar();

            bool bLoaded = false;
            if(ArchiveFormat<500750)
                bLoaded = pWPolar->serializeFl5v726(ar, bIsStoring);
            else
                bLoaded = pWPolar->serializeFl5v750(ar, bIsStoring);

            if(bLoaded)
            {
                strange = "      finished reading plane polar " + pWPolar->planeName() + " / " + pWPolar->name() + "\n";
                outputMessage(strange);

                // clean up : the project may be carrying useless WPolars due to past programming errors
                pPlane = Objects3d::plane(pWPolar->planeName());
                if(pPlane)
                {
                    Objects3d::insertWPolar(pWPolar);
                    if(pWPolar->referenceDim()==Polar3d::CUSTOM)
                    {
                    }
                    else if(pWPolar->referenceDim()==Polar3d::PLANFORM)
                    {
                        pWPolar->setReferenceSpanLength(pPlane->planformSpan());
                        pWPolar->setReferenceChordLength(pPlane->mac());
                        pWPolar->setReferenceArea(pPlane->planformArea(pWPolar->bIncludeOtherWingAreas()));
                    }
                    else if(pWPolar->referenceDim()==Polar3d::PROJECTED)
                    {
                        pWPolar->setReferenceSpanLength(pPlane->projectedSpan());
                        pWPolar->setReferenceChordLength(pPlane->mac());
                        pWPolar->setReferenceArea(pPlane->projectedArea(pWPolar->bIncludeOtherWingAreas()));
                    }
                }
                else delete pWPolar;
            }
            else
            {
                strange = QString::asprintf("      error reading plane polar %s\n", pWPolar->name().toStdString().c_str());
                outputMessage(strange);

                delete pWPolar;
                return false;
            }
        }

        if(ArchiveFormat>=500005)
        {
            //load the external polars
            ar >> n;
            for(int i=0; i<n; i++)
            {
                WPolarExt *pWPolarExt = new WPolarExt();

                bool bLoaded = false;
                if(ArchiveFormat<500750)
                    bLoaded = pWPolarExt->serializeFl5v726(ar, bIsStoring);
                else
                    bLoaded = pWPolarExt->serializeFl5v750(ar, bIsStoring);

                if(bLoaded)
                {
                    // clean up: the project may be carrying useless WPolars due to past programming errors
                    pPlane = Objects3d::plane(pWPolarExt->planeName());
                    if(pPlane)
                    {
                        Objects3d::insertWPolar(pWPolarExt);
                    }
                    else delete pWPolarExt;
                }
                else
                {
                    strange = QString::asprintf("      error reading external plane polar %s\n", pWPolarExt->name().toStdString().c_str());
                    outputMessage(strange);

                    delete pWPolarExt;
                    return false;
                }
            }
        }

        // the PlaneOpps
        ar >> n;
        strange = QString::asprintf("   Reading %d plane operating points\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pPOpp = new PlaneOpp();
            if(pPOpp->serializeFl5(ar, bIsStoring))
            {
                pPlane = Objects3d::plane(pPOpp->planeName());
                pWPolar = Objects3d::wPolar(pPlane, pPOpp->polarName());

                if(pPlane && pWPolar)
                {
                    Objects3d::insertPOpp(pPOpp);
                }
            }
            else
            {
                strange = QString::asprintf("      error reading the plane operating point %s\n", pPOpp->title(false).toStdString().c_str());
                outputMessage(strange);

                delete pPOpp;
                return false;
            }
        }
    }
    return true;
}


bool FileIO::serializeBtObjectsFl5(QDataStream &ar, bool bIsStoring)
{
    int n(0);
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    Boat *pBoat(nullptr);
    BoatPolar *pBtPolar(nullptr);
    BoatOpp *pBtOpp(nullptr);
    int ArchiveFormat = 500750;
    // 500001: up to v726
    // 500750: v7.50+

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        // save the Boats...
        ar << SailObjects::boatCount();
        for (int i=0; i<SailObjects::boatCount();i++)
        {
            pBoat = SailObjects::boat(i);
            if(pBoat) pBoat->serializeBoatFl5(ar, bIsStoring);
        }

        // save the BtPolars
        ar << SailObjects::btPolarCount();
        for (int i=0; i<SailObjects::btPolarCount();i++)
        {
            pBtPolar = SailObjects::btPolar(i);
            if(pBtPolar) pBtPolar->serializeFl5v750(ar, bIsStoring);
        }

        // the BoatOpps
        if(SaveOptions::bSaveBtOpps())
        {
            ar << SailObjects::btOppCount();
            for (int i=0; i<SailObjects::btOppCount();i++)
            {
                pBtOpp = SailObjects::btOpp(i);
                if(pBtOpp) pBtOpp->serializeBoatOppFl5(ar, bIsStoring);
            }
        }
        else ar << 0;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        QString strange;
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001||ArchiveFormat>501000) return false;
        // load the Boats...
        ar >> n;
        strange = QString::asprintf("   Reading %d boats\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pBoat = new Boat();
            if(pBoat->serializeBoatFl5(ar, bIsStoring))
            {
                bool bInserted = false;
                for(int k=0; k<SailObjects::boatCount(); k++)
                {
                    Boat* pOldBoat = SailObjects::boat(k);
                    if(pOldBoat->name().compare(pBoat->name(), Qt::CaseInsensitive)>0)
                    {
                        SailObjects::insertThisBoat(k, pBoat);
                        bInserted = true;
                        break;
                    }
                }
                if(!bInserted)
                {
                    SailObjects::appendBoat(pBoat);
                }
            }
            else
            {
                strange = QString::asprintf("      error reading boat %s\n", pBoat->name().toStdString().c_str());
                outputMessage(strange);

                return false;
            }
        }

        // load the BtPolars
        ar >> n;
        strange = QString::asprintf("   Reading %d boat polars\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pBtPolar = new BoatPolar();
            bool bLoaded(false);
            if(ArchiveFormat==500001)
                bLoaded = pBtPolar->serializeFl5v726(ar, bIsStoring);
            else
                bLoaded = pBtPolar->serializeFl5v750(ar, bIsStoring);

            if(bLoaded)
            {
                // clean up : the project may be carrying useless BPolars due to past programming errors
                pBoat = SailObjects::boat(pBtPolar->boatName());
                if(pBoat)
                {
                    SailObjects::appendBtPolar(pBtPolar);
                }
                else
                {
                }
            }
            else
            {
                strange = QString::asprintf("      error reading boat polar %s\n", pBtPolar->boatName().toStdString().c_str());
                outputMessage(strange);

                return false;
            }
        }

        // the BoatOpps
        ar >> n;
        strange = QString::asprintf("   Reading %d boat operating points\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pBtOpp = new BoatOpp();
            if(pBtOpp->serializeBoatOppFl5(ar, bIsStoring))
            {
                //just append, since POpps have been sorted when first inserted
                pBoat = SailObjects::boat(pBtOpp->boatName());
                pBtPolar = SailObjects::btPolar(pBoat, pBtOpp->polarName());

                // clean up : the project may be carrying useless BoatOpps due to past programming errors
                if(pBoat && pBtPolar) SailObjects::appendBtOpp(pBtOpp);
                {
                }
            }
            else
            {
                strange = QString::asprintf("      error reading boat operating point %s\n", pBtOpp->title(false).toStdString().c_str());
                outputMessage(strange);

                return false;
            }
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool FileIO::storePlaneFl5(Plane *pPlane, QDataStream &ar)
{
    if(!pPlane) return false;

    bool bIsStoring=true;

    // save this plane
    ar << 1; // plane count

    if     (pPlane->isXflType()) ar <<  0;
    else if(pPlane->isSTLType()) ar <<  1;
    else                         ar << -1;

    pPlane->serializePlaneFl5(ar, bIsStoring);

    // count the WPolars associated to this plane
    int nWPlr=0;
    int nWPlrExt = 0;
    for(int iplr=0; iplr<Objects3d::nPolars(); iplr++)
    {
        WPolar const *pWPolar = Objects3d::wPolarAt(iplr);
        if(pWPolar && pWPolar->planeName()==pPlane->name())
        {
            if(pWPolar->isExternalPolar()) nWPlrExt++;
            else                           nWPlr++;
        }
    }

    // save the WPolars
    ar << nWPlr;
    int nWPlr2=0;
    for (int i=0; i<Objects3d::nPolars();i++)
    {
        WPolar *pWPolar = Objects3d::wPolarAt(i);
        if(pWPolar && pWPolar->planeName()==pPlane->name() && !pWPolar->isExternalPolar())
        {
            pWPolar->serializeFl5v750(ar, bIsStoring);
            nWPlr2++;
        }
    }
    Q_ASSERT(nWPlr==nWPlr2);

    //save the external polars
    nWPlr2=0;
    ar <<nWPlrExt;
    for (int i=0; i<Objects3d::nPolars();i++)
    {
        WPolar *pWPolar = Objects3d::wPolarAt(i);
        if(pWPolar && pWPolar->planeName()==pPlane->name() && pWPolar->isExternalPolar())
        {
            pWPolar->serializeFl5v750(ar, bIsStoring);
            nWPlr2++;
        }
    }
    Q_ASSERT(nWPlrExt==nWPlr2);

    int nPOpps=0;
    ar << nPOpps;

    return true;
}

