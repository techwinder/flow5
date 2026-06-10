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

LineStyle gl3dAxesView::s_WindVecsStyle = {true, Line::SOLID, 2, fl5Color(100,100,100),  Line::NOSYMBOL, "Wind"};
LineStyle gl3dAxesView::s_StabStyle     = {true, Line::DASH,  2, fl5Color( 71, 91,225),  Line::NOSYMBOL, "Stability axes"};
LineStyle gl3dAxesView::s_WindStyle     = {true, Line::DASH,  2, fl5Color( 91,225, 71),  Line::NOSYMBOL, "Wind axes"};


double gl3dAxesView::s_Alpha = 0.0;
double gl3dAxesView::s_Beta  = 0.0;


bool gl3dAxesView::s_bWindAxes = false;
bool gl3dAxesView::s_bStabAxes = false;

gl3dAxesView::gl3dAxesView(QWidget *pParent) : gl3dXflView(pParent)
{
    setWindowTitle(tr("Axes test"));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


    m_bResetObject  = true;
    m_RefLength = 1.0;

    updateAxes();
}


void gl3dAxesView::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAxesView");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_Alpha = settings.value("alpha",  s_Alpha).toDouble();
        s_Beta  = settings.value("beta",   s_Beta).toDouble();

        xfl::loadLineSettings(settings, s_WindVecsStyle, "WindVecsStyle");
        xfl::loadLineSettings(settings, s_WindStyle, "WindStyle");
        xfl::loadLineSettings(settings, s_StabStyle, "StabStyle");

        s_bWindAxes = settings.value("bWindAxes", s_bWindAxes).toBool();
        s_bStabAxes = settings.value("bStabAxes", s_bStabAxes).toBool();
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


        xfl::saveLineSettings(settings, s_WindVecsStyle, "WindVecsStyle");
        xfl::saveLineSettings(settings, s_WindStyle, "WindStyle");
        xfl::saveLineSettings(settings, s_StabStyle, "StabStyle");

        settings.setValue("bWindAxes", s_bWindAxes);
        settings.setValue("bStabAxes", s_bStabAxes);

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

    // wind arrow
    Vector3d O(-0.75f/m_glScalef,0,0);
    Vector3d W = objects::windDirection(s_Alpha, s_Beta) * 0.25f;
    paintThickArrow(O, W, xfl::fromfl5Clr(W3dPrefs::s_WindStyle.m_Color), true, true);

    QMatrix4x4 vm(m_matView);
    m_matView = rotationMatrix();
    m_matView.translate(m_glRotCenter.xf()*m_glScalef, m_glRotCenter.yf()*m_glScalef, m_glRotCenter.zf()*m_glScalef);
    m_matView.scale(m_glScalef);
    glRenderText(O.x-0.015f/m_glScalef, O.y, O.z+0.015f/m_glScalef, "Wind", Qt::darkCyan);
    m_matView = vm;

    if(s_bWindAxes)
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

    if(s_bStabAxes)
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


void gl3dAxesView::updateAxes()
{
    m_AF.setOpp(s_Alpha, s_Beta, 0.0, 1.0);

/*    m_ppto->onAppendQText(QString::asprintf("\n Body RH rule: %d\n",   m_CFBody.checkRHRule()));
    m_ppto->onAppendQText(QString::asprintf(  " Wind RH rule: %d\n",   m_AF.CFWind().checkRHRule()));
    m_ppto->onAppendQText(QString::asprintf(  " Stab RH rule: %d\n\n", m_AF.CFStab().checkRHRule()));
*/
    update();
}



