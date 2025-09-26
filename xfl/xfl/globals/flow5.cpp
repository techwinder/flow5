/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <iostream>

#include <qglobal.h>


#ifdef Q_OS_LINUX
    #  include <unistd.h>
#endif

#ifdef Q_OS_WIN
#  include <windows.h>
//#  define sleep(s) Sleep((s)*1000)
#endif

#ifdef Q_OS_MAC
    #  include <unistd.h>
#endif

#include <QFileOpenEvent>

#include <QMessageBox>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include "flow5.h"
#include <xfl/globals/mainframe.h>
#include <xfl/options/prefsdlg.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflcore/saveoptions.h>

#include <xflcore/trace.h>
#include <xflcore/xflcore.h>
#include <xfl3d/views/gl3dview.h>
#include <xflscript/xflscriptexec.h>
#include <xflwidgets/customwts/popup.h>
#include <xflwidgets/customdlg/objectpropsdlg.h>

Flow5App::Flow5App(int &argc, char** argv) : QApplication(argc, argv)
{
    setApplicationName("flow5");   // for qsettings
    setOrganizationName("Vic-Aero");
    setOrganizationDomain("vic-aero.tech");
    setDesktopFileName("flow5");

    //    QString Version;
    //    Version = QString::asprintf("v%d.%02d", MAJOR_VERSION, MINOR_VERSION);
    Flow5App::setApplicationVersion(xfl::versionName(false));
    setWindowIcon(QIcon(":/icons/f5.png"));
    /** usage
     * flow5 -h (--help)                   : help
     * flow5 -v (--version)                : version
     * flow5 -s (--script)  scriptFileName : run scriptFileName
     * flow5 -s -p (--script)  scriptFileName : run scriptFileName progress/verbose mode
     * flow5 -o (--ogl) version            : run the program with the specified OpenGL version
     * flow5 projectFileName.xfl           : run and open projectFileName.xfl
     * flow5                               : run the program
     */

    m_pMainFrame = nullptr;

    bool bScript=false, bShowProgress=false;
    QString scriptPathName;
    QString tracefilename = QDir::tempPath() + "/flow5_trace.log";
    int OGLversion = -1;
    parseCmdLine(*this, scriptPathName, tracefilename, bScript, bShowProgress, OGLversion);


    if(g_bTrace)
    {
        startTrace(tracefilename);
        trace("Initializing trace file in "+tracefilename+"\n\n");
    }

    if(!bScript && !g_bTrace)
    {
#ifdef Q_OS_WIN
#ifdef QT_NO_DEBUG
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
#endif
//        splash.setWindowFlags(Qt::SplashScreen);
//        splash.show();
    }

    m_bDone = false;

    QString StyleName;
    QString str;

#if defined Q_OS_MAC
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5", "flow5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"flow5", "flow5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5");
#endif

    m_pMainFrame = new MainFrame;

    bool bSheet = false;
    if(QFile(settings.fileName()).exists())
    {
        settings.beginGroup("MainFrame");
        {
            m_pMainFrame->restoreGeometry(settings.value("geometry").toByteArray());
            m_pMainFrame->restoreState(settings.value("windowState").toByteArray());

            str = settings.value("StyleName").toString();
            if(str.length()) StyleName = str;

            bSheet = settings.value("bStyleSheet", false).toBool();
        }
        settings.endGroup();
    }


    if(StyleName.length())    qApp->setStyle(StyleName);
    if(bSheet)
    {
        QFile stylefile;
        QString qssPathName =  qApp->applicationDirPath() + QDir::separator() +"/flow5_dark.css";

        QFileInfo fi(qssPathName);
        if(fi.exists())
        {
            stylefile.setFileName(qssPathName);
        }
        else
        {
            stylefile.setFileName(QStringLiteral(":/qss/flow5_dark.css"));
        }
        if (stylefile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString qsStylesheet = QString::fromLatin1(stylefile.readAll());
            qApp->setStyleSheet(qsStylesheet);
            stylefile.close();
        }
    }

    m_pMainFrame->show();

    if(g_bTrace)
    {
        QString strange = "Trace enabled in "+ tracefilename +"\n\n";
        m_pMainFrame->displayMessage(strange, false);
        std::cout << strange.toStdString().c_str()<<std::endl;
    }

    gl3dView::setDebugContext(g_bTrace);

    m_pMainFrame->testOpenGL();


    QStringList arguments = QApplication::arguments();


#ifndef Q_OS_MAC
    bool bProjectFile = false;

    if(arguments.size()>1)
    {
        QString PathName, Extension;
        PathName=arguments.at(1);
        PathName.replace("'","");
        QFileInfo fi(PathName);
        Extension = fi.suffix();
        if(Extension.compare("fl5", Qt::CaseInsensitive)==0 || Extension.compare("plr",Qt::CaseInsensitive)==0)
        {
            m_pMainFrame->displayMessage("Launching flow5 with project file "+fi.canonicalFilePath()+"\n", false);
            bProjectFile = true;
            int iApp = m_pMainFrame->loadXflFile(fi.canonicalFilePath());
            switch (iApp)
            {
                case xfl::XSAIL:    m_pMainFrame->onXSail();           break;
                case xfl::XPLANE:   m_pMainFrame->onXPlane();          break;
                case xfl::XDIRECT:  m_pMainFrame->onXDirect();         break;
                default:
                    break;
            }
        }
        else {
            m_pMainFrame->displayMessage("Not a flow5 file: extension " +Extension+" is not recognized\n", true);
        }
    }

    if(!bProjectFile)
    {
        m_pMainFrame->displayMessage("No argument received\n\n", false);
        if(SaveOptions::bAutoLoadLast())
        {
            m_pMainFrame->loadRecentProject();
        }
    }
#else
    if(SaveOptions::bAutoLoadLast() && !m_pMainFrame->projectName().length())
    {
        // if nothing has been loaded, load the last project file
        m_pMainFrame->onLoadLastProject();
    }
#endif

    m_pMainFrame->displayMessage("Done app initialization\n\n", false);
//    splash.finish(m_pMainFrame);

/*    if(SaveOptions::autoSave())
    {
        QString strange("The auto-save feature has been permanently disabled to prevent accidental file corruption");
        QMessageBox::warning(m_pMainFrame, "Warning", strange);
        SaveOptions::setAutoSave(false);
    }*/


    m_pMainFrame->setFocus();
}


bool Flow5App::event(QEvent *pEvent)
{
    int iApp = xfl::NOAPP;

    switch (pEvent->type())
    {
        case QEvent::FileOpen:
        {
            QFileOpenEvent *pFOEvent = dynamic_cast<QFileOpenEvent *>(pEvent);
            if(!pFOEvent) break;
            if(pFOEvent)
                trace("Processing FileOpenEvent for " +pFOEvent->file() + "\n");

#ifdef Q_OS_MAC
/*           // load licence settings at this point because macOS calls this event
            // and then launches an instance of flow5...
            QSettings licencesettings(QSettings::NativeFormat, QSettings::UserScope,"flow5", "fl5_licence");
            licencesettings.beginGroup("licence");
            {
                Lich::setEMail(licencesettings.value("email").toString());
                loadCryptedKey(licencesettings);
            }
            licencesettings.endGroup(); */
#endif

            iApp = m_pMainFrame->loadXflFile(pFOEvent->file());
            switch (iApp)
            {
                case xfl::XSAIL:    m_pMainFrame->onXSail();           break;
                case xfl::XPLANE:   m_pMainFrame->onXPlane();          break;
                case xfl::XDIRECT:  m_pMainFrame->onXDirect();         break;
                default:
                    break;
            }
            m_pMainFrame->displayMessage("Done processing FileOpen event\n", false);

            return true;
        }
        default:
            break;
    }
    return QApplication::event(pEvent);
}


void Flow5App::parseCmdLine(Flow5App &fl5app,
                            QString &scriptfilename, QString &tracefilename,
                            bool &bScript, bool &bShowProgress,
                            int &OGLVersion) const
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Analysis tool for planes and sails operating at low Reynolds numbers");
    parser.addHelpOption();
    parser.addVersionOption();

    //    parser.addPositionalArgument("ScriptFileName", QCoreApplication::translate("main", "XML Script file to execute."));

    // An integer option with a single name (-o)
    QCommandLineOption OGLOption(QStringList() << "o" << "opengl");
    OGLOption.setValueName("OpenGL_version");
    OGLOption.setDefaultValue("44");
    OGLOption.setDescription("Launches the application with the specified OpenGL version. "
                             "The default is the format 4.4. Test and set higher "
                             "versions using the in-app OpenGL test window in the Options menu. "
                             "Usage: flow5 -o 41 to request a 4.1 context.");
    parser.addOption(OGLOption);

    QCommandLineOption ShowProgressOption(QStringList() << "p" << "progress");
    ShowProgressOption.setDescription(QCoreApplication::translate("main", "Show progress during script execution."));
    parser.addOption(ShowProgressOption);

    QCommandLineOption ScriptOption(QStringList() << "s" << "script");
    ScriptOption.setValueName("file");
    ScriptOption.setDescription("Runs the script file");
    parser.addOption(ScriptOption);

    QCommandLineOption TraceOption(QStringList() << "t" << "trace");
    TraceOption.setValueName("file");
    TraceOption.setDefaultValue(QDir::tempPath() + "/flow5_trace.log");
    TraceOption.setDescription("Runs the program in trace mode.");
    parser.addOption(TraceOption);

    // Process the actual command line arguments provided by the user
    parser.process(fl5app);

    bShowProgress = parser.isSet(ShowProgressOption);
    bScript = parser.isSet(ScriptOption);
    tracefilename = parser.value(TraceOption);
    scriptfilename = parser.value(ScriptOption);


    if(parser.isSet(TraceOption))
    {
        g_bTrace=true;
        trace("Processing option -t", true);
        trace("Trace logged in file: "+tracefilename);
    }

    bScript = parser.isSet(ScriptOption);
    if(bScript)
    {
        trace("Processing option -s", true);
    }
    scriptfilename = parser.value("s");


    if(parser.isSet(OGLOption))
    {
        bool bOK=false;
        int version = parser.value(OGLOption).toInt(&bOK);
        if(bOK) OGLVersion = version; else OGLVersion = -1;
        trace("Processing option -o", OGLVersion);
    }
    else
    {
        OGLVersion = -1;
    }
}


