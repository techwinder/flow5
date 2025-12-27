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

#define _MATH_DEFINES_DEFINED


#include "gl3dxsailctrls.h"
#include <interfaces/controls/poppctrls/opp3dscalesctrls.h>
#include <interfaces/controls/poppctrls/streamlinesctrls.h>
#include <interfaces/controls/poppctrls/flowctrls.h>
#include <interfaces/controls/poppctrls/crossflowctrls.h>
#include <modules/xsail/controls/xsaildisplayctrls.h>


XSail *gl3dXSailCtrls::s_pXSail(nullptr);



gl3dXSailCtrls::gl3dXSailCtrls(gl3dXSailView *pgl3dXSailView, QWidget *pParent) : QTabWidget(pParent)
{
    setMovable(true);

    m_pgl3dXSailView = pgl3dXSailView;

    setupLayout();

    m_pOpp3dScalesCtrls->set3dXSailView(pgl3dXSailView);
    m_pStreamLinesCtrls->set3dXSailView(pgl3dXSailView);
    m_pFlowCtrls->set3dXSailView(pgl3dXSailView);
    m_pCrossFlowCtrls->set3dXSailView(pgl3dXSailView);
}


void gl3dXSailCtrls::setupLayout()
{
    m_pXSailDisplayCtrls = new XSailDisplayCtrls(m_pgl3dXSailView, Qt::Vertical, false);
    m_pOpp3dScalesCtrls  = new Opp3dScalesCtrls;
    m_pStreamLinesCtrls  = new StreamLineCtrls;
    m_pFlowCtrls         = new FlowCtrls;
    m_pCrossFlowCtrls    = new CrossFlowCtrls;

    addTab(m_pXSailDisplayCtrls, "Display");
    addTab(m_pOpp3dScalesCtrls,  "Scales");
    addTab(m_pStreamLinesCtrls,  "Streamlines");
    addTab(m_pFlowCtrls,         "Flow");
    addTab(m_pCrossFlowCtrls,    "Wake");
}


void gl3dXSailCtrls::initWidget()
{
    m_pOpp3dScalesCtrls->initWidget();
    m_pStreamLinesCtrls->initWidget();
    m_pFlowCtrls->initWidget();
    m_pCrossFlowCtrls->initWidget();
}


void gl3dXSailCtrls::setXSail(XSail *pXSail)
{
    s_pXSail = pXSail;
    XSailDisplayCtrls::setXSail(pXSail);
    Opp3dScalesCtrls::setXSail(pXSail);
    FlowCtrls::setXSail(pXSail);
    CrossFlowCtrls::setXSail(pXSail);

}


void gl3dXSailCtrls::setControls()
{
    m_pXSailDisplayCtrls->setControls();
/*    m_pOpp3dScalesCtrls->setControls();
    m_pStreamLinesCtrls->setControls();
    m_pFlowCtrls->setControls();
    m_pCrossFlowCtrls->setControls();*/
}
