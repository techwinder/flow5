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


#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>


#include "shapefixerdlg.h"
#include <api/fuse.h>
#include <api/occ_globals.h>
#include <api/units.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>


QByteArray ShapeFixerDlg::s_Geometry;
double ShapeFixerDlg::s_Precision = 1.e-4; //no idea
double ShapeFixerDlg::s_MinTolerance = 1.e-4; //no idea
double ShapeFixerDlg::s_MaxTolerance = 1.e-3; //no idea


ShapeFixerDlg::ShapeFixerDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Shape fixer"));
    setupLayout();
    connectSignals();
}


void ShapeFixerDlg::setupLayout()
{
    QGridLayout *pFixerLayout = new QGridLayout;
    {
        QLabel *pLabPrec   = new QLabel(tr("Precision"));
        QLabel *pLabMinTol = new QLabel(tr("Min. tolerance"));
        QLabel *pLabMaxTol = new QLabel(tr("Max. tolerance"));
        pLabPrec->setAlignment(::Qt::AlignRight |Qt::AlignVCenter);
        pLabMinTol->setAlignment(::Qt::AlignRight |Qt::AlignVCenter);
        pLabMaxTol->setAlignment(::Qt::AlignRight |Qt::AlignVCenter);
        m_pdePrecision = new FloatEdit;
        QString tip("OCC: the basic precision");
        m_pdePrecision->setToolTip(tip);
        m_pdeMinTolerance = new FloatEdit;
        tip = "OCC: The minimal allowed tolerance. It defines the minimal allowed length of edges. Detected edges having\n"
                    "length less than the specified minimal tolerance will be removed if ModifyTopologyMode in Repairing tool\n"
                    "for wires is set to true.";
        m_pdeMinTolerance->setToolTip(tip);
        m_pdeMaxTolerance = new FloatEdit;
        tip = "OCC: The maximum allowed tolerance. All problems will be detected for cases when a dimension of\n"
        "invalidity is larger than the basic precision or a tolerance of sub-shape on that problem is detected.\n"
        "The maximum tolerance value limits the increasing tolerance for fixing a problem such as fix of not\n"
        "connected and self-intersected wires. If a value larger than the maximum allowed tolerance is necessary\n"
        "for correcting a detected problem the problem can not be fixed. The maximal tolerance is not taking into\n"
        "account during computation of tolerance of edges in ShapeFix_SameParameter() method and\n"
        "ShapeFix_Edge::FixVertexTolerance() method.";
        m_pdeMaxTolerance->setToolTip(tip);

        QLabel *pLabLen0 = new QLabel(Units::lengthUnitQLabel());
        QLabel *pLabLen1 = new QLabel(Units::lengthUnitQLabel());
        QLabel *pLabLen2 = new QLabel(Units::lengthUnitQLabel());
        pLabLen0->setAlignment(::Qt::AlignLeft|Qt::AlignVCenter);
        pLabLen1->setAlignment(::Qt::AlignLeft|Qt::AlignVCenter);
        pLabLen2->setAlignment(::Qt::AlignLeft|Qt::AlignVCenter);

        pFixerLayout->addWidget(pLabPrec,            1,1);
        pFixerLayout->addWidget(m_pdePrecision,    1,2);
        pFixerLayout->addWidget(pLabLen0,            1,3);
        pFixerLayout->addWidget(pLabMinTol,          2,1);
        pFixerLayout->addWidget(m_pdeMinTolerance, 2,2);
        pFixerLayout->addWidget(pLabLen1,            2,3);
        pFixerLayout->addWidget(pLabMaxTol,          3,1);
        pFixerLayout->addWidget(m_pdeMaxTolerance, 3,2);
        pFixerLayout->addWidget(pLabLen2,            3,3);
    }
    QGridLayout *pActionLayout = new QGridLayout;
    {
        m_ppbListShapes = new QPushButton(tr("List shapes"));
        m_ppbStitch = new QPushButton(tr("Stitch shapes"));
        m_ppbReverseShapes = new QPushButton(tr("Reverse shapes"));
        m_ppbSmallEdges = new QPushButton(tr("Remove small edges"));
        m_ppbFixGaps = new QPushButton(tr("Fix gaps"));
        m_ppbFixAll = new QPushButton(tr("Fix everything"));
        pActionLayout->addWidget(m_ppbStitch);
        pActionLayout->addWidget(m_ppbReverseShapes);
        pActionLayout->addWidget(m_ppbSmallEdges);
        pActionLayout->addWidget(m_ppbFixGaps);
        pActionLayout->addWidget(m_ppbFixAll);

        m_ppbFixAll->setEnabled(false);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard, this);
    {
        m_ppbClearOutput = new QPushButton(tr("Clear output"));
        m_pButtonBox->addButton(m_ppbClearOutput, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    m_pptoOutput = new PlainTextOutput;
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pFixerLayout);
        pMainLayout->addLayout(pActionLayout);
        pMainLayout->addWidget(m_pptoOutput);

        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void ShapeFixerDlg::connectSignals()
{
    connect(m_ppbListShapes,    SIGNAL(clicked(bool)), SLOT(onListShapes()));
    connect(m_ppbReverseShapes, SIGNAL(clicked(bool)), SLOT(onReverseShapes()));
    connect(m_ppbStitch,        SIGNAL(clicked(bool)), SLOT(onStitchShapes()));
    connect(m_ppbSmallEdges,    SIGNAL(clicked(bool)), SLOT(onSmallEdges()));
    connect(m_ppbFixGaps,       SIGNAL(clicked(bool)), SLOT(onFixGaps()));
    connect(m_ppbFixAll,        SIGNAL(clicked(bool)), SLOT(onFixAll()));
}


void ShapeFixerDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void ShapeFixerDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    onReadParams();
    s_Geometry = saveGeometry();
}


void ShapeFixerDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("ShapeFixerDlg");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_Precision    = settings.value("Precision",    s_Precision).toDouble();
        s_MinTolerance = settings.value("MinTolerance", s_MinTolerance).toDouble();
        s_MaxTolerance = settings.value("MaxTolerance", s_MaxTolerance).toDouble();
    }
    settings.endGroup();
}


void ShapeFixerDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("ShapeFixerDlg");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("Precision",    s_Precision);
        settings.setValue("MinTolerance", s_MinTolerance);
        settings.setValue("MaxTolerance", s_MaxTolerance);
    }
    settings.endGroup();
}


void ShapeFixerDlg::outputMessage(QString const &msg)
{
    m_pptoOutput->onAppendQText(msg);
}


void ShapeFixerDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
    else if (pButton==m_ppbClearOutput) m_pptoOutput->clear();
}


void ShapeFixerDlg::onReadParams()
{
    s_Precision = m_pdePrecision->value()/Units::mtoUnit();
    s_MinTolerance = m_pdeMinTolerance->value()/Units::mtoUnit();
    s_MaxTolerance = m_pdeMaxTolerance->value()/Units::mtoUnit();
}


void ShapeFixerDlg::initDialog(const TopoDS_ListOfShape &shapes)
{
    m_pdePrecision->setValue(s_Precision*Units::mtoUnit());
    m_pdeMinTolerance->setValue(s_MinTolerance*Units::mtoUnit());
    m_pdeMaxTolerance->setValue(s_MaxTolerance*Units::mtoUnit());

    m_shapes = shapes;
    onListShapes();
}


void ShapeFixerDlg::onStitchShapes()
{
    QString strange;

    s_Precision = m_pdePrecision->value()/Units::mtoUnit();

    BRepBuilderAPI_Sewing stitcher(s_Precision);
    TopoDS_ListIteratorOfListOfShape iterator;
    for (iterator.Initialize(m_shapes); iterator.More(); iterator.Next())
    {
        stitcher.Add(iterator.Value());
    }

    stitcher.Perform();

    QString logmsg;
    logmsg += tr("Performing FACE stitching:\n");
    logmsg += tr("   Nb of free edges=%1\n").arg(stitcher.NbFreeEdges());
    logmsg += QString("   Nb of contiguous edges=%1\n").arg(stitcher.NbContigousEdges());

/*    If all faces have been sewn correctly, the result is a shell. Otherwise, it is a compound.
    After a successful sewing operation all faces have a coherent orientation.*/
    TopoDS_Shape stitchedshape;

    try
    {
        TopoDS_Shape sewedshape = stitcher.SewedShape();
        if(sewedshape.IsNull()) return;
        m_shapes.Clear();

        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(sewedshape, TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            TopoDS_Shell ShellShape = TopoDS::Shell(shapeExplorer.Current());
//            TopoDS_Shell WingShellShape = TopoDS::Shell(sewedshape);
            //make the solid
            if(!ShellShape.IsNull())
            {
                BRepBuilderAPI_MakeSolid solidMaker(ShellShape);
                if(!solidMaker.IsDone())
                {
                    logmsg += "   Solid not made... \n";
                    stitchedshape.Nullify();
                    return;
                }
                stitchedshape = solidMaker.Shape();

                logmsg += "   Stitching result is " + QString::fromStdString(occ::shapeType(stitchedshape)) + "\n";

                BRepCheck_Analyzer ShapeAnalyzer(stitchedshape);
                if(ShapeAnalyzer.IsValid()) logmsg += "   Shape topology is VALID \n\n";
                else                        logmsg += "   Shape topology is NOT VALID \n\n";
            }

            m_shapes.Append(stitchedshape);
        }
    }
    catch(Standard_TypeMismatch &)
    {
        logmsg += "     Type mismatch error\n";
    }

    listShapeProperties(strange, stitchedshape, "   "),
    logmsg += strange;
    outputMessage(logmsg);
}


void ShapeFixerDlg::onListShapes()
{
    TopoDS_ListIteratorOfListOfShape iterator;
    int ishape=0;
    QString strange, logmsg, prefix="      ";
    strange = tr("Fuselage is made of %1 shape(s):").arg(m_shapes.Extent());
    outputMessage(strange+"\n");
    for (iterator.Initialize(m_shapes); iterator.More(); iterator.Next())
    {
        strange = tr("   Shape %1\n").arg(ishape);
        outputMessage(strange);
        listShapeProperties(logmsg, iterator.Value(), prefix);
        outputMessage(logmsg);
        ishape++;
    }
    outputMessage("\n");
}


void ShapeFixerDlg::listShapeProperties(QString &props, TopoDS_Shape const &shape, QString prefix)
{
    QString logmsg, strange;
    std::string properties;
    occ::listShapeContent(shape, properties, prefix.toStdString());

    props = QString::fromStdString(properties);

    double Xmin(1e10), Ymin(1e10), Zmin(1e10), Xmax(-1e10), Ymax(-1e10), Zmax(-1e10);
    occ::shapeBoundingBox(shape, Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
    logmsg = prefix + "Bounding box:\n";
    strange = QString::asprintf("   X=[%9g, %9g] ", Xmin*Units::mtoUnit(), Xmax *Units::mtoUnit());
    logmsg += prefix + strange + Units::lengthUnitQLabel() +"\n";
    strange = QString::asprintf("   Y=[%9g, %9g] ", Ymin*Units::mtoUnit(), Ymax *Units::mtoUnit());
    logmsg += prefix + strange + Units::lengthUnitQLabel() +"\n";
    strange = QString::asprintf("   Z=[%9g, %9g] ", Zmin*Units::mtoUnit(), Zmax *Units::mtoUnit());
    logmsg += prefix + strange + Units::lengthUnitQLabel() +"\n";
    props += logmsg +"\n";
}


void ShapeFixerDlg::onReverseShapes()
{
    QString strange, logmsg;
    TopoDS_ListIteratorOfListOfShape iterator;
    int ishape=0;
    for (iterator.Initialize(m_shapes); iterator.More(); iterator.Next())
    {
        iterator.Value().Reverse();
        if     (iterator.Value().Orientation()==TopAbs_FORWARD)  strange = tr("   After: sub-shape %1 has FORWARD  orientation").arg(ishape);
        else if(iterator.Value().Orientation()==TopAbs_REVERSED) strange = tr("   After: sub-shape %1 has REVERSED orientation").arg(ishape);

        logmsg += strange +"\n";
        ishape++;
    }
    outputMessage(logmsg+"\n");
}


void ShapeFixerDlg::onSmallEdges()
{
    TopoDS_ListOfShape fixedshapes;
    Handle(ShapeFix_Wireframe) SFWF = new ShapeFix_Wireframe;
    SFWF->SetPrecision(s_Precision);
    SFWF->SetMinTolerance(s_MinTolerance);
    SFWF->SetMaxTolerance(s_MaxTolerance);
    SFWF->ModeDropSmallEdges() = Standard_True;

    TopoDS_ListIteratorOfListOfShape iterator;
    for (iterator.Initialize(m_shapes); iterator.More(); iterator.Next())
    {
        SFWF->Load(iterator.Value());
        SFWF->FixSmallEdges();
        fixedshapes.Append(SFWF->Shape());
    }

    m_shapes = fixedshapes;
    outputMessage(tr("Finished fixing small edges if any:\n"));
    onListShapes();
}


void ShapeFixerDlg::onFixGaps()
{
    TopoDS_ListOfShape fixedshapes;
    Handle(ShapeFix_Wireframe) SFWF = new ShapeFix_Wireframe;
    SFWF->SetPrecision(s_Precision);
    SFWF->SetMinTolerance(s_MinTolerance);
    SFWF->SetMaxTolerance(s_MaxTolerance);

    TopoDS_ListIteratorOfListOfShape iterator;
    for (iterator.Initialize(m_shapes); iterator.More(); iterator.Next())
    {
        SFWF->Load(iterator.Value());
        SFWF->FixWireGaps();
        fixedshapes.Append(SFWF->Shape());
    }
//    SFWF->DropSmallEdgesMode() = Standard_True;
      m_shapes = fixedshapes;
      outputMessage(tr("Finished fixing gaps if any:\n"));
      onListShapes();
}


void ShapeFixerDlg::onFixAll()
{
    QString strange, logmsg, prefix="   ";

    TopoDS_ListOfShape fixedshapes;

    Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
    TopoDS_ListIteratorOfListOfShape iterator;
    int ishape=0;
    for (iterator.Initialize(m_shapes); iterator.More(); iterator.Next())
    {
        sfs->Init (iterator.Value());
        sfs->SetPrecision (  s_Precision);
        sfs->SetMinTolerance(s_MinTolerance);
        sfs->SetMaxTolerance(s_MaxTolerance);
        sfs->Perform();
        TopoDS_Shape aResult = sfs->Shape();
        strange = tr("Fixed shape %1\n").arg(ishape);
        outputMessage(strange);
        listShapeProperties(logmsg, aResult, prefix);

        fixedshapes.Append(aResult);

        outputMessage(logmsg);
        ishape++;
    }

    m_shapes = fixedshapes;
}



