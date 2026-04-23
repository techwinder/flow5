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

#include <QCheckBox>

class CrossCheckBox : public QWidget
{
    Q_OBJECT

    public:
        CrossCheckBox(QWidget *pParent = nullptr);
        void setWidthHint(int w) {m_WidthHint = w;}
        void setCheckState(Qt::CheckState state) {m_State=state;}
        Qt::CheckState checkState() const {return m_State;}

    signals:
        void clicked(bool);

    private:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
        void paintEvent(QPaintEvent *) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;



    private:
        Qt::CheckState m_State;
        int m_WidthHint;
};

