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

#include <QRadioButton>


#include <interfaces/widgets/customdlg/xfldialog.h>
#include <api/vector3d.h>

class FloatEdit;

class RotateDlg : public XflDialog
{
    public:
        RotateDlg(QWidget *pParent);

        int axis() const;
        double angle() const;

    private:
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:

        QRadioButton *m_prbX, *m_prbY, *m_prbZ;
        FloatEdit *m_pfeAngle;

        static QByteArray s_WindowGeometry;
        static double s_Angle;
        static int s_iAxis;
};


