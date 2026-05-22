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

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <ShapeFix_Wireframe.hxx>

#include "shapefixerdlg.h"
#include <api/fuse.h>
#include <api/occ_globals.h>
#include <api/units.h>

#include <core/xflcore.h>

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
        QLabel *plabPrec   = new QLabel(tr("Precision"));
        QLabel *plabMinTol = new QLabel(tr("Min. tolerance"));
        QLabel *plabMaxTol = new QLabel(tr("Max. tolerance"));

        m_pfePrecision = new FloatEdit;
        QString tip("OCC: the basic precision");
        m_pfePrecision->setToolTip(tip);
        m_pfeMinTolerance = new FloatEdit;
        tip = "<p>OCC: The minimal allowed tolerance. It defines the minimal allowed length of edges. Detected edges having "
                    "length less than the specified minimal tolerance will be removed if ModifyTopologyMode in Repairing tool "
                    "for wires is set to true.</p>";
        m_pfeMinTolerance->setToolTip(tip);
        m_pfeMaxTolerance = new FloatEdit;
        tip = "<p>OCC: The maximum allowed tolerance. All problems will be detected for cases when a dimension of "
              "invalidity is larger than the basic precision or a tolerance of sub-shape on that problem is detected.<br>"
              "The maximum tolerance value limits the increasing tolerance for fixing a problem such as fix of not "
              "connected and self-intersected wires. If a value larger than the maximum allowed tolerance is necessary "
              "for correcting a detected problem the problem can not be fixed. The maximal tolerance is not taking into "
              "account during computation of tolerance of edges in ShapeFix_SameParameter() method and "
              "ShapeFix_Edge::FixVertexTolerance() method.</p>";
        m_pfeMaxTolerance->setToolTip(tip);

        QLabel *plabLen0 = new QLabel(Units::lengthUnitQLabel());
        QLabel *plabLen1 = new QLabel(Units::lengthUnitQLabel());
        QLabel *plabLen2 = new QLabel(Units::lengthUnitQLabel());


        pFixerLayout->addWidget(plabPrec,            1,1, Qt::AlignRight);
        pFixerLayout->addWidget(m_pfePrecision,      1,2);
        pFixerLayout->addWidget(plabLen0,            1,3);
        pFixerLayout->addWidget(plabMinTol,          2,1, Qt::AlignRight);
        pFixerLayout->addWidget(m_pfeMinTolerance,   2,2);
        pFixerLayout->addWidget(plabLen1,            2,3);
        pFixerLayout->addWidget(plabMaxTol,          3,1, Qt::AlignRight);
        pFixerLayout->addWidget(m_pfeMaxTolerance,   3,2);
        pFixerLayout->addWidget(plabLen2,            3,3);
    }

    QGridLayout *pActionLayout = new QGridLayout;
    {
        m_ppbListShapes    = new QPushButton(tr("List shapes"));
        m_ppbStitch        = new QPushButton(tr("Stitch shapes"));
        m_ppbReverseShapes = new QPushButton(tr("Reverse shapes"));
        m_ppbSmallEdges    = new QPushButton(tr("Remove small edges"));
        m_ppbFixGaps       = new QPushButton(tr("Fix gaps"));
        m_ppbFixAll        = new QPushButton(tr("Fix everything"));
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
        m_ppbClearOutput->setToolTip(tr("<p>Clears the text output</p>"));
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
    s_Precision = m_pfePrecision->value()/Units::mtoUnit();
    s_MinTolerance = m_pfeMinTolerance->value()/Units::mtoUnit();
    s_MaxTolerance = m_pfeMaxTolerance->value()/Units::mtoUnit();
}


void ShapeFixerDlg::initDialog(const NCollection_List<TopoDS_Shape> &shapes)
{
    m_pfePrecision->setValue(s_Precision*Units::mtoUnit());
    m_pfeMinTolerance->setValue(s_MinTolerance*Units::mtoUnit());
    m_pfeMaxTolerance->setValue(s_MaxTolerance*Units::mtoUnit());

    m_Shapes = shapes;
    onListShapes();
}


void ShapeFixerDlg::onStitchShapes()
{
    s_Precision = m_pfePrecision->value()/Units::mtoUnit();

    std::string logmsg;
    occ::stitchShapes(m_Shapes, s_Precision, logmsg);
    outputMessage(QString::fromStdString(logmsg));

    logmsg.clear();
    occ::listAllShapes(m_Shapes, logmsg);
    outputMessage(QString::fromStdString(logmsg));
}


void ShapeFixerDlg::onListShapes()
{
    NCollection_List<TopoDS_Shape>::Iterator iterator;
    int ishape=0;
    QString strange;
    std::string    logmsg, prefix="      ";
    strange = QString::asprintf("Fuselage is made of %d shape(s):", m_Shapes.Extent());
    outputMessage(strange+"\n");
    for (iterator.Initialize(m_Shapes); iterator.More(); iterator.Next())
    {
        strange = QString::asprintf("   Shape %d\n", ishape);
        outputMessage(strange);
        occ::listShapeProperties(iterator.Value(), logmsg, prefix);
        outputMessage(QString::fromStdString(logmsg));
        ishape++;
    }
    outputMessage("\n");
}


void ShapeFixerDlg::onReverseShapes()
{
    QString strange, logmsg;
    NCollection_List<TopoDS_Shape>::Iterator iterator;
    int ishape=0;
    for (iterator.Initialize(m_Shapes); iterator.More(); iterator.Next())
    {
        iterator.Value().Reverse();
        if     (iterator.Value().Orientation()==TopAbs_FORWARD)  strange = QString::asprintf("   After: sub-shape %d has FORWARD  orientation", ishape);
        else if(iterator.Value().Orientation()==TopAbs_REVERSED) strange = QString::asprintf("   After: sub-shape %d has REVERSED orientation", ishape);

        logmsg += strange +"\n";
        ishape++;
    }
    outputMessage(logmsg+"\n");
}


void ShapeFixerDlg::onSmallEdges()
{
    NCollection_List<TopoDS_Shape> fixedshapes;
    NCollection_List<TopoDS_Shape>::Iterator iterator;
    for (iterator.Initialize(m_Shapes); iterator.More(); iterator.Next())
    {
        TopoDS_Shape result;
        occ::shapeFixSmallEdges(iterator.Value(), result, s_Precision, s_MinTolerance, s_MaxTolerance);
        fixedshapes.Append(result);
    }

    m_Shapes = fixedshapes;
    outputMessage("Finished fixing small edges if any\n");
    onListShapes();
}


void ShapeFixerDlg::onFixGaps()
{
    NCollection_List<TopoDS_Shape> fixedshapes;
    NCollection_List<TopoDS_Shape>::Iterator iterator;
    for (iterator.Initialize(m_Shapes); iterator.More(); iterator.Next())
    {
        TopoDS_Shape result;
        occ::shapeFixGaps(iterator.Value(), result, s_Precision, s_MinTolerance, s_MaxTolerance);
        fixedshapes.Append(result);
    }

    m_Shapes = fixedshapes;
    outputMessage("Finished fixing gaps if any:\n");
    onListShapes();
}


void ShapeFixerDlg::onFixAll()
{
    QString strange;
    std::string    logmsg, prefix="   ";

    NCollection_List<TopoDS_Shape> fixedshapes;

    NCollection_List<TopoDS_Shape>::Iterator iterator;
    int ishape=0;
    for (iterator.Initialize(m_Shapes); iterator.More(); iterator.Next())
    {
        TopoDS_Shape aResult;
        occ::shapeFixAll(iterator.Value(), aResult, s_Precision, s_MinTolerance, s_MaxTolerance);
        strange = QString::asprintf("Fixed shape %d\n", ishape);
        outputMessage(strange);
        occ::listShapeProperties(aResult, logmsg, prefix);
        outputMessage(QString::fromStdString(logmsg));

        fixedshapes.Append(aResult);

        ishape++;
    }

    m_Shapes = fixedshapes;
}



