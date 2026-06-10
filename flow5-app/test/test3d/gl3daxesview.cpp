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



#include <QDebug>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>

#include "gl3daxesview.h"

#include <api/objects_global.h>

#include <core/xflcore.h>
#include <core/stlreaderdlg.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <test/test3d/gl3daxesview.h>


Quaternion gl3dAxesView::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);
QByteArray gl3dAxesView::s_Geometry;

LineStyle gl3dAxesView::s_WindVecsStyle = {true, Line::SOLID, 2, fl5Color(100,100,100),  Line::NOSYMBOL, "Wind vectors"};
LineStyle gl3dAxesView::s_StabStyle     = {true, Line::DASH, 3, fl5Color( 71, 91,225),  Line::NOSYMBOL, "Stability axes"};
LineStyle gl3dAxesView::s_WindStyle     = {true, Line::DASH,  3, fl5Color( 91,225, 71),  Line::NOSYMBOL, "Wind axes"};


double gl3dAxesView::s_Alpha = 0.0;
double gl3dAxesView::s_Beta  = 0.0;

Vector3d gl3dAxesView::s_Vector;


gl3dAxesView::gl3dAxesView(QWidget *pParent) : gl3dXflView(pParent)
{
    setWindowTitle(tr("Axes test"));
    setupLayout();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connectSignals();

    m_bResetObject  = true;
    m_RefLength = 1.0;

    onUpdateAxes();
}


void gl3dAxesView::setupLayout()
{
    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);
        QVBoxLayout *pFrLayout = new QVBoxLayout;
        {
            QGroupBox *pgbWind = new QGroupBox(tr("Wind"));
            {
                QGridLayout *pWindLayout = new QGridLayout;
                {
                    QLabel *plabAlpha = new QLabel("<p>&alpha;=</p>");
                    m_pfeAlpha = new FloatEdit(s_Alpha);
                    QLabel *plabBeta = new QLabel("<p>&beta;=</p>");
                    m_pfeBeta = new FloatEdit(s_Beta);
                    QLabel *plabDeg0 = new QLabel("<p>&deg;</p>");
                    QLabel *plabDeg1 = new QLabel("<p>&deg;</p>");


                    pWindLayout->addWidget(plabAlpha,     1,1, Qt::AlignRight);
                    pWindLayout->addWidget(m_pfeAlpha,    1,2);
                    pWindLayout->addWidget(plabDeg0,      1,3);
                    pWindLayout->addWidget(plabBeta,      2,1, Qt::AlignRight);
                    pWindLayout->addWidget(m_pfeBeta,     2,2);
                    pWindLayout->addWidget(plabDeg1,      2,3);
                    pWindLayout->setColumnStretch(3,1);
                }
                pgbWind->setLayout(pWindLayout);
            }

            QGroupBox *pgbDisplay = new QGroupBox(tr("Display"));
            {
                QGridLayout *pDisplayLayout = new QGridLayout;
                {
                    m_pchGeomAxes      = new QCheckBox(tr("Geometry axes"));
                    m_pchGeomAxes->setChecked(m_bAxes);
                    m_pchWindAxes      = new QCheckBox(tr("Wind axes"));
                    m_pchStabilityAxes = new QCheckBox(tr("Stability axes"));
                    m_pchWindAxes->setChecked(true);
                    m_pchStabilityAxes->setChecked(true);
                    m_plbWind = new LineBtn(s_WindStyle);
                    m_plbStab = new LineBtn(s_StabStyle);
                    pDisplayLayout->addWidget(m_pchGeomAxes,      1, 1);
                    pDisplayLayout->addWidget(m_pchWindAxes,      3, 1);
                    pDisplayLayout->addWidget(m_plbWind,          3, 2);
                    pDisplayLayout->addWidget(m_pchStabilityAxes, 4, 1);
                    pDisplayLayout->addWidget(m_plbStab,          4, 2);
                }
                pgbDisplay->setLayout(pDisplayLayout);
            }

            QGroupBox *pgbVector = new QGroupBox(tr("Vector"));
            {
                QGridLayout * pVectorLayout = new QGridLayout;
                {
                    QLabel *plabX = new QLabel(tr("x="));
                    QLabel *plabY = new QLabel(tr("y="));
                    QLabel *plabZ = new QLabel(tr("z="));
                    m_pfeX = new FloatEdit(s_Vector.x);
                    m_pfeY = new FloatEdit(s_Vector.y);
                    m_pfeZ = new FloatEdit(s_Vector.z);

                    pVectorLayout->addWidget(plabX,  2,1);
                    pVectorLayout->addWidget(m_pfeX, 2,2);
                    pVectorLayout->addWidget(plabY,  3,1);
                    pVectorLayout->addWidget(m_pfeY, 3,2);
                    pVectorLayout->addWidget(plabZ,  4,1);
                    pVectorLayout->addWidget(m_pfeZ, 4,2);
                    pVectorLayout->setColumnStretch(3,1);

                }
                pgbVector->setLayout(pVectorLayout);
            }

            m_ppto = new PlainTextOutput;
            m_ppto->setMinimumSize(QSize(450,210));

            pFrLayout->addWidget(pgbWind);
            pFrLayout->addWidget(pgbDisplay);
            pFrLayout->addWidget(pgbVector);
            pFrLayout->addWidget(m_ppto);
        }
        pFrame->setLayout(pFrLayout);
    }
}


void gl3dAxesView::connectSignals()
{
    connect(m_pfeAlpha,         SIGNAL(floatChanged(float)),   SLOT(onUpdateAxes()));
    connect(m_pfeBeta,          SIGNAL(floatChanged(float)),   SLOT(onUpdateAxes()));
    connect(m_pfeAlpha,         SIGNAL(floatChanged(float)),   SLOT(onConvert()));
    connect(m_pfeBeta,          SIGNAL(floatChanged(float)),   SLOT(onConvert()));

    connect(m_pfeX,             SIGNAL(floatChanged(float)),   SLOT(onConvert()));
    connect(m_pfeY,             SIGNAL(floatChanged(float)),   SLOT(onConvert()));
    connect(m_pfeZ,             SIGNAL(floatChanged(float)),   SLOT(onConvert()));


    connect(m_pchStabilityAxes, SIGNAL(clicked(bool)),         SLOT(onUpdateAxes()));
    connect(m_pchWindAxes,      SIGNAL(clicked(bool)),         SLOT(onUpdateAxes()));

    connect(m_plbWind,          SIGNAL(clickedLB(LineStyle)),  SLOT(onWindLineStyle()));
    connect(m_plbStab,          SIGNAL(clickedLB(LineStyle)),  SLOT(onStabLineStyle()));

    connect(m_pchGeomAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));
}


void gl3dAxesView::onWindLineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_WindStyle);
    lm.exec(QCursor::pos());

    s_WindStyle = lm.theStyle();
    m_plbWind->setTheStyle(s_WindStyle);
}


void gl3dAxesView::onStabLineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_StabStyle);
    lm.exec(QCursor::pos());

    s_StabStyle = lm.theStyle();
    m_plbStab->setTheStyle(s_StabStyle);
}


void gl3dAxesView::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAxesView");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_Alpha = settings.value("alpha",  s_Alpha).toDouble();
        s_Beta  = settings.value("beta",   s_Beta).toDouble();

        s_Vector.x     = settings.value("X",  s_Vector.x).toDouble();
        s_Vector.y     = settings.value("Y",  s_Vector.y).toDouble();
        s_Vector.z     = settings.value("Z",  s_Vector.z).toDouble();

        xfl::loadLineSettings(settings, s_WindVecsStyle, "WindVecsStyle");
        xfl::loadLineSettings(settings, s_WindStyle, "WindStyle");
        xfl::loadLineSettings(settings, s_StabStyle, "StabStyle");
    }
    settings.endGroup();
}


void gl3dAxesView::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAxesView");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("alpha", s_Alpha);
        settings.setValue("beta",  s_Beta);

        settings.setValue("X", s_Vector.x);
        settings.setValue("Y", s_Vector.y);
        settings.setValue("Z", s_Vector.z);


        xfl::saveLineSettings(settings, s_WindVecsStyle, "WindVecsStyle");
        xfl::saveLineSettings(settings, s_WindStyle, "WindStyle");
        xfl::saveLineSettings(settings, s_StabStyle, "StabStyle");
    }
    settings.endGroup();
}


void gl3dAxesView::showEvent(QShowEvent *pEvent)
{
    gl3dXflView::showEvent(pEvent);
    if(W3dPrefs::s_bSaveViewPoints) restoreViewPoint(s_ab_quat);
    restoreGeometry(s_Geometry);
    reset3dScale();
}


void gl3dAxesView::hideEvent(QHideEvent *)
{
        if(W3dPrefs::s_bSaveViewPoints) saveViewPoint(s_ab_quat);
        s_Geometry = saveGeometry();
}


void gl3dAxesView::glRenderView()
{
    Vector3d origin;

    paintThinArrow(origin, s_Vector, Qt::red, 2, Line::SOLID);

    // wind arrow
    Vector3d O(-0.75f/m_glScalef,0,0);
    Vector3d W = objects::windDirection(s_Alpha, s_Beta) * 0.25f;
    paintThickArrow(O, W*1.0f/m_glScalef, Qt::darkCyan, true);
    glRenderText(O.x-0.015f/m_glScalef, O.y, O.z+0.015f/m_glScalef, "Wind", Qt::darkCyan);


    if(m_pchWindAxes->isChecked())
    {
        // fixed scale axis for the axes
        QMatrix4x4 vm(m_matView);
        m_matView = rotationMatrix();
        m_matView.scale(m_glScalef, m_glScalef, m_glScalef);
        m_matView.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());
        m_matView.scale(0.5f/m_glScalef, 0.5f/m_glScalef, 0.5f/m_glScalef);

        CartesianFrame const &CFWind = m_AF.CFWind();
        paintThinArrow(origin, CFWind.Idir(), s_WindStyle);
        paintThinArrow(origin, CFWind.Jdir(), s_WindStyle);
        paintThinArrow(origin, CFWind.Kdir(), s_WindStyle);

        glRenderText(CFWind.Idir(), "x_wind", s_WindStyle.m_Color);
        glRenderText(CFWind.Jdir(), "y_wind", s_WindStyle.m_Color);
        glRenderText(CFWind.Kdir()+Vector3d(0,0,+0.05), "z_wind", s_WindStyle.m_Color);

        m_matView=vm; // leave things as they were
    }

    if(m_pchStabilityAxes->isChecked())
    {
        // fixed scale axis for the axes
        QMatrix4x4 vm(m_matView);
        m_matView = rotationMatrix();
        m_matView.scale(m_glScalef, m_glScalef, m_glScalef);
        m_matView.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());
        m_matView.scale(0.5f/m_glScalef, 0.5f/m_glScalef, 0.5f/m_glScalef);

        CartesianFrame const &m_CFStab = m_AF.CFStab();
        paintThinArrow(origin, m_CFStab.Idir(), s_StabStyle);
        paintThinArrow(origin, m_CFStab.Jdir(), s_StabStyle);
        paintThinArrow(origin, m_CFStab.Kdir(), s_StabStyle);

        glRenderText(m_CFStab.Idir(), "x_stab", s_StabStyle.m_Color);
        glRenderText(m_CFStab.Jdir()+Vector3d(0,0,-0.05), "y_stab", s_StabStyle.m_Color);
        glRenderText(m_CFStab.Kdir(), "z_stab", s_StabStyle.m_Color);

        m_matView=vm; // leave things as they were
    }

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadSurf.release();
    paintTriangles3Vtx(m_vboStlTriangulation, QColor(107,107,107), false, s_Light.m_bIsLightOn);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
    paintSegments(m_vboStlOutline, W3dPrefs::s_OutlineStyle);
}


void gl3dAxesView::glMake3dObjects()
{
    if(m_bResetObject)
    {
        QFile StlFile(":/textfiles/stl_mesh.stl");
        std::vector<Triangle3d> m_Triangles;
        std::string name;
        if(StlFile.open(QIODevice::ReadOnly))
        {
            QDataStream binstream(&StlFile);
            StlReaderDlg::importStlBinaryFile(binstream, 0.0005, m_Triangles, name);

            gl::makeTriangles3Vtx(m_Triangles, true, m_vboStlTriangulation);
            gl::makeTrianglesOutline(m_Triangles, Vector3d(), m_vboStlOutline);
        }

        m_bResetObject = false;
    }
}


void gl3dAxesView::onUpdateAxes()
{
    readData();

    m_AF.setOpp(s_Alpha, s_Beta, 0.0, 1.0);

/*    m_ppto->onAppendQText(QString::asprintf("\n Body RH rule: %d\n",   m_CFBody.checkRHRule()));
    m_ppto->onAppendQText(QString::asprintf(  " Wind RH rule: %d\n",   m_AF.CFWind().checkRHRule()));
    m_ppto->onAppendQText(QString::asprintf(  " Stab RH rule: %d\n\n", m_AF.CFStab().checkRHRule()));
*/
    update();
}


void gl3dAxesView::readData()
{
    s_Alpha = m_pfeAlpha->value();
    s_Beta  = m_pfeBeta->value();
    s_Vector.x = m_pfeX->value();
    s_Vector.y = m_pfeY->value();
    s_Vector.z = m_pfeZ->value();
}


void gl3dAxesView::onConvert()
{
    update();

    readData();

    QString strange;

    Vector3d VWind = m_AF.CFWind().globalToLocal(s_Vector);
    Vector3d VStab = m_AF.CFStab().globalToLocal(s_Vector);

    strange = " to        Wind          Stab\n";
    strange += QString::asprintf("x=%13.3g %13.3g\n", VWind.x, VStab.x);
    strange += QString::asprintf("y=%13.3g %13.3g\n", VWind.y, VStab.y);
    strange += QString::asprintf("z=%13.3g %13.3g\n", VWind.z, VStab.z);
    m_ppto->onAppendQText(strange + EOLch);

    // and back
/*    VBody = m_CFBody.localToGlobal(VBody);
    VWind = m_AF.CFWind().localToGlobal(VWind);
    VStab = m_AF.CFStab().localToGlobal(VStab);
    strange  = "from  Body     Wind     Stab\n";
    strange += QString::asprintf("x=%13.3g %13.3g %13.3g\n", VBody.x, VWind.x, VStab.x);
    strange += QString::asprintf("y=%13.3g %13.3g %13.3g\n", VBody.y, VWind.y, VStab.y);
    strange += QString::asprintf("z=%13.3g %13.3g %13.3g\n", VBody.z, VWind.z, VStab.z);
    m_ppto->onAppendQText(strange +"\n\n");*/

/*    m_ppto->onAppendQText("GeomToWindAxes:\n");
    Vector3d Vg = windToGeomAxes(s_Vec, s_Alpha, s_Beta);
    strange = QString::asprintf("   x=%7g\n   y=%7g\n   z=%7g\n\n", Vg.x, Vg.y, Vg.z);
    m_ppto->onAppendQText(strange);*/
}

