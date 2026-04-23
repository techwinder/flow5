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


#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSettings>

#include <interfaces/opengl/controls/gl3dcontrols.h>

class XSail;
class XSailDisplayCtrls;
class gl3dXSailView;
class gl3dGeomControls;
class Opp3dScalesCtrls;
class StreamLineCtrls;
class FlowCtrls;
class CrossFlowCtrls;

class gl3dXSailCtrls : public QTabWidget
{
    friend class XSail;

    public:
        gl3dXSailCtrls(gl3dXSailView *pgl3dXSailView, QWidget *pParent=nullptr);

        void setControls();
        void initWidget();

        static void setXSail(XSail *pXSail);

    private:
        void setupLayout();

    private:
        XSailDisplayCtrls *m_pXSailDisplayCtrls;
        Opp3dScalesCtrls *m_pOpp3dScalesCtrls;
        StreamLineCtrls *m_pStreamLinesCtrls;
        FlowCtrls *m_pFlowCtrls;
        CrossFlowCtrls *m_pCrossFlowCtrls;

        gl3dXSailView *m_pgl3dXSailView;

        static XSail *s_pXSail;
};

