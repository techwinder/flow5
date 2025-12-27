/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>

struct Product {
        int a=0;
        double b = 0.0;
        std::string txt="";
};

class IntEdit;
class PlainTextOutput;
struct MatMultData;

class ThreadTestDlg : public QDialog
{
    Q_OBJECT
    public:
        ThreadTestDlg(QWidget *pParent);


    private slots:
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();
        QSize sizeHint() const override {return QSize(700,900);}
        void customEvent(QEvent*pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void testMulti();

        void testComm();

        void producer();
        void consumer();
        void runComm();

    private:
        QDialogButtonBox *m_pButtonBox;
        PlainTextOutput *m_ppto;

        IntEdit *m_pieNThreads;
        QPushButton *m_ppbMulti;
        QPushButton *m_ppbComm;


        Product m_Prod;

        static QByteArray s_Geometry;
};

