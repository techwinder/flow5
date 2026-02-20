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

#include <QApplication>
#include <QTranslator>


class MainFrame;
class Flow5App : public QApplication
{
    public:
        Flow5App(int&, char**);
        bool done() const {return m_bDone;}

    protected:
        bool event(QEvent *pEvent) override;

    private:
        void parseCmdLine(Flow5App &app, QString &scriptFileName, QString &tracefilename, bool &bScript, bool &bShowProgress, int &OGLVersion) const;


        void startTrace(const QString &filename);

        void loadTranslations();

    private:
        MainFrame *m_pMainFrame;
        bool m_bDone;

        QTranslator m_qtTranslator;
        QTranslator m_appTranslator;
};





