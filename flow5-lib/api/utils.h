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


#include <algorithm>
#include <cctype>

#include <sstream>
#include <vector>

#include <QDataStream>


#define ALPHAstr      std::string("\u03B1")
#define BETAstr       std::string("\u03B2")
#define LAMBDAstr     std::string("\u03BB")
#define NUstr         std::string("\u03BD")
#define PHIstr        std::string("\u03C6")
#define RHOstr        std::string("\u03C1")
#define THETAstr      std::string("\u03B8")
#define XIstr         std::string("\u03BE")
#define DEGstr        std::string("\u00B0")
#define INFstr        std::string("\u221e")
#define SQUAREstr     std::string("\u00b2")
#define EOLstr        std::string("\n")

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


#include <fl5lib_global.h>
#include <fl5color.h>



namespace xfl
{
    FL5LIB_EXPORT  extern fl5Color Orchid;
    FL5LIB_EXPORT  extern fl5Color BlueViolet;
    FL5LIB_EXPORT  extern fl5Color SteelBlue;
    FL5LIB_EXPORT  extern fl5Color CornFlowerBlue;
    FL5LIB_EXPORT  extern fl5Color PhugoidGreen;
    FL5LIB_EXPORT  extern fl5Color Bisque;
    FL5LIB_EXPORT  extern fl5Color FireBrick;
    FL5LIB_EXPORT  extern fl5Color LightCoral;
    FL5LIB_EXPORT  extern fl5Color GreenYellow;
    FL5LIB_EXPORT  extern fl5Color Magenta;
    FL5LIB_EXPORT  extern fl5Color IndianRed;
    FL5LIB_EXPORT  extern fl5Color Turquoise;


    FL5LIB_EXPORT fl5Color readQColor(QDataStream &ar);

    FL5LIB_EXPORT  void readColor(QDataStream &ar, int &r, int &g, int &b);
    FL5LIB_EXPORT  void writeColor(QDataStream &ar, int r, int g, int b);

    FL5LIB_EXPORT  void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
    FL5LIB_EXPORT  void writeColor(QDataStream &ar, int r, int g, int b, int a);

    FL5LIB_EXPORT  float getRed(float tau);
    FL5LIB_EXPORT  float getGreen(float tau);
    FL5LIB_EXPORT  float getBlue(float tau);

    FL5LIB_EXPORT  int   randomInt(int range);
    FL5LIB_EXPORT  float randomfloat(float fmax);


    FL5LIB_EXPORT  void readString(QDataStream &ar, std::string &strong);
    FL5LIB_EXPORT  void writeString(QDataStream &ar, QString const &strong);
    FL5LIB_EXPORT  void writeString(QDataStream &ar, std::string const &strong);

    FL5LIB_EXPORT  int readValues(const std::string &theline, float val[], int nValues);
    FL5LIB_EXPORT  void readFloat(QDataStream &inStream, float &f);
    FL5LIB_EXPORT  void writeFloat(QDataStream &outStream, float f);

    FL5LIB_EXPORT  bool stringToFile(std::string const &string, std::string const &path);
    FL5LIB_EXPORT  bool stringFromFile(std::string &string, std::string const &path);


    FL5LIB_EXPORT  bool stringToBool(QString const &str);
    FL5LIB_EXPORT  QString boolToString(bool b);

    FL5LIB_EXPORT std::string MklVersion();

    /** @enum The status of the 3d analysis */
    enum enumAnalysisStatus {PENDING, RUNNING, CANCELLED, FINISHED};

    /** @todo nothing to do here */
    /** @enum The different formats usable to export data to text format files*/
    enum enumTextFileType {TXT, CSV};


    // https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring
    // Trim from the start (in place)
    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // Trim from the end (in place)
    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }
    // Trim from both ends (in place)
    inline void trim(std::string &s) {
        rtrim(s);
        ltrim(s);
    }


    // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    inline std::vector<std::string> split (const std::string &s, char delim)
    {
        std::vector<std::string> result;
        std::stringstream ss (s);
        std::string item;

        while (getline (ss, item, delim)) {
            result.push_back (item);
        }

        return result;
    }
}
