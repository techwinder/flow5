/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QFrame>
#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QSlider>


class gl3dView;
class FloatEdit;

class Fine3dControls : public QFrame
{
    Q_OBJECT

    friend class XPlane;

    public:
        Fine3dControls(gl3dView *pgl3dview);

    public:
        void connectSignals();
        void setControls();
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;

    public slots:
        void onResetCtrls();

    private slots:
        void onXView(int);
        void onYView(int);
        void onZView(int);
        void onZAnimAngle();

    private:
        gl3dView *m_pgl3dView;

        int m_XLast, m_YLast, m_ZLast;
        int m_XSet, m_YSet, m_ZSet;

    public:
        QSlider *m_pslClipPlanePos;

    private:
        QSlider *m_pslXView, *m_pslYView, *m_pslZView;
        QCheckBox *m_pchZAnimate;
        FloatEdit *m_pdeZAnimAngle;
};

