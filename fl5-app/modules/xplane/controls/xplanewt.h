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

#include <QWidget>
#include <QSettings>
#include <QSplitter>

class XPlane;
class gl3dXPlaneView;
class gl3dGeomControls;
class POpp3dCtrls;

class XPlaneWt : public QWidget
{
    public:
        XPlaneWt(XPlane *pXPlane, gl3dXPlaneView *pgl3dXPlaneView);

        void setControls();
        void updateView();
        void updateObjectData();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        gl3dXPlaneView *gl3dFloatView() {return m_pgl3dXPlaneFloatView;}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:
        gl3dXPlaneView *m_pgl3dXPlaneFloatView;              /**< a pointer to the floating widget for 3d rendering */
        POpp3dCtrls *m_pPOpp3dControls;

        QSplitter *m_pHSplitter;

    private:
        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;
        XPlane *m_pXPlane;
};

