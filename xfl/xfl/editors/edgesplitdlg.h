/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QComboBox>

#include <xflmath/mathelem.h>
#include <xflwidgets/customdlg/xfldialog.h>

class IntEdit;
class EdgeSplit;

class EdgeSplitDlg : public XflDialog
{
    public:
        EdgeSplitDlg(QWidget *pParent, EdgeSplit &es);
        int nSegs() const;
        xfl::enumDistribution distrib() const;

    private:
        void setupLayout();

    private:
        IntEdit *m_pieNSegs;
        QComboBox *m_pcbDistType;

};


