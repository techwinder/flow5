/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QSlider>
#include <QFrame>


class gl3dView;
class Fine3dControls;

class gl3dControls : public QFrame
{
    Q_OBJECT

    public:
        gl3dControls(gl3dView *pgl3dview, QWidget *pParent=nullptr);
        virtual ~gl3dControls();
        void makeCommonWts();
        void resetClipPlane();

    public:
        void connectBaseSignals();

        virtual void setControls();


    protected:
        void showEvent(QShowEvent *pEvent) override;

    protected slots:
        void onFineCtrls(bool bChecked);

    protected:
        gl3dView *m_pgl3dView;
        QToolButton *m_ptbX, *m_ptbY, *m_ptbZ;
        QToolButton *m_ptbIso, *m_ptbHFlip, *m_ptbVFlip, *m_ptbReset;
        QToolButton *m_ptbFineCtrls;
        Fine3dControls *m_pFineCtrls;

    public:
        QToolButton *m_ptbDistance;
};


