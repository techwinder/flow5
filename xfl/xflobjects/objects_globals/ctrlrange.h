/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QString>

class CtrlRange
{
    public:
        CtrlRange() : m_CtrlMin{0.0}, m_CtrlMax{0.0}
        {
        }

        CtrlRange(QString const &name, double cmin, double cmax) {m_CtrlName=name; m_CtrlMin=cmin; m_CtrlMax=cmax;}

        double ctrlVal(double t) const {return (1.0-t)*m_CtrlMin + t*m_CtrlMax;}
        double ctrlMin() const {return m_CtrlMin;}
        double ctrlMax() const {return m_CtrlMax;}
        double range() const {return m_CtrlMax-m_CtrlMin;}
        QString name() const {return m_CtrlName;}

        void set(QString const & name, double cmin, double cmax) {m_CtrlName=name;m_CtrlMin=cmin; m_CtrlMax=cmax;}
        void setName(QString const &controlname) {m_CtrlName=controlname;}
        void setRange(double cmin, double cmax) {m_CtrlMin=cmin; m_CtrlMax=cmax;}
        void setCtrlMin(double cmin) {m_CtrlMin=cmin;}
        void setCtrlMax(double cmax) {m_CtrlMax=cmax;}

    private:
        double m_CtrlMin;
        double m_CtrlMax;
        QString m_CtrlName; //optional
};

