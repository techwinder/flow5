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

#include "xflfiledialog.h"


QByteArray XflFileDialog::s_Geometry;


XflFileDialog::XflFileDialog()
{
}


void XflFileDialog::loadSettings(QSettings &settings)
{
    settings.beginGroup("XflFileDialog");
    {
        s_Geometry = settings.value("Geometry").toByteArray();
    }
    settings.endGroup();
}


void XflFileDialog::saveSettings(QSettings &settings)
{
    settings.beginGroup("XflFileDialog");
    {
        settings.setValue("Geometry", s_Geometry);
    }
    settings.endGroup();
}


void XflFileDialog::showEvent(QShowEvent*)
{
    restoreGeometry(s_Geometry);
}


void XflFileDialog::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}

