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

#pragma once



#include <QPainter>
#include <QFile>
#include <QThread>
#include <QList>
#include <QStandardItem>
#include <QTextStream>

#include <xflcore/linestyle.h>


#define EKEY '#'

#define MAJOR_VERSION    7
#define MINOR_VERSION    53

#define PICHAR         QString(QChar(0x03C0))
#define ALPHACHAR      QString(QChar(0x03B1))
#define BETACHAR       QString(QChar(0x03B2))
#define GAMMACHAR      QString(QChar(0x03B3))
#define DELTACHAR      QString(QChar(0x03B4))
#define DELTACAPCHAR   QString(QChar(0x0394)) // Capital
#define ZETACHAR       QString(QChar(0x03B6))
#define LAMBDACHAR     QString(QChar(0x03BB))
#define NUCHAR         QString(QChar(0x03BD))
#define PHICHAR        QString(QChar(0x03C6))
#define RHOCHAR        QString(QChar(0x03C1))
#define SIGMACHAR      QString(QChar(0x03C3))
#define THETACHAR      QString(QChar(0x03B8))
#define XICHAR         QString(QChar(0x03BE))
#define TAUCHAR        QString(QChar(0x03C4))
#define DEGCHAR        QString(QChar(0x00B0))
#define INFCHAR        QString(QChar(0x221e))
#define TIMESCHAR      QString(QChar(0x00d7))
#define EOLCHAR        QString("\n")

namespace xfl
{
    extern bool g_bLocalize;

    extern bool g_bMultiThread;
    extern int g_MaxThreadCount;
    extern QThread::Priority g_ThreadPriority;

    extern int g_SymbolSize;
    extern int g_DarkFactor;

    extern bool g_bDontUseNativeDlg;
    extern bool g_bConfirmDiscard;


    extern QColor Orchid;
    extern QColor BlueViolet;
    extern QColor SteelBlue;
    extern QColor CornFlowerBlue;
    extern QColor PhugoidGreen;
    extern QColor Bisque;
    extern QColor FireBrick;
    extern QColor LightCoral;
    extern QColor GreenYellow;
    extern QColor Magenta;
    extern QColor IndianRed;
    extern QColor Turquoise;


    QString versionName(bool bFull);

    void loadCoreSettings(QSettings &settings);
    void saveCoreSettings(QSettings &settings);

    int readValues(const QString &theline, double &x, double &y, double &z);
    void readFloat(QDataStream &inStream, float &f);
    void writeFloat(QDataStream &outStream, float f);
    bool readAVLString(QTextStream &in, int &Line, QString &strong);
    QColor readQColor(QDataStream &ar);

    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPoint const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPointF const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, double x, double y);
    void drawLabel(QPainter &painter, int xu, int yu, double value, Qt::Alignment align);



    inline Qt::PenStyle getStyle(Line::enumLineStipple s)
    {
         switch(s)
         {
             default:
             case Line::SOLID:      return Qt::SolidLine;
             case Line::DASH:       return Qt::DashLine;
             case Line::DOT:        return Qt::DotLine;
             case Line::DASHDOT:    return Qt::DashDotLine;
             case Line::DASHDOTDOT: return Qt::DashDotDotLine;
             case Line::NOLINE:     return Qt::NoPen;
         }
    }


    void expFormat(double &f, int &exp);
    void ReynoldsFormat(QString &str, double f);

    QStringList findFiles(const QString &startDir, const QStringList &filters, bool bRecursive);
    bool findFile(QString const &filename, QString const &startDir, const QStringList &filters, bool bRecursive, QString &filePathName);

    void writeCString(QDataStream &ar, QString const &strong);
    void readCString(QDataStream &ar, QString &strong);

    void readString(QDataStream &ar, QString &strong);
    void writeString(QDataStream &ar, QString const &strong);

    bool stringToBool(QString const &str);
    QString boolToString(bool b);

    QColor randomColor(bool bLightColor=true);
    inline QString colorNameARGB(QColor const &colour) {return QString::asprintf("rgba(%d,%d,%3d,%g)", colour.red(), colour.green(), colour.blue(), colour.alphaF());}


    // dummy function arguments used to create a QSettings xml file for the license
    bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map); // dummy argument function
    bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map); // dummy argument function

    QColor getColor(int r, int g, int b, int a=255);


    QList<QStandardItem *> prepareRow(const QString &first, const QString &second=QString(), const QString &third=QString(),  const QString &fourth=QString());
    QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
    QList<QStandardItem *> prepareIntRow(const QString &first, const QString &second, const int &third);
    QList<QStandardItem *> prepareDoubleRow(const QString &first, const QString &second, const double &third,  const QString &fourth);
    QList<QStandardItem *> prepareDoubleRow(const QString &second, const double &value1, const double &value2, const QString &fourth);

    inline bool bConfirmDiscard() {return g_bConfirmDiscard;}
    inline void setConfirmDiscard(bool bConfirm) {g_bConfirmDiscard=bConfirm;}

    inline bool dontUseNativeMacDlg() {return g_bDontUseNativeDlg;}
    inline void setDontUseNativeColorDlg(bool bDont) {g_bDontUseNativeDlg=bDont;}

    inline void setSymbolSize(int s) {g_SymbolSize=s;}
    inline int symbolSize() {return g_SymbolSize;}

    inline void setDarkFactor(int d) {g_DarkFactor=d;}
    inline int darkFactor() {return g_DarkFactor;}

    inline void setLocalized(bool bLocal) {g_bLocalize=bLocal;}
    inline bool isLocalized() {return g_bLocalize;}

    inline bool isMultiThreaded() {return g_bMultiThread;}
    inline void setMultiThreaded(bool bMultiThread)  {g_bMultiThread=bMultiThread;}

    inline void setThreadPriority(QThread::Priority priority) {g_ThreadPriority = priority;}
    inline QThread::Priority threadPriority() {return g_ThreadPriority;}

    inline int maxThreadCount() {return g_MaxThreadCount;}
    inline void setMaxThreadCount(int nMaxThreads) {g_MaxThreadCount=nMaxThreads;}


    void readColor(QDataStream &ar, int &r, int &g, int &b);
    void writeColor(QDataStream &ar, int r, int g, int b);

    void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
    void writeColor(QDataStream &ar, int r, int g, int b, int a);

    QColor colour(QVector<QColor> const &clrs, float tau);

    float getRed(float tau);
    float getGreen(float tau);
    float getBlue(float tau);

    void listSysInfo(QString &info);
}
