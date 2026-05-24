/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois

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

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <fileio.h>

#include <boat.h>
#include <boat.h>
#include <boatopp.h>
#include <boatpolar.h>
#include <foil.h>
#include <objects2d.h>
#include <objects2d_globals.h>
#include <objects3d.h>
#include <oppoint.h>
#include <planeopp.h>
#include <planepolar.h>
#include <planepolarext.h>
#include <planestl.h>
#include <planexfl.h>
#include <polar.h>
#include <sailobjects.h>
#include <splinefoil.h>
#include <units.h>

#include <serialization.h>

bool FileIO::s_bSaveOpps(false);
bool FileIO::s_bSavePOpps(true);
bool FileIO::s_bSaveBtOpps(true);

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

        emit fileLoaded(true);
        return;
    }

//    QFileInfo fi(pathname);

    QString end = pathname.right(4).toLower();

    pathname.replace(QDir::separator(), "/");

    if(end==".xfl" || end==".fl5")
    {
        QDataStream ar(&XFile);

        outputMessage("Loading project file " + pathname+ "\n");

        bool bRead =  false;
        PlanePolar  wpolar; //PlanePolarDlg::staticWPolar;
        if     (end==".xfl")
        {
            bRead = serializeProjectXfl(ar, false, &wpolar);
        }
        else if(end==".fl5")
        {
            bRead = serializeProjectFl5(ar, false);
            outputMessage("\n");
        }

        XFile.close();


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

        outputMessage(log);

        emit fileLoaded(bError);
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
        bLoad = serializeBtObjectsFl5(ar, false);

        if(!bLoad)
        {
            outputMessage("   error reading boat objects... aborting\n");
            return false;
        }

        if(ArchiveFormat<500750)
        {
            Objects3d::updatePlPolarstoV750();
        }


        if(ArchiveFormat<500754)
        {
            // 500754: removed wing tilts from AVL control sets
            for(PlanePolar *pPlPolar : Objects3d::planePolars())
            {
                pPlPolar->clearAVLCtrls();
            }
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
        // 500754: removed wing tilts from AVL control sets

        outputMessage("   Saving project meta-data\n");

        ArchiveFormat = 500754;
        ar << ArchiveFormat;

        Polar polar;
        serial::serializePolarFl5(&polar, ar, bIsStoring); //FoilPolarDlg::thePolar().serializePolarFl5(ar, bIsStoring);
        PlanePolar wpolar;
        serial::serializePlanePolarFl5v750(&wpolar, ar, bIsStoring); //PlanePolarDlg::staticWPolar().serializeFl5v750(ar, bIsStoring);

        ar << QString(); // formerly the sync project name

        // dummy parameters
        SplineFoil SF;
        serial::serializeSplineFoilFl5(&SF, ar, bIsStoring); //Foil2SplineDlg::s_SF.serializeFl5(ar, bIsStoring);
        BSpline CS, TS;
        serial::serializeBSplineFl5(&CS,ar, bIsStoring); //FoilCamberDlg::CSpline().serializeFl5(ar, bIsStoring);
        serial::serializeBSplineFl5(&TS,ar, bIsStoring); //FoilCamberDlg::TSpline().serializeFl5(ar, bIsStoring);

        BSpline BS;
        serial::serializeBSplineFl5(&BS, ar, bIsStoring); // Foil1SplineDlg::Bspline().serializeFl5(ar, bIsStoring);
        CubicSpline C3S;
        serial::serializeCubicSplineFl5(&C3S, ar, bIsStoring); //Foil1SplineDlg::C3Spline().serializeFl5(ar, bIsStoring);

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
        if((ArchiveFormat<500001) || (ArchiveFormat>501000)) return -1; // failsafe

        //Load the default Polar data. Not in the Settings, since this is Project dependant
        Polar  apolar;
        PlanePolar aplanepolar;

        if(!serial::serializePolarFl5(&apolar, ar, bIsStoring)) return -1;

        if(ArchiveFormat<=500006)
        {
            if(!serial::serializePlanePolarFl5v726(&aplanepolar, ar, bIsStoring))
                return -1;
        }
        else
        {
            if(!serial::serializePlanePolarFl5v750(&aplanepolar, ar, bIsStoring))
                return -1;
        }


        QString strange;
        if(ArchiveFormat>=500002) ar >> strange; // formerly the sync project name

        if(ArchiveFormat>=500003)
        {
            SplineFoil SF;
            serial::serializeSplineFoilFl5(&SF, ar, bIsStoring); //Foil2SplineDlg::s_SF.serializeFl5(ar, bIsStoring);
            BSpline CS, TS;
            serial::serializeBSplineFl5(&CS, ar, bIsStoring); //FoilCamberDlg::CSpline().serializeFl5(ar, bIsStoring);
            serial::serializeBSplineFl5(&TS, ar, bIsStoring); //FoilCamberDlg::TSpline().serializeFl5(ar, bIsStoring);
        }

        if(ArchiveFormat>=500006)
        {
            BSpline BS;
            serial::serializeBSplineFl5(&BS, ar, bIsStoring); // Foil1SplineDlg::Bspline().serializeFl5(ar, bIsStoring);
            CubicSpline C3S;
            serial::serializeCubicSplineFl5(&C3S, ar, bIsStoring); //Foil1SplineDlg::C3Spline().serializeFl5(ar, bIsStoring);
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


bool FileIO::serializeProjectXfl(QDataStream &ar, bool bIsStoring, PlanePolar *pMetaWPolar)
{
    QString strong;

    PlanePolar *pWPolar(nullptr);
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
        ar << Units::massUnitIndex();
        ar << Units::speedUnitIndex();
        ar << Units::forceUnitIndex();
        ar << Units::momentUnitIndex();

        // format 200002
        // saving WPolar full data including extra drag
        serial::serializePlanePolarXfl(pMetaWPolar, ar, true);

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
            serial::serializeFoilXfl(pFoil, ar, bIsStoring);
        }

        //the foil polars
        ar << Objects2d::nPolars();
        for (int i=0; i<Objects2d::nPolars();i++)
        {
            pPolar = Objects2d::polarAt(i);
            serial::serializePolarXFL(pPolar, ar, bIsStoring);
        }

        //the oppoints
        ar << 0;

        // and the spline foil whilst we're at it
        SplineFoil SF;
        serial::serializeSplineFoilXfl(&SF, ar, bIsStoring); //Foil2SplineDlg::s_SF.serializeXfl(ar, bIsStoring);

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
        if((ArchiveFormat<200001) || (ArchiveFormat>210000)) return false;

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
            if     (n==1) pMetaWPolar->setAnalysisMethod(xfl::LLT);
            else if(n==2) pMetaWPolar->setAnalysisMethod(xfl::VLM2);
            else if(n==3) pMetaWPolar->setAnalysisMethod(xfl::QUADS);
            else if(n==4) pMetaWPolar->setAnalysisMethod(xfl::TRIUNIFORM);
            else if(n==5) pMetaWPolar->setAnalysisMethod(xfl::TRILINEAR);

            double x=0, y=0, z=0;
            double d=0;
            ar >> d;   pMetaWPolar->setMass(d);
            ar >> d;   pMetaWPolar->setVelocity(d);
            ar >> x>>y>>z;
            pMetaWPolar->setCoG({x,y,z});  /** @todo wrong */

            ar >> d;    pMetaWPolar->setDensity(d);
            ar >> d;    pMetaWPolar->setViscosity(d);
            ar >> d;    pMetaWPolar->setAlphaSpec(d);
            ar >> d;    pMetaWPolar->setBeta(d);

            bool b=false;
            ar >> b;  // pMetaWPolar->setTilted(b);
            ar >> b;  pMetaWPolar->setVortonWake(b);
        }
        else if(ArchiveFormat==200002) serial::serializePlanePolarXfl(pMetaWPolar, ar, false);


        // load the planes...
        // assumes all object have been deleted and the array cleared.
        ar >> n;

        std::vector<PlaneXfl*> planelist;
        for(int i=0; i<n; i++)
        {
            pPlaneXfl = new PlaneXfl();
            if(serial::serializePlaneXflXfl(pPlaneXfl, ar, bIsStoring))
            {
                bool bInserted = false;
                for(int k=0; k<Objects3d::nPlanes(); k++)
                {
                    Plane* pOldPlane = Objects3d::planeAt(k);
                    if(pOldPlane->name().compare(pPlaneXfl->name())>0)
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
                        strong = "   Plane: " + QString::fromStdString(pPlaneXfl->name()) + "\n";
                        log = strong;

                        strong = QString::asprintf("      Forced Nx=%d at all sections of wing: ", nx);
                        strong += QString::fromStdString(pPlaneXfl->wing(iw)->name()) + "\n";
                        log = strong;
                    }
                }

                strong = "   Loaded plane: " + QString::fromStdString(pPlaneXfl->name()) + "\n";
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
            pWPolar = new PlanePolar();
            if(serial::serializePlanePolarXfl(pMetaWPolar, ar, bIsStoring))
            {
                Plane *pPlane = Objects3d::plane(pWPolar->planeName());
                if(!pPlane || !pPlane->isXflType()) continue;
                pPlaneXfl = dynamic_cast<PlaneXfl *>(pPlane);
                double refarea=0;
                Objects3d::appendPlPolar(pWPolar);
                if(pWPolar->referenceDim()==xfl::PLANFORM)
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
                else if(pWPolar->referenceDim()==xfl::PROJECTED)
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
                strong = "   Loaded plane polar:" +
                        QString::fromStdString(pWPolar->planeName()) +
                        " / " +
                        QString::fromStdString(pWPolar->name()) +
                        "\n";
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
            if(pPOpp && serial::serializePlaneOppXFL(pPOpp, ar, bIsStoring))
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
            if(serial::serializeFoilXfl(pFoil, ar, bIsStoring))
            {
                // delete any former foil with that name - necessary in the case of project insertion to avoid duplication
                // there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
                Foil *pOldFoil = Objects2d::foil(pFoil->name());
                if(pOldFoil) Objects2d::deleteFoil(pOldFoil);
                Objects2d::insertThisFoil(pFoil);
                strong = "   Loaded foil: " + QString::fromStdString(pFoil->name()) + "\n";
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
            if(serial::serializePolarXFL(pPolar, ar, bIsStoring))
            {
                Objects2d::appendPolar(pPolar);
                strong = "   Loaded foil polar " + QString::fromStdString(pPolar->foilName()) + " / " + QString::fromStdString(pPolar->name()) + "\n";
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
            if(serial::serializeOppXFL(pOpp, ar, bIsStoring))
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
        SplineFoil SF;
        serial::serializeSplineFoilXfl(&SF, ar, bIsStoring); //Foil2SplineDlg::s_SF.serializeXfl(ar, bIsStoring);


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


bool FileIO::serialize2dObjectsFl5(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
    int n(0), k(0);

    int Archive2dFormat = 500750; // v750: added archive2dformat to serialization

    if (bIsStoring)
    {
        outputMessage("   Saving 2d objects\n");
        storeFoilsFl5(std::vector<Foil*>(), ar, true);
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

        for(int i=0; i<n; i++)
        {
            Foil *pFoil = new Foil();
            if(serial::serializeFoilFl5(pFoil, ar, bIsStoring))
            {
                // delete any former foil with that name - necessary in the case of project insertion to avoid duplication
                // there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
                Foil *pOldFoil = Objects2d::foil(pFoil->name());
                if(pOldFoil) Objects2d::deleteFoil(pOldFoil);
                Objects2d::appendFoil(pFoil);
                strange = QString::asprintf("      foil %s... loaded\n", pFoil->name().c_str());
                outputMessage(strange);
            }
            else
            {
                strange = QString::asprintf("      error reading the foil %s... aborting\n", pFoil->name().c_str());
                outputMessage(strange);

                delete pFoil;
                return false;
            }
        }

        // load the Polars
        ar >> n;
        strange = QString::asprintf("   Reading %d foil polars\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            Polar *pPolar = new Polar();
            if(serial::serializePolarFl5(pPolar, ar, bIsStoring))
            {
                Objects2d::insertPolar(pPolar);
            }
            else
            {
                strange = QString::asprintf("      error reading the polar %s... aborting\n", pPolar->name().c_str());
                outputMessage(strange);

                return false;
            }
        }


        // OpPoints
        ar >> n;
        strange = QString::asprintf("   Reading %d foil operating points\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            OpPoint *pOpp = new OpPoint();

            ar >> k;
            if(serial::serializeOppFl5(pOpp,ar, bIsStoring))
            {
                if(pOpp->isXFoil())
                {
//                    Foil const *pFoil = Objects2d::foil(pOpp->foilName());
//                    if(pFoil) pOpp->setSurfaceNodes(pFoil->nodes());
                }

                if(pOpp->foilName().length()!=0 && pOpp->polarName().length()!=0) // cleaning past errors.
                {
                    Polar const *pPolar = Objects2d::polar(pOpp->foilName(), pOpp->polarName());
                    if(pPolar) pOpp->setPolarType(pPolar->type());
                    Objects2d::appendOpp(pOpp);
                }
                else
                    delete pOpp;
            }
            else
            {
                strange = QString::asprintf("      error reading the operating point %s... aborting\n", pOpp->name().c_str());
                outputMessage(strange);

                delete pOpp;
                return false;
            }
        }
    }

    return true;
}


bool FileIO::storeFoilsFl5(std::vector<Foil*>const &FoilSelection, QDataStream &ar, bool bAll)
{
    bool bIsStoring = true;

    int nPlr0=0;

    int Archive2dFormat = 500750; // v750: added archive2dformat to serialization

    ar <<Archive2dFormat;

    // storing code
    // the foils
    std::vector<Foil*> FoilList;
    if(bAll)
    {
        FoilList = Objects2d::foils();
    }
    else
    {
        for(Foil* pFoil : FoilSelection)
        {
            FoilList.push_back(pFoil);
        }
    }

    int nFoils = int(FoilList.size());
    ar << nFoils;

    outputMessage(QString::asprintf("      Saving %d foils\n", nFoils));

    for(uint iFoil=0; iFoil<FoilList.size(); iFoil++)
    {
        Foil *pFoil = FoilList.at(iFoil);
        if(pFoil)
        {
            serial::serializeFoilFl5(pFoil,ar, bIsStoring);
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
            for(uint ifoil=0; ifoil<FoilList.size(); ifoil++)
            {
                Foil *pFoil = FoilList.at(ifoil);
                if(pFoil->name()==pPolar->foilName())
                {
                    serial::serializePolarFl5(pPolar, ar, bIsStoring);
                    nPlr1++;
                    break; // next polar
                }
            }
        }
    }
    assert(nPlr0==nPlr1);


    //the operating points
    if(s_bSaveOpps)
    {
        std::vector<OpPoint*> opplist;
        for(int iopp=0; iopp<Objects2d::nOpPoints(); iopp++)
        {
            OpPoint *pOpp = Objects2d::opPointAt(iopp);
            for(uint ifoil=0; ifoil<FoilList.size(); ifoil++)
            {
                if(FoilList.at(ifoil)->name()==pOpp->foilName()) opplist.push_back(pOpp);
            }
        }

        int k(0);
        ar << int(opplist.size());
        outputMessage(QString::asprintf("      Saving %d foil operating points\n", Objects2d::nOpPoints()));
        for(uint iopp=0; iopp<opplist.size(); iopp++)
        {
            OpPoint *pOpp = opplist.at(iopp);
            if      (pOpp->isXFoil()) k=0;
            ar <<k;

            if(pOpp) serial::serializeOppFl5(pOpp, ar, bIsStoring);
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
    PlanePolar *pPlPolar(nullptr);
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
            PlaneXfl*pPlaneXfl = dynamic_cast<PlaneXfl*>(pPlane);
            PlaneSTL*pPlaneStl = dynamic_cast<PlaneSTL*>(pPlane);
            if     (pPlaneXfl) serial::serializePlaneXflFl5(pPlaneXfl, ar, bIsStoring);
            else if(pPlaneStl) serial::serializePlaneStlFl5(pPlaneStl, ar, bIsStoring);

        }

        // save the WPolars
        // count the WPolars
        int nWPlr=0;
        int nWPlrExt = 0;
        for(int iplr=0; iplr<Objects3d::nPolars(); iplr++)
        {
            PlanePolar const *pWPolar = Objects3d::plPolarAt(iplr);
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
            pPlPolar = Objects3d::plPolarAt(i);
            if(pPlPolar && !pPlPolar->isExternalPolar()) serial::serializePlanePolarFl5v750(pPlPolar, ar, bIsStoring);
        }
        // save next the xfl type polars
        ar << nWPlrExt;
        outputMessage(QString::asprintf("      Saving %d external plane polars\n", nWPlrExt));
        for (int i=0; i<Objects3d::nPolars();i++)
        {
            PlanePolar *pPlPolar = Objects3d::plPolarAt(i);
            PlanePolarExt *pPlPolarExt = dynamic_cast<PlanePolarExt*>(pPlPolar);
            if(pPlPolarExt)
            {
                serial::serializePlanePolarExtFl5v750(pPlPolarExt, ar, bIsStoring);
            }
        }

        // the PlaneOpps
        if(s_bSavePOpps)
        {
            outputMessage(QString::asprintf("      Saving %d plane operating points\n",  Objects3d::nPOpps()));
            ar << Objects3d::nPOpps();
            for (int i=0; i<Objects3d::nPOpps();i++)
            {
                pPOpp = Objects3d::POppAt(i);
                if(pPOpp) serial::serializePlaneOppFl5(pPOpp, ar, bIsStoring);
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

            if(serial::serializePlaneFl5(pPlane, ar, bIsStoring))
            {
                strange = QString::asprintf("      finished reading plane %s\n", pPlane->name().c_str());
                outputMessage(strange);

                Objects3d::insertPlane(pPlane);
            }
            else
            {
                strange = QString::asprintf("      error reading the plane %s... aborting\n", pPlane->name().c_str());
                outputMessage(strange);

                delete pPlane;
                return false;
            }
        }

        // load the PlanePolars
        ar >> n;
        strange = QString::asprintf("   Reading %d plane polars\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pPlPolar = new PlanePolar();

            bool bLoaded = false;
            if(ArchiveFormat<500750)
                bLoaded = serial::serializePlanePolarFl5v726(pPlPolar, ar, bIsStoring);
            else
                bLoaded = serial::serializePlanePolarFl5v750(pPlPolar, ar, bIsStoring);

            if(bLoaded)
            {
                strange = "      finished reading plane polar " + QString::fromStdString(pPlPolar->planeName()) + " / " + QString::fromStdString(pPlPolar->name()) + "\n";
                outputMessage(strange);

                // clean up : the project may be carrying useless WPolars due to past programming errors
                pPlane = Objects3d::plane(pPlPolar->planeName());
                if(pPlane)
                {
                    Objects3d::insertPlPolar(pPlPolar);
                    if(pPlPolar->referenceDim()==xfl::CUSTOM)
                    {
                    }
                    else if(pPlPolar->referenceDim()==xfl::PLANFORM)
                    {
                        pPlPolar->setReferenceSpanLength(pPlane->planformSpan());
                        pPlPolar->setReferenceChordLength(pPlane->mac());
                        pPlPolar->setReferenceArea(pPlane->planformArea(pPlPolar->bIncludeOtherWingAreas()));
                    }
                    else if(pPlPolar->referenceDim()==xfl::PROJECTED)
                    {
                        pPlPolar->setReferenceSpanLength(pPlane->projectedSpan());
                        pPlPolar->setReferenceChordLength(pPlane->mac());
                        pPlPolar->setReferenceArea(pPlane->projectedArea(pPlPolar->bIncludeOtherWingAreas()));
                    }
                }
                else delete pPlPolar;
            }
            else
            {
                strange = QString::asprintf("      error reading plane polar %s\n", pPlPolar->name().c_str());
                outputMessage(strange);

                delete pPlPolar;
                return false;
            }
        }

        if(ArchiveFormat>=500005)
        {
            //load the external polars
            ar >> n;
            for(int i=0; i<n; i++)
            {
                PlanePolarExt *pPlPolarExt = new PlanePolarExt();

                bool bLoaded = false;
                if(ArchiveFormat<500750)
                    bLoaded = serial::serializePlanePolarExtFl5v726(pPlPolarExt, ar, bIsStoring);
                else
                    bLoaded = serial::serializePlanePolarExtFl5v750(pPlPolarExt, ar, bIsStoring);

                if(bLoaded)
                {
                    // clean up: the project may be carrying useless WPolars due to past programming errors
                    pPlane = Objects3d::plane(pPlPolarExt->planeName());
                    if(pPlane)
                    {
                        Objects3d::insertPlPolar(pPlPolarExt);
                    }
                    else delete pPlPolarExt;
                }
                else
                {
                    strange = QString::asprintf("      error reading external plane polar %s\n", pPlPolarExt->name().c_str());
                    outputMessage(strange);

                    delete pPlPolarExt;
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
            if(serial::serializePlaneOppFl5(pPOpp, ar, bIsStoring))
            {
                pPlane = Objects3d::plane(pPOpp->planeName());
                pPlPolar = Objects3d::wPolar(pPlane, pPOpp->polarName());

                if(pPlane && pPlPolar)
                {
                    Objects3d::insertPlaneOpp(pPOpp);
                }
            }
            else
            {
                strange = QString::asprintf("      error reading the plane operating point %s\n", pPOpp->title(false).c_str());
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
        ar << SailObjects::nBoats();
        for (int i=0; i<SailObjects::nBoats();i++)
        {
            pBoat = SailObjects::boat(i);
            if(pBoat) serial::serializeBoatFl5(pBoat, ar, bIsStoring);
        }

        // save the BtPolars
        ar << SailObjects::nBtPolars();
        for (int i=0; i<SailObjects::nBtPolars();i++)
        {
            pBtPolar = SailObjects::btPolar(i);
            if(pBtPolar) serial::serializeBoatPolarFl5v750(pBtPolar, ar, bIsStoring);
        }


        // the BoatOpps
        if(s_bSaveBtOpps)
        {
            ar << SailObjects::nBtOpps();
            for (int i=0; i<SailObjects::nBtOpps();i++)
            {
                pBtOpp = SailObjects::btOpp(i);
                if(pBtOpp) serial::serializeBoatOppFl5(pBtOpp, ar, bIsStoring);
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
        if((ArchiveFormat<500001) || (ArchiveFormat>501000)) return false;
        // load the Boats...
        ar >> n;
        strange = QString::asprintf("   Reading %d boats\n", n);
        outputMessage(strange);

        for(int i=0; i<n; i++)
        {
            pBoat = new Boat();
            if(serial::serializeBoatFl5(pBoat, ar, bIsStoring))
            {
                bool bInserted = false;
                for(int k=0; k<SailObjects::nBoats(); k++)
                {
                    Boat* pOldBoat = SailObjects::boat(k);
                    if(pOldBoat->name().compare(pBoat->name())>0)
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
                strange = QString::asprintf("      error reading boat %s\n", pBoat->name().c_str());
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
                bLoaded = serial::serializeBoatPolarFl5v726(pBtPolar, ar, bIsStoring);
            else
                bLoaded = serial::serializeBoatPolarFl5v750(pBtPolar, ar, bIsStoring);

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
                strange = QString::asprintf("      error reading boat polar %s\n", pBtPolar->boatName().c_str());
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
            if(serial::serializeBoatOppFl5(pBtOpp, ar, bIsStoring))
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
                strange = QString::asprintf("      error reading boat operating point %s\n", pBtOpp->title(false).c_str());
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

    serial::serializePlaneFl5(pPlane, ar, bIsStoring);

    // count the WPolars associated to this plane
    int nWPlr=0;
    int nWPlrExt = 0;
    for(int iplr=0; iplr<Objects3d::nPolars(); iplr++)
    {
        PlanePolar const *pWPolar = Objects3d::plPolarAt(iplr);
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
        PlanePolar *pWPolar = Objects3d::plPolarAt(i);
        if(pWPolar && pWPolar->planeName()==pPlane->name() && !pWPolar->isExternalPolar())
        {
            serial::serializePlanePolarFl5v750(pWPolar, ar, bIsStoring);
            nWPlr2++;
        }
    }
    assert(nWPlr==nWPlr2);

    //save the external polars
    nWPlr2=0;
    ar <<nWPlrExt;
    for (int i=0; i<Objects3d::nPolars();i++)
    {
        PlanePolar *pWPolar = Objects3d::plPolarAt(i);
        PlanePolarExt *pWPolarExt = dynamic_cast<PlanePolarExt*>(pWPolar);
        if(pWPolarExt && pWPolarExt->planeName()==pPlane->name())
        {
            serial::serializePlanePolarFl5v750(pWPolarExt, ar, bIsStoring);
            nWPlr2++;
        }
    }
    assert(nWPlrExt==nWPlr2);

    int nPOpps=0;
    ar << nPOpps;

    return true;
}

