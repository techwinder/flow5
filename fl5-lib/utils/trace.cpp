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

#include <QDebug>
#include <QTextStream>


#include <api/trace.h>

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

    std::cout<<"Int value=" << n << "\n";
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

    std::cout<<str.toStdString()<<std::endl;
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

    std::cout<<msg.toStdString();
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

    std::cout<<strong.toStdString();
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

    std::cout<<strong.toStdString();
}


void trace(const QString &msg, QString const &txt)
{
    if(!g_bTrace) return;

    if(g_pTraceFile && g_pTraceFile->isOpen())
    {
        QTextStream ts(g_pTraceFile);
        ts << msg + " " + txt;
    }

    std::cout<<(msg + " " + txt).toStdString();
}

