/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QAbstractButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QFont>
#include <QColor>



class TextClrBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        TextClrBtn(QWidget *parent = nullptr);

        void setFont(QFont const &font);
        void setTextColor(QColor const & TextColor);
        void setBackgroundColor(QColor const & TextColor);
        QColor textColor() const { return m_TextColor;}

        void setContour(bool bContour) {m_bContour=bContour;}

    signals:
        void clickedTB();

    public:
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

    private:
        QColor m_TextColor;
        QColor m_BackgroundColor;
        QFont m_TextFont;

        bool m_bContour;
};

