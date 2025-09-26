/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSettings>

#include <xfl3d/controls/gl3dcontrols.h>

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

