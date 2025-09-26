/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QApplication>


class MainFrame;
class Flow5App : public QApplication
{
    public:
        Flow5App(int&, char**);
        bool done() const {return m_bDone;}

    protected:
        bool event(QEvent *pEvent);

    private:
        void parseCmdLine(Flow5App &app, QString &scriptFileName, QString &tracefilename, bool &bScript, bool &bShowProgress, int &OGLVersion) const;

    private:
        MainFrame *m_pMainFrame;
        bool m_bDone;
};





