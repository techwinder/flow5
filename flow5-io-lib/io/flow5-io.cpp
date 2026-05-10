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


#include <QFile>
#include <QDataStream>

#include <flow5-io.h>
#include <fileio.h>

fl5Color io::readQColor(QDataStream &ar)
{
    uchar byte=0;

    ar>>byte; // a format identifier?

    ar>>byte>>byte;
    int a = int(byte);
    ar>>byte>>byte;
    int r = int(byte);
    ar>>byte>>byte;
    int g = int(byte);
    ar>>byte>>byte;
    int b = int(byte);

    return fl5Color(r,g,b,a);
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void io::readColor(QDataStream &ar, int &r, int &g, int &b)
{
    qint32 colorref;

    ar >> colorref;
    b = colorref/256/256;
    colorref -= b*256*256;
    g = colorref/256;
    r = colorref - g*256;
}


/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component

*/
void io::writeColor(QDataStream &ar, int r, int g, int b)
{
    qint32 colorref;

    colorref = b*256*256+g*256+r;
    ar << colorref;
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void io::readColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
    uchar byte=0;

    ar>>byte;//probably a format identificator
    ar>>byte>>byte;
    a = int(byte);
    ar>>byte>>byte;
    r = int(byte);
    ar>>byte>>byte;
    g = int(byte);
    ar>>byte>>byte;
    b = int(byte);
    ar>>byte>>byte; //
}

/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void io::writeColor(QDataStream &ar, int r, int g, int b, int a)
{
    uchar byte;

    byte = 1;
    ar<<byte;
    byte = a & 0xFF;
    ar << byte<<byte;
    byte = r & 0xFF;
    ar << byte<<byte;
    byte = g & 0xFF;
    ar << byte<<byte;
    byte = b & 0xFF;
    ar << byte<<byte;
    byte = 0;
    ar << byte<<byte;
}


void io::readString(QDataStream &ar, std::string &strong)
{
    std::string str;
    qint8 qi(0), ch(0);
    char c(0);

    ar >> qi;
    str.clear();
    for(int j=0; j<qi;j++)
    {
        str += " ";
        ar >> ch;
        c = char(ch);
        str[j] = c;
    }

    strong = str;
}



void io::readFloat(QDataStream &inStream, float &f)
{
    char buffer[4];
    inStream.readRawData(buffer, 4);
    memcpy(&f, buffer, sizeof(float));
}


void io::writeFloat(QDataStream &outStream, float f)
{
    char buffer[4];
    memcpy(buffer, &f, sizeof(float));
    outStream.writeRawData(buffer, 4);
}


void io::writeString(QDataStream &ar, QString const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}


void io::writeString(QDataStream &ar, std::string const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text= textline.data();;

    ar << qi;
    ar.writeRawData(text, qi);
}


bool io::saveProject(std::string const& stdPathName, std::string &logmsg)
{
    QString PathName = QString::fromStdString(stdPathName);

    QFile fp(PathName);
    if (!fp.open(QIODevice::WriteOnly))
    {
        logmsg = "Could not open the file: "+stdPathName+" for writing\n\n";
        return false;
    }

    QDataStream ar(&fp);

    FileIO saver;
    if(!saver.serializeProjectFl5(ar, true))
    {
        logmsg = "Unknown error saving the project file " + stdPathName;
        return false;
    }

    fp.close(); // or let the destructor do it

    return true;
}




bool io::loadProject(std::string const& stdPathName, std::string &logmsg)
{
    QString PathName = QString::fromStdString(stdPathName);

    QFile fp(PathName);
    if (!fp.open(QIODevice::ReadOnly))
    {
        logmsg = "Could not open the file: "+stdPathName+" for reading\n\n";
        return false;
    }

    QDataStream ar(&fp);

    FileIO loader;
    if(!loader.serializeProjectFl5(ar, false))
    {
        logmsg = "Unknown error loading the project file " + stdPathName;
        return false;
    }

    fp.close(); // or let the destructor do it

    return true;
}

