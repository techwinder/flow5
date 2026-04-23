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


#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>



class FormatTextOutput;


class LogMessageDlg : public QWidget
{
    Q_OBJECT

    public:
        LogMessageDlg(QWidget *pParent=nullptr);

        void initDialog(QString const &title, QString const &props);

        void setOutputFont(QFont const &fnt);

        QSize sizeHint() const override {return QSize(750,550);}


    signals:
        void closeLog();

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;

    public slots:
        void onAppendHtmlText(QString const &msg);
        void onAppendPlainText(QString const &msg);
        void onClose();
        void clearText();
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();

    private:
        FormatTextOutput *m_ppto;
        QPushButton *m_ppbClearButton;
        QDialogButtonBox *m_pButtonBox;
};
