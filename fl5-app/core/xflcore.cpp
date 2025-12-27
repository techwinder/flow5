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

#include "xflcore.h"

#include <QDir>
#include <QRandomGenerator>
#include <QColor>
#include <QDate>


#include <core/displayoptions.h>
#include <core/enums_core.h>
#include <api/linestyle.h>
#include <api/utils.h>


int xfl::g_SymbolSize = 3;
int xfl::g_DarkFactor = 100;

bool xfl::g_bMultiThread=true;
int xfl::g_MaxThreadCount = QThread::idealThreadCount();
QThread::Priority xfl::g_ThreadPriority=QThread::NormalPriority;

bool xfl::g_bDontUseNativeDlg(false);
bool xfl::g_bConfirmDiscard(true);

bool xfl::g_bLocalize = false;



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


QColor xfl::colour(QVector<QColor> const &clrs, float tau)
{
//    if(tau<=-1.0f) return Qt::black;
    if(tau<=0.0f)
        return clrs.front();

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
    return clrs.back();
}


fl5Color xfl::randomfl5Color(bool bLightColor)
{
    return tofl5Clr(randomColor(bLightColor));
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


QColor xfl::fromfl5Clr(fl5Color const &clr)
{
    return QColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
}


fl5Color xfl::tofl5Clr(QColor const &clr)
{
    return fl5Color(clr.red(), clr.green(), clr.blue(), clr.alpha());
}

void xfl::loadLineSettings(QSettings &settings, LineStyle &ls, QString const &name)
{
    ls.m_bIsVisible    = settings.value(name+"_visible", true).toBool();
    ls.m_Color.m_Red   = settings.value(name + "_red",   ls.m_Color.m_Red).toInt();
    ls.m_Color.m_Green = settings.value(name + "_green", ls.m_Color.m_Green).toInt();
    ls.m_Color.m_Blue  = settings.value(name + "_blue",  ls.m_Color.m_Blue).toInt();
    ls.m_Color.m_Alpha = settings.value(name + "_alpha", ls.m_Color.m_Alpha).toInt();

    ls.m_Width      = settings.value(name+"_width", 1).toInt();
    ls.m_Tag        = settings.value(name+"_tag", QString()).toString().toStdString();

    if(settings.contains(name+"_line"))
    {
        int istyle = settings.value(name+"_line", 0).toInt();
        switch (istyle)
        {
            default:
            case 0: ls.m_Stipple = Line::SOLID;      break;
            case 1: ls.m_Stipple = Line::DASH;       break;
            case 2: ls.m_Stipple = Line::DOT;        break;
            case 3: ls.m_Stipple = Line::DASHDOT;    break;
            case 4: ls.m_Stipple = Line::DASHDOTDOT; break;
            case 5: ls.m_Stipple = Line::NOLINE;     break;
        }
    }
    if(settings.contains(name+"_pts"))
    {
        int ipts = settings.value(name+"_pts", 0).toInt();
        ls.m_Symbol = LineStyle::convertSymbol(ipts);
    }
}


void xfl::saveLineSettings(QSettings &settings, LineStyle const &ls, QString const &name)
{
    settings.setValue(name +"_visible", ls.m_bIsVisible);
    settings.setValue(name + "_red",    ls.m_Color.m_Red);
    settings.setValue(name + "_green",  ls.m_Color.m_Green);
    settings.setValue(name + "_blue",   ls.m_Color.m_Blue);
    settings.setValue(name + "_alpha",  ls.m_Color.m_Alpha);
    settings.setValue(name +"_width",   ls.m_Width);
    settings.setValue(name +"_tag",     QString::fromStdString(ls.m_Tag));

    switch (ls.m_Stipple)
    {
        case Line::SOLID:      settings.setValue(name+"_line", 0);  break;
        case Line::DASH:       settings.setValue(name+"_line", 1);  break;
        case Line::DOT:        settings.setValue(name+"_line", 2);  break;
        case Line::DASHDOT:    settings.setValue(name+"_line", 3);  break;
        case Line::DASHDOTDOT: settings.setValue(name+"_line", 4);  break;
        case Line::NOLINE:     settings.setValue(name+"_line", 5);  break;
    }

    int ist = LineStyle::convertSymbol(ls.m_Symbol);
    settings.setValue(name+"_pts", ist);

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


/** from Qt examples WordCount
 * startDir = QDir::home().absolutePath()
 * filters = QStringList() << "*.cpp" << "*.h" ;
*/
QStringList xfl::findFiles(const QString &startDir, QStringList const &filters, bool bRecursive)
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


bool xfl::findFile(QString const &filename, QString const &startDir, QStringList const &filters, bool bRecursive, QString &filePathName)
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

