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

#include <QCheckBox>


#include <interfaces/widgets/customdlg/xfldialog.h>

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




