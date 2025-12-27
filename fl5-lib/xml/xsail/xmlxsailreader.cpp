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


#include <xmlxsailreader.h>

#include <sail.h>
#include <sailnurbs.h>
#include <sailspline.h>
#include <sailwing.h>
#include <bspline.h>
#include <bezierspline.h>
#include <cubicspline.h>
#include <pointspline.h>


XmlXSailReader::XmlXSailReader(QFile &file) : XflXmlReader(file)
{

}


bool XmlXSailReader::readNURBSSail(SailNurbs *pNSail, Vector3d &position, double lengthunit, double areaunit)
{
    pNSail->nurbs().clearFrames();
    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("name"),Qt::CaseInsensitive) ==0)
        {
            pNSail->setName(readElementText().trimmed().toStdString());
        }
        else if(name().toString().compare(QString("The_Style"), Qt::CaseInsensitive)==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pNSail->setTheStyle(ls);
        }
        else if (name().toString().compare(QString("Description"), Qt::CaseInsensitive)==0)
        {
            pNSail->setDescription(readElementText().trimmed().toStdString());
        }
        else if (name().compare(QString("Position"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                position.x = coordList.at(0).toDouble()*lengthunit;
                position.y = coordList.at(1).toDouble()*lengthunit;
                position.z = coordList.at(2).toDouble()*lengthunit;
            }
        }
        else if(name().compare(QString("Reference_area"), Qt::CaseInsensitive)==0)
        {
           pNSail->setRefArea(readElementText().toDouble()/areaunit);
        }
        else if(name().compare(QString("Reference_chord"), Qt::CaseInsensitive)==0)
        {
            pNSail->setRefChord(readElementText().toDouble()/lengthunit);
        }
        else if (name().compare(QString("x_panels"), Qt::CaseInsensitive)==0)
        {
            pNSail->setNXPanels(readElementText().toInt());
        }
        else if (name().compare(QString("z_panels"), Qt::CaseInsensitive)==0)
        {
            pNSail->setNZPanels(readElementText().toInt());
        }
        else if (name().compare(QString("x_panel_distribution"), Qt::CaseInsensitive)==0)
        {
            QString strPanelDist = readElementText().trimmed();
            pNSail->setXDistType(xfl::distributionType(strPanelDist.toStdString()));
        }
        else if (name().compare(QString("z_panel_distribution"), Qt::CaseInsensitive)==0)
        {
            QString strPanelDist = readElementText().trimmed();
            pNSail->setZDistType(xfl::distributionType(strPanelDist.toStdString()));
        }
        else if (name().compare(QString("NURBS"), Qt::CaseInsensitive)==0)
        {
            std::vector<int> xpanels;   // dummy arg
            readNurbs(pNSail->nurbs(), xpanels, lengthunit);
        }
        else
            skipCurrentElement();
    }
    if(!hasError())
    {
        pNSail->makeSurface();
        pNSail->makeRuledMesh(Vector3d());
    }
    return(hasError());
}


bool XmlXSailReader::readSplineSail(SailSpline *pSSail, Vector3d &position, double lengthunit, double areaunit)
{
    pSSail->clearSections();

    // make temp arrays in case the spline type is known only after the points are read
    QVector<Vector3d> positions; 
    QVector<double> ry;
    QVector<int>nzpanels;
    QVector<xfl::enumDistribution> distribs;
    QVector<int>degree;
    QVector<std::vector<Node2d>> ctrlpts;
    QVector<std::vector<double>> weights;

    while(!atEnd() && !hasError() && readNextStartElement())
    {
        if (name().toString().compare(QString("name"),Qt::CaseInsensitive) ==0)
        {
            pSSail->setName(readElementText().trimmed().toStdString());
        }
        else if(name().toString().compare(QString("The_Style"), Qt::CaseInsensitive)==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pSSail->setTheStyle(ls);
        }
        else if (name().toString().compare(QString("Description"), Qt::CaseInsensitive)==0)
        {
            pSSail->setDescription(readElementText().trimmed().toStdString());
        }
        else if (name().compare(QString("Position"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                position.x = coordList.at(0).toDouble()*lengthunit;
                position.y = coordList.at(1).toDouble()*lengthunit;
                position.z = coordList.at(2).toDouble()*lengthunit;
            }
            pSSail->setPosition(position);
        }
        else if(name().compare(QString("Reference_area"), Qt::CaseInsensitive)==0)
        {
           pSSail->setRefArea(readElementText().toDouble()/areaunit);
        }
        else if(name().compare(QString("Reference_chord"), Qt::CaseInsensitive)==0)
        {
            pSSail->setRefChord(readElementText().toDouble()/lengthunit);
        }
        else if (name().compare(QString("Type"), Qt::CaseInsensitive)==0)
        {
            QString type = readElementText();
            if     (type.compare("BSPLINE", Qt::CaseInsensitive)==0)      pSSail->setSplineType(Spline::BSPLINE);
            else if(type.compare("BEZIERSPLINE", Qt::CaseInsensitive)==0) pSSail->setSplineType(Spline::BEZIER);
            else if(type.compare("CUBICSPLINE", Qt::CaseInsensitive)==0)  pSSail->setSplineType(Spline::CUBIC);
            else if(type.compare("POINTSPLINE", Qt::CaseInsensitive)==0)  pSSail->setSplineType(Spline::POINT);
            else pSSail->setSplineType(Spline::BSPLINE); // hope for the best
        }
        else if (name().compare(QString("x_panels"), Qt::CaseInsensitive)==0) pSSail->setNXPanels(readElementText().toInt());
        else if (name().compare(QString("x_panel_distribution"), Qt::CaseInsensitive)==0)
        {
            QString strPanelDist = readElementText().trimmed();
            pSSail->setXDistType(xfl::distributionType(strPanelDist.toStdString()));
        }
        else if (name().compare(QString("z_panels"), Qt::CaseInsensitive)==0)
        {
            nzpanels.push_back(readElementText().toInt());
        }
        else if (name().compare(QString("z_panel_distribution"), Qt::CaseInsensitive)==0)
        {
            QString strPanelDist = readElementText().trimmed();
            distribs.push_back(xfl::distributionType(strPanelDist.toStdString()));
        }

         //read splines
        else if (name().compare(QString("Section"), Qt::CaseInsensitive)==0)
        {
            positions.push_back(Vector3d());
            ry.push_back(0.0);
            degree.push_back(2);
            ctrlpts.push_back(std::vector<Node2d>());
            weights.push_back(std::vector<double>());

            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                if (name().compare(QString("Position"), Qt::CaseInsensitive)==0)
                {
                    QStringList coordList = readElementText().simplified().split(",");
                    if(coordList.length()>=3)
                    {
                        double x = coordList.at(0).toDouble()*lengthunit;
                        double y = coordList.at(1).toDouble()*lengthunit;
                        double z = coordList.at(2).toDouble()*lengthunit;
                        positions.back().set(x,y,z);

                    }
                }
                else if (name().compare(QString("Ry"), Qt::CaseInsensitive)==0)
                {
                    ry.back() = readElementText().toDouble();
                }
                else if (name().compare(QString("Degree"), Qt::CaseInsensitive)==0)
                {
                    degree.back() = readElementText().toDouble();
                }
                else if (name().compare(QString("point"), Qt::CaseInsensitive)==0)
                {
                    double x=0,y=0,w=0;
                    QStringList coordList = readElementText().simplified().split(",");
                    if(coordList.length()>=3)
                    {
                        x = coordList.at(0).toDouble()*lengthunit;
                        y = coordList.at(1).toDouble()*lengthunit;
                        w = coordList.at(2).toDouble()*lengthunit;
                        ctrlpts.back().push_back({x,y});
                        weights.back().push_back(w);
                    }
                }

                else
                    skipCurrentElement();

            }
        }
        else
            skipCurrentElement();
    }

    if(!hasError())
    {
        for(int ispl=0; ispl<ctrlpts.size(); ispl++)
        {
            Spline *pSpline=nullptr;
            switch(pSSail->splineType())
            {
                default:
                case Spline::BSPLINE:       pSpline = new BSpline();        break;
                case Spline::BEZIER:  pSpline = new BezierSpline();   break;
                case Spline::CUBIC:   pSpline = new CubicSpline();    break;
                case Spline::POINT:   pSpline = new PointSpline();    break;
            }
            if(pSpline)
            {
                // fix past bugs
/*                while(nzpanels.size()<=ispl) {nzpanels.append(1);}
                while(distribs.size()<=ispl) {distribs.append(xfl::UNIFORM);}*/

                pSSail->appendSpline(pSpline, positions.at(ispl), ry.at(ispl));
                pSpline->setCtrlPoints(ctrlpts[ispl]);
                pSpline->setPointWeights(weights[ispl]);
            }
            if(pSpline->isBSpline())
            {
                BSpline *pbs = dynamic_cast<BSpline*>(pSpline);
                pbs->setDegree(degree.at(ispl));
            }
            pSpline->updateSpline();
        }

        pSSail->makeSurface();
        pSSail->makeRuledMesh(Vector3d());
    }

    return hasError();
}


bool XmlXSailReader::readWingSail(SailWing *pWSail, Vector3d &position, double lengthunit, double areaunit)
{
    pWSail->clearSections();

    while(!atEnd() && !hasError() && readNextStartElement())
    {

        if (name().toString().compare(QString("name"),Qt::CaseInsensitive) ==0)
        {
            pWSail->setName(readElementText().trimmed().toStdString());
        }
        else if(name().toString().compare(QString("The_Style"), Qt::CaseInsensitive)==0)
        {
            LineStyle ls;
            readTheStyle(ls);
            pWSail->setTheStyle(ls);
        }
        else if (name().toString().compare(QString("Description"), Qt::CaseInsensitive)==0)
        {
            pWSail->setDescription(readElementText().trimmed().toStdString());
        }
        else if (name().compare(QString("Position"), Qt::CaseInsensitive)==0)
        {
            QStringList coordList = readElementText().simplified().split(",");
            if(coordList.length()>=3)
            {
                position.x = coordList.at(0).toDouble()*lengthunit;
                position.y = coordList.at(1).toDouble()*lengthunit;
                position.z = coordList.at(2).toDouble()*lengthunit;
            }
            pWSail->setPosition(position);
        }
        else if(name().compare(QString("Reference_area"), Qt::CaseInsensitive)==0)
        {
           pWSail->setRefArea(readElementText().toDouble()/areaunit);
        }
        else if(name().compare(QString("Reference_chord"), Qt::CaseInsensitive)==0)
        {
            pWSail->setRefChord(readElementText().toDouble()/lengthunit);
        }
        //read sections
        else if (name().compare(QString("Section"), Qt::CaseInsensitive)==0)
        {
            pWSail->appendNewSection();
            WingSailSection &sec = pWSail->gaffSection();
            int iSection = pWSail->sectionCount()-1;
            while(!atEnd() && !hasError() && readNextStartElement() )
            {
                QString strange = name().toString();
                if (strange.compare(QString("Position"), Qt::CaseInsensitive)==0)
                {
                    QStringList coordList = readElementText().simplified().split(",");
                    if(coordList.length()>=3)
                    {
                        double x = coordList.at(0).toDouble()*lengthunit;
                        double y = coordList.at(1).toDouble()*lengthunit;
                        double z = coordList.at(2).toDouble()*lengthunit;
                        pWSail->setSectionPosition(iSection, Vector3d(x,y,z));
                    }
                }
                else if (strange.compare(QString("Ry"), Qt::CaseInsensitive)==0)
                {
                    pWSail->setSectionAngle(iSection, readElementText().toDouble());
                }
                else if (strange.compare(QString("FoilName"), Qt::CaseInsensitive)==0)
                {
                    sec.setFoilName(readElementText().trimmed().toStdString());
                }
                else if (strange.compare(QString("Chord"), Qt::CaseInsensitive)==0)
                {
                    sec.m_Chord = readElementText().trimmed().toDouble()*lengthunit;
                }
                else if (strange.compare(QString("Twist"), Qt::CaseInsensitive)==0)
                {
                    sec.m_Twist = readElementText().trimmed().toDouble();
                }
                else if (strange.compare(QString("NXpanels"), Qt::CaseInsensitive)==0)
                {
                    sec.setNXPanels(readElementText().trimmed().toInt());
                }
                else if (strange.compare(QString("NZpanels"), Qt::CaseInsensitive)==0)
                {
                    sec.setNZPanels(readElementText().trimmed().toInt());
                }
                else if (strange.compare(QString("x_panel_distribution"), Qt::CaseInsensitive)==0)
                {
                    QString strPanelDist = readElementText().trimmed();
                    sec.m_XPanelDist = xfl::distributionType(strPanelDist.toStdString());
                }
                else if (strange.compare(QString("z_panel_distribution"), Qt::CaseInsensitive)==0)
                {
                    QString strPanelDist = readElementText().trimmed();
                    sec.setZPanelDistType(xfl::distributionType(strPanelDist.toStdString()));
                }
                else
                    skipCurrentElement();
            }
        }
        else
            skipCurrentElement();
    }

    if(!hasError())
    {
        pWSail->makeSurface();
    }
    return hasError();
}

