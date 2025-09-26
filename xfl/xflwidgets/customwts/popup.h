/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

