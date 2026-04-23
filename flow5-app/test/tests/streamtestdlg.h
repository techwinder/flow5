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

#include <fstream>
#include <iostream>

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QRadioButton>
#include <QDialogButtonBox>

class PlainTextOutput;

class StreamTestDlg : public QDialog
{
    // declared in the class but not member of the class...
    friend std::ostream& operator - (std::ostream& outstream, const int &n);
    friend std::istream& operator + (std::istream& instream, int &n);

/*    template< typename charT, typename Traits > friend
    std::basic_ostream<charT, Traits>& operator<<(
       std::basic_ostream<charT, Traits>& os, const StreamTestDlg& mc ) {
           // stuff here
           return os;
      }*/

    Q_OBJECT
    public:
        StreamTestDlg(QWidget *pParent);


    private slots:
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();
        QSize sizeHint() const override {return QSize(700,900);}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void save();
        void load();


        std::string tempFilePath();

    private:
        QDialogButtonBox *m_pButtonBox;
        PlainTextOutput *m_ppto;

        QPushButton *m_ppbSave;
        QPushButton *m_ppbLoad;

        static QByteArray s_Geometry;

};

