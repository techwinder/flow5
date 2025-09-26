/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>



class PlainTextOutput;


class LogMessageDlg : public QWidget
{
    Q_OBJECT

    public:
        LogMessageDlg(QWidget *pParent=nullptr);

        void initDialog(QString const &title, QString const &props);

        void setOutputFont(QFont const &fnt);

        void setOnTop();

        QSize sizeHint() const override {return QSize(750,550);}

    signals:
        void closeLog();

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;

    public slots:
        void onAppendMessage(QString const &msg);
        void onClose();
        void clearText();
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();

    private:
        PlainTextOutput *m_ppto;
        QPushButton *m_ppbClearButton;
        QDialogButtonBox *m_pButtonBox;
};
