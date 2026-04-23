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
#include <QLabel>



class PopUp : public QWidget
{
    Q_OBJECT

    public:
        PopUp(QWidget *pParent=nullptr);
        PopUp(QString const &message, QWidget *pParent);
        void appendTextMessage(QString const &text);
        void setTextMessage(QString const &text);
        void setDuration(int time_s) {m_Duration=time_s*1000;}
        void setRed();
        void setGreen();
        void setFont(QFont const &fnt);

    protected:
        void showEvent(QShowEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *event) override;

    private:
        void setupLayout();


    private:
        QLabel *m_plabMessage;
        int m_Duration; //ms
};

