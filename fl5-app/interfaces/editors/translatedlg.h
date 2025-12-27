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

#include <api/vector3d.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class FloatEdit;

class TranslateDlg : public XflDialog
{
    public:
        TranslateDlg(QWidget *pParent=nullptr);

        Vector3d const &translationVector() const {return m_Translation;}

    private:
        void setupLayout();
        void accept() override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:

        Vector3d m_Translation;
        FloatEdit *m_pdeX, *m_pdeY, *m_pdeZ;

        static QByteArray s_WindowGeometry;
};

