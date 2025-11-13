/****************************************************************************

    flow5
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

#include <QDebug>
#include <QApplication>
#include <QThread>
#include <QFile>
#include <QOperatingSystemVersion>
#include <QTextStream>
#include <QSurfaceFormat>
#include <QDir>

#include <xflcore/trace.h>

bool g_bTrace = false;
QFile *g_pTraceFile = nullptr;

/**
* Outputs in a debug file the current time and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log
* Used for debugging.
*@param n the integer to output
*/
void trace(int n)
{
    if(!g_bTrace) return;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << "Int value=" << n << "\n";
    }
}


void trace(const QString &msg, bool b)
{
    if(!g_bTrace) return;
    QString str;
    if(b) str += msg + "= true";
    else  str += msg + "= false";

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << str <<"\n";
    }
}


/**
* Outputs in a debug file the current time and a string message passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log.
* Used for debugging.
*@param msg the message to output
*/
void trace(const QString &msg)
{
#ifdef QT_DEBUG
//    qDebug()<<msg;
#endif
    if(!g_bTrace) return;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts<<msg;
        ts.flush();
    }
}


/**
* Outputs in a debug file the current time, a string message and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log.
* Used for debugging.
*@param msg the message to output
*@param n the integer to output
*/
void trace(QString const &msg, int n)
{
    if(!g_bTrace) return;

    QString strong;
    strong = QString::asprintf("  %d\n",n);
    strong = msg + strong;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << strong;
    }
}


/**
* Outputs in a debug file the current time, a string message and the value of the floating number passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log.
* Used for debugging.
*@param msg the message to output
*@param f the float number to output
*/
void trace(const QString &msg, double f)
{
    if(!g_bTrace) return;

    QString strong;
    strong = QString::asprintf("  %g\n",f);
    strong = msg + strong;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << strong;
    }
}


void trace(const QString &msg, QString txt)
{
    if(!g_bTrace) return;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << msg + " " + txt;
    }
}


void startTrace(QString const &filename)
{
    if(!g_bTrace) return;

    g_pTraceFile = new QFile(filename);

    if (!g_pTraceFile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) g_bTrace = false;
    g_pTraceFile->reset();
    std::cout<< "Trace file enabled: " << filename.toStdString().c_str() << std::endl;
    QString strange;
    QOperatingSystemVersion const &sys = QOperatingSystemVersion::current();
    strange = sys.name();
    trace(strange+"\n");

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
    trace(strange);

    const char *qt_version = qVersion();
    strange = QString::asprintf("Qt version: %s\n\n", qt_version);
    trace(strange);

    strange = QString::asprintf("Ideal thread count: %d\n\n", QThread::idealThreadCount());
    trace(strange);

    strange = "OpenGL support:\n";
    strange += QString::asprintf("    Desktop OpenGL: %d\n", qApp->testAttribute(Qt::AA_UseDesktopOpenGL));
    strange += QString::asprintf("    OpenGL ES       %d\n", qApp->testAttribute(Qt::AA_UseOpenGLES));
    strange += QString::asprintf("    Software OpenGL %d\n", qApp->testAttribute(Qt::AA_UseSoftwareOpenGL));
    trace(strange + "\n");


    QString language = QLocale::languageToString(QLocale::system().language());
    strange  = "System default regional settings:\n";
    strange += "   language:          "+language+"\n";
    strange += "   decimal separator: "+QString(QLocale::system().decimalPoint()) + "\n";
    strange += "   group separator:   "+QString(QLocale::system().groupSeparator()) + "\n";
    trace(strange + "\n");
//qDebug("%s",strange.toStdString().c_str());
}


