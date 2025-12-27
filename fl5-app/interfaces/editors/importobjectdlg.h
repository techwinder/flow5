/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QListWidget>
#include <interfaces/widgets/customdlg/xfldialog.h>

class ImportObjectDlg : public XflDialog
{
    Q_OBJECT

    public:
        ImportObjectDlg(QWidget *pParent, bool bWings);

        QString planeName()  const;
        QString objectName() const;

        QSize sizeHint() const override {return QSize(1000,1000);}

    private slots:
        void onPlaneChanged();

    private:
        void setupLayout(bool bWings);
        void fillObjects();

    private:
        QListWidget *m_plvPlanes;
        QListWidget *m_plvObjects;

        bool m_bWings;
};
