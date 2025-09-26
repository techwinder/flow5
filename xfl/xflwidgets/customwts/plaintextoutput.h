/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QSettings>
#include <QApplication>
#include <QPlainTextEdit>


class PlainTextOutput : public QPlainTextEdit
{
    Q_OBJECT
    public:
        PlainTextOutput(QWidget *pWidget=nullptr);
        void setCharDimensions(int nHChar, int nVChar);

        void updateColors(bool bOpaque);


    public slots:
        void onAppendThisPlainText(QString const &sometext);

    protected:
        QSize sizeHint() const override;
        void showEvent(QShowEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    private:
        int m_nHChar;
        int m_nVChar;

};

