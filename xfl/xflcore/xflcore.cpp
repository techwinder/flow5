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

#include "xflcore.h"

#include <QDir>
#include <QRandomGenerator>
#include <QColor>
#include <QDate>


#include <xflcore/enums_core.h>
#include <xflcore/linestyle.h>
#include <xflcore/displayoptions.h>


QColor xfl::Orchid         = QColor("#DA70D6");
QColor xfl::BlueViolet     = QColor("#8A2BE2");
QColor xfl::SteelBlue      = QColor("#4682B4");
QColor xfl::CornFlowerBlue = QColor("#6495ED");
QColor xfl::PhugoidGreen   = QColor("#2d5227");
QColor xfl::Bisque         = QColor("#FFE4C4");
QColor xfl::FireBrick      = QColor("#B22222");
QColor xfl::LightCoral     = QColor("#F08080");
QColor xfl::GreenYellow    = QColor("#ADFF2F");
QColor xfl::Magenta        = QColor("#FF00FF");
QColor xfl::IndianRed      = QColor("#CD5C5C");
QColor xfl::Turquoise      = QColor("#40E0D0");

int xfl::g_SymbolSize = 3;
int xfl::g_DarkFactor = 100;

bool xfl::g_bMultiThread=true;
int xfl::g_MaxThreadCount = QThread::idealThreadCount();
QThread::Priority xfl::g_ThreadPriority=QThread::NormalPriority;

bool xfl::g_bDontUseNativeDlg(true);
bool xfl::g_bConfirmDiscard(true);

bool xfl::g_bLocalize = false;

QString xfl::versionName(bool bFull)
{
    QString VName;
    VName = QString::asprintf("v%d.%02d", MAJOR_VERSION, MINOR_VERSION);
    if(bFull) VName = "flow5 "+VName;
    return VName;
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, QColor const &linecolor,
                     QPoint const &pt)
{
    xfl::drawSymbol(painter, ptstyle, backcolor, linecolor, double(pt.x()), double(pt.y()));
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, QColor const &linecolor,
                     QPointF const & ptf)
{
    xfl::drawSymbol(painter, ptstyle, backcolor, linecolor, ptf.x(), ptf.y());
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, QColor const &linecolor, double x, double y)
{
    painter.save();
    painter.setBackgroundMode(Qt::TransparentMode);

    QPen Pointer(painter.pen());

    painter.setPen(Pointer); 

    QColor bck(backcolor);
    bck.setAlpha(255);

    switch(ptstyle)
    {
        case Line::NOSYMBOL: break;
        case Line::LITTLECIRCLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize);
            painter.drawEllipse(QPointF(x,y), ptSide*1.2, ptSide*1.2);
            break;
        }
        case Line::LITTLECIRCLE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize);
            painter.drawEllipse(QPointF(x,y), ptSide*1.2, ptSide*1.2);
            break;
        }
        case Line::BIGCIRCLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.75;
            painter.drawEllipse(QPointF(x,y), ptSide, ptSide);
            break;
        }
        case Line::BIGCIRCLE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.75;
            painter.drawEllipse(QPointF(x,y), ptSide, ptSide);
            break;
        }
        case Line::LITTLESQUARE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.1;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::LITTLESQUARE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.1;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::BIGSQUARE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.7;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::BIGSQUARE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.7;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::TRIANGLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,        y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,        y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_INV:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,        y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_INV_F:
        {
            QBrush backBrush(linecolor);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,        y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::LITTLECROSS:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize);
            QPointF p0(x-ptSide, y-ptSide);
            QPointF p1(x+ptSide, y+ptSide);
            QPointF p2(x-ptSide, y+ptSide);
            QPointF p3(x+ptSide, y-ptSide);

            painter.drawLine(p0,p1);
            painter.drawLine(p2,p3);
            break;
        }
        case Line::BIGCROSS:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.85;
            QPointF p0(x-ptSide, y-ptSide);
            QPointF p1(x+ptSide, y+ptSide);
            QPointF p2(x-ptSide, y+ptSide);
            QPointF p3(x+ptSide, y-ptSide);

            painter.drawLine(p0,p1);
            painter.drawLine(p2,p3);
            break;
        }
//        default: break;
    }
    painter.restore();
}


/**
* Returns the index of a Qt-style based on the index of the style in the array
* @param s the index of the style
* @return The index of the Qt-style
*/
/*inline Qt::PenStyle getStyle(Line::enumLineStipple s)
{
     switch(s)
     {
         default:
         case Line::SOLID:      return Qt::SolidLine;
         case Line::DASH:       return Qt::DashLine;
         case Line::DOT:        return Qt::DotLine;
         case Line::DASHDOT:    return Qt::DashDotLine;
         case Line::DASHDOTDOT: return Qt::DashDotDotLine;
     }
}*/


 /**
 * Takes a double number holding the value of a Reynolds number and returns a string.
 *@param str the return string  with the formatted number
 *@param f the Reynolds number to be formatted
 */
 void xfl::ReynoldsFormat(QString &str, double f)
 {
     f = (int(f/1000.0))*1000.0;

     int exp = int(log10(f));
     int r = exp%3;
     int q = (exp-r)/3;

     QString strong;
     strong = QString("%1").arg(f,0,'f',0);

     int l = strong.length();

     for (int i=0; i<q; i++){
         strong.insert(l-3*(i+1)-i," ");
         l++;
     }

     for (int i=strong.length(); i<9; i++){
         strong = " "+strong;
     }

     str = strong;
 }


/**
* Returns a double number as its root and its base 10 exponent
* @param f the double number to reformat; is returned as f = f/pow(10.0,exp);
* @param exp the base 10 exponent of f.
*/
void xfl::expFormat(double &f, int &exp)
{
    if (f==0.0)
    {
        exp = 0;
        f = 0.0;
        return;
    }
    double f1 = fabs(f);
    if(f1<1)
        exp = int(log10(f1)-1);
    else
        exp = int(log10(f1));

    f = f/pow(10.0,exp);

    if(fabs(f-10.0)<0.00001)
    {
        f = +1.0;
        exp++;
    }
    else if(fabs(f+10.0)<0.00001)
    {
        f = -1.0;
        exp++;
    }
}



/** from Qt examples WordCount
 * startDir = QDir::home().absolutePath()
 * filters = QStringList() << "*.cpp" << "*.h" ;
*/
QStringList xfl::findFiles(const QString &startDir, QStringList const &filters, bool bRecursive)
{
    QStringList names;
    QDir dir(startDir);

    foreach (QString file, dir.entryList(filters, QDir::Files))
    {
        names += startDir + '/' + file;
    }

    if(bRecursive)
    {
        foreach (QString subdir, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            names += findFiles(startDir + '/' + subdir, filters, bRecursive);
        }
    }

    return names;
}


bool xfl::findFile(QString const &filename, QString const &startDir, QStringList const &filters, bool bRecursive, QString &filePathName)
{
    QDir dir(startDir);

    foreach (QString file, dir.entryList(filters, QDir::Files))
    {
        if(file.compare(filename, Qt::CaseInsensitive)==0)
        {
            filePathName = startDir + '/' + file;
            return true;
        }
    }

    if(bRecursive)
    {
        foreach (QString subdir, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            if(findFile(filename, startDir + '/' + subdir, filters, bRecursive, filePathName))
                return true;
        }
    }

    return false;
}


void xfl::readString(QDataStream &ar, QString &strong)
{
    qint8 qi(0), ch(0);
    char c(0);

    ar >> qi;
    strong.clear();
    for(int j=0; j<qi;j++)
    {
        strong += " ";
        ar >> ch;
        c = char(ch);
        strong[j] = c;
    }
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


void xfl::readCString(QDataStream &ar, QString &strong)
{
    qint8 qi(0), ch(0);
    char c(0);

    ar >> qi;
    strong.clear();
    for(int j=0; j<qi;j++)
    {
        strong += " ";
        ar >> ch;
        c = char(ch);
        strong[j] = c;
    }
}


void xfl::writeCString(QDataStream &ar, QString const &strong)
{
    qint8 qi = qint8(strong.length());

    QByteArray textline;
    char *text;
    textline = strong.toLatin1();
    text = textline.data();
    ar << qi;
    ar.writeRawData(text, qi);
}


bool xfl::stringToBool(const QString &str)
{
    return str.trimmed().compare("true", Qt::CaseInsensitive)==0 ? true : false;
}


QString xfl::boolToString(bool b)
{
    return b ? "true" : "false";
}


/**
* Extracts three double values from a QString, and returns the number of extracted values.
*/
int xfl::readValues(QString const &theline, double &x, double &y, double &z)
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


QColor xfl::randomColor(bool bLightColor)
{
    QColor clr;

    int h = QRandomGenerator::global()->bounded(360);
    int s = QRandomGenerator::global()->bounded(155)+100;
    int v = QRandomGenerator::global()->bounded(80)+120;
    if(bLightColor) v += 55;

    clr.setHsv(h,s,v,255);

    return clr;
}



/**
 * Reads one line from an AVL-format text file
 */
bool xfl::readAVLString(QTextStream &in, int &Line, QString &strong)
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


QColor xfl::readQColor(QDataStream &ar)
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

    return QColor(r,g,b,a);
}


bool xfl::readXmlFile(QIODevice &, QSettings::SettingsMap &) {return true;}
bool xfl::writeXmlFile(QIODevice &, const QSettings::SettingsMap &) {return true;}


QColor xfl::getColor(int r, int g, int b, int a)
{
    r = std::min(r, 255);
    r = std::max(r, 0);
    g = std::min(g, 255);
    g = std::max(g, 0);
    b = std::min(b, 255);
    b = std::max(b, 0);
    a = std::min(a, 255);
    a = std::max(a, 0);
    return QColor(r,g,b,a);
}


QList<QStandardItem *> xfl::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
    for(int ii=0; ii<rowItems.size(); ii++)
        rowItems.at(ii)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem *> xfl::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::BOOLVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> xfl::prepareIntRow(const QString &object, const QString &field, const int &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::INTEGER, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> xfl::prepareDoubleRow(const QString &object, const QString &field, const double &value, const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> xfl::prepareDoubleRow(const QString &field, const double &value1, const double &value2, const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.append(new QStandardItem);
    rowItems.at(1)->setData(value1, Qt::DisplayRole);
    rowItems.at(2)->setData(value2, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
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


QColor xfl::colour(QVector<QColor> const &clrs, float tau)
{
//    if(tau<=-1.0f) return Qt::black;
    if(tau<=0.0f)
        return clrs.first();

    double df = double(clrs.size()-1);
    for(int i=1; i<clrs.size(); i++)
    {
        double fi  = double(i)/df;
        if(tau<fi)
        {
            double hue0 = qMax(0.0, clrs.at(i-1).hueF()); // hue returns -1 if grey
            double sat0 = clrs.at(i-1).saturationF();
            double val0 = clrs.at(i-1).valueF();
            double hue1 = qMax(0.0, clrs.at(i).hueF());
            double sat1 = clrs.at(i).saturationF();
            double val1 = clrs.at(i).valueF();

            // hue is undefined for pure grey colors, so use the other color's hue
            if(sat0<0.005 && sat1>0.005) hue0 = hue1;
            if(sat0>0.005 && sat1<0.005) hue1 = hue0;

            double t = (fi-tau)/(1/df);

            double hue = t*hue0+(1-t)*hue1;
            double sat = t*sat0+(1-t)*sat1;
            double val = t*val0+(1-t)*val1;
            return QColor::fromHsvF(hue, sat, val); // does not accept negative hue
        }
    }
    return clrs.last();
}


void xfl::loadCoreSettings(QSettings &settings)
{
    settings.beginGroup("xflcore");
    {
        g_SymbolSize        = settings.value("SymbolSize",      g_SymbolSize).toUInt();
        g_DarkFactor        = settings.value("DarkFactor",      g_DarkFactor).toInt();

        g_bLocalize         = settings.value("LocalizeApp",     g_bLocalize).toBool();
        g_bMultiThread      = settings.value("Multithreading",  g_bMultiThread).toBool();
        g_MaxThreadCount    = settings.value("MaxThreadCount", QThread::idealThreadCount()).toInt();
        g_ThreadPriority    = QThread::Priority(settings.value("ThreadAnalysisPriority", g_ThreadPriority).toInt());

        g_bConfirmDiscard   = settings.value("ConfirmDiscard",        g_bConfirmDiscard).toBool();
        g_bDontUseNativeDlg = settings.value("DontUseNativeColorDlg", g_bDontUseNativeDlg).toBool();
    }
    settings.endGroup();
}


void xfl::saveCoreSettings(QSettings &settings)
{
    settings.beginGroup("xflcore");
    {
        settings.setValue("SymbolSize",             g_SymbolSize);
        settings.setValue("DarkFactor",             g_DarkFactor);

        settings.setValue("LocalizeApp",            g_bLocalize);
        settings.setValue("Multithreading",         g_bMultiThread);
        settings.setValue("ThreadAnalysisPriority", g_ThreadPriority);
        settings.setValue("MaxThreadCount",         g_MaxThreadCount);

        settings.setValue("ConfirmDiscard",         g_bConfirmDiscard);
        settings.setValue("DontUseNativeColorDlg",  g_bDontUseNativeDlg);
    }
    settings.endGroup();
}


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



#define MININTERVAL  0.000000001

void xfl::drawLabel(QPainter &painter, int xu, int yu, double value, Qt::Alignment align)
{
    if(fabs(value)<MININTERVAL)
    {
        QString strLabel = "0";
        painter.drawText(xu, yu, strLabel);
        return;
    }

    QString strLabel;

    strLabel = QString(xfl::isLocalized() ? "%L1" : "%1").arg(value);
    if(align & Qt::AlignHCenter)
    {
        int px = DisplayOptions::textFontStruct().width(strLabel);
        painter.drawText(xu-px/2, yu, strLabel);
    }
    else if(align & Qt::AlignLeft)
    {
        painter.drawText(xu, yu, strLabel);
    }
}


void xfl::listSysInfo(QString &info)
{
    info.clear();
    QString prefix = "   ";
    info += "System info:";
    info += prefix + "bootUniqueId:            " + QSysInfo::bootUniqueId();
    info += prefix + "buildAbi:                " + QSysInfo::buildAbi(); // Application Binary Interface
    info += prefix + "buildCpuArchitecture:    " + QSysInfo::buildCpuArchitecture();
    info += prefix + "currentCpuArchitecture:  " + QSysInfo::currentCpuArchitecture();
    info += prefix + "kernelType:              " + QSysInfo::kernelType();
    info += prefix + "kernelVersion:           " + QSysInfo::kernelVersion();
    info += prefix + "machineHostName:         " + QSysInfo::machineHostName();
    info += prefix + "machineUniqueId:         " + QSysInfo::machineUniqueId();
    info += prefix + "prettyProductName:       " + QSysInfo::prettyProductName();
    info += prefix + "productType:             " + QSysInfo::productType();
    info += prefix + "productVersion:          " + QSysInfo::productVersion();
    info += "\n";

}
