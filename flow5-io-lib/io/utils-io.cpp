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

#include <QDir>

#include <utils-io.h>


/** from Qt examples WordCount
 * startDir = QDir::home().absolutePath()
 * filters = QStringList() << "*.cpp" << "*.h" ;
*/
QStringList io::findFiles(const QString &startDir, QStringList const &filters, bool bRecursive)
{
    QStringList names;
    QDir dir(startDir);

    for (QString const &file : dir.entryList(filters, QDir::Files))
    {
        names += startDir + '/' + file;
    }

    if(bRecursive)
    {
        for(QString const& subdir : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            names += findFiles(startDir + '/' + subdir, filters, bRecursive);
        }
    }

    return names;
}


bool io::findFile(QString const &filename, QString const &startDir, QStringList const &filters, bool bRecursive, QString &filePathName)
{
    QDir dir(startDir);

    for(QString const &file : dir.entryList(filters, QDir::Files))
    {
        if(file.compare(filename, Qt::CaseInsensitive)==0)
        {
            filePathName = startDir + '/' + file;
            return true;
        }
    }

    if(bRecursive)
    {
        for(QString const &subdir : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            if(findFile(filename, startDir + '/' + subdir, filters, bRecursive, filePathName))
                return true;
        }
    }

    return false;
}


/**
* Extracts three double values from a QString, and returns the number of extracted values.
*/
int io::readValues(QString const &theline, double &x, double &y, double &z)
{
    int res=0;

    QString line, str;
    bool bOK=false;

    line = theline.simplified();
    int pos = line.indexOf(" ");

    if(pos>0)
    {
        str = line.left(pos);
        line = line.right(line.length()-pos);
    }
    else
    {
        str = line;
        line = "";
    }
    x = str.toDouble(&bOK);
    if(bOK) res++;
    else
    {
        y=z=0.0;
        return res;
    }

    line = line.trimmed();
    pos = line.indexOf(" ");
    if(pos>0)
    {
        str = line.left(pos);
        line = line.right(line.length()-pos);
    }
    else
    {
        str = line;
        line = "";
    }
    y = str.toDouble(&bOK);
    if(bOK) res++;
    else
    {
        z=0.0;
        return res;
    }

    line = line.trimmed();
    if(!line.isEmpty())
    {
        z = line.toDouble(&bOK);
        if(bOK) res++;
    }
    else z=0.0;

    return res;
}



/**
 * Reads one line from an AVL-format text file
 */
bool io::readAVLString(QTextStream &in, int &Line, QString &strong)
{
    bool isCommentLine = true;
    int pos=0;
    if(in.atEnd()) return false;

    while(isCommentLine && !in.atEnd())
    {
        isCommentLine = false;

        strong = in.readLine();

        strong = strong.trimmed();
        pos = strong.indexOf("#",0);
        if(pos>=0) strong = strong.left(pos);
        pos = strong.indexOf("!",0);
        if(pos>=0) strong = strong.left(pos);

        if(strong.isEmpty()) isCommentLine = true;

        Line++;
    }

    return true;
}


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


