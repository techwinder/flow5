/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflwidgets/customdlg/xfldialog.h>
class FloatEdit;

class ScaleDlg : public XflDialog
{
    public:
        ScaleDlg(QWidget *pParent);
        double XFactor() const;
        double YFactor() const;
        double ZFactor() const;

    private:
        void setupLayout();

    private:

        FloatEdit *m_pdeXFactor, *m_pdeYFactor, *m_pdeZFactor;
};

