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

#pragma once

#include <QTextStream>
#include <QStringList>
#include <QString>

#include <flow5-io-lib_global.h>
#include <units.h>

namespace io
{
    FL5IOLIB_EXPORT QStringList findFiles(const QString &startDir, QStringList const &filters, bool bRecursive);
    FL5IOLIB_EXPORT bool findFile(QString const &filename, QString const &startDir, QStringList const &filters, bool bRecursive, QString &filePathName);

    FL5IOLIB_EXPORT int readValues(QString const &theline, double &x, double &y, double &z);
    FL5IOLIB_EXPORT bool readAVLString(QTextStream &in, int &Line, QString &strong);

}

namespace Units
{
    // convenience conversion to QString
    inline QString lengthUnitQLabel(int idx=-1)    {return QString::fromStdString(Units::lengthUnitLabel(idx));}
    inline QString speedUnitQLabel(int idx=-1)     {return QString::fromStdString(Units::speedUnitLabel(idx));}
    inline QString massUnitQLabel(int idx=-1)      {return QString::fromStdString(Units::massUnitLabel(idx));}
    inline QString areaUnitQLabel(int idx=-1)      {return QString::fromStdString(Units::areaUnitLabel(idx));}
    inline QString forceUnitQLabel(int idx=-1)     {return QString::fromStdString(Units::forceUnitLabel(idx));}
    inline QString momentUnitQLabel(int idx=-1)    {return QString::fromStdString(Units::momentUnitLabel(idx));}
    inline QString pressureUnitQLabel(int idx=-1)  {return QString::fromStdString(Units::pressureUnitLabel(idx));}
    inline QString inertiaUnitQLabel(int idx=-1)   {return QString::fromStdString(Units::inertiaUnitLabel(idx));}

    inline QString densityUnitQLabel()    {return QString::fromStdString(Units::densityUnitLabel());}
    inline QString viscosityUnitQLabel()  {return QString::fromStdString(Units::viscosityUnitLabel());}

}



#define PIch         QString(QChar(0x03C0))
#define ALPHAch      QString(QChar(0x03B1))
#define BETAch       QString(QChar(0x03B2))
#define GAMMAch      QString(QChar(0x03B3))
#define DELTAch      QString(QChar(0x03B4))
#define DELTACAPch   QString(QChar(0x0394)) // Capital
#define ZETAch       QString(QChar(0x03B6))
#define LAMBDAch     QString(QChar(0x03BB))
#define NUch         QString(QChar(0x03BD))
#define PHIch        QString(QChar(0x03C6))
#define RHOch        QString(QChar(0x03C1))
#define SIGMAch      QString(QChar(0x03C3))
#define THETAch      QString(QChar(0x03B8))
#define XIch         QString(QChar(0x03BE))
#define TAUch        QString(QChar(0x03C4))
#define DEGch        QString(QChar(0x00B0))
#define INFch        QString(QChar(0x221e))
#define TIMESch      QString(QChar(0x00d7))
#define SQUAREch     QString(QChar(0x00b2))
#define EOLch        QString("\n")

