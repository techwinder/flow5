/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <QCheckBox>


#include <xflwidgets/customdlg/xfldialog.h>

class FloatEdit;
class DFoil;
class MainFrame;

class LECircleDlg : public XflDialog
{
    Q_OBJECT

    public:
        LECircleDlg(MainFrame *pParent);

        void setupLayout();
        void initDialog();

        double const &LERadius() const {return m_Radius;}
        void setLERadius(double r) {m_Radius=r;}

        void showRadius(bool bShow) {m_bShowRadius = bShow;}
        bool bShowRadius()const {return m_bShowRadius;}

    private slots:
        void accept() override;

    private:
        QCheckBox *m_pchShow;
        FloatEdit *m_pdeRadius;

        double m_Radius;
        bool m_bShowRadius;
        MainFrame* s_pMainFrame;
};




