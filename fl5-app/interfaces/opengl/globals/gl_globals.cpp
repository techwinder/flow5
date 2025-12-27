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


#include <QtConcurrent/QtConcurrent>
#include <QVector3D>
#include <QQuaternion>

#include <interfaces/opengl/controls/colourlegend.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <core/xflcore.h>
#include <api/geom_global.h>
#include <api/nurbssurface.h>
#include <api/quaternion.h>
#include <api/mathelem.h>
#include <api/utils.h>

double t_lmin(0), t_range(0);
QVector<QVector<Segment3d>> t_futuresegs;


#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
void gl::getMemoryStatus(int &total_mem_kb, int &cur_avail_mem_kb)
{
    total_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
                  &total_mem_kb);

    cur_avail_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
                  &cur_avail_mem_kb);
}


void gl::printFormat(QSurfaceFormat const & ctxtFormat, QString &log, QString const prefix)
{
    QString strange;
    log.clear();

    strange = QString::asprintf("Context version: %d.%d\n", ctxtFormat.majorVersion(),ctxtFormat.minorVersion());
    log += prefix + strange;
    QString vendor, renderer, version, glslVersion;
    const GLubyte *p;
    if ((p = glGetString(GL_VENDOR)))
        vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_RENDERER)))
        renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_VERSION)))
        version = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
        glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

    log += prefix + "*** Context information ***\n";
    log += prefix + QString("   Vendor:         %1\n").arg(vendor).toStdString().c_str();
    log += prefix + QString("   Renderer:       %1\n").arg(renderer).toStdString().c_str();
    log += prefix + QString("   OpenGL version: %1\n").arg(version).toStdString().c_str();
    log += prefix + QString("   GLSL version:   %1\n").arg(glslVersion).toStdString().c_str();

    log += prefix + "Deprecated functions: "+xfl::boolToString(ctxtFormat.testOption(QSurfaceFormat::DeprecatedFunctions))+"\n";
    log += prefix + "Debug context:        "+xfl::boolToString(ctxtFormat.testOption(QSurfaceFormat::DebugContext))       +"\n";
    log += prefix + "Stereo buffers:       "+xfl::boolToString(ctxtFormat.testOption(QSurfaceFormat::StereoBuffers))      +"\n";

    strange = QString::asprintf("Sampling frames for antialiasing: %d\n", ctxtFormat.samples());
    log += prefix + strange;

    switch (ctxtFormat.profile()) {
        case QSurfaceFormat::NoProfile:
            log += prefix + "No Profile\n";
            break;
        case QSurfaceFormat::CoreProfile:
            log += prefix + "Core Profile\n";
            break;
        case QSurfaceFormat::CompatibilityProfile:
            log += prefix + "Compatibility Profile\n";
            break;
    }
    switch (ctxtFormat.renderableType())
    {
        case QSurfaceFormat::DefaultRenderableType:
            log += prefix + "DefaultRenderableType: The default, unspecified rendering method\n";
            break;
        case QSurfaceFormat::OpenGL:
            log += prefix + "OpenGL: Desktop OpenGL rendering\n";
            break;
        case QSurfaceFormat::OpenGLES:
            log += prefix + "OpenGLES: OpenGL ES 2.0 rendering\n";
            break;
        case QSurfaceFormat::OpenVG:
            log += prefix + "OpenVG: Open Vector Graphics rendering\n";
            break;
    }
}


void gl::printBuffer(QOpenGLBuffer &vbo, int stride)
{
    vbo.bind();
    {
        int buffersize = vbo.size();
        int count = vbo.size()/stride/int(sizeof(float));
        QVector<float> output(buffersize);
        output.fill(1.0f);
        if(!vbo.read(0, output.data(), buffersize))
            qDebug()<<"Out buffer read error";
        else
        {
            for(int i=0; i<count; i++)
            {
                QString str;
                for(int j=0; j<stride; j++)
                    str += QString::asprintf(" %11g",  output.at(stride*i+j));
                qDebug("%s", str.toStdString().c_str());
            }
            qDebug("_______________________");
        }
    }
    vbo.release();
}


GLushort gl::stipple(Line::enumLineStipple stipple)
{
    switch(stipple)
    {
        default:
        case Line::SOLID:       return 0xFFFF;
        case Line::DASH:        return 0x1F1F;
        case Line::DOT:         return 0x6666;
        case Line::DASHDOT:     return 0xFF18;
        case Line::DASHDOTDOT:  return 0x7E66;
    }
}


void gl::makeArrows(const QVector<Vector3d> &point, const QVector<Vector3d> &arrows, double coef, QOpenGLBuffer &vbo)
{
    if(!point.size() || (point.size()!=arrows.size()))
    {
        vbo.destroy();
        return;
    }

    Vector3d O;
    //The vectors defining the reference arrow
    QVector3D R(  0.0f,  0.0f,  1.00f);
    QVector3D R1( 0.05f, 0.05f, -0.15f);
    QVector3D R2(-0.05f, -0.05f, -0.15f);
    //The three vectors defining the arrow on the panel
    QVector3D P, P1, P2;

    QVector<float> ArrowVertexArray(point.size()*18);
    QVector3D N(0,0,1);// this is the vector used to define m_vboArrow

    int iv = 0;
    for (int ipt=0; ipt<point.size(); ipt++)
    {
        Vector3d const &arrow = arrows.at(ipt);
        QVector3D A(arrow.xf(), arrow.yf(), arrow.zf());
        A.normalize();
        QQuaternion qqt = QQuaternion::rotationTo(N, A);

        O.set(point.at(ipt));

        P  = qqt.rotatedVector(R);
        P1 = qqt.rotatedVector(R1);
        P2 = qqt.rotatedVector(R2);

        // Scale the pressure vector
        P  *= coef*arrow.normf();
        P1 *= coef*arrow.normf();
        P2 *= coef*arrow.normf();

        ArrowVertexArray[iv++] = O.xf();
        ArrowVertexArray[iv++] = O.yf();
        ArrowVertexArray[iv++] = O.zf();
        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();

        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = O.xf()+P.x()+P1.x();
        ArrowVertexArray[iv++] = O.yf()+P.y()+P1.y();
        ArrowVertexArray[iv++] = O.zf()+P.z()+P1.z();

        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = O.xf()+P.x()+P2.x();
        ArrowVertexArray[iv++] = O.yf()+P.y()+P2.y();
        ArrowVertexArray[iv++] = O.zf()+P.z()+P2.z();
    }
    Q_ASSERT(iv==ArrowVertexArray.size());

    vbo.create();
    vbo.bind();
    vbo.allocate(ArrowVertexArray.data(), ArrowVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeArrows(QVector<Vector2d> const &point, QVector<Vector2d> const &arrows, float zoffset, double coef, QOpenGLBuffer &vbo)
{
    if(!point.size() || (point.size()!=arrows.size()))
    {
        vbo.destroy();
        return;
    }

    Vector3d O;
    //The vectors defining the reference arrow
    QVector3D R(  0.0f,  0.0f,  1.00f);
    QVector3D R1( 0.05f, 0.05f, -0.15f);
    QVector3D R2(-0.05f, -0.05f, -0.15f);
    //The three vectors defining the arrow on the panel
    QVector3D P, P1, P2;

    QVector<float> ArrowVertexArray(point.size()*18);
    QVector3D N(0,0,1);// this is the vector used to define m_vboArrow

    int iv = 0;
    for (int ipt=0; ipt<point.size(); ipt++)
    {
        Vector2d const &arrow = arrows.at(ipt);
        QVector3D A(arrow.xf(), arrow.yf(), 0.0);
        A.normalize();
        QQuaternion qqt = QQuaternion::rotationTo(N, A);

        O.set(point.at(ipt).x, point.at(ipt).y, zoffset);

        P  = qqt.rotatedVector(R);
        P1 = qqt.rotatedVector(R1);
        P2 = qqt.rotatedVector(R2);

        // Scale the pressure vector
        P  *= coef*arrow.norm();
        P1 *= coef*arrow.norm();
        P2 *= coef*arrow.norm();

        ArrowVertexArray[iv++] = O.xf();
        ArrowVertexArray[iv++] = O.yf();
        ArrowVertexArray[iv++] = O.zf();
        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();

        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = O.xf()+P.x()+P1.x();
        ArrowVertexArray[iv++] = O.yf()+P.y()+P1.y();
        ArrowVertexArray[iv++] = O.zf()+P.z()+P1.z();

        ArrowVertexArray[iv++] = O.xf()+P.x();
        ArrowVertexArray[iv++] = O.yf()+P.y();
        ArrowVertexArray[iv++] = O.zf()+P.z();
        ArrowVertexArray[iv++] = O.xf()+P.x()+P2.x();
        ArrowVertexArray[iv++] = O.yf()+P.y()+P2.y();
        ArrowVertexArray[iv++] = O.zf()+P.z()+P2.z();
    }
    Q_ASSERT(iv==ArrowVertexArray.size());

    vbo.create();
    vbo.bind();
    vbo.allocate(ArrowVertexArray.data(), ArrowVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}



void gl::makeLines(std::vector<Vector3d> const &points, QOpenGLBuffer &vbo)
{
    // only even number of points allowed
    if(points.size()<2 || points.size()%2 !=0 )
    {
        vbo.destroy();
        return;
    }

    QVector<float> LinesVertexArray(points.size()*3);

    int iv = 0;
    for (uint ipt=0; ipt<points.size(); )
    {
        LinesVertexArray[iv++] = points.at(ipt).xf();
        LinesVertexArray[iv++] = points.at(ipt).yf();
        LinesVertexArray[iv++] = points.at(ipt).zf();
        LinesVertexArray[iv++] = points.at(ipt+1).xf();
        LinesVertexArray[iv++] = points.at(ipt+1).yf();
        LinesVertexArray[iv++] = points.at(ipt+1).zf();
        ipt +=2;
    }
    Q_ASSERT(iv==int(points.size())*3);

    vbo.create();
    vbo.bind();
    vbo.allocate(LinesVertexArray.data(), LinesVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeLineStrip(QVector<Vector3d> const &strip, QOpenGLBuffer &vbo)
{
    if(strip.size()<2)
    {
        vbo.destroy();
        return;
    }

    QVector<float> StripVertexArray(strip.size()*3);

    int iv = 0;
    for (int ipt=0; ipt<strip.size(); ipt++)
    {
        StripVertexArray[iv++] = strip.at(ipt).xf();
        StripVertexArray[iv++] = strip.at(ipt).yf();
        StripVertexArray[iv++] = strip.at(ipt).zf();
    }
    Q_ASSERT(iv==strip.size()*3);

    vbo.create();
    vbo.bind();
    vbo.allocate(StripVertexArray.data(), StripVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::make2dLineStrip(const QVector<Vector2d> &strip, float zOffset, QOpenGLBuffer &vbo)
{
    if(strip.size()<2)
    {
        vbo.destroy();
        return;
    }

    QVector<float> StripVertexArray(strip.size()*3);

    int iv = 0;
    for (int ipt=0; ipt<strip.size(); ipt++)
    {
        StripVertexArray[iv++] = strip.at(ipt).xf();
        StripVertexArray[iv++] = strip.at(ipt).yf();
        StripVertexArray[iv++] = zOffset;
    }
    Q_ASSERT(iv==strip.size()*3);

    vbo.create();
    vbo.bind();
    vbo.allocate(StripVertexArray.data(), StripVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeQuad(Quad3d const &quad, QOpenGLBuffer &vbo)
{
    gl::makeQuad(quad.vertex(0), quad.vertex(1), quad.vertex(2), quad.vertex(3), vbo);
}


/**
 * makes the VBO for a quad.
 * the vertices are provided in circular order
 */
void gl::makeQuad(Node const&V0, Node const&V1, Node const&V2, Node const&V3, QOpenGLBuffer &vbo)
{
    QVector<GLfloat> QuadVertexArray(32, 0);

    int iv = 0;
    QuadVertexArray[iv++] = V0.xf();
    QuadVertexArray[iv++] = V0.yf();
    QuadVertexArray[iv++] = V0.zf();
    QuadVertexArray[iv++] = V0.normal().xf();
    QuadVertexArray[iv++] = V0.normal().yf();
    QuadVertexArray[iv++] = V0.normal().zf();

    QuadVertexArray[iv++] = V1.xf();
    QuadVertexArray[iv++] = V1.yf();
    QuadVertexArray[iv++] = V1.zf();
    QuadVertexArray[iv++] = V1.normal().xf();
    QuadVertexArray[iv++] = V1.normal().yf();
    QuadVertexArray[iv++] = V1.normal().zf();

    QuadVertexArray[iv++] = V2.xf();
    QuadVertexArray[iv++] = V2.yf();
    QuadVertexArray[iv++] = V2.zf();
    QuadVertexArray[iv++] = V2.normal().xf();
    QuadVertexArray[iv++] = V2.normal().yf();
    QuadVertexArray[iv++] = V2.normal().zf();

    QuadVertexArray[iv++] = V3.xf();
    QuadVertexArray[iv++] = V3.yf();
    QuadVertexArray[iv++] = V3.zf();
    QuadVertexArray[iv++] = V3.normal().xf();
    QuadVertexArray[iv++] = V3.normal().yf();
    QuadVertexArray[iv++] = V3.normal().zf();

    //close the quad
    QuadVertexArray[iv++] = V0.xf();
    QuadVertexArray[iv++] = V0.yf();
    QuadVertexArray[iv++] = V0.zf();
    QuadVertexArray[iv++] = V0.normal().xf();
    QuadVertexArray[iv++] = V0.normal().yf();
    QuadVertexArray[iv++] = V0.normal().zf();

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(QuadVertexArray.data(), 30 * sizeof(GLfloat));
    vbo.release();
}


void gl::makeSegments(std::vector<Segment3d> const &segments, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    QVector<Segment3d> qVec(segments.size());
    for(uint i=0; i<segments.size(); i++)
        qVec[i] = segments.at(i);
    makeSegments(qVec, pos, vbo);
}


void gl::makeSegments(QVector<Segment3d> const &segments, Vector3d const &pos, QOpenGLBuffer &vbo)
{
    int bufferSize = segments.size();
    bufferSize *=2;    // 2 vertices for each segment
    bufferSize *=3;    // (3 coords) for each node

    QVector<float> meshvertexarray(bufferSize);

    int iv = 0;
    for(int it=0; it<segments.size(); it++)
    {
        Segment3d const &seg = segments.at(it);
        meshvertexarray[iv++] = seg.vertexAt(0).xf() + pos.xf();
        meshvertexarray[iv++] = seg.vertexAt(0).yf() + pos.yf();
        meshvertexarray[iv++] = seg.vertexAt(0).zf() + pos.zf();
        meshvertexarray[iv++] = seg.vertexAt(1).xf() + pos.xf();
        meshvertexarray[iv++] = seg.vertexAt(1).yf() + pos.yf();
        meshvertexarray[iv++] = seg.vertexAt(1).zf() + pos.zf();
    }

    Q_ASSERT(meshvertexarray.size()==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriangles3Vtx(QVector<Triangle2d> const &triangles, float z, QOpenGLBuffer &vbo)
{
    //Make surface triangulation
    int bufferSize = triangles.count();
    bufferSize *= 3;    // 4 vertices for each triangle
    bufferSize *= 6;    // (3 coords+3 normal components) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N(0,0,1);

    int iv = 0;
    for(int it=0; it<triangles.size(); it++)
    {
        Triangle2d const &t2d = triangles.at(it);

        meshvertexarray[iv++] = t2d.vertexAt(0).xf();
        meshvertexarray[iv++] = t2d.vertexAt(0).yf();
        meshvertexarray[iv++] = z;
        meshvertexarray[iv++] = N.xf();
        meshvertexarray[iv++] = N.yf();
        meshvertexarray[iv++] = N.zf();

        meshvertexarray[iv++] = t2d.vertexAt(1).xf();
        meshvertexarray[iv++] = t2d.vertexAt(1).yf();
        meshvertexarray[iv++] = z;
        meshvertexarray[iv++] = N.xf();
        meshvertexarray[iv++] = N.yf();
        meshvertexarray[iv++] = N.zf();

        meshvertexarray[iv++] = t2d.vertexAt(2).xf();
        meshvertexarray[iv++] = t2d.vertexAt(2).yf();
        meshvertexarray[iv++] = z;
        meshvertexarray[iv++] = N.xf();
        meshvertexarray[iv++] = N.yf();
        meshvertexarray[iv++] = N.zf();
    }

    Q_ASSERT(iv==bufferSize);

    if(vbo.isCreated()) vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTrianglesOutline(QVector<Triangle2d> const &triangles, const Vector2d &position, QOpenGLBuffer &vbo)
{
    int nPanel3 = triangles.size();
    // vertices array size:
    //        n triangular Panels
    //      x3 edges
    //      x2 nodes per edges
    //        x3 vertex components

    int buffersize = nPanel3*3*2*3;

    QVector<float> nodeVertexArray(buffersize);

    int iv = 0;
    for (int i3=0; i3<nPanel3; i3++)
    {
        Triangle2d const &t2d = triangles.at(i3);

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = t2d.vertexAt(i).xf() + position.xf();
            nodeVertexArray[iv++] = t2d.vertexAt(i).yf() + position.yf();
            nodeVertexArray[iv++] = 0.0f;

            nodeVertexArray[iv++] = t2d.vertexAt(i+1).xf() + position.xf();
            nodeVertexArray[iv++] = t2d.vertexAt(i+1).yf() + position.yf();
            nodeVertexArray[iv++] = 0.0f;
        }
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


/**
 * If bFlatNormal is true, use the Triangles's normal at each vertex. Valid for flat type FuseXfl.
 * Else use the pre-computed normals
 */
void gl::makeTriangles3Vtx(std::vector<Triangle3d> const &triangles, bool bFlatNormals, QOpenGLBuffer &vbo)
{
    //Make surface triangulation
    int bufferSize = int(triangles.size());
    bufferSize *= 3;    // 3 vertices for each triangle
    bufferSize *= 6;    // (3 coords+3 normal components) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N;

    int iv = 0;
    for(uint it=0; it<triangles.size(); it++)
    {
        Triangle3d const &t3d = triangles.at(it);
        N.set(t3d.normal());

        meshvertexarray[iv++] = t3d.vertexAt(0).xf();
        meshvertexarray[iv++] = t3d.vertexAt(0).yf();
        meshvertexarray[iv++] = t3d.vertexAt(0).zf();
        if(bFlatNormals)
        {
            meshvertexarray[iv++] = N.xf();
            meshvertexarray[iv++] = N.yf();
            meshvertexarray[iv++] = N.zf();
        }
        else
        {
            meshvertexarray[iv++] = t3d.vertexAt(0).normal().xf();
            meshvertexarray[iv++] = t3d.vertexAt(0).normal().yf();
            meshvertexarray[iv++] = t3d.vertexAt(0).normal().zf();
        }

        meshvertexarray[iv++] = t3d.vertexAt(1).xf();
        meshvertexarray[iv++] = t3d.vertexAt(1).yf();
        meshvertexarray[iv++] = t3d.vertexAt(1).zf();
        if(bFlatNormals)
        {
            meshvertexarray[iv++] = N.xf();
            meshvertexarray[iv++] = N.yf();
            meshvertexarray[iv++] = N.zf();
        }
        else
        {
            meshvertexarray[iv++] = t3d.vertexAt(1).normal().xf();
            meshvertexarray[iv++] = t3d.vertexAt(1).normal().yf();
            meshvertexarray[iv++] = t3d.vertexAt(1).normal().zf();
        }


        meshvertexarray[iv++] = t3d.vertexAt(2).xf();
        meshvertexarray[iv++] = t3d.vertexAt(2).yf();
        meshvertexarray[iv++] = t3d.vertexAt(2).zf();
        if(bFlatNormals)
        {
            meshvertexarray[iv++] = N.xf();
            meshvertexarray[iv++] = N.yf();
            meshvertexarray[iv++] = N.zf();
        }
        else
        {
            meshvertexarray[iv++] = t3d.vertexAt(2).normal().xf();
            meshvertexarray[iv++] = t3d.vertexAt(2).normal().yf();
            meshvertexarray[iv++] = t3d.vertexAt(2).normal().zf();
        }
    }

    Q_ASSERT(iv==bufferSize);

    if(vbo.isCreated()) vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTrianglesOutline(std::vector<Triangle3d> const &triangles, const Vector3d &position, QOpenGLBuffer &vbo)
{
    int nPanel3 = int(triangles.size());
    // vertices array size:
    //        n triangular Panels
    //      x3 edges
    //      x2 nodes per edges
    //        x3 vertex components

    int buffersize = nPanel3*3*2*3;

    QVector<float> nodeVertexArray(buffersize);

    int iv = 0;
    for (int i3=0; i3<nPanel3; i3++)
    {
        Triangle3d const &t3d = triangles.at(i3);

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = t3d.vertexAt(i).xf() + position.xf();
            nodeVertexArray[iv++] = t3d.vertexAt(i).yf() + position.yf();
            nodeVertexArray[iv++] = t3d.vertexAt(i).zf() + position.zf();

            nodeVertexArray[iv++] = t3d.vertexAt(i+1).xf() + position.xf();
            nodeVertexArray[iv++] = t3d.vertexAt(i+1).yf() + position.yf();
            nodeVertexArray[iv++] = t3d.vertexAt(i+1).zf() + position.zf();
        }
    }

    Q_ASSERT(iv==buffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


/**
 * If bFlatNormal is true, use the Triangles's normal at each vertex. Valid for flat type FuseXfl.
 * Else use the pre-computed normals
 */
void gl::makeTriangulation3Vtx(Triangulation const & triangulation, Vector3d const &pos, QOpenGLBuffer &vbo, bool bFlatNormals)
{
    //Make surface triangulation
    int bufferSize = triangulation.nTriangles();
    bufferSize *= 3;    // 3 vertices for each triangle
    bufferSize *= 6;    // (3 coords+3 normal components) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N;
    int inode=0;
    int iv = 0;
    for(int it=0; it<triangulation.nTriangles(); it++)
    {
        Triangle3d const &t3 = triangulation.triangleAt(it);
        meshvertexarray[iv++] = t3.vertexAt(0).xf()+pos.xf();
        meshvertexarray[iv++] = t3.vertexAt(0).yf()+pos.yf();
        meshvertexarray[iv++] = t3.vertexAt(0).zf()+pos.zf();
        inode = t3.nodeIndex(0);
        if(bFlatNormals) N = t3.normal();
        else if(inode>=0 && inode<triangulation.nNodes()) N = triangulation.nodeAt(inode).normal();
        else N = t3.normal();
        meshvertexarray[iv++] = N.xf();
        meshvertexarray[iv++] = N.yf();
        meshvertexarray[iv++] = N.zf();

        meshvertexarray[iv++] = t3.vertexAt(1).xf()+pos.xf();
        meshvertexarray[iv++] = t3.vertexAt(1).yf()+pos.yf();
        meshvertexarray[iv++] = t3.vertexAt(1).zf()+pos.zf();
        inode = t3.nodeIndex(1);
        if(bFlatNormals) N = t3.normal();
        else if(inode>=0 && inode<triangulation.nNodes()) N = triangulation.nodeAt(inode).normal();
        else N = t3.normal();
        meshvertexarray[iv++] = N.xf();
        meshvertexarray[iv++] = N.yf();
        meshvertexarray[iv++] = N.zf();

        meshvertexarray[iv++] = t3.vertexAt(2).xf()+pos.xf();
        meshvertexarray[iv++] = t3.vertexAt(2).yf()+pos.yf();
        meshvertexarray[iv++] = t3.vertexAt(2).zf()+pos.zf();
        inode = t3.nodeIndex(2);
        if(bFlatNormals) N = t3.normal();
        else if(inode>=0 && inode<triangulation.nNodes()) N = triangulation.nodeAt(inode).normal();
        else N = t3.normal();
        meshvertexarray[iv++] = N.xf();
        meshvertexarray[iv++] = N.yf();
        meshvertexarray[iv++] = N.zf();
    }

    Q_ASSERT(iv==bufferSize);

    if(vbo.isCreated()) vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriangleNormals(std::vector<Triangle3d> const &trianglelist, float coef, QOpenGLBuffer &vbo)
{
    if(fabsf(coef)<1.0e-3f) coef = 1.0f;

    // vertices array size:
    //        n  Panels
    //      x2 nodes per normal
    //        x6 = 3 components
    int nodeVertexSize = int(trianglelist.size()) * 2 * 3;
    QVector<float>nodeVertexArray(nodeVertexSize, 0);

    int iv=0;

    for(uint p=0; p<trianglelist.size(); p++)
    {
        Triangle3d const &t3d = trianglelist.at(p);
        nodeVertexArray[iv++] = t3d.CoG_g().xf();
        nodeVertexArray[iv++] = t3d.CoG_g().yf();
        nodeVertexArray[iv++] = t3d.CoG_g().zf();

        nodeVertexArray[iv++] = t3d.CoG_g().xf() + t3d.normal().xf()*coef;
        nodeVertexArray[iv++] = t3d.CoG_g().yf() + t3d.normal().yf()*coef;
        nodeVertexArray[iv++] = t3d.CoG_g().zf() + t3d.normal().zf()*coef;
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeTriangleNodeNormals(std::vector<Triangle3d> const &trianglelist, float coef, QOpenGLBuffer &vbo)
{
    if(fabsf(coef)<1.0e-3f) coef = 1.0f;

    // vertices array size:
    //        n  Panels
    //      x3 vertices
    //      x2 nodes per normal
    //        x3 = 3 components
    int nodeVertexSize = int(trianglelist.size()) * 3 * 2 * 3;
    QVector<float>nodeVertexArray(nodeVertexSize, 0);

    int iv=0;

    for(uint p=0; p<trianglelist.size(); p++)
    {
        Triangle3d const &t3d = trianglelist.at(p);
        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Node const &nd = t3d.vertexAt(ivtx);
            nodeVertexArray[iv++] = nd.xf();
            nodeVertexArray[iv++] = nd.yf();
            nodeVertexArray[iv++] = nd.zf();

            nodeVertexArray[iv++] = nd.xf() + nd.normal().xf()*coef;
            nodeVertexArray[iv++] = nd.yf() + nd.normal().yf()*coef;
            nodeVertexArray[iv++] = nd.zf() + nd.normal().zf()*coef;
        }
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeNodeNormals(std::vector<Node> const &nodelist, Vector3d const &pos, float coef, QOpenGLBuffer &vbo)
{
    // vertices array size:
    //        n  nodes
    //      x2 nodes per normal
    //        x6 = 3 vertex components

    int nodeVertexSize = int(nodelist.size())* 2 * 3;
    QVector<float>nodeVertexArray(nodeVertexSize, 0);

    int iv=0;

    for(uint p=0; p<nodelist.size(); p++)
    {
        nodeVertexArray[iv++] = nodelist.at(p).xf() + pos.xf();
        nodeVertexArray[iv++] = nodelist.at(p).yf() + pos.yf();
        nodeVertexArray[iv++] = nodelist.at(p).zf() + pos.zf();

        nodeVertexArray[iv++] = nodelist.at(p).xf() + pos.xf() + nodelist.at(p).normal().xf()*coef;
        nodeVertexArray[iv++] = nodelist.at(p).yf() + pos.yf() + nodelist.at(p).normal().yf()*coef;
        nodeVertexArray[iv++] = nodelist.at(p).zf() + pos.zf() + nodelist.at(p).normal().zf()*coef;
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


/** makes a unit arrow point in direction D*/
void gl::makeArrow(Vector3d const &start, Vector3d const &end, QOpenGLBuffer &vbo)
{
    QVector<GLfloat> ArrowVertexArray(18);

    Vector3d Arrow = end-start;
    double amplitude = Arrow.norm();

    int iv = 0;

    Quaternion Qt; // Quaternion operator to align the reference arrow in the direction D
    Vector3d Omega; // rotation vector to align the reference arrow with D
    Vector3d O = start;
    //The vectors defining the reference arrow
    Vector3d R(  0.0,  0.0, 1.0);
    Vector3d R1( 0.05, 0.0, -0.1);
    Vector3d R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    double cosa   = R.dot(Arrow);
    double sina2  = sqrt((1.0 - cosa)*0.5);
    double cosa2  = sqrt((1.0 + cosa)*0.5);

    Omega = R * end;//crossproduct
    Omega.normalize();
    Omega *=sina2;
    Qt.set(cosa2, Omega.x, Omega.y, Omega.z);

    Qt.conjugate(R,  P);
    Qt.conjugate(R1, P1);
    Qt.conjugate(R2, P2);

    P  *= amplitude;
    P1 *= amplitude;
    P2 *= amplitude;

    ArrowVertexArray[iv++] = O.xf();
    ArrowVertexArray[iv++] = O.yf();
    ArrowVertexArray[iv++] = O.zf();

    ArrowVertexArray[iv++] = O.xf()+P.xf();
    ArrowVertexArray[iv++] = O.yf()+P.yf();
    ArrowVertexArray[iv++] = O.zf()+P.zf();

    ArrowVertexArray[iv++] = O.xf()+P.xf();
    ArrowVertexArray[iv++] = O.yf()+P.yf();
    ArrowVertexArray[iv++] = O.zf()+P.zf();
    ArrowVertexArray[iv++] = O.xf()+P.xf()+P1.xf();
    ArrowVertexArray[iv++] = O.yf()+P.yf()+P1.yf();
    ArrowVertexArray[iv++] = O.zf()+P.zf()+P1.zf();

    ArrowVertexArray[iv++] = O.xf()+P.xf();
    ArrowVertexArray[iv++] = O.yf()+P.yf();
    ArrowVertexArray[iv++] = O.zf()+P.zf();
    ArrowVertexArray[iv++] = O.xf()+P.xf()+P2.xf();
    ArrowVertexArray[iv++] = O.yf()+P.yf()+P2.yf();
    ArrowVertexArray[iv++] = O.zf()+P.zf()+P2.zf();

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArrowVertexArray.constData(), 18 * sizeof(GLfloat));
    vbo.release();
}


void gl::makeNurbsOutline(NURBSSurface const &nurbs, Vector3d const &pos, int NX, int NZ, QOpenGLBuffer &vbo)
{
    Vector3d Point, Point1;

    //OUTLINE
    //     frameSize()*(NH+1) : frames
    //     (NX+1) + (NX+1)    : top and bottom lines
    //
    int nsegs =   nurbs.frameCount()*NX // frames
                + NZ                       // top outline
                + NZ;                      // bot outline
    int outlinesize =  nsegs * 2 * 3; // x2vertices x3 vertices components

    QVector<float> OutlineVertexArray(outlinesize);
    std::vector<double> fraclist;
    xfl::getPointDistribution(fraclist, NX, xfl::COSINE);

    double u(0), v(0), v1(0);

    int iv(0);
    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<nurbs.frameCount(); iFr++)
    {
        u = nurbs.getu(nurbs.frameAt(iFr).position().z);
        for(uint j=0; j<fraclist.size()-1; j++)
        {
            v  = fraclist.at(j);
            v1 = fraclist.at(j+1);
            nurbs.getPoint(u,v, Point);
            nurbs.getPoint(u,v1,Point1);
            OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
            OutlineVertexArray[iv++] = Point.yf()  + pos.yf();
            OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
            OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
            OutlineVertexArray[iv++] = Point1.yf() + pos.yf();
            OutlineVertexArray[iv++] = Point1.zf() + pos.zf();
        }
    }

    //top line: NX+1
    v = 0.0;
    for (int iu=0; iu<NZ; iu++)
    {
        nurbs.getPoint(double(iu)  /double(NZ),v, Point);
        nurbs.getPoint(double(iu+1)/double(NZ),v, Point1);
        OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
        OutlineVertexArray[iv++] = Point.yf()  + pos.yf();
        OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
        OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
        OutlineVertexArray[iv++] = Point1.yf() + pos.yf();
        OutlineVertexArray[iv++] = Point1.zf() + pos.zf();
    }

    //bottom line: NX+1
    v = 1.0;
    for (int iu=0; iu<NZ; iu++)
    {
        nurbs.getPoint(double(iu)  /double(NZ),v, Point);
        nurbs.getPoint(double(iu+1)/double(NZ),v, Point1);
        OutlineVertexArray[iv++] = Point.xf()  + pos.xf();
        OutlineVertexArray[iv++] = Point.yf()  + pos.yf();
        OutlineVertexArray[iv++] = Point.zf()  + pos.zf();
        OutlineVertexArray[iv++] = Point1.xf() + pos.xf();
        OutlineVertexArray[iv++] = Point1.yf() + pos.yf();
        OutlineVertexArray[iv++] = Point1.zf() + pos.zf();
    }
    Q_ASSERT(iv==outlinesize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(OutlineVertexArray.data(), outlinesize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeFrameHighlight(NURBSSurface const &nurbs, Vector3d const &nurbspos, QOpenGLBuffer &vbo)
{
    int iFrame = nurbs.activeFrameIndex();
    if(iFrame<0 || iFrame>=nurbs.frameCount())
    {
        vbo.destroy();
        return;
    }
//    int NXXXX = W3dPrefs::bodyAxialRes();
    int NX = 50;
    int k;
    Vector3d Point;
    double hinc, u, v;
    if(iFrame<0) return;

    Frame const &frame = nurbs.frameAt(iFrame);
//    xinc = 0.1;
    hinc = 1.0/double(NX-1);
    int bufferSize = NX *3 ;
    QVector<float> pHighlightVertexArray(bufferSize);

    int iv = 0;

    u = nurbs.getu(frame.position().z);
    v = 0.0;
    for (k=0; k<NX; k++)
    {
        nurbs.getPoint(u,v,Point);
        pHighlightVertexArray[iv++] = Point.xf()+nurbspos.xf();
        pHighlightVertexArray[iv++] = Point.yf()+nurbspos.yf();
        pHighlightVertexArray[iv++] = Point.zf()+nurbspos.zf();
        v += hinc;
    }

    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    vbo.release();
}


/** https://en.wikipedia.org/wiki/Marching_squares#Contouring_triangle_meshes */
void gl::lookUpTriangularKey(int key, int &i0, int &i1, int &i2, int &i3)
{
    /* 2^3 = 8 crossover configurations
    //
    //         N2                 2^2
    //        | \                 | \
    //       |   \               |   \
    //      |     \             |     \
    //     |       \           |       \
    //    N0-------N1         2^0------2^1
    */
    switch(key)
    {
        case 0: i0=i1=i2=i3=-1;  break; // all values below threshold
        case 1:  // 2^0
        {
            i0=0;
            i1=1;

            i2=0;
            i3=2;
            break;
        }
        case 2:  // 2^1
        {
            i0=0;
            i1=1;

            i2=1;
            i3=2;
            break;
        }
        case 3:  // =2^0 + 2^1
        {
            i0=1;
            i1=2;

            i2=2;
            i3=0;
            break;
        }
        case 4:  // =2^2
        {
            i0=1;
            i1=2;

            i2=2;
            i3=0;
            break;
        }
        case 5:  // =2^0 + 2^2
        {
            i0=0;
            i1=1;

            i2=1;
            i3=2;

            break;
        }
        case 6:  // =2^1 + 2^2
        {
            i0=0;
            i1=1;

            i2=2;
            i3=0;

            break;
        }

        case 7: i0=i1=i2=i3=-1;  break;// =2^0 + 2^1 + 2^2 - all vertices above threshold
    }

}


void gl::lookUpQuadKey(int key, int *i)
{
    // 2^4 = 16 crossover configurations
    //
    //    N3------N2           2^3------2^2
    //    |       |             |       |
    //    |       |             |       |
    //    |       |             |       |
    //    N0------N1           2^0------2^1
    //
    i[0] = i[1] = i[2] = i[3] = i[4] = i[5] = i[6] = i[7] = -1;

    switch(key)
    {
        case 0: break;
        case 1:  // 2^0
        {
            i[0]=0;
            i[1]=1;

            i[2]=0;
            i[3]=3;
            break;
        }
        case 2:  // 2^1
        {
            i[0]=0;
            i[1]=1;

            i[2]=1;
            i[3]=2;
            break;
        }
        case 3:  // =2^0 + 2^1
        {
            i[0]=0;
            i[1]=3;

            i[2]=1;
            i[3]=2;
            break;
        }
        case 4:  // =2^2
        {
            i[0]=1;
            i[1]=2;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 5:  // =2^0 + 2^2
        {
            i[0]=0;
            i[1]=1;

            i[2]=3;
            i[3]=0;

            i[4]=1;
            i[5]=2;

            i[6]=2;
            i[7]=3;
            break;
        }
        case 6:  // =2^1 + 2^2
        {
            i[0]=0;
            i[1]=1;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 7:  // =2^0 + 2^1 + 2^2
        {
            i[0]=2;
            i[1]=3;

            i[2]=3;
            i[3]=0;
            break;
        }
        case 8:  // =2^3
        {
            i[0]=2;
            i[1]=3;

            i[2]=3;
            i[3]=0;
            break;

        }
        case 9:  // =2^0 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 10: // =2^1 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=1;
            i[3]=2;

            i[4]=0;
            i[5]=3;

            i[6]=2;
            i[7]=3;
            break;
        }
        case 11: // =2^0 + 2^1 + 2^3
        {
            i[0]=1;
            i[1]=2;

            i[2]=2;
            i[3]=3;
            break;
        }
        case 12: // =2^2 + 2^3
        {
            i[0]=0;
            i[1]=3;

            i[2]=1;
            i[3]=2;

            break;
        }
        case 13: // =2^0 + 2^2 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=1;
            i[3]=2;
            break;
        }
        case 14: // =2^1 + 2^2 + 2^3
        {
            i[0]=0;
            i[1]=1;

            i[2]=0;
            i[3]=3;
            break;
        }
        case 15: break;// =2^0 + 2^1 + 2^2 + 2^3
    }
}


void gl::makeNodeForces(std::vector<Node> const &nodes, std::vector<double> const &CpNodes,
                        float qDyn,
                        double &rmin, double &rmax, bool bAuto, double scale,
                        QOpenGLBuffer &vbo)
{
    int nNodes = int(nodes.size());
    if(nNodes==0 || nNodes>int(CpNodes.size()))
    {
        vbo.destroy();
        return;
    }

    Quaternion Qt; // Quaternion operator to align the reference arrow with the panel's normal
    Vector3d Omega; // rotation vector to align the reference arrow with the panel's normal
//for(int in=0; in<nNodes; in++) qDebug(" %3d   %13g", nodes.at(in).index(), CpNodes.at(in));

    //The vectors defining the reference arrow
    Vector3d Ra(0.0,0.0,1.0);
    Vector3d R1( 0.05, 0.0, -0.1);
    Vector3d R2(-0.05, 0.0, -0.1);
    //The three vectors defining the arrow on the panel
    Vector3d P, P1, P2;

    //define the range of values to set the colors in accordance

    float coef = 0.00001f;

    if(bAuto)
    {
        rmin = 1.e10;
        rmax = -rmin;
        for (int in=0; in<nNodes; in++)
        {
            int index = nodes.at(in).index();
            if(index<int(CpNodes.size()))
            {
                rmax = std::max(rmax, CpNodes[index]);
                rmin = std::min(rmin, CpNodes[index]);
            }
        }

        rmin *= qDyn;
        rmax *= qDyn;
    }

    float range = rmax - rmin;

    // vertices array size:
    //        nNodes x 1 arrow
    //      x3 lines per arrow
    //      x2 vertices per line
    //        x6 = 3 vertex components + 3 color components

    int forceVertexSize = nNodes * 3 * 2 * 6;
    QVector<float> forceVertexArray(forceVertexSize);
    QColor clr;
    int iv=0;
    for (int p=0; p<nNodes; p++)
    {
        Node const &node = nodes.at(p);
        if(node.index()>=int(CpNodes.size())) break;

        float force = qDyn * float(CpNodes[node.index()]);
        float tau = (force-rmin)/range;
        //scale force for display
        force *= scale *coef;

        clr = ColourLegend::colour(tau);
        float r = clr.redF();
        float g = clr.greenF();
        float b = clr.blueF();

        Qt.from2UnitVectors(Ra, node.normal());
        Qt.conjugate(Ra,  P);
        Qt.conjugate(R1, P1);
        Qt.conjugate(R2, P2);

        // Scale the pressure vector
        P  *= double(force);
        P1 *= double(force);
        P2 *= double(force);

        if(CpNodes[node.index()]>0)
        {
            // compression, point towards the surface
            forceVertexArray[iv++] = node.xf();
            forceVertexArray[iv++] = node.yf();
            forceVertexArray[iv++] = node.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = node.xf()+P.xf();
            forceVertexArray[iv++] = node.yf()+P.yf();
            forceVertexArray[iv++] = node.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = node.xf();
            forceVertexArray[iv++] = node.yf();
            forceVertexArray[iv++] = node.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = node.xf()-P1.xf();
            forceVertexArray[iv++] = node.yf()-P1.yf();
            forceVertexArray[iv++] = node.zf()-P1.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = node.xf();
            forceVertexArray[iv++] = node.yf();
            forceVertexArray[iv++] = node.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = node.xf()-P2.xf();
            forceVertexArray[iv++] = node.yf()-P2.yf();
            forceVertexArray[iv++] = node.zf()-P2.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
        }
        else
        {
            // depression, point outwards from the surface
            P.set(-P.x, -P.y, -P.z);

            forceVertexArray[iv++] = node.xf();
            forceVertexArray[iv++] = node.yf();
            forceVertexArray[iv++] = node.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = node.xf()+P.xf();
            forceVertexArray[iv++] = node.yf()+P.yf();
            forceVertexArray[iv++] = node.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = node.xf()+P.xf();
            forceVertexArray[iv++] = node.yf()+P.yf();
            forceVertexArray[iv++] = node.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = node.xf()+P.xf()-P1.xf();
            forceVertexArray[iv++] = node.yf()+P.yf()-P1.yf();
            forceVertexArray[iv++] = node.zf()+P.zf()-P1.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;

            forceVertexArray[iv++] = node.xf()+P.xf();
            forceVertexArray[iv++] = node.yf()+P.yf();
            forceVertexArray[iv++] = node.zf()+P.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
            forceVertexArray[iv++] = node.xf()+P.xf()-P2.xf();
            forceVertexArray[iv++] = node.yf()+P.yf()-P2.yf();
            forceVertexArray[iv++] = node.zf()+P.zf()-P2.zf();
            forceVertexArray[iv++] = r;
            forceVertexArray[iv++] = g;
            forceVertexArray[iv++] = b;
        }
    }

//    Q_ASSERT(iv==forceVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(forceVertexArray.data(), forceVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}



#define PIf    3.141592654f
#define NPOINTS 300
void gl::makeCircle(double radius, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS-1; // 1 segment less than the number of points
    arcbuffersize *= 2; // two vertices per segment
    arcbuffersize *= 3; // three components per vertex

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);


    int iv = 0;

    for(int i=0; i<NPOINTS-1; i++)
    {
        float theta  = float(i)   * 2.0f*PI/float(NPOINTS-1);
        float theta1 = float(i+1) * 2.0f*PIf/float(NPOINTS-1);
        ArcVertexArray[iv++] = O.xf()+radius*cosf(theta);
        ArcVertexArray[iv++] = O.yf()+radius*sinf(theta);
        ArcVertexArray[iv++] = O.zf();
        ArcVertexArray[iv++] = O.xf()+radius*cosf(theta1);
        ArcVertexArray[iv++] = O.yf()+radius*sinf(theta1);
        ArcVertexArray[iv++] = O.zf();
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


/* triangle strip */
void gl::makeUnitDisk(int nTriangles, QOpenGLBuffer &vbo)
{
    QVector<Triangle2d> triangles(nTriangles);

    float th1(0);
    float costh0(0), costh1(0), sinth0(0), sinth1(0);
    costh0 = cosf(0.0f);
    sinth0 = sinf(0.0f);
    for(int i=0; i<nTriangles; i++)
    {
        Triangle2d &t2d = triangles[i];
        th1 = float(i)/float(nTriangles-1)*2.0f*PIf;
        costh1 = cosf(th1);
        sinth1 = sinf(th1);

        t2d.setTriangle({0,0},{costh0, sinth0}, {costh1, sinth1});

        costh0 = costh1;
        sinth0 = sinth1;
    }

    gl::makeTriangles3Vtx(triangles, 0.0f, vbo);
}


void gl::makeDisk(double radius, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS;
    arcbuffersize += 1; // 1 central vertex
    arcbuffersize *= 6; // 3+3 components per vertex

    // NPOINTS-1 triangles

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);

    int iv = 0;
    //set the fixed central vertex
    ArcVertexArray[iv++] = O.xf();
    ArcVertexArray[iv++] = O.yf();
    ArcVertexArray[iv++] = O.zf();
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 1.0f;
    for(int i=0; i<NPOINTS; i++)
    {
        float theta  = float(i)   * 2.0f*PI/float(NPOINTS-1);
        ArcVertexArray[iv++] = O.xf()+radius*cosf(theta);
        ArcVertexArray[iv++] = O.yf()+radius*sinf(theta);
        ArcVertexArray[iv++] = O.zf();
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 1.0f;
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


/**
 * @param a is the half-length of the major axis
 * @param e is the excentricity
 */
void gl::makeEllipseLineStrip(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS;
    arcbuffersize *= 3; // 3 components per vertex for the surface shader

    // NPOINTS-1 triangles

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);

    Vector3d C(O.x+a*e, O.y, 0.0);
    double b = a * sqrt(1.0-e*e);
    int iv = 0;

    for(int i=0; i<NPOINTS; i++)
    {
        double t = float(i)/float(NPOINTS-1);
        float x = float(a * cos(2.0*PI*t));
        float y = float(b * sin(2.0*PI*t));
        ArcVertexArray[iv++] = C.xf()+x;
        ArcVertexArray[iv++] = C.yf()+y;
        ArcVertexArray[iv++] = C.zf();
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


/**
 * @param a is the half-length of the major axis
 * @param e is the excentricity
 */
void gl::makeEllipseFan(double a, double e, Vector3d const &O, QOpenGLBuffer &vbo)
{
    int arcbuffersize = NPOINTS;
    arcbuffersize += 1; // 1 central vertex at the focus point
    arcbuffersize *= 6; // 3+3 components per vertex for the surface shader

    // NPOINTS-1 triangles

    QVector<GLfloat> ArcVertexArray(arcbuffersize, 0);

    Vector3d C(O.x+a*e, O.y, 0.0);
    double b = a * sqrt(1.0-e*e);
    int iv = 0;
    //set the fixed central vertex at the focus point
    ArcVertexArray[iv++] = C.xf();
    ArcVertexArray[iv++] = C.yf();
    ArcVertexArray[iv++] = C.zf();
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 0.0f;
    ArcVertexArray[iv++] = 1.0f;

    for(int i=0; i<NPOINTS; i++)
    {
        double t = float(i)/float(NPOINTS-1);
        float x = float(a * cos(2.0*PI*t));
        float y = float(b * sin(2.0*PI*t));
        ArcVertexArray[iv++] = C.xf()+x;
        ArcVertexArray[iv++] = C.yf()+y;
        ArcVertexArray[iv++] = C.zf();
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 0.0f;
        ArcVertexArray[iv++] = 1.0f;
    }

    Q_ASSERT(iv==arcbuffersize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(ArcVertexArray.data(), arcbuffersize * sizeof(GLfloat));
    vbo.release();
}


void gl::makeCpSection(std::vector<Node> const&pts, std::vector<double> const&Cp, double coef, std::vector<Node> &cpsections)
{
    cpsections.resize(pts.size());
    for(uint i=0; i<pts.size(); i++)
    {
        if(Cp.at(i)<0)
        {
            cpsections[i] = pts.at(i);
            cpsections[i].setNormal(pts.at(i).normal()*-Cp.at(i)*coef);
        }
        else
        {
            // opposite arrow pointing towards the surface
            cpsections[i] = pts.at(i) + pts[i].normal()*Cp.at(i)*coef;
            cpsections[i].setNormal(pts.at(i).normal()*-Cp.at(i)*coef);
        }
    }
}


void gl::makeTetra(Vector3d const &pt, double side, QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges)
{
    Vector3d vtx[4];
    vtx[0] = { sqrt(8.0/9.0)*side,                   0, -1.0/3.0*side};
    vtx[1] = {-sqrt(2.0/9.0)*side,  sqrt(2.0/3.0)*side, -1.0/3.0*side};
    vtx[2] = {-sqrt(2.0/9.0)*side, -sqrt(2.0/3.0)*side, -1.0/3.0*side};
    vtx[3] = { 0.0,                                0.0,          side};

    for(int i=0; i<4; i++) vtx[i] += pt;

    Vector3d N;

    GLfloat b[72];
    int iv=0;
    int i(0), j(2), k(1);
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    i=0; j=3; k=2;
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    i=2; j=3; k=1;
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    i=1; j=3; k=0;
    N = (vtx[j]-vtx[i])*(vtx[k]-vtx[i]).normalized();
    for(int l=0; l<3; l++) b[iv++]=float(vtx[i][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[j][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);
    for(int l=0; l<3; l++) b[iv++]=float(vtx[k][l]);
    for(int l=0; l<3; l++) b[iv++]=float(N[l]);

    if(vboFaces.isCreated()) vboFaces.destroy(); // recreate it in the VAO
    vboFaces.create();
    vboFaces.bind();
    {
        vboFaces.allocate(b, 72*sizeof(float));
    }
    vboFaces.release();

    int segsize = 4 * 2 *3; //4 segments x 2 vertices x3 components;
    iv=0;
    // first
    b[iv++] = vtx[0].xf();
    b[iv++] = vtx[0].yf();
    b[iv++] = vtx[0].zf();
    b[iv++] = vtx[1].xf();
    b[iv++] = vtx[1].yf();
    b[iv++] = vtx[1].zf();
    // second
    b[iv++] = vtx[1].xf();
    b[iv++] = vtx[1].yf();
    b[iv++] = vtx[1].zf();
    b[iv++] = vtx[2].xf();
    b[iv++] = vtx[2].yf();
    b[iv++] = vtx[2].zf();
    // third
    b[iv++] = vtx[2].xf();
    b[iv++] = vtx[2].yf();
    b[iv++] = vtx[2].zf();
    b[iv++] = vtx[3].xf();
    b[iv++] = vtx[3].yf();
    b[iv++] = vtx[3].zf();
    // fourth
    b[iv++] = vtx[0].xf();
    b[iv++] = vtx[0].yf();
    b[iv++] = vtx[0].zf();
    b[iv++] = vtx[3].xf();
    b[iv++] = vtx[3].yf();
    b[iv++] = vtx[3].zf();

    if(vboEdges.isCreated()) vboEdges.destroy(); // recreate it in the VAO
    vboEdges.create();
    vboEdges.bind();
    {
        vboEdges.allocate(b, segsize*sizeof(float));
    }
    vboEdges.release();
}


void gl::makeCube(Vector3d const &pt, double dx, double dy, double dz,
                QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges)
{
    // 12 triangles
    // 3 vertices/triangle
    // (3 position + 3 normal) components/vertex
    int buffersize = 12 *3 * 6;
    QVector<GLfloat> CubeVertexArray(buffersize, 0);

    // 8 vertices
    Vector3d T000 = {pt.x-dx/2, pt.y-dy/2, pt.z-dz/2};
    Vector3d T001 = {pt.x-dx/2, pt.y-dy/2, pt.z+dz/2};
    Vector3d T010 = {pt.x-dx/2, pt.y+dy/2, pt.z-dz/2};
    Vector3d T011 = {pt.x-dx/2, pt.y+dy/2, pt.z+dz/2};
    Vector3d T100 = {pt.x+dx/2, pt.y-dy/2, pt.z-dz/2};
    Vector3d T101 = {pt.x+dx/2, pt.y-dy/2, pt.z+dz/2};
    Vector3d T110 = {pt.x+dx/2, pt.y+dy/2, pt.z-dz/2};
    Vector3d T111 = {pt.x+dx/2, pt.y+dy/2, pt.z+dz/2};

    Vector3d N;

    int iv = 0;
    // X- face
    N.set(-1,0,0);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    //   second triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();


    // X+ face
    N.set(1,0,0);
    //   first triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    //   second triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Y- face
    N.set(0,-1,0);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Y+ face
    N.set(0,1,0);
    //   first triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();


    // Z- face
    N.set(0,0,-1);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Z+ face
    N.set(0,0,1);
    //   first triangle
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    Q_ASSERT(iv==buffersize);

    vboFaces.destroy();
    vboFaces.create();
    vboFaces.bind();
    vboFaces.allocate(CubeVertexArray.data(), buffersize* int(sizeof(GLfloat)));
    vboFaces.release();

    buffersize = 12 * 2 *3; //12 edges x2 vertices x3 components
    QVector<float> EdgeVertexArray(buffersize);
    iv=0;

    //bottom face
    {
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();

        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();
        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();

        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();
        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();

        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
    }

    //top face
    {
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();
        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();

        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();
        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();

        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();
        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();

        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();
    }

    //lateral edges
    {
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();

        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();
        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();

        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();
        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();

        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();
        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();
    }

    Q_ASSERT(iv==buffersize);

    vboEdges.destroy();
    vboEdges.create();
    vboEdges.bind();
    vboEdges.allocate(EdgeVertexArray.data(), buffersize* int(sizeof(GLfloat)));
    vboEdges.release();
}


void gl::makeTriangle(Triangle3d const &t3d, QOpenGLBuffer &vbo)
{
    gl::makeTriangle(t3d.vertexAt(0), t3d.vertexAt(1), t3d.vertexAt(2), vbo);
}


void gl::makeTriangle(Vector3d *V, QOpenGLBuffer &vbo)
{
    gl::makeTriangle(V[0], V[1], V[2], vbo);
}


void gl::makeTriangle(Vector2d const &V0, Vector2d const &V1, Vector2d const &V2, QOpenGLBuffer &vbo)
{
    QVector<GLfloat> TriangleVertexArray(12);

    int iv = 0;

    TriangleVertexArray[iv++] = V0.xf();
    TriangleVertexArray[iv++] = V0.yf();
    TriangleVertexArray[iv++] = 0.0f;

    TriangleVertexArray[iv++] = V1.xf();
    TriangleVertexArray[iv++] = V1.yf();
    TriangleVertexArray[iv++] = 0.0f;

    TriangleVertexArray[iv++] = V2.xf();
    TriangleVertexArray[iv++] = V2.yf();
    TriangleVertexArray[iv++] = 0.0f;

    //close the triangle
    TriangleVertexArray[iv++] = V0.xf();
    TriangleVertexArray[iv++] = V0.yf();
    TriangleVertexArray[iv++] = 0.0f;

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(TriangleVertexArray.data(), 12 * sizeof(GLfloat));
    vbo.release();
}


void gl::makeTriangle(Vector3d const &V0, Vector3d const &V1, Vector3d const &V2, QOpenGLBuffer &vbo)
{
    QVector<GLfloat> TriangleVertexArray(12);

    int iv = 0;

    TriangleVertexArray[iv++] = V0.xf();
    TriangleVertexArray[iv++] = V0.yf();
    TriangleVertexArray[iv++] = V0.zf();

    TriangleVertexArray[iv++] = V1.xf();
    TriangleVertexArray[iv++] = V1.yf();
    TriangleVertexArray[iv++] = V1.zf();

    TriangleVertexArray[iv++] = V2.xf();
    TriangleVertexArray[iv++] = V2.yf();
    TriangleVertexArray[iv++] = V2.zf();

    //close the triangle
    TriangleVertexArray[iv++] = V0.xf();
    TriangleVertexArray[iv++] = V0.yf();
    TriangleVertexArray[iv++] = V0.zf();

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(TriangleVertexArray.data(), 12 * sizeof(GLfloat));
    vbo.release();
}


void gl::makeQuadColorMap(QOpenGLBuffer &vbo, int nrows, int ncols,
                        QVector<Vector3d> const&nodes, QVector<double> const &values,
                        float lmin, float lmax, bool bAuto, bool bMultiThreaded)
{
    if(nodes.count()<4)            {vbo.destroy();  return;}
    if(nodes.size()!=nrows*ncols)  {vbo.destroy();  return;}
    if(values.size()!=nrows*ncols) {vbo.destroy();  return;}


    // find min and max Cp for scale set

    if(bAuto)
    {
        lmin =  1000000.0f;
        lmax = -1000000.0f;


        for(int p=0; p<values.count(); p++)
        {
            lmin = std::min(lmin, float(values.at(p)));
            lmax = std::max(lmax, float(values.at(p)));
        }
    }

    t_range = lmax - lmin;
    t_lmin  = lmin;

    // vertices array size:
    // (nrows-1)*(ncols-1) quads
    //      x2 triangles per quad
    //      x3 nodes per triangle
    //        x6 = 3 vertex components + 3 color components

    int nodeVertexSize = (nrows-1)*(ncols-1) * 2 * 3 * 6;
    QVector<float> nodeVertexArray(nodeVertexSize);

    if(bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for(int r=0; r<nrows-1; r++)
        {
            futureSync.addFuture(QtConcurrent::run(&gl::makeQuadColorMapRow,
                                                   r, ncols, nodes, values, nodeVertexArray.data()));
        }

        futureSync.waitForFinished();
    }
    else
    {
        for(int r=0; r<nrows-1; r++)
        {
            gl::makeQuadColorMapRow(r, ncols, nodes, values, nodeVertexArray.data());
        }
    }

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeQuadColorMapRow(int r, int ncols, QVector<Vector3d> const&nodes, QVector<double> const &values, float *nodeVertexArray)
{
    double tau=0;
    QColor clr;
    int idx[] = {0,0,0};
    int iv = 36*r*(ncols-1);

    for(int c=0; c<ncols-1; c++)
    {
        // each quad is two triangles
        //first triangle
        idx[0] =  r   *ncols+c;
        idx[1] =  r   *ncols+c+1;
        idx[2] = (r+1)*ncols+c;

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = nodes.at(idx[i]).xf();
            nodeVertexArray[iv++] = nodes.at(idx[i]).yf();
            nodeVertexArray[iv++] = nodes.at(idx[i]).zf();
            tau = (float(values.at(idx[i]))-t_lmin)/t_range;
            clr = ColourLegend::colour(tau);
            nodeVertexArray[iv++] = clr.redF();
            nodeVertexArray[iv++] = clr.greenF();
            nodeVertexArray[iv++] = clr.blueF();
        }

        //second triangle
        idx[0] =  r   *ncols+c+1;
        idx[1] = (r+1)*ncols+c+1;
        idx[2] = (r+1)*ncols+c;

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = nodes.at(idx[i]).xf();
            nodeVertexArray[iv++] = nodes.at(idx[i]).yf();
            nodeVertexArray[iv++] = nodes.at(idx[i]).zf();
            tau = (float(values.at(idx[i]))-t_lmin)/t_range;
            clr = ColourLegend::colour(tau);
            nodeVertexArray[iv++] = clr.redF();
            nodeVertexArray[iv++] = clr.greenF();
            nodeVertexArray[iv++] = clr.blueF();
        }
    }
}


/** Implementation of the marching square algorithm.
 *  Use instead the triangle method if possible to increase the accuracy*/
void gl::makeQuadContoursOnGrid(QOpenGLBuffer &vbo, int nrows, int ncols,
                                           QVector<Vector3d> const&node, QVector<double> const &value,
                                           bool bMultithreaded)
{
    if(node.count()<4)            {vbo.destroy();  return;}
    if(node.size()!=nrows*ncols)  {vbo.destroy();  return;}
    if(value.size()!=nrows*ncols) {vbo.destroy();  return;}

    // find min and max Cp for scale set
    float lmin =  1000000.0f;
    float lmax = -1000000.0f;

    float coef = 1.0f;

    for(int p=0; p<value.count(); p++)
    {
        lmin = std::min(lmin, float(value.at(p))*coef);
        lmax = std::max(lmax, float(value.at(p))*coef);
    }

    float range = lmax - lmin;

    //define the threshold values for the contours
    int nContours = W3dPrefs::s_NContourLines;
    QVector<float> contour(nContours);
    for(int ic=0; ic<nContours; ic++) contour[ic] = lmin + float(ic)/float(nContours-1)*range;

    QVector<Segment3d> segs;

    t_futuresegs.resize(nContours);

    if(bMultithreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for(int ic=0; ic<nContours; ic++)
        {
            t_futuresegs[ic].clear();
            t_futuresegs[ic].reserve((nrows-1)*(ncols-1)*4);

            futureSync.addFuture(QtConcurrent::run(&gl::makeQuadContour,
                                                   contour.at(ic), nrows, node, value,
                                                   ic));
        }
        futureSync.waitForFinished();
        for(int iseg=0; iseg<t_futuresegs.size(); iseg++)
            segs.append(t_futuresegs.at(iseg));
    }
    else
    {
        for(int ic=0; ic<nContours; ic++)
        {
            t_futuresegs[ic].clear();
            t_futuresegs[ic].reserve((nrows-1)*(ncols-1)*4);
            gl::makeQuadContour(contour.at(ic), nrows, node, value, ic);
            segs.append(t_futuresegs[ic]);
        }
    }

    // vertex array size
    // nsegs
    // x 2 vertices
    // x 3 components
    int nodeVertexSize = segs.size() * 2 * 3;
    QVector<float> nodeVertexArray(nodeVertexSize);

    int iv=0;
    for(int is=0; is<segs.size(); is++)
    {
        Node const & n0 = segs.at(is).vertexAt(0);
        Node const & n1 = segs.at(is).vertexAt(1);
        nodeVertexArray[iv++] = n0.xf();
        nodeVertexArray[iv++] = n0.yf();
        nodeVertexArray[iv++] = n0.zf();
        nodeVertexArray[iv++] = n1.xf();
        nodeVertexArray[iv++] = n1.yf();
        nodeVertexArray[iv++] = n1.zf();
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeQuadContour(double threshold, int nrows,
                                        QVector<Vector3d> const&node, QVector<double> const &value,
                                        int ic)
{
    QVector<Segment3d> &contoursegs = t_futuresegs[ic];

    int idx[] = {0,0,0,0};
    int ncols = node.size()/nrows;
    double tau = 0.0;
    int ik[] = {-1, -1, -1, -1, -1, -1, -1, -1};
    Vector3d I[2]; // crossover points on edge

    int i0=0, i1=0, i2=0, i3=0;
    int key=0, k=1;

    for(int r=0; r<nrows-1; r++)
    {
        for(int c=0; c<ncols-1; c++)
        {
            // for each cell
            idx[0] =  r   *ncols+c;
            idx[1] =  r   *ncols+c+1;
            idx[2] = (r+1)*ncols+c+1;
            idx[3] = (r+1)*ncols+c;

            // check for crossover of contour value
            // use base 2 key as table index
            key = 0;
            k=1;
            for(int i=0; i<4; i++)
            {
                if(value[idx[i%4]]-threshold<0) // true if there is a crossover
                {
                    key += k;
                }
                k *= 2;
            }

            gl::lookUpQuadKey(key, ik);

            for(int jk=0; jk<2; jk++)
            {
                i0 = ik[4*jk+0];
                i1 = ik[4*jk+1];
                i2 = ik[4*jk+2];
                i3 = ik[4*jk+3];
                if(i0>=0 && i1>=0 && i2>=0 && i3>=0)
                {
                    tau = (threshold - value[idx[i0]]) /(value[idx[i1]]-value[idx[i0]]);
                    I[0].x = node[idx[i0]].x*(1-tau) + node[idx[i1]].x*tau;
                    I[0].y = node[idx[i0]].y*(1-tau) + node[idx[i1]].y*tau;
                    I[0].z = node[idx[i0]].z*(1-tau) + node[idx[i1]].z*tau;
                    tau = (threshold - value[idx[i2]]) /(value[idx[i3]]-value[idx[i2]]);
                    I[1].x = node[idx[i2]].x*(1-tau) + node[idx[i3]].x*tau;
                    I[1].y = node[idx[i2]].y*(1-tau) + node[idx[i3]].y*tau;
                    I[1].z = node[idx[i2]].z*(1-tau) + node[idx[i3]].z*tau;
                    contoursegs.push_back({I[0], I[1]});
                }
            }
        }
    }
}


void gl::makeQuadTex(double xside, double yside, QOpenGLBuffer &vbo)
{
    Node vtx[4];

    vtx[0].set(-xside,-yside, 0);  vtx[0].setNormal(0,0,1);
    vtx[1].set( xside,-yside, 0);  vtx[2].setNormal(0,0,1);
    vtx[2].set( xside, yside, 0);  vtx[1].setNormal(0,0,1);
    vtx[3].set(-xside, yside, 0);  vtx[3].setNormal(0,0,1);
    QVector<Triangle3d> triangles(2);
    triangles.front().setTriangle(vtx[0], vtx[1], vtx[2]);
    triangles.back().setTriangle(vtx[0], vtx[2], vtx[3]);

    //Make surface triangulation
    int bufferSize = triangles.count();
    bufferSize *= 3;    // 4 vertices for each triangle
    bufferSize *= 8;    // (3 coords+3 normal components+2UVcomponents) for each node

    QVector<float> meshvertexarray(bufferSize);

    Vector3d N;
    bool bFlatNormals = true;
    int iv = 0;
    for(int it=0; it<triangles.size(); it++)
    {
        Triangle3d const &t3d = triangles.at(it);
        N.set(t3d.normal());

        for(int ivtx=0; ivtx<3; ivtx++)
        {
            Node const &vertex = t3d.vertexAt(ivtx);

            meshvertexarray[iv++] = vertex.xf();
            meshvertexarray[iv++] = vertex.yf();
            meshvertexarray[iv++] = vertex.zf();
            if(bFlatNormals)
            {
                meshvertexarray[iv++] = N.xf();
                meshvertexarray[iv++] = N.yf();
                meshvertexarray[iv++] = N.zf();
            }
            else
            {
                meshvertexarray[iv++] = vertex.normal().xf();
                meshvertexarray[iv++] = vertex.normal().yf();
                meshvertexarray[iv++] = vertex.normal().zf();
            }
            meshvertexarray[iv++] = (xside-vertex.xf())/xside/2.0; // U component, equal to x
            meshvertexarray[iv++] = (vertex.yf()-yside)/yside/2.0; // V component, equal to y
        }
    }

    Q_ASSERT(iv==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeSegments2d(QVector<Segment2d> const &segments, Vector2d const &pos, QOpenGLBuffer &vbo)
{
    //Make surface triangulation
    int bufferSize = segments.size();
    bufferSize *=2;    // 2 vertices for each segment
    bufferSize *=3;    // (3 coords) for each node

    QVector<float> meshvertexarray(bufferSize);

    int iv = 0;
    for(int it=0; it<segments.size(); it++)
    {
        Segment2d const &seg = segments.at(it);
        meshvertexarray[iv++] = seg.vertexAt(0).xf() + pos.xf();
        meshvertexarray[iv++] = seg.vertexAt(0).yf() + pos.yf();
        meshvertexarray[iv++] = 0.0f;
        meshvertexarray[iv++] = seg.vertexAt(1).xf() + pos.xf();
        meshvertexarray[iv++] = seg.vertexAt(1).yf() + pos.yf();
        meshvertexarray[iv++] = 0.0f;
    }

    Q_ASSERT(meshvertexarray.size()==bufferSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshvertexarray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeColourMap2d(QVector<Triangle2d> const &panel3, float height,
                       QVector<double> const &values,
                       double lmin, double lmax,
                       QOpenGLBuffer &vbo)
{
    float tau=0;

    if(!panel3.size() || !values.size()) return;

    int nPanel3 = panel3.size();
    int nNodes = values.size();

    double range = lmax - lmin;

    // vertices array size:
    //        n triangular Panels
    //      x3 nodes per triangle
    //        x6 = 3 vertex components + 3 color components

    int nodeVertexSize = nPanel3* 3 * 6;
    QVector<float> nodeVertexArray(nodeVertexSize, 0.0f);
    QColor clr;
    int iv=0;
    for (int p=0; p<nPanel3; p++)
    {
        Triangle2d const &p3 = panel3.at(p);

        for(int i=0; i<3; i++)
        {
            nodeVertexArray[iv++] = p3.vertexAt(i).xf();
            nodeVertexArray[iv++] = p3.vertexAt(i).yf();
            nodeVertexArray[iv++] = height;

            int idx= p3.vertexIndex(i);
            if(fabs(range)<PRECISION)
                tau = -1.0; // will map to black
            else
            {
                if(0<=idx && idx<nNodes)
                {
                    if(fabs(values.at(idx))>=LARGEVALUE)
                        tau = -1.0; // will map to black
                    else
                        tau = float((values[idx]-lmin)/range);
                }
                else
                {
                    tau = -1.0; // will map to black
                }
            }
            clr = ColourLegend::colour(tau);
            nodeVertexArray[iv++] = clr.redF();
            nodeVertexArray[iv++] = clr.greenF();
            nodeVertexArray[iv++] = clr.blueF();
        }
    }

    Q_ASSERT(iv==nodeVertexSize);

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(nodeVertexArray.data(), nodeVertexSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl::makeQuad2d(QRectF const &rect, QOpenGLBuffer &vbo)
{
    QVector<GLfloat> QuadVertexArray(12, 0);

    int iv = 0;
    QuadVertexArray[iv++] = 0.0f;
    QuadVertexArray[iv++] = 0.0f;

    QuadVertexArray[iv++] = rect.left();
    QuadVertexArray[iv++] = rect.top();

    QuadVertexArray[iv++] = rect.right();
    QuadVertexArray[iv++] = rect.top();

    QuadVertexArray[iv++] = rect.right();
    QuadVertexArray[iv++] = rect.bottom();

    QuadVertexArray[iv++] = rect.left();
    QuadVertexArray[iv++] = rect.bottom();

    QuadVertexArray[iv++] = rect.left();
    QuadVertexArray[iv++] = rect.top();

    Q_ASSERT(iv==QuadVertexArray.size());

    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(QuadVertexArray.data(), int(QuadVertexArray.size()) * sizeof(GLfloat));
    vbo.release();
}
