/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QString>
#include <QVector>


namespace xfl
{
    enum enumObjectiveType {MINIMIZE, EQUALIZE, MAXIMIZE}; // defines whether the objective is to minimize, maximize or make equal
}



struct OptObjective
{
        OptObjective() : m_Index{-1}, m_bActive{true}, m_Target{0.0}, m_MaxError{0.0}, m_Type{xfl::EQUALIZE}
    {}

    OptObjective(QString const &name, int index, bool bActive, double target, double maxerror, xfl::enumObjectiveType type) :
        m_Name{name}, m_Index{index}, m_bActive{bActive}, m_Target{target}, m_MaxError{maxerror}, m_Type{type}
    {}

    QString m_Name;
    int m_Index{-1};         /**< the objective's index in the array of possible objectives */
    bool m_bActive{true};    /**< true if this objective is active in the current optimization task */
    double m_Target{0.0};    /**< this objective's target value */
    double m_MaxError{0.0};  /**< this objective's maximum error */
    xfl::enumObjectiveType m_Type = xfl::EQUALIZE;
};


struct OptVariable
{
    OptVariable() : m_Min{0.0}, m_Max{0.0}
    {}

    OptVariable(QString const &name, double valmin, double valmax) : m_Name{name}, m_Min{valmin}, m_Max{valmax}
    {}

    OptVariable(QString const &name, double val) : m_Name{name}, m_Min{val}, m_Max{val}
    {}

    QString m_Name;
    double m_Min{0.0};
    double m_Max{0.0};
};


struct OptCp
{
    OptCp() : m_iMin{-1}, m_iMax{-1}
    {}

    OptCp(int iMin, int iMax, QVector<double> Cp) : m_iMin{iMin}, m_iMax{iMax}, m_Cp{Cp}
    {}

    bool isActive() const {return m_iMin>=0 && m_iMax>=0;}

    int m_iMin{-1};           /**< the index on the curve of the spline's first control point */
    int m_iMax{-1};           /**< the index on the curve of the spline's last control point */
    QVector<double>m_Cp;  /**< the array of objective Cp values from node m_iMin to node m_iMax */
};

