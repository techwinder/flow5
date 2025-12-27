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


