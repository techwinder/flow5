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


#include <QSettings>
#include <QFile>
#include <QThread>
#include <QList>
#include <QStandardItem>
#include <QTextStream>
#include <QString>

#include <api/units.h>

struct fl5Color;
struct LineStyle;


namespace xfl
{
    extern bool g_bLocalize;

    extern int g_MaxThreadCount;
    extern QThread::Priority g_ThreadPriority;

    extern int g_SymbolSize;
    extern int g_DarkFactor;

    extern bool g_bDontUseNativeDlg;
    extern bool g_bConfirmDiscard;

    extern bool g_bMKLDynamic;


    void loadCoreSettings(QSettings &settings);
    void saveCoreSettings(QSettings &settings);

    void loadLineSettings(QSettings &settings, LineStyle &ls, QString const &name);
    void saveLineSettings(QSettings &settings, LineStyle const &ls, QString const &name);


    QColor randomColor(bool bLightColor=true);
    fl5Color randomfl5Color(bool bLightColor=true);

    QColor colour(QVector<QColor> const &clrs, float tau);

    QColor fromfl5Clr(const fl5Color &clr);
    fl5Color tofl5Clr(const QColor &clr);

    inline QString colorNameARGB(QColor const &colour) {return QString::asprintf("rgba(%d,%d,%d,%g)", colour.red(), colour.green(), colour.blue(), colour.alphaF());}

    void expFormat(double &f, int &exp);


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

    inline void setThreadPriority(QThread::Priority priority) {g_ThreadPriority = priority;}
    inline QThread::Priority threadPriority() {return g_ThreadPriority;}

    inline int maxThreadCount() {return g_MaxThreadCount;}
    inline void setMaxThreadCount(int nMaxThreads) {g_MaxThreadCount=nMaxThreads;}


    inline bool isMKLDynamic() {return g_bMKLDynamic;}
    void setMKLDynamic(bool b);

    void listSysInfo(QString &info);
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



