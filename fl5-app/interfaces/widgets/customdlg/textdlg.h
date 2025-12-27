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
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QKeyEvent>


class TextDlg : public QDialog
{
    Q_OBJECT

    public:
        TextDlg(QString const &text, QWidget *pParent=nullptr);
        void setQuestion(QString const &quest) {m_plabQuestion->setText(quest);}
        QString const &newText() const {return m_NewText;}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent) override;
        QSize sizeHint() const override {return  QSize(1000,750);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private slots:
        void accept() override;
        void onButton(QAbstractButton*pButton);

    private:
        QDialogButtonBox *m_pButtonBox;
        QLabel *m_plabQuestion;
        QPlainTextEdit *m_ppteText;
        QString m_NewText;
        static QByteArray s_WindowGeometry;
};


