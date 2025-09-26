/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QString>


class Polar;

class PolarNameMaker
{
    friend class PolarAutoNameDlg;

public:
    PolarNameMaker(Polar *pPolar);
    static QString makeName(const Polar *pPolar);

private:
    Polar *m_pPolar;

    static bool s_bType;
    static bool s_bBLMethod;
    static bool s_bReynolds;
    static bool s_bMach;
    static bool s_bNCrit;
    static bool s_bXTrTop;
    static bool s_bXTrBot;
};
