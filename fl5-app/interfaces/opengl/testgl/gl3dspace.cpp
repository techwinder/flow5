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

#include <QGridLayout>
#include <QFile>

#include "gl3dspace.h"

#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <api/geom_global.h>
#include <api/utils.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>


int gl3dSpace::s_NObjects = 869;
double gl3dSpace::s_SphereRadius = 31.0;

gl3dSpace::gl3dSpace(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("The final frontier");

    m_BtnPressed = Qt::NoButton;
    m_bResetArcs = m_bResetInstances = true;
    m_iCloseIndex = -1;
    m_iSelIndex = 0;

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Text,       DisplayOptions::textColor());
        QColor clr = DisplayOptions::backgroundColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window,     clr);
        palette.setColor(QPalette::Base,       clr);

        pFrame->setCursor(Qt::ArrowCursor);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QGridLayout *pMainLayout = new QGridLayout;
        {
            QLabel *pLabTitle = new QLabel("NEARGALCAT - Updated Nearby Galaxy Catalog");
            pLabTitle->setPalette(palette);

            QLabel *pLabNGalaxies = new QLabel("N closest galaxies:");

            m_pieNGalaxies = new IntEdit(s_NObjects);
            m_pieNGalaxies->setPalette(palette);
            connect(m_pieNGalaxies,  SIGNAL(intChanged(int)),  SLOT(onNGalaxies()));

            QLabel *pLabRadius = new QLabel("Sphere radius:");
            QSlider *pPlanetSize = new QSlider(Qt::Horizontal);
            pPlanetSize->setRange(0, 100);
            pPlanetSize->setTickInterval(5);
            pPlanetSize->setTickPosition(QSlider::TicksBelow);
            pPlanetSize->setValue(int(s_SphereRadius));

            connect(pPlanetSize,  SIGNAL(sliderMoved(int)),  SLOT(onObjectRadius(int)));

            m_plwGalaxies = new QListWidget;
            connect(m_plwGalaxies, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(onSelGalaxy(QListWidgetItem*)));

            QCheckBox *pchAxes = new QCheckBox("Axes");
            pchAxes->setChecked(true);
            connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

            QLabel *pNEARGALCATLink = new QLabel;
            pNEARGALCATLink->setText("<a href=https://heasarc.gsfc.nasa.gov/W3Browse/galaxy-catalog/neargalcat.html>https://heasarc.gsfc.nasa.gov</a>");
            pNEARGALCATLink->setOpenExternalLinks(true);
            pNEARGALCATLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);


            pMainLayout->addWidget(pLabTitle,       1, 1, 1, 2);
            pMainLayout->addWidget(pLabNGalaxies,   2, 1);
            pMainLayout->addWidget(m_pieNGalaxies,  2, 2);
            pMainLayout->addWidget(pLabRadius,      3, 1);
            pMainLayout->addWidget(pPlanetSize,     3, 2);
            pMainLayout->addWidget(m_plwGalaxies,   4, 1, 1, 2);
            pMainLayout->addWidget(pchAxes,         5, 1, 1, 2);
            pMainLayout->addWidget(pNEARGALCATLink, 6, 1, 1, 2);
        }

        pFrame->setLayout(pMainLayout);
//        wt::setWidgetStyle(pFrame, palette);
    }

    makeGalaxies();
    selectListItem(m_iSelIndex);
}


QString gl3dSpace::degTo24(double Ra) const
{
    int h = int(Ra/15);
    int m = int((Ra-h*15.0)/15.0*60.0);
    double s = (Ra-h*15.0-m*15.0/60)*60.0*60.0/15.0;

    return QString::asprintf("Ra = %dh %02d\' %02.3g\"",h,m,s);
}


#define NPOINTS 300
void gl3dSpace::glMake3dObjects()
{
    if(m_bResetArcs)
    {
        if(m_iSelIndex>=0 && m_iSelIndex<m_Galaxies.size())
        {
            int arcbuffersize = NPOINTS-1; // 1 segment less than the number of points

            arcbuffersize *= 2; // two arcs Ra and Da
            arcbuffersize *= 2; // two vertices per segment
            arcbuffersize *= 3; // three components per vertex
            QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);
            QVector<GLfloat> RadiusVertexArray(3*2*3, 0);


            Star const &galaxy = m_Galaxies.at(m_iSelIndex);
            // make right ascension arc

            float radius = galaxy.m_Position.normf();
            Vector3d rad(galaxy.m_Position.norm(),0,0);
            int iv = 0;

            for(int i=0; i<NPOINTS-1; i++)
            {
                float theta  = float(i)   * float(galaxy.m_Ra)/float(NPOINTS-1);
                float theta1 = float(i+1) * float(galaxy.m_Ra)/float(NPOINTS-1);
                ArcVertexArray[iv++] = radius*cosf(theta*PI/180.0);
                ArcVertexArray[iv++] = radius*sinf(theta*PI/180.0);
                ArcVertexArray[iv++] = 0.0;
                ArcVertexArray[iv++] = radius*cosf(theta1*PI/180.0);
                ArcVertexArray[iv++] = radius*sinf(theta1*PI/180.0);
                ArcVertexArray[iv++] = 0.0;
            }

            // make declination arc

            RadiusVertexArray[0] = 0.0f;
            RadiusVertexArray[1] = 0.0f;
            RadiusVertexArray[2] = 0.0f;
            RadiusVertexArray[3] = rad.xf();
            RadiusVertexArray[4] = rad.yf();
            RadiusVertexArray[5] = rad.zf();
            m_RaLoc.set(rad);
            m_RaLoc.rotateZ(galaxy.m_Ra/2.0);

            rad.rotateZ(galaxy.m_Ra);

            RadiusVertexArray[6]  = 0.0f;
            RadiusVertexArray[7]  = 0.0f;
            RadiusVertexArray[8]  = 0.0f;
            RadiusVertexArray[9]  = rad.xf();
            RadiusVertexArray[10] = rad.yf();
            RadiusVertexArray[11] = rad.zf();
            RadiusVertexArray[12] = 0.0f;
            RadiusVertexArray[13] = 0.0f;
            RadiusVertexArray[14] = 0.0f;
            RadiusVertexArray[15] = galaxy.m_Position.xf();
            RadiusVertexArray[16] = galaxy.m_Position.yf();
            RadiusVertexArray[17] = galaxy.m_Position.zf();

            Vector3d axis = (rad * galaxy.m_Position).normalized();

            m_DaLoc.set(rad);
            m_DaLoc.rotate(axis, fabs(galaxy.m_Da)/2.0);

            Vector3d vtx0, vtx1;
            for(int i=0; i<NPOINTS-1; i++)
            {
                vtx0.set(rad);
                vtx1.set(rad);
                float theta  = float(i)   * float(fabs(galaxy.m_Da))/float(NPOINTS-1);
                float theta1 = float(i+1) * float(fabs(galaxy.m_Da))/float(NPOINTS-1);
                vtx0.rotate(axis, theta);
                vtx1.rotate(axis, theta1);
                ArcVertexArray[iv++] = vtx0.xf();
                ArcVertexArray[iv++] = vtx0.yf();
                ArcVertexArray[iv++] = vtx0.zf();
                ArcVertexArray[iv++] = vtx1.xf();
                ArcVertexArray[iv++] = vtx1.yf();
                ArcVertexArray[iv++] = vtx1.zf();
            }

            Q_ASSERT(iv==arcbuffersize);

            m_vboArcSegments.destroy();
            m_vboArcSegments.create();
            m_vboArcSegments.bind();
            m_vboArcSegments.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
            m_vboArcSegments.release();

            m_vboRadius.destroy();
            m_vboRadius.create();
            m_vboRadius.bind();
            m_vboRadius.allocate(RadiusVertexArray.data(), 18 * sizeof(GLfloat));
            m_vboRadius.release();
        }
        m_bResetArcs = false;
    }

    if(m_bResetInstances)
    {
        // make the tetrahedron
        gl::makeTetra(Vector3d(), 1.0, m_vboTetra, m_vboTetraEdges);

        int nobj = std::min(s_NObjects, int(m_Galaxies.size()));

        QVector<float>PositionArray(nobj *3);//3 vertices for each galaxy
        int iv=0;
        for(int i=0; i<nobj; i++)
        {
            PositionArray[iv++] = m_Galaxies.at(i).m_Position.xf();
            PositionArray[iv++] = m_Galaxies.at(i).m_Position.yf();
            PositionArray[iv++] = m_Galaxies.at(i).m_Position.zf();
        }

        if(m_vboInstPositions.isCreated()) m_vboInstPositions.destroy();
        m_vboInstPositions.create();
        m_vboInstPositions.bind();
        {
            m_vboInstPositions.allocate(PositionArray.data(), int(PositionArray.size() * sizeof(GLfloat)));
        }
        m_vboInstPositions.release();

        m_bResetInstances = false;
    }
}


void gl3dSpace::glRenderView()
{
    double radius = double(s_SphereRadius)/5500.0/m_glScalef;

    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 0);
    }
    m_shadSurf.release();

    paintSphereInstances(m_vboInstPositions, float(s_SphereRadius)/m_glScalef/5000.0f, Qt::lightGray, false, true);

    if(m_iCloseIndex>=0 && m_iCloseIndex<m_Galaxies.size() && m_iCloseIndex!=m_iSelIndex)
    {
        Star const &galaxy = m_Galaxies.at(m_iCloseIndex);
        paintSphere(galaxy.m_Position, radius*2.0, Qt::darkCyan, true); // twice as fast?
        glRenderText(galaxy.m_Position.x,
                     galaxy.m_Position.y,
                     galaxy.m_Position.z + 2.3*radius,
                     galaxy.m_Name,
                     DisplayOptions::textColor());
    }

    if(m_iSelIndex>=0 && m_iSelIndex<m_Galaxies.size())
    {
        Star const &galaxy = m_Galaxies.at(m_iSelIndex);
        paintSphere(galaxy.m_Position, radius*2.0, Qt::darkYellow, true); // twice as fast?
        glRenderText(galaxy.m_Position.x,
                     galaxy.m_Position.y,
                     galaxy.m_Position.z + 2.3*radius,
                     galaxy.m_Name + QString::asprintf(" - @ %g Mpc", galaxy.m_Position.norm()),
                     Qt::darkYellow);
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        }
        m_shadLine.release();

        paintSegments(m_vboArcSegments, Qt::darkYellow, 1, Line::SOLID, false);
        paintSegments(m_vboRadius,      Qt::darkYellow, 1, Line::DASH, false);

        glRenderText(m_RaLoc.x+0.02f/m_glScalef,m_RaLoc.y+0.02f/m_glScalef, 0.0,
//                     QString::asprintf("Ra= %g", galaxy.m_Ra)+QString(DEGCHAR),
                     degTo24(galaxy.m_Ra),
                     Qt::darkYellow);
        glRenderText(m_DaLoc.x+0.02f/m_glScalef,m_DaLoc.y+0.02f/m_glScalef,m_DaLoc.z,
                     QString::asprintf("Da = %g", galaxy.m_Da)+DEGch,
                     Qt::darkYellow);
    }

    // paint the sun
    paintSphere(Vector3d(), radius, QColor(247, 177, 107), true);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dSpace::onSelGalaxy(QListWidgetItem *pItem)
{
    int ilw = m_plwGalaxies->row(pItem);
    m_iSelIndex = lwToGalaxyIndex(ilw);
    m_bResetArcs = true;
    update();
}


void gl3dSpace::onNGalaxies()
{
    s_NObjects = m_pieNGalaxies->value();
    if(s_NObjects>=m_Galaxies.size())
    {
        s_NObjects = m_Galaxies.size();
        m_pieNGalaxies->setValue(s_NObjects);
    }

    double refdist =0.000001;
    for(int i=0; i<s_NObjects; i++)
    {
        refdist = std::max(refdist, m_Galaxies.at(i).m_Position.norm());
    }

    m_bResetInstances = true;
    setReferenceLength(refdist*1.5);
    update();
}


void gl3dSpace::onObjectRadius(int size)
{
    size = std::max(size, 1);
    s_SphereRadius = double(size);
    update();
}


void gl3dSpace::makeGalaxies()
{
    QFile XFile(":/textfiles/galaxies.csv");

    if(!XFile.open(QIODevice::ReadOnly)) return;
    m_Galaxies.clear();
    m_Galaxies.reserve(1000);

    m_plwGalaxies->clear();

    QTextStream stream(&XFile);
    stream.readLine(); //field names
    int iGal=0;
    double refdist = 0.0001;

    while(!stream.atEnd())
    {
        QString line = stream.readLine().simplified();
        QStringList fields = line.split(",");
        if(fields.count()>=4)
        {
            m_Galaxies.append(Star());
            Star &galaxy = m_Galaxies.back();
            galaxy.m_Name = fields.front().simplified();
            galaxy.m_Ra = fields.at(1).toDouble(); // right ascension
            galaxy.m_Da = fields.at(2).toDouble(); // declination

            galaxy.m_Magnitude = 1;
            double dist = fields.at(3).toDouble();
            // spherical to cartesian
            galaxy.m_Position.x = dist * cos(galaxy.m_Da*PI/180.0)*cos(galaxy.m_Ra*PI/180.0);
            galaxy.m_Position.y = dist * cos(galaxy.m_Da*PI/180.0)*sin(galaxy.m_Ra*PI/180.0);
            galaxy.m_Position.z = dist * sin(galaxy.m_Da*PI/180.0);

            if(iGal<s_NObjects)
                refdist = galaxy.m_Position.norm();

            m_plwGalaxies->addItem(galaxy.m_Name);

            iGal++;
        }
    }
    m_Galaxies.squeeze();
    m_plwGalaxies->sortItems();

    m_iSelIndex = 0;
    for(int iGal=0; iGal<m_plwGalaxies->count(); iGal++)
    {
        if(m_plwGalaxies->item(iGal)->text().compare("Milky Way", Qt::CaseInsensitive)==0)
        {
            m_iSelIndex = lwToGalaxyIndex(iGal);
            break;
        }
    }

    setReferenceLength(refdist*1.5);
    reset3dScale();
}


void gl3dSpace::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSpace");
    {
        s_SphereRadius = settings.value("StarRadius", s_SphereRadius).toDouble();
        s_NObjects     = settings.value("NStars", s_NObjects).toInt();
    }
    settings.endGroup();
}


void gl3dSpace::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSpace");
    {
        settings.setValue("StarRadius", s_SphereRadius);
        settings.setValue("NStars"    , s_NObjects);
    }
    settings.endGroup();
}


void gl3dSpace::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!hasFocus()) setFocus();

    if(pEvent->buttons() || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        m_BtnPressed = Qt::NoButton;
        gl3dTestGLView::mouseMoveEvent(pEvent);
        return;
    }

    if(!m_DynTimer.isActive())
    {
        QPoint point = pEvent->pos();
        Vector3d AA, BB;

        screenToWorld(point, -1.0, AA);
        screenToWorld(point,  1.0, BB);

        QVector4D v4d;

        double dist =0.0;
        double dmax = 100.;
        float zmax = +1.e10;

        double radius = double(s_SphereRadius)/2500.0/m_glScalef;

        s_NObjects = std::min(s_NObjects, int(m_Galaxies.size()));

        m_iCloseIndex = -1;
        for(int in=0; in<s_NObjects; in++)
        {
            Star const &node = m_Galaxies.at(in);

            dist = geom::distanceToLine3d(AA, BB, node.m_Position);
            // first screening: set a max distance
            if(dist<5.0*radius && dist<dmax)
            {
                worldToScreen(node.m_Position, v4d);
                // second screening: keep the node closest to the viewer
                if(v4d.z()<zmax)
                {
                    dmax = dist;
                    zmax = v4d.z();
                    m_iCloseIndex = in;
                }
            }
        }
        update();
    }
}


void gl3dSpace::mousePressEvent(QMouseEvent *pEvent)
{
    m_BtnPressed = pEvent->button();
    if(m_BtnPressed!=Qt::LeftButton)
    {
        m_iCloseIndex = -1;
    }
    gl3dTestGLView::mousePressEvent(pEvent);
}


void gl3dSpace::mouseReleaseEvent(QMouseEvent *pEvent)
{
    /* Returns the button state when the event was generated.
     * For mouse press and double click events this includes the button that caused the event.
     * For mouse release events this excludes the button that caused the event.*/

    if(m_BtnPressed==Qt::LeftButton)
    {
        selectListItem(m_iCloseIndex);
    }

    gl3dTestGLView::mouseReleaseEvent(pEvent);
    m_BtnPressed = Qt::NoButton;
}


void gl3dSpace::selectListItem(int index)
{
    m_iSelIndex = index;
    m_plwGalaxies->setCurrentRow(galaxyToLwIndex(m_iSelIndex));

    m_bResetArcs = true;
}


int gl3dSpace::lwToGalaxyIndex(int idx) const
{
    if(idx<=0 || idx>m_plwGalaxies->count()) return -1;
    QString name = m_plwGalaxies->item(idx)->text();
    for(int iGal=0; iGal<m_Galaxies.size(); iGal++)
    {
        if(m_Galaxies.at(iGal).m_Name==name)   return iGal;
    }
    return -1;
}


int gl3dSpace::galaxyToLwIndex(int iGal) const
{
    if(iGal<0 || iGal>=m_Galaxies.size()) return -1;
    QString const &name = m_Galaxies.at(iGal).m_Name;
    for(int ilw=0; ilw<m_plwGalaxies->count(); ilw++)
    {
        if(m_plwGalaxies->item(ilw)->text().compare(name)==0) return ilw;
    }
    return -1;
}


