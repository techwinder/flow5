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

#pragma once

#include <QString>

#include <flow5-io-lib_global.h>
#include <fl5color.h>

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


namespace io
{
    FL5IOLIB_EXPORT  fl5Color readQColor(QDataStream &ar);
    FL5IOLIB_EXPORT  void readString(QDataStream &ar, std::string &strong);
    FL5IOLIB_EXPORT  void readFloat(QDataStream &inStream, float &f);
    FL5IOLIB_EXPORT  void writeFloat(QDataStream &outStream, float f);

    FL5IOLIB_EXPORT  void writeString(QDataStream &ar, QString const &strong);
    FL5IOLIB_EXPORT  void writeString(QDataStream &ar, std::string const &strong);

    FL5IOLIB_EXPORT  void readColor(QDataStream &ar, int &r, int &g, int &b);
    FL5IOLIB_EXPORT  void writeColor(QDataStream &ar, int r, int g, int b);

    FL5IOLIB_EXPORT  void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
    FL5IOLIB_EXPORT  void writeColor(QDataStream &ar, int r, int g, int b, int a);

    FL5IOLIB_EXPORT  bool saveProject(const std::string &stdPathName, std::string &logmsg);
    FL5IOLIB_EXPORT  bool loadProject(const std::string &stdPathName, std::string &logmsg);

}
