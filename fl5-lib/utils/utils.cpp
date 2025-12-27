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

#include <fstream>
#include <iostream>
#include <iomanip>
#include <QString>
#include <time.h>

#include <QString>

#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
#endif


#include <utils.h>



fl5Color xfl::Orchid         = fl5Color(218,112,214);
fl5Color xfl::BlueViolet     = fl5Color(138,43,226);
fl5Color xfl::SteelBlue      = fl5Color(70,130,180);
fl5Color xfl::CornFlowerBlue = fl5Color(100,149,237);
fl5Color xfl::PhugoidGreen   = fl5Color(45,82,39);
fl5Color xfl::Bisque         = fl5Color(255,228,196);
fl5Color xfl::FireBrick      = fl5Color(178,34,34);
fl5Color xfl::LightCoral     = fl5Color(240,128,128);
fl5Color xfl::GreenYellow    = fl5Color(173,255,47);
fl5Color xfl::Magenta        = fl5Color(255,0,255);
fl5Color xfl::IndianRed      = fl5Color(205,92,92);
fl5Color xfl::Turquoise      = fl5Color(64,224,208);


fl5Color xfl::readQColor(QDataStream &ar)
{
    uchar byte=0;

    ar>>byte; // a format identifyer?

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
void xfl::readColor(QDataStream &ar, int &r, int &g, int &b)
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
void xfl::writeColor(QDataStream &ar, int r, int g, int b)
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
void xfl::readColor(QDataStream &ar, int &r, int &g, int &b, int &a)
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
void xfl::writeColor(QDataStream &ar, int r, int g, int b, int a)
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


void xfl::readString(QDataStream &ar, std::string &strong)
{
    QString str;
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

    strong = str.toStdString();
}


void xfl::writeString(QDataStream &ar, QString const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}


void xfl::writeString(QDataStream &ar, std::string const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text= textline.data();;

    ar << qi;
    ar.writeRawData(text, qi);
}


/**
* Extracts nfloat values from a std::string, and returns the number of extracted values.
* @param nValues is the size of the val array
*/
int xfl::readValues(std::string const &theline, float val[], int nValues)
{
//    std::string line = theline;
//    trim(line);

    std::istringstream buffer(theline);
    std::vector<std::string> split;

    std::copy(std::istream_iterator<std::string>(buffer),
              std::istream_iterator<std::string>(),
              std::back_inserter(split));

    int nread = 0;


    std::string::size_type sz(0);


    try
    {
        for(uint is=0; is<split.size() && nread<nValues; is++)
        {
            val[nread++] = std::stof(split.at(is), &sz);
        }
    }
    catch (const std::invalid_argument& ia)
    {
//          std::cerr << "Invalid argument: " << ia.what() << '\n';
    }
    catch (const std::out_of_range& oor)
    {
//        std::cerr << "Out of Range error: " << oor.what() << '\n';
    }
    catch(...)
    {
//        std::cerr << "Unknown error reading floats"<< '\n';
    }

    return nread;
}


void xfl::readFloat(QDataStream &inStream, float &f)
{
    char buffer[4];
    inStream.readRawData(buffer, 4);
    memcpy(&f, buffer, sizeof(float));
}


void xfl::writeFloat(QDataStream &outStream, float f)
{
    char buffer[4];
    memcpy(buffer, &f, sizeof(float));
    outStream.writeRawData(buffer, 4);
}


float xfl::randomfloat(float fmax)
{
    float f = float(std::rand()) / float(RAND_MAX);
    return f * fmax;
}


int xfl::randomInt(int range)
{
    range++;
    return std::rand() % range;
}

/*
fl5Color xfl::randomObjectColor(bool )
{
    int R = randomInt(255);
    int G = randomInt(255);
    int B = randomInt(255);

    assert(R<256);
    assert(G<256);
    assert(B<256);

    return fl5Color(R,G,B);
}*/


float xfl::getRed(float tau)
{
    if     (tau>5.0f/6.0f) return 1.0f;
    else if(tau>4.0f/6.0f) return (6.0f*(tau-4.0f/6.0f));
    else if(tau>2.0f/6.0f) return 0.0f;
    else if(tau>1.0f/6.0f) return 1.0f - (6.0f*(tau-1.0f/6.0f));
    else                   return 1.0f;
}


float xfl::getGreen(float tau)
{
    if      (tau<2.0f/6.0f) return 0.0f;
    else if (tau<3.0f/6.0f) return 6.0f*(tau-2.0f/6.0f);
    else if (tau<5.0f/6.0f) return 1.0f;
    else if (tau<6.0f/6.0f) return 1.0f - (6.0f*(tau-5.0f/6.0f));
    else                    return 0.0f;
}


float xfl::getBlue(float tau)
{
    if      (tau<0.0f)      return 0.0f;
    else if (tau<1.0f/6.0f) return 6.0f * tau;
    else if (tau<3.0f/6.0f) return 1.0f;
    else if (tau<4.0f/6.0f) return 1.0f - (6.0f*(tau-3.0f/6.0f));
    else                    return 0.0f;
}


bool xfl::stringToFile(std::string const &string, std::string const &path)
{
    std::ofstream outstream;
    try
    {
        outstream.open(path);
        outstream << string;
        outstream.close();
    }
    catch (const std::ofstream::failure &)
    {
      return false;
    }
    return true;
}


bool xfl::stringFromFile(std::string &string, std::string const &path)
{
    std::ifstream instream;

    try {
      instream.open(path);
      std::stringstream buffer(string);
      buffer << instream.rdbuf();
//      string = buffer.str();
    }
    catch (const std::ifstream::failure &)
    {
      return false;
    }
    return true;
}


bool xfl::stringToBool(const QString &str)
{
    return str.trimmed().compare("true", Qt::CaseInsensitive)==0 ? true : false;
}


QString xfl::boolToString(bool b)
{
    return b ? "true" : "false";
}

std::string xfl::MklVersion()
{
    QString strange;

#ifdef INTEL_MKL
    MKLVersion Version;

    mkl_get_version(&Version);

    strange += QString::asprintf("<p><b>Version: </b>%d.%d.%d<br>", Version.MajorVersion, Version.MinorVersion, Version.UpdateVersion);
    strange += QString::asprintf("<b>Processor optimization: </b> %s", Version.Processor) + "</p>";
#endif
    return strange.toStdString();
}


