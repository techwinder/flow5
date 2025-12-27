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


#include <api/plane.h>

#include "xobjects.h"

#include <api/boatopp.h>
#include <api/objects3d.h>
#include <api/planepolar.h>
#include <api/polar.h>
#include <api/planeopp.h>
#include <api/planexfl.h>
#include <api/boat.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/sailspline.h>
#include <api/sailstl.h>
#include <api/sailocc.h>
#include <api/sailwing.h>
#include <interfaces/mesh/gmesh_globals.h>

#include <core/xflcore.h>
#include <interfaces/widgets/customdlg/renamedlg.h>
#include <globals/mainframe.h>

MainFrame *Objects3d::g_pMainFrame = nullptr;


Polar *Objects3d::importXFoilPolar(QFile &txtFile, QString &logmsg)
{
    double Re(0), alpha(0), CL(0), CD(0), CDp(0), CM(0), Xt(0), Xb(0),Cpmn(0), HMom(0);
    QString FoilName;
    QString strong, strange, str;
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
    xfl::readAVLString(in, Line, strong);    // Foil Name

    FoilName = strong.right(strong.length()-22).trimmed();
//    FoilName = FoilName.trimmed();

    pPolar->setFoilName(FoilName.toStdString());

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

                //Do this the Qt way
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
    pPolar->setName(name.toStdString());


    return pPolar;
}


QStringList Objects3d::planeNames()
{
    QStringList names;
    for(int i=0; i<nPlanes(); i++)
    {
        names.push_back(QString::fromStdString(planeAt(i)->name()));
    }
    return names;
}


QStringList Objects3d::polarNames(Plane const*pPlane)
{
    QStringList names;
    if(pPlane)
    {
        for(int i=0; i<nPolars(); i++)
        {
            PlanePolar const *pPolar = plPolarAt(i);
            if(pPolar->planeName()==pPlane->name())
                names.push_back(QString::fromStdString(pPolar->name()));
        }
    }
    return names;
}

/**
 * Inserts a modified Plane object in the array, i.a.w. user instructions
 * @param pModPlane a pointer to the instance of the Plane object to be inserted
 * @return a pointer to the Plane object which was successfully inserted, false otherwise
 */
Plane * Objects3d::setModifiedPlane(Plane *pModPlane)
{
    if(!pModPlane) return nullptr;

    bool bExists = planeExists(pModPlane->name());
    if(!bExists && pModPlane->name().length())
    {
        insertPlane(pModPlane);
        return pModPlane;
    }
    int resp = 0;

    QString OldName = QString::fromStdString(pModPlane->name());

    RenameDlg renDlg(g_pMainFrame);
    renDlg.initDialog(QString::fromStdString(pModPlane->name()), planeNames(), "Enter the new name for the Plane :");

    while (bExists || pModPlane->name().length()==0)
    {
        resp = renDlg.exec();
        if(resp==QDialog::Accepted)
        {
            if (OldName == renDlg.newName()) return pModPlane;

            //Is the new name already used?
            bExists = planeExists(renDlg.newName().toStdString());

            if(!bExists)
            {
                // we have a valid name
                // rename the plane
                pModPlane->setName(renDlg.newName().toStdString());

                insertPlane(pModPlane);
                break;

            }
        }
        else if(resp ==10)
        {
            //the user wants to overwrite the old plane/wing

            Plane *pExistingPlane = plane(renDlg.newName().toStdString());
            deletePlaneResults(pExistingPlane, false);
            deletePlane(pExistingPlane);

            pModPlane->setName(renDlg.newName().toStdString());

            //place the Plane in alphabetical order in the array
            //remove the current Plane from the array
            for (int l=0; l<Objects3d::nPlanes(); l++)
            {
                Plane *pPlane = planeAt(l);
                if(pPlane == pModPlane)
                {
                    removePlaneAt(l);
                    // but don't delete it !
                    break;
                }
            }
            //and re-insert it
            bool bInserted = false;
            for (int l=0; l<nPlanes(); l++)
            {
                Plane *pOldPlane = planeAt(l);
                if(pModPlane->name().compare(pOldPlane->name())<0)
                {
                    //then insert before
                    insertPlane(l, pModPlane);
                    bInserted = true;
                    break;
                }
            }
            if(!bInserted) appendPlane(pModPlane);
            bExists = false;
        }
        else
        {
            return nullptr; //cancelled
        }
    }
    return pModPlane;
}


PlanePolar* Objects3d::insertNewPolar(PlanePolar *pNewWPolar, Plane const*pCurPlane)
{
    if(!pNewWPolar) return nullptr;

    bool bExists = true;

    //check if this WPolar is already inserted
    for(int ip=0; ip<nPolars(); ip++)
    {
         PlanePolar *pOldWPolar = plPolarAt(ip);
        if(pOldWPolar==pNewWPolar)
        {
            // already in the array, nothing to insert
            return nullptr;
        }
    }

    //make a list of existing names
    QStringList NameList;
    for(int k=0; k<nPolars(); k++)
    {
        PlanePolar *pWPolar = plPolarAt(k);
        if(pCurPlane && pWPolar->planeName()==pCurPlane->name())
            NameList.append(QString::fromStdString(pWPolar->name()));
    }

    //Is the new WPolar's name already used?
    bExists = false;
    for (int k=0; k<NameList.count(); k++)
    {
        if(pNewWPolar->name()==NameList.at(k).toStdString())
        {
            bExists = true;
            break;
        }
    }

    if(!bExists)
    {
        //just insert the WPolar in alphabetical order
        insertPlPolar(pNewWPolar);
        return pNewWPolar;
    }

    // an old object with the WPolar's name exists for this Plane, ask for a new one
    RenameDlg dlg(g_pMainFrame);
    dlg.initDialog(QString::fromStdString(pNewWPolar->name()), polarNames(pCurPlane), "Enter the Polar's new name:");
    int resp = dlg.exec();

    if(resp==10)
    {
        //user wants to overwrite an existing name
        //so find the existing PlanePolar with that name
        PlanePolar *pWPolar = nullptr;
        for(int ipb=0; ipb<nPolars(); ipb++)
        {
             PlanePolar *pOldWPolar = plPolarAt(ipb);
            if(pCurPlane && pOldWPolar->name()==dlg.newName().toStdString() &&
               pOldWPolar->planeName()==pCurPlane->name())
            {
                pWPolar = pOldWPolar;
                break;
            }
        }

        if(pWPolar)
        {
            //remove and delete its children POpps from the array
            deletePlPolar(pWPolar);
/*            for (int l=nPOpps()-1;l>=0; l--)
            {
                PlaneOpp *pPOpp = POppAt(l);
                if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->name())
                {
                    removePOppAt(l);
                    delete pPOpp;
                }
            }

            for(int ipb=0; ipb<nPolars(); ipb++)
            {
                 WPolar *pOldWPolar = wPolarAt(ipb);
                if(pOldWPolar==pWPolar)
                {
                    removeWPolarAt(ipb);
                    delete pOldWPolar;
                    break;
                }
            }*/
        }

        //room has been made, insert the new WPolar in alphabetical order
        pNewWPolar->setName(dlg.newName().toStdString());

        insertPlPolar(pNewWPolar);
        return pNewWPolar;

    }
    else if(resp==QDialog::Rejected)
    {
        return nullptr;
    }
    else if(resp==QDialog::Accepted)
    {
        //not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
        pNewWPolar->setName(dlg.newName().toStdString());

        insertPlPolar(pNewWPolar);
        return pNewWPolar;

    }
    return nullptr; //should never get here
}

/**
 * Renames the active wing or plane
 * Updates the references in child polars and oppoints
 * @param PlaneName the new name for the wing or plane
 */
void Objects3d::renamePlane(QString const &PlaneName)
{
    QString OldName;
    Plane *pPlane = plane(PlaneName.toStdString());

    if(pPlane)
    {
        OldName = QString::fromStdString(pPlane->name());
        setModifiedPlane(pPlane);

        for (int l=nPolars()-1;l>=0; l--)
        {
            PlanePolar *pWPolar = plPolarAt(l);
            if (pWPolar->planeName() == OldName.toStdString())
            {
                pWPolar->setPlaneName(pPlane->name());
            }
        }
        for (int l=nPOpps()-1;l>=0; l--)
        {
            PlaneOpp *pPOpp = POppAt(l);
            if (pPOpp->planeName() == OldName.toStdString())
            {
                pPOpp->setPlaneName(pPlane->name());
            }
        }
    }
}


void Objects3d::renamePlPolar(PlanePolar *pWPolar, Plane const *pPlane)
{
    if(!pWPolar) return;
    PlanePolar *pOldWPolar(nullptr);

    RenameDlg dlg(g_pMainFrame);
    dlg.initDialog(QString::fromStdString(pWPolar->name()), Objects3d::polarNames(pPlane), "Enter the polar's new name:");
    int resp = dlg.exec();
    if(resp==QDialog::Rejected)
    {
        return;
    }
    else if(resp==10)
    {
        //the user wants to overwrite an existing name
        if(dlg.newName().toStdString()==pWPolar->name()) return; //what's the point?

        // it's a real overwrite
        // so find and delete the existing WPolar with the new name
        for(int ipb=0; ipb<Objects3d::nPolars(); ipb++)
        {
            pOldWPolar = Objects3d::plPolarAt(ipb);
            if(pOldWPolar->name()==dlg.newName().toStdString() && pOldWPolar->planeName()==pPlane->name())
            {
                Objects3d::deletePlPolar(pOldWPolar);
                break;
            }
        }
    }

    //ready to insert
    //remove the WPolar from its current position in the array
    for (int l=0; l<Objects3d::nPolars();l++)
    {
        pOldWPolar = Objects3d::plPolarAt(l);
        if(pOldWPolar==pWPolar)
        {
            Objects3d::removePlPolarAt(l);
            break;
        }
    }

    //set the new name
    for (int l=Objects3d::nPOpps()-1;l>=0; l--)
    {
        PlaneOpp *pPOpp = Objects3d::POppAt(l);
        if (pWPolar->hasPOpp(pPOpp))
        {
            pPOpp->setPolarName(dlg.newName().toStdString());
        }
    }

    pWPolar->setName(dlg.newName().toStdString());

    Objects3d::insertPlPolar(pWPolar);
}


bool Objects3d::readVSPFoilFile(QString const &FoilFileName, Foil *pFoil)
{
    QString strong;
    QString FoilName;

    int pos(0);
    double x(0), y(0);
    double xp(0), yp(0);
    bool bRead=false;

    QFileInfo fi(FoilFileName);
    if(!fi.exists()) return false;

    QFile xFoilFile(FoilFileName);
    if(!xFoilFile.open(QIODevice::ReadOnly)) return false;

    QTextStream inStream(&xFoilFile);

    QFileInfo fileInfo(xFoilFile);

    QString fileName = fileInfo.fileName();
    int suffixLength = fileInfo.suffix().length()+1;
    fileName = fileName.left(fileName.size()-suffixLength);

    FoilName = inStream.readLine();
    pos = FoilName.length()-FoilName.lastIndexOf("/");
    FoilName = FoilName.right(pos-1);
    pos = FoilName.lastIndexOf(".dat");
    FoilName.truncate(pos);
    pFoil->setName(FoilName.toStdString());

    std::vector<Node2d> basenodes;

    bRead = true;
    xp=-9999.0;
    yp=-9999.0;
    do
    {
        strong = inStream.readLine().trimmed();
        QStringList fields = strong.split(",");
        if(fields.size()==2)
        {
            x = fields.at(0).trimmed().toDouble();
            y = fields.at(1).trimmed().toDouble();
            //add values only if the point is not coincident with the previous one
            double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
            if(dist>0.000001)
            {
                basenodes.push_back({x,y});

                xp = x;
                yp = y;
            }
        }
        else bRead = false;

    }while (bRead && !strong.isNull());

    xFoilFile.close();

/*    pFoil->m_Node.resize(pFoil->nBaseNodes());
    for(int i=0; i<pFoil->nBaseNodes(); i++)
    {
        pFoil->m_Node[i].x = pFoil->xb(i);
        pFoil->m_Node[i].y = pFoil->yb(i);
    }*/

    pFoil->setBaseNodes(basenodes);

    pFoil->initGeometry();
    return true;
}


int Objects3d::exportTriMesh(QDataStream &outStream, double scalefactor, TriMesh const &trimesh)
{
    outStream.setByteOrder(QDataStream::LittleEndian);

    /***
     *  UINT8[80] – Header
     *     UINT32 – Number of triangles
     *
     *     foreach triangle
     *     REAL32[3] – Normal vector
     *     REAL32[3] – Vertex 1
     *     REAL32[3] – Vertex 2
     *     REAL32[3] – Vertex 3
     *     UINT16 – Attribute byte count
     *     end
    */

    //    80 character header, avoid word "solid"
    // leave 1 extra character for end zero
    //                   0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

    QString strong =     "--- STL file ---                                                               ";

    xfl::writeString(outStream, strong);

    outStream << trimesh.nPanels();

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));

    for (int it=0; it<trimesh.nPanels(); it++)
    {
        Panel3 const &p3 = trimesh.panelAt(it);
        xfl::writeFloat(outStream, p3.normal().xf());
        xfl::writeFloat(outStream, p3.normal().yf());
        xfl::writeFloat(outStream, p3.normal().zf());

        if(p3.isPositiveOrientation())
        {
            xfl::writeFloat(outStream, float(p3.vertexAt(0).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(1).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(2).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).z*scalefactor));
        }
        else
        {
            xfl::writeFloat(outStream, float(p3.vertexAt(0).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(0).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(2).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(2).z*scalefactor));

            xfl::writeFloat(outStream, float(p3.vertexAt(1).x*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).y*scalefactor));
            xfl::writeFloat(outStream, float(p3.vertexAt(1).z*scalefactor));
        }

        outStream.writeRawData(buffer, 2);
    }
    return trimesh.nPanels();
}


bool Objects3d::exportMeshToSTLFile(const QString &filename, TriMesh const &trimesh, double mtounit)
{
    if(!filename.length()) return false;

    bool bBinary = true;

    QFile XFile(filename);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    if(bBinary)
    {
        QDataStream out(&XFile);
        exportTriMesh(out,mtounit, trimesh);
    }
    else
    {
//        QTextStream out(&XFile);
    }

    XFile.close();
    return true;
}


int Objects3d::exportTriangulation(QDataStream &outStream, double scalefactor, std::vector<Triangle3d> const &triangle)
{
    /***
     *  UINT8[80] – Header
     *     UINT32 – Number of triangles
     *
     *     foreach triangle
     *     REAL32[3] – Normal vector
     *     REAL32[3] – Vertex 1
     *     REAL32[3] – Vertex 2
     *     REAL32[3] – Vertex 3
     *     UINT16 – Attribute byte count
     *     end
    */

    //    80 character header, avoid word "solid"
    // leave 1 extra character for end zero
    //                   0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

    QString strong =     "--- STL file ---                                                               ";

    xfl::writeString(outStream, strong);

    outStream << int(triangle.size()); /// @todo check STL format

    short zero = 0;
    char buffer[12];
    memcpy(buffer, &zero, sizeof(short));

    for (uint it=0; it<triangle.size(); it++)
    {
        Triangle3d const & t3 = triangle.at(it);
        xfl::writeFloat(outStream, t3.normal().xf());
        xfl::writeFloat(outStream, t3.normal().yf());
        xfl::writeFloat(outStream, t3.normal().zf());

        xfl::writeFloat(outStream, float(t3.vertexAt(0).x*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(0).y*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(0).z*scalefactor));

        xfl::writeFloat(outStream, float(t3.vertexAt(1).x*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(1).y*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(1).z*scalefactor));

        xfl::writeFloat(outStream, float(t3.vertexAt(2).x*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(2).y*scalefactor));
        xfl::writeFloat(outStream, float(t3.vertexAt(2).z*scalefactor));

        outStream.writeRawData(buffer, 2);
    }
    return int(triangle.size());
}


void Objects3d::fillSectionCp3Uniform(PlaneXfl const* pPlaneXfl, PlaneOpp const*pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
{
//    qDebug()<<"istrip"<<iStrip;
    pts.clear();
    Cp.clear();

    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;
    int i3=0;
    bool bFound = false;

    std::vector<Panel3> const &panels = pPlaneXfl->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    for (i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panels.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    if(!bFound) return;

    assert(p3->isTrailing());
    do
    {
        Cp.push_back(pPOpp->Cp(p3->index()*3));
        pts.push_back(p3->CoG());
        pts.back().setNormal(p3->normal());
        if(p3->iPU()==-1) p3=nullptr;
        else              p3=panels.data()+p3->index()+1;
    }
    while (p3);
}


void Objects3d::fillSectionCp3Linear(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip,
                                   std::vector<double> &Cp, std::vector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

//    Vector3d WingLE = pPlaneXfl->wingLE(iWing);

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;

    int nxnodes = 0;
    if(pPOpp->bThickSurfaces()) nxnodes = pWing->nXPanel3()+1;
    else                        nxnodes = pWing->nXPanels()+1;

    bool bFound = false;

    std::vector<Panel3> const &panel3 = pPlaneXfl->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    int iRightNode = -1; // only used in the case of the right tip strip
    for (int i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panel3.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            iRightNode = p3->rightTrailingNode().index();
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    int iStartNode = 0;

    if(bFound)
        iStartNode = p3->leftTrailingNode().index();
    else
    {
        if(iStrip==strip)
            // case of the last right node strip
            iStartNode = iRightNode;
        else  return; // something went wrong
    }

    for(int in=iStartNode; in<iStartNode+nxnodes; in++)
    {
        Node const &nd = pPlaneXfl->node(in);
        Cp.push_back(pPOpp->nodeValue(in));
        pts.push_back(nd);
    }
}


void Objects3d::fillSectionCp4(PlaneXfl const *pPlaneXfl, PlaneOpp const *pPOpp, int iWing, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
{
    pts.clear();
    Cp.clear();
    if(iWing<0 || iWing>=pPlaneXfl->nWings()) return;

//    Vector3d WingLE = pPlaneXfl->wingLE(iWing);

    WingXfl const *pWing = pPlaneXfl->wingAt(iWing);
    WingOpp const& wopp = pPOpp->m_WingOpp.at(iWing);

    if(iStrip<0 || iStrip>pWing->nStations()) return;

    int i4=0;
    bool bFound = false;

    int coef = pPOpp->bThinSurfaces() ? 1 : 2;

    std::vector<Panel4> const &panel4 = pPlaneXfl->quadpanels();
    int strip=0;
    for (i4=0; i4<pPOpp->nPanel4(); i4++)
    {
        Panel4 const &p4 = panel4.at(pWing->firstPanel4Index() + i4);
        if(p4.isTrailing() && p4.surfacePosition()<=xfl::MIDSURFACE)
        {
            if(strip == iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    assert(strip<pWing->nStations());

    if(bFound)
    {
        for (int pp=i4; pp<i4+coef*pWing->surfaceAt(0).NXPanels(); pp++)
        {
            Panel4 const &p4 = panel4.at(pWing->firstPanel4Index() + pp);
            Cp.push_back(wopp.m_dCp[pp]);
//            pts.push_back(p4.m_CollPt-WingLE);
            pts.push_back(p4.m_CollPt);
            pts.back().setNormal(p4.normal());
        }
    }
}


void Objects3d::fillSectionCp3Uniform(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iSail<0 || iSail>=pBoat->nSails()) return;

    Sail const*pWing = pBoat->sailAt(iSail);

    if(iStrip<0 || iStrip>pWing->nStations()) return;
    int i3=0;
    bool bFound = false;

    std::vector<Panel3> const &panel3 = pBoat->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    for (i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panel3.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    if(bFound)
    {
        assert(iStrip>=0 && iStrip<pWing->nStations());
        assert(p3->isTrailing());
        do
        {
            Cp.push_back(pBtOpp->Cp(p3->index()*3));
            pts.push_back(p3->CoG());
            pts.back().setNormal(p3->normal());
            if(p3->iPU()==-1) p3=nullptr;
            else              p3=panel3.data()+p3->index()+1;
        }
        while (p3);
    }
}


void Objects3d::fillSectionCp3Linear(Boat const *pBoat, BoatOpp const *pBtOpp, int iSail, int iStrip, std::vector<double> &Cp, std::vector<Node> &pts)
{
    pts.clear();
    Cp.clear();

    if(iSail<0 || iSail>=pBoat->nSails()) return;

    Sail const*pWing = pBoat->sailAt(iSail);

    if(iStrip<0 || iStrip>pWing->nStations()) return;

    int nxnodes = 0;
    if(pBtOpp->bThickSurfaces()) nxnodes = pWing->nXPanels()*2+1;
    else                         nxnodes = pWing->nXPanels()+1;

    bool bFound = false;

    std::vector<Panel3> const &panel3 = pBoat->triPanels();

    Panel3 const *p3 = nullptr;
    int strip = 0;
    int iRightNode = -1; // only used in the case of the right tip strip
    for (int i3=0; i3<pWing->nPanel3(); i3++)
    {
        p3 = panel3.data() + pWing->firstPanel3Index() + i3;
        if(p3->isTrailing() && (p3->isBotPanel()||p3->isMidPanel()))
        {
            iRightNode = p3->rightTrailingNode().index();
            if(strip==iStrip)
            {
                bFound = true;
                break;
            }
            strip++;
        }
    }

    int iStartNode = 0;

    if(bFound)
        iStartNode = p3->leftTrailingNode().index();
    else
    {
        if(iStrip==strip)
            // case of the last right node strip
            iStartNode = iRightNode;
        else  return; // something went wrong
    }

    for(int in=iStartNode; in<iStartNode+nxnodes; in++)
    {
        Node const &nd = pBoat->node(in);
        Cp.push_back(pBtOpp->nodeValue(in));
        pts.push_back(nd);
    }
}


void Objects3d::makePlaneTriangulation(Plane *pPlane)
{
    QString logmsg;
    for(int i=0; i<pPlane->nFuse(); i++)
    {
        Fuse *pFuse =  pPlane->fuse(i);
        gmesh::makeFuseTriangulation(pFuse, logmsg);
        pFuse->saveBaseTriangulation();
    }
}


void Objects3d::makeBoatTriangulation(Boat *pBoat)
{
    QString logmsg;
    for(int i=0; i<pBoat->nHulls(); i++)
    {
        Fuse *pFuse =  pBoat->hull(i);
        gmesh::makeFuseTriangulation(pFuse, logmsg);
        pFuse->saveBaseTriangulation();
    }

    for(int i=0; i<pBoat->nSails(); i++)
    {
        Objects3d::makeSailTriangulation(pBoat->sail(i));
    }
}


// moved here to remove dependency of fl5-lib to gmsh
void Objects3d::makeSailTriangulation(Sail *pSail, int nx, int nz)
{
    SailOcc *pSailOcc = dynamic_cast<SailOcc*>(pSail);
    if(pSailOcc)
    {
        gmesh::makeSailOccTriangulation(pSailOcc);
    }
    else
        pSail->makeTriangulation(nx, nz);
}














