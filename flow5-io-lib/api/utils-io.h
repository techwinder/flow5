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

//#include <QTextStream>
//#include <QStringList>
#include <QString>

class QTextStream;
class QDataStream;


#include <flow5-io-lib_global.h>
#include <fl5color.h>
#include <units.h>


namespace io
{
    FL5IOLIB_EXPORT QStringList findFiles(const QString &startDir, QStringList const &filters, bool bRecursive);
    FL5IOLIB_EXPORT bool findFile(QString const &filename, QString const &startDir, QStringList const &filters, bool bRecursive, QString &filePathName);

    FL5IOLIB_EXPORT int readValues(QString const &theline, double &x, double &y, double &z);
    FL5IOLIB_EXPORT bool readAVLString(QTextStream &in, int &Line, QString &strong);
}


// Private methods, not intended to be exposed in the API
namespace io
{
    fl5Color readQColor(QDataStream &ar);
    void readString(QDataStream &ar, std::string &strong);
    void readFloat(QDataStream &inStream, float &f);
    void writeFloat(QDataStream &outStream, float f);

    void writeString(QDataStream &ar, QString const &strong);
    void writeString(QDataStream &ar, std::string const &strong);

    void readColor(QDataStream &ar, int &r, int &g, int &b);
    void writeColor(QDataStream &ar, int r, int g, int b);

    void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
    void writeColor(QDataStream &ar, int r, int g, int b, int a);
}
