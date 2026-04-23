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

