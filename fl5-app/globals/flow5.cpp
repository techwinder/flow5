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


#include <iostream>

#include <qglobal.h>

#include <QOperatingSystemVersion>

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
#include <QLibraryInfo>
#include <QTranslator>

#include "flow5.h"



#include <api/fl5core.h>

#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <api/trace.h>
#include <core/xflcore.h>
#include <core/xflcore.h>
#include <globals/mainframe.h>
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/script/xflscriptexec.h>
#include <interfaces/widgets/customdlg/objectpropsdlg.h>
#include <interfaces/widgets/customwts/popup.h>
#include <options/prefsdlg.h>



Flow5App::Flow5App(int &argc, char** argv) : QApplication(argc, argv)
{
    setApplicationName("flow5");   // for qsettings
    setOrganizationName("Vic-Aero");
    setOrganizationDomain("vic-aero.tech");
    setDesktopFileName("flow5");

    //    QString Version;
    //    Version = QString::asprintf("v%d.%02d", MAJOR_VERSION, MINOR_VERSION);
    Flow5App::setApplicationVersion(QString::fromStdString(fl5::versionName(true)));
    setWindowIcon(QIcon(":/icons/f5.png"));

    loadTranslations();
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


    if(xfl::g_bTrace)
    {
        startTrace(tracefilename);
        xfl::trace("Initializing trace file in "+tracefilename+"\n\n");
    }

    if(!bScript && !xfl::g_bTrace)
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

    if(xfl::g_bTrace)
    {
        QString strange = "Trace enabled in "+ tracefilename +"\n\n";
        m_pMainFrame->displayMessage(strange, false);
    }

    gl3dView::setDebugContext(xfl::g_bTrace);

    m_pMainFrame->testOpenGL();

    if(bScript)
    {
        m_pMainFrame->executeScript(scriptPathName, bShowProgress);
        m_bDone = true;
        return;
    }

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
            int iApp = m_pMainFrame->loadProjectFile(fi.canonicalFilePath());
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


void Flow5App::loadTranslations()
{
#if defined Q_OS_MAC
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5", "flow5");
#elif defined Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"flow5", "flow5");
#else
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"flow5");
#endif

    PrefsDlg::loadSettings(settings);

    QString lang = PrefsDlg::language();
    QLocale locale = QLocale::system();

    if (lang == "English") 
    {
        return; 
    }
    else if (lang == "Chinese") 
    {
        locale = QLocale(QLocale::Chinese, QLocale::China);
    }
    else if (lang == "French")
    {
        locale = QLocale(QLocale::French, QLocale::France);
    }
    else if (lang == "German")
    {
        locale = QLocale(QLocale::German, QLocale::Germany);
    }

    const QString qtTrDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if(m_qtTranslator.load(locale, QStringLiteral("qt"), QStringLiteral("_"), qtTrDir))
    {
        installTranslator(&m_qtTranslator);
    }

    const QString appTrDir = QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("translations");
    if(m_appTranslator.load(locale, QStringLiteral("flow5"), QStringLiteral("_"), appTrDir))
    {
        installTranslator(&m_appTranslator);
    }
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
                xfl::trace("Processing FileOpenEvent for " +pFOEvent->file() + "\n");

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

            iApp = m_pMainFrame->loadProjectFile(pFOEvent->file());
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
        xfl::g_bTrace=true;
        xfl::trace("Processing option -t", true);
        xfl::trace("Trace logged in file: "+tracefilename);
    }

    bScript = parser.isSet(ScriptOption);
    if(bScript)
    {
        xfl::trace("Processing option -s", true);
    }
    scriptfilename = parser.value("s");


    if(parser.isSet(OGLOption))
    {
        bool bOK=false;
        int version = parser.value(OGLOption).toInt(&bOK);
        if(bOK) OGLVersion = version; else OGLVersion = -1;
        xfl::trace("Processing option -o", OGLVersion);
    }
    else
    {
        OGLVersion = -1;
    }
}


void Flow5App::startTrace(QString const &filename)
{
    if(!xfl::g_bTrace) return;

    xfl::g_pTraceFile = new QFile(filename);

    if (!xfl::g_pTraceFile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) xfl::g_bTrace = false;
    xfl::g_pTraceFile->reset();
    std::cout<< "Trace file enabled: " << filename.toStdString().c_str() << std::endl;
    QString strange;
    QOperatingSystemVersion const &sys = QOperatingSystemVersion::current();
    strange = sys.name();
    xfl::trace(strange+"\n");

    QSysInfo sysInfo;

    strange  = "SysInfo:\n";
    strange += "   build ABI:       "  + sysInfo.buildAbi() +"\n";
    strange += "   build CPU:       "  + sysInfo.buildCpuArchitecture() +"\n";
    strange += "   current CPU:     "  + sysInfo.currentCpuArchitecture() +"\n";
    strange += "   kernel type:     "  + sysInfo.kernelType() +"\n";
    strange += "   kernel version:  "  + sysInfo.kernelVersion() +"\n";
    strange += "   product name:    "  + sysInfo.prettyProductName() +"\n";
    strange += "   product type:    "  + sysInfo.productType() +"\n";
    strange += "   product version: "  + sysInfo.productVersion() +"\n\n";
    xfl::trace(strange);

    const char *qt_version = qVersion();
    strange = QString::asprintf("Qt version: %s\n\n", qt_version);
    xfl::trace(strange);

    strange = QString::asprintf("Ideal thread count: %d\n\n", QThread::idealThreadCount());
    xfl::trace(strange);

    strange = "OpenGL support:\n";
    strange += QString::asprintf("    Desktop OpenGL: %d\n", qApp->testAttribute(Qt::AA_UseDesktopOpenGL));
    strange += QString::asprintf("    OpenGL ES       %d\n", qApp->testAttribute(Qt::AA_UseOpenGLES));
    strange += QString::asprintf("    Software OpenGL %d\n", qApp->testAttribute(Qt::AA_UseSoftwareOpenGL));
    xfl::trace(strange + "\n");


    QString language = QLocale::languageToString(QLocale::system().language());
    strange  = "System default regional settings:\n";
    strange += "   language:          "+language+"\n";
    strange += "   decimal separator: "+QString(QLocale::system().decimalPoint()) + "\n";
    strange += "   group separator:   "+QString(QLocale::system().groupSeparator()) + "\n";
    xfl::trace(strange + "\n");
//qDebug("%s",strange.toStdString().c_str());
}


