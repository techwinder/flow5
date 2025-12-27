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


#include <QtCore>
#include <QApplication>
#include <QSurfaceFormat>

#ifdef Q_OS_WIN
#include <windows.h>
#endif


//#include <sys/resource.h>
#include "flow5.h"
#include <interfaces/opengl/views/gl3dview.h>

void customLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    (void)context;

    QByteArray localMsg = msg.toLocal8Bit();
//    const char *file = context.file ? context.file : "";
//    const char *function = context.function ? context.function : "";
    switch (type)
    {
        case QtDebugMsg:
//            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "%s\n", localMsg.constData());
            break;
        case QtInfoMsg:
//            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Info: %s\n", localMsg.constData());
            break;
        case QtWarningMsg:
//            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Warning: %s\n", localMsg.constData());
            break;
        case QtCriticalMsg:
//            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Critical: %s\n", localMsg.constData());
            break;
        case QtFatalMsg:
//            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            fprintf(stderr, "Fatal: %s\n", localMsg.constData());
            break;
    }
}


/** OpenGL Default format must be set prior to app construction if Qt::AA_ShareOpenGLContexts is set */
void setOGLDefaultFormat(int version)
{
#if defined Q_OS_MAC
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5", "flow5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"flow5", "flow5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5");
#endif

    // Load preferred OpenGL version
    // and set the default format before any 3d view is created
    int OGLMajor = 4;
    int OGLMinor = 6;
    if(QFile(settings.fileName()).exists())
    {
        gl3dView::loadSettings(settings);
        OGLMajor = gl3dView::oglMajor();
        OGLMinor = gl3dView::oglMinor();
    }

    if(version>0)
    {
        OGLMajor = int(version/10);
        OGLMinor = version - 10*OGLMajor;
        qDebug()<<"setting context"<<OGLMajor<<OGLMinor;
    }

    // choose between the version passed as option if valid and the saved setting

    if(OGLMajor<=2 || (OGLMajor==3 && OGLMinor<3))
    {
        // Systems (may? commonly?) respond with the latest 4.x context,
        // so force deprecated functions and compatibility profile
        // Will also force v120 style shaders in GL initialization
        gl3dView::setProfile(QSurfaceFormat::NoProfile);
        gl3dView::setDeprecatedFuncs(true);
    }
    else
    {
        gl3dView::setProfile(QSurfaceFormat::CoreProfile);
        gl3dView::setDeprecatedFuncs(false);
    }

    gl3dView::setRenderableType(QSurfaceFormat::OpenGL);
    gl3dView::setOGLVersion(OGLMajor, OGLMinor);

    if(gl3dView::defaultXflSurfaceFormat().samples()<0) gl3dView::setDefaultSamples(4);

    QSurfaceFormat::setDefaultFormat(gl3dView::defaultXflSurfaceFormat()); // for all QOpenGLWidgets
//    qDebug()<<gl3dView::defaultXflSurfaceFormat();
}


/**
 * The app's point of entry !
 */
int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
/*    struct rlimit limit ;
    getrlimit (RLIMIT_STACK, &limit); // The maximum size of the process stack, in bytes.
    qDebug("\nStack Limit = %.2fMb soft limit and %ld bytes hard limit\n", (float)limit.rlim_cur/1024./1024., limit.rlim_max);*/
#endif

    //QLoggingCategory::setFilterRules("qt.*.debug=false");
    //QLoggingCategory::setFilterRules(QStringLiteral("flow5.debug = true"));
    //QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    qInstallMessageHandler(&customLogHandler);


#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    //    qDebug()<<QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif


    /*To set up sharing between QOpenGLWidget instances belonging to different windows,
     * set the Qt::AA_ShareOpenGLContexts application attribute before instantiating QApplication.
     * This will trigger sharing between all QOpenGLWidget instances without any further steps.*/
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    int version = -1;
    for(int i=0; i<argc; i++)
    {
        QString strange = argv[i];
        if(strange.compare("-o", Qt::CaseSensitive)==0 && i<argc-1)
        {
            version = QString(argv[i+1]).toInt();
            qDebug()<<"OGL version" << version << "requested";
        }
    }
    setOGLDefaultFormat(version);

    Flow5App app(argc, argv);
    Flow5App::setApplicationDisplayName("flow5");


    if(app.done()) return 0;
    else           return app.exec();
}






